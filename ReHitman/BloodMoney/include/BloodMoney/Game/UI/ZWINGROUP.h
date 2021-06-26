#pragma once

#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZWinEvents.h>

namespace Hitman::BloodMoney
{
    class ZWINDOWS;

    class ZWINGROUP : public Glacier::ZGROUP
    {
    public:
        //vftable
        virtual ZWINDOWS* GetSystem();
        virtual void WndMessage(Glacier::ZWMEVENT*);
        virtual void GetMouseColi(Glacier::SMouseColi*, float*, float*);
        virtual void RecalcMaxMin();
        virtual void CalcRealCenSize(float*, float*);
        virtual void SetText(const char*);
        virtual void SetAlpha(int);
        virtual void SetLineSpacing(int);
        virtual void SetField50(bool);

        //data (total size is 0x54, ZGROUP size is 0x4C)
        ZWINDOWS* m_pWinSystem;
        bool m_field50;
        bool m_field51; // ALIGN ONLY
        bool m_field52; // ALIGN ONLY
        bool m_field53; // ALIGN ONLY
    };
}