#include <Glacier/Filesystem/ZBigFile.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        int GetChildCount(CHUNKFILE* chunk)
        {
            if (!chunk || !chunk->IsContainer())
            {
                return 0;
            }

            return chunk->type.Type2.NrChunks;
        }
    }

    ZBigFile::ZBigFile(CHUNKFILE* pFiles, const char* pName)
        : m_pFiles(pFiles)
    {
        errorStatus = 0;
        statusOK = true;
        fsAccess = IOFS_READ;

        fsName = static_cast<char*>(ZUniMemory::Allocate(strlen(pName) + 1));
        strcpy(fsName, pName);
    }

    ZBigFile::~ZBigFile()
    {
        ZUniMemory::Free(fsName);
        fsName = nullptr;
    }

    void ZBigFile::PrintStatus()
    {
        printf("ZBigFile::PrintStatus()\n");

        auto count = GetChildCount(m_pFiles);
        auto* child = m_pFiles ? m_pFiles->FirstChild() : nullptr;
        while (count-- > 0 && child)
        {
            printf("Name %s\n", static_cast<const char*>(child->Data()));
            child = child->GetNextSibling();
        }
    }

    void ZBigFile::Add(const char*, _FILETIME*, IBuffer*)
    {
        ZASSERT(false);
    }

    void ZBigFile::Add(const char*, _FILETIME*, void*, uint32_t)
    {
        ZASSERT(false);
    }

    void ZBigFile::Add(const char*, char*)
    {
        ZASSERT(false);
    }

    void ZBigFile::Save(const char*)
    {
        ZASSERT(false);
    }

    int ZBigFile::GetSize(const char* filename)
    {
        auto* file = Find(filename);
        if (!file)
        {
            return -1;
        }

        auto* child = file->FirstChild();
        return child ? child->Name : -1;
    }

    bool ZBigFile::Exists(const char* filename)
    {
        return Find(filename) != nullptr;
    }

    bool ZBigFile::GetFileTime(const char*, _FILETIME*)
    {
        ZASSERT(false);
        return false;
    }

    int ZBigFile::Load(const char* filename, void* ptr, int size, int offset)
    {
        auto* file = Find(filename);
        if (!file)
        {
            return -1;
        }

        auto* child = file->FirstChild();
        if (!child)
        {
            return -1;
        }

        ZASSERT(offset + size <= child->Name);
        memcpy(ptr, static_cast<char*>(child->Data()) + offset, size);

        return size;
    }

    void ZBigFile::GetDirectory(STRREFTAB*)
    {
        ZASSERT(false);
    }

    void ZBigFile::SyncClose()
    {
        ZASSERT(false);
    }

    void ZBigFile::SyncOpen()
    {
        ZASSERT(false);
    }

    void ZBigFile::invalidateRedundantFiles(FsZip_t&)
    {
        ZASSERT(false);
    }

    bool ZBigFile::initFS(const char*, IOFSAccess_t)
    {
        return false;
    }

    void ZBigFile::unloadFS()
    {
    }

    IOFSHandle_t* ZBigFile::open(const char*, IOFSAccess_t)
    {
        return nullptr;
    }

    int ZBigFile::read(IOFSHandle_t*, void*, unsigned int)
    {
        return 0;
    }

    int ZBigFile::write(IOFSHandle_t*, void*, unsigned int)
    {
        return 0;
    }

    void ZBigFile::close(IOFSHandle_t*)
    {
    }

    bool ZBigFile::eof(IOFSHandle_t*)
    {
        return false;
    }

    bool ZBigFile::Compare(const char*, const char*)
    {
        return false;
    }

    bool ZBigFile::InvalidateFile(const char*)
    {
        return false;
    }

    CHUNKFILE* ZBigFile::Find(const char* pFileName)
    {
        char normalizedName[512] {};
        const auto length = strlen(pFileName);
        for (size_t i = 0; i <= length && i < sizeof(normalizedName); ++i)
        {
            normalizedName[i] = pFileName[i] == '/' ? '\\' : pFileName[i];
        }

        auto count = GetChildCount(m_pFiles);
        auto* child = m_pFiles ? m_pFiles->FirstChild() : nullptr;
        while (count-- > 0 && child)
        {
            const auto* childName = static_cast<const char*>(child->Data());
            if (stricmp(normalizedName, childName) == 0)
            {
                return child;
            }

            child = child->GetNextSibling();
        }

        return nullptr;
    }
}
