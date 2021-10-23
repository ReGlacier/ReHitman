#pragma once

#include <memory>
#include <string_view>

namespace Hitman::BloodMoney::Scripting {
    class ScriptingRuntime final
    {
    public:
        using Ptr = std::unique_ptr<ScriptingRuntime>;

        ScriptingRuntime();
        ~ScriptingRuntime();

        bool Setup();

        /// Common methods
        void OnDraw();
        void OnUpdate(float dt);

    private:
        struct RuntimeContext;
        RuntimeContext* m_pRuntimeContext;
    };
}