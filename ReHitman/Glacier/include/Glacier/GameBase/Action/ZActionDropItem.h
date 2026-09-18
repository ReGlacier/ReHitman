#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLnkAction.h>

namespace Glacier
{
    class ZLNKWHANDS;
    class ZIKHAND;
    namespace Animation { struct Header; }

    class ZActionDropItem : public ZLnkAction
    {
    public:
        ~ZActionDropItem() override;
        bool Execute() override;
        bool CallBack() override;
        const char* Name() const override;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        ZActionDropItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand, Animation::Header* pAnimDropItem);
        ZIKHAND* GetHand() const;

        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pHand;
        Animation::Header* m_pAnimDropItem;
    };
    RE_VERIFY_SIZE(ZActionDropItem, 0x14);
}
