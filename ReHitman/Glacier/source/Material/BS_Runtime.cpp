#include <Glacier/Materials/BS_Runtime.h>


namespace Glacier::BS_Runtime
{
    const uint8_t* ZStream::Get(int lOffset) const
    {
        return m_Data + lOffset;
    }

    const char* ZStringDB::GetCStr(const uint8_t* pBuffer, int lStringRef)
    {
        auto lRef = StringTable[lStringRef];
        return reinterpret_cast<const char*>(lRef.GetObject(pBuffer));
    }

    ZMaterialDescriptionDB::ZMaterialDescriptionDB()
    {
        m_ByteStream = nullptr;
        m_MaterialDefinition = nullptr;
        m_PropertyDefinition = nullptr;
        m_EnumDefinition = nullptr;
        m_MaterialSize = 0;
        m_StringDB = nullptr;
        m_MaterialData = nullptr;
    }

    ZMaterialDescriptionDB& ZMaterialDescriptionDB::Instance()
    {
        ZASSERT(ZMaterialDescriptionDB::m_Instance);
        return *ZMaterialDescriptionDB::m_Instance;
    }

    void ZMaterialDescriptionDB::Create()
    {
        ZMaterialDescriptionDB::m_Instance = ZUniMemory::New<ZMaterialDescriptionDB>();
    }

    void ZMaterialDescriptionDB::Destroy()
    {
        if (!ZMaterialDescriptionDB::m_Instance)
            return;

        ZUniMemory::Delete(ZMaterialDescriptionDB::m_Instance);
        ZMaterialDescriptionDB::m_Instance = nullptr;
    }

    void ZMaterialDescriptionDB::Init(uint8_t* bytestream)
    {
        auto& root = *reinterpret_cast<SRoot*>(bytestream);

        m_ByteStream = bytestream;
        m_MaterialDefinition = root.m_MaterialDefinition.GetObject(bytestream);
        m_PropertyDefinition = root.m_PropertyDefinition.GetObject(bytestream);
        m_EnumDefinition = root.m_EnumDefinition.GetObject(bytestream);
        m_MaterialSize = root.m_MaterialSize;
        m_StringDB = root.m_StringDB.GetObject(bytestream);
        m_MaterialData = root.m_MaterialData.GetObject(bytestream);
    }

    void ZMaterialDescriptionDB::RemapGeoms(uint32_t* pRemaps)
    {
        const int32_t nMaterials = m_MaterialDefinition->GetSize();
        const int32_t nProperties = m_PropertyDefinition->GetSize();

        for (int32_t i = 0; i < nProperties; ++i)
        {
            const auto& prop = m_PropertyDefinition->Get(i);
            if (prop.m_Type != static_cast<int>(EPropertyType::PT_ResID_Scene))
                continue;

            if (prop.m_EnumIndex == -1)
            {
                for (int32_t j = 0; j < nMaterials; ++j)
                {
                    const int32_t lMaterialId = m_MaterialDefinition->Get(j).m_ID;
                    auto* pValue = const_cast<int*>(GetProperty<int>(EPropertyType::PT_ResID_Scene, lMaterialId, i));
                    if (*pValue == -1)
                        *pValue = 0;
                    else
                        *pValue = pRemaps[*pValue - 1];
                }
            }
            else
            {
                const auto* pEnumValues = m_EnumDefinition->Get(prop.m_EnumIndex).m_Values.GetObject(m_ByteStream);
                const int32_t nEnumValues = pEnumValues->GetSize();

                for (int32_t k = 0; k < nMaterials; ++k)
                {
                    const int32_t lMaterialId = m_MaterialDefinition->Get(k).m_ID;
                    for (int32_t m = 0; m < nEnumValues; ++m)
                    {
                        auto* pValue = const_cast<int*>(GetProperty<int>(EPropertyType::PT_ResID_Scene, lMaterialId, i, m));
                        if (*pValue == -1)
                            *pValue = 0;
                        else
                            *pValue = pRemaps[*pValue - 1];
                    }
                }
            }
        }
    }

    const char* ZMaterialDescriptionDB::GetCStr(const ZString& str) const
    {
        return str.GetCStr(m_ByteStream, m_StringDB);
    }

    int32_t ZMaterialDescriptionDB::GetPropertyId(const char* psPropName) const
    {
        for (auto* it = m_PropertyDefinition->begin(); it != m_PropertyDefinition->end(); ++it)
        {
            if (!strcmp(GetCStr(it->m_Name), psPropName))
            {
                return it - m_PropertyDefinition->begin();
            }
        }

        ZERROR("ZMaterialDescriptionDB error: Property '%s' doesn't exist in the database! (check final/materialdescriptions/definition.xml)", psPropName);
        return -1;
    }

    TMaterialDescID ZMaterialDescriptionDB::GetMaterialDescriptionId(const char* psPropName)
    {
        for (auto* it = m_MaterialDefinition->begin(); it != m_MaterialDefinition->end(); ++it)
        {
            if (!strcmp(GetCStr(it->m_Name), psPropName))
            {
                return TMaterialDescID{ it->m_ID };
            }
        }

        ZERROR("ZMaterialDescriptionDB error: MaterialDescription '%s' doesn't exist in the database! (check final/materialdescriptions/descriptions.xml)", psPropName);
        return {};
    }

