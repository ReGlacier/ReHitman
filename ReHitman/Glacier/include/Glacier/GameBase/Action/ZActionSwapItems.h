#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLnkAction.h>

namespace Glacier
{
    class ZLNKWHANDS;
    class ZIKHAND;
    namespace Animation { struct Header; }

    class ZActionSwapItems : public ZLnkAction
    {
    public:
        ~ZActionSwapItems() override;
        bool Execute() override;
        bool CallBack() override;
        const char* Name() const override;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        ZActionSwapItems(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pRHand,
                         ZIKHAND* pLHand, Animation::Header* pAnimSwapItems);
        ZIKHAND* GetLHand() const;

        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pRHand;
        ZIKHAND* m_pLHand;
        Animation::Header* m_pAnimSwapItems;
    };
    RE_VERIFY_SIZE(ZActionSwapItems, 0x18);
}
