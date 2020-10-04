#include <BloodMoney/Client.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    bool Client::OnAttach()
    {
        spdlog::info("----------[ WELCOME TO RE:HITMAN PROJECT ]----------");
        return true;
    }

    void Client::OnDestroy()
    {
        spdlog::info("----------[ BB ]----------");
    }
}