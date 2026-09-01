#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZAction.h>



namespace Glacier
{
    class ZCTRLIKLNKOBJ : public ZIKLNKOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCTRLIKLNKOBJ, 0x200074u);

        // types
        enum CONTROLLER : int
        {
            CONTROLLER_HUMAN = 0x0,
            CONTROLLER_COMPUTER = 0x1,
            CONTROLLER_FORCE32 = 0x7FFFFFFF,
        };

        // vtbl
        ~ZCTRLIKLNKOBJ() override;

        // ZSerializable
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZCTRLIKLNKOBJ
        virtual void SetController(CONTROLLER eController);
        virtual CONTROLLER GetController() const;
        virtual void SetContactGeom(Glacier::ZREF ref);
        virtual Glacier::ZREF GetContactGeom();
        virtual bool CanOperateObject(ZAction* pAction, ZMat3x3& mMat, ZVector3& vPos, bool bUseValues);
        virtual void OperateObject(ZAction* pAction);
        virtual bool IsRunning() const;
        virtual bool IsSneaking() const;
        virtual bool IsDead() const;
        virtual void SetLightReceived(float fLightReceived);
        virtual float LightReceived();
        virtual bool ReducedSight(uint32_t);
        virtual void GetSeerPosDir(ZVector3& vPos, ZVector3& vDir);
        virtual void GetVisionPos(ZVector3& vPos);

        // methods
        ZCTRLIKLNKOBJ(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        CONTROLLER m_eController;
        ZREF m_rContactGeom;
        float m_fLightReceived;
        struct CExplodeMolotov* m_pMolotovFireController; // lmao, IOI
    };
    RE_VERIFY_SIZE(ZCTRLIKLNKOBJ, 0x1A0); // Verified PC alloc
}
