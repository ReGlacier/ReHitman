#pragma once

#include <Client/ReHitmanClientInterface.h>

namespace Hitman::BloodMoney
{
    class Client : public ReHitman::Client::ReHitmanClientInterface
    {
    public:
        bool OnAttach() override;
        void OnDestroy() override;
    };
}