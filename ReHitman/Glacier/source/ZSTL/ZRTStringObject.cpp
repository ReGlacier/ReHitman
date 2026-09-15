#include <Glacier/ZSTL/ZRTStringObject.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    namespace
    {
        template <typename T>
        T* NodeToObject(ZListNodeBase<T>* pNode)
        {
            return static_cast<T*>(static_cast<ZListNode<T, 0>*>(pNode));
        }

        template <typename T>
        const T* NodeToObject(const ZListNodeBase<T>* pNode)
        {
            return static_cast<const T*>(static_cast<const ZListNode<T, 0>*>(pNode));
        }

        template <typename T>
        ZListNode<T, 0>* ObjectToNode(T* pObject)
        {
            return static_cast<ZListNode<T, 0>*>(pObject);
        }
    }

    ZStringMemoryManager::ZHeader::ZRefAndSize::ZRefAndSize()
        : m_Data(0x7FFF0000)
    {
    }

    ZStringMemoryManager::ZHeader::ZRefAndSize::ZRefAndSize(bool isFree, uint32_t size)
    {
        if (isFree)
            SetFreeSize(size);
        else
            SetAllocatedSize(size);
    }

    void ZStringMemoryManager::ZHeader::ZRefAndSize::SetRefCounter(uint32_t refCounter)
    {
        ZASSERT(!IsFree());

        m_Data = (m_Data & 0x8000FFFF) | ((refCounter << 16) & 0x7FFF0000);

        ZASSERT(GetRefCounter() == refCounter);
    }

    void ZStringMemoryManager::ZHeader::ZRefAndSize::SetFreeSize(uint32_t size)
    {
        m_Data = (size & 0x7FFFFFFF) | 0x80000000;

        ZASSERT((m_Data & 0x7FFFFFFF) == size);
    }

    void ZStringMemoryManager::ZHeader::ZRefAndSize::SetAllocatedSize(uint32_t size)
    {
        m_Data = static_cast<uint16_t>(size);

        ZASSERT(static_cast<uint16_t>(m_Data) == size);
    }

    void ZStringMemoryManager::ZHeader::ZRefAndSize::Resize(uint32_t size)
    {
        ZASSERT(IsFree());

        SetFreeSize(size);
    }

    bool ZStringMemoryManager::ZHeader::ZRefAndSize::IsFree() const
    {
        return (m_Data >> 31) != 0;
    }

    bool ZStringMemoryManager::ZHeader::ZRefAndSize::IsEnd() const
    {
        return m_Data == 0x7FFF0000;
    }

    uint32_t ZStringMemoryManager::ZHeader::ZRefAndSize::GetSize() const
    {
        if (IsFree())
            return m_Data & 0x7FFFFFFF;

        return static_cast<uint16_t>(m_Data);
    }

    uint32_t ZStringMemoryManager::ZHeader::ZRefAndSize::GetRefCounter() const
    {
        ZASSERT(!IsFree());

        return (m_Data & 0x7FFF0000) >> 16;
    }

    ZStringMemoryManager::ZHeader::ZHeader()
        : m_RefOrSize()
    {
    }

    ZStringMemoryManager::ZHeader::ZHeader(bool isFree, uint32_t size)
        : m_RefOrSize(isFree, size)
    {
    }

    bool ZStringMemoryManager::ZHeader::IsFreeHeader() const
    {
        return m_RefOrSize.IsFree();
    }

    bool ZStringMemoryManager::ZHeader::IsEnd() const
    {
        return m_RefOrSize.IsEnd();
    }

    uint32_t ZStringMemoryManager::ZHeader::GetSize() const
    {
        return m_RefOrSize.GetSize();
    }

    ZStringMemoryManager::ZStringMemoryManager(uint32_t lSize)
        : m_Buffer(static_cast<char*>(ZUniMemory::Allocate(lSize + sizeof(ZHeader) + sizeof(ZFreeFooter))))
        , m_FreeList()
        , m_HashTable()
        , m_StringObjectStart(m_Buffer + sizeof(ZFreeFooter))
        , m_StringObjectEnd(static_cast<char*>(m_StringObjectStart) + lSize)
    {
        ZFreeHeader* pInitialFreeBlock = new (m_StringObjectStart) ZFreeHeader(lSize);
        m_FreeList.m_Head.AddPrev(ObjectToNode(pInitialFreeBlock));

        new (m_StringObjectEnd) ZBufferTerminator();
    }

    ZStringMemoryManager::~ZStringMemoryManager()
    {
        m_FreeList.UnlinkAll();
        for (ZList<ZRTStringObject, true, 0>& rBucket : m_HashTable)
            rBucket.UnlinkAll();

        ZUniMemory::Free(m_Buffer);
    }

    ZStringMemoryManager* ZStringMemoryManager::Instance()
    {
        static ZStringMemoryManager s_Instance(0x10000);
        return &s_Instance;
    }

    void ZStringMemoryManager::NukeIt()
    {
        ZStringMemoryManager* pInstance = Instance();
        pInstance->~ZStringMemoryManager();
        new (pInstance) ZStringMemoryManager(0x10000);
    }

    ZRTStringObject* ZStringMemoryManager::GetStringObject(const char* pStr)
    {
        if (!pStr)
            return nullptr;

        ZList<ZRTStringObject, true, 0>& rBucket = m_HashTable[GetHashCode(pStr)];
        for (auto it = rBucket.Begin(); it != rBucket.End(); ++it)
        {
            ZRTStringObject* pStringObject = it;
            if (std::strcmp(*pStringObject, pStr) == 0)
                return pStringObject;
        }

        ZRTStringObject* pStringObject = Alloc(pStr);
        if (pStringObject)
            rBucket.m_Head.AddNext(ObjectToNode(pStringObject));

        return pStringObject;
    }

    ZStringMemoryManager::ZFreeHeader* ZStringMemoryManager::GetPrevIfFree(const char* pObject) const
    {
        const ZFreeFooter* pFooter = reinterpret_cast<const ZFreeFooter*>(pObject - sizeof(ZFreeFooter));
        if (!pFooter->IsFreeFooter())
            return nullptr;

        ZFreeHeader* pHeader = pFooter->GetHeader();
        ZASSERT(pHeader->IsFreeHeader());
        return pHeader;
    }

    uint32_t ZStringMemoryManager::GetHashCode(const char* pStr) const
    {
        int16_t hash = 0;
        while (*pStr)
        {
            hash = static_cast<int16_t>(8 * hash + *pStr);
            ++pStr;
        }

        return static_cast<uint16_t>(hash) & 0x0FFF;
    }

    void ZStringMemoryManager::Free(ZRTStringObject* pObject)
    {
        ZASSERT(pObject >= m_StringObjectStart && pObject < m_StringObjectEnd);

        Check();

        uint32_t lSize = pObject->GetSize();
        ZFreeHeader* pPrevIfFree = GetPrevIfFree(reinterpret_cast<const char*>(pObject));
        ZHeader* pNextHeader = reinterpret_cast<ZHeader*>(reinterpret_cast<char*>(pObject) + lSize);

        if (pNextHeader->IsFreeHeader())
        {
            lSize += pNextHeader->GetSize();

            ZFreeHeader* pNextFree = static_cast<ZFreeHeader*>(pNextHeader);
            pNextFree->~ZFreeHeader();
        }

        if (pPrevIfFree)
        {
            pPrevIfFree->Extend(lSize);
        }
        else
        {
            ZFreeHeader* pFreeBlock = new (pObject) ZFreeHeader(lSize);
            m_FreeList.m_Head.AddPrev(ObjectToNode(pFreeBlock));
        }

        Check();
    }

    ZStringMemoryManager::ZFreeHeader* ZStringMemoryManager::FindFreeBlock(uint32_t lSize)
    {
        for (auto it = m_FreeList.Begin(); it != m_FreeList.End(); ++it)
        {
            ZFreeHeader* pFreeBlock = it;
            if (pFreeBlock->IsLargeEnough(lSize))
                return pFreeBlock;
        }

        return nullptr;
    }

    bool ZStringMemoryManager::Check()
    {
        ZHeader* pHeader = reinterpret_cast<ZHeader*>(m_Buffer + sizeof(ZFreeFooter));
        bool expectAllocated = false;

        while (!pHeader->IsEnd())
        {
            if (pHeader->IsFreeHeader())
            {
                ZASSERT(!expectAllocated);
                expectAllocated = true;
            }
            else
            {
                expectAllocated = false;
            }

            pHeader = reinterpret_cast<ZHeader*>(reinterpret_cast<char*>(pHeader) + pHeader->GetSize());
        }

        return true;
    }

    ZRTStringObject* ZStringMemoryManager::Alloc(const char* pStr)
    {
        Check();

        const uint32_t lStringSize = static_cast<uint32_t>(std::strlen(pStr) + 1);
        uint32_t lObjectSize = static_cast<uint32_t>((lStringSize + sizeof(ZRTStringObject) + 3) & ~3u);
        lObjectSize = std::max<uint32_t>(lObjectSize, 16);

        ZFreeHeader* pFreeBlock = FindFreeBlock(lObjectSize);
        if (!pFreeBlock)
            return nullptr;

        ZHeader* pAllocatedHeader = pFreeBlock->Resize(lObjectSize);
        if (!pAllocatedHeader)
        {
            lObjectSize = pFreeBlock->GetSize();
            pFreeBlock->~ZFreeHeader();
            pAllocatedHeader = pFreeBlock;
        }

        ZRTStringObject* pStringObject = new (pAllocatedHeader) ZRTStringObject(lObjectSize);
        std::memcpy(pStringObject + 1, pStr, lStringSize);
        *(reinterpret_cast<char*>(pAllocatedHeader) + lObjectSize - 1) = 0;

        Check();

        return pStringObject;
    }

    ZRTStringObject::ZRTStringObject(uint32_t lSize)
        : ZHeader(false, lSize)
        , ZListNode<ZRTStringObject, 0>()
    {
        m_RefOrSize.SetRefCounter(0);
    }

    ZRTStringObject::~ZRTStringObject()
    {
        if (m_RefOrSize.GetRefCounter() != 0)
            std::printf("String buffer leak: \"%s\"\n", c_str());
    }

    void ZRTStringObject::operator delete(void* pMemory)
    {
        ZStringMemoryManager::Instance()->Free(static_cast<ZRTStringObject*>(pMemory));
    }

    uint32_t ZRTStringObject::IncRef()
    {
        const uint32_t lRefCounter = m_RefOrSize.GetRefCounter();
        m_RefOrSize.SetRefCounter(lRefCounter + 1);
        return lRefCounter + 1;
    }

    bool ZRTStringObject::DecRef()
    {
        const uint32_t lRefCounter = m_RefOrSize.GetRefCounter();
        ZASSERT(lRefCounter > 0);

        const uint32_t lNewRefCounter = lRefCounter - 1;
        m_RefOrSize.SetRefCounter(lNewRefCounter);
        return lNewRefCounter == 0;
    }

    const char* ZRTStringObject::c_str() const
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    ZRTStringObject::operator const char*() const
    {
        return c_str();
    }

    ZRTString::ZRTString()
        : m_StringObject(nullptr)
    {
    }

    ZRTString::ZRTString(const char* pCString)
        : m_StringObject(nullptr)
    {
        Setup(ZStringMemoryManager::Instance()->GetStringObject(pCString));
    }

    ZRTString::ZRTString(const ZRTString& other)
        : m_StringObject(nullptr)
    {
        Setup(other.m_StringObject);
    }

    ZRTString::~ZRTString()
    {
        Cleanup();
    }

    void ZRTString::Setup(ZRTStringObject* pStringObject)
    {
        m_StringObject = pStringObject;
        if (m_StringObject)
            m_StringObject->IncRef();
    }

    void ZRTString::Cleanup()
    {
        if (m_StringObject && m_StringObject->DecRef())
        {
            ZRTStringObject* pStringObject = m_StringObject;
            m_StringObject = nullptr;
            pStringObject->~ZRTStringObject();
            ZRTStringObject::operator delete(pStringObject);
            return;
        }

        m_StringObject = nullptr;
    }

    const char* ZRTString::c_str() const
    {
        return m_StringObject ? m_StringObject->c_str() : nullptr;
    }

    int ZRTString::Compare(const char* pCString) const
    {
        const char* pThis = c_str();

        if (pThis)
            return pCString ? std::strcmp(pThis, pCString) : 1;

        return pCString ? -1 : 0;
    }

    ZRTString& ZRTString::operator=(const char* pCString)
    {
        Cleanup();
        Setup(ZStringMemoryManager::Instance()->GetStringObject(pCString));
        return *this;
    }

    ZRTString& ZRTString::operator=(const ZRTString& other)
    {
        if (m_StringObject != other.m_StringObject)
        {
            Cleanup();
            Setup(other.m_StringObject);
        }

        return *this;
    }

    bool ZRTString::operator==(const char* pCString) const
    {
        return Compare(pCString) == 0;
    }

    bool ZRTString::operator<(const char* pCString) const
    {
        return Compare(pCString) < 0;
    }

    char ZRTString::operator*() const
    {
        ZASSERT(m_StringObject);
        return *m_StringObject->c_str();
    }

    ZRTString::operator const char*() const
    {
        return c_str();
    }
}
