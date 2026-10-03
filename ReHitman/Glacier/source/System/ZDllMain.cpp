#include <Glacier/System/ZDllMain.h>

#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    ZDllMain* ZDllMain::BuildInstance()
    {
        return ZUniMemory::New<ZDllMain>();
    }

    void ZDllMain::Init()
    {
        const char* pSceneName = g_pSysInterface->m_sDefaultScene;
        if (!*pSceneName)
        {
            pSceneName = g_pSysInterface->m_sProjectFile;
        }

        g_pSysFile->RemoveLoadFilter(".anm");
        g_pSysInterface->SetEngineData(ZUniMemory::New<ZEngineDataBase>(pSceneName));
    }

    void ZDllMain::End()
    {
        g_pSysInterface->SetEngineData(nullptr);
    }
}
