#include <Glacier/Physics/Fysix/ZWaterBoxManager.h>
#include <Glacier/Physics/Fysix/ZWaterBox.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    ZWaterBox::ZWaterBox(const char* psName, ZBaseGeom* pBaseGeom)
        : ZBoxPrimitive(psName, pBaseGeom)
        , m_Geoms()
    {
    }

    ZWaterBox::~ZWaterBox()
    {
        ZWaterBoxManager::Instance().Remove(GetRef());
    }

    bool ZWaterBox::PostLoad(ISerializerStream& stream)
    {
        ZBoxPrimitive::PostLoad(stream);
        return true;
    }

    const RTP::ZPropertyInfo& ZWaterBox::GetProperties() const
    {
        return ZWaterBox::Info;
    }

    uint32_t ZWaterBox::GetObjectId() const
    {
        return ZWaterBox::m_Id;
    }

    void ZWaterBox::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWaterBox::m_Id;
        mask = ZWaterBox::m_Mask;
    }

    ZGEOMCLASSINFO* ZWaterBox::GetOldClassInfo() const
    {
        return ZWaterBox::m_OldClassInfo;
    }

    void ZWaterBox::ClassInit()
    {
        ZWaterBoxManager::Instance().Add(GetRef());
    }

    void ZWaterBox::ClassFrameUpdate()
    {
        RemoveLostGeoms();
    }

    int32_t ZWaterBox::ClassCommand(ZMSGID Msg, void* pData)
    {
        if (Msg == ZWaterBox::m_msgSetCurrent)
        {
            memcpy(m_vCurrent, pData, sizeof(m_vCurrent));
        }
        else if (Msg == ZWaterBox::m_msgRemoveGeom && pData)
        {
            Remove(*reinterpret_cast<ZREF*>(pData));
        }

        return ZGEOM::ClassCommand(Msg, pData);
    }

    void ZWaterBox::CopyData(const ZGEOM* Source)
    {
        ZBoxPrimitive::CopyData(Source);

        auto* pSource = geom_cast<ZWaterBox>(Source);
        if (!pSource)
            return;

        m_Geoms.Clear();
        if (m_Geoms.Count())
        {
            for (auto rGeom : m_Geoms.As<ZREF>())
            {
                m_Geoms.Add(rGeom);
            }
        }

        memcpy(m_vCurrent, pSource->m_vCurrent, sizeof(m_vCurrent));
    }

    void ZWaterBox::RemoveLostGeoms()
    {
        if (m_Geoms.Count())
        {
            REFTAB32 aToRemove;

            for (auto rGeom : m_Geoms.As<ZREF>())
            {
                if (!ref_cast<ZWaterBox>(rGeom))
                    aToRemove.Add(rGeom);
            }

            for (auto rGeomToRemove : aToRemove.As<ZREF>())
            {
                m_Geoms.Remove(rGeomToRemove);
            }
        }
    }

    bool ZWaterBox::Exists(ZREF rGeom) const
    {
        return m_Geoms.Exists(rGeom);
    }

    bool ZWaterBox::Add(ZREF rGeom)
    {
        if (!m_Geoms.Exists(rGeom))
        {
            SendCommand(rGeom, ZWaterBox::m_msgWaterBoxEnter, nullptr);
            m_Geoms.Add(rGeom);
            return true;
        }

        return false;
    }

    void ZWaterBox::Remove(ZREF rGeom)
    {
        SendCommand(rGeom, ZWaterBox::m_msgWaterBoxLeave, nullptr);
        m_Geoms.Remove(rGeom);
    }

#   pragma region " --- Static vars ---"
    STATIC_CLASS_VAR_IMPL(ZWaterBox, ZMessageResolver, m_msgSetCurrent, 0x0099C81C, {"SetCurrent"});
    STATIC_CLASS_VAR_IMPL(ZWaterBox, ZMessageResolver, m_msgRemoveGeom, 0x0099C828, {"RemoveGeom"});
    STATIC_CLASS_VAR_IMPL(ZWaterBox, ZMessageResolver, m_msgWaterBoxEnter, 0x0099C834, {"WaterBoxEnter"});
    STATIC_CLASS_VAR_IMPL(ZWaterBox, ZMessageResolver, m_msgWaterBoxLeave, 0x0099C840, {"WaterBoxLeave"});
#   pragma endregion

#   pragma region " --- ZGEOM RTTI --- "
    STATIC_CLASS_VAR_IMPL(ZWaterBox, const char*, FactoryName, 0x007765EC, "ZWaterBox");
    DECLARE_ID_AND_MASK_IMPL(ZWaterBox, 0x0099C7C8, 0x0099C7CC);
    REGISTER_GLACIER_GEOM_CLASS(ZWaterBox, ZSTDOBJ, ZWaterBox::m_TypeId, 0x0099C818);
#   pragma endregion

#   pragma region "RTTI"
    namespace cProperties
    {
        static RTP::ZDataProperty<float[3]> NamespaceItem_1327 {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_vCurrent",
                .m_Filter = 3
            },
            .m_VirtualTable = VirtualTable_DP__12,
            .m_Offset = CLASS_PROPERTY(ZWaterBox, m_vCurrent)
        };

        static RTP::ZDataProperty<REFTAB32> NamespaceItem_1326 {
            .m_Node = {
                .m_Next = NamespaceItem_1327,
                .m_Name = "m_Geoms",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__4,
            .m_Offset = CLASS_PROPERTY(ZWaterBox, m_Geoms)
        };
    }

    // Entry
    STATIC_CLASS_VAR_IMPL(ZWaterBox, RTP::ZPropertyInfo, Info, 0x0080B9A8, (RTP::ZPropertyInfo {
        .First = cProperties::NamespaceItem_1326,
        .Super = &ZBoxPrimitive::Info,
        .Name = ZWaterBox::FactoryName
    }));
#   pragma endregion
}
