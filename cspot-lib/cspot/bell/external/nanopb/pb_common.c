

#include "pb_common.h"

static bool load_descriptor_values(pb_field_iter_t *iter)
{
    uint32_t word0;
    uint32_t data_offset;
    int_least8_t size_offset;

    if (iter->index >= iter->descriptor->field_count)
        return false;

    word0 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);
    iter->type = (pb_type_t)((word0 >> 8) & 0xFF);

    switch(word0 & 3)
    {
        case 0: {

            iter->array_size = 1;
            iter->tag = (pb_size_t)((word0 >> 2) & 0x3F);
            size_offset = (int_least8_t)((word0 >> 24) & 0x0F);
            data_offset = (word0 >> 16) & 0xFF;
            iter->data_size = (pb_size_t)((word0 >> 28) & 0x0F);
            break;
        }

        case 1: {

            uint32_t word1 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 1]);

            iter->array_size = (pb_size_t)((word0 >> 16) & 0x0FFF);
            iter->tag = (pb_size_t)(((word0 >> 2) & 0x3F) | ((word1 >> 28) << 6));
            size_offset = (int_least8_t)((word0 >> 28) & 0x0F);
            data_offset = word1 & 0xFFFF;
            iter->data_size = (pb_size_t)((word1 >> 16) & 0x0FFF);
            break;
        }

        case 2: {

            uint32_t word1 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 1]);
            uint32_t word2 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 2]);
            uint32_t word3 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 3]);

            iter->array_size = (pb_size_t)(word0 >> 16);
            iter->tag = (pb_size_t)(((word0 >> 2) & 0x3F) | ((word1 >> 8) << 6));
            size_offset = (int_least8_t)(word1 & 0xFF);
            data_offset = word2;
            iter->data_size = (pb_size_t)word3;
            break;
        }

        default: {

            uint32_t word1 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 1]);
            uint32_t word2 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 2]);
            uint32_t word3 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 3]);
            uint32_t word4 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 4]);

            iter->array_size = (pb_size_t)word4;
            iter->tag = (pb_size_t)(((word0 >> 2) & 0x3F) | ((word1 >> 8) << 6));
            size_offset = (int_least8_t)(word1 & 0xFF);
            data_offset = word2;
            iter->data_size = (pb_size_t)word3;
            break;
        }
    }

    if (!iter->message)
    {

        iter->pField = NULL;
        iter->pSize = NULL;
    }
    else
    {
        iter->pField = (char*)iter->message + data_offset;

        if (size_offset)
        {
            iter->pSize = (char*)iter->pField - size_offset;
        }
        else if (PB_HTYPE(iter->type) == PB_HTYPE_REPEATED &&
                 (PB_ATYPE(iter->type) == PB_ATYPE_STATIC ||
                  PB_ATYPE(iter->type) == PB_ATYPE_POINTER))
        {

            iter->pSize = &iter->array_size;
        }
        else
        {
            iter->pSize = NULL;
        }

        if (PB_ATYPE(iter->type) == PB_ATYPE_POINTER && iter->pField != NULL)
        {
            iter->pData = *(void**)iter->pField;
        }
        else
        {
            iter->pData = iter->pField;
        }
    }

    if (PB_LTYPE_IS_SUBMSG(iter->type))
    {
        iter->submsg_desc = iter->descriptor->submsg_info[iter->submessage_index];
    }
    else
    {
        iter->submsg_desc = NULL;
    }

    return true;
}

static void advance_iterator(pb_field_iter_t *iter)
{
    iter->index++;

    if (iter->index >= iter->descriptor->field_count)
    {

        iter->index = 0;
        iter->field_info_index = 0;
        iter->submessage_index = 0;
        iter->required_field_index = 0;
    }
    else
    {

        uint32_t prev_descriptor = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);
        pb_type_t prev_type = (prev_descriptor >> 8) & 0xFF;
        pb_size_t descriptor_len = (pb_size_t)(1 << (prev_descriptor & 3));

        iter->field_info_index = (pb_size_t)(iter->field_info_index + descriptor_len);
        iter->required_field_index = (pb_size_t)(iter->required_field_index + (PB_HTYPE(prev_type) == PB_HTYPE_REQUIRED));
        iter->submessage_index = (pb_size_t)(iter->submessage_index + PB_LTYPE_IS_SUBMSG(prev_type));
    }
}

bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_msgdesc_t *desc, void *message)
{
    memset(iter, 0, sizeof(*iter));

    iter->descriptor = desc;
    iter->message = message;

    return load_descriptor_values(iter);
}

