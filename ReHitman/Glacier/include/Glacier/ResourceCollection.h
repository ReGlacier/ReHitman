#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    /**
     * @brief Runtime view over one locale resource entry.
     *
     * Resource does not own the pointed data. The data pointer addresses the
     * packed locale payload flag byte; optional text, meta info, element, and
     * sound id payloads are stored immediately after that flag byte.
     */
    struct Resource
    {
        // methods
        /** @brief Creates an empty resource view. */
        Resource();
        /** @brief Returns the normalized slash-separated resource key. */
        const char* GetKey() const;
        /** @brief Normalizes a key and builds the corresponding .wav filename. */
        void SetKey(const char* key);
        /** @brief Assigns the non-owned packed payload pointer. */
        void SetData(char* psData);
        /** @brief Returns text payload when present, otherwise an empty string. */
        const char* GetText() const;
        /** @brief Returns the optional little-endian sound resource id, or zero. */
        uint32_t GetSoundResourceId() const;

        // members
        /** @brief Non-owned pointer to the packed resource payload flag byte. */
        char* m_Data{nullptr};
        /** @brief Normalized resource key without leading or duplicate slashes. */
        char m_sKey[256]{'\0'};
        /** @brief Normalized resource key with a .wav suffix. */
        char m_sFilename[256]{'\0'};
    };

    /**
     * @brief Lookup table for packed locale resources.
     *
     * The collection owns a loaded packed locale buffer and resolves slash-separated
     * paths through sorted per-folder tables. Leaf entries point to Resource payloads;
     * entries flagged with 0x10 contain element tables that can be queried by index.
     */
    class ResourceCollection
    {
    public:
        // vtbl
        /** @brief Looks up a resource text by full key. */
        virtual const char* GetResourceText(char const*); //#0000 at 00550F58
        /** @brief Looks up base/last joined by slash, falling back to last. */
        virtual const char* GetResourceText(char const*,char const*); //#0001 at 0014CD94 org ResourceCollection::GetResourceText(char const*,char const*)
        /** @brief Looks up text from an indexed element inside a folder resource. */
        virtual const char* GetElementText(char const*,int); //#0002 at 0014CE34 org ResourceCollection::GetElementText(char const*,int)

        // methods
        /** @brief Creates an empty collection with no loaded buffer. */
        ResourceCollection();

        /** @brief Resolves a full key inside a packed table and returns its payload flag byte. */
        char* Lookup(const char*, const char*);
        /** @brief Resolves an element resource payload and writes the element name pointer. */
        char* LookupElement(const char*, char**, int);
        /** @brief Resolves an element payload from an already-located element table. */
        char* LookupElementFrom(const char*, int, char**);

        /** @brief Loads a packed locale file into the owned buffer. */
        void LoadFile(const char* psFile);
        /** @brief Fills a caller-owned Resource view for the requested key. */
        Resource* GetResource(const char* psName, Resource* resource);
        /** @brief Returns whether a resource key exists in the loaded buffer. */
        bool HasResource(const char* psName);
        /** @brief Fills a caller-owned Resource view for an indexed element. */
        Resource* GetElementResource(const char*, int, Resource* resource);

        /** @brief Returns the number of elements in a folder resource, or -1 if not an element folder. */
        int GetNumElements(const char*) const;

        // members
        /** @brief Owned packed locale buffer loaded by LoadFile. */
        uint8_t* m_pBuffer;
    };
}
