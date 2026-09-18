#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    class ZCompletionList;

    /**
     * @brief Element kinds reported by ZDebugCommand::GetType().
     *
     * Matches the original ECLASSTYPE enum (hitman3/_ps2/startup.cpp).
     */
    enum ECLASSTYPE
    {
        ECLASS_FLOAT = 0,
        ECLASS_INT = 1,
        ECLASS_UNKNOWN = 2,
    };

    /**
     * @brief Base node of the global debug command/var registry
     * (common/zdebugii/zdebug_var.cpp).
     *
     * Every constructed instance links itself into the singly linked list headed by
     * the static m_pFirst and unlinks again on destruction, which is how debug vars
     * and console commands self-register from static initializers.
     */
    class ZDebugCommand
    {
    public:
        // methods
        ZDebugCommand(const char* pszName, const char* pszDescription);
        virtual ~ZDebugCommand();

        virtual ECLASSTYPE GetType();
        virtual void Execute(uint32_t iArgC, const char** ppszArgV) = 0;
        virtual void GetTabCompletionList(uint32_t iArgC, const char** ppszArgV, ZCompletionList* pList);
        virtual void Inc();
        virtual void Dec();
        virtual void CalcValue();
        virtual char* GetValue();

        static ZDebugCommand* First();
        ZDebugCommand* Next();
        const char* Name();
        const char* Description();
        const char* GetPath();

        // members
        static ZDebugCommand* m_pFirst;
        const char* m_pszName;
        const char* m_pszDescription;
        ZDebugCommand* m_pNext;
        const char* m_pszPath;
        char m_szValue[16];
    };
    RE_VERIFY_SIZE(ZDebugCommand, 0x24);
    RE_VERIFY_OFFSET(ZDebugCommand, m_pszName, 0x04);
    RE_VERIFY_OFFSET(ZDebugCommand, m_pszDescription, 0x08);
    RE_VERIFY_OFFSET(ZDebugCommand, m_pNext, 0x0C);
    RE_VERIFY_OFFSET(ZDebugCommand, m_pszPath, 0x10);
    RE_VERIFY_OFFSET(ZDebugCommand, m_szValue, 0x14);
}
