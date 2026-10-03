#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
	class ZBaseCamera : public CBaseEvent<ZCAMERA>
	{
	public:
	    // RTTI
		DECLARE_ROUT_CLASS(ZBaseCamera, ZCAMERA, BaseCamera, 304, 0);

		// static
		STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgRequestQuality);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgSendQuality);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgActivateCamera);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgDeactivateCamera);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgEnableCamera);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgDisableCamera);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgAddCamToSort);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgRemoveCamFromSort);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgResetCamToBestPos);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgFollowGeom);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgGetFollowGeom);
        STATIC_CLASS_VAR(ZBaseCamera, ZMessageResolver, m_msgIsFreeCam);

		// vtbl
		~ZBaseCamera() override;
		// ZSerializable
		// RTP::cBase
		const RTP::ZPropertyInfo& GetProperties() const override;
		// ZEventBase
		EEventPriority GetEventPriority() override;
		void FrameUpdate() override;
		void CopyData(const ZEventBase*) override;
		void Init() override;
		int Command(Glacier::ZMSGID command, Glacier::ZDATA data) override;
		// ZBaseConRout
		// ZBaseCamera
		virtual bool Is1stPerson();
		virtual void SetFollowGeom(Glacier::ZREF rGeom);
		virtual void SetFreeCam(bool bIsFree);
		virtual bool IsFreeCam();
		virtual void GetFocalPos(Glacier::Vector3* pvFocalPos);
		virtual void GetFocalOffset(Glacier::Vector3* pvFocalOffset);
		virtual void GetCameraPosition(Glacier::Vector3* pvCameraPosition);
		virtual float GetQuality();
		virtual void ActivateCamera();
		virtual void DeactivateCamera();
		virtual void EnableCamera(bool bIsEnabled);
		virtual void ResetCamToBestPos();

		// methods
		ZBaseCamera();

		void Initialize();
        ZGEOM* FollowGeom();
        bool FreeCam();

		// members
		bool m_bIsActive;
		bool m_bEnabled;
		bool m_bAlignment;
		bool m_KeyConOn;
		float m_fCameraRotSpeed;
		float m_fCameraMoveSpeed;
		ZREF m_rFollowGeom;
	};
	RE_VERIFY_SIZE(ZBaseCamera, 0x40); // Verified PC alloc
}
