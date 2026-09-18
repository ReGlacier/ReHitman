#include <Glacier/Render/Entry/ZRenderEntryLight.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntryLight::ZRenderEntryLight(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeom(sInfo)
    {
        m_lGeomListsControl = 2;

        if (m_pBaseGeom)
        {
            m_hPrimLight = ZPrimHandle{ m_pBaseGeom->Prim() };
        }
    }

    ZRenderEntryLight::~ZRenderEntryLight() = default;

    void ZRenderEntryLight::Update()
    {
        if (m_pBaseGeom && (m_pBaseGeom->m_lControl & 0x02000000u) != 0)
            m_pBaseGeom->LightNotifyPotentialDetachment(false);

        // PC 0x00477180 jumps to ZBaseGeom::UpdateLightListForLight at 0x00434710.
        if (m_pBaseGeom)
            m_pBaseGeom->UpdateLightListForLight();
    }

    ZRenderEntryLight::RENDERENTRY_BASETYPE ZRenderEntryLight::GetType() const
    {
        return ZRenderEntryLight::RENDERENTRY_BASETYPE::RT_LIGHT;
    }

    const ZPrimHandle& ZRenderEntryLight::GetPrimLight() const
    {
        return m_hPrimLight;
    }

    ZRenderEntryLight* ZRenderEntryLight::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom)
            return nullptr;

        if (!sInfo.m_pBaseGeom->IsDerivedFrom<ZLIGHT>())
            return nullptr;

        return ZUniMemory::New<ZRenderEntryLight>(sInfo);
    }
}
