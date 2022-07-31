#pragma once

#include <BloodMoney/Engine/ZHM3CameraBaseFunc.h>

namespace Hitman::BloodMoney
{
	class ZHM3CameraBase : public ZHM3CameraBaseFunc
	{
	public:
		//vftable
		virtual void Update();
		virtual void UpdateCameraPosition(Glacier::Matrix3x3* pmMat, Glacier::Vector3* pvPos);
		virtual void Activate();
		virtual void Deactivate();
		virtual void UpdateInput();
		virtual void AddPoseOffset(Glacier::Vector3* pvOffset);

		//data (base size is 0x10, current size is 24)
		int m_vtbl2; //contains self vtbl
		int m_field14;
		int m_field18;
		int m_field1C;
		int m_field20;
	};

	/**
	 * Size deduction scheme:
	 * ZHM3Camera1stPerson   | 0x24
	 * ZHM3CameraBullet      | 0x38
	 * ZHM3CameraObservation | 0x64
	 * ZHM3CameraBinocular   | 0x80
	 * ZHM3CameraScope       | 0x84
	 * ZHM3CameraDialog      | 0xD4
	 * ZHM3CameraConsole     | 0x1D0
	 * ZHM3CameraPositional  | 0x1D0
	 */
}