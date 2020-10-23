#pragma once

namespace Hitman::BloodMoney
{
    class ZWINDOWS;

    namespace Action
    {
        struct ZHandle
        {
            const char *hotkey;
            int m_field4;
            int m_field8;
        };
    }

    class ZWinInput
    {
    public:
        ZWINDOWS* m_parent;
        Action::ZHandle m_downAction;
        Action::ZHandle m_upAction;
        Action::ZHandle m_leftAction;
        Action::ZHandle m_rightAction;
        Action::ZHandle m_selectAction;
        Action::ZHandle m_select2Action;
        Action::ZHandle m_select3Action;
        Action::ZHandle m_select4Action;
        Action::ZHandle m_cancelAction;
        Action::ZHandle m_misc1Action;
        Action::ZHandle m_mouseButtonAction;
        Action::ZHandle m_leftMouseButtonAction;
        Action::ZHandle m_rightMouseButtonAction;
        Action::ZHandle m_mouseWheelUpAction;
        Action::ZHandle m_mouseWheelDownAction;
        Action::ZHandle m_startAction;
        Action::ZHandle m_z1Action;
        Action::ZHandle m_r1Action;
        int field_dc;
        int field_e0;
        int field_e4;
        int field_e8;
        int field_ec;
        int field_f0;
        int field_f4;
        int field_f8;
        int field_fc;
        int field_100;
        int field_104;
        int field_108;
        int field_10c;
        int field_110;
        int field_114;
        int field_118;
        int field_11c;
        int field_120;
        int field_124;
        int field_128;
        int field_12c;
        int field_130;
        int field_134;
        int field_138;
        int field_13c;
        int field_140;
        int field_144;
        int field_148;
        int field_14c;
        int field_150;
        int field_154;
        int field_158;
        int field_15c;
        int field_160;
        int field_164;
        int field_168;
        int field_16c;
    };
}