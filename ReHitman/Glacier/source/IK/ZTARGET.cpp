#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/IK/ZTARGET.h>


namespace Glacier
{
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

    float ZTARGET::GetTimePrc() const
    {
        const float fPassedTime = static_cast<float>(g_pSysInterface->GetRealTime()) - m_fStartTime;
        const bool bCountdown = (m_fStartTime < 0.0);

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

        if (fTimePrc == 1.0f)
        {
            m_fStartTime = (bRemove ? -1.f : 1.f) * static_cast<float>(g_pSysInterface->GetRealTime());
        }
        else if (bRemove)
        {
            // TODO: Finish me
        }
        else
        {
            // TODO: Finish me
        }
    }

    bool ZTARGET::GetPos(ZVector3& vPos) const
    {
        if (isnan(m_vTarget.x))
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
                lId = static_cast<ZLNKWHANDS*>(pLnkObj)->IKCallBackToId(&m_CallBack.m_pCallback);
                ZASSERT(lId >= 0);
            }

            stream.Exchange("lCallBackId", lId);
        }
        else
        {
            int32_t lId = 0;
            stream.Exchange("lCallBackId", lId);

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
        if (m_CallBack.m_pCallback)
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

    void ZTARGET::Enable(bool bEnabled)
    {
        m_bEnabled = bEnabled;
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
            m_mTarget.data[0] = NAN;
        }

        if (pPos)
        {
            m_vTarget = *pPos;
        }
        else
        {
            m_vTarget.x = NAN;
        }

        m_rGeom = rGeom;
    }

    void ZTARGET::SetCallBack(ZIKCALLBACK cb)
    {
        m_CallBack = cb;
    }
}
