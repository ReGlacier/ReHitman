#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZEntityLocator.h> //ZBaseGeom
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZROOM;
    class ZRender;
    class ZEntityLocator;
    struct SScreenSelect;

    struct CCLIPPLANES
    {
        unsigned int m_lNrActiveClipPlanes;
        unsigned int m_ActiveClipPlanes[13];
        float m_ClipPlanes[48];
    };

    class ZCAMERA : public ZGEOM
    {
    public:
        //vftable
        virtual void SetNear(float);
        virtual void SetFar(float);
        virtual void SetBackColor(int);
        virtual void SetTargetLen(float);
        virtual void SetFOV(float);
        virtual float GetFOV();
        virtual void SetFOVFirstPerson(float);
        virtual float GetFOVFirstPerson();
        virtual void ViewUpdateBegin(); //nullstub
        virtual void ViewUpdateEnd();   //nullstub
        virtual void Init(ZRender*);
        virtual void SetViewport(const ZVector4*);
        virtual void GetViewport(ZVector4*);
        virtual void SetViewAspect(float);
        virtual float GetViewAspect();
        virtual ZGEOM* GetCameraRoot();
        virtual void ActivateCam();
        virtual void DeactivateCam();
        virtual bool IsActive();
        virtual void AddAlwaysDrawGeom(const ZEntityLocator*);
        virtual void RemoveAlwaysDrawGeom(const ZEntityLocator*);
        virtual void SetCamPrio(int); //nullstub
        virtual void SetCamTarget(Vector3*, float);
        virtual void SetCam6ClipPlanes(float,float,float,int,int,float);
        virtual void Proj2D(ZVector2*, const ZVector3*);
        virtual void Proj3D(ZVector3*, const ZVector3*);
        virtual void Proj2D3D(ZVector3*, const ZVector3*);
        virtual void SetCameraRoot(unsigned int);
        virtual void SetCameraListPrio(float);
        virtual void SetCurrentRoomHint(ZROOM*);
        virtual ZROOM* GetCurrentRoomHint();
        virtual void* GetScreenSelect(SScreenSelect*, bool, unsigned int, float);
        virtual void SetWideScreen(bool);
        virtual void FindCurrentRoom(ZROOM** room, unsigned int searchLimit);

		// API
		void SetFogEnabled(bool* bEnabled);
		bool IsFogEnabled();

        //data (total size is 0x18C, ZGEOM size is 0x10)
        ZBaseGeom* m_AlwaysDrawGeoms[2];
        int m_lNrAlwaysDrawGeoms;
        int CameraType;
        Glacier::ZVector3 m_vScale;
        int CameraCon;
        int CameraListPri;
        Glacier::ZVector2 ScrSize;
        float Pers;
        float FOV;
        float FOVFirstPerson;
        float m_TargetLen;
        float Near;
        float Far;
        unsigned int BackCol;
        unsigned int m_uBorderColor;
        float m_fFogNear;
        float m_fFogFar;
        CCLIPPLANES m_ClipPlanes;
        ZROOM* m_pCurrentRoom;
        ZBaseGeom* m_rCameraRoot;
        Glacier::ZVector4 m_viewport;
        float m_fViewAspect;
        float m_fLightMultiplier;
        float m_fOldLightMultiplier;
        ZROOM* m_pCurrentRoomHint;
        bool m_bWideScreen;
        RE_ADD_PADDING(3);
        struct ZRender* m_SubWindow;
    };
    RE_VERIFY_SIZE(ZCAMERA, 0x18C); // Verified
}