#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/PF4.h>
#include <Glacier/PF4/Fwd.h>
#include <Glacier/PF4/EPathWayActions.h>
#include <Glacier/PF4/ZLocation.h>
#include <Glacier/PF4/ZMetaNode.h>


namespace Glacier::PF4
{
    struct ZInterface;

    struct ZDataRef 
    {
        unsigned int m_Id   : 14;
        unsigned int m_Type : 2;
        ZVector2 m_Pos;
    };
    RE_VERIFY_SIZE(ZDataRef, 0xC);

    struct ZPath 
    {
        ZDataRef* m_PathIdx;
        int m_iMaxSize;
        ZVector3 m_Vertices[4];
        int m_CustomVertices;
        ZInterface* m_PathFinder;
        int m_Size;
        float m_Cost;
    };
    RE_VERIFY_SIZE(ZPath, 0x48);

    struct ZPathRequest 
    {
        const PF4::ZLocation* m_Source;
        const PF4::ZLocation* m_Dest;
        PF4::ZMetaNode* m_Reservation;
        PF4::ZPath* m_Path;
        bool m_GoOutside;
        RE_ADD_PADDING(3);
        unsigned int m_ActorKeymask;
        bool m_IgnoreReservation;
        RE_ADD_PADDING(3);
        float m_StoppingDistance;
    };
    RE_VERIFY_SIZE(ZPathRequest, 0x20);

    struct ZInterface
    {
        struct ZResult
        {
            float fDistance;
            PF4::ZMetaNode *pNode;
        };

        virtual ~ZInterface();
        virtual int MapNodeIdx(ZDataRef, float*, float*, EPathWayActions&, unsigned int&);
        virtual int GetMetaId();
        virtual void AddNode(ZMetaNode*, const ZLocation&);
        virtual void RemoveNode(ZMetaNode*);
        virtual void MoveNodeConstrained(ZMetaNode*, const float*);
        virtual void TeleportNode(ZMetaNode*, const float*);
        virtual int FindNodes(const ZLocation&, ZResult*, int, float, int);
        virtual void AddObstacle(ZDynamicObstacle*, const ZLocation&);
        virtual void RemoveObstacle(ZDynamicObstacle*);
        virtual void MoveObstacle(ZDynamicObstacle*, const ZLocation&);
        virtual void AddObstacle(ZDynamicObstacle*, const float*);
        virtual void MoveObstacle(ZDynamicObstacle*, const float*);
        virtual void PushOutOfObstacles(ZMetaNode*, int, float*);
        virtual bool HasObstacles();
        virtual bool FindPath(ZPathRequest*);
        virtual void FreePath(ZPath*);
        virtual bool AllocateBufferPath(const ZPath&, ZPath*);
        virtual float FindPathLength(ZPathRequest*);
        virtual bool PositionInside(const float*);
        virtual bool MapLocation(const float*, ZLocation&);
        virtual bool MapValidLocation(const float*, ZLocation&);
        virtual bool MapInside(const ZLocation&, ZLocation&, bool);
        virtual float ComponentHeight(int, float, float);
        virtual bool MapOntoComponent(const ZLocation&, ZLocation&);
        virtual int FindComponents(const ZLocation&, int*, int, float);
        virtual int CloseExit(float*);
        virtual int CloseLinks(float*, float*, unsigned int);
        virtual int OpenLinks(float*, float*, unsigned int);
        virtual int GraphCount();
        virtual void FindCornersInGraph(int, int&, float*);
        virtual float FindWallIntersection(const ZLocation&, const float*, float, void*, float*, void*, ZLocation&, bool, bool);
        virtual int FindWalls(const ZLocation&, float, float*);
        virtual int FindObstacles(const ZLocation&, ZDynamicObstacle**, int);
        virtual void RemapDoorRefs(unsigned int*, unsigned int);
    };
}