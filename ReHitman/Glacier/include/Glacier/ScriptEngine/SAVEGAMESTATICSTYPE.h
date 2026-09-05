#pragma once


namespace Glacier
{
    enum _SAVEGAMESTATICSTYPE 
    {
        SGST_RAW = 0,
        SGST_STRING = 1,
        SGST_THREAD = 2,
        SGST_STATEREF = 3,
        SGST_EVENTREF = 4,
        SGST_END = 5
    };

    using SAVEGAMESTATICSTYPE = _SAVEGAMESTATICSTYPE;
}