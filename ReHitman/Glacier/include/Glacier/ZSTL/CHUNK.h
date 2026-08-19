#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct CHUNK;
    struct CHUNKDATA;

    /**
     * Owns one binary payload attached to a CHUNK.
     *
     * CHUNKDATA nodes form a doubly-linked list under their parent CHUNK. Each
     * node owns a copied data buffer and unlinks itself from the parent on
     * destruction.
     */
    struct CHUNKDATA
    {
        // methods
        CHUNKDATA();
        ~CHUNKDATA();

        void ReplaceData(const void* pData, uint32_t lSize);
        void InsertData(const void* pData, uint32_t lSize, CHUNK* pParent, CHUNKDATA* pNextNode);

        // members
        CHUNK* Parent;
        CHUNKDATA* Prev;
        CHUNKDATA* Next;
        uint8_t* Data;
        int32_t Size;
    };
    RE_VERIFY_SIZE(CHUNKDATA, 0x14); // Verified PC alloc at CHUNK::InsertData

    /**
     * Mutable tree node used to build Glacier chunk hierarchies.
     *
     * A CHUNK can contain child chunks and an ordered list of CHUNKDATA payloads.
     * The class owns its header, data payloads and child chunks, maintains
     * parent/child sibling links, and caches indexed access to data nodes via
     * DataList.
     */
    struct CHUNK
    {
        // vtbl
        virtual void BaseToDerived(CHUNK* pBase);
        virtual void DerivedToBase(CHUNK* pChunk);
        virtual CHUNK* NewChunk(int lName);
        virtual void AddToMem(char** ppMem);
        virtual int BuildHeaders(bool bCompact);
        virtual ~CHUNK();
        virtual CHUNK* AddChunk(int lName);
        virtual void AddData(const void* pData, int lLen);
        virtual void AddString(const char* pszString);
        virtual void CopyData(CHUNK* pTarget, int lStartIndex);
        virtual int CountChildren();
        virtual int CountData();
        virtual CHUNK* FindChild(int lName);
        virtual int FindChild(CHUNK* pChunk);
        virtual CHUNK* GetChild(int lName);
        virtual void* GetData(int lName);
        virtual int GetDataSize(int lName);
        virtual int GetFileLen();
        virtual int GetLong(int lName);
        virtual float GetFloat(int lName);
        virtual double GetDouble(int lName);
        virtual char* GetString(int lName);
        virtual char GetChar(int lName);
        virtual void InsertData(int lName, void* pData, int lLen);
        virtual void Print(int lIndent);
        virtual void RemoveData(int lName);
        virtual void ReplaceData(int lName, const void* pData, int lLen);
        virtual void SaveToMem(void* pMem, bool bCompact);
        virtual void SetPosition(CHUNK* pChunk, int lPos);

        // methods
        CHUNK(int lName, CHUNK* pParent, int lPos);
        CHUNK(CHUNK* pSource, CHUNK* pParent);

        CHUNKDATA* GetDataPtr(int No);
        
        // members
        int32_t TotalSize;
        int32_t HeaderSize;
        uint32_t* Header;
        int32_t DataListCount;
        CHUNKDATA** DataList;
        CHUNKDATA* DataFirst;
        CHUNKDATA* DataLast;
        int Name;
        struct CHUNK* ChildFirst;
        struct CHUNK* ChildLast;
        struct CHUNK* Parent;
        struct CHUNK* Next;
        struct CHUNK* Prev;
    };
    RE_VERIFY_SIZE(CHUNK, 0x38); // Verified PC alloc at CHUNK::BaseToDerived, CHUNK::NewChunk
}
