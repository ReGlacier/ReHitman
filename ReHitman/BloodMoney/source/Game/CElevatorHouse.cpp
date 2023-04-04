#include <BloodMoney/Game/Elevator/CElevatorHouse.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    CElevatorHouse::EDoorStatus CElevatorHouse::GetDoorStatusOfElevatorByREF(Glacier::ZREF elevatorObjectREF) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CElevatorHouse_GetElevatorDoorStatus != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CElevatorHouse_GetElevatorDoorStatus != BMConfigurationService::kNotConfiguredOption) {
            using Func = EDoorStatus(__stdcall*)(Glacier::ZREF);
            return ((Func)BMConfigurationService::BMAPI_FunctionAddress_CElevatorHouse_GetElevatorDoorStatus)(elevatorObjectREF);
        }
        return CElevatorHouse::EDoorStatus::DOOR_STATUS_INVALID;
    }
}