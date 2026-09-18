#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLnkAction.h>


namespace Glacier
{
    // fwds
    class ZItem;
    class ZLNKWHANDS;
    class ZIKHAND;
    namespace Animation { struct Header; }

    class ZActionPickupItem : public ZLnkAction
    {
    public:
        // vtbl
        ~ZActionPickupItem() override;

        bool Execute() override;
        bool CallBack() override;
        const char* Name() const override;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // methods
        ZActionPickupItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand, Animation::Header* pAnimPickupItem);
        void SetItem(ZItem* pItem);
        ZItem* GetItem() const;
        ZLNKWHANDS* GetActor() const;

        // members
        ZItem* m_pItem;
        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pHand;
        Animation::Header* m_pAnimPickupItem;
    };
    RE_VERIFY_SIZE(ZActionPickupItem, 0x18); // Verified PC alloc
}
