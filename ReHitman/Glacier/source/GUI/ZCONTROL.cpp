#include <Glacier/GUI/ZCONTROL.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/RTP/VirtualTables.h>

namespace Glacier
{
    namespace
    {
        constexpr uint32_t STATE_RELEASED = 0x01;
        constexpr uint32_t STATE_PUSHED = 0x02;
        constexpr uint32_t STATE_HOVERED = 0x08;
        constexpr uint32_t STATE_DISABLED = 0x20;
        constexpr uint32_t STYLE_VISIBLE = 0x01;
        constexpr uint32_t STYLE_DISABLED = 0x02;
    }

    ZCONTROL::ZCONTROL(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINGROUP(psName, pBaseGeom)
        , m_rOwner(0)
        , m_dwShortcut(0)
        , m_dwId(0)
        , m_dwControlStyle(0)
        , m_dwControlState(0)
        , m_dwStates(0)
        , m_bNotifyControl(true)
        , m_bTextControl(false)
        , m_dwNormalColor(0xFF990000)
        , m_dwHoverColor(0xFFFF0000)
        , m_dwFocusColor(0xFF009900)
        , m_dwPushColor(0xFF00FF00)
        , m_dwDisabledColor(0xFF666666)
        , m_dwCheckColor(0xFF66FF66)
    {
    }

    ZCONTROL::~ZCONTROL() = default;

    const RTP::ZPropertyInfo& ZCONTROL::GetProperties() const { return ZCONTROL::Info; }
    uint32_t ZCONTROL::GetObjectId() const { return ZCONTROL::m_Id; }
    void ZCONTROL::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { id = ZCONTROL::m_Id; mask = ZCONTROL::m_Mask; }
    ZGEOMCLASSINFO* ZCONTROL::GetOldClassInfo() const { return ZCONTROL::m_OldClassInfo; }

    void ZCONTROL::Hide(bool bHide)
    {
        if (bHide)
        {
            if (ZWINDOWS* pSystem = GetSystem())
                pSystem->ReleaseFocusedControl(this, true);
            ZGEOM::Hide(true);
            m_dwControlStyle &= ~STYLE_VISIBLE;
        }
        else
        {
            ZGEOM::Hide(false);
            m_dwControlStyle |= STYLE_VISIBLE;
        }
    }

    void ZCONTROL::HideRecursive(bool bHide)
    {
        if (bHide)
        {
            if (ZWINDOWS* pSystem = GetSystem())
                pSystem->ReleaseFocusedControl(this, true);
            ZGEOM::HideRecursive(true);
            m_dwControlStyle &= ~STYLE_VISIBLE;
        }
        else
        {
            ZGEOM::HideRecursive(false);
            m_dwControlStyle |= STYLE_VISIBLE;
        }
    }

    void ZCONTROL::ClassInit()
    {
        ZWINGROUP::ClassInit();
        ZGROUP* pParent = Parent();
        ZASSERT(pParent && pParent->IsDerivedFrom<ZWINGROUP>());
        m_rOwner = pParent->GetRef();
    }

    void ZCONTROL::ClassInit2()
    {
        ZGEOM::ClassInit2();
        for (ZBaseGeom* pBaseGeom = m_pGroupLast; pBaseGeom && !ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            if (pBaseGeom->IsDerivedFrom<ZWINOBJ>())
                m_dwStates |= static_cast<ZWINOBJ*>(pBaseGeom->GetGeom())->m_dwType;
        }
        if (m_bTextControl)
            m_dwStates = 0x3F;
        Hide((m_dwControlStyle & STYLE_VISIBLE) == 0);
        (m_dwControlStyle & STYLE_DISABLED) ? Disable() : Enable();
        const uint32_t state = m_dwControlState;
        m_dwControlState = 0;
        SetState(state);
    }

    void ZCONTROL::CopyData(const ZGEOM* pSource)
    {
        ZWINGROUP::CopyData(pSource);
        const auto* pControl = static_cast<const ZCONTROL*>(pSource);
        m_dwControlStyle = pControl->m_dwControlStyle;
        m_dwControlState = pControl->m_dwControlState;
    }

    bool ZCONTROL::WndMessage(ZWMEVENT* pEvent)
    {
        switch (pEvent->Message)
        {
            case ZWM_COMMAND: return OnCommand(pEvent->Param1);
            case ZWM_MOUSEMOVE: { float p[2]; std::memcpy(p, &pEvent->Param1, sizeof(p)); return OnMouseMove(p); }
            case ZWM_KEYDOWN: return OnKeyDown(pEvent->Param1) || OnKeyPress(pEvent->Param1);
            case ZWM_KEYREPEAT: return OnKeyRepeat(pEvent->Param1) || OnKeyPress(pEvent->Param1);
            case ZWM_KEYUP: return OnKeyUp(pEvent->Param1);
            case ZWM_FOCUSRECEIVED: return OnFocusReceived(pEvent->Param1);
            case ZWM_FOCUSLOST: return OnFocusLost(pEvent->Param1);
            case ZWM_CLICK: { float p[2]; std::memcpy(p, &pEvent->Param1, sizeof(p)); return OnClick(p, pEvent->Target); }
            case ZWN_SLIDERCHANGE: return OnSliderChange(pEvent->Param1, pEvent->Param2);
            case ZWN_SCROLLBARCHANGE: { float value; std::memcpy(&value, &pEvent->Param2, sizeof(value)); return OnScrollbarChange(pEvent->Param1, value); }
            case ZWN_FOCUSCHANGED: return OnFocusChanged(pEvent->Param1, pEvent->Param2);
            case ZWN_STATECHANGED: return OnStateChanged(pEvent->Param1, pEvent->Param2);
            default: return ZWINGROUP::WndMessage(pEvent);
        }
    }

    void ZCONTROL::Enable() { if (m_dwControlState == STATE_DISABLED) { m_dwControlStyle &= ~STYLE_DISABLED; SetState(STATE_RELEASED); } }
    void ZCONTROL::Disable() { if (m_dwControlState != STATE_DISABLED) { m_dwControlStyle |= STYLE_DISABLED; SetState(STATE_DISABLED); if (auto* p = GetSystem()) p->ReleaseFocusedControl(this, true); } }
    bool ZCONTROL::NotifyControl() { return m_bNotifyControl; }
    void ZCONTROL::SetNotifyControl(bool notify) { m_bNotifyControl = notify; }
    void ZCONTROL::SetOwner(ZREF owner) { m_rOwner = owner; }
    uint32_t ZCONTROL::GetState() { return m_dwControlState; }
    void ZCONTROL::SetAvailibleStates(uint32_t states) { m_dwStates = states; }

    void ZCONTROL::SetState(uint32_t state)
    {
        if (m_dwControlState == state)
            return;
        if (ZWINDOWS* pSystem = GetSystem())
            pSystem->Notify(ZWN_STATECHANGED, GetRef(), state, nullptr);
        m_dwControlState = state;
        UpdateStateGraphics((state & m_dwStates) ? state : STATE_RELEASED);
    }

    void ZCONTROL::HideNonStateObject(uint32_t state, ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom->IsDerivedFrom<ZWINOBJ>())
            return;
        auto* pObject = static_cast<ZWINOBJ*>(pBaseGeom->GetGeom());
        pObject->Hide((pObject->m_dwType & state) == 0);
        if (pObject->m_dwType & 0x80)
            pObject->Hide(false);
    }

