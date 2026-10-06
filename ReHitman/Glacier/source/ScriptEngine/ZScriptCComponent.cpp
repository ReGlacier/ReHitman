#include <Glacier/ScriptEngine/ZScriptCComponent.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/Globals.h>


namespace Glacier
{
    ZScriptCComponent::ZScriptCComponent()
    {
        ScriptEngine::InstallScriptMessages(ZScriptC_ZMessages, Uniques);
    }

    ZScriptCComponent::~ZScriptCComponent()
    {
        ScriptEngine::DestroyScriptMessages();
    }

    // Global thing
    template <>
    ZScriptCComponent* ZComponentSingleton<ZScriptCComponent, Glacier::ZGlobalComponentBase>::m_pInstance = nullptr;
}