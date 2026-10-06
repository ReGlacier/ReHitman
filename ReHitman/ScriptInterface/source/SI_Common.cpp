#include <SI/SI_Common.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItem.h>


namespace Glacier
{
    ZGEOM* GetGeom(ZREF rGeom)
    {
        ZGEOM* pGeom = ZGEOM::RefToPtr(rGeom);
        if (!pGeom)
        {
            ZERROR("ERROR: couldn't convert ref to pointer, in ZGeom.si, You probably called an interface function with a Null ref");
            return nullptr;
        }

        if (pGeom->IsDerivedFrom<ZGEOM>())
        {
            return pGeom;
        }

        ZERROR("ERROR: %s isn't derived from ZGeom", pGeom->Name());
        return nullptr;
    }

    ZLNKWHANDS* GetValidIKLnkObj(ZREF rObj)
    {
        ZGEOM* pObj = ZGEOM::RefToPtr(rObj);
        if (!pObj)
        {
            ZWARN2("WARNING: Unable to locate actor");
            return nullptr;
        }

        if (!pObj->IsDerivedFrom<ZLNKWHANDS>())
        {
            ZWARN2("WARNING: Object %s is not an ZGT_ZLNKWHANDS", pObj->Name());
            return nullptr;
        }

        return static_cast<ZLNKWHANDS*>(pObj);
    }

    ZLNKOBJ* GetValidLnkObj(ZREF rObj)
    {
        ZGEOM* pObj = ZGEOM::RefToPtr(rObj);
        if (!pObj)
        {
            ZWARN2("WARNING: Unable to locate lnkobj");
            return nullptr;
        }

        if (!pObj->IsDerivedFrom<ZLNKOBJ>())
        {
            ZWARN2("WARNING: Object %s is not an ZLNKOBJ", pObj->Name());
            return nullptr;
        }

        return static_cast<ZLNKOBJ*>(pObj);
    }

    ZItem* GetValidItem(ZREF rItem)
    {
        ZGEOM* pItem = ZGEOM::RefToPtr(rItem);
        if (!pItem)
        {
            ZWARN2("WARNING: GetValidItem -> Unable to locate object");
            return nullptr;
        }

        if (!pItem->IsDerivedFrom<ZItem>())
        {
            ZWARN2("WARNING: Object %s is not an ZItem", pItem->Name());
            return nullptr;
        }

        return static_cast<ZItem*>(pItem);
    }

    void SlipItemFromHand(ZLNKWHANDS* pActor, ZItem* pItem, bool a3)
    {
        // TODO: Finish me
    }
}
