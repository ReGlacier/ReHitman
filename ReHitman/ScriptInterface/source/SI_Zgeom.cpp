#include <SI/SI_Zgeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <cstdio>


namespace Glacier
{
    namespace 
    {
        ZGEOM* GetGeom(ZREF rGeom)
        {
            ZGEOM* pGeom = ZGEOM::RefToPtr(rGeom);
            if (!pGeom)
            {
                printf("ERROR: couldn't convert ref to pointer, in ZGeom.si, You probably called an interface function with a Null ref\n");
                return nullptr;
            }

            if (pGeom->IsDerivedFrom<ZGEOM>())
            {
                return pGeom;
            }

            printf("ERROR: %s isn't derived from ZGeom\n", pGeom->Name());
            return nullptr;
        }
    }

    v3 Zgeom__Getsize(ZREF rGeom)
    {
        // TODO: Finish me
        return {};
    }

    void Zgeom__Setposition(ZREF rGeom, float x, float y, float z)
    {
        // TODO: Finish me
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
        // TODO: Finish me
    }

    void Zgeom__Getposdir(ZREF rGeom, v3& position, v3& direction)
    {
        // TODO: Finish me
    }

    void Zgeom__Setposdir(ZREF rGeom, v3& position, v3& direction)
    {
        // TODO: Finish me
    }

    bool Zgeom__Getclosestposdirinbox(ZREF rBox, ZREF rGeom, ZREF rTarget, float radius, v3& position, v3& direction)
    {
        // TODO: Finish me
        return false;
    }

    float Zgeom__Getdistancetoobject(ZREF rGeom, ZREF rTarget)
    {
        // TODO: Finish me
        return 0.0f;
    }

    float Zgeom__Getdistancetopos(ZREF rGeom, v3 position)
    {
        // TODO: Finish me
        return 0.0f;
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
        // TODO: Finish me
        return 0.0f;
    }

    float Zgeom__Getangletodir(ZREF rGeom, v3 direction)
    {
        // TODO: Finish me
        return 0.0f;
    }

    float Zgeom__Getangletogeomdir(ZREF rGeom, ZREF rTarget)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zgeom__Copyobjectposdir(ZREF rGeom, ZREF rTarget)
    {
        // TODO: Finish me
    }

    void Zgeom__Copyobjectpos(ZREF rGeom, ZREF rTarget)
    {
        // TODO: Finish me
    }

    void Zgeom__Copyobjectdir(ZREF rGeom, ZREF rTarget)
    {
        // TODO: Finish me
    }

    v3 Zgeom__Getobjectrelpos(ZREF rGeom, float x, float y, float z)
    {
        // TODO: Finish me
        return {};
    }

    bool Zgeom__Checkworldpointinside(ZREF rGeom, v3 worldPosition)
    {
        // TODO: Finish me
        return false;
    }

    bool Zgeom__Isinview(ZREF rGeom)
    {
        // TODO: Finish me
        return false;
    }

    void Zgeom__Printname(ZREF rGeom)
    {
        // TODO: Finish me
    }

    float Zgeom__Getdistancetoitem(ZREF rGeom, ZREF rItem)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zgeom__Makeinactive(ZREF rGeom)
    {
        // TODO: Finish me
    }

    void Zgeom__Makeactive(ZREF rGeom)
    {
        // TODO: Finish me
    }

    void Zgeom__Hide(ZREF rGeom, char hide)
    {
        // TODO: Finish me
    }

    ZREF Zgeom__Getcurrentroom(ZREF rGeom)
    {
        // TODO: Finish me
        return 0;
    }

    int Zgeom__Numberofexitsfromgeomscurrentroom(ZREF rGeom, ZREF rRoom)
    {
        // TODO: Finish me
        return 0;
    }
}
