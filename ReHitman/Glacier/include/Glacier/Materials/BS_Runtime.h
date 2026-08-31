#pragma once

#include <cstdint>
#include <cassert>

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Materials/ZTypedef.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier::BS_Runtime
{
    enum EPropertyType : int
    {
        PT_Int = 0x1,
        PT_Float = 0x2,
        PT_ResID_Scene = 0x4,
        PT_ResID_Audio = 0x5,
        PT_SIZE = 0x6,
    };

    struct ZStream
    {
        // methods
        const uint8_t* Get(int lOffset) const;

        // members
        const uint8_t* m_Data;
    };

    template <typename T>
    struct ZReference
    {
        // methods
        const T* GetObject(uint8_t* BaseBuffer)
        {
            return reinterpret_cast<const T*>(BaseBuffer + Offset);
        }

        const T* GetObject(const uint8_t* BaseBuffer) const
        {
            return reinterpret_cast<const T*>(BaseBuffer + Offset);
        }

        // members
        uint32_t Offset;
    };

    template <typename T>
    struct ZArray
    {
        // methods
        const T& operator[](size_t index) const
        {
            return Get(index);
        }

        uint32_t GetSize() const { return m_Size; }

        const T& Get(int index) const
        {
            ZASSERT(index < m_Size);
            return m_Data[index];
        }

        T* begin() { return &m_Data[0]; }
        const T* begin() const { return &m_Data[0]; }

        T* end() { return &m_Data[m_Size]; }
        const T* end() const { return &m_Data[m_Size]; }

        // members
        int m_Size;
        const T* m_Data;
    };

    struct ZStringDB
    {
        // methods
        const char* GetCStr(const uint8_t* pBuffer, int lStringRef);

        // members
        ZArray<ZReference<struct ZStringData>> StringTable;
    };

    struct ZString
    {
        // methods
        ZString() = default;

        ZString(const BS_Runtime::ZString& copy)
            : ID(copy.ID)
        {
        }

        const char* GetCStr(const uint8_t* baseptr, const BS_Runtime::ZStringDB* strdb) const
        {
            return reinterpret_cast<const char*>(&baseptr[strdb->StringTable.m_Data[ID].Offset]);
        }

        // members
        int ID = 0;
    };

    struct ZSerializable {};

    struct SPropertyDef : ZSerializable
    {
        // members
        ZString m_Name {};
        int m_Type = -1;
        int m_Offset = -1;
        int m_EnumIndex = -1;
    };

    struct SMaterialDef : ZSerializable
    {
        // members
        ZString m_Name;
        int m_ID;
    };

    struct SEnumValueDef : ZSerializable
    {
        // members
        ZString m_Name;
        int m_Index;
    };

    struct SEnumDef : ZSerializable
    {
        // members
        ZString m_Name;
        ZReference<ZArray<SEnumValueDef>> m_Values;
    };

    struct SRoot : ZSerializable
    {
        // members
        ZReference<ZArray<SMaterialDef>> m_MaterialDefinition;
        ZReference<ZArray<SPropertyDef>> m_PropertyDefinition;
        ZReference<ZArray<SEnumDef>> m_EnumDefinition;
        uint32_t m_MaterialSize;
        ZReference<ZStringDB> m_StringDB;
        ZReference<ZStream> m_MaterialData;
    };
    RE_VERIFY_SIZE(SRoot, 0x18);

    class ZMaterialDescriptionDB
    {
    public:
        // static
        STATIC_CLASS_VAR(ZMaterialDescriptionDB, ZMaterialDescriptionDB*, m_Instance);

        // methods
        ZMaterialDescriptionDB();
        void Init(uint8_t* bytestream);
        void RemapGeoms(uint32_t* pRemaps);
        const char* GetCStr(const ZString& str) const;
        int32_t GetPropertyId(const char* psPropName) const;
        TMaterialDescID GetMaterialDescriptionId(const char* psPropName);
        const char* GetMaterialDescriptionName(TMaterialDescID lDescId);
        TEnumID GetEnumId(const char* enumname, const char* enumvalue);
        TBoolPropertyID GetBoolPropertyId(const char* psPropName);
        TIntPropertyID GetIntPropertyId(const char* psPropName);
        TFloatPropertyID GetFloatPropertyId(const char* psPropName);
        TStringPropertyID GetStringPropertyId(const char* psPropName);
        TScenePropertyID GetScenePropertyId(const char* psPropName);
        TAudioPropertyID GetAudioPropertyId(const char* psPropName);
        ZREF GetSceneResourceProperty(TMaterialDescID lMaterialDescId, TScenePropertyID lScenePropId, TEnumID lEnumId);
        int GetAudioResourceProperty(TMaterialDescID lMaterialDescId, TAudioPropertyID lAudioPropId, TEnumID lEnumId);

        template <typename T>
        const T* GetProperty(EPropertyType eType, int32_t lMaterialId, int32_t lPropertyId)
        {
            ZASSERT(m_PropertyDefinition->Get(lPropertyId).m_Type == eType);
            if (m_PropertyDefinition->Get(lPropertyId).m_EnumIndex == -1)
                return reinterpret_cast<const T*>(GetProperty_(lMaterialId, lPropertyId));

            ZERROR("ZMaterialDescriptionDB error: Trying to access a non-map, but this is a map (propertyid:%d)", lPropertyId);
            return reinterpret_cast<const T*>(GetProperty_(lMaterialId, lPropertyId, 0));
        }

        template <typename T>
        const T* GetProperty(EPropertyType eType, int32_t lMaterialId, int32_t lPropertyId, int32_t lEnumId)
        {
            ZASSERT(m_PropertyDefinition->Get(lPropertyId).m_Type == eType);
            if (m_PropertyDefinition->Get(lPropertyId).m_EnumIndex != -1)
                return reinterpret_cast<const T*>(GetProperty_(lMaterialId, lPropertyId, lEnumId));

            ZERROR("ZMaterialDescriptionDB error: Trying to access a map, but this is not a map (propertyid:%d)", lPropertyId);
            return reinterpret_cast<const T*>(GetProperty_(lMaterialId, lPropertyId, 0));
        }

        const uint8_t* GetProperty_(int32_t lMaterialId, int32_t lPropertyId);
        const uint8_t* GetProperty_(int32_t lMaterialId, int32_t lPropertyId, int32_t lEnumId);

        static ZMaterialDescriptionDB& Instance();
        static void Create();
        static void Destroy();

        // members
        const uint8_t* m_ByteStream;
        const ZArray<SMaterialDef>* m_MaterialDefinition;
        const ZArray<SPropertyDef>* m_PropertyDefinition;
        const ZArray<SEnumDef>* m_EnumDefinition;
        int32_t  m_MaterialSize;
        const ZStringDB* m_StringDB;
        const ZStream* m_MaterialData;
    };
    RE_VERIFY_SIZE(ZMaterialDescriptionDB, 0x1C); // Verified PC alloc at ZEngineDataBase::AllocSequence
}
