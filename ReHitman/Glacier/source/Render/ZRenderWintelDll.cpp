#include <Glacier/Render/ZRenderWintelDll.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    // PC 0x00484A90; iOS 0x10023F984.
    ZRenderWintelDll::ZRenderWintelDll() = default;

    // PC 0x00484AB0; iOS 0x10023F9A4.
    void ZRenderWintelDll::CrashFree()
    {
        for (ZRender* pRender = g_pSysInterface->WindowFirst; pRender; pRender = pRender->Nxt)
            pRender->CrashClose();
    }
}
