#include <Glacier/GUI/ZBUTTON.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        constexpr uint32_t STATE_RELEASED = 1;
        constexpr uint32_t STATE_PUSHED = 2;
        constexpr uint32_t STATE_CHECKED = 16;
        constexpr uint32_t STATE_DISABLED = 32;

        void LoadInitialState(RTP::ZVirtualEnumProperty<EInitialState>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
        {
            EInitialState value{};
            stream.ExchangeEnum(pProperty->m_Node.m_Name, &value, 1, *pProperty->m_Info);
            uintptr_t address = 0;
            std::memcpy(&address, &pProperty->m_Set.__pfn, sizeof(address));
            reinterpret_cast<void(__thiscall*)(void*, const EInitialState&)>(address)(
                reinterpret_cast<uint8_t*>(&object) + pProperty->m_Set.__delta, value);
        }

        void SaveInitialState(RTP::ZVirtualEnumProperty<EInitialState>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
        {
            EInitialState value{};
            uintptr_t address = 0;
            std::memcpy(&address, &pProperty->m_Get.__pfn, sizeof(address));
            reinterpret_cast<void(__thiscall*)(void*, EInitialState&)>(address)(
                reinterpret_cast<uint8_t*>(&object) + pProperty->m_Get.__delta, value);
            stream.ExchangeEnum(pProperty->m_Node.m_Name, &value, 1, *pProperty->m_Info);
        }
    }

    ZBUTTON::ZBUTTON(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCONTROL(psName, pBaseGeom),
          m_dwButtonType(ZBT_NORMAL),
          m_dwButtonState(STATE_RELEASED)
    {
    }

    ZBUTTON::~ZBUTTON() = default;

    const RTP::ZPropertyInfo& ZBUTTON::GetProperties() const
    {
        return ZBUTTON::Info;
    }

    uint32_t ZBUTTON::GetObjectId() const
    {
        return ZBUTTON::m_Id;
    }

    void ZBUTTON::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZBUTTON::m_Id;
        mask = ZBUTTON::m_Mask;
    }

    ZGEOMCLASSINFO* ZBUTTON::GetOldClassInfo() const
    {
        return ZBUTTON::m_OldClassInfo;
    }

    void ZBUTTON::ClassInit()
    {
        ZCONTROL::ClassInit();
    }

    void ZBUTTON::CopyData(const ZGEOM* pSource)
    {
        ZCONTROL::CopyData(pSource);
        const auto* pButton = static_cast<const ZBUTTON*>(pSource);
        m_dwButtonType = pButton->m_dwButtonType;
        m_dwButtonState = pButton->m_dwButtonState;
    }

    void ZBUTTON::MouseLeave()
    {
        if (m_dwButtonType != ZBT_CHECKBUTTON && m_dwControlState != STATE_DISABLED && m_dwControlState != STATE_PUSHED)
            ZCONTROL::MouseLeave();
    }

    void ZBUTTON::MouseEnter(bool bFromMouseMove)
    {
        if (m_dwButtonType != ZBT_CHECKBUTTON && m_dwControlState != STATE_DISABLED && m_dwControlState != STATE_PUSHED)
            ZCONTROL::MouseEnter(bFromMouseMove);
    }

    void ZBUTTON::Push(bool)
    {
        if (m_dwControlState == STATE_DISABLED)
            return;
        if (m_dwButtonType != ZBT_CHECKBUTTON)
            SetState(STATE_PUSHED);

        // TODO: Finish this place after ZWINDOWS will be reversed
        // GetSystem()->SetPressedControl(this); GetSystem()->SetKeyRepeat(16, 136, this, 10.0f, true);
    }

    void ZBUTTON::Release()
    {
        if (m_dwControlState != STATE_DISABLED && m_dwButtonType != ZBT_CHECKBUTTON)
            SetState(STATE_RELEASED);
    }

    void ZBUTTON::Click()
    {
        if (m_dwControlState == STATE_DISABLED)
            return;
        if (m_dwButtonType == ZBT_CHECKBUTTON)
            CheckButton(!IsChecked());
        ZCONTROL::Click();
    }

    bool ZBUTTON::OnMouseMove(float* params)
    {
        return m_dwControlState == STATE_DISABLED || ZCONTROL::OnMouseMove(params);
    }

    bool ZBUTTON::OnKeyUp(uint32_t key)
    {
        if (key == 136 && m_dwControlState == STATE_PUSHED)
        {
            SetState(STATE_RELEASED);
            return true;
        }
        return ZCONTROL::OnKeyUp(key);
    }

    void ZBUTTON::CheckButton(bool bChecked)
    {
        SetState(bChecked ? STATE_CHECKED : STATE_RELEASED);
    }
    void ZBUTTON::SetButtonType(_ButtonTypes type)
    {
        m_dwButtonType = type;
    }
    bool ZBUTTON::IsChecked()
    {
        return (m_dwControlState & STATE_CHECKED) != 0;
    }
    void ZBUTTON::GetInitialState(EInitialState&)
    {
        ZASSERT(false);
    }
    void ZBUTTON::SetInitialState(const EInitialState& state)
    {
        m_dwControlState = state == INST_PUSHED ? STATE_CHECKED : STATE_RELEASED;
    }

#pragma region " --- RTTI --- "
    namespace cProperties
    {
        static ZEnumEntry ButtonTypeEntries[]{
            {nullptr, ZBT_NORMAL, "ZBT_NORMAL"},
            {&ButtonTypeEntries[0], ZBT_CHECKBUTTON, "ZBT_CHECKBUTTON"}};
        static ZEnumInfo ButtonTypeInfo{&ButtonTypeEntries[1], "_ButtonTypes", sizeof(_ButtonTypes)};

        static ZEnumEntry InitialStateEntries[]{
            {nullptr, INST_NORMAL, "INST_NORMAL"},
            {&InitialStateEntries[0], INST_PUSHED, "INST_PUSHED"}};
        static ZEnumInfo InitialStateInfo{&InitialStateEntries[1], "EInitialState", sizeof(EInitialState)};
        static RTP::tVirtualTable<RTP::ZVirtualEnumProperty<EInitialState>> InitialStateTable{
            LoadInitialState, SaveInitialState};
        static RTP::ZEnumProperty ButtonType{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_dwButtonType", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZBUTTON, m_dwButtonType),
            .m_Info = &ButtonTypeInfo};
        static RTP::ZVirtualEnumProperty<EInitialState> InitialState{
            .m_Node = {.m_Next = ButtonType, .m_Name = "InitialState", .m_Filter = 1},
            .m_VirtualTable = &InitialStateTable,
            .m_Get = &ZBUTTON::GetInitialState,
            .m_Set = &ZBUTTON::SetInitialState,
            .m_Info = &InitialStateInfo};
    }
    DECLARE_GEOM_CLASS_IMPL(ZBUTTON, ZCONTROL, 0x009A2A88, "ZBUTTON", 0x0077DA88, cProperties::InitialState, 0x00807A14, 0x009A2A38, 0x009A2A3C);
#pragma endregion
}
