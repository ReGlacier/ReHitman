#pragma once

#include <cstdint>
#include <cassert>

#include <Glacier/ReGlacier.h>


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
        const uint8_t* m_Data;
    };

    template <typename T>
    struct ZReference
    {
        uint32_t Offset;

        T* GetObject(uint8_t* BaseBuffer)
        {
            return reinterpret_cast<T*>(BaseBuffer + Offset);
        }
    };

    template <typename T>
    struct ZArray
    {
        int m_Size;
        const T* m_Data;

        T& operator[](size_t index)
        {
            assert(index < m_Size);
            return m_Data[index];
        }
    };

    struct ZStringDB
    {
        ZArray<ZReference<struct ZStringData>> StringTable;
    };

    struct ZString
    {
        // methods
        const char* GetCStr(const uint8_t* baseptr, const BS_Runtime::ZStringDB* strdb) const
        {
            return reinterpret_cast<const char*>(&baseptr[strdb->StringTable.m_Data[ID].Offset]);
        }
        // members
        int ID;
    };

    struct ZSerializable {};

    struct SPropertyDef : ZSerializable
    {
        ZString m_Name;
        int m_Type;
        int m_Offset;
        int m_EnumIndex;
    };

    struct SMaterialDef : ZSerializable
    {
        ZString m_Name;
        int m_ID;
    };

    struct SEnumValueDef : ZSerializable
    {
        ZString m_Name;
        int m_Index;
    };

    struct SEnumDef : ZSerializable
    {
        ZString m_Name;
        ZReference<ZArray<SEnumValueDef>> m_Values;
    };

    struct SRoot : ZSerializable
    {
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
        uint8_t* m_ByteStream;
        ZArray<SMaterialDef>* m_MaterialDefinition;
        ZArray<SPropertyDef>* m_PropertyDefinition;
        ZArray<SEnumDef>* m_EnumDefinition;
        int32_t  m_MaterialSize;
        ZStringDB* m_StringDB;
        ZStream* m_MaterialData;
    };
    RE_VERIFY_SIZE(ZMaterialDescriptionDB, 0x1C);
}
