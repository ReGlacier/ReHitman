#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier
{
    ZPoseModel::ZPoseModel()
    {
        ResetIndex();
    }

    void ZPoseModel::ResetIndex()
    {
        SetIndex(0);
        SetActive(true);
    }

    uint16_t ZPoseModel::GetIndex()
    {
        return m_dwIndex;
    }

    void ZPoseModel::SetIndex(uint16_t index)
    {
        m_dwIndex = index;
    }

    float ZPoseModel::GetWeight(float fWeight)
    {
        constexpr uint16_t INTERPOLATION_LINEAR = 0;
        constexpr uint16_t INTERPOLATION_QUADRATIC_BEZIER = 1;
        constexpr uint16_t INTERPOLATION_CUBIC_CATMULL_ROM = 2;
        constexpr float POSE_KEY_STRENGTH_TO_WEIGHT = 1.0f / 256.0f;

        const float frame = fWeight * 25.0f;
        float weight = 0.0f;

        m_dwIndex = 0;

        while (m_dwIndex <= m_dwSize)
        {
            if (m_dwSize == 1)
                return static_cast<float>(m_pPoseKeys[m_dwIndex].Strength) * POSE_KEY_STRENGTH_TO_WEIGHT;

            if (m_dwIndex == m_dwSize)
            {
                weight = static_cast<float>(m_pPoseKeys[m_dwIndex].Strength) * POSE_KEY_STRENGTH_TO_WEIGHT;
                break;
            }

            const uint16_t interpolationType = (m_dwSize - m_dwIndex >= 4 && m_dwIndex)
                ? INTERPOLATION_CUBIC_CATMULL_ROM
                : ((m_dwSize - m_dwIndex >= 3) ? INTERPOLATION_QUADRATIC_BEZIER : INTERPOLATION_LINEAR);
            SPoseKey* pKey = &m_pPoseKeys[m_dwIndex];
            const uint16_t firstFrame = pKey[0].Frame;
            const uint16_t secondFrame = pKey[1].Frame;

            if (static_cast<float>(firstFrame) <= frame && frame <= static_cast<float>(secondFrame))
            {
                if (secondFrame == firstFrame)
                    return static_cast<float>(m_pPoseKeys[m_dwIndex + 1].Strength) * POSE_KEY_STRENGTH_TO_WEIGHT;

                switch (interpolationType)
                {
                case INTERPOLATION_CUBIC_CATMULL_ROM:
                {
                    const float frameDelta = static_cast<float>(pKey[1].Frame - pKey[0].Frame);
                    float t = frame - static_cast<float>(pKey[0].Frame);

                    if (frameDelta != 0.0f)
                        t /= frameDelta;

                    const float previousStrength = static_cast<float>(m_pPoseKeys[m_dwIndex - 1].Strength);
                    const float currentStrength = static_cast<float>(pKey[0].Strength);
                    const float nextStrength = static_cast<float>(m_pPoseKeys[m_dwIndex + 1].Strength);
                    const float nextNextStrength = static_cast<float>(m_pPoseKeys[m_dwIndex + 2].Strength);
                    const float t2 = t * t;
                    const float t3 = t2 * t;

                    weight = ((currentStrength + currentStrength) + ((nextStrength - previousStrength) * t)
                        + (((previousStrength + previousStrength) - (currentStrength * 5.0f) + (nextStrength * 4.0f) - nextNextStrength) * t2)
                        + (((currentStrength * 3.0f) - previousStrength - (nextStrength * 3.0f) + nextNextStrength) * t3)) * 0.5f;
                    break;
                }
                case INTERPOLATION_QUADRATIC_BEZIER:
                {
                    const float frameDelta = static_cast<float>(m_pPoseKeys[m_dwIndex + 2].Frame - pKey[0].Frame);
                    float t = frame - static_cast<float>(pKey[0].Frame);

                    if (frameDelta != 0.0f)
                        t /= frameDelta;

                    weight = (static_cast<float>(pKey[0].Strength) * ((1.0f - t) * (1.0f - t)))
                        + (((static_cast<float>(pKey[1].Strength) + static_cast<float>(pKey[1].Strength)) * (1.0f - t)) * t)
                        + (static_cast<float>(m_pPoseKeys[m_dwIndex + 2].Strength) * (t * t));
                    break;
                }
                case INTERPOLATION_LINEAR:
                    if (secondFrame != firstFrame)
                    {
                        weight = static_cast<float>(pKey[0].Strength)
                            + (((static_cast<float>(m_pPoseKeys[m_dwIndex + 1].Strength) - static_cast<float>(pKey[0].Strength)) / static_cast<float>(secondFrame - firstFrame))
                                * (frame - static_cast<float>(firstFrame)));
                    }
                    break;
                }

                weight *= POSE_KEY_STRENGTH_TO_WEIGHT;
                break;
            }

            ++m_dwIndex;
        }

        if (m_dwIndex >= m_dwSize)
            SetActive(false);

        return weight;
    }
    
    void ZPoseModel::SetName(char cName)
    {
        m_cName = cName;
    }

    void ZPoseModel::SetActive(bool bIsActive)
    {
        m_bActive = bIsActive;
    }

    void ZPoseModel::SetKeys(SPoseKey* pKey)
    {
        m_pPoseKeys = pKey;
    }
    
    void ZPoseModel::SetSize(uint16_t size)
    {
        m_dwSize = size;
    }

    uint16_t ZPoseModel::Size() const
    {
        return m_dwSize;
    }

    char ZPoseModel::Name() const
    {
        return m_cName;
    }

    bool ZPoseModel::Active() const
    {
        return m_bActive;
    }
}
