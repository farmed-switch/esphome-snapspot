

#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUCC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning (push)

#pragma warning (disable : 4001)
#endif

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <math.h>

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
#ifdef __GNUCC__
#pragma GCC visibility pop
#endif

#include "cJSON_Utils.h"

#ifdef true
#undef true
#endif
#define true ((cJSON_bool)1)

#ifdef false
#undef false
#endif
#define false ((cJSON_bool)0)

static unsigned char* cJSONUtils_strdup(const unsigned char* const string)
{
    size_t length = 0;
    unsigned char *copy = NULL;

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*) cJSON_malloc(length);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

static int compare_strings(const unsigned char *string1, const unsigned char *string2, const cJSON_bool case_sensitive)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    if (case_sensitive)
    {
        return strcmp((const char*)string1, (const char*)string2);
    }

    for(; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

static cJSON_bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static cJSON_bool compare_pointers(const unsigned char *name, const unsigned char *pointer, const cJSON_bool case_sensitive)
{
    if ((name == NULL) || (pointer == NULL))
    {
        return false;
    }

    for (; (*name != '\0') && (*pointer != '\0') && (*pointer != '/'); (void)name++, pointer++)
    {
        if (*pointer == '~')
        {

            if (((pointer[1] != '0') || (*name != '~')) && ((pointer[1] != '1') || (*name != '/')))
            {

                return false;
            }
            else
            {
                pointer++;
            }
        }
        else if ((!case_sensitive && (tolower(*name) != tolower(*pointer))) || (case_sensitive && (*name != *pointer)))
        {
            return false;
        }
    }
    if (((*pointer != 0) && (*pointer != '/')) != (*name != 0))
    {

        return false;;
    }

    return true;
}

static size_t pointer_encoded_length(const unsigned char *string)
{
    size_t length;
    for (length = 0; *string != '\0'; (void)string++, length++)
    {

        if ((*string == '~') || (*string == '/'))
        {
            length++;
        }
    }

    return length;
}

static void encode_string_as_pointer(unsigned char *destination, const unsigned char *source)
{
    for (; source[0] != '\0'; (void)source++, destination++)
    {
        if (source[0] == '/')
        {
            destination[0] = '~';
            destination[1] = '1';
            destination++;
        }
        else if (source[0] == '~')
        {
            destination[0] = '~';
            destination[1] = '0';
            destination++;
        }
        else
        {
            destination[0] = source[0];
        }
    }

    destination[0] = '\0';
}

CJSON_PUBLIC(char *) cJSONUtils_FindPointerFromObjectTo(const cJSON * const object, const cJSON * const target)
{
    size_t child_index = 0;
    cJSON *current_child = 0;

    if ((object == NULL) || (target == NULL))
    {
        return NULL;
    }

    if (object == target)
    {

        return (char*)cJSONUtils_strdup((const unsigned char*)"");
    }

    for (current_child = object->child; current_child != NULL; (void)(current_child = current_child->next), child_index++)
    {
        unsigned char *target_pointer = (unsigned char*)cJSONUtils_FindPointerFromObjectTo(current_child, target);

        if (target_pointer != NULL)
        {
            if (cJSON_IsArray(object))
            {

                unsigned char *full_pointer = (unsigned char*)cJSON_malloc(strlen((char*)target_pointer) + 20 + sizeof("/"));

                if (child_index > ULONG_MAX)
                {
                    cJSON_free(target_pointer);
                    cJSON_free(full_pointer);
                    return NULL;
                }
                sprintf((char*)full_pointer, "/%lu%s", (unsigned long)child_index, target_pointer);
                cJSON_free(target_pointer);

                return (char*)full_pointer;
            }

            if (cJSON_IsObject(object))
            {
                unsigned char *full_pointer = (unsigned char*)cJSON_malloc(strlen((char*)target_pointer) + pointer_encoded_length((unsigned char*)current_child->string) + 2);
                full_pointer[0] = '/';
                encode_string_as_pointer(full_pointer + 1, (unsigned char*)current_child->string);
                strcat((char*)full_pointer, (char*)target_pointer);
                cJSON_free(target_pointer);

                return (char*)full_pointer;
            }

            cJSON_free(target_pointer);
            return NULL;
        }
    }

    return NULL;
}

static cJSON *get_array_item(const cJSON *array, size_t item)
{
    cJSON *child = array ? array->child : NULL;
    while ((child != NULL) && (item > 0))
    {
        item--;
        child = child->next;
    }

    return child;
}

static cJSON_bool decode_array_index_from_pointer(const unsigned char * const pointer, size_t * const index)
{
    size_t parsed_index = 0;
    size_t position = 0;

    if ((pointer[0] == '0') && ((pointer[1] != '\0') && (pointer[1] != '/')))
    {

        return 0;
    }

    for (position = 0; (pointer[position] >= '0') && (pointer[0] <= '9'); position++)
    {
        parsed_index = (10 * parsed_index) + (size_t)(pointer[position] - '0');

    }

    if ((pointer[position] != '\0') && (pointer[position] != '/'))
    {
        return 0;
    }

    *index = parsed_index;

    return 1;
}

static cJSON *get_item_from_pointer(cJSON * const object, const char * pointer, const cJSON_bool case_sensitive)
{
    cJSON *current_element = object;

    if (pointer == NULL)
    {
        return NULL;
    }

    while ((pointer[0] == '/') && (current_element != NULL))
    {
        pointer++;
        if (cJSON_IsArray(current_element))
        {
            size_t index = 0;
            if (!decode_array_index_from_pointer((const unsigned char*)pointer, &index))
            {
                return NULL;
            }

            current_element = get_array_item(current_element, index);
        }
        else if (cJSON_IsObject(current_element))
        {
            current_element = current_element->child;

            while ((current_element != NULL) && !compare_pointers((unsigned char*)current_element->string, (const unsigned char*)pointer, case_sensitive))
            {
                current_element = current_element->next;
            }
        }
        else
        {
            return NULL;
        }

        while ((pointer[0] != '\0') && (pointer[0] != '/'))
        {
            pointer++;
        }
    }

    return current_element;
}

CJSON_PUBLIC(cJSON *) cJSONUtils_GetPointer(cJSON * const object, const char *pointer)
{
    return get_item_from_pointer(object, pointer, false);
}

CJSON_PUBLIC(cJSON *) cJSONUtils_GetPointerCaseSensitive(cJSON * const object, const char *pointer)
{
    return get_item_from_pointer(object, pointer, true);
}

static void decode_pointer_inplace(unsigned char *string)
{
    unsigned char *decoded_string = string;

    if (string == NULL) {
        return;
    }

    for (; *string; (void)decoded_string++, string++)
    {
        if (string[0] == '~')
        {
            if (string[1] == '0')
            {
                decoded_string[0] = '~';
            }
            else if (string[1] == '1')
            {
                decoded_string[1] = '/';
            }
            else
            {

                return;
            }

            string++;
        }
    }

    decoded_string[0] = '\0';
}

static cJSON *detach_item_from_array(cJSON *array, size_t which)
{
    cJSON *c = array->child;
    while (c && (which > 0))
    {
        c = c->next;
        which--;
    }
    if (!c)
    {

        return NULL;
    }
    if (c != array->child)
    {

        c->prev->next = c->next;
    }
    if (c->next)
    {
        c->next->prev = c->prev;
    }
    if (c == array->child)
    {
        array->child = c->next;
    }
    else if (c->next == NULL)
    {
        array->child->prev = c->prev;
    }

    c->prev = c->next = NULL;

    return c;
}

static cJSON *detach_path(cJSON *object, const unsigned char *path, const cJSON_bool case_sensitive)
{
    unsigned char *parent_pointer = NULL;
    unsigned char *child_pointer = NULL;
    cJSON *parent = NULL;
    cJSON *detached_item = NULL;

    parent_pointer = cJSONUtils_strdup(path);
    if (parent_pointer == NULL) {
        goto cleanup;
    }

    child_pointer = (unsigned char*)strrchr((char*)parent_pointer, '/');
    if (child_pointer == NULL)
    {
        goto cleanup;
    }

    child_pointer[0] = '\0';
    child_pointer++;

    parent = get_item_from_pointer(object, (char*)parent_pointer, case_sensitive);
    decode_pointer_inplace(child_pointer);

    if (cJSON_IsArray(parent))
    {
        size_t index = 0;
        if (!decode_array_index_from_pointer(child_pointer, &index))
        {
            goto cleanup;
        }
        detached_item = detach_item_from_array(parent, index);
    }
    else if (cJSON_IsObject(parent))
    {
        detached_item = cJSON_DetachItemFromObject(parent, (char*)child_pointer);
    }
    else
    {

        goto cleanup;
    }

cleanup:
    if (parent_pointer != NULL)
    {
        cJSON_free(parent_pointer);
    }

    return detached_item;
}

static cJSON *sort_list(cJSON *list, const cJSON_bool case_sensitive)
{
    cJSON *first = list;
    cJSON *second = list;
    cJSON *current_item = list;
    cJSON *result = list;
    cJSON *result_tail = NULL;

    if ((list == NULL) || (list->next == NULL))
    {

        return result;
    }

    while ((current_item != NULL) && (current_item->next != NULL) && (compare_strings((unsigned char*)current_item->string, (unsigned char*)current_item->next->string, case_sensitive) < 0))
    {

        current_item = current_item->next;
    }
    if ((current_item == NULL) || (current_item->next == NULL))
    {

        return result;
    }

    current_item = list;
    while (current_item != NULL)
    {

        second = second->next;
        current_item = current_item->next;

        if (current_item != NULL)
        {
            current_item = current_item->next;
        }
    }
    if ((second != NULL) && (second->prev != NULL))
    {

        second->prev->next = NULL;
        second->prev = NULL;
    }

    first = sort_list(first, case_sensitive);
    second = sort_list(second, case_sensitive);
    result = NULL;

    while ((first != NULL) && (second != NULL))
    {
        cJSON *smaller = NULL;
        if (compare_strings((unsigned char*)first->string, (unsigned char*)second->string, case_sensitive) < 0)
        {
            smaller = first;
        }
        else
        {
            smaller = second;
        }

        if (result == NULL)
        {

            result_tail = smaller;
            result = smaller;
        }
        else
        {

            result_tail->next = smaller;
            smaller->prev = result_tail;
            result_tail = smaller;
        }

        if (first == smaller)
        {
            first = first->next;
        }
        else
        {
            second = second->next;
        }
    }

    if (first != NULL)
    {

        if (result == NULL)
        {
            return first;
        }
        result_tail->next = first;
        first->prev = result_tail;
    }
    if (second != NULL)
    {

        if (result == NULL)
        {
            return second;
        }
        result_tail->next = second;
        second->prev = result_tail;
    }

    return result;
}

static void sort_object(cJSON * const object, const cJSON_bool case_sensitive)
{
    if (object == NULL)
    {
        return;
    }
    object->child = sort_list(object->child, case_sensitive);
}

static cJSON_bool compare_json(cJSON *a, cJSON *b, const cJSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)))
    {

        return false;
    }
    switch (a->type & 0xFF)
    {
        case cJSON_Number:

            if ((a->valueint != b->valueint) || (!compare_double(a->valuedouble, b->valuedouble)))
            {
                return false;
            }
            else
            {
                return true;
            }

        case cJSON_String:

            if (strcmp(a->valuestring, b->valuestring) != 0)
            {
                return false;
            }
            else
            {
                return true;
            }

        case cJSON_Array:
            for ((void)(a = a->child), b = b->child; (a != NULL) && (b != NULL); (void)(a = a->next), b = b->next)
            {
                cJSON_bool identical = compare_json(a, b, case_sensitive);
                if (!identical)
                {
                    return false;
                }
            }

            if ((a != NULL) || (b != NULL))
            {
                return false;
            }
            else
            {
                return true;
            }

        case cJSON_Object:
            sort_object(a, case_sensitive);
            sort_object(b, case_sensitive);
            for ((void)(a = a->child), b = b->child; (a != NULL) && (b != NULL); (void)(a = a->next), b = b->next)
            {
                cJSON_bool identical = false;

                if (compare_strings((unsigned char*)a->string, (unsigned char*)b->string, case_sensitive))
                {

                    return false;
                }
                identical = compare_json(a, b, case_sensitive);
                if (!identical)
                {
                    return false;
                }
            }

            if ((a != NULL) || (b != NULL))
            {
                return false;
            }
            else
            {
                return true;
            }

        default:
            break;
    }

    return true;
}

