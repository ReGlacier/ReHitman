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

    CHUNK::CHUNK(int name)
        : TotalSize(0)
        , HeaderSize(0)
        , Header(nullptr)
        , DataListCount(0)
        , DataList(nullptr)
        , DataFirst(nullptr)
        , DataLast(nullptr)
        , Name(name)
        , ChildFirst(nullptr)
        , ChildLast(nullptr)
        , Parent(nullptr)
        , Next(nullptr)
        , Prev(nullptr)
    {
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
        auto* chunk = ZUniMemory::New<CHUNK>(name);
        chunk->SetPosition(this, -1);
        return chunk;
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
}
