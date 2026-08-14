#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/SRenderMaterialBinderValidator.h>
#include <Glacier/Render/Material/ZRenderMaterialClass.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <cstdint>

namespace Glacier
{
    class ZRenderMaterialClassD3D : public ZRenderMaterialClass
    {
    public:
        // vtbl
        ~ZRenderMaterialClassD3D() override;
        ZRenderMaterialInstance* CreateMaterialInstance(const SRMaterialProperties* pMatProperties, uint32_t lMaterialId) override;
        virtual void Initialize();
        virtual void AddMaterialSubClassTemplate(const ZRPropertyReader* pMatProperties);
        virtual ZRenderMaterialSubClass* CreateMaterialSubClass(const char* pszSubClassName, uint32_t lObjectType, uint32_t lObjectSubType, uint32_t lMaterialFlags) = 0;

        // methods
        ZRenderMaterialClassD3D();

        // members
        // TODO: Finish me
    };
}