static cJSON_bool insert_item_in_array(cJSON *array, size_t which, cJSON *newitem)
{
    cJSON *child = array->child;
    while (child && (which > 0))
    {
        child = child->next;
        which--;
    }
    if (which > 0)
    {

        return 0;
    }
    if (child == NULL)
    {
        cJSON_AddItemToArray(array, newitem);
        return 1;
    }

    newitem->next = child;
    newitem->prev = child->prev;
    child->prev = newitem;

    if (child == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }

    return 1;
}

static cJSON *get_object_item(const cJSON * const object, const char* name, const cJSON_bool case_sensitive)
{
    if (case_sensitive)
    {
        return cJSON_GetObjectItemCaseSensitive(object, name);
    }

    return cJSON_GetObjectItem(object, name);
}

enum patch_operation { INVALID, ADD, REMOVE, REPLACE, MOVE, COPY, TEST };

static enum patch_operation decode_patch_operation(const cJSON * const patch, const cJSON_bool case_sensitive)
{
    cJSON *operation = get_object_item(patch, "op", case_sensitive);
    if (!cJSON_IsString(operation))
    {
        return INVALID;
    }

    if (strcmp(operation->valuestring, "add") == 0)
    {
        return ADD;
    }

    if (strcmp(operation->valuestring, "remove") == 0)
    {
        return REMOVE;
    }

