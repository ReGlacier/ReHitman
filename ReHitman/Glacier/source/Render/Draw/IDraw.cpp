#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    IDraw* IDraw::Instance()
    {
        ZASSERT(g_pSysInterface != nullptr);
        ZASSERT(g_pSysInterface->WindowFirst != nullptr);
        
        return reinterpret_cast<IDraw*>(g_pSysInterface->WindowFirst->GetDrawBase());
    }
}