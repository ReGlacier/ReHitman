#pragma once

#include <G1ConfigurationService.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZMath.h>

namespace Glacier
{
    class ZEntityLocator
    {
    public:
        // Data
        ZMat3x3 m_transform;
        ZVector3 position;
        ZVector3 field_30;
        int field_3C;
        int field_40;
        Glacier::ZVector3 field_44;
        ZEntityLocator *parent; //+0x50
        int field_54;           //+0x54
        uint16_t field_58;      //+0x58
        uint16_t m_id;          //+0x5A
        uint32_t m_Instance;    //+0x5C
        uint32_t m_assignedTo;  //+0x60
        int field_64;           //+0x64
        int m_primitive;        //+0x68
        const char *entityName; //+0x6C
        int field_70;
        int field_74;
        int field_78;
        int field_7C;

        // API
        ZGROUP* ParentGroup();
        void SetName(const char* name);
        bool SetPrim(int primId);
        void GetMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos);
        ZEntityLocator* Next();
        void SetNext(ZEntityLocator* next);
        ZEntityLocator* GetPrev();
        void SetPrev(ZEntityLocator* prev);
    }; //Size: 0x0080
}