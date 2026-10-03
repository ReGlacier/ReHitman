#include <Glacier/EventBase/ZScheduledScript.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZEventBase.h>


namespace Glacier
{
    extern bool s_bSleeperChanged;

    ZScheduledScript::ZScheduledScript(ZEventBase* pEvent) 
        : ZScheduledEvent(pEvent)
    {
    }

    void ZScheduledScript::Run()
    {
        m_bRunning = 1;

        auto* pEvent = ZEventBase::RefToPtr(m_rEvent);
        if (pEvent)
        {
            // Attach us to event
            ZScheduledUpdate::SetEvent(pEvent, this);

            // Call event
            pEvent->Call(0x1000, nullptr, 0);
        }

        m_bRunning = false;
    }

    ZScheduledEvent* ZScheduledScript::GetNextThread()
    {
        return m_pThread;
    }

    ZScheduledEvent* ZScheduledScript::Fork()
    {
        auto* pResult = new ZScheduledScript(ZEventBase::RefToPtr(m_rEvent));
        return ZScheduledEvent::Fork(pResult);
    }

    void ZScheduledScript::Sleep(float fTime)
    {
        TIMETYPE oldTime = m_tNextRun;

        float fFinalTime = (fTime < 0.0f) ? -1.0f : fTime;

        m_tNextRun = TIMETYPE(fFinalTime); 

        if (oldTime != m_tNextRun)
        {
            m_bChanged = 1;
            s_bSleeperChanged = true;
        }

        if (m_pNxt == nullptr)
        {
            m_bChanged = 1;
            ZScheduledUpdate::s_pScheduler->AttachEvent(this);
        }
    }

    void ZScheduledScript::Restart()
    {
        m_tNextRun = TIMETYPE(-3);
    }

    void ZScheduledScript::SetUserData(void* pData)
    {
        m_pUserData = pData;
    }

    void* ZScheduledScript::GetUserData() const
    {
        return m_pUserData;
    }
}