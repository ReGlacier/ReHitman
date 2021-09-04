#pragma once

#include <Glacier/EventBase/ZEventBase.h>

namespace Glacier {
    class ZGEOM;

    class ZActorHeroCheckInside : public Glacier::ZEventBase {
    public:
        //consts
        static constexpr Glacier::ZMSGID kActivateFrameUpdateMsg = 0x83A;
        //vftable (no changes)
        //api
        bool IsInside(Glacier::ZGEOM* pGeom);
        //data (total size is 0x15C, base size is 0x30)
        int m_field30;
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_field44;
        int m_field48;
        int m_field4C;
        int m_field50;
        int m_field54;
        int m_field58;
        int m_field5C;
        int m_field60;
        int m_field64;
        int m_field68;
        int m_field6C;
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
        int m_fieldB8;
        int m_fieldBC;
        int m_fieldC0;
        int m_fieldC4;
        int m_fieldC8;
        int m_fieldCC;
        int m_fieldD0;
        int m_fieldD4;
        int m_fieldD8;
        int m_fieldDC;
        int m_fieldE0;
        int m_fieldE4;
        int m_fieldE8;
        int m_fieldEC;
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
        int m_field144;
        int m_field148;
        int m_field14C;
        int m_field150;
        int m_field154;
        int m_field158;
    };
}