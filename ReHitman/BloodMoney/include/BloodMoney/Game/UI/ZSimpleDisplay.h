#pragma once

#include <BloodMoney/Game/UI/CWinEvent.h>
#include <BloodMoney/Game/UI/ZWINGROUP.h>
#include <Glacier/ZSTL/zstring.h>

namespace Hitman::BloodMoney
{

    class ZSimpleDisplay : public CWinEvent<ZWINGROUP>
    {
    public:
        //vftable
        virtual bool AllClear();
        virtual void SetModified(bool);
        virtal void ChangeText(Glacier::zstring str, int, float);

        //data (total size is 0x80, base size is 0x30)
        int m_field30;
        int m_field34;
        Glacier::zstring m_text;
        int m_field44;
        float m_field48;
        int m_field4C;
        int m_field50;
        int m_field54;
        char m_field58;
        char field_59;
        char field_5A;
        char field_5B;
        Glacier::zstring m_field5C;
        int m_field68;
        int m_field6C;
        int m_field70;
        int m_field74;
        int m_field78;
        bool m_field7C;
        bool m_field7D;
        bool m_field7E;
        bool m_field7F;
    };
}