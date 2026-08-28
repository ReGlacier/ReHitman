#pragma once

#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/ZAction.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    class ZCTRLIKLNKOBJ : public ZIKLNKOBJ
    {
    public:
        // types
        enum CONTROLLER : int
        {
            CONTROLLER_HUMAN = 0x0,
            CONTROLLER_COMPUTER = 0x1,
            CONTROLLER_FORCE32 = 0x7FFFFFFF,
        };

        //vftable
        virtual void SetController(int*);
        virtual int GetController();
        virtual void SetContactGeom(Glacier::ZREF ref);
        virtual Glacier::ZREF GetContactGeom();
        virtual bool CanOperateObject(ZAction*, ZMat3x3*, ZVector3*, bool);
        virtual void OperateObject(ZAction*);
        virtual bool IsRunning();
        virtual bool IsSneaking();
        virtual bool IsDead();
        virtual void SetLightReceived(float);
        virtual float LightReceived();
        virtual void ReducesSight(unsigned int);
        virtual void GetSeerPosDir(ZVector3* pos, ZVector3* dir);
        virtual void GetVisionPos(ZVector3* pos);

        //data (total size is 0x1A0, base size is 0x190)
        CONTROLLER m_eController;
        unsigned int m_rContactGeom;
        float m_fLightReceived;
        struct CExplodeMolotov* m_pMolotovFireController; // lmao, IOI
    };
    RE_VERIFY_SIZE(ZCTRLIKLNKOBJ, 0x1A0);
}