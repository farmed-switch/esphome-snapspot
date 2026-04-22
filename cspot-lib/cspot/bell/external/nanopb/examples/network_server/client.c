

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "fileproto.pb.h"
#include "common.h"

bool ListFilesResponse_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t *field)
{
    PB_UNUSED(ostream);
    if (istream != NULL && field->tag == ListFilesResponse_file_tag)
    {
        FileInfo fileinfo = {};

        if (!pb_decode(istream, FileInfo_fields, &fileinfo))
            return false;

        printf("%-10lld %s\n", (long long)fileinfo.inode, fileinfo.name);
    }

    return true;
}

bool listdir(int fd, char *path)
{

    {
        ListFilesRequest request = {};
        pb_ostream_t output = pb_ostream_from_socket(fd);

        if (path == NULL)
        {
            request.has_path = false;
        }
        else
        {
            request.has_path = true;
            if (strlen(path) + 1 > sizeof(request.path))
            {
                fprintf(stderr, "Too long path.\n");
                return false;
            }

            strcpy(request.path, path);
        }

        if (!pb_encode_delimited(&output, ListFilesRequest_fields, &request))
        {
            fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&output));
            return false;
        }
    }

    {
        ListFilesResponse response = {};
        pb_istream_t input = pb_istream_from_socket(fd);

        if (!pb_decode_delimited(&input, ListFilesResponse_fields, &response))
        {
            fprintf(stderr, "Decode failed: %s\n", PB_GET_ERROR(&input));
            return false;
        }

        if (response.path_error)
        {
            fprintf(stderr, "Server reported error.\n");
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    char *path = NULL;

    if (argc > 1)
        path = argv[1];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(1234);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect");
        return 1;
    }

    if (!listdir(sockfd, path))
        return 2;

    close(sockfd);

    return 0;
}
