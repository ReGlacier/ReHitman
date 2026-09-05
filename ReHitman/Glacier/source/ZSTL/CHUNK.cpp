#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    // CHUNKDATA
    CHUNKDATA::CHUNKDATA()
        : Parent(nullptr)
        , Prev(nullptr)
        , Next(nullptr)
        , Data(nullptr)
        , Size(0)
    {
    }

    CHUNKDATA::~CHUNKDATA()
    {
        if (Parent)
        {
            if (Prev)
            {
                Prev->Next = Next;
            }
            else
            {
                Parent->DataFirst = Next;
            }

            if (Next)
            {
                Next->Prev = Prev;
            }
            else
            {
                Parent->DataLast = Prev;
            }

            Parent = nullptr;
            Prev = nullptr;
            Next = nullptr;
        }

        ZUniMemory::Free(Data);
        Data = nullptr;
        Size = 0;
    }

    void CHUNKDATA::ReplaceData(const void* pData, uint32_t lSize)
    {
        uint8_t* pOldData = Data;

        auto* pMem = ZUniMemory::Allocate(lSize);
        memcpy(pMem, pData, lSize);

        Data = (uint8_t*)pMem;
        Size = lSize;

        ZUniMemory::Free(pOldData);
    }

    void CHUNKDATA::InsertData(const void* pData, uint32_t lSize, CHUNK* pParent, CHUNKDATA* pNextNode)
    {
        Data = (uint8_t*)ZUniMemory::Allocate(lSize);
        memcpy(Data, pData, lSize);

        Size = lSize;
        Parent = pParent;
        if (pNextNode)
        {
            // Insert before
            Prev = pNextNode->Prev;
            Next = pNextNode;

            if (pNextNode->Prev)
            {
                pNextNode->Prev->Next = this;
            }
            else
            {
                pParent->DataFirst = this;
            }

            pNextNode->Prev = this;
        }
        else
        {
            // Insert to end
            Prev = pParent->DataLast;
            Next = nullptr;

            if (pParent->DataLast)
            {
                pParent->DataLast->Next = this;
            }
            else{
                pParent->DataFirst = this;
            }

            pParent->DataLast = this;
        }
    }
    
    CHUNK::CHUNK(int lName, CHUNK* pParent, int lPos)
    {
        Header = nullptr;
        ChildFirst = nullptr;
        ChildLast = nullptr;
        DataList = nullptr;
        DataListCount = 0;
        DataFirst = nullptr;
        DataLast = nullptr;
        Parent = nullptr;
        Prev = nullptr;
        Next = nullptr;
        Name = lName;
        SetPosition(pParent, lPos);
    }

    CHUNK::CHUNK(CHUNK* pSource, CHUNK* pParent)
    {
        Header = nullptr;
        ChildFirst = nullptr;
        ChildLast = nullptr;
        DataList = nullptr;
        DataListCount = 0;
        DataFirst = nullptr;
        DataLast = nullptr;
        Name = pSource->Name;
        Prev = nullptr;
        Next = nullptr;
        Parent = pParent;

        if (pParent)
        {
            Prev = pParent->ChildLast;
            Next = nullptr;

            if (pParent->ChildLast)
                pParent->ChildLast->Next = this;
            else
                pParent->ChildFirst = this;

            pParent->ChildLast = this;
        }
        else
        {
            Prev = nullptr;
            Next = nullptr;
        }

        for (CHUNK* pChild = pSource->ChildFirst; pChild; pChild = pChild->Next)
        {
            ZUniMemory::New<CHUNK>(pChild, this);
        }

        pSource->DerivedToBase(this);
    }

    CHUNK::~CHUNK()
    {
        ZUniMemory::Free(DataList);
        DataList = nullptr;
        DataListCount = 0;

        ZUniMemory::Free(Header);
        Header = nullptr;

        while (ChildFirst)
        {
            auto* child = ChildFirst;
            ZUniMemory::Delete(child);
        }

        RemoveData(-1);

        if (Parent)
        {
            if (Prev)
            {
                Prev->Next = Next;
            }
            else
            {
                Parent->ChildFirst = Next;
            }

            if (Next)
            {
                Next->Prev = Prev;
            }
            else
            {
                Parent->ChildLast = Prev;
            }

            Next = nullptr;
            Prev = nullptr;
            Parent = nullptr;
        }
    }

    void CHUNK::SetPosition(CHUNK* parent, int no)
    {
        if (Parent)
        {
            if (Prev)
            {
                Prev->Next = Next;
            }
            else
            {
                Parent->ChildFirst = Next;
            }

            if (Next)
            {
                Next->Prev = Prev;
            }
            else
            {
                Parent->ChildLast = Prev;
            }

            Next = nullptr;
            Prev = nullptr;
        }

        Parent = parent;
        if (!parent)
        {
            Prev = nullptr;
            Next = nullptr;
            return;
        }

        if (no != -1)
        {
            auto* child = parent->ChildFirst;
            if (no)
            {
                while (true)
                {
                    --no;
                    if (!child)
                    {
                        break;
                    }

                    child = child->Next;
                    if (!no)
                    {
                        break;
                    }
                }
            }

            if (child)
            {
                auto* prev = child->Prev;
                Next = child;
                Prev = prev;
                if (prev)
                {
                    prev->Next = this;
                }
                else
                {
                    parent->ChildFirst = this;
                }

                child->Prev = this;
                return;
            }
        }

        Next = nullptr;
        Prev = parent->ChildLast;
        if (parent->ChildLast)
        {
            parent->ChildLast->Next = this;
        }
        else
        {
            parent->ChildFirst = this;
        }

        parent->ChildLast = this;
    }

    void CHUNK::RemoveData(int no)
    {
        if (no >= 0)
        {
            auto* data = GetDataPtr(no);
            if (data)
            {
                ZUniMemory::Delete(data);
            }
        }
        else
        {
            while (DataFirst)
            {
                ZUniMemory::Delete(DataFirst);
            }
        }

        ZUniMemory::Free(DataList);
        DataList = nullptr;
        DataListCount = 0;
    }

    CHUNK* CHUNK::NewChunk(int name)
    {
        return ZUniMemory::New<CHUNK>(name, this, -1);
    }

    CHUNKDATA* CHUNK::GetDataPtr(int no)
    {
        if (no < DataListCount)
        {
            return DataList[no];
        }

        ZUniMemory::Free(DataList);
        DataList = nullptr;

        uint32_t count = 0;
        for (auto* data = DataFirst; data; data = data->Next)
        {
            ++count;
        }

        DataListCount = count;
        if (!count)
        {
            return nullptr;
        }

        DataList = static_cast<CHUNKDATA**>(ZUniMemory::Allocate(sizeof(CHUNKDATA*) * count));

        auto* data = DataFirst;
        for (uint32_t i = 0; data; ++i)
        {
            DataList[i] = data;
            data = data->Next;
        }

        return no < DataListCount ? DataList[no] : nullptr;
    }

    void CHUNK::AddString(const char* string)
    {
        AddData(string, static_cast<int>(strlen(string) + 1));
    }

    void CHUNK::AddData(const void* data, int size)
    {
        auto* chunkData = ZUniMemory::New<CHUNKDATA>();
        chunkData->Data = static_cast<uint8_t*>(ZUniMemory::Allocate(size));
        memcpy(chunkData->Data, data, size);
        chunkData->Size = size;
        chunkData->Parent = this;
        chunkData->Prev = DataLast;
        chunkData->Next = nullptr;

        if (DataLast)
        {
            DataLast->Next = chunkData;
        }
        else
        {
            DataFirst = chunkData;
        }

        DataLast = chunkData;

        ZUniMemory::Free(DataList);
        DataList = nullptr;
        DataListCount = 0;
    }

    CHUNK* CHUNK::AddChunk(int name)
    {
        return NewChunk(name);
    }

    void CHUNK::BaseToDerived(CHUNK* pBase)
    {
        if (!pBase)
            return;

        for (CHUNK* pChild = pBase->ChildFirst; pChild; pChild = pChild->Next)
        {
            ZUniMemory::New<CHUNK>(pChild, this);
        }

        const int nDataCount = pBase->CountData();
        for (int i = 0; i < nDataCount; ++i)
        {
            const int nSize = pBase->GetDataSize(i);
            void* pData = pBase->GetData(i);

            AddData(pData, nSize);
        }
    }

    void CHUNK::DerivedToBase(CHUNK* pChunk)
    {
        const int nDataCount = CountData();

        for (int i = 0; i < nDataCount; ++i)
        {
            const int nSize = GetDataSize(i);
            void* pData = GetData(i);

            pChunk->AddData(pData, nSize);
        }
    }

    void CHUNK::AddToMem(char** ppMem)
    {
        memcpy(*ppMem, Header, HeaderSize);
        *ppMem += HeaderSize;

        for (CHUNK* pChild = ChildFirst; pChild; pChild = pChild->Next)
        {
            pChild->AddToMem(ppMem);
        }

        int lDataTotalSize = 0;
        for (CHUNKDATA* pData = DataFirst; pData; pData = pData->Next)
        {
            memcpy(*ppMem, pData->Data, pData->Size);
            *ppMem += pData->Size;
            lDataTotalSize += pData->Size;
        }

        const uint32_t lTail = ((lDataTotalSize + 3) & ~3) - lDataTotalSize;
        if (lTail)
        {
            memset(*ppMem, 0, lTail);
            *ppMem += lTail;
        }
    }

    int CHUNK::BuildHeaders(bool bCompact)
    {
        int nChildCount = 0;
        TotalSize = 0;

        // Compute total size
        for (CHUNK* pChild = ChildFirst; pChild; pChild = pChild->Next)
        {
            ++nChildCount;
            const int nChildDataSize = pChild->BuildHeaders(bCompact);
            TotalSize = (TotalSize + nChildDataSize) | 0x80000000;
        }

        // Compute payload count
        int nDataCount = 0;
        const int nChildrenSize = TotalSize & 0x3FFFFFFF;

        for (CHUNKDATA* pData = DataFirst; pData; pData = pData->Next)
        {
            TotalSize += pData->Size;
            ++nDataCount;
        }

        // Align data size by 4 bytes (DWORD)
        TotalSize = (TotalSize + 3) & ~3;

        // Build flags
        if (bCompact)
        {
            if (nDataCount > 1)
                nDataCount = 1;
        }
        else if (nDataCount > 1)
        {
            TotalSize |= 0x40000000;
        }

        // Release old header
        if (Header)
        {
            ZUniMemory::Delete(Header);
        }

        const size_t nHeaderBufferSize = sizeof(uint32_t) * nDataCount + 32;
        Header = static_cast<uint32_t*>(ZUniMemory::Allocate(nHeaderBufferSize));

        // Make binary header
        Header[0] = Name;
        uint32_t* pWrite = &Header[2];

        if ((TotalSize & 0xC0000000) != 0)
        {
            pWrite++;

            if (TotalSize < 0)
            {
                *pWrite++ = nChildCount;
            }

            if ((TotalSize & 0x40000000) != 0)
            {
                *pWrite++ = nDataCount;
                for (CHUNKDATA* pData = DataFirst; pData; pData = pData->Next)
                {
                    *pWrite++ = pData->Size;
                }
            }

            const int nHeaderBytesSoFar = static_cast<int>((pWrite - Header) * sizeof(uint32_t));
            Header[2] = nChildrenSize + nHeaderBytesSoFar;
        }

        // Final compute HeaderSize & TotalSize
        const int nHeaderSize = static_cast<int>((pWrite - Header) * sizeof(uint32_t));
        HeaderSize = nHeaderSize;
        TotalSize += nHeaderSize;
        Header[1] = TotalSize;

        return TotalSize & 0x3FFFFFFF;
    }

    void CHUNK::CopyData(CHUNK* pTarget, int lStartIndex)
    {
        CHUNKDATA* pData = DataFirst;

        for (int i = 0; i < lStartIndex && pData; ++i)
        {
            pData = pData->Next;
        }

        while (pData)
        {
            pTarget->AddData(pData->Data, pData->Size);
            pData = pData->Next;
        }
    }

    int CHUNK::CountChildren()
    {
        int lCount = 0;
        auto* pChild = ChildFirst;

        while (pChild)
        {
            ++lCount;
            pChild = pChild->Next;
        }

        return lCount;
    }

    int CHUNK::CountData()
    {
        int lCount = 0;
        auto* pData = DataFirst;

        while (pData)
        {
            ++lCount;
            pData = pData->Next;
        }

        return lCount;
    }

    CHUNK* CHUNK::FindChild(int lName)
    {
        for (CHUNK* pChild = ChildFirst; pChild; pChild = pChild->Next)
        {
            if (pChild->Name == lName)
            {
                return pChild;
            }
        }

        return nullptr;
    }

    int CHUNK::FindChild(CHUNK* pChunk)
    {
        int nIndex = 0;

        for (CHUNK* pChild = ChildFirst; pChild; pChild = pChild->Next)
        {
            if (pChild == pChunk)
            {
                return nIndex;
            }
            ++nIndex;
        }

        return -1;
    }

    CHUNK* CHUNK::GetChild(int lName)
    {
        if (!lName) return nullptr;

        CHUNK* pChild = ChildFirst;
        for (int i = 1; i < lName && pChild; ++i)
        {
            pChild = pChild->Next;
        }

        return pChild;
    }

    void* CHUNK::GetData(int lName)
    {
        auto* pDataPtr = GetDataPtr(lName);
        return pDataPtr ? pDataPtr->Data : nullptr;
    }

    int CHUNK::GetDataSize(int lName)
    {
        auto* pDataPtr = GetDataPtr(lName);
        return pDataPtr ? pDataPtr->Size : 0;
    }

    int CHUNK::GetFileLen()
    {
        BuildHeaders(false);
        return static_cast<int>(Header[1] & 0x3FFFFFFFu);
    }

    int CHUNK::GetLong(int lName)
    {
        return *reinterpret_cast<int*>(GetData(lName));
    }

    float CHUNK::GetFloat(int lName)
    {
        return *reinterpret_cast<float*>(GetData(lName));
    }

    double CHUNK::GetDouble(int lName)
    {
        return *reinterpret_cast<double*>(GetData(lName));
    }

    char* CHUNK::GetString(int lName)
    {
        return reinterpret_cast<char*>(GetData(lName));
    }

    char CHUNK::GetChar(int lName)
    {
        return *reinterpret_cast<char*>(GetData(lName));
    }

    void CHUNK::InsertData(int lName, void* pData, int lLen)
    {
        if (auto* pDataPtr = GetDataPtr(lName))
        {
            if (DataList)
            {
                ZUniMemory::Free(DataList);
                DataList = nullptr;
            }

            DataListCount = 0;

            auto* pHeader = ZUniMemory::New<CHUNKDATA>();
            pHeader->InsertData(pData, lLen, this, pDataPtr);
        }
    }

    void CHUNK::Print(int lIndent)
    {
        if (lIndent == 0)
        {
            BuildHeaders(false);
        }

        char* pszIndent = (char*)ZUniMemory::Allocate(lIndent + 1);
        memset(pszIndent, ' ', lIndent);
        pszIndent[lIndent] = '\0';

        int nDataCount = CountData();
        for (int i = 0; i < nDataCount; ++i)
        {
            GetDataSize(i); 
        }

        for (CHUNK* pChild = ChildFirst; pChild; pChild = pChild->Next)
        {
            pChild->Print(lIndent + 6);
        }

        ZUniMemory::Free(pszIndent);
    }

    void CHUNK::ReplaceData(int lName, const void* pData, int lLen)
    {
        if (auto* pChunkData = GetDataPtr(lName))
        {
            pChunkData->ReplaceData(pData, lLen);
        }
    }

    void CHUNK::SaveToMem(void* pMem, bool bCompact)
    {
        BuildHeaders(bCompact);
        AddToMem((char**)(&pMem));
    }

}
