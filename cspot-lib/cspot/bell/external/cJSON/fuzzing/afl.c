

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cJSON.h"

static char *read_file(const char *filename)
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';

cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

int main(int argc, char** argv)
{
    const char *filename = NULL;
    cJSON *item = NULL;
    char *json = NULL;
    int status = EXIT_FAILURE;
    char *printed_json = NULL;

    if ((argc < 2) || (argc > 3))
    {
        printf("Usage:\n");
        printf("%s input_file [enable_printing]\n", argv[0]);
        printf("\t input_file: file containing the test data\n");
        printf("\t enable_printing: print after parsing, 'yes' or 'no', defaults to 'no'\n");
        goto cleanup;
    }

    filename = argv[1];

#if __AFL_HAVE_MANUAL_CONTROL
    while (__AFL_LOOP(1000))
    {
#endif
    status = EXIT_SUCCESS;

    json = read_file(filename);
    if ((json == NULL) || (json[0] == '\0') || (json[1] == '\0'))
    {
        status = EXIT_FAILURE;
        goto cleanup;
    }
    item = cJSON_Parse(json + 2);
    if (item == NULL)
    {
        goto cleanup;
    }

    if ((argc == 3) && (strncmp(argv[2], "yes", 3) == 0))
    {
        int do_format = 0;
        if (json[1] == 'f')
        {
            do_format = 1;
        }

        if (json[0] == 'b')
        {

            printed_json = cJSON_PrintBuffered(item, 1, do_format);
        }
        else
        {

            if (do_format)
            {
                printed_json = cJSON_Print(item);
            }
            else
            {
                printed_json = cJSON_PrintUnformatted(item);
            }
        }
        if (printed_json == NULL)
        {
            status = EXIT_FAILURE;
            goto cleanup;
        }
        printf("%s\n", printed_json);
    }

cleanup:
    if (item != NULL)
    {
        cJSON_Delete(item);
        item = NULL;
    }
    if (json != NULL)
    {
        free(json);
        json = NULL;
    }
    if (printed_json != NULL)
    {
        free(printed_json);
        printed_json = NULL;
    }
#if __AFL_HAVE_MANUAL_CONTROL
    }
#endif

    return status;
}
