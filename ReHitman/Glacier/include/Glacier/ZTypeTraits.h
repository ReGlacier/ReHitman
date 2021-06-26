#pragma once

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/ZCAMERA.h>

#include <BloodMoney/Game/ZHM3Actor.h>

namespace Glacier
{
    template <typename T>
    struct ZCast;

    template <>
    struct ZCast<Glacier::ZGROUP>
    {
        static bool IsBasedOn(Glacier::ZGEOM* pGeom) {
            if (!pGeom) return false;

            int* m_Mask = (int*)0x00972988;
            int* m_Id = (int*)0x00972984;

            return ((*m_Mask) & pGeom->GetObjectId()) == (*m_Id);
        }
    };

    template <>
    struct ZCast<Hitman::BloodMoney::ZHM3Actor>
    {
        static bool IsBasedOn(Glacier::ZGEOM* pGeom) {
            if (!pGeom) return false;

            int* m_Mask_ZHM3Actor = (int*)0x009B11E0;
            int* m_Id_ZHM3Actor = (int*)0x009B11E4;

            int* m_Mask_ZActor = (int*)0x0097BD64;
            int* m_Id_ZActor = (int*)0x0097BD60;

            const bool isZHM3Actor = ((*m_Mask_ZHM3Actor) & pGeom->GetObjectId()) == (*m_Id_ZHM3Actor);
            const bool isZActor = ((*m_Mask_ZActor) & pGeom->GetObjectId()) == (*m_Id_ZActor);

            return isZHM3Actor || isZActor;
        }
    };

    template <>
    struct ZCast<Glacier::ZCAMERA>
    {
        static bool IsBasedOn(Glacier::ZGEOM* pGeom) {
            if (!pGeom) return false;

            int* m_Mask_ZCAMERA = (int*)0x009727BC;
            int* m_Id_ZCAMERA = (int*)0x009727B8;

            return ((*m_Mask_ZCAMERA) & pGeom->GetObjectId()) == (*m_Id_ZCAMERA);
        }
    };

    template <>
    struct ZCast<Glacier::ZSNDOBJ>
    {
        static bool IsBasedOn(Glacier::ZGEOM* pGeom) {
            if (!pGeom) return false;

            int* m_Mask_ZSNDOBJ = (int*)0x0097B5FC;
            int* m_Id_ZSNDOBJ = (int*)0x0097B5F8;

            return ((*m_Mask_ZSNDOBJ) & pGeom->GetObjectId()) == (*m_Id_ZSNDOBJ);
        }
    };
}