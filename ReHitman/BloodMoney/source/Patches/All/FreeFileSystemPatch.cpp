#include <BloodMoney/Patches/All/FreeFileSystemPatch.h>
#include <BloodMoney/FreeFS/HBMFreeFsProxy.h>
#include <Glacier/FsZip_t.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney {
    namespace Consts {
        static constexpr std::intptr_t FsZip_Ctor = 0x0042D0FC;
        //static constexpr std::intptr_t FsZip_Dtor = 0x0042D137;
        static constexpr std::intptr_t FsZip_Read_Index = 7;
    }

    namespace Globals {
        [[maybe_unused]] static std::unique_ptr<HF::Hook::VFHook<FreeFS::HBMFreeFsProxy, false>> g_FsZip_Read_Hook;
    }

    namespace Callbacks {
        static void __stdcall OnFsZipConstructed(Glacier::FsZip_t* instance) {
            spdlog::info("FsZip| Constructed at {:08X}", ((std::intptr_t)instance));
            auto proxy = reinterpret_cast<FreeFS::HBMFreeFsProxy*>(instance);
            Globals::g_FsZip_Read_Hook = HF::Hook::HookVirtualFunctionWithoutRestore<FreeFS::HBMFreeFsProxy, Consts::FsZip_Read_Index>(proxy,
                                                                                                                         &FreeFS::HBMFreeFsProxy::readFileProvider);
        }
    }

    std::string_view FreeFileSystemPatch::GetName() const { return "FreeFileSystem Patch"; }

    bool FreeFileSystemPatch::Apply(const ModPack& modules) {
        if (auto process = modules.process.lock())
        {
            //TODO: Make a patch
            m_FsZipConstructor = HF::Hook::HookFunction<void(__stdcall*)(Glacier::FsZip_t*), 10>(
                    process,
                    Consts::FsZip_Ctor,
                    &Callbacks::OnFsZipConstructed,
                    {
                            HF::X86::PUSH_AD,
                            HF::X86::PUSH_FD,
                            HF::X86::PUSH_EAX
                    },
                    {
                            HF::X86::POP_FD,
                            HF::X86::POP_AD,
                    });
            if (!m_FsZipConstructor->setup())
            {
                spdlog::error("Failed to apply patch for ZCutSequencePlayerPatch (ctor)");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void FreeFileSystemPatch::Revert(const ModPack& modules) {
        BasicPatch::Revert(modules);

        if (auto process = modules.process.lock())
        {
            Globals::g_FsZip_Read_Hook = nullptr;

            (void)process; // avoid warning
            if (m_FsZipConstructor) {
                m_FsZipConstructor->remove();
                m_FsZipConstructor = nullptr;
            }
        }
    }
}