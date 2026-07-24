#include <Glacier/Action/ZActionMapTree.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    static constexpr const char* s_EmptyString = "";

    ZActionMapTree::~ZActionMapTree()
    {
        if (m_eMods & eSTATIC)
        {
            printf("Action --> ZActionMapTree::~ZActionMapTree() --> deleting static object\n");
        }
    }

    ZActionMapTree::ZActionMapTree()
    {
        Init();
    }

    void ZActionMapTree::Init()
    {
        m_iLastDevice = -1;
        m_pkParent = nullptr;
        m_pkFirstChild = nullptr;
        m_pkNext = nullptr;
        m_pkGetTree = nullptr;
        m_acName[0] = 0;
        m_eType = EType::eUNDEFINED;
        m_iDeviceId = -1;
        m_iControlId = -1;
        m_iOrgDeviceId = m_iDeviceId;
        m_iOrgControlId = m_iControlId;
        m_eMods = 0;
        m_fLastFired = 0.0;
    }

    void ZActionMapTree::Init(float fConstant)
    {
        m_eType = EType::eCONSTANT;
        m_fConstant = fConstant;
    }

    void ZActionMapTree::Init(ZTokenStream::EToken eToken)
    {
        switch (eToken)
        {
            case ZTokenStream::EToken::eVTRUE:
                m_eType = eCONSTANT;
                m_fConstant = 1.f;
            break;
            case ZTokenStream::EToken::eVFALSE:
                m_eType = eCONSTANT;
                m_fConstant = 0.f;
            break;
            case ZTokenStream::EToken::eOR:
                m_eType = eOR;
            break;
            case ZTokenStream::EToken::eAND:
                m_eType = eAND;
            break;
            case ZTokenStream::EToken::eGT:
                m_eType = eGT;
            break;
            case ZTokenStream::EToken::eLT:
                m_eType = eLT;
            break;
            case ZTokenStream::EToken::ePLUS:
                m_eType = ePLUS;
            break;
            case ZTokenStream::EToken::eMINUS:
                m_eType = eMINUS;
            break;
            case ZTokenStream::EToken::eMULT:
                m_eType = eMULT;
            break;
            case ZTokenStream::EToken::eSEQ:
                m_eType = eSEQUENCE;
            break;
            default:
                printf("Action --> Invalid ZActionMapTree type\n");
                break;
        }
    }

    void ZActionMapTree::Init(ZTokenStream::EToken eToken, int iDeviceId, int iControlId)
    {
        switch (eToken)
        {
            case ZTokenStream::EToken::eTAP:
                m_eType = eTAP;
            break;
            case ZTokenStream::EToken::eRELEASE:
                m_eType = eRELEASE;
            break;
            case ZTokenStream::EToken::eDOWNEDGE:
                m_eType = eDOWNEDGE;
            break;
            case ZTokenStream::EToken::eFASTTAP:
                m_eType = eFASTTAP;
            break;
            case ZTokenStream::EToken::eDOUBLECLICK:
                m_eType = eDOUBLECLICK;
            break;
            case ZTokenStream::EToken::eHOLDDOWN:
                m_eType = eHOLDDOWN;
            break;
            case ZTokenStream::EToken::eCLICKHOLD:
                m_eType = eCLICKHOLD;
            break;
            case ZTokenStream::EToken::ePRESS:
                m_eType = ePRESS;
            break;
            case ZTokenStream::EToken::eHOLD:
                m_eType = eHOLD;
            break;
            case ZTokenStream::EToken::eANA:
                m_eType = eANALOG;
            break;
            case ZTokenStream::EToken::eREL:
                m_eType = eRELATIVE;
            break;
            default: break; 
        }

        m_iOrgDeviceId = m_iDeviceId = iDeviceId;
        m_iOrgControlId = m_iControlId = iControlId;
    }

    void ZActionMapTree::Init(ZActionMapTree* gettree)
    {
        m_eType = EType::eGET;
        ZASSERT(gettree);
        m_pkGetTree = gettree;
    }

    void ZActionMapTree::SetName(const char* psName)
    {
        strncpy(&m_acName[0], psName, 24);
        if (!strcmp("DebugKeys", psName))
        {
            m_eMods |= eDEBUGKEYS;
        }

        if (!strcmp(psName, "DebugAlways"))
        {
            m_eMods |= eALWAYS;
        }
    }

    const char* ZActionMapTree::GetName() const
    {
        return &m_acName[0];
    }

    void ZActionMapTree::SetDevice(int iDevice)
    {
        // TODO: Finish me when SysInput will be reversed
    }

    void ZActionMapTree::RevertToDefault()
    {
        m_iDeviceId = m_iOrgDeviceId;
        m_iControlId = m_iOrgControlId;
        m_eMods &= ~eOVERRIDDEN;

        for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            pCurrent->RevertToDefault();
        }
    }

    void ZActionMapTree::RemoveFromTree()
    {
        if (!m_pkParent)
        {
            return;
        }

        auto* pPrevChild = Parent()->FirstChild();
        if (pPrevChild == this)
        {
            Parent()->m_pkFirstChild = Parent()->FirstChild()->Next();
        }
        else
        {
            while (pPrevChild && pPrevChild->Next() != this)
            {
                pPrevChild = pPrevChild->Next();
            }

            ZASSERT(pPrevChild);

            pPrevChild->m_pkNext = pPrevChild->Next()->Next(); // or pPrevChild->m_pkNext = this->m_pkNext;
        }

        m_pkParent = nullptr;
    }

    void ZActionMapTree::Display(int,char *)
    {
        // TODO: Finish me later
    }

    class ZActionMapTree* ZActionMapTree::Parent() const
    {
        return m_pkParent;
    }

    const char* ZActionMapTree::GetKeyName() const
    {
        if (m_iDeviceId == -1 || m_iControlId == -1)
        {
            return s_EmptyString;
        }

        // TODO: Finish me when SysInput will be finished!
        return s_EmptyString;
    }

    const char* ZActionMapTree::GetSystemKeyName(bool firstKey) const
    {
        if (m_iDeviceId == -1 || m_iControlId == -1)
        {
            return s_EmptyString;
        }

        // TODO: Finish me when SysInput will be finished!
        return "NA";
    }

    int ZActionMapTree::GetDeviceId() const
    {
        return m_iDeviceId;
    }

    int ZActionMapTree::GetControlId() const
    {
        return m_iControlId;
    }

    void ZActionMapTree::ClearInputKeys()
    {
        if (m_iDeviceId == -1 && (m_iControlId & 0x8000u) == 0)
        {
            // TODO: Finish me when SysInput will be reversed
        }

        for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            pCurrent->ClearInputKeys();
        }
    }

    bool ZActionMapTree::Digital()
    {
        // TODO: Finish me
        return false;
    }

    float ZActionMapTree::Analog()
    {
        // TODO: Finish me
        return 0.f;
    }

    int ZActionMapTree::ActivatedBy() const
    {
        return m_iLastDevice;
    }

    int ZActionMapTree::Override(int* pDeviceId, int* pControlId)
    {
        switch (m_eType)
        {
            case eGET:
                return m_pkGetTree->Override(pDeviceId, pControlId);
            
            case eHOLD:
            case eTAP:
            case eRELEASE:
            case eDOWNEDGE:
            case eFASTTAP:
            case eDOUBLECLICK:
            case eHOLDDOWN:
            case eCLICKHOLD:
            case ePRESS:
            case eANALOG:
            case eRELATIVE:
                m_iDeviceId = *pDeviceId;
                m_iControlId = *pControlId;
                m_eMods |= eOVERRIDDEN;
                return 1;

            default:
            {
                int result = 0;

                for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                {
                    result += pCurrent->Override(pDeviceId, pControlId);
                }

                return result;
            } 
        }

        return 0;
    }

    int ZActionMapTree::AddChild(ZActionMapTree* pChild)
    {
        auto* pCurrent = FirstChild();
        if (pCurrent)
        {
            for ( ; pCurrent->Next(); pCurrent = pCurrent->Next())
            {
            }
        }
        else
        {
            m_pkFirstChild = pChild;
        }

        pChild->m_pkNext = nullptr;
        pChild->m_pkParent = this;
        return 1;
    }

    void ZActionMapTree::Neg()
    {
        m_eMods ^= eNEG;
    }

    void ZActionMapTree::Not()
    {
        Neg();
    }

    void ZActionMapTree::Minus()
    {
        Neg();
    }

    ZActionMapTree* ZActionMapTree::FirstChild() const
    {
        return m_pkFirstChild;
    }
    ZActionMapTree* ZActionMapTree::Next() const
    {
        return m_pkNext;
    }

    bool ZActionMapTree::Disabled() const
    {
        if (!Action::instance->IsEnabled())
        {
            return true;
        }
        
        auto* pCurrent = this;

        while (pCurrent)
        {
            if (m_eMods & eDISABLED) return true;
            if (m_eMods & eALWAYS) return false;
            if (m_eMods & eDEBUGKEYS) return !Action::instance->GetDebugKeys();

            pCurrent = pCurrent->Parent();
        }

        return Action::instance->GetDebugKeys();
    }
}