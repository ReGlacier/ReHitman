#include <Glacier/Geom/ZLIST.h>
#include <Glacier/System/ZSysInterface.h>
#include <SI/SI_Zlist.h>


namespace Glacier
{
    namespace
    {
        ZLIST* ZlistResolve(ZREF rList)
        {
            auto* pList = ZGEOM::RefToPtr(rList);
            if (pList && pList->IsDerivedFrom<ZLIST>())
            {
                return static_cast<ZLIST*>(pList);
            }

            return nullptr;
        }
    }

    ZREF Zlist__Getrndref(ZREF rList)
    {
        auto* pZList = ZlistResolve(rList);
        if (!pZList)
        {
            return 0;
        }

        const int lCount = pZList->Script_GetCount();
        if (lCount)
        {
            return static_cast<ZREF>(pZList->Script_GetRefNr(g_pSysInterface->Rand(nullptr, 0) % lCount));
        }

        return 0;
    }

    int Zlist__Getcount(ZREF rList)
    {
        auto* pZList = ZlistResolve(rList);
        if (!pZList)
        {
            return 0;
        }

        return pZList->Script_GetCount();
    }

    ZREF Zlist__Getref(ZREF rList, int index)
    {
        auto* pZList = ZlistResolve(rList);
        if (!pZList)
        {
            return 0;
        }

        return static_cast<ZREF>(pZList->Script_GetRefNr(index));
    }

    int Zlist__Getindex(ZREF rList, ZREF rItem)
    {
        auto* pZList = ZlistResolve(rList);
        if (!pZList)
        {
            return 0;
        }

        return pZList->m_pZList->GetIndex(rItem);
    }

    bool Zlist__Contains(ZREF rList, ZREF rItem)
    {
        auto* pZList = ZlistResolve(rList);
        if (!pZList)
        {
            return true;
        }

        return pZList->m_pZList->GetIndex(rItem) != -1;
    }

    void Zlist__Add(ZREF rList, ZREF rItem)
    {
        if (auto* pZList = ZlistResolve(rList))
        {
            pZList->AddGeom(rItem);
        }
    }

    void Zlist__Remove(ZREF rList, ZREF rItem)
    {
        if (auto* pZList = ZlistResolve(rList))
        {
            pZList->RemoveGeomById(rItem);
        }
    }

    void Zlist__Clear(ZREF rList)
    {
        if (auto* pZList = ZlistResolve(rList))
        {
            pZList->m_pZList->Clear();
        }
    }

    void Zlist__Sendeventtolist(ZREF rList, ZSC_EVENT event)
    {
        if (auto* pZList = ZlistResolve(rList))
        {
            // DronCode: it's weird due event contains zmsg & pData, but we are not using pData from event
            pZList->SendCommandToList(event.zmsg, nullptr);
        }
    }
}
