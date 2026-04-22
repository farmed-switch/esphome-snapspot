#include "BellTar.h"

#include <sys/stat.h>

using namespace bell::BellTar;

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#endif

#ifdef ENABLE_LOGGING
#define LOG printf
#else
#ifdef _WIN32
#define LOG(fmt, ...) ((void)0)
#else
#define LOG(fmt, args...) ((void)0)
#endif
#endif

const char FILL_CHAR = '\0';
const int FILE_NAME_LENGTH = 100;

typedef enum tar_file_type {
  tar_file_type_normal = '0',
  tar_file_type_hard_link = '1',
  tar_file_type_soft_link = '2',
  tar_file_type_directory = '5'
} tar_file_type_t;

struct tar_header {
  char name[FILE_NAME_LENGTH];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];

  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

void header_set_metadata(tar_header* header) {
  std::memset(header, 0, sizeof(tar_header));

  std::sprintf(header->magic, "ustar");
  std::sprintf(header->mtime, "%011lo", (unsigned long)std::time(NULL));
  std::sprintf(header->mode, "%07o", 0644);
  std::sprintf(header->uname, "unkown");
  std::sprintf(header->gname, "users");
  header->typeflag[0] = 0;
}

void header_set_checksum(tar_header* header) {
  unsigned int sum = 0;

  char* pointer = (char*)header;
  char* end = pointer + sizeof(tar_header);

  while (pointer < header->checksum) {
    sum += *pointer & 0xff;
    pointer++;
  }

  sum += ' ' * 8;
  pointer += 8;

  while (pointer < end) {
    sum += *pointer & 0xff;
    pointer++;
  }

  std::sprintf(header->checksum, "%06o", sum);
}

void header_set_filetype(tar_header* header, tar_file_type_t file_type) {
  header->typeflag[0] = file_type;
}

tar_file_type_t header_get_filetype(tar_header* header) {
  return tar_file_type_t(header->typeflag[0]);
}

void header_set_filesize(tar_header* header, file_size_t file_size) {
  std::sprintf(header->size, "%011llo", file_size);
}

file_size_t header_get_filesize(tar_header* header) {
  file_size_t file_size;
  std::sscanf(header->size, "%011llo", &file_size);
  return file_size;
}

void header_set_filename(tar_header* header, const char* file_name) {
  size_t len = std::strlen(file_name);

  if (len == 0 || len >= FILE_NAME_LENGTH) {
    LOG("Invalid file name for tar: %s\n", file_name);
    std::sprintf(header->name, "INVALID_%d", std::rand());
  } else {
    std::sprintf(header->name, "%s", file_name);
  }
}

std::string header_get_filename(tar_header* header) {
  return std::string(header->name);
}

void _write_header(std::ostream& dst, const char* file_name,
                   file_size_t file_size,
                   tar_file_type_t file_type = tar_file_type_normal) {
  tar_header header;
  header_set_metadata(&header);
  header_set_filename(&header, file_name);
  header_set_filesize(&header, file_size);
  header_set_filetype(&header, file_type);
  header_set_checksum(&header);

  dst.write((const char*)&header, sizeof(tar_header));
}

void _read_header(std::istream& inp, tar_header* header) {
  inp.read((char*)header, sizeof(tar_header));
}

void _fill(std::ostream& dst, unsigned long file_size) {
  while (file_size % sizeof(tar_header) != 0) {
    dst.put(FILL_CHAR);
    file_size++;
  }
}
bool _check_if_header_is_next(std::istream& inp) {
  if (inp.eof() || inp.peek() == EOF) {
    LOG("Can not read next file info, istream at EOF.\n");
    return false;
  }

  if (inp.peek() == FILL_CHAR) {
    LOG("Can not read next file info, istream is pointing "
        "to %d, which a tar header can not start with.\n",
        FILL_CHAR);
    return false;
  }

  return true;
}

void _seek_to_next_header(std::istream& inp) {

  while (inp.peek() == FILL_CHAR)
    inp.get();
}

void writer::put(std::string path_in_tar, char const* const data,
                 const file_size_t data_size) {
  _write_header(_dst, path_in_tar.c_str(), data_size);
  _dst.write(data, data_size);
  _fill(_dst, data_size);
}

void writer::put_directory(std::string path_in_tar) {
  _write_header(_dst, path_in_tar.c_str(), 0, tar_file_type_directory);
}

void writer::finish() {
  unsigned long i = 0;
  while (i < 2 * sizeof(tar_header)) {
    _dst.put(FILL_CHAR);
    i++;
  }
}

bool reader::contains_another_file() {
  return _check_if_header_is_next(_inp);
}

void reader::_cache_header() {
  if (_cached_header_data_valid)
    return;

  assert(contains_another_file());

  tar_header h;
  _read_header(_inp, &h);

  _cached_header_data.file_name = header_get_filename(&h);
  _cached_header_data.file_size = header_get_filesize(&h);
  _cached_header_data.file_type = h.typeflag[0];
  _cached_header_data_valid = true;
}

std::string reader::get_next_file_name() {
  _cache_header();
  return _cached_header_data.file_name;
}

file_size_t reader::get_next_file_size() {
  _cache_header();
  return _cached_header_data.file_size;
}

void reader::read_next_file(char* const data) {
  _inp.read(data, get_next_file_size());

  _cached_header_data_valid = false;
  _seek_to_next_header(_inp);
}

void reader::skip_next_file() {
  _inp.seekg(get_next_file_size(), std::ios::cur);

  _cached_header_data_valid = false;
  _seek_to_next_header(_inp);
}

char reader::get_next_file_type() {
  _cache_header();
  return _cached_header_data.file_type;
}

int reader::number_of_files() {
  if (_number_of_files == -1) {
    std::streampos current_position = _inp.tellg();
    _inp.seekg(0, std::ios::beg);

    _number_of_files = 0;
    while (contains_another_file()) {
      _number_of_files++;
      skip_next_file();
    }

    _inp.seekg(current_position);
  }

  return _number_of_files;
}

void reader::extract_all_files(std::string dest_directory) {
  std::vector<uint8_t> scratch_buffer(1024);

  while (contains_another_file()) {
    char fileType = get_next_file_type();
    auto fileName = get_next_file_name();

#if __cplusplus >= 202002L
    if (fileType == '0' && !fileName.starts_with("._")) {
#else
    if (fileType == '0' && fileName.find("._") != 0) {
#endif
      std::string path = dest_directory + "/" + fileName;

      size_t pos = 0;
      while ((pos = path.find('/', pos)) != std::string::npos) {
        std::string dir = path.substr(0, pos);

#ifdef _WIN32
        mkdir(dir.c_str());
#else
        mkdir(dir.c_str(), 0777);
#endif
        pos++;
      }

      std::ofstream out(path, std::ios::binary);

      size_t read_size = 0;
      size_t file_size = get_next_file_size();

      while (read_size < file_size) {
        size_t to_read = std::min(file_size - read_size, scratch_buffer.size());

        _inp.read((char*)scratch_buffer.data(), to_read);

        read_size += _inp.gcount();

        out.write((char*)scratch_buffer.data(), _inp.gcount());
      }

      _cached_header_data_valid = false;
      _seek_to_next_header(_inp);
    } else {
      skip_next_file();
    }
  }
}