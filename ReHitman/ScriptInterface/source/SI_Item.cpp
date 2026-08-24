#include <SI/SI_Item.h>
#include <SI/SI_Common.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItem.h>


namespace Glacier
{
    bool Item__Attachrhand(ZREF rActor, ZREF rItem)
    {
        auto* pActor = GetValidIKLnkObj(rActor);
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.AttachRHand pItem is NULL");
            return false;
        }

        if (!pActor)
        {
            ZWARN2("item.AttachRHand pActor is NULL");
            return false;
        }

        auto* pOwner = static_cast<ZGROUP*>(pItem->GetItemOwner());
        if (pOwner != pItem->BaseGeom()->ParentGroup() && pOwner != static_cast<ZGEOM*>(pActor))
        {
            ZWARN2("AttachRHand: %s is not the owner of %s", pActor->Name(), pItem->Name());
            return false;
        }

        pItem->Hide(false);
        pItem->MakeActive();
        pActor->ForceRHandItem(pItem, false);

        return true;
    }

    bool Item__Attachlhand(ZREF rActor, ZREF rItem)
    {
        auto* pActor = GetValidIKLnkObj(rActor);
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.AttachLHand pItem is NULL");
            return false;
        }

        if (!pActor)
        {
            ZWARN2("item.AttachLHand pActor is NULL");
            return false;
        }

        auto* pOwner = static_cast<ZGROUP*>(pItem->GetItemOwner());
        if (pOwner != pItem->BaseGeom()->ParentGroup() && pOwner != static_cast<ZGEOM*>(pActor))
        {
            ZWARN2("AttachLHand: %s is not the owner of %s", pActor->Name(), pItem->Name());
            return false;
        }

        pItem->Hide(false);
        pItem->MakeActive();
        pActor->ForceLHandItem(pItem, false);

        return true;
    }

    bool Item__Detachrhand(ZREF rActor, ZREF rItem)
    {
        auto* pActor = GetValidIKLnkObj(rActor);
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.DetachRHand pItem is NULL");
            return false;
        }

        if (!pActor)
        {
            ZWARN2("item.DetachRHand pActor is NULL");
            return false;
        }

        auto* pOwner = static_cast<ZGROUP*>(pItem->GetItemOwner());
        if (pOwner != pItem->BaseGeom()->ParentGroup() && pOwner != static_cast<ZGEOM*>(pActor))
        {
            ZWARN2("DetachRHand: %s is not the owner of %s", pActor->Name(), pItem->Name());
            return false;
        }

        pActor->AttachRHandItem(0);

        return true;
    }

    bool Item__Detachlhand(ZREF rActor, ZREF rItem)
    {
        auto* pActor = GetValidIKLnkObj(rActor);
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.DetachLHand pItem is NULL");
            return false;
        }

        if (!pActor)
        {
            ZWARN2("item.DetachLHand pActor is NULL");
            return false;
        }

        auto* pOwner = static_cast<ZGROUP*>(pItem->GetItemOwner());
        if (pOwner != pItem->BaseGeom()->ParentGroup() && pOwner != static_cast<ZGEOM*>(pActor))
        {
            ZWARN2("DetachLHand: %s is not the owner of %s", pActor->Name(), pItem->Name());
            return false;
        }

        pActor->AttachLHandItem(0);

        return true;
    }

    void Item__Resetedibleitem(ZREF rItem, v3 position)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            return;
        }

        auto* pOwnerGeom = pItem->GetItemOwner();
        if (pOwnerGeom)
        {
            return;
        }

        if (auto* pOwner = geom_cast<ZLNKWHANDS>(pOwnerGeom))
        {
            if (auto* pRHandItem = pOwner->GetRHandItem(); pRHandItem == pItem)
            {
                pOwner->AttachRHandItem(0);
            }
            else if (auto* pLHandItem = pOwner->GetLHandItem(); pLHandItem == pItem)
            {
                pOwner->AttachLHandItem(0);
            }

            pItem->SetItemOwner(0, nullptr, true, true);
            pItem->SetWorldPosition(position);

            // TODO: Finish me when ZHM3Item::RestoreBites will be reversed!
        }
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

    void Item__Enablepickup(ZREF rItem, bool bEnabled)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.EnablePickup pItem is NULL");
            return;
        }

        pItem->EnablePickup(bEnabled);
    }

    bool Item__Setowner(ZREF rOwner, ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.SetOwner pItem is NULL");
            return false;
        }

        auto* pOwner = geom_cast<ZGROUP>(GetGeom(rOwner));
        if (!pOwner)
        {
            ZWARN2("item.SetOwner pOwner is NULL");
            return false;
        }

        pItem->SetItemOwner(0, pOwner, true, true);
        return true;
    }

    bool Item__Setownerparent(ZREF rParent, ZREF rOwner, ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.SetOwner pItem is NULL");
            return false;
        }

        auto* pOwner = geom_cast<ZGROUP>(GetGeom(rOwner));
        if (!pOwner)
        {
            ZWARN2("item.SetOwner pOwner is NULL");
            return false;
        }

        // TODO: Finish me
        return false;
    }

    ZREF Item__Getparent(ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.GetParent pItem is NULL");
            return 0;
        }

        return pItem->BaseGeom()->ParentGroup()->GetRef();
    }

    ZREF Item__Getowner(ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.GetOwner pItem is NULL");
            return 0;
        }

        return pItem->GetItemOwner()->GetRef();
    }

    void Item__Setitemstate(ZREF rItem, int state)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.SetItemState pItem is NULL");
            return;
        }

        pItem->SetState(static_cast<ITEMSTATE>(state), nullptr);
    }

    int Item__Getitemstate(ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.GetItemState pItem is NULL");
            return 0;
        }

        return pItem->GetState();
    }

    bool Item__Isitemhidden(ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.IsItemHidden pItem is NULL");
            return false;
        }

        return (pItem->BaseGeom()->Control() & ZCHIDDEN) != 0;
    }

    bool Item__Isitemactive(ZREF rItem)
    {
        auto* pItem = GetValidItem(rItem);

        if (!pItem)
        {
            ZWARN2("item.IsItemActive pItem is NULL");
            return false;
        }

        return (pItem->BaseGeom()->Control() & ZCINACTIVE) == 0;
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
