#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLnkAction.h>

namespace Glacier
{
    class ZItem;
    class ZLNKWHANDS;
    class ZIKHAND;
    namespace Animation { struct Header; struct ActiveAnimation; }

    class ZActionGetItem : public ZLnkAction
    {
    public:
        ~ZActionGetItem() override;
        bool Execute() override;
        bool CallBack() override;
        bool AnimEnd(Animation::ActiveAnimation* pAnim) override;
        const char* Name() const override;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        ZActionGetItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
                       Animation::Header* pAnimGetItem, Animation::Header* pAnimGetWeapon,
                       bool bMirrorLHandAnims);
        void SetItem(ZItem* pItem);
        ZItem* GetItem() const;

        ZItem* m_pItem;
        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pHand;
        bool m_bMirrorLHandAnims;
        RE_ADD_PADDING(3);
        Animation::Header* m_pAnimGetItem;
        Animation::Header* m_pAnimGetWeapon;
        Animation::ActiveAnimation* m_pActiveAnim;
    };
    RE_VERIFY_SIZE(ZActionGetItem, 0x24);
}
