#pragma once

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
    protected:
        void Stop() { m_isOnline = false; }
    };
}