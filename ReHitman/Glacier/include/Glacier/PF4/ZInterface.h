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
    // fwds
    struct ZInterface;
    struct ZPath;

    struct ZDataRef
    {
        unsigned int m_Id   : 14;
        unsigned int m_Type : 2;
        ZVector2 m_Pos;
    };
    RE_VERIFY_SIZE(ZDataRef, 0xC);

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

        virtual ~ZInterface() = default;
        virtual int MapNodeIdx(ZDataRef rRef, float*, float*, EPathWayActions&, unsigned int&) = 0;
        virtual int GetMetaId() = 0;
        virtual void AddNode(ZMetaNode* pNode, const ZLocation& kLocation) = 0;
        virtual void RemoveNode(ZMetaNode* pNode) = 0;
        virtual void MoveNodeConstrained(ZMetaNode* pNode, const ZVector3& vPos) = 0;
        virtual void TeleportNode(ZMetaNode* pNode, const ZVector3& vPos) = 0;
        virtual int FindNodes(const ZLocation& kSource, ZResult* pList, int iMaxEntities, float fMaxDistance, int type) = 0;
        virtual void AddObstacle(ZDynamicObstacle* pObstacle2, const ZLocation& kLocation) = 0;
        virtual void RemoveObstacle(ZDynamicObstacle* pObstacle2) = 0;
        virtual void MoveObstacle(ZDynamicObstacle* pObstacle2, const ZLocation& kPos) = 0;
        virtual void AddObstacle(ZDynamicObstacle* pObstacle2, const ZVector3& vPos) = 0;
        virtual void MoveObstacle(ZDynamicObstacle* pObstacle2, const ZVector3& vPos) = 0;
        virtual void PushOutOfObstacles(ZMetaNode* pNode, int iObstacleTypeMask, const ZVector3& vPos) = 0;
        virtual bool HasObstacles() = 0;
        virtual bool FindPath(ZPathRequest* pRequest) = 0;
        virtual void FreePath(ZPath* pPath) = 0;
        virtual bool AllocateBufferPath(const ZPath& sPath, ZPath* pResult) = 0;
        virtual float FindPathLength(ZPathRequest* pRequest) = 0;
        virtual bool PositionInside(const ZVector3& vPos) = 0;
        virtual bool MapLocation(const ZVector3& vPos, ZLocation& kLocation) = 0;
        virtual bool MapValidLocation(const ZVector3& vPos, ZLocation& kLocation) = 0;
        virtual bool MapInside(const ZLocation& kSource, ZLocation& kNewTarget, bool bUseObstacle) = 0;
        virtual float ComponentHeight(int iComponent, float x, float z) = 0;
        virtual bool MapOntoComponent(const ZLocation& kSource, ZLocation& kTarget) = 0;
        virtual int FindComponents(const ZLocation& kSource, int* pList, int iMax, float fMaxDistance) = 0;
        virtual int CloseExit(float*) = 0;
        virtual int CloseLinks(const ZVector3& vMin, const ZVector3& vMax, uint32_t lNewKeyMask) = 0;
        virtual int OpenLinks(const ZVector3& vMin, const ZVector3& vMax, uint32_t lNewKeyMask) = 0;
        virtual int GraphCount() = 0;
        virtual void FindCornersInGraph(int, int&, float*) = 0;
        virtual float FindWallIntersection(
            const ZLocation& kLoc,
            const ZVector3& vEndPoint,
            float fMaxDistance,
            const ZVector3&,
            ZVector3&,
            bool& bHitWall,
            ZLocation& endLocation,
            bool bReportDoorsAsWalls,
            bool bIgnoreObstacles) = 0;
        virtual int FindWalls(const ZLocation& kSource, float fMaxDistance, float* pWall) = 0;
        virtual int FindObstacles(const ZLocation& kLocation, ZDynamicObstacle** obstacles2, int max) = 0;
        virtual void RemapDoorRefs(ZREF* pRefs, uint32_t lRefsNr) = 0;
    };

    ZInterface* CreatePathFinder(void* data);
}
