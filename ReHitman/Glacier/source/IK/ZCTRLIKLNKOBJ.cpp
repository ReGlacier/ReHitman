#include <Glacier/IK/ZCTRLIKLNKOBJ.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
    ZCTRLIKLNKOBJ::ZCTRLIKLNKOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZIKLNKOBJ(psName, pBaseGeom)
        , m_eController(CONTROLLER_COMPUTER)
        , m_rContactGeom(0)
        , m_fLightReceived(1.0f)
    {
    }

    ZCTRLIKLNKOBJ::~ZCTRLIKLNKOBJ()
    {
    }

    void ZCTRLIKLNKOBJ::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZIKLNKOBJ::LoadSave(stream, bSaving);

        int32_t lController = static_cast<int32_t>(m_eController);
        stream.Exchange("m_eController", lController);
        m_eController = static_cast<CONTROLLER>(lController);

        stream.Exchange("m_rContactGeom", m_rContactGeom);
        stream.Exchange("m_fLightReceived", m_fLightReceived);
    }

    const RTP::ZPropertyInfo& ZCTRLIKLNKOBJ::GetProperties() const
    {
        return ZCTRLIKLNKOBJ::Info;
    }

    uint32_t ZCTRLIKLNKOBJ::GetObjectId() const
    {
        return ZCTRLIKLNKOBJ::m_Id;
    }

    void ZCTRLIKLNKOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZCTRLIKLNKOBJ::m_Id;
        mask = ZCTRLIKLNKOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* ZCTRLIKLNKOBJ::GetOldClassInfo() const
    {
        return ZCTRLIKLNKOBJ::m_OldClassInfo;
    }

    void ZCTRLIKLNKOBJ::SetController(CONTROLLER eController)
    {
        m_eController = eController;
    }

    ZCTRLIKLNKOBJ::CONTROLLER ZCTRLIKLNKOBJ::GetController() const
    {
        return m_eController;
    }

    void ZCTRLIKLNKOBJ::SetContactGeom(Glacier::ZREF ref)
    {
        m_rContactGeom = ref;
    }

    Glacier::ZREF ZCTRLIKLNKOBJ::GetContactGeom()
    {
        return m_rContactGeom;
    }

    bool ZCTRLIKLNKOBJ::CanOperateObject(ZAction* pAction, ZMat3x3& mMat, ZVector3& vPos, bool bUseValues)
    {
        return false;
    }

    void ZCTRLIKLNKOBJ::OperateObject(ZAction* pAction)
    {
        // Do nothing
    }

    bool ZCTRLIKLNKOBJ::IsRunning() const
    {
        return false;
    }

    bool ZCTRLIKLNKOBJ::IsSneaking() const
    {
        return false;
    }

    bool ZCTRLIKLNKOBJ::IsDead() const
    {
        return false;
    }

    void ZCTRLIKLNKOBJ::SetLightReceived(float fLightReceived)
    {
        m_fLightReceived = fLightReceived;
    }

    float ZCTRLIKLNKOBJ::LightReceived()
    {
        return m_fLightReceived;
    }

    bool ZCTRLIKLNKOBJ::ReducedSight(uint32_t)
    {
        return false;
    }

    void ZCTRLIKLNKOBJ::GetSeerPosDir(ZVector3& vPos, ZVector3& vDir)
    {
        if ((BaseGeom()->Control() & ZCINVIEW) != 0)
        {
            ZMat3x3 mHead;

            const auto lHeadBone = HeadBoneIndex();
            if (GetIKBoneMatPos(lHeadBone, mHead, vPos))
            {
                vPos.y += 10.0f;
            }
            else
            {
                GetFocusMatPos(mHead, vPos);
                vPos.y += 30.0f;
            }

            GetRootMatPos(mHead, vPos);

            vDir.x = mHead.data[6];
            vDir.y = mHead.data[7];
            vDir.z = mHead.data[8];
        }
        else
        {
            vPos.Reset();
            GetRootPoint(vPos);

            vDir = { 0.0f, 0.0f, -1.0f };
            GetRootVect(vDir);

            vPos.y += 170.0f;
        }
    }

    void ZCTRLIKLNKOBJ::GetVisionPos(ZVector3& vPos)
    {
        if ((BaseGeom()->Control() & ZCINVIEW) != 0)
        {
            float fOffset = 0.0f;
            uint32_t lTargetBoneIdx = 0u;

            if (GetController() == CONTROLLER_COMPUTER && IsDead())
            {
                lTargetBoneIdx = PelvisBoneIndex();
                fOffset = 10.0f;
            }
            else
            {
                lTargetBoneIdx = HeadBoneIndex();
            }

            ZMat3x3 mMat;
            if (!GetIKBoneMatPos(lTargetBoneIdx, mMat, vPos))
            {
                GetFocusMatPos(mMat, vPos);
            }

            GetRootPoint(vPos);
            vPos.y += fOffset;
        }
        else
        {
            GetCen(vPos);
            GetRootPoint(vPos);
        }
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZCTRLIKLNKOBJ,
        ZIKLNKOBJ,
        0x0097BEA8,
        "ZCTRLIKLNKOBJ",
        0x007719B0,
        nullptr,
        0x00809D88,
        0x0097BE58,
        0x0097BE5C
    );
#   pragma endregion
}