    if (strcmp(operation->valuestring, "replace") == 0)
    {
        return REPLACE;
    }

    if (strcmp(operation->valuestring, "move") == 0)
    {
        return MOVE;
    }

    if (strcmp(operation->valuestring, "copy") == 0)
    {
        return COPY;
    }

    if (strcmp(operation->valuestring, "test") == 0)
    {
        return TEST;
    }

    return INVALID;
}

static void overwrite_item(cJSON * const root, const cJSON replacement)
{
    if (root == NULL)
    {
        return;
    }

    if (root->string != NULL)
    {
        cJSON_free(root->string);
    }
    if (root->valuestring != NULL)
    {
        cJSON_free(root->valuestring);
    }
    if (root->child != NULL)
    {
        cJSON_Delete(root->child);
    }

    memcpy(root, &replacement, sizeof(cJSON));
}

static int apply_patch(cJSON *object, const cJSON *patch, const cJSON_bool case_sensitive)
{
    cJSON *path = NULL;
    cJSON *value = NULL;
    cJSON *parent = NULL;
    enum patch_operation opcode = INVALID;
    unsigned char *parent_pointer = NULL;
    unsigned char *child_pointer = NULL;
    int status = 0;

    path = get_object_item(patch, "path", case_sensitive);
    if (!cJSON_IsString(path))
    {

        status = 2;
        goto cleanup;
    }

    opcode = decode_patch_operation(patch, case_sensitive);
    if (opcode == INVALID)
    {
        status = 3;
        goto cleanup;
    }
    else if (opcode == TEST)
    {

        status = !compare_json(get_item_from_pointer(object, path->valuestring, case_sensitive), get_object_item(patch, "value", case_sensitive), case_sensitive);
        goto cleanup;
    }

    if (path->valuestring[0] == '\0')
    {
        if (opcode == REMOVE)
        {
            static const cJSON invalid = { NULL, NULL, NULL, cJSON_Invalid, NULL, 0, 0, NULL};

            overwrite_item(object, invalid);

            status = 0;
            goto cleanup;
        }

        if ((opcode == REPLACE) || (opcode == ADD))
        {
            value = get_object_item(patch, "value", case_sensitive);
            if (value == NULL)
            {

                status = 7;
                goto cleanup;
            }

            value = cJSON_Duplicate(value, 1);
            if (value == NULL)
            {

                status = 8;
                goto cleanup;
            }

            overwrite_item(object, *value);

            cJSON_free(value);
            value = NULL;

            if (object->string != NULL)
            {
                cJSON_free(object->string);
                object->string = NULL;
            }

            status = 0;
            goto cleanup;
        }
    }

    if ((opcode == REMOVE) || (opcode == REPLACE))
    {

        cJSON *old_item = detach_path(object, (unsigned char*)path->valuestring, case_sensitive);
        if (old_item == NULL)
        {
            status = 13;
            goto cleanup;
        }
        cJSON_Delete(old_item);
        if (opcode == REMOVE)
        {

            status = 0;
            goto cleanup;
        }
    }

    if ((opcode == MOVE) || (opcode == COPY))
    {
        cJSON *from = get_object_item(patch, "from", case_sensitive);
        if (from == NULL)
        {

            status = 4;
            goto cleanup;
        }

        if (opcode == MOVE)
        {
            value = detach_path(object, (unsigned char*)from->valuestring, case_sensitive);
        }
        if (opcode == COPY)
        {
            value = get_item_from_pointer(object, from->valuestring, case_sensitive);
        }
        if (value == NULL)
        {

            status = 5;
            goto cleanup;
        }
        if (opcode == COPY)
        {
            value = cJSON_Duplicate(value, 1);
        }
        if (value == NULL)
        {

            status = 6;
            goto cleanup;
        }
    }
    else
    {
        value = get_object_item(patch, "value", case_sensitive);
        if (value == NULL)
        {

            status = 7;
            goto cleanup;
        }
        value = cJSON_Duplicate(value, 1);
        if (value == NULL)
        {

            status = 8;
            goto cleanup;
        }
    }

    parent_pointer = cJSONUtils_strdup((unsigned char*)path->valuestring);
    if (parent_pointer) {
        child_pointer = (unsigned char*)strrchr((char*)parent_pointer, '/');
    }
    if (child_pointer != NULL)
    {
        child_pointer[0] = '\0';
        child_pointer++;
    }
    parent = get_item_from_pointer(object, (char*)parent_pointer, case_sensitive);
    decode_pointer_inplace(child_pointer);

    if ((parent == NULL) || (child_pointer == NULL))
    {

        status = 9;
        goto cleanup;
    }
    else if (cJSON_IsArray(parent))
    {
        if (strcmp((char*)child_pointer, "-") == 0)
        {
            cJSON_AddItemToArray(parent, value);
            value = NULL;
        }
        else
        {
            size_t index = 0;
            if (!decode_array_index_from_pointer(child_pointer, &index))
            {
                status = 11;
                goto cleanup;
            }

            if (!insert_item_in_array(parent, index, value))
            {
                status = 10;
                goto cleanup;
            }
            value = NULL;
        }
    }
    else if (cJSON_IsObject(parent))
    {
        if (case_sensitive)
        {
            cJSON_DeleteItemFromObjectCaseSensitive(parent, (char*)child_pointer);
        }
        else
        {
            cJSON_DeleteItemFromObject(parent, (char*)child_pointer);
        }
        cJSON_AddItemToObject(parent, (char*)child_pointer, value);
        value = NULL;
    }
    else
    {

        status = 9;
        goto cleanup;
    }

cleanup:
    if (value != NULL)
    {
        cJSON_Delete(value);
    }
    if (parent_pointer != NULL)
    {
        cJSON_free(parent_pointer);
    }

    return status;
}

