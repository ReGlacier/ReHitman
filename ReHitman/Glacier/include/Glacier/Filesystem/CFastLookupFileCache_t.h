#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/FileCache_t.h>
#include <Glacier/ZSTL/CFastLookup2.h>
#include <Glacier/ZSTL/REFTAB32.h>


namespace Glacier
{
    struct CFastLookupFileCache_t : public FileCache_t
    {
        // vtbl
        ~CFastLookupFileCache_t() override;
        void add(const char* psFileName, IOZip_LocalFileHeader_t* pHeader, int) override;
        bool lookup(const char* psFileName, IOZip_LocalFileHeader_t* pHeader, int*) override;
        void invalidate(const char* psFileName) override;
        void flush() override;

        // methods
        CFastLookupFileCache_t();
        
        // members
        CFastLookup2 filemap;
        REFTAB32 m_rtActiveCacheInfo;
    };
    RE_VERIFY_SIZE(CFastLookupFileCache_t, 0xB8);
}