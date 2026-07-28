#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Serializer/ZTokenStream.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZActionManager::ZActionManager() 
        : Action::Interface::Interface()
    {
        ++ZActionManager::m_iSeq;
        m_kRoot.SetName("Root");
        m_pkCurentBlock = &m_kRoot;

        for (auto* pCurrent = Action::ZStaticBinds::GetFirst(); pCurrent; pCurrent = pCurrent->GetNext())
        {
            auto* pBinds = pCurrent->GetBinds();
            AddBindings(pBinds);
        }

        m_EventHorizonDelay = TIMETYPE(0);
    }

    ZActionManager::~ZActionManager()
    {
        for (auto* pCurrent = m_kRoot.FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            ZUniMemory::Delete(pCurrent);
        }
    }

    void ZActionManager::SetJoinControllers(bool bJoinControllers, int iDeviceId)
    {
        m_JoinControllers = bJoinControllers;

        if (!m_JoinControllers && iDeviceId != -1)
        {
            m_kRoot.SetDevice(iDeviceId);
        }
    }
    
    bool ZActionManager::GetJoinControllers() const
    {
        return m_JoinControllers;
    }
    
    bool ZActionManager::AddBindings(const char* binds)
    {
        m_iBindMem += strlen(binds);
        ZTokenStream stream { binds };

        m_pkCurentBlock = &m_kRoot;

        if (ParseBinds(stream))
        {
            ++ZActionManager::m_iSeq;
            return true;
        }

        printf("Action --> Error parsing bindings (may cause crashes)\n");
        printf("Bindings: %s", binds);
        return false;
    }

    bool ZActionManager::LoadBindings(const char* pszFilename)
    {
        // NOTE: In PS2 build used static buffer for this file of size 4096 bytes
        // But in PC, XBox and other platforms we will allocate memory dynamically and it's ok

        const auto lSize = g_pSysFile->GetSize(pszFilename, false);
        if (lSize <= 0)
            return false;

        char* pBuffer = (char*)ZUniMemory::Allocate(sizeof(char) * lSize);
        const auto lRdyBytes = g_pSysFile->Load(pszFilename, pBuffer, lSize, 0, false);
        ZASSERT(lRdyBytes == lSize); // Not enough bytes rdy

        pBuffer[lSize] = '\0';
        const bool bApplied = AddBindings(pBuffer);
        ZUniMemory::Free(pBuffer);

        if (!bApplied)
        {
            printf("Error in action config file %s\n", pszFilename);
            // Yep, original code will say 'ok' in this case, it's non-fatal
        }

        return true;
    }
    
    const char* ZActionManager::GetKeyName(const char* pKey)
    {
        if (auto* pMapping = GetMapping(pKey))
        {
            return pMapping->GetKeyName();
        }

        return "";
    }
    
    const char* ZActionManager::GetSystemKeyName(const char* szAction, bool firstKey)
    {
        auto* pMapping = GetMapping(&m_kRoot, szAction);
        if (pMapping)
        {
            return pMapping->GetSystemKeyName(firstKey);
        }

        return "";
    }

    const TIMETYPE& ZActionManager::GetDisableInputTimes()
    {
        return m_EventHorizonDelay;
    }

    void ZActionManager::DisableInputTimer(double deltaTime)
    {
        const TIMETYPE sEmptyTime{};

        if (deltaTime == -1.f && m_EventHorizonDelay != sEmptyTime)
        {
            m_EventHorizonDelay = TIMETYPE();
        }

        if (m_EventHorizonDelay != sEmptyTime)
        {
            // I'm not sure about next line, better to do it directly in TIMETYPE
            const auto iConvertedTime = static_cast<int32_t>(deltaTime * -1024.0);
            m_EventHorizonDelay.secs = g_pSysInterface->m_fRealTime.secs - iConvertedTime;
        }
    }

    void ZActionManager::RevertToDefault()
    {
        m_kRoot.RevertToDefault();
    }

    void ZActionManager::SetDebugKeys(bool bDebugKeys)
    {
        m_bDebugKeys = bDebugKeys;
    }

    bool ZActionManager::GetDebugKeys() const
    {
        return m_bDebugKeys;
    }

    void ZActionManager::Enable()
    {
        m_bEnabled = true;
    }

    void ZActionManager::Disable()
    {
        m_bEnabled = false;
    }
    
    bool ZActionManager::IsEnabled() const
    {
        return m_bEnabled;
    }

    int32_t ZActionManager::SeqNr() const
    {
        return ZActionManager::m_iSeq;
    }

    ZActionMapTree* ZActionManager::GetMapping(const char* psName)
    {
        return GetMapping(&m_kRoot, psName);
    }

    ZActionMapTree* ZActionManager::GetMapping(ZActionMapTree* node, const char* psName)
    {
        if (!strcmp(node->GetName(), psName))
        {
            return node;
        }

        for (auto* pCurrent = node->FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            auto* pMapping = GetMapping(pCurrent, psName);
            if (pMapping)
            {
                return pMapping;
            }
        }

        return nullptr;
    }

    bool ZActionManager::ParseBinds(ZTokenStream& stream)
    {
        while (stream.Peek() != ZTokenStream::EToken::eEND)
        {
            if (!ParseAsignment(stream))
            {
                return false;
            }
        }

        return true;
    }

    ZActionMapTree* ZActionManager::ParseAsignment(ZTokenStream& stream)
    {
        char aBuffer[32] { 0 };

        if (stream.Peek() != ZTokenStream::EToken::eSYMBOL)
            return nullptr;
        
        if (stream.Swallow(ZTokenStream::EToken::eSYMBOL, &aBuffer[0]))
        {
            if (!stream.Swallow(ZTokenStream::EToken::eEQUAL))
            {
                return nullptr;
            }

            auto* pExpr = ParseExp(stream);
            if (pExpr)
            {
                auto* pCurrent = m_pkCurentBlock->FirstChild();
                for ( ; pCurrent; pCurrent = pCurrent->Next())
                {
                    const auto* psName = pCurrent->GetName();
                    if (!strcmp(psName, &aBuffer[0]))
                    {
                        break;
                    }
                }

                if (pCurrent)
                {
                    pCurrent->RemoveFromTree();

                    for (pCurrent = pCurrent->FirstChild(); pCurrent; pCurrent = pCurrent->Next())
                    {
                        DeleteTree(pCurrent);
                    }
                }

                m_pkCurentBlock->AddChild(pExpr);
                pExpr->SetName(&aBuffer[0]);
            }

            if (!pExpr)
            {
                pExpr = ParseBlock(stream, &aBuffer[0]);
            }

            if (!pExpr)
            {
                return nullptr;
            }

            if (!stream.Swallow(ZTokenStream::EToken::eSEMICOLON))
            {
                return nullptr;
            }

            return pExpr;
        }

        return nullptr;
    }

    ZActionMapTree* ZActionManager::ParseExp(ZTokenStream& stream)
    {
        char aBuffer1[32]{ 0 };
        char aBuffer2[32]{ 0 };
        int iDeviceId = -1;
        int iControlId = -1;
        float fNumVal = 0.0f;

        const auto eInitialToken = stream.Peek();
        ZActionMapTree* pCurrentExpr = nullptr;

        switch (eInitialToken)
        {
            case ZTokenStream::EToken::eVTRUE:
            case ZTokenStream::EToken::eVFALSE:
            {
                auto eToken = stream.Swallow();
                auto* pEntry = AllocMap();
                pEntry->Init(eToken);
                return pEntry;
            }

            case ZTokenStream::EToken::eGET:
            {
                if (!stream.Swallow(ZTokenStream::EToken::eGET))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eLPAR))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer2))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eRPAR))
                    return nullptr;

                auto* pMapping = GetMapping(aBuffer2);
                auto* pEntry = AllocMap();
                pEntry->Init(pMapping);
                return pEntry;
            }

            case ZTokenStream::EToken::eOR:
            case ZTokenStream::EToken::eAND:
            case ZTokenStream::EToken::eGT:
            case ZTokenStream::EToken::eLT:
            case ZTokenStream::EToken::ePLUS:
            case ZTokenStream::EToken::eMINUS:
            case ZTokenStream::EToken::eMULT:
            {
                auto eToken = stream.Swallow();

                auto* pExpr1 = ParseExp(stream);
                if (!pExpr1)
                    return nullptr;

                auto* pExpr2 = ParseExp(stream);
                if (!pExpr2)
                    return nullptr;

                auto* pEntry = AllocMap();
                pEntry->Init(eToken);
                pEntry->AddChild(pExpr1);
                pEntry->AddChild(pExpr2);
                return pEntry;
            }

            case ZTokenStream::EToken::eUMINUS:
            case ZTokenStream::EToken::eBANG:
            {
                auto eToken = stream.Swallow();
                auto* pExpr = ParseExp(stream);
                if (!pExpr)
                    return nullptr;

                if (eToken == ZTokenStream::EToken::eBANG)
                    pExpr->Not();
                else
                    pExpr->Minus();

                return pExpr;
            }

            case ZTokenStream::EToken::eNUMBER:
            {
                if (!stream.Swallow(ZTokenStream::EToken::eNUMBER, fNumVal))
                    return nullptr;

                auto* pEntry = AllocMap();
                pEntry->Init(fNumVal);
                return pEntry;
            }

            case ZTokenStream::EToken::eTAP:
            case ZTokenStream::EToken::eRELEASE:
            case ZTokenStream::EToken::eDOWNEDGE:
            case ZTokenStream::EToken::eFASTTAP:
            case ZTokenStream::EToken::eDOUBLECLICK:
            case ZTokenStream::EToken::eHOLDDOWN:
            case ZTokenStream::EToken::eCLICKHOLD:
            case ZTokenStream::EToken::ePRESS:
            case ZTokenStream::EToken::eHOLD:
            {
                auto eToken = stream.Swallow();

                if (!stream.Swallow(ZTokenStream::EToken::eLPAR))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer1))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eCOMMA))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer2))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eRPAR))
                    return nullptr;

                // TODO: Finsih this place after SysInput reversed
                // Smth like
                // if (SysInput::instance)
                // {
                //     SysInput::instance->GetInputIndices(aBuffer1, aBuffer2, &iDeviceId, &iControlId);
                // }

                auto* pEntry = AllocMap();
                pEntry->Init(eToken, iDeviceId, iControlId);
                return pEntry;
            }

            case ZTokenStream::EToken::eANA:
            {
                if (!stream.Swallow(ZTokenStream::EToken::eANA))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eLPAR))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer1))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eCOMMA))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer2))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eRPAR))
                    return nullptr;

                // TODO: Finsih this place after SysInput reversed
                // Smth like that
                // if (SysInput::instance)
                // {
                //     SysInput::instance->GetAnalogIndices(aBuffer1, aBuffer2, &iDeviceId, &iControlId);
                // }

                auto* pEntry = AllocMap();
                pEntry->Init(ZTokenStream::EToken::eANA, iDeviceId, iControlId);
                return pEntry;
            }

            case ZTokenStream::EToken::eREL:
            {
                if (!stream.Swallow(ZTokenStream::EToken::eREL))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eLPAR))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer1))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eCOMMA))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eSYMBOL, aBuffer2))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eRPAR))
                    return nullptr;

                // TODO: Finsih this place after SysInput reversed
                
                auto* pEntry = AllocMap();
                pEntry->Init(ZTokenStream::EToken::eREL, iDeviceId, iControlId);
                return pEntry;
            }

            case ZTokenStream::EToken::eSEQ:
            {
                if (!stream.Swallow(ZTokenStream::EToken::eSEQ))
                    return nullptr;

                if (!stream.Swallow(ZTokenStream::EToken::eLPAR))
                    return nullptr;

                pCurrentExpr = AllocMap();
                pCurrentExpr->Init(ZTokenStream::EToken::eSEQ);
                break;
            }

            default:
                return pCurrentExpr;
        }

        // Arguments sequence handler
        while (stream.Peek() != ZTokenStream::EToken::eRPAR)
        {
            auto* pExpr = ParseExp(stream);
            if (!pExpr)
                return nullptr;

            pCurrentExpr->AddChild(pExpr);

            if (stream.Peek() == ZTokenStream::EToken::eRPAR)
                break;

            if (!stream.Swallow(ZTokenStream::EToken::eCOMMA))
                return nullptr;
        }

        if (stream.Swallow(ZTokenStream::EToken::eRPAR))
            return pCurrentExpr;

        return nullptr;
    }

    ZActionMapTree* ZActionManager::ParseBlock(ZTokenStream& stream, char* pBuffer)
    {
        if (stream.Peek() != ZTokenStream::EToken::eLBRACE)
            return nullptr;

        if (!stream.Swallow(ZTokenStream::EToken::eLBRACE))
            return nullptr;

        ZActionMapTree* pTargetBlock = m_pkCurentBlock->FirstChild();
        for (; pTargetBlock; pTargetBlock = pTargetBlock->Next())
        {
            const char* psName = pTargetBlock->GetName();
            if (psName && strcmp(psName, pBuffer) == 0)
            {
                // found
                break;
            }
        }

        // Make new block if not found
        if (!pTargetBlock)
        {
            pTargetBlock = AllocMap();
            pTargetBlock->SetName(pBuffer);
            m_pkCurentBlock->AddChild(pTargetBlock);
        }

        ZActionMapTree* pPreviousBlock = m_pkCurentBlock;
        m_pkCurentBlock = pTargetBlock;

        // Parse inside braces
        while (stream.Peek() != ZTokenStream::EToken::eRBRACE)
        {
            if (!ParseAsignment(stream))
            {
                // Fallback on failure
                m_pkCurentBlock = pPreviousBlock;
                return nullptr;
            }
        }

        if (!stream.Swallow(ZTokenStream::EToken::eRBRACE))
        {
            m_pkCurentBlock = pPreviousBlock;
            return nullptr;
        }

        // Go back to parent
        m_pkCurentBlock = m_pkCurentBlock->Parent();

        return pTargetBlock;
    }

    void ZActionManager::DeleteTree(ZActionMapTree* pTree)
    {
        for (auto* pCurrent = pTree->FirstChild(); pCurrent; pCurrent = pCurrent->Next())
        {
            DeleteTree(pCurrent);
        }

        FreeMap(pTree);
    }

    void ZActionManager::FreeMap(ZActionMapTree* pTree)
    {
        if (pTree)
        {
            ZUniMemory::Delete(pTree);
        }
    }

    ZActionMapTree* ZActionManager::AllocMap()
    {
        return ZUniMemory::New<ZActionMapTree>();
    }

    bool Action::Initialize()
    {
        Action::instance = ZUniMemory::New<ZActionManager>();
        return Action::instance != nullptr;
    }

    void Action::Free()
    {
        if (Action::instance)
        {
            ZUniMemory::Delete(Action::instance);
            Action::instance = nullptr;
        }
    }

    // Statics & PC relations
    STATIC_CLASS_VAR_IMPL(Action, ZActionManager*, instance, 0x008ACAC4, nullptr);
    STATIC_CLASS_VAR_IMPL(ZActionManager, int32_t, m_iSeq, 0x008ACACC, 0);
}