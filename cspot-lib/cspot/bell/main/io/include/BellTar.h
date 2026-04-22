#pragma once

#include <iostream>
#include <string>

namespace bell::BellTar {
typedef long long unsigned file_size_t;

class writer {
  std::ostream& _dst;

 public:
  writer(std::ostream& dst) : _dst(dst) {}
  ~writer() { finish(); }

  void put(std::string path_in_tar, char const* const data,
           const file_size_t data_size);

  void put_directory(std::string path_in_tar);

  void finish();
};

class reader {
  std::istream& _inp;
  struct {
    std::string file_name;
    file_size_t file_size;
    char file_type;
  } _cached_header_data;
  bool _cached_header_data_valid;
  void _cache_header();
  int _number_of_files;

 public:

  reader(std::istream& inp)
      : _inp(inp), _cached_header_data_valid(false), _number_of_files(-1) {}

  bool contains_another_file();

  std::string get_next_file_name();

  file_size_t get_next_file_size();

  void read_next_file(char* const data);

  char get_next_file_type();

  void extract_all_files(std::string output_dir);

  void skip_next_file();

  int number_of_files();
};
}
