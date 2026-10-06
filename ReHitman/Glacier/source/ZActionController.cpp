#include <Glacier/ZActionController.h>

#include <Glacier/ZAction.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Com/Globals.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>

namespace Glacier
{
    REFTAB32* ZActionController::m_pDeleteActions = nullptr;

    ZActionController::ZActionController()
    {
        m_iEvaluators = 0;
        for (int i = 0; i < 4; ++i)
        {
            m_alNextAction[i] = 0;
            m_apDistCheck[i] = nullptr;
        }
        m_msgAddEvaluator = 0;
        m_msgRemoveEvaluator = 0;

        if (g_pEngineData)
        {
            if (CCom* pCom = g_pEngineData->GetSceneCom())
                pCom->SetVal("rActionController", static_cast<int>(GetRef()));
        }
    }

    ZActionController::~ZActionController()
    {
    }

    const RTP::ZPropertyInfo& ZActionController::GetProperties() const
    {
        return ZActionController::Info;
    }

    void ZActionController::Init()
    {
        if (CCom* pCom = g_pEngineData->GetSceneCom())
            pCom->SetVal("rActionControllerGeom", static_cast<int>(GetGeom()->GetRef()));

        m_msgAddEvaluator = g_pEngineData->RegisterZMsg("AddActionEvaluator", 0, __FILE__, __LINE__);
        m_msgRemoveEvaluator = g_pEngineData->RegisterZMsg("RemoveActionEvaluator", 0, __FILE__, __LINE__);

        for (int i = 0; i < 4; ++i)
        {
            m_apDistCheck[i] = nullptr;
            m_alNextAction[i] = 0;
        }
        m_iEvaluators = 0;

        m_DistCheck.Init();
    }

    void ZActionController::End()
    {
        if (CCom* pCom = g_pEngineData->GetSceneCom())
        {
            pCom->RemoveVal("rActionController", 0);
            pCom->RemoveVal("rActionControllerGeom", 0);
        }

        for (int i = 0; i < m_iEvaluators; ++i)
        {
            ZUniMemory::Delete(m_apDistCheck[i]);
            m_apDistCheck[i] = nullptr;
        }
        m_iEvaluators = 0;
    }

