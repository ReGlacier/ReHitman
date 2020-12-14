#pragma once

#include <Glacier/ZIKLNKOBJ.h>
#include <Glacier/ZAction.h>

namespace Glacier
{
    class ZCTRLIKLNKOBJ : public ZIKLNKOBJ
    {
    public:
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
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
    };
}