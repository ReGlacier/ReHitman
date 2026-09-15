#pragma once


namespace Glacier::SysInput
{
    enum EKeyState 
    {
        eNONE = 0,
        eLSHIFT = 1,
        eRSHIFT = 2,
        eSHIFT = 4,
        eLCTRL = 8,
        eRCTRL = 16,
        eCTRL = 32,
        eLALT = 64,
        eRALT = 128,
        eALT = 256,
        eDOWN = 512,
        eTAP = 1024,
        eANY = 2048,
    };
}