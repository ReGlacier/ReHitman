#include <ScriptingRuntime.h>
#include <MonoException.h>
#include <MonoBindings.h>

#include <filesystem>
#include <stdexcept>
#include <utility>

#include <spdlog/spdlog.h>

// Mono
#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

namespace Hitman::BloodMoney::Scripting {
    constexpr const char* kRuntimeName = "ReHitman.ScriptingRuntime";
    constexpr const char* kScriptingRuntimeAssemblyName = "ReHitman.ScriptingRuntime.dll";

    struct ScriptingRuntime::RuntimeContext {
        // Mono & JIT specific things
        MonoDomain* m_pMonoDomain {};
        MonoAssembly* m_pRuntimeAssembly {};

        MonoMethod* m_pRuntimeBroker_Init_Method;

        // Methods
        ~RuntimeContext();
        bool CheckRuntimeBeforeConstruct() const;
        bool Init();
        bool RuntimeBroker_Init();
        void RuntimeBroker_Update(float dt);
        void RuntimeBroker_Draw();

        static void HandleMonoException(MonoObject* pException);
    };

    struct Callbacks {
        using OnUpdateCallback_t = void(__stdcall *)(float); //__stdcall is important!
        using OnDrawCallback_t = void(__stdcall *)(); //__stdcall is important!

    private:
        static OnUpdateCallback_t g_pOnUpdateCallback;
        static OnDrawCallback_t  g_pOnDrawCallback;

    public:
        static void SetOnUpdateCallback(OnUpdateCallback_t pCallback) {
            g_pOnUpdateCallback = pCallback;
        }

        static void SetOnDrawCallback(OnDrawCallback_t pCallback) {
            g_pOnDrawCallback = pCallback;
        }

        static void TryInvokeOnUpdate(float dt) {
            if (g_pOnUpdateCallback) {
                (*g_pOnUpdateCallback)(dt);
            }
        }

        static void TryInvokeOnDraw() {
            if (g_pOnDrawCallback) {
                (*g_pOnDrawCallback)();
            }
        }
    };

    Callbacks::OnUpdateCallback_t Callbacks::g_pOnUpdateCallback = nullptr;
    Callbacks::OnDrawCallback_t Callbacks::g_pOnDrawCallback = nullptr;

    ScriptingRuntime::ScriptingRuntime()
        : m_pRuntimeContext(new ScriptingRuntime::RuntimeContext())
    {
    }

    ScriptingRuntime::~ScriptingRuntime() {
        if (m_pRuntimeContext) {
            delete m_pRuntimeContext;
            m_pRuntimeContext = nullptr;
        }
    }

    bool ScriptingRuntime::Setup() {
        return m_pRuntimeContext->Init();
    }

    void ScriptingRuntime::OnDraw() {
        if (m_pRuntimeContext) {
            m_pRuntimeContext->RuntimeBroker_Draw();
        }
    }

    void ScriptingRuntime::OnUpdate(float dt) {
        if (m_pRuntimeContext) {
            m_pRuntimeContext->RuntimeBroker_Update(dt);
        }
    }

    ScriptingRuntime::RuntimeContext::~RuntimeContext() {
        if (m_pMonoDomain) {
            mono_jit_cleanup(m_pMonoDomain);
            m_pMonoDomain = nullptr;
        }
    }

    bool ScriptingRuntime::RuntimeContext::CheckRuntimeBeforeConstruct() const {
        std::filesystem::path kRuntimeLibraryPath = std::filesystem::current_path() / kScriptingRuntimeAssemblyName;
        std::filesystem::path kMonoPath = std::filesystem::current_path() / "Mono";
        std::filesystem::path kMonoEtcPath = kMonoPath / "etc";
        std::filesystem::path kMonoLibPath = kMonoPath / "lib";

        if (!std::filesystem::is_regular_file(kRuntimeLibraryPath)) {
            spdlog::error("No ReHitman.ScriptingRuntime.dll in current folder. Scripting support will be disabled");
            return false;
        }

        if (!std::filesystem::is_directory(kMonoPath)) {
            spdlog::error("No Mono folder in current folder. Scripting support will be disabled");
            return false;
        }

        if (!std::filesystem::is_directory(kMonoEtcPath)) {
            spdlog::error("No Mono/lib folder in current folder. Scripting support will be disabled");
            return false;
        }

        if (!std::filesystem::is_directory(kMonoLibPath)) {
            spdlog::error("No Mono/etc folder in current folder. Scripting support will be disabled");
            return false;
        }

        return true;
    }

