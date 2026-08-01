#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/ZUniAssert.h>


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
        ZASSERT(Action::instance);
        if (!Action::instance)
            return false;

        if (!CheckMap())
        {
            return false;
        }

        if (!Action::instance->GetJoinControllers())
            return m_pkMap->Digital();

        bool bResult = false;
        for (int i = 0; i < SysInput::instance->DeviceCount(SysInput::EDeviceType::eGAMECONTROL_TYPE); ++i)
        {
            const auto lDevId = SysInput::instance->GetNthDevice(SysInput::EDeviceType::eGAMECONTROL_TYPE, i);
            m_pkMap->SetDevice(lDevId);
            bResult |= m_pkMap->Digital();
        }
        
        return bResult;
    }

    float Action::ZHandle::Analog()
    {
        ZASSERT(Action::instance);
        if (!Action::instance)
            return false;

        if (!CheckMap())
        {
            return false;
        }
        
        if (Action::instance->GetJoinControllers())
        {
            float fResult = 0.0f;

            for (int i = 0; i < SysInput::instance->DeviceCount(SysInput::EDeviceType::eGAMECONTROL_TYPE); ++i)
            {
                const auto lDevId = SysInput::instance->GetNthDevice(SysInput::EDeviceType::eGAMECONTROL_TYPE, i);
                m_pkMap->SetDevice(lDevId);
                fResult += m_pkMap->Analog();
            }
            return fResult;
        }
        
        return m_pkMap->Analog();
    }

    int Action::ZHandle::ActivatedBy()
    {
        return (CheckMap() ? m_pkMap->ActivatedBy() : -1);
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
