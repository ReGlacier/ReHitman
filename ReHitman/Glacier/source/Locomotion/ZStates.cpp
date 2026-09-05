#include <Glacier/Locomotion/ZStates.h>

#include <Glacier/Locomotion/ZDead.h>
#include <Glacier/Locomotion/ZFollowPath.h>
#include <Glacier/Locomotion/ZFullBody.h>
#include <Glacier/Locomotion/ZIdle.h>
#include <Glacier/Locomotion/ZSpecialLink.h>
#include <Glacier/Locomotion/ZWounded.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier::Locomotion
{
    ZFollowPath ZStates::m_FollowPath;
    ZSpecialLink ZStates::m_SpecialLink;
    ZFullBody ZStates::m_FullBody;
    ZDead ZStates::m_Dead;
    ZWounded ZStates::m_Wounded;
    ZIdle ZStates::m_Idle;

    ZState* ZStates::m_States[eMaxPathfinderActions] = {
        &m_FollowPath,
        &m_FullBody,
        &m_Dead,
        &m_Wounded,
        &m_SpecialLink,
        &m_Idle,
    };

    int ZStates::m_PathAction2Program[eMaxPathfinderActions] = {
        0, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    };

    ZState* ZStates::GetState(int iProgram)
    {
        ZASSERT(iProgram >= 0 && iProgram < eMaxPathfinderActions);
        return m_States[iProgram];
    }

    void ZStates::SetState(int iProgram, ZState* pState)
    {
        ZASSERT(iProgram >= 0 && iProgram < eMaxPathfinderActions);
        m_States[iProgram] = pState;
    }

    void ZStates::SetActionMap(int iPathAction, int iProgram)
    {
        ZASSERT(iPathAction >= 0 && iPathAction < eMaxPathfinderActions);
        m_PathAction2Program[iPathAction] = iProgram;
    }

    int ZStates::GetActionProgram(int iPathAction)
    {
        ZASSERT(iPathAction >= 0 && iPathAction < eMaxPathfinderActions);
        return m_PathAction2Program[iPathAction];
    }
}
