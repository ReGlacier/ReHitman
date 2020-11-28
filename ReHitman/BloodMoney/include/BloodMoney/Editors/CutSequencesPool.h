#pragma once

#include <BloodMoney/Game/ZCutSequencePlayer.h>

#include <functional>
#include <set>

namespace Hitman::BloodMoney
{
    class CutSequencesPool final
    {
        std::set<ZCutSequencePlayer*> m_cutSequencePlayers;
    public:
        static CutSequencesPool& GetInstance();

        void Add(ZCutSequencePlayer* instance);
        void Remove(ZCutSequencePlayer* instance);
        void ForEach(std::function<void(ZCutSequencePlayer*)> pred);

        bool IsEmpty() const;
    };
}