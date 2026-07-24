#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>

namespace Glacier
{
    Action::ZHandle::ZHandle(const char* psName)
        : m_szName(psName)
        , m_pkMap(nullptr)
        , m_iSeq(-1)
    {
    }

    Action::ZHandle::~ZHandle() = default;

    void Action::ZHandle::SetDevice(int iDevice)
    {
        CheckMap();

        if (m_pkMap)
        {
            m_pkMap->SetDevice(iDevice);
        }
    }

    int Action::ZHandle::Override(int* pDeviceId, int* pControlId)
    {
        CheckMap();

        if (m_pkMap)
        {
            return m_pkMap->Override(pDeviceId, pControlId);
        }

        return 0;
    }

    bool Action::ZHandle::Digital()
    {
        // TODO: Finish after SysInput reversed
        return false;
    }

    float Action::ZHandle::Analog()
    {
        // TODO: Finish after SysInput reversed
        return 0.f;
    }

    int Action::ZHandle::ActivatedBy()
    {
        return (CheckMap() ? ActivatedBy() : -1);
    }

    void Action::ZHandle::ClearInputKeys()
    {
        if (CheckMap())
        {
            m_pkMap->ClearInputKeys();
        }
    }

    bool Action::ZHandle::CheckMap()
    {
        if (Action::instance)
        {
            if (Action::instance->SeqNr() != m_iSeq)
            {
                m_pkMap = Action::instance->GetMapping(m_szName);
                m_iSeq = Action::instance->SeqNr();
            }

            return m_pkMap != nullptr;
        }

        return false;
    }
}