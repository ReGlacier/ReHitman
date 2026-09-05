#include <Glacier/GUI/ZWINPIC.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
    ZWINPIC::ZWINPIC(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINOBJ(psName, pBaseGeom)
        , m_iOriginalSizeX(0)
        , m_iOriginalSizeY(0)
        , m_dwModifications(0)
        , m_v4UVModified {}
    {
    }

    ZWINPIC::~ZWINPIC()
    {
    }

    bool ZWINPIC::PostLoad(ISerializerStream& stream)
    {
        RemoveGeometry();
        m_dwModifications |= 1;
        return true;
    }

    void ZWINPIC::LoadSaveGeometry(ISerializerStream& stream, bool bSaving)
    {
        if (m_dwModifications & 1)
        {
            m_dwModifications ^= 1;
            uint32_t primId = 0;
            SetPicResourcePrim(primId);
        }

        if (m_dwModifications & 2)
        {
            float a[4];
            stream.Exchange("m_v4UVModified", a);
            SetUV(a[0], a[1], a[2], a[3]);
        }

        if (!bSaving)
        {
            m_dwModifications ^= 1;
            SetDrawMode(m_dwDrawMode, m_dwAmount);
            CreateSpriteArray(0, m_dwDrawMode, static_cast<SPRITETYPE>(0));
            m_dwModifications |= 1;
        }
    }

    void ZWINPIC::ConvertIndicesToPtrs()
    {
        for (uint16_t i = 0; i < m_iNumSpriteArrays; ++i)
        {
            uint32_t index = reinterpret_cast<uint32_t>(m_pSpriteArrays[i].pSprites);
            m_pSpriteArrays[i].pSpritesUV = &m_pSprites[index];
        }
    }

    void ZWINPIC::SetPicResourcePrim(const uint32_t& pPrimID)
    {
    }

    void ZWINPIC::GetPicResourcePrim(uint32_t& primID)
    {
        primID = 0;
    }

    void ZWINPIC::SetUV(float u0, float v0, float u1, float v1)
    {
        m_dwModifications |= 2;
        m_v4UVModified.x = u0;
        m_v4UVModified.y = v0;
        m_v4UVModified.z = u1;
        m_v4UVModified.w = v1;

        for (uint16_t i = 0; i < m_iNumSprites; ++i)
        {
            m_pSprites[i].u.x = u0;
            m_pSprites[i].u.y = v0;
            m_pSprites[i].v.x = u1;
            m_pSprites[i].v.y = v1;
        }
    }

    void ZWINPIC::Mirror(bool bMirror)
    {
        m_dwModifications ^= 1;

        for (uint16_t i = 0; i < m_iNumSprites; ++i)
        {
            if (bMirror)
            {
                std::swap(m_pSprites[i].v.x, m_pSprites[i].v.y);
            }
            else
            {
                std::swap(m_pSprites[i].u.x, m_pSprites[i].u.y);
            }
        }
    }

    const RTP::ZPropertyInfo& ZWINPIC::GetProperties() const
    {
        return ZWINPIC::Info;
    }

    uint32_t ZWINPIC::GetObjectId() const
    {
        return ZWINPIC::m_Id;
    }

    void ZWINPIC::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWINPIC::m_Id;
        mask = ZWINPIC::m_Mask;
    }

    ZGEOMCLASSINFO* ZWINPIC::GetOldClassInfo() const
    {
        return ZWINPIC::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<uint32_t> Modifications
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "m_dwModifications", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZWINPIC, m_dwModifications)
        };

        static RTP::ZDataProperty<float[4]> UVModified
        {
            .m_Node = { .m_Next = Modifications, .m_Name = "m_v4UVModified", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__24,
            .m_Offset = reinterpret_cast<float(*)[4]>(CLASS_PROPERTY(ZWINPIC, m_v4UVModified))
        };

        static RTP::ZDataProperty<uint32_t> OriginalSizeY
        {
            .m_Node = { .m_Next = UVModified, .m_Name = "m_iOriginalSizeY", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZWINPIC, m_iOriginalSizeY)
        };

        static RTP::ZDataProperty<uint32_t> OriginalSizeX
        {
            .m_Node = { .m_Next = OriginalSizeY, .m_Name = "m_iOriginalSizeX", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZWINPIC, m_iOriginalSizeX)
        };

        static RTP::ZVirtualProperty<uint32_t> PicResourcePrim
        {
            .m_Node = { .m_Next = OriginalSizeX, .m_Name = "PicResourcePrim", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_VP__6,
            .m_Get = &ZWINPIC::GetPicResourcePrim,
            .m_Set = &ZWINPIC::SetPicResourcePrim
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZWINPIC,
        ZWINOBJ,
        0x009A2F70,
        "ZWINPIC",
        0x007802E0,
        cProperties::PicResourcePrim,
        0x00807580,
        0x009A2F20,
        0x009A2F24
    );
#   pragma endregion
}
