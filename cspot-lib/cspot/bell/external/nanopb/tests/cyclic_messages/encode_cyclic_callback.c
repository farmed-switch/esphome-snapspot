

#include <pb_encode.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cyclic_callback.pb.h"

static char *find_end_of_item(char *p)
{
    int depth = 0;
    do {
        if (*p == '[' || *p == '{') depth++;
        if (*p == ']' || *p == '}') depth--;
        p++;
    } while (depth > 0 || (*p != ',' && *p != '}'));

    if (*p == '}')
        return p;

    p++;
    while (*p == ' ') p++;
    return p;
}

static bool encode_tree(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    TreeNode tree = TreeNode_init_zero;
    char *p = (char*)*arg;

    if (*p == '[')
    {

        p++;
        tree.left.funcs.encode = encode_tree;
        tree.left.arg = p;

        p = find_end_of_item(p);
        tree.right.funcs.encode = encode_tree;
        tree.right.arg = p;
    }
    else
    {

        tree.has_leaf = true;
        tree.leaf = atoi(p);
    }

    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_submessage(stream, TreeNode_fields, &tree);
}

static bool encode_dictionary(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    int textlen;
    char *p = (char*)*arg;
    if (*p == '{') p++;
    while (*p != '}')
    {
        KeyValuePair pair = KeyValuePair_init_zero;

        if (*p != '\'')
            PB_RETURN_ERROR(stream, "invalid key, missing quote");

        p++;
        textlen = strchr(p, '\'') - p;
        strncpy(pair.key, p, textlen);
        pair.key[textlen] = 0;
        p += textlen + 2;

        while (*p == ' ') p++;

        if (*p == '[')
        {

            pair.treeValue.funcs.encode = encode_tree;
            pair.treeValue.arg = p;
        }
        else if (*p == '\'')
        {

            pair.has_stringValue = true;
            p++;
            textlen = strchr(p, '\'') - p;
            strncpy(pair.stringValue, p, textlen);
            pair.stringValue[textlen] = 0;
        }
        else if (*p == '{')
        {

            pair.has_dictValue = true;
            pair.dictValue.dictItem.funcs.encode = encode_dictionary;
            pair.dictValue.dictItem.arg = p;
        }
        else
        {

            pair.has_intValue = true;
            pair.intValue = atoi(p);
        }

        p = find_end_of_item(p);

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_submessage(stream, KeyValuePair_fields, &pair))
            return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    Dictionary dict = Dictionary_init_zero;

    if (argc <= 1)
    {
        fprintf(stderr, "Usage: %s \"{'foobar': 1234, ...}\"\n", argv[0]);
        return 1;
    }

    dict.dictItem.funcs.encode = encode_dictionary;
    dict.dictItem.arg = argv[1];

    if (!pb_encode(&stream, Dictionary_fields, &dict))
    {
        fprintf(stderr, "Encoding error: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    fwrite(buffer, 1, stream.bytes_written, stdout);
    return 0;
}

