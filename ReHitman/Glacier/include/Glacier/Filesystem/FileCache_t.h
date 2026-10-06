#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/ZIP.h>
#include <cstdint>


namespace Glacier
{
    struct FileCache_t
    {
        // types
        struct CacheInfo_t
        {
            IOZip_LocalFileHeader_t fileHeader;
            RE_ADD_PADDING(2);
            int filePos;
        };

        // vtbl
        virtual ~FileCache_t() = default;
        virtual void add(const char* psFileName, IOZip_LocalFileHeader_t* pHeader, int) = 0;
        virtual bool lookup(const char* psFileName, IOZip_LocalFileHeader_t* pHeader, int*) = 0;
        virtual void invalidate(const char* psFileName) = 0;
        virtual void flush() = 0;

        // methods
        FileCache_t() = default;
    };
}