CJSON_PUBLIC(int) cJSONUtils_ApplyPatches(cJSON * const object, const cJSON * const patches)
{
    const cJSON *current_patch = NULL;
    int status = 0;

    if (!cJSON_IsArray(patches))
    {

        return 1;
    }

    if (patches != NULL)
    {
        current_patch = patches->child;
    }

    while (current_patch != NULL)
    {
        status = apply_patch(object, current_patch, false);
        if (status != 0)
        {
            return status;
        }
        current_patch = current_patch->next;
    }

    return 0;
}

CJSON_PUBLIC(int) cJSONUtils_ApplyPatchesCaseSensitive(cJSON * const object, const cJSON * const patches)
{
    const cJSON *current_patch = NULL;
    int status = 0;

    if (!cJSON_IsArray(patches))
    {

        return 1;
    }

    if (patches != NULL)
    {
        current_patch = patches->child;
    }

    while (current_patch != NULL)
    {
        status = apply_patch(object, current_patch, true);
        if (status != 0)
        {
            return status;
        }
        current_patch = current_patch->next;
    }

    return 0;
}

static void compose_patch(cJSON * const patches, const unsigned char * const operation, const unsigned char * const path, const unsigned char *suffix, const cJSON * const value)
{
    cJSON *patch = NULL;

    if ((patches == NULL) || (operation == NULL) || (path == NULL))
    {
        return;
    }

    patch = cJSON_CreateObject();
    if (patch == NULL)
    {
        return;
    }
    cJSON_AddItemToObject(patch, "op", cJSON_CreateString((const char*)operation));

    if (suffix == NULL)
    {
        cJSON_AddItemToObject(patch, "path", cJSON_CreateString((const char*)path));
    }
    else
    {
        size_t suffix_length = pointer_encoded_length(suffix);
        size_t path_length = strlen((const char*)path);
        unsigned char *full_path = (unsigned char*)cJSON_malloc(path_length + suffix_length + sizeof("/"));

        sprintf((char*)full_path, "%s/", (const char*)path);
        encode_string_as_pointer(full_path + path_length + 1, suffix);

        cJSON_AddItemToObject(patch, "path", cJSON_CreateString((const char*)full_path));
        cJSON_free(full_path);
    }

    if (value != NULL)
    {
        cJSON_AddItemToObject(patch, "value", cJSON_Duplicate(value, 1));
    }
    cJSON_AddItemToArray(patches, patch);
}

