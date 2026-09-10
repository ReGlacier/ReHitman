#include <Glacier/Render/Entry/ZRenderEntryCameraD3D.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/Render/Prim/ELightType.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Com/CCom.h>


namespace Glacier
{
    ZRenderEntryCameraD3D::ZRenderEntryCameraD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryCamera(sInfo)
    {
        int32_t lEnableDotShadows = 0;
        g_pEngineData->GetSceneCom()->GetVal("PC_XBOX_EnableDotFadeDropShadows", &lEnableDotShadows);
        g_PC_XBOX_EnableDotFadeDropShadows = lEnableDotShadows != 0;
    }

    ZRenderEntryCameraD3D::~ZRenderEntryCameraD3D() = default;

    void ZRenderEntryCameraD3D::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList)
    {
        // TODO: Finish me (PC 004B9A50)
    }

    ZRenderEntryCameraD3D* ZRenderEntryCameraD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom->IsDerivedFrom<ZCAMERA>())
            return nullptr;

        return ZUniMemory::New<ZRenderEntryCameraD3D>(sInfo);
    }

    void ZRenderEntryCameraD3D::BuildDrawChainDropShadows(ZRenderEntryGeom* pParentRenderEntryGeom, ZRenderView* pRenderView, ZCmdList* pCmdList, ZRenderEntryLists* pGeomList, uint32_t* plMaxProjections)
    {
        // TODO: Finish me (PC 004B4DA0)
    }

    void ZRenderEntryCameraD3D::BuildDrawChainStaticShadowTextureProjections(
        ZCmdList* pCmdList,
        uint32_t lMaxProjections,
        ZStackArray<1024u, ZRenderEntryGeom*>& LnkList,
        ZRenderView* pRenderView,
        const ZVector3& vObserver,
        float fLODScale,
        uint32_t lType,
        uint32_t lLayer)
    {

        // TODO: Finish me (PC 004B4BE0)
    }

    float ZRenderEntryCameraD3D::CalculateShadowIntensity(const ZBaseGeom* pLight, const ZBaseGeom* pCaster)
    {
        ZVector3 vCaster;
        ZVector3 vVect(0.0f, 1.0f, 0.0f);

        pCaster->GetCen(vCaster);
        pCaster->GetRootPoint(vCaster);
        const_cast<ZBaseGeom*>(pLight)->GetLocalPointVect(vCaster, vVect);

        const auto* const pLightPrim = ZPrimHandle{ pLight->m_lPrim }.Get<SPrimLightSpot>();

        if (pLightPrim->lLightType == LTENVIRONMENT || pLightPrim->lLightType == LTDIRECTIONAL)
        {
            return 1.0f;
        }
        else if (pLightPrim->lLightType > 2u)
        {
            return 0.0f;
        }

        const float fDist = 1.0f / (pLightPrim->fFarRange - pLightPrim->fNearRange);
        const float fInvDist = -fDist;
        const float fIntensity = pLightPrim->fFarRange * fDist;

        ZVector3 vToLight;
        const float fRadialBottleneck = (std::max)(0.0f, vnorm(vToLight.Get(), vCaster.Get()) - pCaster->m_fRadius);
        const float fAttenuation = clamp(fIntensity + fRadialBottleneck * fInvDist, 0.0f, 1.0f);

        vneg(vToLight.Get());

        float fShadowIntensity;
        if (g_PC_XBOX_EnableDotFadeDropShadows)
        {
            const float fDotFade = vdot(vToLight.Get(), vVect.Get()) * fAttenuation;
            fShadowIntensity = (std::max)(0.0f, fDotFade);
        }
        else
        {
            fShadowIntensity = fAttenuation;
        }

        const uint8_t lLightType = pLightPrim->lLightType;
        if (lLightType != LTSPOT && lLightType != LTSPOTSQUARE)
        {
            return fShadowIntensity;
        }

        if (vCaster.z <= 0.0f)
        {
            return 0.0f;
        }

        float fConeFactor;
        if (lLightType == LTSPOTSQUARE)
        {
            const auto* const pSquare = static_cast<const SPrimLightSpotSquare*>(pLightPrim);
            const float fTanFallOff = std::tan(pSquare->fFallOff);
            const float fSquareSize = std::sqrt(pSquare->fAspect) * fTanFallOff;
            const float fHotSpotRange = 1.0f - std::tan(pSquare->fHotSpot) / fTanFallOff;
            const float fInvDepth = 1.0f / vCaster.z;
            const float fOffset = -1.0f / fHotSpotRange;
            float fX = std::fabs((1.0f / (fHotSpotRange * fSquareSize)) * vCaster.x * fInvDepth) + fOffset;
            float fY = std::fabs((1.0f / (fSquareSize / pSquare->fAspect * fHotSpotRange)) * vCaster.y * fInvDepth) + fOffset;
            fX = clamp(fX, -1.0f, 0.0f);
            fY = clamp(fY, -1.0f, 0.0f);
            fConeFactor = fY * fX;
        }
        else
        {
            const float fInvRange = 1.0f / (pLightPrim->fCosHotSpot - pLightPrim->fCosFallOff);
            const float fCone = fInvRange * (-vToLight.z - pLightPrim->fCosFallOff);
            fConeFactor = clamp(fCone, 0.0f, 1.0f);
        }

        return fShadowIntensity * fConeFactor * fConeFactor * (3.0f - 2.0f * fConeFactor);
    }
}
