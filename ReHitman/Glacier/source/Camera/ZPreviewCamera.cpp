#include <Glacier/Camera/ZPreviewCamera.h>
#include <Glacier/Camera/KeyControlGeom.h>
#include <Glacier/Debug/ZDebugFloat.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        ZDebugInt g_lCamColiCheckDynamic { "cam_coli_check_dynamic", "Colision against dynamic objects", 0, -1000000, 1000000, 1, nullptr }; // PC
        ZDebugInt g_lCamColiCheckStatic { "cam_coli_check_static", "Colision against static objects", 0, -1000000, 1000000, 1, nullptr }; // PC
        ZDebugInt g_lCamColiEnable { "cam_coli_enable", "Coli Line is enabled", 0, -1000000, 1000000, 1, nullptr };
        ZDebugFloat g_fCamColiLen { "cam_coli_len", "Length of line", 0.0f, FLT_MAX, 1.0f, nullptr, 1.0f };
    }

    ZPreviewCamera::ZPreviewCamera() : CBaseEvent<ZCAMERA>()
    {
    }

    ZPreviewCamera::~ZPreviewCamera() = default;

    const RTP::ZPropertyInfo& ZPreviewCamera::GetProperties() const
    {
        return ZPreviewCamera::Info;
    }

    void ZPreviewCamera::Init()
    {
        ZASSERT(m_pBaseGeom);
    }

    void ZPreviewCamera::FrameUpdate()
    {
        ZGEOM* pGeom = GetGeom();

        if (g_pEngineData->IsPaused())
        {
            ZASSERT(pGeom);
            if (pGeom)
            {
                pGeom->ClassFrameUpdate();
            }
        }

        ZASSERT(pGeom);
        if (!pGeom)
        {
            return;
        }

        // PC 0x00535690: keyboard movement of the preview camera
        KeyControlGeom(pGeom->BaseGeom(), 100.0f);

        if (g_lCamColiEnable)
        {
            SExtendedImpactInfo info;

            ZVector3 vPos;
            pGeom->GetRootPoint(vPos);

            // Downward camera collision probe for the level editor (PC flt_7FD008)
            ZVector3 vDir(0.0f, 0.0f, static_cast<float>(g_fCamColiLen));
            pGeom->GetRootVect(vDir);

            g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);
            const bool bCheckStatic = g_lCamColiCheckStatic != 0;
            const bool bCheckDynamic = g_lCamColiCheckDynamic != 0;

            auto* pCollision = ZCollisionBase::s_pCollisionBase;
            ZASSERT(pCollision);
            if (pCollision)
            {
                pCollision->CalcLineColi(
                    &info,
                    eGlobalTreeType::GT_StdObjs,
                    vPos,
                    vDir,
                    false,
                    0xFFFFFFFFu,
                    bCheckStatic,
                    bCheckDynamic);
            }
            g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);

            info.fPercent = 1.0f;
            g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);
            if (pCollision)
            {
                pCollision->CalcLineColi(
                    &info,
                    eGlobalTreeType::GT_StdObjs,
                    vPos,
                    vDir,
                    false,
                    0xFFFFFFFFu,
                    bCheckStatic,
                    bCheckDynamic);
            }
            g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);
        }
    }

#   pragma region " --- RTTI --- "
    DEFINE_ROUT_CLASS(ZPreviewCamera, ZCAMERA, PreviewCamera, 272, 0, 0x00814DE8, nullptr, ZCAMERA);
#   pragma endregion
}