CJSON_PUBLIC(void) cJSONUtils_AddPatchToArray(cJSON * const array, const char * const operation, const char * const path, const cJSON * const value)
{
    compose_patch(array, (const unsigned char*)operation, (const unsigned char*)path, NULL, value);
}

static void create_patches(cJSON * const patches, const unsigned char * const path, cJSON * const from, cJSON * const to, const cJSON_bool case_sensitive)
{
    if ((from == NULL) || (to == NULL))
    {
        return;
    }

    if ((from->type & 0xFF) != (to->type & 0xFF))
    {
        compose_patch(patches, (const unsigned char*)"replace", path, 0, to);
        return;
    }

    switch (from->type & 0xFF)
    {
        case cJSON_Number:
            if ((from->valueint != to->valueint) || !compare_double(from->valuedouble, to->valuedouble))
            {
                compose_patch(patches, (const unsigned char*)"replace", path, NULL, to);
            }
            return;

        case cJSON_String:
            if (strcmp(from->valuestring, to->valuestring) != 0)
            {
                compose_patch(patches, (const unsigned char*)"replace", path, NULL, to);
            }
            return;

        case cJSON_Array:
        {
            size_t index = 0;
            cJSON *from_child = from->child;
            cJSON *to_child = to->child;
            unsigned char *new_path = (unsigned char*)cJSON_malloc(strlen((const char*)path) + 20 + sizeof("/"));

            for (index = 0; (from_child != NULL) && (to_child != NULL); (void)(from_child = from_child->next), (void)(to_child = to_child->next), index++)
            {

                if (index > ULONG_MAX)
                {
                    cJSON_free(new_path);
                    return;
                }
                sprintf((char*)new_path, "%s/%lu", path, (unsigned long)index);
                create_patches(patches, new_path, from_child, to_child, case_sensitive);
            }

            for (; (from_child != NULL); (void)(from_child = from_child->next))
            {

                if (index > ULONG_MAX)
                {
                    cJSON_free(new_path);
                    return;
                }
                sprintf((char*)new_path, "%lu", (unsigned long)index);
                compose_patch(patches, (const unsigned char*)"remove", path, new_path, NULL);
            }

            for (; (to_child != NULL); (void)(to_child = to_child->next), index++)
            {
                compose_patch(patches, (const unsigned char*)"add", path, (const unsigned char*)"-", to_child);
            }
            cJSON_free(new_path);
            return;
        }

        case cJSON_Object:
        {
            cJSON *from_child = NULL;
            cJSON *to_child = NULL;
            sort_object(from, case_sensitive);
            sort_object(to, case_sensitive);

            from_child = from->child;
            to_child = to->child;

            while ((from_child != NULL) || (to_child != NULL))
            {
                int diff;
                if (from_child == NULL)
                {
                    diff = 1;
                }
                else if (to_child == NULL)
                {
                    diff = -1;
                }
                else
                {
                    diff = compare_strings((unsigned char*)from_child->string, (unsigned char*)to_child->string, case_sensitive);
                }

                if (diff == 0)
                {

                    size_t path_length = strlen((const char*)path);
                    size_t from_child_name_length = pointer_encoded_length((unsigned char*)from_child->string);
                    unsigned char *new_path = (unsigned char*)cJSON_malloc(path_length + from_child_name_length + sizeof("/"));

                    sprintf((char*)new_path, "%s/", path);
                    encode_string_as_pointer(new_path + path_length + 1, (unsigned char*)from_child->string);

                    create_patches(patches, new_path, from_child, to_child, case_sensitive);
                    cJSON_free(new_path);

                    from_child = from_child->next;
                    to_child = to_child->next;
                }
                else if (diff < 0)
                {

                    compose_patch(patches, (const unsigned char*)"remove", path, (unsigned char*)from_child->string, NULL);

                    from_child = from_child->next;
                }
                else
                {

                    compose_patch(patches, (const unsigned char*)"add", path, (unsigned char*)to_child->string, to_child);

                    to_child = to_child->next;
                }
            }
            return;
        }

        default:
            break;
    }
}

