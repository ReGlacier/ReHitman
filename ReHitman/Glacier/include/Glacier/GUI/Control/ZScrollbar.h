#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    class ZScrollbar : public ZCONTROL
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZScrollbar, 0x100040u);

        // vtbl
        ~ZScrollbar() override;

        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit2() override;
        ZGEOM* Duplicate(ZGROUP* pDestGroup, const char* psName, bool bRecursive) override;
        ZGEOM* DuplicateToResource(ZGROUP* pDestGroup, uint32_t lGeomResourceId, const char* psName, bool bRecursive) override;

        bool OnMouseMove(float* params) override;
        bool OnStateChanged(uint32_t param, uint32_t state) override;

        virtual void Update();
        virtual void UpdateBarPos(float delta);
        virtual void SetBarSize(int size);
        virtual void SetMoveAxis(int axis);
        virtual int32_t GetMoveAxis();
        virtual void SetSliderButton(uint32_t slider);
        virtual void SetPosition(float position);
        virtual float GetPosition();

        // methods
        ZScrollbar(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        int32_t m_dwMoveAxis;
        ZREF m_rTop;
        ZREF m_rBottom;
        ZREF m_rSlider;
        ZVector2 m_v2MousePos;
        float m_fBarPos;
        int32_t m_dwSize;
        int32_t m_dwOffset;
        bool m_bTracking;
    };
    RE_VERIFY_SIZE(ZScrollbar, 0xC0); // Verified PC alloc
}
