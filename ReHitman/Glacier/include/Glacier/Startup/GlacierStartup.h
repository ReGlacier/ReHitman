#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE(const char*, g_sProjectID);
    

    void InitGlacierSystem(int hInstance, bool bEditorMode, char* pszGameName);
    void FreeGlacierSystem();
    int Glacier_Main(int hInstance, int hPrevInstance, const char* psCmdLine);
    int Glacier_RunNormal(int hInstance, int hPrevInstance, const char* psCmdLine);
}

extern "C"
{
    GLACIER_API int RunEngine(bool bOnce);
}
