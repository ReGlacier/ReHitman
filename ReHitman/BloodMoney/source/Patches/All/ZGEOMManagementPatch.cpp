#include <BloodMoney/Patches/All/ZGEOMManagementPatch.h>
#include <BloodMoney/Editors/LevelEditorGeomsPool.h>
#include <Glacier/ZGEOM.h>

#include <spdlog/spdlog.h>
#include <cstdint>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        constexpr std::intptr_t ZGEOM_CtorAddr = 0x004E7F43;
        constexpr std::intptr_t ZGEOM_DtorAddr = 0x004E8A97;
    }

    namespace Callbacks
    {
        void __stdcall ZGEOM_OnConstructed(Glacier::ZGEOM* instance)
        {
            LevelEditorGeomsPool::GetInstance().AddGeom(instance);
        }

        void __stdcall ZGEOM_OnReadyToDestroy(Glacier::ZGEOM* instance)
        {
            LevelEditorGeomsPool::GetInstance().RemoveGeom(instance);
        }
    }

    std::string_view ZGEOMManagementPatch::GetName() const { return "(Demo) ZGEOM construction patch"; }

    bool ZGEOMManagementPatch::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            m_ZGEOM_Ctor = HF::Hook::HookFunction<void(__stdcall*)(Glacier::ZGEOM*), CtorPatchSize>(
                    process,
                    Consts::ZGEOM_CtorAddr,
                    &Callbacks::ZGEOM_OnConstructed,
                    {
                        HF::X86::PUSH_AD,
                        HF::X86::PUSH_FD,
                        HF::X86::PUSH_EAX
                    },
                    {
                        HF::X86::POP_FD,
                        HF::X86::POP_AD
                    });

            if (!m_ZGEOM_Ctor->setup())
            {
                spdlog::error("ZGEOMManagementPatch| Failed to setup ZGEOM constructor patch!");
                return false;
            }

            m_ZGEOM_Dtor = HF::Hook::HookFunction<void(__stdcall*)(Glacier::ZGEOM*), DtorPatchSize>(
                    process,
                    Consts::ZGEOM_DtorAddr,
                    &Callbacks::ZGEOM_OnReadyToDestroy,
                    {
                            HF::X86::PUSH_AD,
                            HF::X86::PUSH_FD,
                            HF::X86::PUSH_ECX
                    },
                    {
                            HF::X86::POP_FD,
                            HF::X86::POP_AD
                    });

            if (!m_ZGEOM_Dtor->setup())
            {
                m_ZGEOM_Ctor->remove();

                spdlog::error("ZGEOMManagementPatch| Failed to setup ZGEOM destructor patch!");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void ZGEOMManagementPatch::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_ZGEOM_Ctor->remove();
        m_ZGEOM_Dtor->remove();
    }
}