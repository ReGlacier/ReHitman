#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZSlider : public ZCONTROL
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZSlider, 0x100034u);

        // vtbl
        ~ZSlider() override;

        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void CopyData(const ZGEOM* pSource) override;

        void SetState(uint32_t state) override;
        bool OnMouseMove(float* params) override;
        bool OnKeyPress(uint32_t key) override;
        bool OnStateChanged(uint32_t param, uint32_t state) override;

        virtual void SetRange(int min, int max);
        virtual void SetSteps(int steps);
        virtual int32_t GetValue();
        virtual bool SetValue(int value, bool notify);
        virtual void UpdateSlidePos(float delta);
        virtual void Update(bool notify);
        virtual void SetExtraText(const char* text);
        virtual void AddExtraText(ZGEOM* pGeom);
        virtual void ClearExtraText();

        // methods
        ZSlider(const char* psName, ZBaseGeom* pBaseGeom);

        float GetSliderSize() const;
        void SetSliderSize(float size);
        float GetSliderPos() const;
        bool IsChanging() const;

        void GetWidth(int32_t& width);
        void SetWidth(const int32_t& width);

    protected:
        void NotifyChange();

    public:
        // members
        int32_t m_dwMin;
        int32_t m_dwMax;
        int32_t m_dwSteps;
        int32_t m_dwButtonWidth;
        float m_fStepSize;
        float m_fSlidePos;
        float m_fSlideSize;
        ZREF m_rButton;
        ZREF m_rBackGround;
        REFTAB m_rtExtraText;
        float m_fMousePos;
        bool m_bTracking;
        float m_fDefaultSize;
    };
    RE_VERIFY_SIZE(ZSlider, 0xE4); // Verified PC alloc
}
