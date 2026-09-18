#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTl/STLport.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GameBase/Boid/EBoidState.h>
#include <Glacier/PF4/ZMetaNode.h>


namespace Glacier
{
    class ZBoid
    {
    public:
        // vtbl
        virtual void FrameUpdate(const stlp::vector<ZBoid*>& aBoids, float);
        virtual void Draw();
        virtual float GetSpeed() const;
        virtual void Move(const stlp::vector<ZBoid*>& aBoids, float);
        virtual ZVector3 AvoidBoids(const stlp::vector<ZBoid*>& aBoids, float);
        virtual ZVector3 HardAvoidBoids(const stlp::vector<ZBoid*>& aBoids, float);
        virtual void MovePosition(const ZVector3& vPos);
        virtual bool TeleportPosition(const ZVector3& vPos);
        virtual void SetSpeed(float fSpeed);
        virtual void LoadSave(ISerializerStream& stream, bool bSaving);

        // methods
        ZBoid(int lID, ZVector3 vInitialPos);

        // members
        EBoidState m_eState;
        float m_fSpeed;
        float m_TrackerDist;
        ZLNKWHANDS* m_pActor;
        int m_Mask;
        PF4::ZMetaNode* m_pMetaNode;
        ZVector3 m_kPosition;
        ZVector3 m_Tracker;
        float m_Weight;
        float m_PushRadius;
        int m_iID;
    };
    RE_VERIFY_SIZE(ZBoid, 0x40);
}
