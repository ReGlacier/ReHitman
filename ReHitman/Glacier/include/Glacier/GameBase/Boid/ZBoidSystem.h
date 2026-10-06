#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/STLport.h>
#include <Glacier/PF4/Fwd.h>


namespace Glacier
{
    class ZBoid;

    /**
     * @class ZBoidSystem
     * @brief Owns the pool of live boids, hands out boid ids and drives their
     *        per-frame update.
     *
     * Reversed from the PC image (`ZBoidSystem::Ctor` at `0x5855F0`,
     * `ZBoidSystem::AddBoid` at `0x585610`) and the PS2 id allocator
     * (`ZBoidSystem::GetNextBoidID` at `0x735C5C`). The single instance lives in
     * `ZGameData::m_pkBoidSystem`; `ZActor::Initialize` (PC `0x507780`) pulls a
     * fresh id from it and registers the actor's `ZHumanBoid` through
     * @ref AddBoid.
     */
    class ZBoidSystem
    {
    public:
        ZBoidSystem(PF4::ZInterface* pPathFinder);

        // Returns the next unused boid id and advances the internal counter.
        int GetNextBoidID();

        // Registers a boid with the system so it is updated every frame.
        ZBoid* AddBoid(ZBoid* pBoid);

        // Runs FrameUpdate() then Move() on every registered boid.
        void FrameUpdate();

        // members (total size 0x14)
        stlp::vector<ZBoid*> m_BoidsPool; // +0x00: { _M_start, _M_finish, _M_end_of_storage }
        PF4::ZInterface* m_pPF4Interface; // +0x0C
        int m_iNextBoidID;                // +0x10
    };
    RE_VERIFY_SIZE(ZBoidSystem, 0x14);
}
