#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/SRenderMaterialBinderValidator.h>
#include <Glacier/Render/Material/ZRenderMaterialClass.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Runtime/ZFactory.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    struct EmptyType
    {
    };

    template <typename T>
    class ZProductDefaultCreator
    {
    public:
        struct Create
        {
            template <typename U>
            static U* Do(const EmptyType& __formal)
            {
                return ZUniMemory::New<U>();
            }
        };
    };

    class ZRenderMaterialClassD3D : public ZRenderMaterialClass
    {
    public:
        // constants
        static constexpr uint32_t MAX_NUM_SUB_CLASS_TEMPLATES = 0x80; // Verified PC AddMaterialSubClassTemplate

        // factory
        DECLARE_FACTORY(EmptyType, const char*, ZProductDefaultCreator<ZRenderMaterialClassD3D>);

        // types
        struct SSubClassTemplate
        {
            uint32_t m_lObjectType;
            uint32_t m_lObjectSubType;
            ZRPropertyReader m_Properties;
            uint32_t m_lNumBinderValidators;
            SRenderMaterialBinderValidator* m_pBinderValidators[16];
        };

        // vtbl
        ~ZRenderMaterialClassD3D() override;
        ZRenderMaterialInstance* CreateMaterialInstance(const SRMaterialProperties* pMatProperties, uint32_t lMaterialId) override;
        virtual void Initialize();
        virtual void AddMaterialSubClassTemplate(const ZRPropertyReader* pMatProperties);
        virtual ZRenderMaterialSubClass* CreateMaterialSubClass(const char* pszSubClassName, uint32_t lObjectType, uint32_t lObjectSubType, uint32_t lMaterialFlags) = 0;

        // methods
        ZRenderMaterialClassD3D();

        static ZFactory<ZRenderMaterialClassD3D>& GetFactory();

        // members
        uint32_t m_lNumSubClassTemplates{0u};
        SSubClassTemplate m_SubClassTemplates[MAX_NUM_SUB_CLASS_TEMPLATES];
    };
    RE_VERIFY_OFFSET(ZRenderMaterialClassD3D, m_lNumSubClassTemplates, 0x214); // Verified PC AddMaterialSubClassTemplate
    RE_VERIFY_OFFSET(ZRenderMaterialClassD3D, m_SubClassTemplates, 0x218); // Verified PC AddMaterialSubClassTemplate
    RE_VERIFY_SIZE(ZRenderMaterialClassD3D, 0x2C18); // Verified PC layout
}
