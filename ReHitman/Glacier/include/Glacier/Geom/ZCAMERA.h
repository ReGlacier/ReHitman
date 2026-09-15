#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/CCLIPPLANES.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZROOM;
    class ZRender;
    class ZBaseGeom;

    struct SScreenSelect
    {
        ZVector3 p0;    // +0x0 select point (world)
        uint32_t rGeom; // +0xC selected geom ref
    };
    RE_VERIFY_SIZE(SScreenSelect, 0x10);

    class ZCAMERA : public ZGEOM
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCAMERA, 0x400003u);

        // vtbl
        ~ZCAMERA() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void CopyData(const ZGEOM* Source) override;

        // ZCAMERA
        virtual void SetNear(float fNear);
        virtual void SetFar(float fFar);
        virtual void SetBackColor(int lColor);
        virtual void SetTargetLen(float fTargetLen);
        virtual void SetFOV(float fFOV);
        virtual float GetFOV();
        virtual void SetFOVFirstPerson(float fFOVFirstPerson);
        virtual float GetFOVFirstPerson();
        virtual void ViewUpdateBegin(); //nullstub
        virtual void ViewUpdateEnd();   //nullstub
        virtual void Init(ZRender* pRender);
        virtual void SetViewport(const ZVector4* pViewport);
        virtual void GetViewport(ZVector4* pViewport);
        virtual void SetViewAspect(float fViewAspect);
        virtual float GetViewAspect();
        virtual ZGEOM* GetCameraRoot();
        virtual void ActivateCam();
        virtual void DeactivateCam();
        virtual bool IsActive();
        virtual void AddAlwaysDrawGeom(const ZBaseGeom* pBaseGeom);
        virtual bool RemoveAlwaysDrawGeom(const ZBaseGeom* pBaseGeom);
        virtual void SetCamPrio(int lCamPrio); //nullstub
        virtual void SetCamTarget(Vector3* pTarget, float fTargetLen);
        virtual void SetCam6ClipPlanes(float fFOV, float fNear, float fFar, int lScreenWidth, int lScreenHeight, float fScreenAspect);
        virtual void Proj2D(ZVector2* pScreenPos, const ZVector3* pViewPos);
        virtual void Proj3D(ZVector3* pScreenPos, const ZVector3* pViewPos);
        virtual void Proj2D3D(ZVector3* pScreenPos, ZVector3* pViewDir);
        virtual void SetCameraRoot(unsigned int lRootRef);
        virtual void SetCameraListPrio(float fPrio);
        virtual void SetCurrentRoomHint(ZROOM* pRoom);
        virtual ZROOM* GetCurrentRoomHint();
        virtual bool GetScreenSelect(SScreenSelect* pScreenSelect, bool bUseColi, unsigned int lGeomControlMask, float fSelectDistance);
        virtual void SetWideScreen(bool bWideScreen);
        virtual void FindCurrentRoom(ZROOM** pRooms, unsigned int searchLimit);

		// methods
		ZCAMERA(const char* psName, ZBaseGeom* pBaseGeom);
		void SetFogEnabled(bool* bEnabled);
		bool IsFogEnabled();
		int GetCameraCon() const;

        // members
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
        ZREF m_rCameraRoot;
        Glacier::ZVector4 m_viewport;
        float m_fViewAspect;
        float m_fLightMultiplier;
        float m_fOldLightMultiplier;
        ZROOM* m_pCurrentRoomHint;
        bool m_bWideScreen;
        RE_ADD_PADDING(3);
        struct ZRender* m_SubWindow;
    };
    RE_VERIFY_SIZE(ZCAMERA, 0x18C); // Verified PC alloc
}
