#pragma once


namespace Glacier
{
    enum _SCRIPTIMPORTTYPE
    {
        SIT_BYTE = 0,
        SIT_SHORT = 1,
        SIT_LONG = 2,
        SIT_FLOAT = 3,
        SIT_STRING = 4,
        SIT_REF = 5,
        SIT_END = 6
    };

    using SCRIPTIMPORTTYPE = _SCRIPTIMPORTTYPE;
}