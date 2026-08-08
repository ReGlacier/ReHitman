#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Friskbox__Addguardtofriskbox(ZREF rFriskbox, ZREF rGuard);
    void Friskbox__Removeguardfromfriskbox(ZREF rFriskbox, ZREF rGuard);
    void Friskbox__Setenabled(ZREF rFriskbox, bool enabled);
    char Friskbox__Isenabled(ZREF rFriskbox);
    char Friskbox__Playfriskokanimmirrored(ZREF rFriskbox, ZREF rActor);
    void Friskbox__Addsuitcase(ZREF rFriskbox, ZREF rSuitcase);
    void Friskbox__Removesuitcase(ZREF rFriskbox, ZREF rSuitcase);
    char Friskbox__Ishitmancarryingconfsuitcase(ZREF rFriskbox);
    void Friskbox__Setsuitcaseplaced(ZREF rFriskbox, ZREF rSuitcase);
    char Friskbox__Issurrendweaponenabled(ZREF rFriskbox);
    char Friskbox__Canpickupweaponsitems(ZREF rFriskbox);
    void Friskbox__Sethitmanssuitcasechecked(ZREF rFriskbox, bool checked);
}
