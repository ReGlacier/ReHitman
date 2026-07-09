#include <BloodMoney/Game/UI/ZColorSet.h>

namespace Hitman::BloodMoney 
{
    uint32_t ZColorSet::GetColor(EColorIndex colorIndex) 
    {
        return m_aiColors[colorIndex];
    }
}