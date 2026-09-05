#pragma once

#include <Glacier/Serializer/ZToken.h>


namespace Glacier
{
    struct IDictionary
    {
        // vtbl
        virtual ~IDictionary();
        virtual ZToken GetToken(const char* word) = 0;
    };
}
