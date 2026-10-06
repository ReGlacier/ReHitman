#include <Glacier/Geom/ZBloodStains.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/Physics/eGlobalTreeType.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    // ZActor::ShootIntoGround (PC 0x507A70) builds the foot dust / impact decal
    // through this call: it hands over the contact COLI and feeds the result to
    // ZGEOM::RefToPtr() before storing it in ZIKLNKOBJ::m_pFootDustTemplate.
    // ZProjectileBase::ShotImpact (PC 0x54012D) and
    // ZHM3Actor::CreateBloodPoolAndCheckRetry (PC 0x63C476) use the same return
    // value through ZEngineDataBase::GeomRefToPtr(), i.e. it is a geom ZREF.
    //
    // The PC image only keeps a 31-byte wrapper at 0x4F5D00 (it checks the position
    // pointer and tail-calls a ZMaterialDescriptionDB lookup). The body below
    // follows the fully reversed XBOX_KL2 (0x823F5028), XBOX_MiniNinjas 0x821C75C8)
    // and iOS (0x1003652EC) implementations, which fill the COLI hit line from the
    // given position and cast it straight down (-200). The colli parameters
    // (GT_StdObjs, GeomConMask 3, lGeomType -1, static only) match the sibling
    // helper PC 0x4F53A0 in the same zprojectmarks translation unit, and the
    // returned ref is ZTreeGroup::ChkLineColi's pColi->ColiRef (see
    // Geom/ZTreeGroup.cpp), i.e. the ref of the geom that was hit.
    ZREF ZBloodStains::MakeBloodStainColiCheck(const float* pPos, COLI& rColi) const
    {
        if (!pPos)
            return 0;

        rColi.lp = *reinterpret_cast<const ZVector3*>(pPos);
        rColi.ln = ZVector3 { 0.0f, -200.0f, 0.0f };

        if (!ZROOT->ChkLineColi(&rColi, GT_StdObjs, 3, -1, true, false))
            return 0;

        return rColi.ColiRef;
    }
}
