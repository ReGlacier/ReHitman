#include <Glacier/Startup/GlacierStartup.h>
#include <Glacier/Filesystem/ZSysFileWintel.h>
#include <Glacier/System/ZSysInterfaceWintel.h>
#include <Glacier/Component/ZComponentGlobalList.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/ZSTL/MYSTR.h>


namespace Glacier
{
    void InitGlacierSystem(int hInstance, bool bEditorMode, char* pszGameName)
    {
        ZComponentGlobalList::Instance().CreateComponents(1u);
        
        g_pGlobalCOM = &g_GlobalCom;
        g_pSysFile = ZUniMemory::New<ZSysFileWintel>();

        g_pSysInterface = ZUniMemory::New<ZSysInterfaceWintel>(hInstance, bEditorMode);

        SysInput::Initialize();

        if (pszGameName)
        {
            g_pSysInterface->m_sGameTitle = MYSTR(pszGameName);
        }

        g_pSysInterface->Init();

        ZComponentGlobalList::Instance().CreateComponents(0u);

        // TODO: Uncomment or remove next line after g_pNetwork will be reversed (or not)
        // g_pNetwork = Glacier::InitNetworkWintel();
    }

    void FreeGlacierSystem()
    {
        ZComponentGlobalList::Instance().DestroyComponents(0u);

        // Weird code from iOS build, but inner things looks not referred
        // So, just keep this code here, maybe in future we will solve this (or remove)
        // GlobalInstanceBase::FreeAllInstance();

        if (g_pSysInterface)
        {
            ZUniMemory::Delete(g_pSysInterface);
            g_pSysInterface = nullptr;
        }

        if (g_pSysFile)
        {
            ZUniMemory::Delete(g_pSysFile);
            g_pSysFile = nullptr;
        }
        
        ZComponentGlobalList::Instance().DestroyComponents(1u);
    }
}