#pragma once

#include <Glacier/ZSTL/CHUNKFILE.h>
#include <Glacier/Filesystem/IOFilesystem_t.h>


namespace Glacier
{
    struct ZBigFile : IOFilesystem_t
    {
        ZBigFile(CHUNKFILE* pFiles, const char* pName);
        ~ZBigFile() override;

        void PrintStatus() override;
        void Add(const char* name, _FILETIME* filetime, IBuffer* pBuffer) override;
        void Add(const char* name, _FILETIME* filetime, void* data, uint32_t datalen) override;
        void Add(const char* Name, char* pszNameInFile) override;
        void Save(const char* filename) override;
        int GetSize(const char* filename) override;
        bool Exists(const char* filename) override;
        bool GetFileTime(const char*, _FILETIME*) override;
        int Load(const char* filename, void* ptr, int size, int offset) override;
        void GetDirectory(STRREFTAB* files) override;
        void SyncClose() override;
        void SyncOpen() override;
        void invalidateRedundantFiles(FsZip_t& zipfs) override;

        bool initFS(const char*, IOFSAccess_t) override;
        void unloadFS() override;
        IOFSHandle_t* open(const char*, IOFSAccess_t) override;
        int read(IOFSHandle_t*, void*, unsigned int) override;
        int write(IOFSHandle_t*, void*, unsigned int) override;
        void close(IOFSHandle_t*) override;
        bool eof(IOFSHandle_t*) override;
        bool Compare(const char*, const char*) override;
        bool InvalidateFile(const char*) override;

        CHUNKFILE* Find(const char* pFileName);

        CHUNKFILE* m_pFiles;
    };

    RE_VERIFY_SIZE(ZBigFile, 0x18);
    RE_VERIFY_OFFSET(ZBigFile, m_pFiles, 0x14);
}