CJSON_PUBLIC(cJSON *) cJSONUtils_GeneratePatches(cJSON * const from, cJSON * const to)
{
    cJSON *patches = NULL;

    if ((from == NULL) || (to == NULL))
    {
        return NULL;
    }

    patches = cJSON_CreateArray();
    create_patches(patches, (const unsigned char*)"", from, to, false);

    return patches;
}

CJSON_PUBLIC(cJSON *) cJSONUtils_GeneratePatchesCaseSensitive(cJSON * const from, cJSON * const to)
{
    cJSON *patches = NULL;

    if ((from == NULL) || (to == NULL))
    {
        return NULL;
    }

    patches = cJSON_CreateArray();
    create_patches(patches, (const unsigned char*)"", from, to, true);

    return patches;
}

CJSON_PUBLIC(void) cJSONUtils_SortObject(cJSON * const object)
{
    sort_object(object, false);
}

CJSON_PUBLIC(void) cJSONUtils_SortObjectCaseSensitive(cJSON * const object)
{
    sort_object(object, true);
}

static cJSON *merge_patch(cJSON *target, const cJSON * const patch, const cJSON_bool case_sensitive)
{
    cJSON *patch_child = NULL;

    if (!cJSON_IsObject(patch))
    {

        cJSON_Delete(target);
        return cJSON_Duplicate(patch, 1);
    }

    if (!cJSON_IsObject(target))
    {
        cJSON_Delete(target);
        target = cJSON_CreateObject();
    }

    patch_child = patch->child;
    while (patch_child != NULL)
    {
        if (cJSON_IsNull(patch_child))
        {

            if (case_sensitive)
            {
                cJSON_DeleteItemFromObjectCaseSensitive(target, patch_child->string);
            }
            else
            {
                cJSON_DeleteItemFromObject(target, patch_child->string);
            }
        }
        else
        {
            cJSON *replace_me = NULL;
            cJSON *replacement = NULL;

            if (case_sensitive)
            {
                replace_me = cJSON_DetachItemFromObjectCaseSensitive(target, patch_child->string);
            }
            else
            {
                replace_me = cJSON_DetachItemFromObject(target, patch_child->string);
            }

            replacement = merge_patch(replace_me, patch_child, case_sensitive);
            if (replacement == NULL)
            {
                cJSON_Delete(target);
                return NULL;
            }

            cJSON_AddItemToObject(target, patch_child->string, replacement);
        }
        patch_child = patch_child->next;
    }
    return target;
}