    void ZCONTROL::UpdateStateGraphics(uint32_t state)
    {
        if (!m_dwStates)
            return;
        uint32_t color = 0xFFFF9988;
        if (m_bTextControl)
        {
            switch (state)
            {
                case 1: color = m_dwNormalColor; break;
                case 2: color = m_dwPushColor; break;
                case 4: color = m_dwHoverColor; break;
                case 8: color = m_dwFocusColor; break;
                case 16: color = m_dwCheckColor; break;
                case 32: color = m_dwDisabledColor; break;
            }
        }
        for (ZBaseGeom* pBaseGeom = m_pGroupLast; pBaseGeom && !ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            if (m_bTextControl && pBaseGeom->IsDerivedFrom<ZCHAROBJ>())
            {
                auto* pChar = static_cast<ZCHAROBJ*>(pBaseGeom->GetGeom());
                if (pChar->m_dwType != 0x80)
                    pChar->SetColor(color);
            }
            else
                HideNonStateObject(state, pBaseGeom);
        }
    }

    void ZCONTROL::MouseLeave() {}
    void ZCONTROL::MouseEnter(bool) { if (m_dwControlState != STATE_DISABLED) { GrabFocus(); SetState(STATE_HOVERED); } }
    void ZCONTROL::Push(bool) { if (m_dwControlState != STATE_DISABLED) { SetState(STATE_PUSHED); GrabFocus(); SetState(STATE_HOVERED); } }
    void ZCONTROL::Release() { if (m_dwControlState != STATE_DISABLED) SetState(STATE_RELEASED); }
    void ZCONTROL::Click() { if (m_dwControlState != STATE_DISABLED) GetSystem()->Notify(ZWM_COMMAND, GetControlId(), 0, static_cast<ZWINGROUP*>(ZGEOM::RefToPtr(m_rOwner))); }
    void ZCONTROL::SetControlId(int id) { m_dwId = id; }
    int ZCONTROL::GetControlId() { return m_dwId; }
    void ZCONTROL::GrabFocus() { GetSystem()->SetFocusedControl(this); }

