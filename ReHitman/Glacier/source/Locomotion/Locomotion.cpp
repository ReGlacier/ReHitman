#include <Glacier/Locomotion/Locomotion.h>


namespace Glacier::Locomotion
{
    const char* GetProgramName(int lId)
    {
        switch (lId)
        {
            case -1: return "None";
            case 0: return "FollowPath";
            case 1: return "FullBody";
            case 2: return "Dead";
            case 3: return "Wounded";
            case 4: return "SpecialLink";
            case 5: return "Idle";
            case 6: return "FirstCustom";
            default:
                if (lId < 20 && lId >= -1)
                {
                    return "Custom";
                }
                return "OUT_OF_BOUNDS";
        }
    }
}
