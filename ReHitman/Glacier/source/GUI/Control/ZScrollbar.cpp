#include <Glacier/GUI/Control/ZScrollbar.h>
#include <Glacier/GUI/ZWINDOWS.h>


namespace Glacier
{
    ZScrollbar::ZScrollbar(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCONTROL(psName, pBaseGeom)
        , m_dwMoveAxis(1)
        , m_rTop(0)
        , m_rBottom(0)
        , m_rSlider(0)
        , m_v2MousePos{}
        , m_fBarPos(0.0f)
        , m_dwSize(0)
        , m_dwOffset(0)
        , m_bTracking(false)
    {
    }

    ZScrollbar::~ZScrollbar() = default;

    const RTP::ZPropertyInfo& ZScrollbar::GetProperties() const
    {
        return ZScrollbar::Info;
    }

    uint32_t ZScrollbar::GetObjectId() const
    {
        return ZScrollbar::m_Id;
    }

    void ZScrollbar::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZScrollbar::m_Id;
        mask = ZScrollbar::m_Mask;
    }

    ZGEOMCLASSINFO* ZScrollbar::GetOldClassInfo() const
    {
        return ZScrollbar::m_OldClassInfo;
    }

    void ZScrollbar::ClassInit2()
    {
        ZCONTROL::ClassInit2();
        SetSliderButton(m_rSlider);
        Update();
    }

    ZGEOM* ZScrollbar::Duplicate(ZGROUP* pDestGroup, const char* psName, bool bRecursive)
    {
        auto* pDuplicate = static_cast<ZScrollbar*>(ZGROUP::Duplicate(pDestGroup, psName, bRecursive));
        pDuplicate->m_dwMoveAxis = m_dwMoveAxis;
        pDuplicate->m_rTop = m_rTop;
        pDuplicate->m_rBottom = m_rBottom;
        pDuplicate->m_rSlider = m_rSlider;
        pDuplicate->m_v2MousePos = m_v2MousePos;
        pDuplicate->m_fBarPos = m_fBarPos;
        pDuplicate->m_dwSize = m_dwSize;
        pDuplicate->m_dwOffset = m_dwOffset;
        return pDuplicate;
    }

    ZGEOM* ZScrollbar::DuplicateToResource(ZGROUP* pDestGroup, uint32_t lGeomResourceId, const char* psName, bool bRecursive)
    {
        auto* pDuplicate = static_cast<ZScrollbar*>(ZGROUP::DuplicateToResource(pDestGroup, lGeomResourceId, psName, bRecursive));
        pDuplicate->m_dwMoveAxis = m_dwMoveAxis;
        pDuplicate->m_rTop = m_rTop;
        pDuplicate->m_rBottom = m_rBottom;
        pDuplicate->m_rSlider = m_rSlider;
        pDuplicate->m_v2MousePos = m_v2MousePos;
        pDuplicate->m_fBarPos = m_fBarPos;
        pDuplicate->m_dwSize = m_dwSize;
        pDuplicate->m_dwOffset = m_dwOffset;
        return pDuplicate;
    }

    bool ZScrollbar::OnMouseMove(float* params)
    {
        if (!m_bTracking)
            return false;

        const float* pPosition = Pos();
        float& axisPosition = params[m_dwMoveAxis];
        if (axisPosition > pPosition[m_dwMoveAxis] + static_cast<float>(m_dwSize))
            axisPosition = pPosition[m_dwMoveAxis] + static_cast<float>(m_dwSize);
        if (axisPosition < pPosition[m_dwMoveAxis])
            axisPosition = pPosition[m_dwMoveAxis];

        const float delta = axisPosition - (&m_v2MousePos.x)[m_dwMoveAxis];
        std::memcpy(&m_v2MousePos, params, sizeof(m_v2MousePos));
        UpdateBarPos(delta);

        float position = m_fBarPos / static_cast<float>(m_dwSize);
        int encodedPosition;
        std::memcpy(&encodedPosition, &position, sizeof(position));
        GetSystem()->Notify(ZWN_SCROLLBARCHANGE, GetRef(), encodedPosition, nullptr);
        return true;
    }

    bool ZScrollbar::OnStateChanged(uint32_t param, uint32_t state)
    {
        if (param != m_rSlider)
            return false;

        ZWINDOWS* pSystem = GetSystem();
        pSystem->GetMousePos(m_v2MousePos);
        pSystem->RemoveSubscriber(2, 0, this);
        if (state == 2)
        {
            m_bTracking = true;
            pSystem->AddSubscriber(2, 0, this, 0.0f, true);
        }
        else
        {
            m_bTracking = false;
        }
        return true;
    }

    void ZScrollbar::Update()
    {
        ZGEOM* pSlider = ZGEOM::RefToPtr(m_rSlider);
        if (!pSlider)
            return;

        ZVector3 position{};
        pSlider->GetPos(position);
        (&position.x)[m_dwMoveAxis] = m_fBarPos - static_cast<float>(m_dwOffset);
        pSlider->SetPos(position);
    }

    void ZScrollbar::UpdateBarPos(float delta)
    {
        if (delta == 0.0f)
            return;

        m_fBarPos += delta;
        if (m_fBarPos < 0.0f)
            m_fBarPos = 0.0f;
        if (m_fBarPos > static_cast<float>(m_dwSize))
            m_fBarPos = static_cast<float>(m_dwSize);
        Update();
    }

    void ZScrollbar::SetBarSize(int size)
    {
        m_dwSize = size;
        Update();
    }

    void ZScrollbar::SetMoveAxis(int axis)
    {
        m_dwMoveAxis = axis;
    }

    int32_t ZScrollbar::GetMoveAxis()
    {
        return m_dwMoveAxis;
    }

    void ZScrollbar::SetSliderButton(uint32_t slider)
    {
        if (ZGEOM* pSlider = ZGEOM::RefToPtr(slider))
        {
            ZVector3 size{};
            pSlider->GetSize(size);
            m_dwOffset = static_cast<int32_t>((&size.x)[m_dwMoveAxis]);
        }

        ZWINDOWS* pSystem = GetSystem();
        pSystem->RemoveSubscriber(ZWN_STATECHANGED, m_rSlider, this);
        m_rSlider = slider;
        pSystem->AddSubscriber(ZWN_STATECHANGED, m_rSlider, this, 10.0f, true);
    }

    void ZScrollbar::SetPosition(float position)
    {
        if (position < 0.0f)
            position = 0.0f;
        else if (position > 1.0f)
            position = 1.0f;
        m_fBarPos = static_cast<float>(m_dwSize) * position;
        Update();
    }

    float ZScrollbar::GetPosition()
    {
        return m_fBarPos / static_cast<float>(m_dwSize);
    }


#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZScrollbar,
        ZCONTROL,
        0x009A2B38,
        "ZScrollbar",
        0x0077E09C,
        nullptr,
        0x00806C24,
        0x009A2A98,
        0x009A2A9C
    );
#   pragma endregion
}
