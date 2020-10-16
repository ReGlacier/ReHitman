#pragma once

#include <cstdint>

#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    class ZBoid;

    /**
     * @class ZBoidSystem
     * @brief Store boids, update boids
     */
    class ZBoidSystem // Size 0x14
    {
    public:
        // Data
        ZBoid** m_boidsPool; //0x0000
        int32_t m_pFreeAvailableMemForPoolBegin; //0x0004
        int32_t m_pFreeAvailableMemForPoolEnd; //0x0008
        Glacier::PF4::Interface* m_pPF4Interface; //0x000C (always nullptr, maybe unused)
        int32_t m_totalBoids; //0x0010
        int32_t m_unknownField; //0x0014

        // Methods
        ZBoid* AddBoid(ZBoid*);
        void FrameUpdate();
    };
}