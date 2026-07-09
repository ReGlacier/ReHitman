#pragma once

#include <Glacier/ReGlacier.h>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <cstring>


namespace Glacier
{
    /**
     * Mostly used for ANM file format
     */
    struct CHUNKFILE
    {
        // types
        struct TYPE0 
        {
        }; // Empty struct

        struct TYPE1
        {
            int32_t DataOffset;
            int32_t NrDataPieces;
            int32_t PieceSizes[1];
        };
        RE_VERIFY_SIZE(TYPE1, 0xC);

        struct TYPE2
        {
            int32_t DataOffset;
            int32_t NrChunks;
        };
        RE_VERIFY_SIZE(TYPE2, 0x8);

        struct TYPE3 
        {
            int32_t DataOffset;
            int32_t NrChunks;
            int32_t NrDataPieces;
            int32_t PieceSizes[1];
        };
        RE_VERIFY_SIZE(TYPE3, 0x10);

        // data
        int Name;
        int m_lTotalSize;
        union 
        {
            TYPE0 Type0;
            TYPE1 Type1;
            TYPE2 Type2;
            TYPE3 Type3;
        } type;

        // methods
        CHUNKFILE* FirstChild()
        {
            if (m_lTotalSize >= 0)
            {
                return nullptr;
            }

            if ((m_lTotalSize & 0x40000000) != 0)
            {
                return reinterpret_cast<CHUNKFILE*>(reinterpret_cast<char*>(this) + 0x14 + (type.Type3.NrDataPieces * sizeof(int)));
            }

            return reinterpret_cast<CHUNKFILE*>(reinterpret_cast<char*>(this) + 0x10);
        }

        CHUNKFILE* FindChild(int lName)
        {
            if (m_lTotalSize >= 0)
            {
                return nullptr;
            }

            CHUNKFILE* currentChild = nullptr;

            if ((this->m_lTotalSize & 0x40000000) != 0)
            {
                int nrDataPieces = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x10);                
                currentChild = reinterpret_cast<CHUNKFILE*>(
                    reinterpret_cast<char*>(this) + 0x14 + (nrDataPieces * sizeof(int))
                );
            }
            else
            {
                currentChild = reinterpret_cast<CHUNKFILE*>(reinterpret_cast<char*>(this) + 0x10);
            }

            if (!currentChild)
            {
                return nullptr;
            }

            int chunksCount = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x0C);
            if (chunksCount <= 0)
            {
                return nullptr;
            }

            while (currentChild->Name != lName)
            {
                int actualSize = currentChild->m_lTotalSize & 0x3FFFFFFF;

                currentChild = reinterpret_cast<CHUNKFILE*>(
                    reinterpret_cast<char*>(currentChild) + actualSize
                );

                if (--chunksCount <= 0)
                {
                    return nullptr;
                }
            }