    int ZActionController::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        if (command == m_msgAddEvaluator)
        {
            if (data)
                AddEvaluator(*static_cast<unsigned int*>(data));
        }
        else if (command == m_msgRemoveEvaluator)
        {
            if (data)
                RemoveEvaluator(*static_cast<unsigned int*>(data));
        }
        return 0;
    }

    void ZActionController::Add(ZAction* pAction)
    {
        const ZREF rRef = pAction->GetRef();

        if (!m_DistCheck.Exists(rRef))
            m_DistCheck.Add(rRef);

        for (int i = 0; i < m_iEvaluators; ++i)
        {
            if (!m_apDistCheck[i]->Exists(rRef))
                m_apDistCheck[i]->Add(rRef);
        }
    }

    void ZActionController::Remove(ZAction* pAction)
    {
        const ZREF rRef = pAction->GetRef();

        const int lIndex = m_DistCheck.Find(rRef);
        if (lIndex != -1)
            m_DistCheck.Remove(lIndex);

        for (int i = 0; i < m_iEvaluators; ++i)
        {
            const int lEval = m_apDistCheck[i]->Find(rRef);
            if (lEval != -1)
                m_apDistCheck[i]->Remove(lEval);
        }
    }

    bool ZActionController::UpdateSingleObject(ZAction* pAction)
    {
        const ZREF rRef = pAction->GetRef();
        for (int i = 0; i < m_iEvaluators; ++i)
            m_apDistCheck[i]->UpdateSingleObject(rRef);
        return true;
    }

    void ZActionController::AddEvaluator(unsigned int lRef)
    {
        if (m_iEvaluators < 4)
        {
            ZDistCheck* pCheck = ZUniMemory::New<ZDistCheck>();
            pCheck->Init();
            m_apDistCheck[m_iEvaluators] = pCheck;
            pCheck->Migrate(&m_DistCheck);
            pCheck->SetOtherObj(lRef);
            ++m_iEvaluators;
        }
        else
        {
            printf("Warning: Trying to add more than 4 action evaluators\n");
        }
    }

    void ZActionController::RemoveEvaluator(unsigned int lRef)
    {
        for (int i = m_iEvaluators - 1; i >= 0; --i)
        {
            if (m_apDistCheck[i]->GetOtherObj() != lRef)
                continue;

            ZUniMemory::Delete(m_apDistCheck[i]);

            // Compact: shift the trailing evaluators down one slot.
            for (int k = i; k < m_iEvaluators - 1; ++k)
                m_apDistCheck[k] = m_apDistCheck[k + 1];
            m_apDistCheck[m_iEvaluators - 1] = nullptr;
            --m_iEvaluators;
        }
    }

    ZActionController* ZActionController::GetCurrentController(
        bool bCreateActionControllerIfItNotCreatedYet)
    {
        CCom* pCom = g_pEngineData ? g_pEngineData->GetSceneCom() : nullptr;
        if (!pCom)
            return nullptr;

        ZREF rController = 0;
        pCom->GetVal("rActionController", reinterpret_cast<int*>(&rController));
        if (rController)
            return static_cast<ZActionController*>(ZEventBase::RefToPtr(rController));

        if (!bCreateActionControllerIfItNotCreatedYet)
            return nullptr;

        ZGEOM* pGeom = ZROOT->CreateGeom("ActionController", ZLIST::m_TypeId, true);
        if (!pGeom)
            return nullptr;

        ZBaseConRout* pRout = pGeom->AddEvent("ZLIST_ActionController");
        if (!pRout)
            return nullptr;

        ZActionController* pController = static_cast<ZActionController*>(static_cast<ZEventBase*>(pRout));
        pController->DoInit();
        return pController;
    }

    void ZActionController::FrameUpdate()
    {
        for (int i = 0; i < m_iEvaluators; ++i)
            m_apDistCheck[i]->Update();

        if (m_pDeleteActions)
        {
            // Deletion is deferred exactly as the action requested through
            // ZAction::SafeDelete: each queued event receives its "ActionRemove"
            // rout command (ZEventBase::Call case 0x20), which ends the action.
            for (int n = 0; n < m_pDeleteActions->Count(); ++n)
            {
                ZEventBase* pEvent = ZEventBase::RefToPtr((*m_pDeleteActions)[n]);
                if (pEvent)
                    pEvent->Call(0x20u, nullptr, static_cast<uint16_t>(ZAction::s_msgRemove));
            }

            m_pDeleteActions->Clear();
            ZUniMemory::Delete(m_pDeleteActions);
            m_pDeleteActions = nullptr;
        }

        for (int i = 0; i < m_iEvaluators; ++i)
        {
            ZDistCheck* pCheck = m_apDistCheck[i];
            const int lTotal = pCheck->Count(7);

            int lStep = lTotal / 5;
            if (lStep < 1)
                lStep = 1;

            m_alNextAction[i] %= lTotal > 0 ? lTotal : 1;
            ZASSERT(m_alNextAction[i] >= 0);

            // Walk a contiguous block of this evaluator's table this frame so the
            // whole set is refreshed within a few frames.
            int lEnd = m_alNextAction[i] + lStep;
            if (lEnd > lTotal)
                lEnd = lTotal;

            ZGEOM* pOther = ZGEOM::RefToPtr(pCheck->GetOtherObj());
            for (int k = m_alNextAction[i]; k < lEnd; ++k)
            {
                ZEventBase* pEvent = ZEventBase::RefToPtr(pCheck->Get(k));
                if (!pEvent)
                {
                    pCheck->Remove(k);
                    --k;
                    --lEnd;
                    continue;
                }
                // PS2 vtable slot +196 = ZAction::FrameUpdate(ZGEOM*) (the action's
                // own per-range update, given the target geom it is evaluated against).
                static_cast<ZAction*>(pEvent)->ActionFrameUpdate(pOther);
            }

            m_alNextAction[i] += lStep;
        }
    }

    void ZActionController::LoadObject(IInputSerializerStream& stream)
    {
        m_DistCheck.LoadObject(stream);
        for (int i = 0; i < m_iEvaluators; ++i)
            m_apDistCheck[i]->LoadObject(stream);
    }

    void ZActionController::SaveObject(IOutputSerializerStream& stream)
    {
        m_DistCheck.SaveObject(stream);
        for (int i = 0; i < m_iEvaluators; ++i)
            m_apDistCheck[i]->SaveObject(stream);
    }

#pragma region " --- RTTI --- "
    DEFINE_ROUT_CLASS(ZActionController, ZLIST, ActionController, 0, 0, 0x811DE8, nullptr, ZEventBase);
#pragma endregion
}
