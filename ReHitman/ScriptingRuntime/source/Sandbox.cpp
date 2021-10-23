#include <ScriptingRuntime.h>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
    spdlog::info("Trying to load DLL");

    Hitman::BloodMoney::Scripting::ScriptingRuntime runtime;
    runtime.Setup();
    runtime.OnUpdate(10.f);
    runtime.OnDraw();

    return 0;
}