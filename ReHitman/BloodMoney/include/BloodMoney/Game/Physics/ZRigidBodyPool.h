#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZBaseConRout;
    class ZGEOM;
    class ZGROUP;
    template <typename T> struct ZArray;
}

namespace Hitman
{
    /**
     * @brief Fixed-size pool of `CRigidBody` events used by the shatter system.
     *
     * Reversed from `engine/fysix/rigidbody.cpp` (IDA: `ZRigidBodyPool`, x86/32 size `0x18`).
     * Every slot owns the `CRigidBody` event whose id is built by
     * `ZRigidBodyPoolManager::CreateID` (`poolId << 12 | slot`). The pool itself is created
     * by name through `ZRigidBodyPoolManager::Add` and destroyed through
     * `ZRigidBodyPoolManager::Remove`.
     */
    class ZRigidBodyPool
    {
    public:
        // methods
        ZRigidBodyPool();
        ~ZRigidBodyPool();

        bool Create(uint32_t total);
        void CleanUp();
        uint32_t Count() const;
        uint32_t Unused() const;

        bool Assign(Glacier::ZGEOM* pShard);
        bool Assign(const Glacier::ZGROUP* pShards, uint32_t lAssigned, Glacier::ZArray<Glacier::ZGEOM*>* pAssigned,
                    Glacier::ZArray<Glacier::ZGEOM*>* pLeftovers, bool bForced);

        // members
        uint16_t m_id;
        Glacier::ZBaseConRout** m_pPool;
        bool* m_pActive;
        uint32_t m_lCount;
        uint32_t m_lUnused;
        uint32_t m_lLast;

    private:
        bool Remove(uint16_t index);
        bool ForceRemoval(uint16_t total);
    };
    RE_VERIFY_SIZE(ZRigidBodyPool, 0x18);
    RE_VERIFY_OFFSET(ZRigidBodyPool, m_pPool, 0x4);
    RE_VERIFY_OFFSET(ZRigidBodyPool, m_pActive, 0x8);
    RE_VERIFY_OFFSET(ZRigidBodyPool, m_lCount, 0xC);
    RE_VERIFY_OFFSET(ZRigidBodyPool, m_lUnused, 0x10);
    RE_VERIFY_OFFSET(ZRigidBodyPool, m_lLast, 0x14);

    /**
     * @brief One named pool entry owned by `ZRigidBodyPoolManager::m_pEntries`.
     *
     * Reversed from `engine/fysix/rigidbody.cpp`. IDA lists it as a top-level struct, while
     * the mangled symbols (`ZRigidBodyPoolManager::SRBPoolEntry`) show it was nested in
     * `ZRigidBodyPoolManager`; the layout is identical either way.
     */
    struct SRBPoolEntry
    {
        SRBPoolEntry() : kRBPool(nullptr) {}
        ~SRBPoolEntry();

        char sName[64];
        ZRigidBodyPool* kRBPool;
    };
    RE_VERIFY_SIZE(SRBPoolEntry, 0x44);
    RE_VERIFY_OFFSET(SRBPoolEntry, kRBPool, 0x40);
}
