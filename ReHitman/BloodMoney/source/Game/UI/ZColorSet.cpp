#include <BloodMoney/Game/UI/ZColorSet.h>

namespace Hitman::BloodMoney {
    uint32_t ZColorSet::GetColor(EColorIndex colorIndex) {
        switch (colorIndex) {
            case EColorIndex::NormalColor:
                return m_NormalColor;
            case EColorIndex::DisableColor:
                return m_DisableColor;
            case EColorIndex::FocusColor:
                return m_FocusColor;
        }

        return 0u;
    }
}