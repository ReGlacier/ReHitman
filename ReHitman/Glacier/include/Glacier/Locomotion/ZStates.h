#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier::Locomotion
{
    class ZState;
    class ZFollowPath;
    class ZSpecialLink;
    class ZFullBody;
    class ZDead;
    class ZWounded;
    class ZIdle;

    class ZStates
    {
    public:
        static constexpr int eMaxPathfinderActions = 0x14;

        static ZState* GetState(int iProgram);
        static void SetState(int iProgram, ZState* pState);
        static void SetActionMap(int iPathAction, int iProgram);
        static int GetActionProgram(int iPathAction);

        static ZFollowPath m_FollowPath;
        static ZSpecialLink m_SpecialLink;
        static ZFullBody m_FullBody;
        static ZDead m_Dead;
        static ZWounded m_Wounded;
        static ZIdle m_Idle;
        static ZState* m_States[eMaxPathfinderActions];
        static int m_PathAction2Program[eMaxPathfinderActions];
    };
}
