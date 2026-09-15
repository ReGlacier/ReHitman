#include <Glacier/Com/CComRead.h>
#include <Glacier/Com/CSharedCom.h>


namespace Glacier
{
    // CComRead

    CComRead::CComRead(CSharedCom* pCom, const char* pName)
    {
        m_pCom = pCom;
        m_pName = pName;
    }

    CComRead::CComRead(const CComRead& copy)
    {
        m_pCom = copy.m_pCom;
        m_pName = copy.m_pName;
    }

    CComRead::operator uint32_t() const
    {
        int32_t val = 0;
        m_pCom->GetVal(m_pName, &val);
        return static_cast<uint32_t>(val);
    }

    CComRead::operator int32_t() const
    {
        int32_t val = 0;
        m_pCom->GetVal(m_pName, &val);
        return val;
    }

    CComRead::operator float() const
    {
        float val = 0.0f;
        m_pCom->GetVal(m_pName, &val);
        return val;
    }

    CComRead::operator char*() const
    {
        // Or maybe GetVal ?
        return (char*)m_pCom->GetValPtr(m_pName, CCOM_FORMAT_CHAR);
    }

    CComRead::operator bool() const
    {
        bool val = false;
        m_pCom->GetVal(m_pName, &val);
        return val;
    }

}