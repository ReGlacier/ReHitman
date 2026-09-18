#pragma once

#include <Glacier/ReGlacier.h>

namespace Hitman
{
    struct SDifficultySettings
    {
        //total size is 0xA4
        float fstanddowntime;
        float ffollowtrailtime;
        float fhunttargettracktime;
        float fseethroughdisguisedistance;
        float fseethroughdisguisespeed;
        float fforgetdisguisespeed;
        float frunsuspiciousrange;
        float faccuracymodifier;
        float fburstlength;
        float frofmultiplier;
        float faimspeed;
        float fdrawspeedmultiplier;
        float fshothearingrange;
        float fscreamhearingrange;
        float fcheckfootstepsbehindrange;
        float factorrelaxedvisionrange;
        float factorrelaxedfov;
        float factoralertvisionrange;
        float factoralertfov;
        float ffallingdamagemultiplier;
        float fusecoverpercentage;
        float fbleedtodeathtime;
        float fciviliansattackhmpercentage;
        float fcivilianhidetime;
        float flockpicktimemultiplier;
        float ffriskintrusiondecaytime;
        float fsmallweaponvisiblemultiplierciv;
        float fsmallweaponvisiblemultipliergrd;
        int ihitmanhitpoints;
        int iguardhitpoints;
        int icivilianhitpoints;
        int itargethitpoints;
        bool bautoaim;
        bool bheadshotsagainsthitman;
        float fmoneymultiplier;
        float fmaxmoneymultiplier;
        float fcleanupprice;
        float finnocentbribeprice;
        float fpolicebribeprice;
        float fnewidentityprice;
        float fweaponupgradeprice;
        float fweaponretrieveprice;
    };
    RE_VERIFY_SIZE(SDifficultySettings, 0xA4);
}
