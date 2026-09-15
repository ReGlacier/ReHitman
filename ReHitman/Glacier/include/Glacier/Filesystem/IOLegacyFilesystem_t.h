#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class STRREFTAB;

    struct IOLegacyFilesystem_t
    {
        // vtbl
        virtual void PrintStatus() = 0;
        virtual void Add(const char* name, _FILETIME* filetime, IBuffer* pBuffer) = 0;
        virtual void Add(const char* name, _FILETIME* filetime, void* data, uint32_t datalen) = 0;
        virtual void Add(const char* Name, char* pszNameInFile) = 0;
        virtual void Save(const char* filename) = 0;
        virtual int GetSize(const char* filename) = 0;
        virtual bool Exists(const char* filename) = 0;
        virtual bool GetFileTime(const char*, _FILETIME*) = 0;
        virtual int Load(const char* filename, void* ptr, int size, int offset) = 0;
        virtual void GetDirectory(STRREFTAB* files) = 0;
        virtual void SyncClose() = 0;
        virtual void SyncOpen() = 0;
        virtual void invalidateRedundantFiles(FsZip_t& zipfs) = 0;
        virtual ~IOLegacyFilesystem_t() = default;

        // data
        int errorStatus; // +0x4
        bool statusOK; // +0x8
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(IOLegacyFilesystem_t, 0xC);
}