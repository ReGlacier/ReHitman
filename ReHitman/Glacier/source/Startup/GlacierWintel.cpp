#include <Glacier/Startup/GlacierStartup.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/System/ZSysInterface.h>
#include <Windows.h>
#include <signal.h>


namespace Glacier
{
    namespace
    {
        void SignalHandler(int lSignal)
        {
            if (g_pSysInterface->m_pSoundDll)
            {
                g_pSysInterface->m_pSoundDll->CrashFree();
            }

            if (g_pRenderDll)
            {
                g_pRenderDll->CrashFree();
            }
        }
    }

    int Glacier_Main(int hInstance, int hPrevInstance, const char* psCmdLine)
    {
        CreateMutexA(nullptr, false, "Hitboy");
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return 0;
        }

        signal(8, SignalHandler);
        signal(22, SignalHandler);
        signal(4, SignalHandler);
        signal(11, SignalHandler);
        // TODO: Finish me

        return 0;
    }
}
