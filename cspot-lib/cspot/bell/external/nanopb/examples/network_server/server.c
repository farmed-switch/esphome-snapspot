

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
    PB_UNUSED(istream);
    if (ostream != NULL && field->tag == ListFilesResponse_file_tag)
    {
        DIR *dir = *(DIR**)field->pData;
        struct dirent *file;
        FileInfo fileinfo = {};

        while ((file = readdir(dir)) != NULL)
        {
            fileinfo.inode = file->d_ino;
            strncpy(fileinfo.name, file->d_name, sizeof(fileinfo.name));
            fileinfo.name[sizeof(fileinfo.name) - 1] = '\0';

            if (!pb_encode_tag_for_field(ostream, field))
                return false;

            if (!pb_encode_submessage(ostream, FileInfo_fields, &fileinfo))
                return false;
        }

        rewinddir(dir);
    }

    return true;
}

void handle_connection(int connfd)
{
    DIR *directory = NULL;

    {
        ListFilesRequest request = {};
        pb_istream_t input = pb_istream_from_socket(connfd);

        if (!pb_decode_delimited(&input, ListFilesRequest_fields, &request))
        {
            printf("Decode failed: %s\n", PB_GET_ERROR(&input));
            return;
        }

        directory = opendir(request.path);
        printf("Listing directory: %s\n", request.path);
    }

    {
        ListFilesResponse response = {};
        pb_ostream_t output = pb_ostream_from_socket(connfd);

        if (directory == NULL)
        {
            perror("opendir");

            response.has_path_error = true;
            response.path_error = true;
        }
        else
        {

            response.has_path_error = false;
            response.file = directory;
        }

        if (!pb_encode_delimited(&output, ListFilesResponse_fields, &response))
        {
            printf("Encoding failed: %s\n", PB_GET_ERROR(&output));
        }
    }

    if (directory != NULL)
        closedir(directory);
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    int reuse = 1;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(1234);
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(listenfd, 5) != 0)
    {
        perror("listen");
        return 1;
    }

    for(;;)
    {

        connfd = accept(listenfd, NULL, NULL);

        if (connfd < 0)
        {
            perror("accept");
            return 1;
        }

        printf("Got connection.\n");

        handle_connection(connfd);

        printf("Closing connection.\n");

        close(connfd);
    }

    return 0;
}
