#pragma once

#include <Glacier/Animation/ZBone.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/ZSTL/MYSTR.h>

namespace Glacier
{
    class CRigidBody;

    class ZPhysicsLinkage : public CBaseEvent<ZGEOM>
    {
    public:
        struct SRB
        {
            ZMat3x3 m_LastMat[2];
            ZVector3 m_LastPos[2];
        };

        enum EStatus : uint8_t
        {
            ePBALINKOBJ = 1u,
            ePARTICLELINK = 2u,
            eRIGIDBODYLINK = 4u,
            eCLOTHLINK = 8u,
            eCONNECTED = 0x10u,
            eUPDATE = 0x20u,
            eVALID = 0x80u,
        };

        static constexpr const char* NAME = "PhysicsLinkage";

        DECLARE_ROUT_CLASS(ZPhysicsLinkage, ZGEOM, PhysicsLinkage, 0, 0);

        ~ZPhysicsLinkage() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        bool PostLoad(ISerializerStream& stream) override;
        void FrameUpdate() override;
        int Command(ZMSGID command, ZDATA data) override;

        ZPhysicsLinkage();
        void Update(const ZBone* pBone);
        void ReleaseLinkage();

        uint32_t m_nBoneLink;
        ZMat3x3 m_Rot;
        ZVector3 m_Pos;
        SRB* m_pRB;
        uint8_t m_Status;
        RE_ADD_PADDING(3);
        uint32_t m_verify;
        const char* m_sName;
    };

    RE_VERIFY_SIZE(ZPhysicsLinkage, 0x74);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_nBoneLink, 0x30);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_Rot, 0x34);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_Pos, 0x58);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_pRB, 0x64);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_Status, 0x68);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_verify, 0x6C);
    RE_VERIFY_OFFSET(ZPhysicsLinkage, m_sName, 0x70);
}
