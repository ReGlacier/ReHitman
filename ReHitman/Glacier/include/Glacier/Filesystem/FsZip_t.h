#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/IOFSPhysicalAccess_t.h>
#include <Glacier/Filesystem/IOFilesystem_t.h>
#include <Glacier/Filesystem/FileCache_t.h>
#include <Glacier/Filesystem/ZIP.h>
#include <Glacier/ZSTL/STRREFTAB.h>
#include <Glacier/ZSTL/DynamicArray_t.h>
#include <cstdint>


namespace Glacier
{
    class FsZip_t : public IOFilesystem_t
    {
    public:
        // vtbl
        ~FsZip_t() override;

        // -> IOLegacyFilesystem_t:
        void PrintStatus() override;
        void Add(const char* name, _FILETIME* filetime, IBuffer* pBuffer) override;
        void Add(const char* name, _FILETIME* filetime, void* data, uint32_t datalen) override;
        void Add(const char* Name, char* pszNameInFile) override;
        void Save(const char* filename) override;
        int GetSize(const char* filename) override;
        bool Exists(const char* filename) override;
        bool GetFileTime(const char* filename, _FILETIME* pFiletime) override;
        int Load(const char* filename, void* ptr, int size, int offset) override;
        void GetDirectory(STRREFTAB* prtFiles) override;
        void SyncClose() override;
        void SyncOpen() override;
        void invalidateRedundantFiles(FsZip_t& zipfs);

        // -> IOFilesystem_t:
        bool initFS(const char* fsname, IOFSAccess_t access) override;
        void unloadFS() override;
        IOFSHandle_t* open(const char* filename, IOFSAccess_t access) override;
        int read(IOFSHandle_t* fh, void* buffer, unsigned int len) override;
        int write(IOFSHandle_t* fh, void* buffer, unsigned int len) override;
        void close(IOFSHandle_t* fh) override;
        bool eof(IOFSHandle_t* fh) override;
        bool Compare(const char* filename1, const char* filename2) override;
        bool InvalidateFile(const char* filename) override;

        // -> FsZip_t
        virtual void setCompressionLevel(CompressLevel_t eLevel);
        virtual void usePrimaryArchive();
        virtual void useSecondaryArchive();
        virtual bool AddZipFSArchive(const char* psArchive);
        virtual bool hadErrorsDuringWrite() const;

        // methods
        FsZip_t();
        FsZip_t(void* pBuffer, uint32_t cbBufferSize);

        uint8_t* extractData(IOZip_LocalFileHeader_t& header, void* outbuf, int offset, int outsize);
        uint32_t findEOCDOffset();
        bool findFile(const char* psFilename, IOZip_LocalFileHeader_t& header, uint32_t* pOffset);
        void initializeFileCache();
        void writeCDirs();
        void writeEOCD();
        void writeLocalFile(const char* psFilename, _FILETIME* pFileTime, IBuffer* pBuffer);

        // members
        ImpNativeFd_t* fp; // +0x14
        IOFSPhysicalAccess_t* m_pAccess; // +0x18
        int m_iEocdOffset; // +0x1C
        CompressLevel_t m_icurCompressLevel; // +0x20
        bool m_binvisible; // +0x24
        #pragma pack(push, 1)
        IOZip_EndOfCentralDir_t m_Eocd; // +0x25
        IOZip_EndOfCentralDir_t m_InvisibleEocd; // +0x37
        #pragma pack(pop)
        DynamicArray_t<IOZip_CentralDirStructure_t> m_cdirArray; // +0x4C
        DynamicArray_t<IOZip_CentralDirStructure_t> m_InvisibleCdirArray; // +0x58
        DynamicArray_t<long> m_InvalidOffsets; // +0x64
        FileCache_t* m_pFileCache; // +0x70
        bool m_bErrorDuringWrite; // +0x74
    };

    // HM PS2 0x80, HBM PC, MiniNinjas XBox - 0x78
    RE_VERIFY_SIZE(FsZip_t, 0x78);
    RE_VERIFY_OFFSET(FsZip_t, fp, 0x14);
    RE_VERIFY_OFFSET(FsZip_t, m_pAccess, 0x18);
    RE_VERIFY_OFFSET(FsZip_t, m_iEocdOffset, 0x1C);
    RE_VERIFY_OFFSET(FsZip_t, m_binvisible, 0x24);
    RE_VERIFY_OFFSET(FsZip_t, m_Eocd, 0x25);
    RE_VERIFY_OFFSET(FsZip_t, m_InvisibleEocd, 0x37);
    RE_VERIFY_OFFSET(FsZip_t, m_cdirArray, 0x4C);
    RE_VERIFY_OFFSET(FsZip_t, m_InvisibleCdirArray, 0x58);
    RE_VERIFY_OFFSET(FsZip_t, m_InvalidOffsets, 0x64);
    RE_VERIFY_OFFSET(FsZip_t, m_pFileCache, 0x70);
    RE_VERIFY_OFFSET(FsZip_t, m_bErrorDuringWrite, 0x74);
}
