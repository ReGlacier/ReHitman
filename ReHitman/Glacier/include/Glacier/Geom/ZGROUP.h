#pragma once

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
        float m_field10; //0x0010
        float m_field14; //0x0014
        float m_field18; //0x0018
        float m_field1C; //0x001C
        float m_field20; //0x0020
        float m_field24; //0x0024
        int32_t m_control; //0x0028
        int32_t m_field2C; //0x002C
        int32_t m_field30; //0x0030
        int32_t m_field34; //0x0034
        int32_t m_field38; //0x0038
        ZEntityLocator* m_pEntity0; //0x003C
        ZEntityLocator* m_pEntity1; //0x0040
        int32_t m_field44; //0x0044
        int32_t m_field48; //0x0048

        //API
        ZGEOM* CreateGeom(const char* name, int typeId, bool unk3);
        bool IsRoot();
    };
}