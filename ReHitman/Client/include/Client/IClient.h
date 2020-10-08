#pragma once

#include <Client/IDebugView.h>

namespace ReHitman::Client
{
    class IClient
    {
    private:
        bool m_isOnline = true;

    public:
        virtual ~IClient() noexcept = default;

        virtual bool OnAttach() = 0;
        virtual void OnDestroy() = 0;
        virtual void Run() { m_isOnline = true; }

        [[nodiscard]] bool IsOnline() const { return m_isOnline; }

        virtual IDebugView* GetDebugView() { return nullptr; /* default impl: no debug view */ }

    protected:
        void Stop() { m_isOnline = false; }
    };
}