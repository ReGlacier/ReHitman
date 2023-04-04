#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Hitman::BloodMoney
{
	class ZHM3CameraBaseFunc
	{
	public:
		//vftable
		virtual Glacier::ZGEOM* CameraGeom(); //pure virtual
		virtual void CameraCalcRotVector(Glacier::Vector3* pvVec, float, float, float);
		virtual void GetCameraRotVector(Glacier::Vector3* pvRotVec);
		virtual void LoadObject(int); //IInputSerializerStream&
		virtual void SaveObject(int); //IOutputSerializerStream&

		//data
		int m_field4;
		int m_field8;
		int m_fieldC;
	};
}