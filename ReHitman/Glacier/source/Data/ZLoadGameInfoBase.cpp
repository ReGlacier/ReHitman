#include <Glacier/Data/ZLoadGameInfoBase.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>

namespace Glacier
{
    STATIC_CLASS_VAR_IMPL(ZLoadGameInfoBase, ZLoadGameInfoBase*, m_Instance, 0x008BE150, nullptr);

    // PC 0x4681E0. The first (and only) concrete instance registers itself.
    ZLoadGameInfoBase::ZLoadGameInfoBase()
    {
        ZASSERT(m_Instance == nullptr);
        m_Instance = this;
        m_ControlStreamFilename[0] = '\0';
        m_DataStreamFilename[0] = '\0';
    }

    // PC 0x468210 / 0x4682A0. Base sub-object destructor; unregisters the instance.
    ZLoadGameInfoBase::~ZLoadGameInfoBase()
    {
        ZASSERT(m_Instance == this);
        m_Instance = nullptr;
    }

    // PC (iOS 0x1002E1594 / PS2 0x1B664C). Copies the control and data file names.
    void ZLoadGameInfoBase::SetFilenames(const char* pszControlStreamName, const char* pszDataStreamName)
    {
        strcpy(m_ControlStreamFilename, pszControlStreamName);
        strcpy(m_DataStreamFilename, pszDataStreamName);
    }

    // PS2 0x70A1DC. Opens the control stream through the registered instance.
    IInputStream* ZLoadGameInfoBase::CreateControlStream()
    {
        if (m_Instance)
            return m_Instance->CreateStream(m_Instance->m_ControlStreamFilename);
        return nullptr;
    }

    // PS2 0x70A260. Opens the data stream through the registered instance.
    IInputStream* ZLoadGameInfoBase::CreateDataStream()
    {
        if (m_Instance)
            return m_Instance->CreateStream(m_Instance->m_DataStreamFilename);
        return nullptr;
    }

    // PS2 0x70A2E4. PC 0x460B66 calls the deleting-destructor slot with flag 0
    // (destruct only, no free), matching the static-global allocation model of the
    // concrete instances, so this runs the destructor without releasing memory.
    void ZLoadGameInfoBase::Destroy()
    {
        if (m_Instance)
            m_Instance->~ZLoadGameInfoBase();
    }
}
