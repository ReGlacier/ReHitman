#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZCAMERA.h>


namespace Glacier
{
	class ZBaseCamera : public CBaseEvent<ZCAMERA>
	{
	public:
		//vftable
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

		//data (base size is 0x30, current size is 0x40)
		bool m_bIsActive;
		bool m_bEnabled;
		bool m_bAlignment;
		bool m_KeyConOn;
		float m_fCameraRotSpeed;
		float m_fCameraMoveSpeed;
		ZREF m_rFollowGeom;
	};
	RE_VERIFY_SIZE(ZBaseCamera, 0x40); // Verified
}
