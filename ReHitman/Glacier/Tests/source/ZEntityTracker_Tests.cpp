#include <Glacier/ZEntityTracker.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

namespace Glacier::PF4
{
    ZInterface::~ZInterface() = default;
    int ZInterface::MapNodeIdx(ZDataRef, float*, float*, EPathWayActions&, unsigned int&) { return 0; }
    int ZInterface::GetMetaId() { return 0; }
    void ZInterface::AddNode(ZMetaNode*, const ZLocation&) {}
    void ZInterface::RemoveNode(ZMetaNode*) {}
    void ZInterface::MoveNodeConstrained(ZMetaNode*, const float*) {}
    void ZInterface::TeleportNode(ZMetaNode*, const float*) {}
    int ZInterface::FindNodes(const ZLocation&, ZResult*, int, float, int) { return 0; }
    void ZInterface::AddObstacle(ZDynamicObstacle*, const ZLocation&) {}
    void ZInterface::RemoveObstacle(ZDynamicObstacle*) {}
    void ZInterface::MoveObstacle(ZDynamicObstacle*, const ZLocation&) {}
    void ZInterface::AddObstacle(ZDynamicObstacle*, const float*) {}
    void ZInterface::MoveObstacle(ZDynamicObstacle*, const float*) {}
    void ZInterface::PushOutOfObstacles(ZMetaNode*, int, float*) {}
    bool ZInterface::HasObstacles() { return false; }
    bool ZInterface::FindPath(ZPathRequest*) { return false; }
    void ZInterface::FreePath(ZPath*) {}
    bool ZInterface::AllocateBufferPath(const ZPath&, ZPath*) { return false; }
    float ZInterface::FindPathLength(ZPathRequest*) { return 0.0f; }
    bool ZInterface::PositionInside(const float*) { return false; }
    bool ZInterface::MapLocation(const float*, ZLocation&) { return false; }
    bool ZInterface::MapValidLocation(const float*, ZLocation&) { return false; }
    bool ZInterface::MapInside(const ZLocation&, ZLocation&, bool) { return false; }
    float ZInterface::ComponentHeight(int, float, float) { return 0.0f; }
    bool ZInterface::MapOntoComponent(const ZLocation&, ZLocation&) { return false; }
    int ZInterface::FindComponents(const ZLocation&, int*, int, float) { return 0; }
    int ZInterface::CloseExit(float*) { return 0; }
    int ZInterface::CloseLinks(float*, float*, unsigned int) { return 0; }
    int ZInterface::OpenLinks(float*, float*, unsigned int) { return 0; }
    int ZInterface::GraphCount() { return 0; }
    void ZInterface::FindCornersInGraph(int, int&, float*) {}
    float ZInterface::FindWallIntersection(const ZLocation&, const float*, float, void*, float*, void*, ZLocation&, bool, bool) { return 0.0f; }
    int ZInterface::FindWalls(const ZLocation&, float, float*) { return 0; }
    int ZInterface::FindObstacles(const ZLocation&, ZDynamicObstacle**, int) { return 0; }
    void ZInterface::RemapDoorRefs(unsigned int*, unsigned int) {}
}

namespace
{
    struct RawMetaNode
    {
        alignas(PF4::ZMetaNode) unsigned char Storage[sizeof(PF4::ZMetaNode)]{};

        PF4::ZMetaNode* Get()
        {
            return reinterpret_cast<PF4::ZMetaNode*>(Storage);
        }
    };

    struct FakePathFinder final : PF4::ZInterface
    {
        int NextMetaId = 10;
        int LastFindType = 0;
        int LastFindMaxEntities = 0;
        float LastFindMaxDistance = 0.0f;
        int FindReturnValue = 0;
        PF4::ZMetaNode* LastAddedNode = nullptr;
        PF4::ZMetaNode* LastMovedNode = nullptr;
        PF4::ZMetaNode* LastRemovedNode = nullptr;
        const float* LastMappedPosition = nullptr;
        const float* LastMovedPosition = nullptr;
        int LastAddedComponent = 0;
        int LastAddedGraph = 0;
        uint8_t LastAddedInside = 0;
        const PF4::ZLocation* LastFindSource = nullptr;
        PF4::ZInterface::ZResult* LastFindOutput = nullptr;

