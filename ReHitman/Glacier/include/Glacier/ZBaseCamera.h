#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/EventBase/ZEventBase.h>


namespace Glacier
{
	class ZBaseCamera : public ZEventBase
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
		bool m_field32;
		bool m_bIsFreeCam;
		float m_fFreeCamMouseSensitivity;
		float m_field38;
		uint16_t m_rFollowedGeom;
		uint16_t m_unk3E;
	};
}