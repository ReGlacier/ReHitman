#include <G1ConfigurationService.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/Render/ZRender.h>
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
	    m_rCameraRoot = 0;
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

	void ZCAMERA::SetNear(float fNear)
	{
	    if (fNear < 1.0f)
	    {
	        fNear = 1.0f;
	    }

	    if (Near != fNear)
	    {
	        Near = fNear;
	        CameraCon |= 2;
	    }
	}

	void ZCAMERA::SetFar(float fFar)
	{
	    if (Far != fFar)
	    {
	        Far = fFar;
	        CameraCon |= 2;
	    }
	}

	void ZCAMERA::SetBackColor(int lColor)
	{
	    BackCol = static_cast<unsigned int>(lColor);
	}

	void ZCAMERA::SetTargetLen(float fTargetLen)
	{
	    if (m_TargetLen != fTargetLen)
	    {
	        m_TargetLen = fTargetLen;
	        CameraCon |= 2;
	    }
	}

	void ZCAMERA::SetFOV(float fFOV)
	{
	    const float fRadians = fFOV * 0.017453292f;

	    if (FOV != fRadians)
	    {
	        FOV = fRadians;
	        CameraCon |= 2;
	    }
	}

	float ZCAMERA::GetFOV()
	{
	    return FOV * 57.29578f;
	}

	void ZCAMERA::SetFOVFirstPerson(float fFOVFirstPerson)
	{
	    if (fFOVFirstPerson > FOV)
	    {
	        ZASSERT(false);
	    }

	    const float fRadians = fFOVFirstPerson * 0.017453292f;

	    if (FOVFirstPerson != fRadians)
	    {
	        FOVFirstPerson = fRadians;
	        CameraCon |= 2;
	    }
	}

	float ZCAMERA::GetFOVFirstPerson()
	{
	    return FOVFirstPerson * 57.29578f;
	}

	void ZCAMERA::ViewUpdateBegin()
	{
	    // nullstub
	}

	void ZCAMERA::ViewUpdateEnd()
	{
	    // nullstub
	}

	void ZCAMERA::SetCamPrio(int)
	{
	    // nullstub
	}

	void ZCAMERA::Init(ZRender* pRender)
	{
	    ScrSize.x = static_cast<float>(pRender->GetSizeX());
	    ScrSize.y = static_cast<float>(pRender->GetSizeY());
	}

	void ZCAMERA::SetViewport(const ZVector4* pViewport)
	{
	    m_viewport.x = pViewport->x;
	    m_viewport.y = pViewport->y;
	    m_viewport.z = pViewport->z;
	    m_viewport.w = pViewport->w;

	    m_fViewAspect = pViewport->w / pViewport->z;
	}

	void ZCAMERA::GetViewport(ZVector4* pViewport)
	{
	    *pViewport = m_viewport;
	}

	void ZCAMERA::SetViewAspect(float fViewAspect)
	{
	    m_fViewAspect = fViewAspect;
	}

	float ZCAMERA::GetViewAspect()
	{
	    return m_fViewAspect;
	}

	ZGEOM* ZCAMERA::GetCameraRoot()
	{
	    ZROOM* pRoot = g_pSysInterface->m_pEngineData->m_pRoot;

	    if (!m_rCameraRoot)
	    {
	        return pRoot;
	    }

	    ZGEOM* pGeomRoot = ZGEOM::RefToPtr(m_rCameraRoot);
	    if (!pGeomRoot)
	    {
	        return pRoot;
	    }

	    return pGeomRoot;
	}

	void ZCAMERA::ActivateCam()
	{
	    if ((CameraCon & 0x20) == 0)
	    {
	        if (g_pSysInterface->WindowFirst)
	        {
	            g_pSysInterface->WindowFirst->ForceAllLeave();
	        }

	        CameraCon |= 0x20;
	    }
	}

	void ZCAMERA::DeactivateCam()
	{
	    if ((CameraCon & 0x20) != 0)
	    {
	        if (g_pSysInterface->WindowFirst)
	        {
	            g_pSysInterface->WindowFirst->ForceAllLeave();
	        }

	        CameraCon &= ~0x20;
	    }
	}

	bool ZCAMERA::IsActive()
	{
	    return (CameraCon & 0x20) == 0x20;
	}

	void ZCAMERA::AddAlwaysDrawGeom(const ZBaseGeom* pBaseGeom)
	{
	    for (int i = 0; i < m_lNrAlwaysDrawGeoms; ++i)
	    {
	        if (m_AlwaysDrawGeoms[i] == pBaseGeom)
	        {
	            return;
	        }
	    }

	    ZASSERT(m_lNrAlwaysDrawGeoms < 2);

	    m_AlwaysDrawGeoms[m_lNrAlwaysDrawGeoms] = const_cast<ZBaseGeom*>(pBaseGeom);
	    ++m_lNrAlwaysDrawGeoms;
	}

	bool ZCAMERA::RemoveAlwaysDrawGeom(const ZBaseGeom* pBaseGeom)
	{
	    for (int i = 0; i < m_lNrAlwaysDrawGeoms; ++i)
	    {
	        if (m_AlwaysDrawGeoms[i] == pBaseGeom)
	        {
	            --m_lNrAlwaysDrawGeoms;
	            m_AlwaysDrawGeoms[i] = m_AlwaysDrawGeoms[m_lNrAlwaysDrawGeoms];
	            return true;
	        }
	    }

	    return false;
	}

	void ZCAMERA::SetCamTarget(Vector3* pTarget, float fTargetLen)
	{
	    (void)fTargetLen;

	    ZMat3x3 mMat;
	    if (m_baseGeom->m_pParent)
	    {
	        mMat = m_baseGeom->m_pParent->m_mMat;
	    }
	    else
	    {
	        mMat.Reset();
	    }

	    // ZAxis is the normalized direction from the camera position to the target.
	    vsub(&mMat.data[0], pTarget->Get(), &m_baseGeom->m_vPos.x);

	    const float fDirLength = vlen(&mMat.data[0]);
	    if (fDirLength != 0.0f)
	    {
	        vscalar(&mMat.data[0], 1.0f / fDirLength);
	    }

	    // If the direction is (anti)parallel to the reference up axis, perturb it a bit.
	    if (fabsf(mMat.data[4] * mMat.data[1] + mMat.data[5] * mMat.data[2] + mMat.data[0] * mMat.data[3]) == 1.0f)
	    {
	        mMat.data[3] += 1.0f;
	    }

	    // XAxis = YAxis x ZAxis
	    vcross(&mMat.data[6], &mMat.data[3], &mMat.data[0]);

	    const float fRightLength = vlen(&mMat.data[6]);
	    if (fRightLength != 0.0f)
	    {
	        vscalar(&mMat.data[6], 1.0f / fRightLength);
	    }

	    // YAxis = ZAxis x XAxis
	    vcross(&mMat.data[3], &mMat.data[0], &mMat.data[6]);

	    SetMat(mMat);
	}

	void ZCAMERA::SetCam6ClipPlanes(float fFOV, float fNear, float fFar, int lScreenWidth, int lScreenHeight, float fScreenAspect)
	{
	    // PC 0x7FBE38 (g_fFirstPersonFOV in degrees) converted to radians.
	    const float fFirstPersonFOV = 67.5f * 0.017453292f;
	    if (FOVFirstPerson != fFirstPersonFOV)
	    {
	        FOVFirstPerson = fFirstPersonFOV;
	    }

	    const float fAspect = m_fViewAspect;

	    CameraCon &= ~2;
	    ScrSize.x = static_cast<float>(lScreenWidth);
	    ScrSize.y = static_cast<float>(lScreenHeight);

	    const float fRatio = static_cast<float>(lScreenHeight) * fScreenAspect / static_cast<float>(lScreenWidth);

	    float aPlanes[6][4];
	    ZRender* pWindow = g_pSysInterface->WindowFirst;
	    const bool bWideScreen = pWindow && pWindow->m_bWideScreen;

	    if (CameraCon & 1)
	    {
	        FOV = fFOV;

	        if (m_vScale.x == 1.0f)
	        {
	            m_vScale.x = 1.0f / (tanf(fFOV * 0.5f) * m_TargetLen * 4.0f);
	        }

	        m_vScale.y = m_vScale.x / fRatio;
	        Pers = 0.00012207031f;

	        const float fHalfX = 0.5f / m_vScale.x;
	        const float fHalfY = 0.5f / m_vScale.y;

	        aPlanes[0][0] = 0.0f;  aPlanes[0][1] = 0.0f; aPlanes[0][2] = 1.0f;   aPlanes[0][3] = -fNear;
	        aPlanes[1][0] = 0.0f;  aPlanes[1][1] = 0.0f; aPlanes[1][2] = -1.0f;  aPlanes[1][3] = fFar;
	        aPlanes[2][0] = -1.0f; aPlanes[2][1] = 0.0f; aPlanes[2][2] = 0.0f;   aPlanes[2][3] = fHalfX;
	        aPlanes[3][0] = 1.0f;  aPlanes[3][1] = 0.0f; aPlanes[3][2] = 0.0f;   aPlanes[3][3] = fHalfX;
	        aPlanes[4][0] = 0.0f;  aPlanes[4][1] = -1.0f; aPlanes[4][2] = 0.0f;  aPlanes[4][3] = fHalfY;
	        aPlanes[5][0] = 0.0f;  aPlanes[5][1] = 1.0f; aPlanes[5][2] = 0.0f;   aPlanes[5][3] = fHalfY;
	    }
	    else
	    {
	        m_vScale.x = 1.0f;
	        FOV = fFOV;
	        m_vScale.y = 1.0f / fRatio;

	        Pers = fFar / (tanf(fFOV * 0.5f) * fFar) * 0.5f;
	        const float fInvPers = 1.0f / Pers;
	        const float fHalfTangens = fInvPers * 0.5f;

	        // near / far
	        aPlanes[0][0] = 0.0f;  aPlanes[0][1] = 0.0f; aPlanes[0][2] = 1.0f;   aPlanes[0][3] = -fNear;
	        aPlanes[1][0] = 0.0f;  aPlanes[1][1] = 0.0f; aPlanes[1][2] = -1.0f;  aPlanes[1][3] = fFar;

	        // Side planes (through the camera origin), D = 0.
	        const ZVector3 vUp(0.0f, 1.0f, 0.0f);

	        ZVector3 vSideDir(-fHalfTangens, 0.0f, 1.0f);
	        if (bWideScreen)
	        {
	            vSideDir.x *= 1.3333334f;
	        }
	        vcross(&aPlanes[3][0], vUp, vSideDir);

	        vSideDir.x = fHalfTangens;
	        if (bWideScreen)
	        {
	            vSideDir.x *= 1.3333334f;
	        }
	        vcross(&aPlanes[2][0], vSideDir, vUp);

	        const float fVertical = (fInvPers / m_vScale.y) * fAspect * 0.5f;
	        const ZVector3 vXAxis(1.0f, 0.0f, 0.0f);

	        ZVector3 vVerticalDir(0.0f, fVertical, 1.0f);
	        vcross(&aPlanes[4][0], vXAxis, vVerticalDir);

	        vVerticalDir.y = -fVertical;
	        vcross(&aPlanes[5][0], vVerticalDir, vXAxis);
	    }

	    m_ClipPlanes.Init();
	    m_ClipPlanes.Add(&aPlanes[0][0], 0);
	    m_ClipPlanes.Add(&aPlanes[1][0], 1);
	    m_ClipPlanes.Add(&aPlanes[2][0], 2);
	    m_ClipPlanes.Add(&aPlanes[3][0], 3);
	    m_ClipPlanes.Add(&aPlanes[4][0], 4);
	    m_ClipPlanes.Add(&aPlanes[5][0], 5);

	    m_vScale.z = 1.0f;
	}

	void ZCAMERA::Proj2D(ZVector2* pScreenPos, const ZVector3* pViewPos)
	{
	    if (CameraCon & 1)
	    {
	        pScreenPos->x = m_vScale.x * pViewPos->x;
	        pScreenPos->y = -(m_vScale.y * pViewPos->y);
	    }
	    else
	    {
	        float fZ = pViewPos->z;
	        if (fZ < 0.001f)
	        {
	            fZ = 0.001f;
	        }

	        const float fFactor = Pers / fZ;
	        pScreenPos->x = fFactor * pViewPos->x;
	        pScreenPos->y = -(m_vScale.y * pViewPos->y * fFactor);
	    }
	}

	void ZCAMERA::Proj3D(ZVector3* pScreenPos, const ZVector3* pViewPos)
	{
	    if (CameraCon & 1)
	    {
	        pScreenPos->x = pViewPos->x / m_vScale.x;
	        pScreenPos->y = -(pViewPos->y / m_vScale.y);
	    }
	    else
	    {
	        const float fFactor = pScreenPos->z / Pers;
	        pScreenPos->x = pViewPos->x * fFactor;
	        pScreenPos->y = -(pViewPos->y / m_vScale.y * fFactor);
	    }
	}

	void ZCAMERA::Proj2D3D(ZVector3* pScreenPos, ZVector3* pViewDir)
	{
	    if (CameraCon & 1)
	    {
	        const float fX = pScreenPos->x / m_vScale.x;
	        const float fY = pScreenPos->y / m_vScale.y;

	        pScreenPos->z = 0.0f;
	        pScreenPos->x = fX;
	        pScreenPos->y = fY;

	        if (pViewDir)
	        {
	            pViewDir->x = 0.0f;
	            pViewDir->y = 0.0f;
	            pViewDir->z = 1.0f;
	        }
	    }
	    else
	    {
	        const float fInvPers = 1.0f / Pers;
	        const float fDirX = fInvPers * pScreenPos->x;
	        const float fDirY = pScreenPos->y / m_vScale.y * fInvPers;

	        pViewDir->x = fDirX;
	        pViewDir->y = fDirY;
	        pViewDir->z = 1.0f;

	        pScreenPos->x = 0.0f;
	        pScreenPos->y = 0.0f;
	        pScreenPos->z = 0.0f;
	    }
	}

	void ZCAMERA::SetCameraRoot(unsigned int lRootRef)
	{
	    m_rCameraRoot = lRootRef;
	}

	void ZCAMERA::SetCameraListPrio(float fPrio)
	{
	    CameraListPri = static_cast<int>(fPrio);

	    for (ZRender* pWindow = g_pSysInterface->WindowFirst; pWindow; pWindow = pWindow->Nxt)
	    {
	        if (pWindow->ChkCamera(this))
	        {
	            pWindow->SetCameraPrio(this, fPrio);
	        }
	    }
	}

	void ZCAMERA::SetCurrentRoomHint(ZROOM* pRoom)
	{
	    m_pCurrentRoomHint = pRoom;
	}

	ZROOM* ZCAMERA::GetCurrentRoomHint()
	{
	    return m_pCurrentRoomHint;
	}

	bool ZCAMERA::GetScreenSelect(SScreenSelect* pScreenSelect, bool bUseColi, unsigned int lGeomControlMask, float fSelectDistance)
	{
	    (void)bUseColi;

	    ZROOM* pRoot = g_pSysInterface->m_pEngineData->m_pRoot;
	    if (!pRoot)
	    {
	        return false;
	    }

	    COLI coli;

	    coli.lp = ZVector3(0.0f, 0.0f, fSelectDistance + Near);
	    GetRootPoint(coli.lp);

	    coli.ln = ZVector3(0.0f, 0.0f, Far);
	    GetRootVect(coli.ln);

	    if (pRoot->ChkLineColi(&coli, eGlobalTreeType::GT_StdObjs, static_cast<int32_t>(lGeomControlMask), -1, true, true))
	    {
	        ZASSERT(ZGeomBuffer::m_Instance);
	        if (ZGeomBuffer::m_Instance->GeomRefToBasePtr(coli.ColiRef))
	        {
	            pScreenSelect->p0 = coli.cp;
	            pScreenSelect->rGeom = coli.ColiRef;
	            return true;
	        }
	    }

	    // No (valid) hit: still report the point at the far end of the select ray.
	    vadd(pScreenSelect->p0, coli.lp, coli.ln);
	    return false;
	}

	void ZCAMERA::SetWideScreen(bool bWideScreen)
	{
	    m_bWideScreen = bWideScreen;

	    if (bWideScreen)
	    {
	        FOV = 1.4137168f;
	    }
	    else
	    {
	        FOV = 1.1693707f;
	    }
	}

	void ZCAMERA::FindCurrentRoom(ZROOM** pRooms, unsigned int searchLimit)
	{
	    if (searchLimit == 1)
	    {
	        m_pCurrentRoom = pRooms[0];
	        return;
	    }

	    if (ZROOM* pRoomHint = GetCurrentRoomHint())
	    {
	        for (unsigned int i = 0; i < searchLimit; ++i)
	        {
	            if (pRooms[i] == pRoomHint)
	            {
	                m_pCurrentRoom = pRoomHint;
	                return;
	            }
	        }
	    }

	    if (m_pCurrentRoom)
	    {
	        for (unsigned int i = 0; i < searchLimit; ++i)
	        {
	            if (pRooms[i] == m_pCurrentRoom)
	            {
	                return;
	            }
	        }
	    }

	    if (searchLimit)
	    {
	        m_pCurrentRoom = pRooms[0];
	    }
	}

	void ZCAMERA::SetFogEnabled(bool* bEnabled)
	{
	    if (*bEnabled)
	    {
	        CameraCon &= 0xFFF7FFFF;
	    }
	    else
	    {
	        CameraCon |= 0x80000;
	    }
	}

	bool ZCAMERA::IsFogEnabled()
	{
	    return (CameraCon & 0x80000) == 0;
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
