#include <G1ConfigurationService.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <cassert>
#include <cmath>

namespace Glacier
{
	ZCAMERA::ZCAMERA(const char* psName, ZBaseGeom* pBaseGeom)
	    : ZGEOM(psName, pBaseGeom)
	{
	    m_pCurrentRoom = g_pSysInterface->m_pEngineData->m_pRoot;
	    m_pCurrentRoomHint = nullptr;
	    m_rCameraRoot = nullptr;
	    m_lNrAlwaysDrawGeoms = 0;
	    CameraType = 0;
	    CameraCon = 0x20u;
	    CameraListPri = 0;
	    ScrSize.x = 0.0f;
	    ScrSize.y = 0.0f;
	    Pers = 1.0f;
	    FOV = 1.1693707f;
	    FOVFirstPerson = 0.52359879f;
	    m_TargetLen = 400.0f;
	    Near = 5.0f;
	    Far = 20000.0f;
	    BackCol = 0;
	    m_fFogNear = 0.0f;
	    m_fFogFar = 0.0f;
	    m_vScale.x = 1.0f;
	    m_vScale.y = 1.0f;
	    m_vScale.z = 1.0f;
	    m_fLightMultiplier = 1.0f;
	    m_fOldLightMultiplier = 1.0f;
	    m_viewport = Glacier::ZVector4(0.0f, 0.0f, 1.0f, 1.0f);
	    m_fViewAspect = 1.0f;
	    m_bWideScreen = false;
	}

	ZCAMERA::~ZCAMERA() = default;

	int ZCAMERA::GetCameraCon() const
	{
	    return CameraCon;
	}

	bool ZCAMERA::PostLoad(ISerializerStream& stream)
	{
	    if (Near < 1.0f)
	    {
	        Near = 1.0f;
	    }

	    if (m_SubWindow)
	    {
	        g_pSysInterface->m_pEngineData->m_pRoot->SetFogColor(BackCol);
	    }

	    return true;
	}

	void ZCAMERA::LoadSave(ISerializerStream& stream, bool bSaving)
	{
	    ZGEOM::LoadSave(stream, bSaving);

	    stream.Exchange("m_lNrActiveClipPlanes", m_ClipPlanes.m_lNrActiveClipPlanes);
	    stream.ExchangeArray("m_ActiveClipPlanes", m_ClipPlanes.m_ActiveClipPlanes, 13);
	    stream.ExchangeArray("m_ClipPlanes", m_ClipPlanes.m_ClipPlanes, 48);
	}

	const RTP::ZPropertyInfo& ZCAMERA::GetProperties() const
	{
	    return ZCAMERA::Info;
	}

	uint32_t ZCAMERA::GetObjectId() const
	{
	    return ZCAMERA::m_Id;
	}

	void ZCAMERA::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
	{
	    id = ZCAMERA::m_Id;
	    mask = ZCAMERA::m_Mask;
	}

	ZGEOMCLASSINFO* ZCAMERA::GetOldClassInfo() const
	{
	    return ZCAMERA::m_OldClassInfo;
	}

	void ZCAMERA::CalcCenSize()
	{
	    if (m_lGeomControl & 1)
	    {
	        const float fExtent = std::tanf(FOV * 0.5f) * m_TargetLen;

	        float fPosX = fExtent;
	        float fPosY = fExtent;
	        if (fExtent < 30.0f)
	        {
	            fPosX = 30.0f;
	        }
	        if (fExtent < 20.0f)
	        {
	            fPosY = 20.0f;
	        }

	        const ZVector3 vPos(fPosX, fPosY, m_TargetLen);
	        const ZVector3 vNeg(-fPosX, -fPosY, -56.0f);

	        const ZVector3 vCen = (vPos + vNeg) * 0.5f;
	        const ZVector3 vSize = vPos - vCen;

	        SetCen(vCen);
	        SetSize(vSize);
	        SetRadius(vSize.Length() + 1.0f);
	    }
	    else
	    {
	        SetCen(ZVector3(0.0f, 0.0f, 0.0f));
	        SetSize(ZVector3(0.0f, 0.0f, 0.0f));
	        SetRadius(0.0f);
	    }
	}

	void ZCAMERA::CopyData(const ZGEOM* Source)
	{
	    const auto* pSource = static_cast<const ZCAMERA*>(Source);

	    ZGEOM::CopyData(pSource);

	    m_viewport = pSource->m_viewport;
	    m_fViewAspect = pSource->m_fViewAspect;
	    FOV = pSource->FOV;
	    m_TargetLen = pSource->m_TargetLen;
	    BackCol = pSource->BackCol;
	    CameraType = pSource->CameraType;
	    m_rCameraRoot = pSource->m_rCameraRoot;
	    CameraListPri = pSource->CameraListPri;
	    CameraCon = pSource->CameraCon;
	    Near = pSource->Near;
	    Far = pSource->Far;
	    m_fFogNear = pSource->m_fFogNear;
	    m_fFogFar = pSource->m_fFogFar;
	}

#	pragma region " --- RTTI --- "
	namespace cProperties
	{
	}

	DECLARE_GEOM_CLASS_IMPL(
	    ZCAMERA, // ClassName
	    ZGEOM, // BaseClass
	    0x00972808, // OldClassInfoAddr
	    "ZCAMERA", // FactoryName
	    0x00769CEC, // FactoryNameAddr
	    nullptr, // TODO: Finish me (PC 0x00807988)
	    0x0080799C, // PropertiesAddr
	    0x009727B8, // IdAddr
	    0x009727BC  // MaskAddr
	);
#	pragma endregion
}
