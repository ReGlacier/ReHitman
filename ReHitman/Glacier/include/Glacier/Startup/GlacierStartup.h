#pragma once

#include <Glacier/Glacier.h>


namespace Glacier
{
    void InitGlacierSystem(int hInstance, bool bEditorMode, char* pszGameName);
    void FreeGlacierSystem();
    int Glacier_Main(int hInstance, int hPrevInstance, const char* psCmdLine);
}

extern "C"
{
    GLACIER_API int RunEngine(bool bOnce);
}
