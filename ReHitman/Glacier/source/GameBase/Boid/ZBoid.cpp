#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System//ZSysInterface.h>
#include <Glacier/GameBase/Boid/ZBoid.h>
#include <Glacier/Data/ZEngineDataBase.h>


namespace Glacier
{
    ZBoid::ZBoid(int lID, ZVector3 vInitialPos)
        : m_iID(lID)
        , m_kPosition(vInitialPos)
        , m_Tracker(vInitialPos)
        , m_Mask(1)
        , m_Weight(1.0f)
        , m_TrackerDist(1.0f)
        , m_pActor(nullptr)
        , m_fSpeed(0.f)
        , m_PushRadius(70.0f)
    {
    }

    void ZBoid::FrameUpdate(const stlp::vector<ZBoid*>& aBoids, float)
    {
        // Do nothing
    }

    void ZBoid::Draw()
    {
        // Do nothing
    }

    float ZBoid::GetSpeed() const
    {
        return m_fSpeed;
    }

    void ZBoid::Move(const stlp::vector<ZBoid*>& aBoids, float)
    {
        // Do nothing
    }

    ZVector3 ZBoid::AvoidBoids(const stlp::vector<ZBoid*>& aBoids, float)
    {

        // TODO: Finish me
        return {};
    }

    ZVector3 ZBoid::HardAvoidBoids(const stlp::vector<ZBoid*>& aBoids, float)
    {
        // TODO: Finish me
        return {};
    }

    void ZBoid::MovePosition(const ZVector3& vPos)
    {
        m_kPosition = vPos;
        m_Tracker = vPos;
    }

    bool ZBoid::TeleportPosition(const ZVector3& vPos)
    {
        m_kPosition = vPos;
        m_Tracker = vPos;

        return true;
    }

    void ZBoid::SetSpeed(float fSpeed)
    {
        ZASSERT(!std::isnan(fSpeed) & isfinite(fSpeed));
        m_fSpeed = fSpeed;
    }

    void ZBoid::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        stream.ExchangeArray("m_kPosition", m_kPosition, 3);
        stream.ExchangeArray("m_Tracker", m_Tracker, 3);

        char eState = static_cast<char>(m_eState);
        stream.Exchange("m_eState", eState);
        m_eState = static_cast<EBoidState>(eState);

        stream.Exchange("m_fSpeed", m_fSpeed);
        stream.Exchange("m_TrackerDist", m_TrackerDist);
        stream.Exchange("m_Mask", m_Mask);
        stream.Exchange("m_Weight", m_Weight);
        stream.Exchange("m_PushRadius", m_PushRadius);

        if (bSaving)
        {
            m_pMetaNode->m_Location.LoadSave(stream, bSaving);
        }
        else
        {
            PF4::ZLocation sNewLoc {};
            m_pMetaNode->m_Location = sNewLoc;
            m_pMetaNode->m_Location.LoadSave(stream, false);

            g_pEngineData->m_pPathfinder4Data->RemoveNode(m_pMetaNode);
            g_pEngineData->m_pPathfinder4Data->AddNode(m_pMetaNode, sNewLoc);
        }
    }
}
