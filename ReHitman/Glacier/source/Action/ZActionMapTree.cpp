#include <Glacier/Action/ZActionMapTree.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZDevice.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <algorithm>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    static constexpr const char* s_EmptyString = "";

    static bool IsMappedControl(const ZActionMapTree* pTree)
    {
        return static_cast<int16_t>(pTree->GetDeviceId()) != -1 && (pTree->GetControlId() & 0x8000u) == 0;
    }

    static float CurrentRealTime()
    {
        return static_cast<float>(g_pSysInterface->m_fRealTime);
    }

    static float DigitalHistSeconds(SysInput::ZDevice* pDevice, int controlId, int nth)
    {
        return static_cast<float>(pDevice->DigitalHist(controlId, nth));
    }

    static void PrintActionDisplayLine(const char* pText)
    {
        printf("%s\n", pText);
    }

    static void AppendString(char* pBuffer, size_t size, const char* pValue)
    {
        if (!pValue)
        {
            return;
        }

        const auto length = strlen(pBuffer);
        if (length >= size - 1)
        {
            return;
        }

        strncat(pBuffer, pValue, size - length - 1);
    }

    static char* AppendBinaryDisplay(char* pBuffer, const char* pOperator, ZActionMapTree* pFirstChild)
    {
        AppendString(pBuffer, 300, pOperator);

        if (pFirstChild)
        {
            pFirstChild->Display(0, pBuffer);

            if (auto* pSecondChild = pFirstChild->Next())
            {
                pSecondChild->Display(0, pBuffer);
            }
        }

        return pBuffer + strlen(pBuffer);
    }

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
        if (SysInput::instance->DeviceType(iDevice) == SysInput::instance->DeviceType(static_cast<int16_t>(m_iDeviceId)))
        {
            m_iDeviceId = static_cast<uint16_t>(iDevice);
        }

        for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            pCurrent->SetDevice(iDevice);
        }
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

    void ZActionMapTree::Display(int iIndent, char* pOut)
    {
        char localBuffer[304]{};
        bool bRoot = false;

        if (!pOut)
        {
            const int indentLength = std::min(iIndent * 2, 299);
            memset(localBuffer, ' ', indentLength);
            localBuffer[indentLength] = '\0';

            AppendString(localBuffer, 300, GetName());
            if (!strcmp(GetName(), "Root"))
            {
                bRoot = true;
            }

            AppendString(localBuffer, 300, " =");
            pOut = localBuffer + strlen(localBuffer);
        }

        const char* pInputCall = nullptr;
        bool bAnalogInput = false;

        switch (m_eType)
        {
            case eUNDEFINED:
                AppendString(pOut, 300, " {");
                PrintActionDisplayLine(localBuffer);

                for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                {
                    pCurrent->Display(iIndent + 1, nullptr);
                }

                memset(localBuffer, ' ', std::min(iIndent * 2, 299));
                localBuffer[std::min(iIndent * 2, 299)] = '\0';
                AppendString(localBuffer, 300, "}");
                break;

            case eGET:
                AppendString(pOut, 300, " get(");
                if (m_pkGetTree)
                {
                    AppendString(pOut, 300, m_pkGetTree->GetName());
                }
                AppendString(pOut, 300, ")");
                break;

            case eCONSTANT:
            {
                char valueBuffer[64]{};
                sprintf(valueBuffer, " %f", m_fConstant);
                AppendString(pOut, 300, valueBuffer);
                break;
            }

            case eHOLD:
                pInputCall = "hold";
                break;

            case eTAP:
                pInputCall = "tap";
                break;

            case eRELEASE:
                pInputCall = "release";
                break;

            case eDOWNEDGE:
                pInputCall = "downedge";
                break;

            case eFASTTAP:
                pInputCall = "fasttap";
                break;

            case eDOUBLECLICK:
                pInputCall = "doubleclick";
                break;

            case eHOLDDOWN:
                pInputCall = "holddown";
                break;

            case eCLICKHOLD:
                pInputCall = "clickhold";
                break;

            case ePRESS:
                pInputCall = "press";
                break;

            case eAND:
                pOut = AppendBinaryDisplay(pOut, " &", FirstChild());
                break;

            case eOR:
                pOut = AppendBinaryDisplay(pOut, " |", FirstChild());
                break;

            case eGT:
                pOut = AppendBinaryDisplay(pOut, " >", FirstChild());
                break;

            case eLT:
                pOut = AppendBinaryDisplay(pOut, " <", FirstChild());
                break;

            case eSEQUENCE:
                AppendString(pOut, 300, " seq(");
                for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                {
                    pOut += strlen(pOut);
                    pCurrent->Display(0, pOut);
                    if (pCurrent->Next())
                    {
                        AppendString(pOut, 300, ",");
                    }
                }
                AppendString(pOut, 300, ")");
                break;

            case eANALOG:
                pInputCall = "ana";
                bAnalogInput = true;
                break;

            case eRELATIVE:
                pInputCall = "rel";
                bAnalogInput = true;
                break;

            case ePLUS:
                pOut = AppendBinaryDisplay(pOut, " +", FirstChild());
                break;

            case eMINUS:
                pOut = AppendBinaryDisplay(pOut, " -", FirstChild());
                break;

            case eMULT:
                pOut = AppendBinaryDisplay(pOut, " *", FirstChild());
                break;

            default:
                break;
        }

        if (pInputCall)
        {
            char inputBuffer[128]{};
            auto* pDevice = SysInput::instance->GetDevice(static_cast<int16_t>(m_iDeviceId));
            if (pDevice)
            {
                const char* pDeviceName = SysInput::instance->GetDeviceName(static_cast<int16_t>(m_iDeviceId));
                const char* pControlName = bAnalogInput ?
                    pDevice->AnalogName(static_cast<int16_t>(m_iControlId)) :
                    pDevice->DigitalName(static_cast<int16_t>(m_iControlId));
                sprintf(inputBuffer, "%s %s(%s, %s)", pOut, pInputCall, pDeviceName, pControlName);
            }
            else
            {
                sprintf(inputBuffer, "%s %s(Unavailable, Unavailable)", pOut, pInputCall);
            }

            strncpy(pOut, inputBuffer, 299);
            pOut[299] = '\0';
        }

        if (m_acName[0])
        {
            AppendString(pOut, 300, ";");
            PrintActionDisplayLine(localBuffer);
        }

        if (bRoot)
        {
            PrintActionDisplayLine("}");
        }
    }

    class ZActionMapTree* ZActionMapTree::Parent() const
    {
        return m_pkParent;
    }

    const char* ZActionMapTree::GetKeyName() const
    {
        if (static_cast<int16_t>(m_iDeviceId) == -1 || static_cast<int16_t>(m_iControlId) == -1)
        {
            return s_EmptyString;
        }

        auto* pDevice = SysInput::instance->GetDevice(static_cast<int16_t>(m_iDeviceId));

        if (m_eType < eHOLD)
        {
            return s_EmptyString;
        }

        if (m_eType < eAND)
        {
            return pDevice->DigitalName(static_cast<int16_t>(m_iControlId));
        }

        if (m_eType == eANALOG || m_eType == eRELATIVE)
        {
            return pDevice->AnalogName(static_cast<int16_t>(m_iControlId));
        }

        return s_EmptyString;
    }

    const char* ZActionMapTree::GetSystemKeyName(bool firstKey) const
    {
        static char nameBuf[49]{};

        if (static_cast<int16_t>(m_iDeviceId) == -1 || static_cast<int16_t>(m_iControlId) == -1)
        {
            return s_EmptyString;
        }

        auto* pDevice = SysInput::instance->GetDevice(static_cast<int16_t>(m_iDeviceId));

        switch (m_eType)
        {
            case eHOLD:
            case eTAP:
            case eRELEASE:
            case eDOWNEDGE:
            case eFASTTAP:
            case eDOUBLECLICK:
            case eHOLDDOWN:
            case eCLICKHOLD:
            case ePRESS:
                return pDevice->DigitalSystemName(static_cast<int16_t>(m_iControlId));

            case eAND:
            case eOR:
            {
                if (firstKey)
                {
                    return FirstChild()->GetSystemKeyName(false);
                }

                const char* pFirstName = FirstChild()->GetSystemKeyName(false);
                const char* pSecondName = FirstChild()->Next()->GetSystemKeyName(false);
                const char* pOperator = m_eType == eOR ? "or" : "and";

                if (strlen(pFirstName) + strlen(pOperator) + strlen(pSecondName) + 2 < sizeof(nameBuf))
                {
                    sprintf(nameBuf, "%s %s %s", pFirstName, pOperator, pSecondName);
                }

                return nameBuf;
            }

            case eANALOG:
            case eRELATIVE:
                return pDevice->AnalogName(static_cast<int16_t>(m_iControlId));

            default:
                break;
        }

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
        if (static_cast<int16_t>(m_iDeviceId) != -1 && (m_iControlId & 0x8000u) == 0)
        {
            auto* pDevice = SysInput::instance->GetDevice(static_cast<int16_t>(m_iDeviceId));
            if (pDevice)
            {
                pDevice->ClearKey(static_cast<int16_t>(m_iControlId));
            }
        }

        for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            pCurrent->ClearInputKeys();
        }
    }

    bool ZActionMapTree::Digital()
    {
        if (!SysInput::instance || Disabled())
        {
            return false;
        }

        bool bResult = false;
        SysInput::ZDevice* pDevice = nullptr;
        const float fCurrentTime = CurrentRealTime();
        const float fLastFiredThreshold = std::max(fCurrentTime - 0.2f, m_fLastFired);

        if (static_cast<int16_t>(m_iDeviceId) != -1)
        {
            pDevice = SysInput::instance->GetDevice(static_cast<int16_t>(m_iDeviceId));
        }

        switch (m_eType)
        {
            case eGET:
                bResult = m_pkGetTree->Digital();
                break;

            case eCONSTANT:
                bResult = m_fConstant != 0.0f;
                break;

            case eHOLD:
            case ePRESS:
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                bResult = pDevice->DigitalState(static_cast<int16_t>(m_iControlId)) != 0;
                break;

            case eTAP:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                const bool bPressed = pDevice->DigitalState(controlId) != 0;
                const float fHist0 = DigitalHistSeconds(pDevice, controlId, 0);
                const float fHist1 = DigitalHistSeconds(pDevice, controlId, 1);

                if (bPressed)
                {
                    bResult = fLastFiredThreshold < fHist0;
                }
                else
                {
                    bResult = fLastFiredThreshold < fHist0 && fLastFiredThreshold < fHist1;
                }
                break;
            }

            case eRELEASE:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                bResult = fLastFiredThreshold < DigitalHistSeconds(pDevice, controlId, 0) &&
                    pDevice->DigitalState(controlId) == 0;
                break;
            }

            case eDOWNEDGE:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                bResult = fLastFiredThreshold < DigitalHistSeconds(pDevice, controlId, 0) &&
                    pDevice->DigitalState(controlId) != 0;
                break;
            }

            case eFASTTAP:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                bResult = pDevice->DigitalState(controlId) == 0 &&
                    fLastFiredThreshold < DigitalHistSeconds(pDevice, controlId, 0) &&
                    fLastFiredThreshold < DigitalHistSeconds(pDevice, controlId, 1);
                break;
            }

            case eDOUBLECLICK:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                const float fThreshold = std::max(fCurrentTime - 0.3f, m_fLastFired);
                bResult = pDevice->DigitalState(controlId) == 0 &&
                    fThreshold < DigitalHistSeconds(pDevice, controlId, 0) &&
                    fThreshold < DigitalHistSeconds(pDevice, controlId, 1) &&
                    fThreshold < DigitalHistSeconds(pDevice, controlId, 2) &&
                    fThreshold < DigitalHistSeconds(pDevice, controlId, 3);
                break;
            }

            case eHOLDDOWN:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                bResult = DigitalHistSeconds(pDevice, controlId, 0) < fLastFiredThreshold &&
                    pDevice->DigitalState(controlId) != 0;
                break;
            }

            case eCLICKHOLD:
            {
                if (!pDevice || (m_iControlId & 0x8000u) != 0)
                    return false;

                const auto controlId = static_cast<int16_t>(m_iControlId);
                const float fHist0 = DigitalHistSeconds(pDevice, controlId, 0);
                bResult = pDevice->DigitalState(controlId) != 0 &&
                    fHist0 - 0.2f < DigitalHistSeconds(pDevice, controlId, 1) &&
                    fHist0 - 0.2f < DigitalHistSeconds(pDevice, controlId, 2);
                break;
            }

            case eAND:
                bResult = true;
                for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                {
                    bResult = pCurrent->Digital();
                    if (!bResult)
                        break;
                }
                break;

            case eOR:
                for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                {
                    bResult = pCurrent->Digital();
                    if (bResult)
                        break;
                }
                break;

            case eGT:
            case eLT:
            {
                auto* pFirstChild = FirstChild();
                if (pFirstChild && pFirstChild->Next())
                {
                    const float fFirst = pFirstChild->Analog();
                    const float fSecond = pFirstChild->Next()->Analog();
                    bResult = m_eType == eGT ? fSecond < fFirst : fFirst < fSecond;
                }
                break;
            }

            case eSEQUENCE:
            {
                if (FirstChild())
                {
                    for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                    {
                        pCurrent->Digital();
                    }

                    int iCount = 1;
                    bool bInOrder = true;
                    for (auto* pCurrent = FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                    {
                        if (!pCurrent->Next())
                            break;

                        bInOrder = pCurrent->m_fLastFired < pCurrent->Next()->m_fLastFired;
                        if (!bInOrder)
                            break;

                        ++iCount;
                    }

                    const float fThreshold = std::max(fCurrentTime - static_cast<float>(iCount) * 0.2f, m_fLastFired);
                    bResult = fThreshold < FirstChild()->m_fLastFired && bInOrder;
                }
                break;
            }

            default:
                break;
        }

        if (m_eMods & eNEG)
        {
            bResult = !bResult;
        }

        if (bResult)
        {
            m_fLastFired = fCurrentTime;
            m_iLastDevice = m_iDeviceId;
        }

        return bResult;
    }

    float ZActionMapTree::Analog()
    {
        if (Disabled())
        {
            return 0.0f;
        }

        float fResult = 0.0f;
        SysInput::ZDevice* pDevice = nullptr;

        switch (m_eType)
        {
            case eGET:
                fResult = m_pkGetTree->Analog();
                break;

            case eCONSTANT:
                fResult = m_fConstant;
                break;

            case eANALOG:
            case eRELATIVE:
                if (IsMappedControl(this))
                {
                    pDevice = SysInput::instance->GetDevice(static_cast<int16_t>(m_iDeviceId));
                    if (pDevice)
                    {
                        const auto controlId = static_cast<int16_t>(m_iControlId);
                        fResult = m_eType == eANALOG ? pDevice->AnalogState(controlId) : pDevice->AnalogMotion(controlId);
                    }
                }
                break;

            case ePLUS:
            case eMINUS:
            case eMULT:
            {
                auto* pFirstChild = FirstChild();
                if (pFirstChild && pFirstChild->Next())
                {
                    const float fFirst = pFirstChild->Analog();
                    const float fSecond = pFirstChild->Next()->Analog();

                    if (m_eType == ePLUS)
                    {
                        fResult = fFirst + fSecond;
                    }
                    else if (m_eType == eMINUS)
                    {
                        fResult = fFirst - fSecond;
                    }
                    else
                    {
                        fResult = fFirst * fSecond;
                    }
                }
                break;
            }

            default:
                fResult = static_cast<float>(Digital());
                break;
        }

        if (m_eMods & eNEG)
        {
            fResult = -fResult;
        }

        return fResult;
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

            pCurrent->m_pkNext = pChild;
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
