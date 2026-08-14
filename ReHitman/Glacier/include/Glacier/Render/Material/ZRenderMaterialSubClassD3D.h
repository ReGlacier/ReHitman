#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct SRenderMaterialBinderValidator
    {
        const char* m_pszBinderName;
        bool m_bBinderValue;
    };

    class ZRenderMaterialSubClassD3D : public ZRenderMaterialSubClass
    {
    public:
        // vtbl
        ~ZRenderMaterialSubClassD3D() override;
        void Draw(const ZRenderMaterialInstance* pMaterial, ZRenderObjectInstance** ppObjects, uint32_t lObjectsNr, ZRenderContext* pCtx) override;
        ZRenderMaterialInstance* CreateMaterialInstance(const SRMaterialProperties* pProperties, uint32_t lMaterialId) override;
        virtual bool CanCreateMaterialInstance(const SRMaterialProperties* pMatProps);

        // methods
        ZRenderMaterialSubClassD3D(const char* pszName, ZRenderMaterialClass* pMaterialClass, uint16_t lObjectType, uint16_t lObjectSubType, uint32_t lMaterialFlags);

        // members
        uint32_t m_lNumBinderValidators;
        SRenderMaterialBinderValidator m_BinderValidators[16];
    };

    RE_VERIFY_OFFSET(ZRenderMaterialSubClassD3D, m_lNumBinderValidators, 0x60); // Verified PC ZRenderMaterialSubClassD3D::CanCreateMaterialInstance
    RE_VERIFY_OFFSET(ZRenderMaterialSubClassD3D, m_BinderValidators, 0x64); // Verified PC ZRenderMaterialSubClassD3D::CanCreateMaterialInstance
}