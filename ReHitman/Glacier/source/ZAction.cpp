#include <Glacier/ZAction.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    ZAction** ZAction::GetActionArray()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((ZAction**(__thiscall*)(ZAction*))(G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray))(this);
        }
        return nullptr;
    }

    void ZAction::Show() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZAction_Show != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZAction_Show != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZAction*))(G1ConfigurationService::G1API_FunctionAddress_ZAction_Show))(this);
        }
    }

    void ZAction::Hide() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZAction_Hide != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZAction_Hide != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZAction*))(G1ConfigurationService::G1API_FunctionAddress_ZAction_Hide))(this);
        }
    }

    int ZAction::AddAction(ZGEOM* pGeom,
                                const char* psLocalizedActionName,
                                const char* psActionName,
                                EActionType actionType,
                                Glacier::ZMSGID commandId,
                                Glacier::ZREF entityRef,
                                int unk0,
                                int radius) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZAction_AddAction != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZAction_AddAction != G1ConfigurationService::kNotConfiguredOption) {
            return ((int(__cdecl*)(ZGEOM*, const char*, const char*, EActionType, Glacier::ZMSGID, Glacier::ZREF, int, int))(G1ConfigurationService::G1API_FunctionAddress_ZAction_AddAction))
            (pGeom, psLocalizedActionName, psActionName, actionType, commandId, entityRef, unk0, radius);
        }

        return 0;
    }
}