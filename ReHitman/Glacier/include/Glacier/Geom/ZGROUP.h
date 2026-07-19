#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Glacier
{
    class ZGROUP : public ZGEOM
    {
    public:
        STATIC_CLASS_VAR(ZGROUP, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZGROUP, uint32_t, m_Mask);

        // vtbl
        virtual bool IsRecursiveActivateAllowed();
        virtual void DynamicGroupOnScreen();
        virtual void CheckBoxInside(const float*, const float*, const float*) const;
        virtual ZGEOM* FindLoadWorldGeom(char const*) const;
        virtual ZGEOM* FindMasterGeom(char const*) const;
        virtual ZGEOM* FindGeom(const char*, ZBaseGeom*);
        virtual int GroupDepth();
        virtual float GetPFResMultiplier() const;
        virtual void LinkBound(unsigned int);
        virtual void RemoveBound(unsigned int);
        virtual void GetAmbientSettings(const float*, float*, float*, float*) const;
        virtual void CreateParentsRecur(ZGROUP*, ZGROUP**, bool);
        virtual void SetOverRideNearFar(float*);
        virtual void CorrectCenSizeRecur();
        virtual void CorrectCenSize();
        virtual void InvalidateBounds();
        virtual void AttachGeom(ZBaseGeom* pBaseGeom, bool bCalcMinMax);
        virtual void AttachGeom(ZGEOM* pGeom, bool bCalcMinMax);
        virtual void DetachGeom(ZBaseGeom* pBaseGeom, bool bCalcMinMax);
        virtual void RecurGetNextGroup(const ZBaseGeom**) const;
        virtual void RecurGetNextExclRoom(const ZBaseGeom**) const;
        virtual void SetGroupControl(unsigned int, unsigned int);
        virtual unsigned int GroupControl() const;
        virtual void ResetGroupPosition(bool);
        virtual void MakeActiveRecursive();
        virtual void GetStaticLights(ZBaseGeom**, ZBaseGeom**);
        virtual void CalcCenSizeRecur();
        virtual void GetCenSizeRecur(float*, float*, bool);
        virtual ZGEOM* FindMaskGeom(char const*, int) const;

        // methods
        ZGEOM* CreateResourceGeom(const char* pName, uint32_t iGeomResourceId, uint32_t lGeomClassType, bool bCalcMinMax);
        

        // members
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
        RE_ADD_PADDING(2);

        //API
        ZGEOM* CreateGeom(const char* pName, uint32_t iGeomClassId, bool bCalcMinMax);
        bool IsRoot();
    };
    RE_VERIFY_SIZE(ZGROUP, 0x4C); // Verified
}
