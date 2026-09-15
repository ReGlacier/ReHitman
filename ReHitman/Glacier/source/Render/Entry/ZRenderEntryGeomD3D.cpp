#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Entry/SRenderEntryInstance.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomD3D.h>
#include <Glacier/Render/Light/ZRenderSoftwareLight.h>
#include <Glacier/Render/Light/SUpdateLightData.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntryGeomD3D::~ZRenderEntryGeomD3D() = default;

    void ZRenderEntryGeomD3D::Update()
    {
        ZRenderEntryGeom::Update();

        if ((m_lControl & RE_NEEDUPDATE) != 0 && m_pBaseGeom)
        {
            SUpdateLightData sUpdateLightData[12] {};

            uint32_t lIndex = 0,
                     lNumLightSources = ZRenderSoftwareLight::GetUpdateLightData(sUpdateLightData, m_pBaseGeom, m_pEnvironment);

            for (; lIndex < m_lNumRenderEntryInstances; ++lIndex)
            {
                auto* pInstance = m_pRenderEntryInstances[lIndex]->pRenderObjectInstance;

                if ((m_lControl & RE_UPDATELIGHT) != 0 || (pInstance->m_lFlags & 1) != 0)
                {
                    pInstance->UpdateLight(sUpdateLightData, lNumLightSources);
                    pInstance->m_lFlags &= ~0x1u;
                }
            }
        }
    }

    ZRenderEntryGeomD3D* ZRenderEntryGeomD3D::Create(ZRenderEntryGeomCreateInfo& sCreationInfo)
    {
        if (sCreationInfo.m_pBaseGeom && (sCreationInfo.m_pBaseGeom->IsDerivedFrom<ZROOM>() || sCreationInfo.m_pBaseGeom->IsDerivedFrom<ZLNKOBJ>()))
        {
            return nullptr;
        }

        return ZUniMemory::New<ZRenderEntryGeomD3D>(sCreationInfo);
    }
}
