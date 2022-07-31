#pragma once

#include <Glacier/ZBaseCamera.h>
#include <BloodMoney/Engine/ZHM3CameraConsole.h>


namespace Hitman::BloodMoney {
	class ZHM3Camera : public Glacier::ZBaseCamera
	{
	public:
		//vftable
		virtual bool IsActive();

		//data (base size is 0x40, current size is 0x14C)
		Glacier::Vector3 m_vPos;
		Glacier::Matrix3x3 m_mMat;
		int m_eCameraMode;
		float m_fAngleYaw;
		float m_fAnglePitch;
		float m_fAngleRoll;
		Glacier::Vector3 m_vPoseOffset;
		Glacier::Vector3 m_vCurrentLookAtPoint;
		int m_field98;
		int m_pCameraScope;
		int m_pCamera1stPerson;
		ZHM3CameraConsole* m_pCameraConsole;
		int m_pCameraObservation;
		int m_pCameraBullet;
		int m_pCameraPositional;
		int m_pCameraDialog;
		int m_pCameraBinocular;
		int m_fieldBC;
		int m_fieldC0;
		bool m_fieldC4;
		bool m_fieldC5;
		bool m_fieldC6;
		bool m_fieldC7;
		int m_fieldC8;
		int m_fieldCC;
		int m_eCameraModeStack;
		int m_fieldD4;
		int m_fieldD8;
		int m_fieldDC;
		int m_fieldE0;
		int m_fieldE4;
		int m_fieldE8;
		int m_fieldEC;
		int m_fieldF0;
		int m_fieldF4;
		int m_fieldF8;
		int m_fieldFC;
		int m_field100;
		int m_field104;
		int m_field108;
		int m_field10C;
		int m_field110;
		int m_field114;
		int m_field118;
		int m_field11C;
		int m_field120;
		int m_field124;
		int m_field128;
		int m_field12C;
		int m_field130;
		int m_field134;
		int m_field138;
		int m_MsgCameraModeExit;
		int m_field140;
		int m_field144;
		float m_field148;
	};
}