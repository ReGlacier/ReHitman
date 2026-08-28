#pragma once

#include <Glacier/Glacier.h>


namespace Glacier
{
    void InitGlacierSystem(int hInstance, bool bEditorMode, char* pszGameName);
    void FreeGlacierSystem();
}

extern "C"
{
    GLACIER_API int RunEngine(bool bOnce);
}
