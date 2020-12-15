#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZTreeGroup.h>

namespace Glacier
{
    class ZROOM : public ZTreeGroup
    {
    public:
        //vftable
        virtual void CalcBestRoom(unsigned int, float const*, float const*, float const*); //DANGEROUS METHOD
        virtual void AddDynamicGeomToRoom(ZEntityLocator*);
        virtual void RemoveDynamicGeomFromRoom(ZEntityLocator*);
        virtual void SetRoomControl(unsigned int, unsigned int);
        virtual unsigned int RoomControl();
        virtual void GetDynamicLightsInRoom(ZEntityLocator**, ZEntityLocator**);
        virtual void GetStaticPrimDrawGeomsListsRecur(ZEntityLocator**, ZEntityLocator**);
        virtual void GetStaticCustomDrawGeomsListsRecur(ZEntityLocator**, ZEntityLocator**);
        virtual void GetStaticLightsRecur(ZEntityLocator**, ZEntityLocator**);
        virtual bool NotInRoomTree();

        //data (total size is 0x144, base size is 0x70)
        int m_field70;
        int m_field74;
        int m_field78;
        int m_field7C;
        int m_field80;
        int m_field84;
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_field98;
        int m_field9C;
        int m_fieldA0;
        int m_fieldA4;
        int m_fieldA8;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        REFTAB m_reftabB8;
        REFTAB m_reftabD4;
        int m_fieldF0;
        int m_fieldF4;
        int m_fieldF8;
        int m_fieldFC;
        int m_field100;
        int m_field104;
        int m_field108;
        int m_field10C;
        int m_field110;
        int m_field114;
        int m_field118;
        int m_field11C;
        int m_field120;
        int m_field124;
        int m_field128;
        int m_field12C;
        int m_field130;
        int m_field134;
        int m_field138;
        int m_field13C;
        int m_field140;
    };
}