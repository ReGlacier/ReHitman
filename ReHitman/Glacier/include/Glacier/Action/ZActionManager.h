#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Action/ZActionMapTree.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct ZTokenStream;

    class ZActionManager : public Action::Interface
    {
    public:
        // static
        STATIC_CLASS_VAR(ZActionManager, int32_t, m_iSeq);

        // vtbl
        ~ZActionManager() override;
        void SetJoinControllers(bool bJoinControllers, int iDeviceId) override;
        bool GetJoinControllers() const override;
        bool AddBindings(const char* binds) override;
        bool LoadBindings(const char* szFilename) override;
        const char* GetKeyName(const char* pKey) override;
        const char* GetSystemKeyName(const char* szAction, bool firstKey) override;
        const TIMETYPE& GetDisableInputTimes() override;
        void DisableInputTimer(double deltaTime) override;
        void RevertToDefault() override;
        void SetDebugKeys(bool bDebugKeys) override;
        bool GetDebugKeys() const override;
        void Enable() override;
        void Disable() override;
        bool IsEnabled() const override;

        // methods
        ZActionManager();

        int32_t SeqNr() const;
        ZActionMapTree* GetMapping(const char* psName);
        ZActionMapTree* GetMapping(ZActionMapTree* node, const char* psName);
        bool ParseBinds(ZTokenStream& stream);
        ZActionMapTree* ParseAsignment(ZTokenStream& stream);
        ZActionMapTree* ParseExp(ZTokenStream& stream);
        ZActionMapTree* ParseBlock(ZTokenStream& stream, char* pBuffer);
        void DeleteTree(ZActionMapTree* pTree);
        void FreeMap(ZActionMapTree* pTree);
        ZActionMapTree* AllocMap();

        // members
        ZActionMapTree m_kRoot{};
        ZActionMapTree* m_pkCurentBlock{nullptr};
        int  m_iBindMem{0};
        bool m_bDebugKeys{false};
        bool m_bEnabled{true};
        bool m_JoinControllers{false};
        RE_ADD_PADDING(1);
        TIMETYPE m_EventHorizonDelay{}; // Added for PC, iOS and next Glacier games
    };
    RE_VERIFY_SIZE(ZActionManager, 0x54); // Verified by PC
}