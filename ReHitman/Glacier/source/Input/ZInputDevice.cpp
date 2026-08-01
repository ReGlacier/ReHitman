#include <Glacier/Input/ZInputDevice.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZInputDevice::ZInputDevice(SysInput::EDeviceType deviceType, int digcount, int anacount)
    {
        m_updatetime = TIMETYPE(0);
        m_type = deviceType;
        m_pbDigital = nullptr;
        m_dighist = {};
        m_digcount = 0;
        m_analog = nullptr;
        m_anamotion = 0;
        m_anacount = 0;
        m_vHandlers[0] = nullptr;
        m_vHandlers[1] = nullptr;
        strcpy(name, "Unknown");
        m_active = false;
        m_connected = false;
        setControlCount(digcount, anacount);
    }

    ZInputDevice::~ZInputDevice()
    {
        clear();
    }

    SysInput::EDeviceType ZInputDevice::Type()
    {
        return m_type;
    }

    int ZInputDevice::DigitalCount()
    {
        return m_digcount;
    }

    int ZInputDevice::DigitalState(int nr)
    {
        if (nr < 0)
            return 0;

        return m_pbDigital[nr];
    }

    const char* ZInputDevice::DigitalName(int nr)
    {
        if (!m_diginf || !m_diginf->name)
            return nullptr;

        auto* pCurrentInfo = m_diginf;

        while (pCurrentInfo->nr != nr)
        {
            ++pCurrentInfo;

            if (!pCurrentInfo->name)
                return nullptr;
        }

        return pCurrentInfo->name;
    }

    int ZInputDevice::DigitalId(const char* name)
    {
        return lookupNr(m_diginf, name);
    }

    TIMETYPE ZInputDevice::DigitalHist(int nr, int nth)
    {
        return m_dighist[nr][nth];
    }
    
    int ZInputDevice::AnalogCount()
    {
        return m_anacount;
    }

    float ZInputDevice::AnalogState(int nr)
    {
        return m_analog[nr];
    }
    
    const char* ZInputDevice::AnalogName(int nr)
    {
        if (!m_anainf || !m_anainf->name)
        {
            return nullptr;
        }

        auto* pCurrentInfo = m_anainf;
        while (pCurrentInfo->nr != nr)
        {
            ++pCurrentInfo;

            if (!pCurrentInfo->name)
            {
                return nullptr;
            }
        }

        return pCurrentInfo->name;
    }
    
    int ZInputDevice::AnalogId(const char* name)
    {
        return lookupNr(m_anainf, name);
    }

    float ZInputDevice::AnalogMotion(int nr)
    {
        return m_anamotion[nr];
    }

    void ZInputDevice::ResetTables(bool bClearKeys)
    {
        if (bClearKeys && m_pbDigital)
        {
            for (int i = 0; i < m_digcount; ++i)
            {
                m_pbDigital[i] = false;
            }
        }

        memset(m_dighist, 0, sizeof(TIMETYPE) * DigHistSize * m_digcount);
        memset(m_analog, 0, sizeof(float) * m_anacount);
        memset(m_anamotion, 0, sizeof(float) * m_anacount);
    }

    void ZInputDevice::ClearKey(int id)
    {
        if (!m_pbDigital) return;

        m_pbDigital[id] = 0;
        m_dighist[id][0] = TIMETYPE(0);
        m_dighist[id][1] = TIMETYPE(0);
        m_dighist[id][2] = TIMETYPE(0);
        m_dighist[id][3] = TIMETYPE(0);
    }
    
    void ZInputDevice::TrapKey(int id, bool bTrap)
    {
        // Do nothing
    }

    void ZInputDevice::RumbleSetPitch(int axis, float p)
    {
        // Do nothing
    }

    bool ZInputDevice::StartHaptic(unsigned int id, float a, float b, bool bLoop)
    {
        // Do nothing
        return false;
    }

    void ZInputDevice::StopHaptics()
    {
        // Do nothing
    }

    void ZInputDevice::Activate()
    {
        m_active = true;
    }

    void ZInputDevice::Deactivate()
    {
        m_active = false;
    }

    const char* ZInputDevice::GetName()
    {
        return name;
    }

    void ZInputDevice::PrintInfo()
    {
        // Do nothing
    }

    bool ZInputDevice::Initialize()
    {
        return false;
    }
    
    bool ZInputDevice::Terminate()
    {
        // Do nothing
        return false;
    }
    
    void ZInputDevice::Update()
    {
        // Do nothing
    }

    int ZInputDevice::Acquire(int)
    {
        // Do nothing
        return 0;
    }
    
    int ZInputDevice::Acquire()
    {
        // Do nothing
        return 0;
    }
    
    int ZInputDevice::Unacquire()
    {
        // Do nothing
        return 0;
    }

    bool ZInputDevice::InstallHandler(SysInput::DeviceHandler handler, void* pUserData)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (!m_vHandlers[i])
            {
                m_vHandlers[i] = handler;
                m_vHandlerArgs[i] = pUserData;
                return true;
            }
        }
        
        return false;
    }

    bool ZInputDevice::UninstallHandler(SysInput::DeviceHandler handler)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_vHandlers[i] == handler)
            {
                m_vHandlers[i] = nullptr;
                m_vHandlerArgs[i] = nullptr;
                return true;
            }
        }
        
        return false;
    }
    
    int ZInputDevice::setControlCount(int digcount, int anacount)
    {
        clear();

        m_digcount = digcount;
        if (digcount)
        {
            m_pbDigital = static_cast<bool*>(ZUniMemory::Allocate(sizeof(bool) * digcount));
            m_dighist = static_cast<TIMETYPE(*)[DigHistSize]>(ZUniMemory::Allocate(sizeof(TIMETYPE) * DigHistSize * m_digcount));
        }

        m_anacount = anacount;
        if (anacount)
        {
            m_analog = static_cast<float*>(ZUniMemory::Allocate(sizeof(float) * anacount));
            m_anamotion = static_cast<float*>(ZUniMemory::Allocate(sizeof(float) * m_anacount));
        }

        if (m_pbDigital)
        {
            std::memset(m_pbDigital, 0, sizeof(bool) * m_digcount);
        }

        if (m_dighist)
        {
            std::memset(m_dighist, 0, sizeof(TIMETYPE) * DigHistSize * m_digcount);
        }

        if (m_analog)
        {
            std::memset(m_analog, 0, sizeof(float) * m_anacount);
        }

        if (m_anamotion)
        {
            std::memset(m_anamotion, 0, sizeof(float) * m_anacount);
        }

        return 0;
    }
     
    int ZInputDevice::lookupNr(CtrlInfo* list, const char* name)
    {
        if (list == nullptr || !list->name)
            return -1;
        
        const char* pCurrentName = list->name;
        while (strcmp(pCurrentName, name))
        {
            ++list;
            pCurrentName = list->name;
            if (!pCurrentName)
                return -1;
        }

        return list->nr;
    }

    void ZInputDevice::clear()
    {
        ZUniMemory::Free(m_pbDigital);
        ZUniMemory::Free(m_dighist);
        ZUniMemory::Free(m_analog);
        ZUniMemory::Free(m_anamotion);

        m_pbDigital = nullptr;
        m_dighist = nullptr;
        m_digcount = 0;
        m_analog = nullptr;
        m_anamotion = nullptr;
        m_anacount = 0;
    }

    void ZInputDevice::buttonSample(int nr, bool state, TIMETYPE time)
    {
        if (nr == -1 || m_pbDigital[nr] == state)
        {
            return;
        }

        for (int i = DigHistSize - 1; i > 0; --i)
        {
            m_dighist[nr][i] = m_dighist[nr][i - 1];
        }

        m_dighist[nr][0] = time;
        m_pbDigital[nr] = state;
    }

    int ZInputDevice::AnalogSample(int nr, float value)
    {
        if (nr >= m_anacount)
        {
            ZASSERT(false);
        }

        m_anamotion[nr] = value - m_analog[nr];
        m_analog[nr] = value;
        return nr;
    }

    float ZInputDevice::AnalogMotionSample(int nr, float value)
    {
        if (nr >= m_anacount)
        {
            ZASSERT(false);
        }

        m_analog[nr] = value;
        m_anamotion[nr] += value;
        return m_anamotion[nr];
    }
}
