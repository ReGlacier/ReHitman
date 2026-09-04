#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/Control/ZCONTROL.h>


namespace Glacier
{
    enum _ButtonTypes
    {
        ZBT_NORMAL = 0,
        ZBT_CHECKBUTTON = 1,
    };

    enum EInitialState
    {
        INST_NORMAL = 0,
        INST_PUSHED = 1,
    };

    class ZBUTTON : public ZCONTROL
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZBUTTON, 0x100033u);

        // vtbl
        ~ZBUTTON() override;

        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void CopyData(const ZGEOM* pSource) override;

        void MouseLeave() override;
        void MouseEnter(bool bFromMouseMove) override;
        void Push(bool bPush) override;
        void Release() override;
        void Click() override;
        bool OnMouseMove(float* params) override;
        bool OnKeyUp(uint32_t key) override;

        virtual void CheckButton(bool bChecked);
        virtual void SetButtonType(_ButtonTypes type);
        virtual bool IsChecked();

        // methods
        ZBUTTON(const char* psName, ZBaseGeom* pBaseGeom);
        void GetInitialState(EInitialState& state);
        void SetInitialState(const EInitialState& state);

        // members
        _ButtonTypes m_dwButtonType;
        uint32_t m_dwButtonState;
    };
    RE_VERIFY_SIZE(ZBUTTON, 0xA0); // Verified PC alloc
    RE_VERIFY_OFFSET(ZBUTTON, m_dwButtonType, 0x98);
    RE_VERIFY_OFFSET(ZBUTTON, m_dwButtonState, 0x9C);
}
