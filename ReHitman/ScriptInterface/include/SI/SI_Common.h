#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    ZGEOM* GetGeom(ZREF rGeom);
    ZLNKWHANDS* GetValidIKLnkObj(ZREF rObj);
    ZLNKOBJ* GetValidLnkObj(ZREF rObj);
    ZItem* GetValidItem(ZREF rItem);
    void SlipItemFromHand(ZLNKWHANDS* pActor, ZItem* pItem, bool a3);
}
