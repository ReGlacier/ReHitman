#include <SI/SI_Zgeom.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/ZMath.h>
#include <SI/SI_Common.h>


namespace Glacier
{
    v3 Zgeom__Getsize(ZREF rGeom)
    {
        ZVector3 vSize{};

        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return vSize;

        pGeom->GetSize(vSize);
        return vSize;
    }

    void Zgeom__Setposition(ZREF rGeom, float x, float y, float z)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return;

        pGeom->SetWorldPosition({ x, y, z });
    }

    v3 Zgeom__Getposition(ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return v3 { 0.f, 0.f, 0.f };

        Glacier::ZVector3 vWorldPos {};
        pGeom->GetWorldPosition(vWorldPos);
        return v3 { vWorldPos.x, vWorldPos.y, vWorldPos.z };
    }

    void Zgeom__Getlocalpoint(ZREF rGeom, v3& point)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return;

        pGeom->GetLocalPoint(point);
    }

    void Zgeom__Getposdir(ZREF rGeom, v3& position, v3& direction)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
        {
            direction = { 0.0f, 0.0f, 1.0f };
            position = { 0.0f, 0.0f, 0.0f };
            return;
        }

        ZMat3x3 mMat;

        pGeom->GetRootTM(mMat, position);
        direction = mMat.Row(0);
    }

    void Zgeom__Setposdir(ZREF rGeom, v3& position, v3& direction)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom) // || !pGeom->IsDerivedFrom<ZActor>()
            return;

        ZVector3 vUp { 0.0f, 1.0f, 0.0f };
        ZMat3x3 mMat;
        createmat(mMat, direction, vUp);

        pGeom->SetRootTM(mMat, position);
    }

    bool Zgeom__Getclosestposdirinbox(ZREF rBox, ZREF rGeom, ZREF rTarget, float edgeDist, v3& position, v3& direction)
    {
        std::ignore = rBox; // unused

        auto* pBox = ref_cast<ZBoxPrimitive>(rGeom);
        auto* pTarget = GetGeom(rTarget);

        if (!pTarget || !pBox || !pTarget->IsDerivedFrom<ZGEOM>())
            return false;

        ZVector3 vTarget, vLocalPoint;
        pTarget->GetRootPoint(vTarget);
        pBox->GetLocalPoint(vLocalPoint);

        return pBox->GetClosestPosDirInBox(vTarget, edgeDist, position, direction);
    }

    float Zgeom__Getdistancetoobject(ZREF rGeom, ZREF rTarget)
    {
        auto* pGeom = GetGeom(rGeom);
        auto* pTarget = GetGeom(rTarget);
        if (!pGeom || !pTarget)
            return 0.0f;

        return pGeom->GetDistanceToObject(pTarget);
    }

    float Zgeom__Getdistancetopos(ZREF rGeom, v3 position)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0.0f;

        return pGeom->GetDistanceToPos(position);
    }

    bool Zgeom__Hascontroller(ZREF rGeom, const char* controllerName)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom) return false;

        return pGeom->FindEvent(controllerName) != nullptr;
    }

    ZREF Zgeom__Getcontroller(ZREF rGeom, const char* controllerName)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0;

        auto* pEvent = pGeom->FindEvent(controllerName);
        if (!pEvent)
            return 0;

        return pEvent->GetRef();
    }

    float Zgeom__Getangletoobject(ZREF rGeom, ZREF rTarget)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0;

        return pGeom->GetAngleToObject(rTarget);
    }

    float Zgeom__Getangletodir(ZREF rGeom, v3 direction)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0.0f;

        return pGeom->GetAngleToDir(direction);
    }

    float Zgeom__Getangletogeomdir(ZREF rGeom, ZREF rTarget)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0.0f;

        return pGeom->GetAngleToGeomDir(rTarget);
    }

    void Zgeom__Copyobjectposdir(ZREF rGeom, ZREF rTarget)
    {
        auto* pGeom = GetGeom(rGeom);
        auto* pTarget = GetGeom(rTarget);
        if (!pGeom || !pTarget)
            return;

        ZVector3 vPos;
        ZMat3x3 mMat;

        pTarget->GetRootTM(mMat, vPos);
        pGeom->SetRootTM(mMat, vPos);
    }

    void Zgeom__Copyobjectpos(ZREF rGeom, ZREF rTarget)
    {
        auto* pGeom = GetGeom(rGeom);
        auto* pTarget = GetGeom(rTarget);
        if (!pGeom || !pTarget)
            return;

        ZVector3 vTarget, vGeom;
        ZMat3x3 mTarget, mGeom;

        pTarget->GetRootTM(mTarget, vTarget);
        pGeom->GetRootTM(mGeom, vGeom);

        pGeom->SetRootTM(mGeom, vTarget);
    }

    void Zgeom__Copyobjectdir(ZREF rGeom, ZREF rTarget)
    {
        auto* pGeom = GetGeom(rGeom);
        auto* pTarget = GetGeom(rTarget);
        if (!pGeom || !pTarget)
            return;

        ZVector3 vTarget, vGeom;
        ZMat3x3 mTarget, mGeom;

        pTarget->GetRootTM(mTarget, vTarget);
        pGeom->GetRootTM(mGeom, vGeom);

        pGeom->SetRootTM(mTarget, vGeom);
    }

    v3 Zgeom__Getobjectrelpos(ZREF rGeom, float x, float y, float z)
    {
        ZVector3 vPos;
        auto* pGeom = GetGeom(rGeom);
        if (pGeom)
        {
            pGeom->GetRootPoint(vPos);
        }

        return vPos;
    }

    bool Zgeom__Checkworldpointinside(ZREF rGeom, v3 worldPosition)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return false;

        ZVector3 vLocalPoint = worldPosition;
        pGeom->GetLocalPoint(vLocalPoint);

        return pGeom->CheckPointInside(vLocalPoint, 0.0f);
    }

    bool Zgeom__Isinview(ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return false;

        return (pGeom->Control() & ZCINVIEW) != 0;
    }

    void Zgeom__Printname(ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return;

        MYSTR sName = pGeom->CalcTotalName(true);
        ZINFO("%s", sName.String);
    }

    float Zgeom__Getdistancetoitem(ZREF rGeom, ZREF rItem)
    {
        auto* pGeom = GetGeom(rGeom);
        auto* pItem = reinterpret_cast<ZItem*>(GetGeom(rItem));
        if (!pGeom || !pItem)
            return 0.0f;

        ZVector3 vGeom, vItem;
        ZMat3x3 mItem;
        pItem->GetMainItemRootTM(mItem, vItem);
        pGeom->GetLocalPoint(vItem);
        return vdist(vItem, pGeom->Cen());
    }

    void Zgeom__Makeinactive(ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return;

        pGeom->MakeInactive();
    }

    void Zgeom__Makeactive(ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return;

        pGeom->MakeActive();
    }

    void Zgeom__Hide(ZREF rGeom, bool bHide)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return;

        pGeom->Hide(bHide);
    }

    ZREF Zgeom__Getcurrentroom(ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0;

        return pGeom->BaseGeom()->GetOwnerRoom()->GetRef();
    }

    int Zgeom__Numberofexitsfromgeomscurrentroom(ZREF _, ZREF rGeom)
    {
        auto* pGeom = GetGeom(rGeom);
        if (!pGeom)
            return 0;

        return pGeom->BaseGeom()->GetOwnerRoom()->m_lNrExits;
    }
}
