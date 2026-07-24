#include <Glacier/Filesystem/CFastLookupFileCache_t.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    struct SInternalBlock
    {
        // members
        IOZip_LocalFileHeader_t sHeader{};
        int32_t lInFileOffset{};
    };
    RE_VERIFY_SIZE(SInternalBlock, 0x20);
    RE_VERIFY_OFFSET(SInternalBlock, sHeader, 0x0);
    RE_VERIFY_OFFSET(SInternalBlock, lInFileOffset, 0x1C);

    CFastLookupFileCache_t::CFastLookupFileCache_t()
        : FileCache_t()
        , filemap(sizeof(SInternalBlock)) // 32 in original code
        , m_rtActiveCacheInfo()
    {
    }

    CFastLookupFileCache_t::~CFastLookupFileCache_t()
    {
        flush();
    }

    void CFastLookupFileCache_t::add(const char* psFileName, IOZip_LocalFileHeader_t* pHeader, int filepos)
    {
        auto* pBlock = ZUniMemory::New<SInternalBlock>();
        pBlock->sHeader = *pHeader;
        pBlock->lInFileOffset = filepos;

        const auto rLink = reinterpret_cast<uint32_t>(pBlock);
        filemap.SetLowerCase(psFileName, rLink);
        m_rtActiveCacheInfo.Add(rLink);
    }

    bool CFastLookupFileCache_t::lookup(const char* psFileName, IOZip_LocalFileHeader_t* pHeader, int* filePos)
    {
        auto* pBlock = reinterpret_cast<SInternalBlock*>(filemap.GetLowerCase(psFileName));
        if (!pBlock)
            return false;

        *filePos = pBlock->lInFileOffset;
        memcpy(pHeader, &pBlock->sHeader, sizeof(IOZip_LocalFileHeader_t));

        return true;
    }

    void CFastLookupFileCache_t::invalidate(const char* psFileName)
    {
        filemap.RemoveLowerCase(psFileName);
    }

    void CFastLookupFileCache_t::flush()
    {
        filemap.Reset();

        for (auto entry : m_rtActiveCacheInfo)
        {
            ZUniMemory::Delete<SInternalBlock>(reinterpret_cast<SInternalBlock*>(entry));
        }

        m_rtActiveCacheInfo.Clear();
    }
}
