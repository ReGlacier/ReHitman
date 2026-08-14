#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZPoseAnim::ZPoseAnim() : ZPoseAnim(0u) {}
    
    ZPoseAnim::ZPoseAnim(uint32_t rHost)
    {
        m_rHost = rHost;
        m_pPoseList = nullptr;
        m_fTimeMultiplier = 1.0f;
        m_iUpdate = 0;
        m_lEmotionID = 0;
    }

    ZPoseAnim::~ZPoseAnim()
    {
        if (m_pPoseList)
        {
            ZUniMemory::Free(m_pPoseList);
            m_pPoseList = nullptr;
        }
    }
    
    void ZPoseAnim::SetID(uint32_t lId)
    {
        m_dwID = lId;
    }

    uint32_t ZPoseAnim::GetID() const
    {
        return m_dwID;
    }

    int8_t ZPoseAnim::GetExpressionID() const
    {
        if (m_lEmotionID)
        {
            return static_cast<int8_t>(m_lEmotionID - 0x64);
        }

        return -1;
    }

    uint8_t ZPoseAnim::Count() const
    {
        return m_lCount;
    }

    uint8_t ZPoseAnim::StartCount() const
    {
        return m_lStartCount;
    }

    float ZPoseAnim::StartTime() const
    {
        return m_fStartTime;
    }
    
    float ZPoseAnim::TimeMultiplier() const
    {
        return m_fTimeMultiplier;
    }

    float ZPoseAnim::EndTime() const
    {
        return m_fEndTime;
    }
    
    float ZPoseAnim::FrameTime() const
    {
        return m_fFrameTime;
    }

    float ZPoseAnim::FrameStartTime() const
    {
        return m_fFrameStartTime;
    }

    uint8_t ZPoseAnim::ActivePoses() const
    {
        return m_lActivePoses;
    }

    uint32_t ZPoseAnim::PoseSize() const
    {
        return m_lPoseSize;
    }

    void ZPoseAnim::SetStartTime(float fStartTime)
    {
        m_fStartTime = fStartTime;
    }

    void ZPoseAnim::SetTimeMultiplier(float fTimeMul)
    {
        m_fTimeMultiplier = fTimeMul;
    }

    void ZPoseAnim::SetEndTime(float fEndTime)
    {
        m_fEndTime = fEndTime;
    }

    void ZPoseAnim::SetCount(int lCount)
    {
        m_lCount = lCount;
    }

    void ZPoseAnim::SetStartCount(int lStartCount)
    {
        m_lStartCount = lStartCount;
    }

    void ZPoseAnim::SetActivePoses(uint8_t activePoses)
    {
        m_lActivePoses = activePoses;
    }

    void ZPoseAnim::SetPoseSize(uint32_t lPosesSize)
    {
        m_lPoseSize = lPosesSize;
    }

    void ZPoseAnim::SetFrameStartTime(float fFrameStartTime)
    {
        m_fFrameStartTime = fFrameStartTime;
    }

    void ZPoseAnim::SetFrameTime(float fFrameTime)
    {
        m_fFrameTime = fFrameTime;
    }

    void ZPoseAnim::SetEmotionID(uint16_t emotionId)
    {
        m_lEmotionID = emotionId;
    }

    void ZPoseAnim::SetHost(uint32_t rHost)
    {
        m_rHost = rHost;
    }

    uint32_t ZPoseAnim::GetHost() const
    {
        return m_rHost;
    }

    bool ZPoseAnim::Create(char* pBuffer)
    {
        char* pCurrent = pBuffer;

        SetID(*reinterpret_cast<uint32_t*>(pCurrent));
        pCurrent += sizeof(uint32_t);

        const uint32_t poseDataSize = *reinterpret_cast<uint32_t*>(pCurrent);
        pCurrent += sizeof(uint32_t);

        const uint16_t count = *reinterpret_cast<uint16_t*>(pCurrent);
        pCurrent += sizeof(uint16_t);

        const float frameTime = *reinterpret_cast<float*>(pCurrent);
        pCurrent += sizeof(float);

        const uint16_t emotionId = *reinterpret_cast<uint16_t*>(pCurrent);
        pCurrent += sizeof(uint16_t);

        SetStartCount(0);
        SetCount(count);
        SetActivePoses(static_cast<uint8_t>(count));
        SetFrameTime(frameTime);
        SetEmotionID(emotionId);
        SetPoseSize(poseDataSize + 0x10);

        m_pPoseList = static_cast<ZPoseModel*>(ZUniMemory::Allocate(sizeof(ZPoseModel) * count));

        ZPoseModel* pPoseModel = m_pPoseList;
        for (int i = static_cast<int>(count) - 1; i != -1; --i)
        {
            znew_placement(pPoseModel++);
        }

        UpdateData(pCurrent);
        return true;
    }

    void ZPoseAnim::UpdateData(char* pBuffer)
    {
        char* pCurrent = pBuffer;

        for (int i = 0; i < Count(); ++i)
        {
            const char poseName = *pCurrent;
            pCurrent += 2;

            const uint16_t poseKeyCount = *reinterpret_cast<uint16_t*>(pCurrent);
            pCurrent += sizeof(uint16_t);

            SPoseKey* pPoseKeys = reinterpret_cast<SPoseKey*>(pCurrent);

            if (m_pPoseList)
            {
                ZPoseModel& poseModel = m_pPoseList[i];
                poseModel.SetName(poseName);
                poseModel.SetSize(poseKeyCount);

                if (poseKeyCount && poseName > 0 && poseName < 60)
                {
                    poseModel.ResetIndex();
                    poseModel.SetKeys(pPoseKeys);
                }
                else
                {
                    poseModel.SetKeys(nullptr);
                    poseModel.SetActive(false);
                }
            }

            pCurrent = reinterpret_cast<char*>(&pPoseKeys[poseKeyCount]);
        }
    }

    void ZPoseAnim::UpdateFrame()
    {
        if (FrameTime() == 0.0f)
        {
            return;
        }

        const float realTime = static_cast<float>(g_pSysInterface->GetRealTime());
        if (realTime < FrameStartTime() + FrameTime() * TimeMultiplier())
        {
            return;
        }

        // TODO: Finish this place after ZEngineDataBase will be reversed.
        // ZSoundObject* pSoundObject = g_pEngineData->SRefToPtr(m_rHost);
        // if (pSoundObject && pSoundObject->GetPoseData())
        // {
        //     UpdateData(static_cast<char*>(pSoundObject->GetPoseData()));
        //     SetFrameStartTime(static_cast<float>(g_pSysInterface->GetRealTime()));
        //     ++m_iUpdate;
        // }
    }

    float ZPoseAnim::GetPoseWeight(int lIdx, float fWeight)
    {
        if (fWeight < m_fStartTime)
        {
            return 0.0f;
        }

        const float poseWeight = (fWeight - m_fStartTime) / m_fTimeMultiplier;
        ZPoseModel& sModel = m_pPoseList[lIdx];

        if (!sModel.Active())
        {
            return 0.0f;
        }

        return sModel.GetWeight(poseWeight);
    }

    char ZPoseAnim::GetPoseName(int lIdx)
    {
        ZASSERT(m_pPoseList != nullptr);
        // ZASSERT(lIdx >= 0 && lIdx < m_lPoseSize); // ???

        return m_pPoseList[lIdx].Name();
    }
}
