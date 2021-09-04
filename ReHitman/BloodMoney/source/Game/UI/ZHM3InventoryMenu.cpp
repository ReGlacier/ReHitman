#include <BloodMoney/Game/UI/ZHM3InventoryMenu.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void ZHM3InventoryMenu::InitInventoryList() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_InitInventoryList != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_InitInventoryList != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_InitInventoryList)(this);
        }
    }

    void ZHM3InventoryMenu::CloseItemView() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView)(this);
        }
    }

    void ZHM3InventoryMenu::CloseInventoryMenu() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView)(this);
        }
    }

    void ZHM3InventoryMenu::UpdateItemView() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemView != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemView != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemView)(this);
        }
    }

    void ZHM3InventoryMenu::UpdateItemInfo() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemInfo != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemInfo != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemInfo)(this);
        }
    }

    void ZHM3InventoryMenu::SpinCircle(bool a1) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_SpinCircle != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_SpinCircle != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*, bool))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_SpinCircle)(this, a1);
        }
    }

    void ZHM3InventoryMenu::TurnLeft() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnLeft != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnLeft != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnLeft)(this);
        }
    }

    void ZHM3InventoryMenu::TurnRight() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnRight != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnRight != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3InventoryMenu*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnRight)(this);
        }
    }
}