#include <Glacier/Geom/ZCubeGrid.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    ZCubeGrid::ZCubeGrid(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_vSize(kfDefaultCubeSize)
        , m_vCenter(0.0f)
        , m_pSubBoxes(nullptr)
        , m_lNrSubBoxes(0)
    {
        // PC 0x4E4FD0 / 0x4E52C0: the constructor leaves m_vBaseCubeSize untouched.
    }

    ZCubeGrid::~ZCubeGrid()
    {
        // PC 0x4E4CC0: release the sub-box array, then chain to the ZSTDOBJ destructor.
        if (m_pSubBoxes)
        {
            ZUniMemory::Free(m_pSubBoxes);
        }
    }

    const RTP::ZPropertyInfo& ZCubeGrid::GetProperties() const
    {
        return ZCubeGrid::Info;
    }

    uint32_t ZCubeGrid::GetObjectId() const
    {
        return ZCubeGrid::m_Id;
    }

    void ZCubeGrid::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZCubeGrid::m_Id;
        mask = ZCubeGrid::m_Mask;
    }

    ZGEOMCLASSINFO* ZCubeGrid::GetOldClassInfo() const
    {
        return ZCubeGrid::m_OldClassInfo;
    }

    void ZCubeGrid::CalcCenSize()
    {
        // PC 0x4E5030.
        ZGEOM::SetCen(m_vCenter);
        ZGEOM::SetSize(m_vSize);
        ZGEOM::SetRadius(vlen(BaseGeom()->m_vSize) + 1.0f);
    }

    bool ZCubeGrid::IsLocalPointInside(const float* pPoint) const
    {
        // PC 0x4E5350: true when the local point is contained by any sub-box.
        for (uint32_t i = 0; i < m_lNrSubBoxes; ++i)
        {
            if (CheckPointInsideSubBox(pPoint, &m_pSubBoxes[i]))
            {
                return true;
            }
        }

        return false;
    }

    void ZCubeGrid::GetThetrasFromSubBox(float* pThetras, const ZSubBox* pSubBox) const
    {
        // PC 0x4E5080: expands a sub-box into five tetrahedra (20 vertices total),
        // each vertex being one of the box's eight corners relocated by its centre.
        static const int32_t s_aiCornerIndex[20] =
        {
            0,  3,  9, 12,
            3,  6,  9, 18,
            3, 12, 15, 18,
            9, 18, 21, 12,
            3,  9, 12, 18
        };

        for (int32_t i = 0; i < 20; ++i)
        {
            vadd(&pThetras[i * 3], pSubBox->m_vCenter, &pSubBox->m_vCorners[s_aiCornerIndex[i]]);
        }
    }

    bool ZCubeGrid::CheckPointInsideThetra(const float* pPoint, const float* pThetra) const
    {
        // PC 0x4E5140. A "thetra" is a tetrahedron stored as four vertices (12 floats);
        // the point is inside when it sits on the inner side of all four faces. Each
        // face is described by a pivot vertex plus the two edges leaving it.
        static const int32_t s_aiFaceIndex[12] =
        {
            0, 3, 6,
            9, 3, 0,
            9, 6, 3,
            9, 0, 6
        };

        for (int32_t i = 0; i < 4; ++i)
        {
            const float* pPivot = &pThetra[s_aiFaceIndex[i * 3 + 0]];
            const float* pEdgeA = &pThetra[s_aiFaceIndex[i * 3 + 1]];
            const float* pEdgeB = &pThetra[s_aiFaceIndex[i * 3 + 2]];

            ZVector3 vEdgeA;
            ZVector3 vEdgeB;
            ZVector3 vNormal;
            ZVector3 vToPoint;

            vsub(vEdgeA, pEdgeA, pPivot);
            vsub(vEdgeB, pEdgeB, pPivot);
            vcross(vNormal, vEdgeB, vEdgeA);
            vsub(vToPoint, pPoint, pPivot);

            if (vdot(vNormal, vToPoint) > 0.0f)
            {
                return false;
            }
        }

        return true;
    }

    bool ZCubeGrid::CheckPointInsideSubBox(const float* pPoint, const ZSubBox* pSubBox) const
    {
        // PC 0x4E5250: the five tetrahedra partition the sub-box, so a hit on any one
        // of them means the point is inside the box.
        float aThetras[5 * 4 * 3];
        GetThetrasFromSubBox(aThetras, pSubBox);

        for (int32_t i = 0; i < 5; ++i)
        {
            if (CheckPointInsideThetra(pPoint, &aThetras[i * 4 * 3]))
            {
                return true;
            }
        }

        return false;
    }

    void ZCubeGrid::GetSubBoxes(ZRawData& data)
    {
        (void)data;

        // PS2 0x208878 / XBOX_KL2 0x823D63E0: IOI never implemented the getter, it is a
        // hard assert (serialising the sub-boxes is unsupported).
        ZASSERT(false);
    }

    void ZCubeGrid::SetSubBoxes(const ZRawData& data)
    {
        // PS2 0x208908 / XBOX_KL2 0x823D6658 (the platforms differ only in endianness;
        // this is the little-endian PC form).
        ZASSERT(!m_pSubBoxes);

        if (data.m_Size)
        {
            const uint8_t* pRaw = static_cast<const uint8_t*>(data.m_Data);

            ZSubBoxHeader header;
            memcpy(&header, pRaw, sizeof(header));

            if (header.m_lVersion)
            {
                m_lNrSubBoxes = (data.m_Size - sizeof(ZSubBoxHeader)) / sizeof(ZSubBox);
                m_vSize = header.m_vSize;
                ZASSERT(static_cast<uint32_t>(m_lNrSubBoxes * sizeof(ZSubBox) + sizeof(ZSubBoxHeader)) == data.m_Size);

                m_pSubBoxes = static_cast<ZSubBox*>(ZUniMemory::Allocate(sizeof(ZSubBox) * m_lNrSubBoxes));
                m_vBaseCubeSize = header.m_vBaseCubeSize;
                memcpy(m_pSubBoxes, pRaw + sizeof(ZSubBoxHeader), sizeof(ZSubBox) * m_lNrSubBoxes);
            }

            // Derive the grid extent from the corners of every enabled sub-box.
            ZVector3 vMin;
            ZVector3 vMax;
            bool bFirst = true;

            for (uint32_t i = 0; i < m_lNrSubBoxes; ++i)
            {
                const ZSubBox& subBox = m_pSubBoxes[i];
                if (subBox.m_lEnabled & 0x80)
                {
                    continue;
                }

                const float* pCorner = subBox.m_vCorners;
                for (int32_t j = 0; j < 8; ++j)
                {
                    ZVector3 vCorner;
                    vadd(vCorner, subBox.m_vCenter, pCorner);

                    if (bFirst)
                    {
                        bFirst = false;
                        vMin = vCorner;
                        vMax = vCorner;
                    }
                    else
                    {
                        vmin(vMin, vCorner);
                        vmax(vMax, vCorner);
                    }

                    pCorner += 3;
                }
            }

            m_vSize = vMax - vMin;
            m_vSize.x *= 0.5f;
            m_vSize.y *= 0.5f;
            m_vSize.z *= 0.5f;
            m_vCenter = vMax - m_vSize;
        }

        if (!m_pSubBoxes)
        {
            // No data: fall back to a single +/-100 cube.
            m_pSubBoxes = static_cast<ZSubBox*>(ZUniMemory::Allocate(sizeof(ZSubBox)));
            memset(m_pSubBoxes, 0, sizeof(ZSubBox));

            m_vSize = ZVector3(kfDefaultCubeSize, kfDefaultCubeSize, kfDefaultCubeSize);
            m_lNrSubBoxes = 1;
            m_pSubBoxes->m_lSubBoxNr = 1;

            const float afDefaultCorners[24] =
            {
                 100.0f,  100.0f,  100.0f,
                 100.0f, -100.0f,  100.0f,
                -100.0f, -100.0f,  100.0f,
                -100.0f,  100.0f,  100.0f,
                 100.0f,  100.0f, -100.0f,
                 100.0f, -100.0f, -100.0f,
                -100.0f, -100.0f, -100.0f,
                -100.0f,  100.0f, -100.0f
            };
            memcpy(m_pSubBoxes->m_vCorners, afDefaultCorners, sizeof(afDefaultCorners));

            m_vBaseCubeSize = ZVector3(kfDefaultCubeSize, kfDefaultCubeSize, kfDefaultCubeSize);
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PC 0x00816780 (`ZCubeGrid::Property::SubBoxes`), a ZVirtualProperty<ZRawData>
        // backed by SetSubBoxes/GetSubBoxes.
        static RTP::ZVirtualProperty<ZRawData> NamespaceItem_SubBoxes
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "SubBoxes",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__44,
            .m_Get = &ZCubeGrid::GetSubBoxes,
            .m_Set = &ZCubeGrid::SetSubBoxes
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZCubeGrid,
        ZSTDOBJ,
        0x00972860, // ZCubeGrid::m_OldClassInfo
        "ZCubeGrid",
        0x00769EFC, // ZCubeGrid::FactoryName string
        cProperties::NamespaceItem_SubBoxes,
        0x00813364, // ZCubeGrid::Info
        0x00972810, // ZCubeGrid::m_Id
        0x00972814  // ZCubeGrid::m_Mask
    );
#   pragma endregion
}
