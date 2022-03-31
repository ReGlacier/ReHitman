#include <Glacier/ZActorCommunication.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    void ZActorCommunication::RegisterRadioUser(Glacier::ZREF rActor, unsigned int iChannel) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZActorCommunication_RegisterRadioUser != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZActorCommunication_RegisterRadioUser != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZActorCommunication*, Glacier::ZREF, unsigned int))G1ConfigurationService::G1API_FunctionAddress_ZActorCommunication_RegisterRadioUser)(this, rActor, iChannel);
        }
    }
}