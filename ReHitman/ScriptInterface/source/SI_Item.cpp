#include <SI/SI_Item.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItem.h>


namespace Glacier
{
    namespace
    {
        ZLNKWHANDS* GetValidIKLnkObj(ZREF rObj)
        {
            // TODO: Finish me
            return nullptr;
        }

        ZItem* GetValidItem(ZREF rItem)
        {
            // TODO: Finish me
            return nullptr;
        }
    }

    bool Item__Attachrhand(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Attachlhand(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Detachrhand(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Detachlhand(ZREF rActor, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Item__Resetedibleitem(ZREF rItem, v3 position)
    {
        // TODO: Finish me
    }

    ZREF Item__Getlhanditem(ZREF rActor)
    {
        auto* p = GetValidIKLnkObj(rActor);
        if (!p) return 0;

        auto* pItem = p->GetLHandItem();
        return pItem ? pItem->GetRef() : 0;
    }

    ZREF Item__Getrhanditem(ZREF rActor)
    {
        auto* p = GetValidIKLnkObj(rActor);
        if (!p) return 0;

        auto* pItem = p->GetRHandItem();
        return pItem ? pItem->GetRef() : 0;
    }

    void Item__Enablepickup(ZREF rItem, int enabled)
    {
        // TODO: Finish me
    }

    bool Item__Setowner(ZREF rOwner, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Setownerparent(ZREF rParent, ZREF rOwner, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Item__Getparent(ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Item__Getowner(ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    void Item__Setitemstate(ZREF rItem, int state)
    {
        // TODO: Finish me
    }

    int Item__Getitemstate(ZREF rItem)
    {
        // TODO: Finish me
        return 0;
    }

    bool Item__Isitemhidden(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Isitemactive(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Item__Sliprighthanditem(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Item__Sliprighthanditemnoevent(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Item__Sliplefthanditem(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Item__Sliplefthanditemnoevent(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    bool Item__Isitemcontainer(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Item__Insertitemincontainer(ZREF rContainer, ZREF rItem)
    {
        // TODO: Finish me
    }

    void Item__Removeitemfromcontainer(ZREF rContainer, ZREF rItem)
    {
        // TODO: Finish me
    }

    ZREF Item__Putlhitemtopos(ZREF rActor, v3 position)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Item__Putrhitemtopos(ZREF rActor, v3 position)
    {
        // TODO: Finish me
        return 0;
    }

    bool Item__Containweapons(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Containmetallicweapons(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Iscontainerisempty(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Item__Setvisibletonpcs(ZREF rItem, bool bVisible)
    {
        // TODO: Finish me
    }

    bool Item__Iscontainercontainsitem(ZREF rContainer, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Item__Getitemincontainer(ZREF rContainer, const char* itemName)
    {
        // TODO: Finish me
        return 0;
    }

    void Item__Clearfooddrinkinfection(ZREF rItem)
    {
        // TODO: Finish me
    }

    anim Item__Getanim(ZREF rItem, const char* name)
    {
        // TODO: Finish me
        return 0;
    }

    int Item__Playanimsegment(ZREF rItem, anim animId, int segment, float start, float end, float blendTime)
    {
        // TODO: Finish me
        return 0;
    }

    bool Item__Useitemactivateanimation(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Item__Setforceunpickablebyhm(ZREF rItem, int forceUnpickable)
    {
        // TODO: Finish me
    }

    bool Item__Getforceunpickablebyhm(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Item__Hideitem(ZREF rItem)
    {
        // TODO: Finish me
    }

    void Item__Showitem(ZREF rItem)
    {
        // TODO: Finish me
    }

    bool Item__Isitemlarge(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Isitemcigarette(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Hastemplate(ZREF rTemplate, ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    bool Item__Isitemontopofelevator(ZREF rItem)
    {
        // TODO: Finish me
        return false;
    }

    void Item__Setscale(ZREF rItem, float scale)
    {
        // TODO: Finish me
    }
}
