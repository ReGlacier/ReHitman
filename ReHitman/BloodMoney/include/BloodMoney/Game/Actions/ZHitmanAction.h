#pragma once

#include <Glacier/EventBase/ZLnkAction.h>

namespace Hitman::BloodMoney
{
    class ZHitman3;

    class ZHitmanAction : public Glacier::ZLnkAction
    {
    public:
        // vftable
        virtual void AddCallBack(float, int); //Animation::ActiveAnimation::ECallType
        virtual void AddCallBack(int, int); //ZHitmanAction::AddCallBack(MetaKey::Value,Animation::ActiveAnimation::ECallType)
        virtual void AddCallBack(const char*, int); // ZHitmanAction::AddCallBack(char const*,Animation::ActiveAnimation::ECallType)
        virtual void PlayAnim(Glacier::Animation::Header* anim, float, float);

        // data
        ZHitman3* m_player;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
    };
}