        ~FakePathFinder() override = default;
        int MapNodeIdx(PF4::ZDataRef, float*, float*, PF4::EPathWayActions&, unsigned int&) override { return 0; }
        int GetMetaId() override { return NextMetaId++; }
        void AddNode(PF4::ZMetaNode* node, const PF4::ZLocation& location) override
        {
            LastAddedNode = node;
            LastAddedComponent = location.Component();
            LastAddedGraph = location.Graph();
            LastAddedInside = location.Inside();
        }
        void RemoveNode(PF4::ZMetaNode* node) override { LastRemovedNode = node; }
        void MoveNodeConstrained(PF4::ZMetaNode* node, const float* position) override
        {
            LastMovedNode = node;
            LastMovedPosition = position;
        }
        void TeleportNode(PF4::ZMetaNode*, const float*) override {}
        int FindNodes(const PF4::ZLocation& source, PF4::ZInterface::ZResult* output, int maxEntities, float maxDistance, int type) override
        {
            LastFindSource = &source;
            LastFindOutput = output;
            LastFindMaxEntities = maxEntities;
            LastFindMaxDistance = maxDistance;
            LastFindType = type;
            return FindReturnValue;
        }
        void AddObstacle(PF4::ZDynamicObstacle*, const PF4::ZLocation&) override {}
        void RemoveObstacle(PF4::ZDynamicObstacle*) override {}
        void MoveObstacle(PF4::ZDynamicObstacle*, const PF4::ZLocation&) override {}
        void AddObstacle(PF4::ZDynamicObstacle*, const float*) override {}
        void MoveObstacle(PF4::ZDynamicObstacle*, const float*) override {}
        void PushOutOfObstacles(PF4::ZMetaNode*, int, float*) override {}
        bool HasObstacles() override { return false; }
        bool FindPath(PF4::ZPathRequest*) override { return false; }
        void FreePath(PF4::ZPath*) override {}
        bool AllocateBufferPath(const PF4::ZPath&, PF4::ZPath*) override { return false; }
        float FindPathLength(PF4::ZPathRequest*) override { return 0.0f; }
        bool PositionInside(const float*) override { return false; }
        bool MapLocation(const float* position, PF4::ZLocation& location) override
        {
            LastMappedPosition = position;
            location.Set(ZVector3(position), 2, 3, true);
            return true;
        }
        bool MapValidLocation(const float*, PF4::ZLocation&) override { return false; }
        bool MapInside(const PF4::ZLocation&, PF4::ZLocation&, bool) override { return false; }
        float ComponentHeight(int, float, float) override { return 0.0f; }
        bool MapOntoComponent(const PF4::ZLocation&, PF4::ZLocation&) override { return false; }
        int FindComponents(const PF4::ZLocation&, int*, int, float) override { return 0; }
        int CloseExit(float*) override { return 0; }
        int CloseLinks(float*, float*, unsigned int) override { return 0; }
        int OpenLinks(float*, float*, unsigned int) override { return 0; }
        int GraphCount() override { return 0; }
        void FindCornersInGraph(int, int&, float*) override {}
        float FindWallIntersection(const PF4::ZLocation&, const float*, float, void*, float*, void*, PF4::ZLocation&, bool, bool) override { return 0.0f; }
        int FindWalls(const PF4::ZLocation&, float, float*) override { return 0; }
        int FindObstacles(const PF4::ZLocation&, PF4::ZDynamicObstacle**, int) override { return 0; }
        void RemapDoorRefs(unsigned int*, unsigned int) override {}
    };
}

