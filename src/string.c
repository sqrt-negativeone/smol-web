#define s8_lit(str_lit) s8{(char *)str_lit, sizeof(str_lit) - 1}
#define s8_const_array(str_array) {(s8 *)str_array, sizeof(str_array) / sizeof((str_array)[0])}

internal_function
s8 s8_split(s8 *str, char sep)
{
    s8 result = *str;
    char *buf;
    for (buf = result.str; *buf && (*buf != sep); ++buf);
    *buf = 0;
    result.len = buf - result.str;
    str->str += result.len + 1;
    return result;
}

internal_function
b32 s8_compare_equal(s8 a, s8 b)
{
    b32 result = 0;
    if (a.len == b.len)
    {
        u32 i = 0;
        for (; i < a.len && a.str[i] == b.str[i]; ++i);
        if (i == a.len)
        {
            result = 1;
        }
    }
    return result;
}

internal_function void
s8_append_str(s8 *dest, s8 src)
{
    if (dest)
    {
        char *src_ptr = (char *)src.str;
        char *dest_ptr = (char *)dest->str;
        for (; *src_ptr; ++src_ptr)
        {
            *dest_ptr = *src_ptr;
        }
        *dest_ptr = 0;
        dest->len += src.len;
    }
}

internal_function b32
s8_substring(s8 src, s8 pattern)
{
    b32 result = 0;
    for (u32 i = 0, max_n = src.len - pattern.len; i <= max_n; ++i)
    {
        result = 1;
        for (u32 j = 0; j < pattern.len; ++j)
        {
            if (src.str[i + j] != pattern.str[j])
            {
                result = 0;
                break;
            }
        }
        if (result)
        {
            break;
        }
    }
    return result;
}

internal_function s8
s8_int_to_str(Memory_Arena *arena, u32 n)
{
    s8 result = {};
    char buf[20];
    buf[0] = '0';
    u32 n_len = 0;
    while(n)
    {
        buf[n_len] = '0' + (n % 10);
        n /= 10;
        ++n_len;
    }
    result.str = (char *)Push_Arena(arena, n_len);
    result.len = n_len;
    for (u32 i = 0; i < n_len; ++i)
    {
        result.str[n_len - i - 1] = buf[i];
    }
    return result;
}

internal_function s8
s8_copy(Memory_Arena *arena, s8 src)
{
    s8 result = {};
    result.str = (char *)Push_Arena(arena, src.len + 1);
    if (result.str)
    {
        result.len = src.len;
        for (u32 i = 0; i < result.len; ++i)
        {
            result.str[i] = src.str[i];
        }
    }
    result.str[result.len] = 0;
    return result;
}

internal_function s8
s8_concat_array(Memory_Arena *arena, s8_array strings)
{
    s8 result = {};
    if (arena)
    {
        u32 strings_len = 0;
        for (u32 i = 0; i < strings.size; ++i)
        {
            strings_len += strings.array[i].len;
        }
        result.str = (char *)Push_Arena(arena, strings_len + 1);
        result.len = strings_len;
        if (result.str)
        {
            char *str_pos = result.str;
            for (u32 i = 0; i < strings.size; ++i)
            {
                for (u32 j = 0; j < strings.array[i].len; ++j)
                {
                    *str_pos = strings.array[i].str[j];
                    ++str_pos;
                }
            }
            *str_pos = 0;
        }
    }
    return result;
}

internal_function s8_array
s8_make_array(s8 *strings, u32 count)
{
    s8_array result = {};
    if (strings)
    {
        result.array = strings;
        result.size = count;
    }
    return result;
}