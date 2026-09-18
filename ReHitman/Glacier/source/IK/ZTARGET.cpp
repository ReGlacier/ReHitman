#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/IK/ZTARGET.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t INVALID_TARGET = 0x7FFFFFFF;

        bool IsInvalidTarget(float value)
        {
            uint32_t bits;
            std::memcpy(&bits, &value, sizeof(bits));
            return bits == INVALID_TARGET;
        }

        void SetInvalidTarget(float& value)
        {
            std::memcpy(&value, &INVALID_TARGET, sizeof(value));
        }
    }

    ZTARGET::ZTARGET()
    {
        m_mTarget.Reset();
        m_vTarget.Reset();
        m_CallBack.m_pCallback = nullptr;
        m_CallBack.m_delta = 0;
        m_CallBack.m_vbtableOffset = 0;
        m_CallBack.lCallBackId = -1;
        m_rGeom = 0;
        m_fTime = 0.0f;
        m_fStartTime = 0.0f;
        m_lBoneId = 0;
        m_bEnabled = false;
    }

    void ZTARGET::SetBoneId(int lBoneId)
    {
        m_lBoneId = lBoneId;
    }

    int ZTARGET::BoneId() const
    {
        return m_lBoneId;
    }

    float ZTARGET::GetTimePrc() const
    {
        const int32_t lStartTime = static_cast<int32_t>(fabs(m_fStartTime) * TIMETYPE::kTicksPerSecond);
        const float fPassedTime = static_cast<float>(g_pSysInterface->FrameTime.secs - lStartTime) * TIMETYPE::kInvTPS;
        const bool bCountdown = m_fStartTime < 0.0f;

        if (bCountdown)
        {
            if (fPassedTime > m_fTime || m_fTime == 0.0f)
                return 0.0f;
            else
                return 1.0f - (fPassedTime / m_fTime);
        }
        else
        {
            if (fPassedTime >= m_fTime)
                return 1.0f;

            return fPassedTime / m_fTime;
        }
    }

    void ZTARGET::SetTime(float fTime, bool bRemove)
    {
        const float fTimePrc = GetTimePrc();
        m_fTime = fTime;

        if (fTimePrc == 1.0f)
        {
            m_fStartTime = (bRemove ? -1.0f : 1.0f) * static_cast<float>(g_pSysInterface->FrameTime);

            if (bRemove)
                m_rGeom = 0;
        }
        else if (bRemove)
        {
            const int32_t lRemainingTime = static_cast<int32_t>((1.0f - fTimePrc) * fTime * TIMETYPE::kTicksPerSecond);
            m_fStartTime = static_cast<float>(-g_pSysInterface->FrameTime.secs - lRemainingTime) * TIMETYPE::kInvTPS;
            m_rGeom = 0;
        }
        else
        {
            const int32_t lPassedTime = static_cast<int32_t>(fTimePrc * fTime * TIMETYPE::kTicksPerSecond);
            m_fStartTime = static_cast<float>(g_pSysInterface->FrameTime.secs - lPassedTime) * TIMETYPE::kInvTPS;
        }
    }

    bool ZTARGET::GetPos(ZVector3& vPos) const
    {
        if (IsInvalidTarget(m_vTarget.x))
        {
            return false;
        }

        vPos = m_vTarget;
        return true;
    }

    void ZTARGET::LoadSave(ZLNKOBJ* pLnkObj, ISerializerStream& stream, bool bSaving)
    {
        stream.Exchange("m_bEnabled", m_bEnabled);
        stream.ExchangeArray("m_mTarget", m_mTarget, 9);
        stream.ExchangeArray("m_vTarget", m_vTarget, 3);
        stream.Exchange("m_fTime", m_fTime);
        stream.Exchange("m_fStartTime", m_fStartTime);
        stream.Exchange("m_rGeom", m_rGeom);
        stream.Exchange("m_lBoneId", m_lBoneId);

        if (bSaving)
        {
            int32_t lId = 0;

            if (m_CallBack.m_pCallback)
            {
                lId = static_cast<ZLNKWHANDS*>(pLnkObj)->IKCallBackToId(&m_CallBack);
                ZASSERT(lId >= 0);
            }

            stream.Exchange("lCallBackId", lId);
        }
        else
        {
            int32_t lId = 0;
            stream.Exchange("lCallBackId", lId);
            ZASSERT(lId >= 0);

            if (lId <= 0)
            {
                m_CallBack.m_pCallback = nullptr;
                m_CallBack.m_delta = 0;
                m_CallBack.m_vbtableOffset = 0;
                m_CallBack.lCallBackId = -1;
            }
            else
            {
                auto* pCallBack = static_cast<ZLNKWHANDS*>(pLnkObj)->IKCallBackFromId(lId);
                m_CallBack.m_pCallback = pCallBack->m_pCallback;
                m_CallBack.m_delta = pCallBack->m_delta;
                m_CallBack.m_vbtableOffset = pCallBack->m_vbtableOffset;
                m_CallBack.lCallBackId = pCallBack->lCallBackId;
                ZASSERT(m_CallBack.m_pCallback);
            }
        }
    }

    void ZTARGET::CallBackIfTime(ZIKLNKOBJ* pIkLnkObj)
    {
        const float fTimePrc = GetTimePrc();
        if (fTimePrc == 0.0f || fTimePrc == 1.0f)
        {
            IKCallBack_t pFunc = nullptr;
            int32_t lDelta = 0;

            if (m_fStartTime >= 0.0f)
            {
                if (fTimePrc == 1.0f)
                {
                    pFunc = m_CallBack.m_pCallback;
                    lDelta = m_CallBack.m_delta;
                }
            }
            else
            {
                if (fTimePrc == 0.0f)
                {
                    m_bEnabled = false;
                    pFunc = m_CallBack.m_pCallback;
                    lDelta = m_CallBack.m_delta;
                }
            }

            if (pFunc)
            {
                m_CallBack.m_pCallback = nullptr;
                m_CallBack.m_delta = 0;
                m_CallBack.m_vbtableOffset = 0;
                m_CallBack.lCallBackId = -1;
                pFunc(reinterpret_cast<ZGEOM*>(reinterpret_cast<char*>(pIkLnkObj) + lDelta));
            }
        }
    }

    void ZTARGET::CallBackAndRemove(ZIKLNKOBJ* pDest)
    {
        while (m_CallBack.m_pCallback)
        {
            IKCallBack_t pFunc = m_CallBack.m_pCallback;
            const int32_t lDelta = m_CallBack.m_delta;

            m_CallBack.m_pCallback = nullptr;
            m_CallBack.m_delta = 0;
            m_CallBack.m_vbtableOffset = 0;
            m_CallBack.lCallBackId = -1;

            pFunc(reinterpret_cast<ZGEOM*>(reinterpret_cast<char*>(pDest) + lDelta));
        }
    }

    void ZTARGET::Clear()
    {
        m_mTarget.Reset();
        m_vTarget.Reset();
        m_CallBack.m_pCallback = nullptr;
        m_CallBack.m_delta = 0;
        m_CallBack.m_vbtableOffset = 0;
        m_CallBack.lCallBackId = -1;
        m_rGeom = 0;
        m_fTime = 0.0f;
        m_fStartTime = 0.0f;
        m_lBoneId = 0;
        m_bEnabled = false;
    }

    bool ZTARGET::Enable(bool bEnabled)
    {
        const bool bWasEnabled = m_bEnabled;
        m_bEnabled = bEnabled;
        return bWasEnabled;
    }

    bool ZTARGET::IsEnabled() const
    {
        return m_bEnabled;
    }

    bool ZTARGET::Removing() const
    {
        return m_fStartTime < 0.0f;
    }

    void ZTARGET::SetMatPosGeom(const ZMat3x3* pMat, const ZVector3* pPos, ZREF rGeom)
    {
        if (pMat)
        {
            m_mTarget = *pMat;
        }
        else
        {
            SetInvalidTarget(m_mTarget.data[0]);
        }

        if (pPos)
        {
            m_vTarget = *pPos;
        }
        else
        {
            SetInvalidTarget(m_vTarget.x);
        }

        m_rGeom = rGeom;
    }

    void ZTARGET::SetCallBack(ZIKCALLBACK cb)
    {
        m_CallBack = cb;
    }

    IKCallBack_t ZTARGET::GetCallBack() const
    {
        return m_CallBack.m_pCallback;
    }

    void ZTARGET::Update(ZIKLNKOBJ* pIkLnkObj)
    {
        if (m_bEnabled)
            CallBackIfTime(pIkLnkObj);
    }

    void ZTARGET::Remove(ZIKLNKOBJ* pIkLnkObj, float fTime, ZIKCALLBACK cb)
    {
        CallBackAndRemove(pIkLnkObj);
        SetCallBack(cb);

        if (fTime == 0.0f)
        {
            SetTime(0.0f, true);
            CallBackIfTime(pIkLnkObj);
        }
        else if (!Removing())
        {
            SetTime(fTime, true);
        }
    }

    ZREF ZTARGET::Geom() const
    {
        return m_rGeom;
    }
}
