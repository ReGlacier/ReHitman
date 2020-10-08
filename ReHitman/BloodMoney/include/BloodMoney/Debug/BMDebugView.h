#pragma once

#include <Client/IDebugView.h>

namespace Hitman::BloodMoney::DebugUI
{
    class BMDebugModel;

    class BMDebugView final : public ReHitman::Client::IDebugView
    {
        using Base = ReHitman::Client::IDebugView;

    protected:
        BMDebugModel* m_model { nullptr };
        BMDebugModel* m_modelToSwap { nullptr };

    public:
        void SetModel(BMDebugModel* model);

        void OnDrawBegin() override;
        void OnDrawEnd() override;
    };
}