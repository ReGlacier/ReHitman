#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZItem;
    class ZItemTemplate;

    enum class HandType {
        LeftHand	= 32,
        RightHand	= 31,
        RatUnk0		= 20,
        RatUnk1		= 25
    };

    class ZIKHAND
    {
    public:
        virtual void LoadSave(void*, bool&);
        virtual void AttachItem(unsigned int itemID);
        virtual void SlipItem();
        virtual ZItem* GetItem();
        virtual ZItemTemplate* GetItemType();
        virtual void Reset();
        virtual void SetTarget(const Matrix4x4* transform, const Vector3* point, float veliocity, void* callback);
    };
}