CJSON_PUBLIC(cJSON *) cJSONUtils_MergePatch(cJSON *target, const cJSON * const patch)
{
    return merge_patch(target, patch, false);
}

CJSON_PUBLIC(cJSON *) cJSONUtils_MergePatchCaseSensitive(cJSON *target, const cJSON * const patch)
{
    return merge_patch(target, patch, true);
}

static cJSON *generate_merge_patch(cJSON * const from, cJSON * const to, const cJSON_bool case_sensitive)
{
    cJSON *from_child = NULL;
    cJSON *to_child = NULL;
    cJSON *patch = NULL;
    if (to == NULL)
    {

        return cJSON_CreateNull();
    }
    if (!cJSON_IsObject(to) || !cJSON_IsObject(from))
    {
        return cJSON_Duplicate(to, 1);
    }

    sort_object(from, case_sensitive);
    sort_object(to, case_sensitive);

    from_child = from->child;
    to_child = to->child;
    patch = cJSON_CreateObject();
    if (patch == NULL)
    {
        return NULL;
    }
    while (from_child || to_child)
    {
        int diff;
        if (from_child != NULL)
        {
            if (to_child != NULL)
            {
                diff = strcmp(from_child->string, to_child->string);
            }
            else
            {
                diff = -1;
            }
        }
        else
        {
            diff = 1;
        }

        if (diff < 0)
        {

            cJSON_AddItemToObject(patch, from_child->string, cJSON_CreateNull());

            from_child = from_child->next;
        }
        else if (diff > 0)
        {

            cJSON_AddItemToObject(patch, to_child->string, cJSON_Duplicate(to_child, 1));

            to_child = to_child->next;
        }
        else
        {

            if (!compare_json(from_child, to_child, case_sensitive))
            {

                cJSON_AddItemToObject(patch, to_child->string, cJSONUtils_GenerateMergePatch(from_child, to_child));
            }

            from_child = from_child->next;
            to_child = to_child->next;
        }
    }
    if (patch->child == NULL)
    {

        cJSON_Delete(patch);
        return NULL;
    }

    return patch;
}

CJSON_PUBLIC(cJSON *) cJSONUtils_GenerateMergePatch(cJSON * const from, cJSON * const to)
{
    return generate_merge_patch(from, to, false);
}

CJSON_PUBLIC(cJSON *) cJSONUtils_GenerateMergePatchCaseSensitive(cJSON * const from, cJSON * const to)
{
    return generate_merge_patch(from, to, true);
}
