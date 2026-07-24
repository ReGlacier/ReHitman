#include <Glacier/ZEntityTracker.h>
#include <Glacier/PF4/ZMetaNode.h>


namespace Glacier
{
    ZEntityTracker::ZEntityTracker(PF4::ZInterface* pPathFinder)
        : m_PathFinder(pPathFinder)
        , m_ActorTypeId(pPathFinder->GetMetaId())
        , m_HeroTypeId(pPathFinder->GetMetaId())
        , m_ReservedPointId(pPathFinder->GetMetaId())
    {
    }
    
    ZEntityTracker::~ZEntityTracker() = default;

    void ZEntityTracker::AddActor(PF4::ZMetaNode* pEntity, const ZVector3& vPos)
    {
        pEntity->m_Type = m_ActorTypeId;

        PF4::ZLocation location {};
        m_PathFinder->MapLocation(vPos.Get(), location);
        m_PathFinder->AddNode(pEntity, location);
    }

    void ZEntityTracker::AddHero(PF4::ZMetaNode* pEntity, const ZVector3& vPos)
    {
        pEntity->m_Type = m_HeroTypeId;

        PF4::ZLocation location {};
        m_PathFinder->MapLocation(vPos.Get(), location);
        m_PathFinder->AddNode(pEntity, location);
    }

    void ZEntityTracker::AddReservation(PF4::ZMetaNode* pEntity, const ZVector3& vPos)
    {
        pEntity->m_Type = m_ReservedPointId;

        PF4::ZLocation location {};
        m_PathFinder->MapLocation(vPos.Get(), location);
        m_PathFinder->AddNode(pEntity, location);
    }
    
    int ZEntityTracker::ActorType() const 
    { 
        return m_ActorTypeId;
    }

    int ZEntityTracker::HeroType() const
    {
        return m_HeroTypeId;
    }
    
    int ZEntityTracker::ReservedType() const
    {
        return m_ReservedPointId;
    }
    
    int ZEntityTracker::GetClosest(const PF4::ZLocation& kSource, PF4::ZInterface::ZResult* pList, int iMaxEntities, float fMaxDistance, int type)
    {
        return m_PathFinder->FindNodes(kSource, pList, iMaxEntities, fMaxDistance, type);
    }

    int ZEntityTracker::GetClosest(const PF4::ZLocation& kSource, PF4::ZInterface::ZResult* pList, int iMaxEntities, float fMaxDistance)
    {
        return m_PathFinder->FindNodes(kSource, pList, iMaxEntities, fMaxDistance, -1);
    }

    void ZEntityTracker::Move(PF4::ZMetaNode& node, const ZVector3& vPos)
    {
        m_PathFinder->MoveNodeConstrained(&node, vPos.Get());
    }

    void ZEntityTracker::Remove(PF4::ZMetaNode& node)
    {
        m_PathFinder->RemoveNode(&node);
    }
}