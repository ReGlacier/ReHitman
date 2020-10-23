#pragma once

#include <memory>
#include <vector>

namespace Hitman::BloodMoney
{
    class DebugWidget
    {
    public:
        using Ref = std::weak_ptr<DebugWidget>;
        using Ptr = std::shared_ptr<DebugWidget>;

        virtual ~DebugWidget() noexcept = default;

        DebugWidget() = default;

        void addChild(const DebugWidget::Ptr& child);
        void removeChild(const DebugWidget::Ptr& child);

        virtual void draw();

    protected:
        DebugWidget::Ref m_parent;

    private:
        std::vector<DebugWidget::Ptr> m_child;
    };
}