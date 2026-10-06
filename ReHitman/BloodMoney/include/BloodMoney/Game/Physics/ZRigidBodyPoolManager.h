#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZRuntimeComponentBase.h>
#include <BloodMoney/Game/Physics/ZRigidBodyPool.h>
#include <cstdint>


namespace Hitman
{
    // fwds
    class CRigidBody;

    /**
     * @brief Singleton registry that owns the named `ZRigidBodyPool` instances.
     *
     * Reversed from `engine/fysix/rigidbodypool.cpp` (IDA: `ZRigidBodyPoolManager`,
     * x86/32 size `0x18`, `ZComponentSingleton<ZRigidBodyPoolManager, ZRuntimeComponentBase>`).
     * The named pools ("ShardPool", ...) are created lazily through `Add`; the first
     * `m_lMaxEntries` (3) names are stored in `m_pEntries`.
     */
    class ZRigidBodyPoolManager : public Glacier::ZComponentSingleton<ZRigidBodyPoolManager, Glacier::ZRuntimeComponentBase>
    {
    public:
        // friends
        friend class CRigidBody;
        friend class ZRigidBodyPool;

        // constants
        static constexpr uint16_t POOL_ID_MAX = 0x000F;
        static constexpr uint16_t BODY_ID_MAX = 0x0FFF;

        // static
        static uint32_t m_lMaxEntries;

        // vtbl
        ~ZRigidBodyPoolManager() override;

        // methods
        ZRigidBodyPoolManager();

        ZRigidBodyPool* Add(const char* pszName);
        uint32_t Count() const { return m_lCount; }

        ZRigidBodyPool* operator[](const char* pszName) { return Find(pszName); }
        const ZRigidBodyPool* operator[](const char* pszName) const { return Find(pszName); }

        // members
        SRBPoolEntry* m_pEntries;
        uint32_t m_lCount;

    private:
        ZRigidBodyPool* Find(const char* pszName) const;
        bool Remove(uint16_t id);
        uint16_t CreateID(uint16_t wPoolId, uint16_t wBodyId) const;
    };
    RE_VERIFY_SIZE(ZRigidBodyPoolManager, 0x18);
    RE_VERIFY_OFFSET(ZRigidBodyPoolManager, m_pEntries, 0x10);
    RE_VERIFY_OFFSET(ZRigidBodyPoolManager, m_lCount, 0x14);
}
