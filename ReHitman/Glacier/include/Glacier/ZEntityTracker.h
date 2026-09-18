#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/ZInterface.h>
#include <Glacier/PF4/Fwd.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    /**
     * @brief Registers and updates gameplay entity nodes in a PF4 path-finder.
     *
     * The tracker owns three PF4 meta type ids: actors, hero nodes, and reserved
     * points. It assigns those type ids to ZMetaNode instances, maps world-space
     * positions to PF4 locations, and forwards node add/move/remove/query calls to
     * the configured PF4::ZInterface implementation.
     */
    struct ZEntityTracker
    {
        // methods
        /** @brief Creates a tracker and reserves actor, hero, and reservation meta type ids. */
        ZEntityTracker(PF4::ZInterface* pPathFinder);
        /** @brief Destroys the tracker without owning the path-finder or nodes. */
        ~ZEntityTracker();
        /** @brief Assigns actor type to a node and registers it at a world position. */
        void AddActor(PF4::ZMetaNode* pEntity, const ZVector3& vPos);
        /** @brief Assigns hero type to a node and registers it at a world position. */
        void AddHero(PF4::ZMetaNode* pEntity, const ZVector3& vPos);
        /** @brief Assigns reservation type to a node and registers it at a world position. */
        void AddReservation(PF4::ZMetaNode* pEntity, const ZVector3& vPos);
        /** @brief Returns the PF4 meta type id used for actor nodes. */
        int ActorType() const;
        /** @brief Returns the PF4 meta type id used for hero nodes. */
        int HeroType() const;
        /** @brief Returns the PF4 meta type id used for reserved point nodes. */
        int ReservedType() const;
        /** @brief Finds nearby nodes of a specific PF4 meta type. */
        int GetClosest(const PF4::ZLocation& kSource, PF4::ZInterface::ZResult* pList, int iMaxEntities, float fMaxDistance, int type);
        /** @brief Finds nearby nodes of any PF4 meta type. */
        int GetClosest(const PF4::ZLocation& kSource, PF4::ZInterface::ZResult* pList, int iMaxEntities, float fMaxDistance);
        /** @brief Moves a registered node to a constrained world-space position. */
        void Move(PF4::ZMetaNode& node, const ZVector3& vPos);
        /** @brief Removes a registered node from the path-finder. */
        void Remove(PF4::ZMetaNode& node);

        // members
        /** @brief Non-owning path-finder interface used by all operations. */
        PF4::ZInterface* m_PathFinder;
        /** @brief PF4 meta type id assigned to actor nodes. */
        int m_ActorTypeId;
        /** @brief PF4 meta type id assigned to hero nodes. */
        int m_HeroTypeId;
        /** @brief PF4 meta type id assigned to reserved point nodes. */
        int m_ReservedPointId;
    };
    RE_VERIFY_SIZE(ZEntityTracker, 0x10);
}