    bool ScriptingRuntime::RuntimeContext::Init() {
        if (!CheckRuntimeBeforeConstruct()) {
            return false;
        }

        mono_set_dirs("Mono\\lib","Mono\\etc");

        m_pMonoDomain = mono_jit_init_version(kRuntimeName, "v4.0");
        if (!m_pMonoDomain) {
            spdlog::error("Failed to initialise MonoRuntime");
            return false;
        }

        MonoBindings::DeclareFunctions();
        mono_add_internal_call("ReHitman.ScriptingRuntime.RuntimeBroker::SetOnUpdateCallbackCxxPtr", Callbacks::SetOnUpdateCallback);
        mono_add_internal_call("ReHitman.ScriptingRuntime.RuntimeBroker::SetOnDrawCallbackCxxPtr", Callbacks::SetOnDrawCallback);

        // Load our ReHitman.ScriptingRuntime.dll as assembly
        m_pRuntimeAssembly = mono_domain_assembly_open(m_pMonoDomain, kScriptingRuntimeAssemblyName);
        if (!m_pRuntimeAssembly) {
            spdlog::error("Failed to load runtime assembly!");
            return false;
        }

        MonoImage* pImage = mono_assembly_get_image(m_pRuntimeAssembly);
        if (!pImage) {
            spdlog::error("Failed to locate image of runtime assembly!");
            return false;
        }

        // Load methods map
        MonoClass* pRuntimeBrokerClass = mono_class_from_name(pImage, "ReHitman.ScriptingRuntime", "RuntimeBroker");
        if (!pRuntimeBrokerClass) {
            spdlog::error("No RuntimeBroker class found!");
            return false;
        }

        void* ptr = nullptr;
        MonoMethod* pIter = nullptr;
        while ((pIter = mono_class_get_methods(pRuntimeBrokerClass, &ptr)) != nullptr) {
            std::string_view kName = mono_method_get_name(pIter);
            constexpr std::string_view kInit = "Init";

            if (kName == kInit) {
                m_pRuntimeBroker_Init_Method = pIter;
                break;
            }
        }

        if (!m_pRuntimeBroker_Init_Method) {
            spdlog::error("Failed to locate method RuntimeBroker::Init()");
            return false;
        }

        spdlog::info("Scripting: RuntimeBroker found and valid. Scripting is ready to be started!");
        return RuntimeBroker_Init();
    }

    bool ScriptingRuntime::RuntimeContext::RuntimeBroker_Init() {
        if (m_pRuntimeBroker_Init_Method != nullptr) {
            MonoObject* pException = nullptr;
            mono_runtime_invoke(m_pRuntimeBroker_Init_Method, nullptr, nullptr, &pException);
            HandleMonoException(pException);

            return (pException == nullptr);
        }

        return false;
    }

    void ScriptingRuntime::RuntimeContext::RuntimeBroker_Update(float dt) {
        Callbacks::TryInvokeOnUpdate(dt);
    }

    void ScriptingRuntime::RuntimeContext::RuntimeBroker_Draw() {
        Callbacks::TryInvokeOnDraw();
    }

    void ScriptingRuntime::RuntimeContext::HandleMonoException(MonoObject* pException) {
        MonoException exception { pException };
        if (exception) {
            spdlog::error("---------[ MONO RUNTIME EXCEPTION ]---------\n{}", exception);
        }
    }
}