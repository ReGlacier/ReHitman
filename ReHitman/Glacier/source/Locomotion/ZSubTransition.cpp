#include <Glacier/Locomotion/ZSubTransition.h>


namespace Glacier::Locomotion
{
    ZSubTransition::ZSubTransition()
        : m_StartFramePct(0.0f)
        , m_EndFramePct(1.0f)
        , m_Mirrored(false)
        , m_BlendFrames(-1.0f)
    {
    }
}