    ZCONTROL* ZCONTROL::GetNextFocus(EFocusMove move)
    {
        if (move == Whatever)
        {
            if (auto* p = GetNextFocus(Down)) return p;
            if (auto* p = GetNextFocus(Right)) return p;
            if (auto* p = GetNextFocus(Up)) return p;
            return GetNextFocus(Left);
        }
        ZREF ref = move == Left ? m_rLeftControl : move == Right ? m_rRightControl : move == Up ? m_rUpControl : move == Down ? m_rDownControl : 0;
        auto* pControl = geom_cast<ZCONTROL>(ZGEOM::RefToPtr(ref));
        if (pControl && pControl != this && (pControl->m_dwControlStyle & STYLE_DISABLED))
            return pControl->GetNextFocus(move);
        return pControl;
    }

    void ZCONTROL::SetNextFocus(ZCONTROL* control, EFocusMove move)
    {
        const ZREF ref = control ? control->GetRef() : 0;
        if (move == Left) m_rLeftControl = ref;
        else if (move == Right) m_rRightControl = ref;
        else if (move == Up) m_rUpControl = ref;
        else if (move == Down) m_rDownControl = ref;
    }

    bool ZCONTROL::OnCommand(uint32_t) { return false; }
    bool ZCONTROL::OnMouseMove(float*) { return false; }
    bool ZCONTROL::OnKeyUp(uint32_t) { return false; }
    bool ZCONTROL::OnKeyDown(uint32_t key) { if (key == 127) { Click(); return true; } return false; }
    bool ZCONTROL::OnKeyPress(uint32_t key)
    {
        ZCONTROL* pNext = key == 0x7B ? GetNextFocus(Down) : key == 0x7C ? GetNextFocus(Up) : key == 0x7D ? GetNextFocus(Left) : key == 0x7E ? GetNextFocus(Right) : nullptr;
        if (pNext) GetSystem()->SetFocusedControl(pNext);
        return false;
    }
    bool ZCONTROL::OnKeyRepeat(uint32_t) { return false; }
    bool ZCONTROL::OnFocusReceived(uint32_t) { if (!(m_dwControlStyle & STYLE_DISABLED)) SetState(STATE_HOVERED); return false; }
    bool ZCONTROL::OnFocusLost(uint32_t) { if (!(m_dwControlStyle & STYLE_DISABLED)) SetState(STATE_RELEASED); return false; }
    bool ZCONTROL::OnClick(float*, uint32_t) { Click(); return true; }
    bool ZCONTROL::OnSliderChange(uint32_t, uint32_t) { return false; }
    bool ZCONTROL::OnScrollbarChange(uint32_t, float) { return false; }
    bool ZCONTROL::OnFocusChanged(uint32_t, uint32_t) { return false; }
    bool ZCONTROL::OnStateChanged(uint32_t, uint32_t) { return false; }