bool pb_field_iter_begin_extension(pb_field_iter_t *iter, pb_extension_t *extension)
{
    const pb_msgdesc_t *msg = (const pb_msgdesc_t*)extension->type->arg;
    bool status;

    uint32_t word0 = PB_PROGMEM_READU32(msg->field_info[0]);
    if (PB_ATYPE(word0 >> 8) == PB_ATYPE_POINTER)
    {

        status = pb_field_iter_begin(iter, msg, &extension->dest);
    }
    else
    {
        status = pb_field_iter_begin(iter, msg, extension->dest);
    }

    iter->pSize = &extension->found;
    return status;
}

bool pb_field_iter_next(pb_field_iter_t *iter)
{
    advance_iterator(iter);
    (void)load_descriptor_values(iter);
    return iter->index != 0;
}

bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag)
{
    if (iter->tag == tag)
    {
        return true;
    }
    else if (tag > iter->descriptor->largest_tag)
    {
        return false;
    }
    else
    {
        pb_size_t start = iter->index;
        uint32_t fieldinfo;

        if (tag < iter->tag)
        {

            iter->index = iter->descriptor->field_count;
        }

        do
        {

            advance_iterator(iter);

            fieldinfo = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);

            if (((fieldinfo >> 2) & 0x3F) == (tag & 0x3F))
            {

                (void)load_descriptor_values(iter);

                if (iter->tag == tag &&
                    PB_LTYPE(iter->type) != PB_LTYPE_EXTENSION)
                {

                    return true;
                }
            }
        } while (iter->index != start);

        (void)load_descriptor_values(iter);
        return false;
    }
}

bool pb_field_iter_find_extension(pb_field_iter_t *iter)
{
    if (PB_LTYPE(iter->type) == PB_LTYPE_EXTENSION)
    {
        return true;
    }
    else
    {
        pb_size_t start = iter->index;
        uint32_t fieldinfo;

        do
        {

            advance_iterator(iter);

            fieldinfo = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);

            if (PB_LTYPE((fieldinfo >> 8) & 0xFF) == PB_LTYPE_EXTENSION)
            {
                return load_descriptor_values(iter);
            }
        } while (iter->index != start);

        (void)load_descriptor_values(iter);
        return false;
    }
}

static void *pb_const_cast(const void *p)
{

    union {
        void *p1;
        const void *p2;
    } t;
    t.p2 = p;
    return t.p1;
}

bool pb_field_iter_begin_const(pb_field_iter_t *iter, const pb_msgdesc_t *desc, const void *message)
{
    return pb_field_iter_begin(iter, desc, pb_const_cast(message));
}

bool pb_field_iter_begin_extension_const(pb_field_iter_t *iter, const pb_extension_t *extension)
{
    return pb_field_iter_begin_extension(iter, (pb_extension_t*)pb_const_cast(extension));
}

bool pb_default_field_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
    if (field->data_size == sizeof(pb_callback_t))
    {
        pb_callback_t *pCallback = (pb_callback_t*)field->pData;

        if (pCallback != NULL)
        {
            if (istream != NULL && pCallback->funcs.decode != NULL)
            {
                return pCallback->funcs.decode(istream, field, &pCallback->arg);
            }

            if (ostream != NULL && pCallback->funcs.encode != NULL)
            {
                return pCallback->funcs.encode(ostream, field, &pCallback->arg);
            }
        }
    }

    return true;

}

#ifdef PB_VALIDATE_UTF8

bool pb_validate_utf8(const char *str)
{
    const pb_byte_t *s = (const pb_byte_t*)str;
    while (*s)
    {
        if (*s < 0x80)
        {

            s++;
        }
        else if ((s[0] & 0xe0) == 0xc0)
        {

            if ((s[1] & 0xc0) != 0x80 ||
                (s[0] & 0xfe) == 0xc0)
                return false;
            else
                s += 2;
        }
        else if ((s[0] & 0xf0) == 0xe0)
        {

            if ((s[1] & 0xc0) != 0x80 ||
                (s[2] & 0xc0) != 0x80 ||
                (s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||
                (s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||
                (s[0] == 0xef && s[1] == 0xbf &&
                (s[2] & 0xfe) == 0xbe))
                return false;
            else
                s += 3;
        }
        else if ((s[0] & 0xf8) == 0xf0)
        {

            if ((s[1] & 0xc0) != 0x80 ||
                (s[2] & 0xc0) != 0x80 ||
                (s[3] & 0xc0) != 0x80 ||
                (s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||
                (s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4)
                return false;
            else
                s += 4;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#endif

