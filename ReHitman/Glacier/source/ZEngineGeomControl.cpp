#include <Glacier/Glacier.h>
#include <Glacier/ZEngineGeomControl.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>


namespace Glacier
{
    STATIC_GLOBAL_VAR(ZEngineGeomControl*, g_pGeomControl, 0x007F5AA8, nullptr);

    ZEngineGeomControl::ZEngineGeomControl()
        : m_bChangeDetection{false}
    {
    }

    bool ZEngineGeomControl::GetChangeDetection() const
    {
        return m_bChangeDetection;
    }

    bool ZEngineGeomControl::GeomMoved(ZBaseGeom* pBaseGeom)
    {
        if (GetChangeDetection() && (pBaseGeom->Control() & ZCHASMOVED) == 0)
        {
            UpdateMovedGeoms();

            ZASSERT(m_lNrMovedGeoms < MAX_MOVED_GEOMS_NR);
            m_MovedGeoms[m_lNrMovedGeoms] = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);
            pBaseGeom->SetControl(ZCHASMOVED | 0x3000000u, 0);
            ++m_lNrMovedGeoms;
        }

        return true;
    }

    void ZEngineGeomControl::UpdateMovedGeoms()
    {
        int lNrMovedGeoms = 0;

        for (int i = 0; i < m_lNrMovedGeoms; ++i)
        {
            auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(m_MovedGeoms[i]);
            if (pBaseGeom)
            {
                ZASSERT(pBaseGeom->Control() & ZCHASMOVED);
                pBaseGeom->SetControl(0, ZCHASMOVED);
                m_MovedGeoms[lNrMovedGeoms++] = m_MovedGeoms[i];
            }
        }

        if (GetChangeDetection())
        {
            m_lNrMovedGeoms = lNrMovedGeoms;

            for (int i = 0; i < m_lNrMovedGeoms; ++i)
            {
                auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(m_MovedGeoms[i]);
                ZASSERT(pBaseGeom);
                pBaseGeom->UpdateMovedGeom();
            }

            JonsLights();
        }

        m_lNrMovedGeoms = 0;
    }

    void ZEngineGeomControl::Clear()
    {
        m_lNrMovedGeoms = 0;
    }

    void ZEngineGeomControl::JonsLights()
    {
        ZBaseGeom* pChangedLights[MAX_MOVED_GEOMS_NR];
        uint32_t lNrLights = 0;

        for (int i = 0; i < m_lNrMovedGeoms; ++i)
        {
            auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(m_MovedGeoms[i]);
            ZASSERT(pBaseGeom);

            if (!pBaseGeom || (pBaseGeom->Control() & (ZCINACTIVE | ZCHIDDEN)) != 0)
            {
                continue;
            }

            if (pBaseGeom->IsDerivedFrom<ZLIGHT>())
            {
                if (pBaseGeom->ListId() != 0)
                {
                    ZASSERT(lNrLights < MAX_MOVED_GEOMS_NR);
                    pChangedLights[lNrLights++] = pBaseGeom;
                }

                continue;
            }

            if (pBaseGeom->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroup = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
                ZASSERT(pGroup);

                if (!pGroup)
                {
                    continue;
                }

                for (auto* pCurrentGeom = pGroup->BaseGeom(); pCurrentGeom; pGroup->RecurGetNext(&pCurrentGeom))
                {
                    if (pCurrentGeom->ListId() == 0 || (pCurrentGeom->Control() & (ZCINACTIVE | ZCHIDDEN)) != 0)
                    {
                        continue;
                    }

                    if ((pCurrentGeom->Control() & 0x3000000u) != 0x3000000u)
                    {
                        pCurrentGeom->SetControl(0x3000000u, 0);
                    }

                    if (pCurrentGeom->IsDerivedFrom<ZLIGHT>())
                    {
                        ZASSERT(lNrLights < MAX_MOVED_GEOMS_NR);
                        pChangedLights[lNrLights++] = pCurrentGeom;
                    }
                }
            }
        }

        UpdateChangedLights(pChangedLights, lNrLights);
    }

    void ZEngineGeomControl::UpdateChangedLights(ZBaseGeom** pBaseGeomList, uint32_t lNrLights)
    {
        if (!g_pEngineData) return; // DronCode: in original code no checks for this, but it's better for testing purposes

        if (g_pEngineData->GetListUser() && lNrLights)
        {
            for (int i = 0; i < lNrLights; ++i)
            {
                ZASSERT(pBaseGeomList[i] && pBaseGeomList[i]->IsDerivedFrom<ZLIGHT>());
                pBaseGeomList[i]->LightNotifyPotentialDetachment(true);
            }
        }
    }

    ZEngineGeomControl& ZEngineGeomControl::GetInstance() 
    {
        return *g_pGeomControl;
    }
}
