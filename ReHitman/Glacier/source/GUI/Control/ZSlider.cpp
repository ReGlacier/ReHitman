#include <Glacier/GUI/ZSlider.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/GUI/Frame/ZCUSTOMFRAME.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/RTP/VirtualTables.h>

namespace Glacier
{
    ZSlider::ZSlider(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCONTROL(psName, pBaseGeom),
          m_dwMin(0),
          m_dwMax(1),
          m_fSlidePos(0.0f),
          m_rButton(0),
          m_rBackGround(0),
          m_rtExtraText(4, 0),
          m_bTracking(false)
    {
    }

    ZSlider::~ZSlider() = default;

    const RTP::ZPropertyInfo& ZSlider::GetProperties() const
    {
        return ZSlider::Info;
    }

    uint32_t ZSlider::GetObjectId() const
    {
        return ZSlider::m_Id;
    }

    void ZSlider::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSlider::m_Id;
        mask = ZSlider::m_Mask;
    }

    ZGEOMCLASSINFO* ZSlider::GetOldClassInfo() const
    {
        return ZSlider::m_OldClassInfo;
    }

    void ZSlider::ClassInit()
    {
        ZCONTROL::ClassInit();

        if (!m_rButton)
        {
            for (ZBaseGeom* pBaseGeom = m_pGroupFirst; ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->Next())
            {
                ZGEOM* pGeom = pBaseGeom->GetGeom();
                if (pGeom && pGeom->IsDerivedFrom<ZBUTTON>())
                {
                    m_rButton = pGeom->GetRef();
                    break;
                }
            }
        }

        for (ZBaseGeom* pBaseGeom = m_pGroupLast; pBaseGeom; pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINOBJ>())
            {
                auto* pWinObj = static_cast<ZWINOBJ*>(pGeom);
                if (pWinObj->m_dwType == ZWINOBJ::TYPE_Background)
                    m_rBackGround = pGeom->GetRef();
            }
        }

        if (m_rButton)
            GetSystem()->AddSubscriber(0x200000, m_rButton, this, 10.0f, true);

        SetSliderSize(m_fSlideSize);
    }

    void ZSlider::CopyData(const ZGEOM* pSource)
    {
        ZCONTROL::CopyData(pSource);
        m_dwSteps = static_cast<const ZSlider*>(pSource)->m_dwSteps;
    }

    void ZSlider::SetState(uint32_t state)
    {
        ZCONTROL::SetState(state);
        for (ZBaseGeom* pBaseGeom = m_pGroupFirst; ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZCONTROL>())
                static_cast<ZCONTROL*>(pGeom)->SetState(state);
        }
    }

    bool ZSlider::OnMouseMove(float* params)
    {
        ZWINDOWS* pSystem = GetSystem();
        if (!pSystem || pSystem->GetMouseGeom() != m_rButton)
            return false;

        const float oldPosition = m_fSlidePos;
        UpdateSlidePos((params[0] - m_fMousePos) / m_fSlideSize);
        m_fMousePos -= (oldPosition - m_fSlidePos) * m_fSlideSize;
        return ZCONTROL::OnMouseMove(params);
    }

    bool ZSlider::OnKeyPress(uint32_t key)
    {
        if (key == 125)
        {
            UpdateSlidePos(-m_fStepSize);
            return true;
        }
        if (key == 126)
        {
            UpdateSlidePos(m_fStepSize);
            return true;
        }
        return ZCONTROL::OnKeyPress(key);
    }

    bool ZSlider::OnStateChanged(uint32_t param, uint32_t state)
    {
        ZASSERT(param == m_rButton);

        if (state == 1)
        {
            GetSystem()->RemoveSubscriber(12, 125, this);
            GetSystem()->RemoveSubscriber(12, 126, this);
            if (m_bTracking)
            {
                m_bTracking = false;
                NotifyChange();
            }
        }
        else if (state == 2)
        {
            ZVector2 mousePosition{};
            GetSystem()->GetMousePos(mousePosition);
            m_fMousePos = mousePosition.x;
            m_bTracking = true;
        }
        else if (state != 8)
        {
            return false;
        }

        if (state == 2 || state == 8)
        {
            GetSystem()->AddSubscriber(12, 125, this, 10.0f, true);
            GetSystem()->AddSubscriber(12, 126, this, 10.0f, true);
        }

        return false;
    }

    void ZSlider::SetRange(int min, int max)
    {
        m_dwMin = min;
        m_dwMax = max;
        SetSteps(max - min + 1);
        SetValue(0, false);
    }

    void ZSlider::SetSteps(int steps)
    {
        m_dwSteps = steps;
        m_fStepSize = 1.0f / static_cast<float>(steps - 1);
    }

    int32_t ZSlider::GetValue()
    {
        return static_cast<int32_t>((m_dwMax - m_dwMin) * m_fSlidePos + m_dwMin + 0.00012207031f);
    }

    bool ZSlider::SetValue(int value, bool notify)
    {
        if (value < m_dwMin || value > m_dwMax)
            return false;

        m_fSlidePos = static_cast<float>(value - m_dwMin) / static_cast<float>(m_dwMax - m_dwMin);
        Update(notify);
        return true;
    }

    void ZSlider::UpdateSlidePos(float delta)
    {
        if (delta == 0.0f)
            return;

        m_fSlidePos += delta;
        if (m_fSlidePos < 0.0f)
            m_fSlidePos = 0.0f;
        if (m_fSlidePos > 1.0f)
            m_fSlidePos = 1.0f;
        Update(true);
    }

    void ZSlider::Update(bool notify)
    {
        ZGEOM* pButton = ZGEOM::RefToPtr(m_rButton);
        if (!pButton)
            return;

        const float position = static_cast<float>(static_cast<int32_t>((m_fSlideSize - m_dwButtonWidth) * m_fSlidePos));
        ZVector3 buttonPosition{};
        pButton->GetPos(buttonPosition);
        if (buttonPosition.x != position)
        {
            buttonPosition.x = position;
            pButton->SetPos(buttonPosition);
            if (notify)
                NotifyChange();
        }
    }

    void ZSlider::SetExtraText(const char* text)
    {
        for (auto it = m_rtExtraText.As<ZREF>().begin(); it != m_rtExtraText.As<ZREF>().end();)
        {
            ZGEOM* pGeom = ZGEOM::RefToPtr(*it);
            if (!pGeom || !pGeom->IsDerivedFrom<ZCHAROBJ>())
            {
                it.Erase();
                continue;
            }

            auto* pText = static_cast<ZCHAROBJ*>(pGeom);
            if (text[0] == '%')
                pText->SetText(text + 1);
            else
                pText->SetTextId("", text);
            ++it;
        }
    }

    void ZSlider::AddExtraText(ZGEOM* pGeom)
    {
        m_rtExtraText.Add(pGeom->GetRef());
    }

    void ZSlider::ClearExtraText()
    {
        m_rtExtraText.Clear();
    }

    float ZSlider::GetSliderSize() const
    {
        return m_fSlideSize;
    }

    void ZSlider::SetSliderSize(float size)
    {
        m_fSlideSize = size;
        if (m_rBackGround)
        {
            auto* pFrame = geom_cast<ZCUSTOMFRAME>(ZGEOM::RefToPtr(m_rBackGround));
            ZASSERT(pFrame);
            int32_t width;
            int32_t height;
            pFrame->GetOuterSize(width, height);
            pFrame->SetOuterSize(static_cast<int>(m_fSlideSize), height);
        }
        Update(false);
    }

    float ZSlider::GetSliderPos() const
    {
        return m_fSlidePos;
    }

    bool ZSlider::IsChanging() const
    {
        return m_bTracking;
    }

    void ZSlider::GetWidth(int32_t& width)
    {
        width = static_cast<int32_t>(m_fSlideSize);
    }

    void ZSlider::SetWidth(const int32_t& width)
    {
        m_fSlideSize = static_cast<float>(width);
    }

    void ZSlider::NotifyChange()
    {
        if (ZWINDOWS* pSystem = GetSystem())
            pSystem->Notify(ZWN_SLIDERCHANGE, GetRef(), GetValue(), static_cast<ZWINGROUP*>(ZGEOM::RefToPtr(m_rOwner)));
    }


