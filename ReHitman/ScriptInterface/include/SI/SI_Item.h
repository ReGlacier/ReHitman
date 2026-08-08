#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    bool Item__Attachrhand(ZREF rActor, ZREF rItem);
    bool Item__Attachlhand(ZREF rActor, ZREF rItem);
    bool Item__Detachrhand(ZREF rActor, ZREF rItem);
    bool Item__Detachlhand(ZREF rActor, ZREF rItem);
    void Item__Resetedibleitem(ZREF rItem, v3 position);
    ZREF Item__Getlhanditem(ZREF rActor);
    ZREF Item__Getrhanditem(ZREF rActor);
    void Item__Enablepickup(ZREF rItem, int enabled);
    bool Item__Setowner(ZREF rOwner, ZREF rItem);
    bool Item__Setownerparent(ZREF rParent, ZREF rOwner, ZREF rItem);
    ZREF Item__Getparent(ZREF rItem);
    ZREF Item__Getowner(ZREF rItem);
    void Item__Setitemstate(ZREF rItem, int state);
    int Item__Getitemstate(ZREF rItem);
    bool Item__Isitemhidden(ZREF rItem);
    bool Item__Isitemactive(ZREF rItem);
    ZREF Item__Sliprighthanditem(ZREF rActor);
    ZREF Item__Sliprighthanditemnoevent(ZREF rActor);
    ZREF Item__Sliplefthanditem(ZREF rActor);
    ZREF Item__Sliplefthanditemnoevent(ZREF rActor);
    bool Item__Isitemcontainer(ZREF rItem);
    void Item__Insertitemincontainer(ZREF rContainer, ZREF rItem);
    void Item__Removeitemfromcontainer(ZREF rContainer, ZREF rItem);
    ZREF Item__Putlhitemtopos(ZREF rActor, v3 position);
    ZREF Item__Putrhitemtopos(ZREF rActor, v3 position);
    bool Item__Containweapons(ZREF rItem);
    bool Item__Containmetallicweapons(ZREF rItem);
    bool Item__Iscontainerisempty(ZREF rItem);
    void Item__Setvisibletonpcs(ZREF rItem, bool bVisible);
    bool Item__Iscontainercontainsitem(ZREF rContainer, ZREF rItem);
    ZREF Item__Getitemincontainer(ZREF rContainer, const char* itemName);
    void Item__Clearfooddrinkinfection(ZREF rItem);
    anim Item__Getanim(ZREF rItem, const char* name);
    int Item__Playanimsegment(ZREF rItem, anim animId, int segment, float start, float end, float blendTime);
    bool Item__Useitemactivateanimation(ZREF rItem);
    void Item__Setforceunpickablebyhm(ZREF rItem, int forceUnpickable);
    bool Item__Getforceunpickablebyhm(ZREF rItem);
    void Item__Hideitem(ZREF rItem);
    void Item__Showitem(ZREF rItem);
    bool Item__Isitemlarge(ZREF rItem);
    bool Item__Isitemcigarette(ZREF rItem);
    bool Item__Hastemplate(ZREF rTemplate, ZREF rItem);
    bool Item__Isitemontopofelevator(ZREF rItem);
    void Item__Setscale(ZREF rItem, float scale);
}
