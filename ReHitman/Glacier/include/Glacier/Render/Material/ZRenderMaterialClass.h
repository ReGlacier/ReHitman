#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialClass
    {
    public:
        // constants
        static constexpr int MAX_NUM_SUB_CLASSES = 0x20;

        // vtbl
        virtual ~ZRenderMaterialClass();
        virtual ZRenderMaterialInstance* CreateMaterialInstance(const SRMaterialProperties* pMatProperties, uint32_t lMaterialId) = 0;

        // methods
        ZRenderMaterialClass();

        // members
        const char* m_pszName{nullptr};
        const char* m_pszType{nullptr};
        uint32_t m_lIndex{0};
        ZRenderMaterialSubClass* m_pSubClasses[MAX_NUM_SUB_CLASSES]{nullptr};
        uint32_t m_lNumSubClasses{0};
    };
}