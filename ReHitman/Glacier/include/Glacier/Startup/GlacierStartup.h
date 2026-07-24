#pragma once


namespace Glacier
{
    // fwds
    struct ZComponentGlobalList;

    void InitGlacierSystem(ZComponentGlobalList* pComponents, bool, char* pszGameName);
    void FreeGlacierSystem();
}