    const char* ZMaterialDescriptionDB::GetMaterialDescriptionName(TMaterialDescID lDescId)
    {
        for (auto* it = m_MaterialDefinition->begin(); it != m_MaterialDefinition->end(); ++it)
        {
            if (it->m_ID == lDescId.m_Value)
            {
                return GetCStr(it->m_Name);
            }
        }

        ZERROR("ZMaterialDescriptionDB error: MaterialDescriptionID '%d' doesn't exist in the database!", lDescId.m_Value);
        return {};
    }

    TEnumID ZMaterialDescriptionDB::GetEnumId(const char* enumname, const char* enumvalue)
    {
        for (auto* it = m_EnumDefinition->begin(); it != m_EnumDefinition->end(); ++it)
        {
            if (!strcmp(GetCStr(it->m_Name), enumname))
            {
                const auto* pEnumDefs = it->m_Values.GetObject(m_ByteStream);;
                for (auto* defIt = pEnumDefs->begin(); defIt != pEnumDefs->end(); ++defIt)
                {
                    if (!_stricmp(GetCStr(defIt->m_Name), enumvalue))
                    {
                        return TEnumID { defIt->m_Index };
                    }
                }
            }
        }

        ZERROR("ZMaterialDescriptionDB error: EnumName '%s' doesn't exist! (check final/materialdescriptions/definition.xml))", enumname);
        return {};
    }

    TBoolPropertyID ZMaterialDescriptionDB::GetBoolPropertyId(const char* psPropName)
    {
        ZASSERT(m_ByteStream);

        auto lId = GetPropertyId(psPropName);
        return { lId };
    }

    TIntPropertyID ZMaterialDescriptionDB::GetIntPropertyId(const char* psPropName)
    {
        ZASSERT(m_ByteStream);

        auto lId = GetPropertyId(psPropName);
        return { lId };
    }

    TFloatPropertyID ZMaterialDescriptionDB::GetFloatPropertyId(const char* psPropName)
    {
        ZASSERT(m_ByteStream);

        auto lId = GetPropertyId(psPropName);
        return { lId };
    }

    TStringPropertyID ZMaterialDescriptionDB::GetStringPropertyId(const char* psPropName)
    {
        ZASSERT(m_ByteStream);

        auto lId = GetPropertyId(psPropName);
        return { lId };
    }

    TScenePropertyID ZMaterialDescriptionDB::GetScenePropertyId(const char* psPropName)
    {
        ZASSERT(m_ByteStream);

        auto lId = GetPropertyId(psPropName);
        return { lId };
    }

    TAudioPropertyID ZMaterialDescriptionDB::GetAudioPropertyId(const char* psPropName)
    {
        ZASSERT(m_ByteStream);

        auto lId = GetPropertyId(psPropName);
        return { lId };
    }

    ZREF ZMaterialDescriptionDB::GetSceneResourceProperty(TMaterialDescID lMaterialDescId, TScenePropertyID lScenePropId, TEnumID lEnumId)
    {
        if (lMaterialDescId.m_Value)
        {
            return *GetProperty<ZREF>(EPropertyType::PT_ResID_Scene, lMaterialDescId.m_Value, lScenePropId.m_Value, lEnumId.m_Value);
        }

        return 0;
    }

    int ZMaterialDescriptionDB::GetAudioResourceProperty(TMaterialDescID lMaterialDescId, TAudioPropertyID lAudioPropId, TEnumID lEnumId)
    {
        if (lMaterialDescId.m_Value)
        {
            return *GetProperty<int>(EPropertyType::PT_ResID_Audio, lMaterialDescId.m_Value, lAudioPropId.m_Value, lEnumId.m_Value);
        }

        return 0;
    }

    const uint8_t* ZMaterialDescriptionDB::GetProperty_(int32_t lMaterialId, int32_t lPropertyId)
    {
        ZASSERT(lMaterialId <= m_MaterialDefinition->GetSize());
        ZASSERT(lPropertyId < m_PropertyDefinition->GetSize());

        return m_MaterialData->Get(m_PropertyDefinition->Get(lPropertyId).m_Offset + (lMaterialId - 1) * m_MaterialSize);
    }

    const uint8_t* ZMaterialDescriptionDB::GetProperty_(int32_t lMaterialId, int32_t lPropertyId, int32_t lEnumId)
    {
        ZASSERT(lMaterialId <= m_MaterialDefinition->GetSize());
        ZASSERT(lPropertyId < m_PropertyDefinition->GetSize());
        ZASSERT(m_PropertyDefinition->Get(lPropertyId).m_EnumIndex < m_EnumDefinition->GetSize());
        ZASSERT(lEnumId < m_EnumDefinition->Get(m_PropertyDefinition->Get(lPropertyId).m_EnumIndex).m_Values.GetObject(m_ByteStream)->GetSize());

        const auto* pBase = m_MaterialData->Get(m_PropertyDefinition->Get(lPropertyId).m_Offset + (lMaterialId - 1) * m_MaterialSize);
        return pBase + lEnumId * *reinterpret_cast<const int32_t*>(pBase) + 4;
    }

    STATIC_CLASS_VAR_IMPL(ZMaterialDescriptionDB, ZMaterialDescriptionDB*, m_Instance, 0x008BA0FC, nullptr);
}
