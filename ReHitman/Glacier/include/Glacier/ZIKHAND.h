#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZItem;
    class ZItemTemplate;
    class ZIKLNKOBJ;

    enum class HandType {
        LeftHand	= 32,
        RightHand	= 31,
        RatUnk0		= 20,
        RatUnk1		= 25
    };

    class ZIKHAND
    {
    public:
        virtual void LoadSave(ZPackedInput*, bool);
        virtual void AttachItem(ZIKLNKOBJ* owner, Glacier::ZREF itemID);
        virtual void SlipItem(ZIKLNKOBJ* owner);
        virtual ZItem* GetItem();
        virtual ZItemTemplate* GetItemType();
        virtual void Reset();
        virtual void SetTarget(ZIKLNKOBJ* owner, const Matrix4x4* transform, const Vector3* point, float veliocity, void* callback);

        // data
        int field_4;
        char m_bEnabled;
        char field_9;
        char field_A;
        char field_B;
        Glacier::ZMat3x3 m_mTarget;
        Glacier::ZVector3 m_vTarget;
        float m_fTime;
        float m_fStartTime;
        Glacier::ZREF m_rGeom;
        int m_lBoneId;
        int field_4C;
        int field_50;
        int field_54;
        int field_58;
        int field_5C;
        int m_itemId;
        bool m_hasItem;
        char field_65;
        char field_66;
        char field_67;
        HandType m_handType;
        Glacier::ZVector3 m_vItemOffset;
        int MSG_INVERTORYSETACTIVELEFT;
        int field_7C;
    };
}