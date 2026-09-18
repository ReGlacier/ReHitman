#include <Glacier/ZSTL/StringUtils.h>
#include <cstring>


namespace Glacier
{
    uint32_t striwcmp(const char* str, const char* wildcard)
    {
        const char* s = str;
        const char* w = wildcard;

        if (!*w) return false;

        while (1)
        {
            if (*w == '*')
            {
                const char* next_chunk = w + 1;
                const char* next_star = strchr(next_chunk, '*');
                w = next_star;

                if (!next_star)
                {
                    size_t str_len = strlen(s);
                    size_t chunk_len = strlen(next_chunk);

                    if (str_len >= chunk_len)
                    {
                        return stricmp(&s[str_len - chunk_len], next_chunk) == 0;
                    }
                    return false;
                }

                size_t chunk_len = next_star - next_chunk;
                if (next_star == next_chunk) return false;

                size_t str_len = strlen(s);
                int max_search_pos = str_len - chunk_len;
                if (max_search_pos < 0) return false;

                int checked = 0;
                while (memicmp(next_chunk, s, chunk_len) != 0)
                {
                    checked++;
                    s++;
                    if (checked == max_search_pos + 1) return false;
                }
            }
            else
            {
                const char* next_star = strchr(w, '*');

                if (!next_star)
                {
                    return stricmp(w, s) == 0;
                }

                size_t chunk_len = next_star - w;
                if (memicmp(w, s, chunk_len) != 0) return false;

                s += chunk_len;
                w = next_star;
            }

            if (!*w) return true;
        }
    }

    int strcasecmp(const char* pszLhs, const char* pszRhs)
    {
#       ifdef _WIN32
        return _stricmp(pszLhs, pszRhs);
#       else
        return StrCaseCmp(pszLhs, pszRhs);
#       endif
    }
}
