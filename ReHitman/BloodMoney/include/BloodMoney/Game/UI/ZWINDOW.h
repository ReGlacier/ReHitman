#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>

#include <BloodMoney/Game/UI/UIFWD.h>
#include <BloodMoney/Game/UI/ZWINGROUP.h>

namespace Hitman::BloodMoney
{
    class ZWINDOW : public ZWINGROUP
    {
    public:
        //vftable
        virtual void OnWindowOpen(unsigned int, bool);
        virtual void OnWindowClose(unsigned int, bool);
        virtual ZWINGROUP* GetDefaultFocus();
        virtual void SetDefaultFocus(ZWINGROUP* winGroup);

        //data (total size is 0x5C, ZWINGROUP size is 0x54)
        int m_field54;
        int m_field58;
    };
}