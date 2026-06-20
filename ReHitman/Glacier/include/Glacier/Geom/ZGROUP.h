#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZEntityLocator.h> // ZBaseGeom
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Glacier
{
    class ZGROUP : public ZGEOM
    {
    public:
        //vftable
        virtual bool IsRecursiveActivateAllowed();
        virtual void DynamicGroupOnScreen();
        virtual void CheckBoxInside_ZGROUP(const float*, const float*,const float*);
        virtual ZGEOM* FindLoadWorldGeom(char const*);
        virtual ZGEOM* FindMasterGeom(char const*);
        virtual ZGEOM* FindGeom(const char*, ZEntityLocator*);
        virtual int GroupDepth();
        virtual float GetPFResMultiplier();
        virtual void LinkBound(unsigned int);
        virtual void RemoveBound(unsigned int);
        virtual void GetAmbientSettings(const float*, float*, float*, float*);
        virtual void CreateParentsRecur(ZGROUP*,ZGROUP**,bool);
        virtual void SetOverRideNearFar(float *);
        virtual void CorrectCenSizeRecur();
        virtual void CorrectCenSize();
        virtual void InvalidateBounds();
        virtual void AttachGeom(ZEntityLocator*, bool);
        virtual void AttachGeom(ZGEOM*, bool);
        virtual void DetachGeom(ZEntityLocator*, bool);
        virtual void RecurGetNextGroup(const ZEntityLocator**);
        virtual void RecurGetNextExclRoom(const ZEntityLocator**);
        virtual void SetGroupControl(unsigned int, unsigned int);
        virtual unsigned int GroupControl();
        virtual void ResetGroupPosition(bool);
        virtual void MakeActiveRecursive();
        virtual void GetStaticLights(ZEntityLocator**, ZEntityLocator**);
        virtual void CalcCenSizeRecur();
        virtual void GetCenSizeRecur(float*, float*, bool);
        virtual ZGEOM* FindMaskGeom(char const*, int);

        //data (total size is 0x4F)
        float m_vSizeInsideCheck[3];
        float m_vCenInsideCheck[3];
        unsigned int m_lGroupCon;
        float m_OverRideNearFar[2];
        REFTAB* m_pZBounds;
        unsigned int m_LightList;
        ZBaseGeom* m_pGroupFirst;
        ZBaseGeom* m_pGroupLast;
        float m_fPFResMultiplier;
        uint16_t m_NrAttachGeom;
        uint16_t m_pad3A;

        //API
        ZGEOM* CreateGeom(const char* name, int typeId, bool unk3);
        bool IsRoot();
    };
    RE_VERIFY_SIZE(ZGROUP, 0x4C); // Verified
}