#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<int32_t> Width{
            .m_Node = {.m_Next = nullptr, .m_Name = "Width", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Virtual_int,
            .m_Get = &ZSlider::GetWidth,
            .m_Set = &ZSlider::SetWidth};
        static RTP::ZDataProperty<float> MousePos{
            .m_Node = {.m_Next = Width, .m_Name = "m_fMousePos", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_fMousePos)};
        static RTP::ZDataProperty<REFTAB> ExtraText{
            .m_Node = {.m_Next = MousePos, .m_Name = "m_rtExtraText", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_rtExtraText)};
        static RTP::ZDataProperty<ZGEOMREF> BackGround{
            .m_Node = {.m_Next = ExtraText, .m_Name = "m_rBackGround", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZSlider, m_rBackGround))};
        static RTP::ZDataProperty<ZGEOMREF> Button{
            .m_Node = {.m_Next = BackGround, .m_Name = "m_rButton", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZSlider, m_rButton))};
        static RTP::ZDataProperty<float> SlideSize{
            .m_Node = {.m_Next = Button, .m_Name = "m_fSlideSize", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_fSlideSize)};
        static RTP::ZDataProperty<float> SlidePos{
            .m_Node = {.m_Next = SlideSize, .m_Name = "m_fSlidePos", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_fSlidePos)};
        static RTP::ZDataProperty<float> StepSize{
            .m_Node = {.m_Next = SlidePos, .m_Name = "m_fStepSize", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_fStepSize)};
        static RTP::ZDataProperty<int32_t> ButtonWidth{
            .m_Node = {.m_Next = StepSize, .m_Name = "m_dwButtonWidth", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_dwButtonWidth)};
        static RTP::ZDataProperty<int32_t> Steps{
            .m_Node = {.m_Next = ButtonWidth, .m_Name = "m_dwSteps", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_dwSteps)};
        static RTP::ZDataProperty<int32_t> Max{
            .m_Node = {.m_Next = Steps, .m_Name = "m_dwMax", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_dwMax)};
        static RTP::ZDataProperty<int32_t> Min{
            .m_Node = {.m_Next = Max, .m_Name = "m_dwMin", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZSlider, m_dwMin)};
    }
    DECLARE_GEOM_CLASS_IMPL(
        ZSlider,
        ZCONTROL,
        0x009A2DF8,
        "ZSlider",
        0x0077F940,
        cProperties::Min,
        0x00814C60,
        0x009A2DA8,
        0x009A2DAC
    );
#   pragma endregion
}
