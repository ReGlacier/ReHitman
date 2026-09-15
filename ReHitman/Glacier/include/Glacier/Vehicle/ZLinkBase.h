#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Vehicle/PPart.h>


namespace Glacier
{
    class ZLinkBase : public ZLNKOBJ, public PPart
    {
    public:
        // vtbl (new methods)
        virtual void InitBoneIndexes();
        virtual void OnCameraLeave();
        virtual void OnDamage();
        virtual void LocalBoneUpdate();
        virtual void GlobalBoneUpdate();
    };
    RE_VERIFY_SIZE(ZLinkBase, 0x110); // Verified
}