TEST(ZEntityTracker, ConstructorReservesThreeMetaTypes)
{
    FakePathFinder pathFinder;

    ZEntityTracker tracker(&pathFinder);

    EXPECT_EQ(tracker.m_PathFinder, &pathFinder);
    EXPECT_EQ(tracker.ActorType(), 10);
    EXPECT_EQ(tracker.HeroType(), 11);
    EXPECT_EQ(tracker.ReservedType(), 12);
}

TEST(ZEntityTracker, AddActorMapsPositionAndRegistersActorNode)
{
    FakePathFinder pathFinder;
    ZEntityTracker tracker(&pathFinder);
    RawMetaNode rawNode;
    auto* node = rawNode.Get();
    ZVector3 position{ 1.0f, 2.0f, 3.0f };

    tracker.AddActor(node, position);

    EXPECT_EQ(node->m_Type, tracker.ActorType());
    EXPECT_EQ(pathFinder.LastMappedPosition, position.Get());
    EXPECT_EQ(pathFinder.LastAddedNode, node);
    EXPECT_EQ(pathFinder.LastAddedComponent, 3);
    EXPECT_EQ(pathFinder.LastAddedGraph, 2);
    EXPECT_EQ(pathFinder.LastAddedInside, 1u);
}

TEST(ZEntityTracker, AddHeroAndReservationAssignTheirMetaTypes)
{
    FakePathFinder pathFinder;
    ZEntityTracker tracker(&pathFinder);
    RawMetaNode rawHero;
    RawMetaNode rawReservation;
    auto* hero = rawHero.Get();
    auto* reservation = rawReservation.Get();
    ZVector3 position{ 4.0f, 5.0f, 6.0f };

    tracker.AddHero(hero, position);
    EXPECT_EQ(hero->m_Type, tracker.HeroType());
    EXPECT_EQ(pathFinder.LastAddedNode, hero);

    tracker.AddReservation(reservation, position);
    EXPECT_EQ(reservation->m_Type, tracker.ReservedType());
    EXPECT_EQ(pathFinder.LastAddedNode, reservation);
}

TEST(ZEntityTracker, GetClosestForwardsTypedAndUntypedQueries)
{
    FakePathFinder pathFinder;
    pathFinder.FindReturnValue = 2;
    ZEntityTracker tracker(&pathFinder);
    PF4::ZLocation source{ ZVector3{ 1.0f, 0.0f, 0.0f }, 7, 8, false };
    PF4::ZInterface::ZResult results[4]{};

    EXPECT_EQ(tracker.GetClosest(source, results, 4, 12.5f, tracker.ActorType()), 2);
    EXPECT_EQ(pathFinder.LastFindOutput, results);
    EXPECT_EQ(pathFinder.LastFindMaxEntities, 4);
    EXPECT_EQ(pathFinder.LastFindMaxDistance, 12.5f);
    EXPECT_EQ(pathFinder.LastFindType, tracker.ActorType());

    EXPECT_EQ(tracker.GetClosest(source, results, 3, 6.0f), 2);
    EXPECT_EQ(pathFinder.LastFindMaxEntities, 3);
    EXPECT_EQ(pathFinder.LastFindMaxDistance, 6.0f);
    EXPECT_EQ(pathFinder.LastFindType, -1);
}

TEST(ZEntityTracker, MoveAndRemoveForwardToPathFinder)
{
    FakePathFinder pathFinder;
    ZEntityTracker tracker(&pathFinder);
    RawMetaNode rawNode;
    auto* node = rawNode.Get();
    ZVector3 position{ 9.0f, 8.0f, 7.0f };

    tracker.Move(*node, position);
    EXPECT_EQ(pathFinder.LastMovedNode, node);
    EXPECT_EQ(pathFinder.LastMovedPosition, position.Get());

    tracker.Remove(*node);
    EXPECT_EQ(pathFinder.LastRemovedNode, node);
}
