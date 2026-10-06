#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLnkAction.h>

namespace Glacier
{
    class ZLNKWHANDS;
    class ZIKHAND;
    namespace Animation { struct Header; struct ActiveAnimation; }

    class ZActionPutItem : public ZLnkAction
    {
    public:
        ~ZActionPutItem() override;
        bool Execute() override;
        bool CallBack() override;
        bool Update() override;
        const char* Name() const override;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        ZActionPutItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
                       Animation::Header* pAnimPutItem, Animation::Header* pAnimPutWeapon,
                       bool bMirrorLHandAnims);
        ZIKHAND* GetHand() const;

        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pHand;
        Animation::Header* m_pAnimPutItem;
        Animation::Header* m_pAnimPutWeapon;
        bool m_bMirrorLHandAnims;
        RE_ADD_PADDING(3);
        float m_fHideItemFrame;
        Animation::ActiveAnimation* m_pBoneAnim;
    };
    RE_VERIFY_SIZE(ZActionPutItem, 0x24);
}
