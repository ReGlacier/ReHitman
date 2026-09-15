#include <Glacier/GameBase/SSeer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/ZSTL/ZMath.h>

#include <cmath>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    SSeer::SSeer()
        : fLastTimeFraction(0.0f)
    {
    }

    void SSeer::Dump()
    {
        const float cone = std::acos(fVisionCone) * 57.2957795f;
        const float verticalCone = std::acos(fVerticalVisionCone) * 57.2957795f;
        const MYSTR seerName = pSeer->CalcTotalName(false);

        printf("Seer: %s\n", static_cast<const char*>(seerName));
        printf("\tRange    : %.2f\n", fVisionRange);
        printf("\tCone     : %.2f\n", cone);
        printf("\tVert Cone: %.2f\n", verticalCone);
        printf("\tLow  cell: %d, %d, %d\n", m_cLowCell.m_X, m_cLowCell.m_Y, m_cLowCell.m_Z);
        printf("\tHigh cell: %d, %d, %d\n", m_cHighCell.m_X, m_cHighCell.m_Y, m_cHighCell.m_Z);
        printf("\tList of Seeables:\n");

        ZCheckVisible& checkVisible = ZCheckVisible::Instance();
        for (uint32_t i = 0; i < Vision::MAXSEEABLES; ++i)
        {
            if (!Objects[i].GetVisible())
            {
                continue;
            }

            ZSeeable& seeable = checkVisible.GetSeeable(static_cast<uint8_t>(i));
            if (seeable.IsDecal())
            {
                printf("\t\tDecal of type %d at (%.2f, %.2f, %.2f)\n",
                    seeable.wDecalType, seeable.pos.x, seeable.pos.y, seeable.pos.z);
            }
            else
            {
                const MYSTR name = seeable.pSeeable->CalcTotalName(false);
                printf("\t\t%s at (%.2f, %.2f, %.2f)\n",
                    static_cast<const char*>(name), seeable.pos.x, seeable.pos.y, seeable.pos.z);
            }
        }
    }

    void SSeer::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        if (bSaving)
        {
            memset(this, 0, sizeof(SSeer));
            ttLastTimeChecked = g_pSysInterface->m_fRealTime;
        }

        ZREF ref = 0;
        if (bSaving)
        {
            stream.Exchange("pSeer", ref);
            pSeer = ZGEOM::RefToPtr(ref);
        }
        else
        {
            ref = pSeer ? pSeer->GetRef() : 0;
            stream.Exchange("pSeer", ref);
        }
        stream.Exchange("bDisabled", bDisabled);
        stream.Exchange("lVisibleTypes", lVisibleTypes);
        stream.Exchange("fVisionRange", fVisionRange);
        stream.Exchange("fVisionCone", fVisionCone);
        stream.Exchange("fVerticalVisionCone", fVerticalVisionCone);
        stream.Exchange("fLightMultiplier", fLightMultiplier);
        stream.Exchange("fSixthSenseRange", fSixthSenseRange);
        stream.ExchangeArray("vOffset", vOffset.Get(), 3);
        stream.Exchange("lFlag", lFlag);
    }

    int32_t SSeer::CalculateTimeElapsed(TIMETYPE& time)
    {
        const float fTime = static_cast<float>(time - ttLastTimeChecked) * 60.0f;
        int32_t lTime = static_cast<int32_t>(fTime);
        fLastTimeFraction += fTime - static_cast<float>(lTime);
        if (fLastTimeFraction > 1.0f)
        {
            fLastTimeFraction -= 1.0f;
            ++lTime;
        }
        if (lTime >= 128)
        {
            return 127;
        }
        return lTime;
    }
}
