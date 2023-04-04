#include <BloodMoney/Game/UI/IGUIElement.h>
#include <BloodMoney/Game/UI/ZWINGROUP.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void IGUIElement::ChangeColorSet(ZWINGROUP *pGroup, ZColorSet *pColorSet) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_ChangeColorSet != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_ChangeColorSet != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(IGUIElement*,ZWINGROUP*,ZColorSet*))BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_ChangeColorSet)(this, pGroup, pColorSet);
        }
    }

    void IGUIElement::ChangeColor(ZWINGROUP *pGroup, ZColorSet *pColorSet, ZColorSet::EColorIndex eColorIndex) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_ChangeColor != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_ChangeColor != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(IGUIElement*,ZWINGROUP*,ZColorSet*,ZColorSet::EColorIndex))BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_ChangeColor)(this, pGroup, pColorSet, eColorIndex);
        }
    }

    void IGUIElement::SetColor(uint32_t rgba, ZWINGROUP *pGroup, uint32_t mask) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_SetColor != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_SetColor != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(IGUIElement*,uint32_t,ZWINGROUP*,uint32_t))BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_SetColor)(this, rgba, pGroup, mask);
        }
    }

    void IGUIElement::GetRightPosOfTextGroup(ZWINGROUP* pGroup, Glacier::Vector3 *pPos, int iOffsetByX) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_GetRightPosOfTextGroup != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_GetRightPosOfTextGroup != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(IGUIElement*,ZWINGROUP*,Glacier::Vector3*,int))BMConfigurationService::BMAPI_FunctionAddress_IGUIElement_GetRightPosOfTextGroup)(this, pGroup, pPos, iOffsetByX);
        }
    }
}