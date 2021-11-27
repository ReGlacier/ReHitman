#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZScheduledUpdate.h>
#include <G1ConfigurationService.h>
#include <HF/HackingFramework.hpp>
#include <cassert>

namespace Glacier
{
    CCom* ZEngineDataBase::GetSceneCom()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((CCom*(__thiscall*)(ZEngineDataBase*))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom))(this);
        }

        return 0;
    }

    std::intptr_t ZEngineDataBase::GetSceneVar(const char* varname) {
        using CCom_t = int;

        auto ccom = (CCom_t*)GetSceneCom();
        if (!ccom) {
            return 0;
        }

        return HF::Hook::VFHook<CCom_t>::invoke<std::intptr_t, const char*, int>(ccom, 36, varname, 2);
    }

    std::intptr_t ZEngineDataBase::SRefToPtr(Glacier::ZREF sref) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((std::intptr_t(__thiscall*)(ZEngineDataBase*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr))(this, sref);
        }

        return 0;
    }

    ZScheduledUpdate* ZEngineDataBase::GetEventScheduler() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetEventScheduler != G1ConfigurationService::kNotConfiguredOption);

        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetEventScheduler != G1ConfigurationService::kNotConfiguredOption) {
            return ((ZScheduledUpdate*(__thiscall*)(ZEngineDataBase*))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetEventScheduler))(this);
        }

        return nullptr;
    }

    CCom* ZEngineDataBase::GetGlobalCom() {
        assert(G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom != G1ConfigurationService::kNotConfiguredOption);

        if (G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom != G1ConfigurationService::kNotConfiguredOption) {
            return reinterpret_cast<Glacier::CCom*>(G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom);
        }

        return nullptr;
    }
}