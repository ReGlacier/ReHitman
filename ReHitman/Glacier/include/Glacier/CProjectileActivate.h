#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier {
    struct CProjectileActivate {
        Glacier::ZREF m_unkREF0;
        int m_field4;
        float m_field8;
        int m_fieldC;
        bool m_field10;
        bool m_field11;
        bool m_field12;
        bool m_field13;
        Glacier::ZVector3 m_vTargetPosition;
        float m_fPrecision;
        int m_field24;
        int m_field28;
    };
}