#include <BloodMoney/UI/DebugWidget.h>

#include <algorithm>

namespace Hitman::BloodMoney
{
    void DebugWidget::addChild(const DebugWidget::Ptr& child)
    {
        if (child)
        {
            m_child.push_back(child);
        }
    }

    void DebugWidget::removeChild(const DebugWidget::Ptr& child)
    {
        if (child)
        {
            auto foundIt = std::find(std::begin(m_child), std::end(m_child), child);
            if (foundIt != std::end(m_child))
            {
                m_child.erase(foundIt);
            }
        }
    }

    void DebugWidget::draw()
    {
        for (const auto& child : m_child)
        {
            child->draw();
        }
    }
}