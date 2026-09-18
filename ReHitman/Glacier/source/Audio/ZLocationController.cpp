#include <Glacier/Audio/ZLocationController.h>

namespace Glacier
{
    ZLocationController::ZLocationController()
        : m_rCurrentRoom(0)
        , m_pCurrentLocation(nullptr)
        , m_pPreviousLocation(nullptr)
    {
    }

    ZLocationController::~ZLocationController() = default;

    void ZLocationController::Update(ZROOM*, float*)
    {
    }
}
