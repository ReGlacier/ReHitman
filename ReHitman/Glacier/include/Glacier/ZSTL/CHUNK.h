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

        // members
        CHUNK* Parent;
        CHUNKDATA* Prev;
        CHUNKDATA* Next;
        uint8_t* Data;
        int32_t Size;
    };
    RE_VERIFY_SIZE(CHUNKDATA, 0x14);

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
        // methods
        CHUNK(int Name = 0);
        ~CHUNK();

        void SetPosition(CHUNK* Parent, int No);
        void RemoveData(int No);
        CHUNK* NewChunk(int Name);
        CHUNKDATA* GetDataPtr(int No);
        void AddString(const char* String);
        void AddData(const void* Data, int Size);
        CHUNK* AddChunk(int Name);

        // members
        int32_t TotalSize;
        int32_t HeaderSize;
        uint32_t* Header;
        int32_t DataListCount;
        CHUNKDATA** DataList;
        CHUNKDATA* DataFirst;
        CHUNKDATA* DataLast;
        int Name;
        CHUNK* ChildFirst;
        CHUNK* ChildLast;
        CHUNK* Parent;
        CHUNK* Next;
        CHUNK* Prev;
    };
    RE_VERIFY_SIZE(CHUNK, 0x34);
}
