#include <Glacier/System/CConfigurationWintel.h>
#include <Glacier/System/ZSysInterfaceWintel.h>


namespace Glacier
{
    void CConfigurationWintel::Load()
    {
        CConfiguration::Load();

        m_lNumSoundBuffers = 16;
        m_bUseEAX = true;

        char* pszUseEAX;
        if (g_pSysInterface->GetOption("UseEAX", &pszUseEAX))
        {
            m_bUseEAX = atoi(pszUseEAX) != 0;
        }

        char* pszNumSoundBuffers;
        if (g_pSysInterface->GetOption("NumSoundBuffers", &pszNumSoundBuffers))
        {
            m_lNumSoundBuffers = atoi(pszNumSoundBuffers);
        }
    }

    void CConfigurationWintel::Save()
    {
        CConfiguration::Save();
        reinterpret_cast<ZSysInterfaceWintel*>(g_pSysInterface)->SaveGraphicsOptions();
    }

    CConfigurationWintel::CConfigurationWintel()
        : CConfiguration()
    {
        m_sNewLine = "\r\n";
    }
}