    void ZCONTROL::GetVisible(bool& visible) { visible = (m_dwControlStyle & STYLE_VISIBLE) != 0; }
    void ZCONTROL::SetVisible(const bool& visible) { visible ? m_dwControlStyle |= STYLE_VISIBLE : m_dwControlStyle &= ~STYLE_VISIBLE; }
    void ZCONTROL::GetDisabled(bool& disabled) { disabled = (m_dwControlStyle & STYLE_DISABLED) != 0; }
    void ZCONTROL::SetDisabled(const bool& disabled) { disabled ? m_dwControlStyle |= STYLE_DISABLED : m_dwControlStyle &= ~STYLE_DISABLED; }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<bool> Visible
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "Visible", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZCONTROL::GetVisible,
            .m_Set = &ZCONTROL::SetVisible
        };
        static RTP::ZVirtualProperty<bool> Disabled
        {
            .m_Node = { .m_Next = Visible, .m_Name = "Disabled", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZCONTROL::GetDisabled,
            .m_Set = &ZCONTROL::SetDisabled
        };
#define CONTROL_UINT_PROPERTY(Name, Next, Member, Filter) \
        static RTP::ZDataProperty<uint32_t> Name { \
            .m_Node = { .m_Next = Next, .m_Name = #Member, .m_Filter = Filter }, \
            .m_VirtualTable = VirtualTable_DP__6, .m_Offset = CLASS_PROPERTY(ZCONTROL, Member) };
        CONTROL_UINT_PROPERTY(CheckColor, Disabled, m_dwCheckColor, 1)
        CONTROL_UINT_PROPERTY(DisabledColor, CheckColor, m_dwDisabledColor, 1)
        CONTROL_UINT_PROPERTY(PushColor, DisabledColor, m_dwPushColor, 1)
        CONTROL_UINT_PROPERTY(FocusColor, PushColor, m_dwFocusColor, 1)
        CONTROL_UINT_PROPERTY(HoverColor, FocusColor, m_dwHoverColor, 1)
        CONTROL_UINT_PROPERTY(NormalColor, HoverColor, m_dwNormalColor, 1)
        static RTP::ZDataProperty<bool> TextControl
        {
            .m_Node = { .m_Next = NormalColor, .m_Name = "m_bTextControl", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__1, .m_Offset = CLASS_PROPERTY(ZCONTROL, m_bTextControl)
        };
        static RTP::ZDataProperty<bool> NotifyControl
        {
            .m_Node = { .m_Next = TextControl, .m_Name = "m_bNotifyControl", .m_Filter = 3 },
            .m_VirtualTable = VirtualTable_DP__1, .m_Offset = CLASS_PROPERTY(ZCONTROL, m_bNotifyControl)
        };
#define CONTROL_REF_PROPERTY(Name, Next, Member) \
        static RTP::ZDataProperty<ZGEOMREF> Name { \
            .m_Node = { .m_Next = Next, .m_Name = #Member, .m_Filter = 3 }, \
            .m_VirtualTable = VirtualTable_DP__5, \
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZCONTROL, Member)) };
        CONTROL_REF_PROPERTY(DownControl, NotifyControl, m_rDownControl)
        CONTROL_REF_PROPERTY(UpControl, DownControl, m_rUpControl)
        CONTROL_REF_PROPERTY(RightControl, UpControl, m_rRightControl)
        CONTROL_REF_PROPERTY(LeftControl, RightControl, m_rLeftControl)
#undef CONTROL_REF_PROPERTY
        CONTROL_UINT_PROPERTY(States, LeftControl, m_dwStates, 2)
        CONTROL_UINT_PROPERTY(ControlState, States, m_dwControlState, 2)
        CONTROL_UINT_PROPERTY(ControlStyle, ControlState, m_dwControlStyle, 2)
#undef CONTROL_UINT_PROPERTY
        static RTP::ZDataProperty<ZSDOwner> ControlId
        {
            .m_Node = { .m_Next = ControlStyle, .m_Name = "m_dwId", .m_Filter = 3 },
            .m_VirtualTable = VirtualTable_DP__18,
            .m_Offset = reinterpret_cast<ZSDOwner*>(CLASS_PROPERTY(ZCONTROL, m_dwId))
        };
        static RTP::ZDataProperty<uint32_t> Shortcut
        {
            .m_Node = { .m_Next = ControlId, .m_Name = "m_dwShortcut", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__6, .m_Offset = CLASS_PROPERTY(ZCONTROL, m_dwShortcut)
        };
        static RTP::ZDataProperty<ZGEOMREF> Owner
        {
            .m_Node = { .m_Next = Shortcut, .m_Name = "m_rOwner", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__5,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZCONTROL, m_rOwner))
        };
    }
    DECLARE_GEOM_CLASS_IMPL(ZCONTROL, ZWINGROUP, 0x009A2AE8, "ZCONTROL", 0x0077C640, cProperties::Owner, 0x00806C18, 0x009A2A90, 0x009A2A94);
#   pragma endregion
}
