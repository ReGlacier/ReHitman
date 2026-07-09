#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier::Animation
{
    void ActiveAnimation::Create(ZLNKOBJ* pLnkObj)
    {
        m_fRemCallBackFrame = -1.0f;
        m_prtCallBacks = nullptr;
        m_pLnkObj = pLnkObj;
        sequenceId = 0;
    }
}