#include <Glacier/Render/Entry/ZRenderEntryDeformerD3D.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Physics/ZCloth.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/SRenderEntryInstance.h>
#include <Glacier/Render/Light/SUpdateLightData.h>
#include <Glacier/Render/Light/ZRenderSoftwareLight.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    ZRenderEntryDeformerD3D::ZRenderEntryDeformerD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryDeformer(sInfo)
        , m_lUnknown9C(0)
        , m_hPrimAccess{}
        , m_pCloth(nullptr)
    {
        if (m_pBaseGeom)
        {
            const auto* pGeom = m_pBaseGeom->GetGeom();
            if (pGeom)
                m_pCloth = pGeom->IsDerivedFrom<ZCloth>() ? const_cast<ZCloth*>(static_cast<const ZCloth*>(pGeom)) : nullptr;
            else if (m_pBaseGeom->IsDerivedFromStdObj(ZCloth::m_Id))
                m_pCloth = reinterpret_cast<ZCloth*>(m_pBaseGeom->m_pExtraGeom);
        }

        if (!g_pRenderDll || !g_pRenderDll->m_pPrimControl)
            return;

        if (!m_pBaseGeom)
            return;

        const ZPrimHandle hPrim{ g_pRenderDll->m_pPrimControl->GetSubPrim(m_pBaseGeom->Prim(), 0) };
        auto* pPrimAccess = ZPrimAccess::Create(hPrim);
        if (!pPrimAccess)
            return;

        pPrimAccess->Lock(ZPrimAccess::LF_READONLY);
        m_hPrimAccess = pPrimAccess->m_hPrim;
        pPrimAccess->Unlock();
        pPrimAccess->Destroy();

        if (m_hPrimAccess)
        {
            auto* pRenderDraw = IDraw::Instance<ZRenderDraw>();
            const auto* pPrimData = m_hPrimAccess.Get<const uint8_t>();
            if (pRenderDraw && pPrimData)
            {
                if (auto* pInstance = pRenderDraw->CreateRenderEntryInstance(m_hPrimAccess, static_cast<ZRenderEntry*>(this), m_pBaseGeom, true))
                {
                    InitRenderEntryInstance(pInstance, pPrimData[14], *pPrimData, 1u, 0u);
                    SRenderEntryInstance* instances[] = { pInstance };
                    AddRenderEntryInstances(instances, 1u);
                }
            }
        }

        if (m_pCloth)
            m_pCloth->m_ppPrimAccess = &m_hPrimAccess;
    }

    ZRenderEntryDeformerD3D::~ZRenderEntryDeformerD3D()
    {
        if (m_pCloth)
            m_pCloth->m_ppPrimAccess = nullptr;
    }

    void ZRenderEntryDeformerD3D::Notify(const SRenderEntryNotifyInfo*)
    {
        if (!m_pCloth)
            return;

        m_pCloth->DrawIt(m_ObjectToWorldMatrix.m0, m_ObjectToWorldMatrix.p0);
        for (uint16_t i = 0; i < m_lNumRenderEntryInstances; ++i)
        {
            if (auto* pInstance = m_pRenderEntryInstances[i]->pRenderObjectInstance)
                pInstance->m_lFlags |= 1u;
        }
        m_lControl |= RE_NEEDUPDATE;
        m_lFade = 0xFF;
    }

    void ZRenderEntryDeformerD3D::Update()
    {
        if (!m_pBaseGeom)
            return;

        ZRenderEntryGeom::Update();
        if ((m_lControl & RE_NEEDUPDATE) == 0)
            return;

        SUpdateLightData updateLightData[12] {};
        const uint32_t lightCount = ZRenderSoftwareLight::GetUpdateLightData(
            updateLightData, m_pBaseGeom, m_pEnvironment);

        for (uint16_t i = 0; i < m_lNumRenderEntryInstances; ++i)
        {
            auto* pInstance = m_pRenderEntryInstances[i]->pRenderObjectInstance;
            if (pInstance && ((m_lControl & RE_UPDATELIGHT) != 0 || (pInstance->m_lFlags & 1) != 0))
            {
                pInstance->UpdateLight(updateLightData, lightCount);
                pInstance->m_lFlags &= ~1u;
            }
        }
    }

    void ZRenderEntryDeformerD3D::GetVisible(ZCmdList*, ZRenderEntryGeom*, ZViewSpace*, ZRenderView*, ZRenderEntryLists*)
    {
    }

    ZRenderEntryDeformerD3D* ZRenderEntryDeformerD3D::Create(ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom)
            return nullptr;

        const auto* pGeom = sInfo.m_pBaseGeom->GetGeom();
        const bool bCloth = pGeom
            ? pGeom->IsDerivedFrom<ZCloth>()
            : sInfo.m_pBaseGeom->IsDerivedFromStdObj(ZCloth::m_Id);
        if (!bCloth)
            return nullptr;

        return ZUniMemory::New<ZRenderEntryDeformerD3D>(sInfo);
    }
}
