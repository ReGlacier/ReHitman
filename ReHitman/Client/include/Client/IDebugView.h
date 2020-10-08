#pragma once

namespace ReHitman::Client
{
    class IDebugView
    {
    private:
        bool m_bIsDrawInProgress { false };
        bool m_visible { false };

    public:
        virtual ~IDebugView() noexcept = default;

        virtual void OnDrawBegin() { m_bIsDrawInProgress = true; }
        virtual void OnDrawEnd() { m_bIsDrawInProgress = false; }

        [[nodiscard]] bool IsVisible() const { return m_visible; }
        void SetVisible(bool visible) { m_visible = visible; }
    protected:
        [[nodiscard]] bool IsDrawInProgress() const { return m_bIsDrawInProgress; };
    };
}