#include <Glacier/ZEngineGeomControl.h>
#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <G1ConfigurationService.h>
#include <Glacier/Glacier.h>

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
        // TODO: Finish me
    }

    void ZEngineGeomControl::UpdateChangedLights(ZBaseGeom** pBaseGeomList, uint32_t lNrLights)
    {
        //if (g_pEngineData->GetListUser() && lNrLights) // TODO: USEME
        if (lNrLights)
        {
            for (int i = 0; i < lNrLights; ++i)
            {
                ZASSERT(pBaseGeomList[i] && pBaseGeomList[i]->IsDerivedFrom<ZLIGHT>());
                // pBaseGeomList[i]->LightNotifyPotentialDetachment(true); // TODO: Finish me
            }
        }
    }

    ZEngineGeomControl& ZEngineGeomControl::GetInstance() 
    {
        return *g_pGeomControl;
    }
}
