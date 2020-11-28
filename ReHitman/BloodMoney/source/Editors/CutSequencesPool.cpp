#include <BloodMoney/Editors/CutSequencesPool.h>

namespace Hitman::BloodMoney
{
    CutSequencesPool& CutSequencesPool::GetInstance()
    {
        static CutSequencesPool instance;
        return instance;
    }

    void CutSequencesPool::Add(ZCutSequencePlayer* instance)
    {
        if (!instance) return;
        m_cutSequencePlayers.insert(instance);
    }

    void CutSequencesPool::Remove(ZCutSequencePlayer* instance)
    {
        if (!instance) return;
        m_cutSequencePlayers.erase(instance);
    }

    void CutSequencesPool::ForEach(std::function<void(ZCutSequencePlayer*)> pred)
    {
        if (!pred) return;
        for (const auto& item : m_cutSequencePlayers)
            pred(item);
    }

    bool CutSequencesPool::IsEmpty() const
    {
        return m_cutSequencePlayers.empty();
    }
}