            return currentChild;
        }

        int CountData()
        {
            int totalSize = m_lTotalSize;
            int flags = totalSize & 0xC0000000;
            int dataSize = 0;

            if (flags != 0)
            {
                int dataOffset = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x08);
                dataSize = (totalSize & 0x3FFFFFFF) - dataOffset;
            }
            else
            {
                dataSize = totalSize - 8;
            }

            if (dataSize <= 0)
            {
                return 0;
            }

            if (flags == 0x40000000)
            {
                return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x0C);
            }
            else if (flags == 0xC0000000)
            {
                return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x10);
            }

            return 1;
        }
    
        int DataSize()
        {
            unsigned int flags = m_lTotalSize & 0xC0000000;

            if (flags == 0x40000000 || flags == 0x80000000 || flags == 0xC0000000)
            {
                return (m_lTotalSize & 0x3FFFFFFF) - type.Type2.DataOffset;
            }
            else
            {
                return m_lTotalSize - 8;
            }
        }

        void* Data()
        {
            unsigned int flags = m_lTotalSize & 0xC0000000;

            if (flags == 0x40000000 || flags == 0x80000000 || flags == 0xC0000000)
            {
                return reinterpret_cast<void*>(reinterpret_cast<char*>(this) + type.Type2.DataOffset);
            }
            else
            {
                return reinterpret_cast<void*>(&type);
            }
        }

        void* GetData(int lName, int* outNrDataPieces)
        {
            CHUNKFILE* child = FirstChild();
            if (!child)
            {
                return nullptr;
            }

            int childCount = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x0C);
            if (childCount <= 0)
            {
                return nullptr;
            }

            while (child->Name != lName)
            {
                int actualSize = child->m_lTotalSize & 0x3FFFFFFF;
                child = reinterpret_cast<CHUNKFILE*>(reinterpret_cast<char*>(child) + actualSize);

                if (--childCount == 0)
                {
                    return nullptr;
                }
            }

            if (outNrDataPieces)
            {
                int count = 1;
                unsigned int childFlags = child->m_lTotalSize & 0xC0000000;

                if (childFlags == 0x40000000)
                {
                    count = child->type.Type2.NrChunks;
                }
                else if (childFlags == 0xC0000000) // -1073741824
                {
                    count = child->type.Type3.NrDataPieces;
                }

                *outNrDataPieces = count;
            }

            return child->Data();
        }

        CHUNKFILE* GetNextSibling()
        {
            int actualSize = m_lTotalSize & 0x3FFFFFFF;
            
            CHUNKFILE* nextSibling = reinterpret_cast<CHUNKFILE*>(reinterpret_cast<char*>(this) + actualSize);
            return nextSibling;
        }

        bool IsContainer() const
        {
            return m_lTotalSize < 0;
        }
    };
    RE_VERIFY_SIZE(CHUNKFILE, 0x18);

    class ChunkBuilder 
    {
    public:
        uint32_t name = 0;
        std::vector<uint8_t> rawData;
        std::vector<std::unique_ptr<ChunkBuilder>> children;

        uint32_t dataOffset = 0;
        std::vector<int32_t> pieceSizes;

        ChunkBuilder(uint32_t chunkName) : name(chunkName) {}

        void SetData(const void* data, size_t size) 
        {
            rawData.assign(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + size);
        }

        void AddDataPiece(const void* data, size_t size) 
        {
            pieceSizes.push_back(static_cast<int32_t>(size));
            const uint8_t* bytePtr = reinterpret_cast<const uint8_t*>(data);
            rawData.insert(rawData.end(), bytePtr, bytePtr + size);
        }

        ChunkBuilder& AddChild(uint32_t childName) 
        {
            children.push_back(std::make_unique<ChunkBuilder>(childName));
            return *children.back();
        }

        std::vector<uint8_t> Serialize() {
            std::vector<uint8_t> buffer;
            SerializeInternal(buffer);
            return buffer;
        }

    private:
        void SerializeInternal(std::vector<uint8_t>& buffer) {
    size_t headerStartIndex = buffer.size();

    // 1. Резервируем место под базовый заголовок Name + m_lTotalSize
    buffer.resize(headerStartIndex + 8);

    uint32_t flags = 0;

    if (!children.empty()) {
        flags |= 0x80000000; // Контейнер

        uint32_t offsetValue = 0;
        if (!pieceSizes.empty()) {
            flags |= 0x40000000; // Type3
            offsetValue = 0x14 + (pieceSizes.size() * sizeof(int32_t));
        } else {
            offsetValue = 0x10; // Type2
        }
        
        // Временный DataOffset (мы обновим его позже, когда узнаем, где реально будут данные)
        size_t offsetFieldPos = buffer.size();
        AppendUint32(buffer, offsetValue);              // +0x08: DataOffset
        AppendUint32(buffer, children.size());          // +0x0C: NrChunks

        if (!pieceSizes.empty()) {
            AppendUint32(buffer, pieceSizes.size());    // +0x10: NrDataPieces
            for (int32_t size : pieceSizes) {
                AppendUint32(buffer, size);             // +0x14...: PieceSizes[i]
            }
        }

        // КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ: Дети идут СРАЗУ за метаданными заголовка!
        for (const auto& child : children) {
            child->SerializeInternal(buffer);
        }

        // А вот теперь, ПОСЛЕ детей, кладем сырые порционные данные текущего чанка
        size_t realDataOffset = buffer.size() - headerStartIndex;
        if (!rawData.empty()) {
            buffer.insert(buffer.end(), rawData.begin(), rawData.end());
        }

        // Обновляем DataOffset правильным значением
        std::memcpy(&buffer[offsetFieldPos], &realDataOffset, 4);

    } else {
        // Логика для плоских чанков (без детей) остается прежней
        if (!pieceSizes.empty()) {
            flags |= 0xC0000000; 
            uint32_t offsetValue = 0x14 + (pieceSizes.size() * sizeof(int32_t));
            AppendUint32(buffer, offsetValue);
            AppendUint32(buffer, 0); 
            AppendUint32(buffer, pieceSizes.size());
            for (int32_t size : pieceSizes) {
                AppendUint32(buffer, size);
            }
        }
        
        if (!rawData.empty()) {
            buffer.insert(buffer.end(), rawData.begin(), rawData.end());
        }
    }

    // Записываем финальный размер и флаги
    size_t totalChunkSize = buffer.size() - headerStartIndex;
    uint32_t finalTotalSizeField = (totalChunkSize & 0x3FFFFFFF) | flags;

    std::memcpy(&buffer[headerStartIndex], &name, 4);
    std::memcpy(&buffer[headerStartIndex + 4], &finalTotalSizeField, 4);
}

        void AppendUint32(std::vector<uint8_t>& buffer, uint32_t value) 
        {
            uint8_t bytes[4];
            std::memcpy(bytes, &value, 4);
            buffer.insert(buffer.end(), bytes, bytes + 4);
        }
    };
}