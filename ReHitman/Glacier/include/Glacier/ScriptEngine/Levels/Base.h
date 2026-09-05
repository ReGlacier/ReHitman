#pragma once


namespace Glacier
{
    enum _e03PhotoTouristType
    {
        e03PhotoTouristType_CAMERA_MAN = 0,
        e03PhotoTouristType_FEMALE = 1,
        e03PhotoTouristType_MALE = 2,
    };
    using e03PhotoTouristType = _e03PhotoTouristType;

    enum _eBarkType
    {
        eBarkType_RELAXED = 0,
        eBarkType_AGGRESIVE = 1,
        eBarkType_AGAINSTFENCE = 2,
    };
    using eBarkType = _eBarkType;

    enum _eDialogControl
    {
        eDialogControl_Master = 0,
        eDialogControl_Slave = 1,
    };
    using eDialogControl = _eDialogControl;

    enum _eDifficulty
    {
        eDifficulty_Rookie = 0,
        eDifficulty_Normal = 1,
        eDifficulty_Expert = 2,
        eDifficulty_Professional = 3,
    };
    using eDifficulty = _eDifficulty;

    // TODO: Continue export, search mask: "scriptc.cpp"
}