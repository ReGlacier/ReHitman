#include <Glacier/ResourceCollection.h>

#include <cstdio>
#include <cstdint>
#include <cstring>

#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        int StrNCaseCmp(const char* lhs, const char* rhs, const size_t count)
        {
#ifdef _WIN32
            return _strnicmp(lhs, rhs, count);
#else
            return strncasecmp(lhs, rhs, count);
#endif
        }

        uint32_t ReadTableOffset(const char* table, const int index)
        {
            return (static_cast<uint32_t>(static_cast<uint8_t>(table[4 * index])) << 24)
                | (static_cast<uint32_t>(static_cast<uint8_t>(table[4 * index - 1])) << 16)
                | (static_cast<uint32_t>(static_cast<uint8_t>(table[4 * index - 2])) << 8)
                | static_cast<uint32_t>(static_cast<uint8_t>(table[4 * index - 3]));
        }

        char* FindEntry(const char* path, char* table)
        {
            while (true)
            {
                while (*path == '/')
                    ++path;

                auto segmentLength = 0;

                while (path[segmentLength] != '/' && path[segmentLength] != '\0')
                    ++segmentLength;

                const auto entryCount = static_cast<uint8_t>(*table);
                const auto stringsOffset = 4 * entryCount - 3;
                auto count = static_cast<int>(entryCount);
                auto first = 0;

                while (count > 0)
                {
                    const auto middle = first + (count >> 1);
                    const auto entry = middle != 0 ? &table[ReadTableOffset(table, middle) + stringsOffset] : &table[stringsOffset];

                    if (StrNCaseCmp(entry, path, segmentLength) >= 0)
                    {
                        count >>= 1;
                    }
                    else
                    {
                        first = middle + 1;
                        count = count - (count >> 1) - 1;
                    }
                }

                const auto entryOffset = first != 0 ? static_cast<int>(ReadTableOffset(table, first)) + stringsOffset : stringsOffset;

                if (first >= entryCount || StrNCaseCmp(&table[entryOffset], path, segmentLength) != 0)
                    return nullptr;

                table += entryOffset + segmentLength + 2;

                if (path[segmentLength] == '\0')
                    return table - 1;

                path += segmentLength + 1;
            }
        }
    }

    // Resource
    Resource::Resource() = default;

    const char* Resource::GetKey() const
    {
        return &m_sKey[0];
    }

    void Resource::SetKey(const char* key)
    {
        auto keyOffset = 0;
        auto outOffset = 0;

        while (key[keyOffset] == '/')
            ++keyOffset;

        do
        {
            if (key[keyOffset] == '/')
            {
                m_sKey[outOffset] = '/';
                m_sFilename[outOffset] = '/';

                while (key[keyOffset + 1] == '/')
                    ++keyOffset;
            }
            else
            {
                m_sKey[outOffset] = key[keyOffset];
                m_sFilename[outOffset] = key[keyOffset];
            }

            ++keyOffset;
            ++outOffset;
        }
        while (key[keyOffset] != '\0');

        m_sKey[outOffset] = '\0';
        std::strcpy(&m_sFilename[outOffset], ".wav");
    }

    void Resource::SetData(char* psData)
    {
        m_Data = psData;
    }

    const char* Resource::GetText() const
    {
        static constexpr char emptyString[] = "";

        if ((*m_Data & 1) != 0)
            return m_Data + 1;

        return emptyString;
    }

    uint32_t Resource::GetSoundResourceId() const
    {
        if (m_Data == nullptr)
            return 0;

        const auto flags = *m_Data;

        if ((flags & 0x20) == 0)
            return 0;

        auto soundResourceId = m_Data + 1;

        if ((flags & 1) != 0)
            soundResourceId += std::strlen(soundResourceId) + 1;

        if ((flags & 2) != 0)
            soundResourceId += std::strlen(soundResourceId) + 1;

        return (static_cast<uint32_t>(static_cast<uint8_t>(soundResourceId[3])) << 24)
            | (static_cast<uint32_t>(static_cast<uint8_t>(soundResourceId[2])) << 16)
            | (static_cast<uint32_t>(static_cast<uint8_t>(soundResourceId[1])) << 8)
            | static_cast<uint32_t>(static_cast<uint8_t>(soundResourceId[0]));
    }

    // ResourceCollection
    ResourceCollection::ResourceCollection()
        : m_pBuffer(nullptr)
    {}

    char* ResourceCollection::LookupElementFrom(const char* data, const int elementIndex, char** elementName)
    {
        const auto elementCount = static_cast<uint8_t>(*data);

        if (elementIndex < 0 || elementIndex >= elementCount)
            return nullptr;

        auto firstElement = const_cast<char*>(&data[4 * elementCount - 3]);
        auto element = firstElement;

        if (elementIndex != 0)
            element = &firstElement[ReadTableOffset(data, elementIndex)];

        *elementName = element;

        return &element[std::strlen(*elementName) + 1];
    }

    char* ResourceCollection::LookupElement(const char* path, char** elementName, const int elementIndex)
    {
        if (m_pBuffer == nullptr)
            return nullptr;

        auto data = FindEntry(path, reinterpret_cast<char*>(m_pBuffer));

        if (data == nullptr || ((*data & 0x10) == 0))
            return nullptr;

        return LookupElementFrom(data + 1, elementIndex, elementName);
    }

    char* ResourceCollection::Lookup(const char* path, const char* data)
    {
        return FindEntry(path, const_cast<char*>(data));
    }

    void ResourceCollection::LoadFile(const char* psFile)
    {
        auto file = std::fopen(psFile, "rb");

        if (file == nullptr)
            return;

        std::fseek(file, 0, SEEK_END);
        const auto fileSize = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);

        if (fileSize <= 0)
        {
            std::fclose(file);
            return;
        }

        delete[] m_pBuffer;
        m_pBuffer = new uint8_t[fileSize];

        const auto bytesRead = std::fread(m_pBuffer, 1, fileSize, file);
        std::fclose(file);

        if (bytesRead != static_cast<size_t>(fileSize))
            std::printf("failed to load locale resource file %s\n", psFile);
    }

    bool ResourceCollection::HasResource(const char* psName)
    {
        return m_pBuffer != nullptr && Lookup(psName, reinterpret_cast<const char*>(m_pBuffer)) != nullptr;
    }

    const char* ResourceCollection::GetResourceText(const char* path, const char* last)
    {
        char buffer[208];
        auto length = std::strlen(path);

        if (length != 0)
        {
            std::strcpy(buffer, path);

            if (std::strlen(last) != 0)
            {
                buffer[length] = '/';
                ++length;
            }
        }

        if (length + std::strlen(last) >= sizeof(buffer))
        {
            std::printf("%s(%d): ZASSERT(%s)", "engine/enginedata/locale.cpp", 434, "len+strlen(last) < iBufferLen");
            std::printf("INT3 in %s at line %d", "engine/enginedata/locale.cpp", 434);
            ZASSERT(length + std::strlen(last) < sizeof(buffer));
        }

        std::strcpy(&buffer[length], last);

        const auto resourceText = GetResourceText(buffer);

        if (resourceText != nullptr)
            return resourceText;

        return last;
    }

    const char* ResourceCollection::GetResourceText(const char* psName)
    {
        Resource resource;

        if (GetResource(psName, &resource) != nullptr)
            return resource.GetText();

        return nullptr;
    }

    Resource* ResourceCollection::GetResource(const char* psName, Resource* resource)
    {
        if (m_pBuffer == nullptr)
            return nullptr;

        auto data = Lookup(psName, reinterpret_cast<const char*>(m_pBuffer));

        if (data == nullptr)
        {
            std::printf("failing to get locale resource %s\n", psName);
            return nullptr;
        }

        resource->SetKey(psName);
        resource->SetData(data);

        return resource;
    }

    int ResourceCollection::GetNumElements(const char* path) const
    {
        if (m_pBuffer == nullptr)
            return 0;

        auto data = FindEntry(path, reinterpret_cast<char*>(m_pBuffer));

        if (data == nullptr)
            return 0;

        if ((*data & 0x10) == 0)
            return -1;

        return static_cast<uint8_t>(data[1]);
    }

    const char* ResourceCollection::GetElementText(const char* path, const int elementIndex)
    {
        Resource resource;

        if (GetElementResource(path, elementIndex, &resource) != nullptr)
            return resource.GetText();

        return nullptr;
    }

    Resource* ResourceCollection::GetElementResource(const char* path, const int elementIndex, Resource* resource)
    {
        char* elementName = nullptr;

        if (m_pBuffer == nullptr)
            return nullptr;

        auto data = LookupElement(path, &elementName, elementIndex);

        if (data == nullptr)
        {
            std::printf("failing to get %d. locale resource in folder %s\n", elementIndex, path);
            return nullptr;
        }

        char key[256];
        std::strcpy(key, path);
        std::strcat(key, "/");
        std::strcat(key, elementName);

        resource->SetKey(key);
        resource->SetData(data);

        return resource;
    }
}
