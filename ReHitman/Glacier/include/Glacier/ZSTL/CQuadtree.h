#pragma once

#include <Glacier/ZSTL/CTreeObject.h>
#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
	//fwds
	struct CQuadtreeObj;
	struct SRecurseInfoCompiled;

	struct SRecurseAdd
	{
		CMemPool* pPool{nullptr};
		CQuadtreeObj *pObject{nullptr};
		int iCurDepth;
		int iWantedDepth;
		int iX;
		int iY;
		int iZ;	
	};
	RE_VERIFY_SIZE(SRecurseAdd, 0x1C);

	struct CNodeQuad
	{
		// methods
		void Init(CMemPool *pPool, CNodeQuad *pQuad);
		void DeleteEmpty(CMemPool* pPool);
		void DeleteEmptySingle(CMemPool* pPool);
		void Add(SRecurseAdd* pInfo);
		void GetObjects(SRecurseInfoCompiled* pInfo, CMemPool* pPool);
		void CheckCube(SRecurseInfoCompiled* pInfo, CMemPool* pPool, int iDepth, int iNodeCenterX, int iNodeCenterZ);
		bool CheckLinesegmentB(SRecurseInfoCompiled* pInfo, CMemPool* pPool, int iNodeDim, int iCenX, int iCenZ);
		bool IsEmpty() const;
		bool IsLeaf() const;
		CNodeQuad* GetParentPtr(CMemPool* pPool) const;
		CNodeQuad* GetChildPtr(CMemPool* pPool, int iChildIdx) const;
		int GetDepth() const;
		void Attach(CQuadtreeObj* pObject);
		void Detach(CQuadtreeObj* pObject);
		void SetChild(int iChildIndex, int16_t nChildId);
		bool ChildExists(int iChildIndex) const;

		// members
		uint16_t m_aiChildren[4];
		uint16_t m_iParent;
		uint16_t m_iDepth;
		CTreeObjectList m_tObjectList;
	};
	RE_VERIFY_SIZE(CNodeQuad, 0x10);

	struct CQuadtreeObj : CTreeObject
	{
		// methods
		CQuadtreeObj(int iID) : CTreeObject(iID) {}
		
		// members
		CNodeQuad* m_pNode;
	};
	RE_VERIFY_SIZE(CQuadtreeObj, 0x1C);

	struct SOctreeChk;

	struct SOctreeChk
	{
		// methods
		SOctreeChk() = default;
		// members
		float fResT = 1.0f;
		bool(*pChkFunc)(unsigned int, SOctreeChk*) = nullptr;
		void* pUserData = nullptr;
	};
	RE_VERIFY_SIZE(SOctreeChk, 0xC);

	struct SRecurseInfoCompiled : public SOctreeChk
	{
		// methods
		SRecurseInfoCompiled() = default;

		// members
		void *pxBasePtrNodes = nullptr;
		void *pxBasePtrObjects = nullptr;
		int iMinX = 0;
		int iMinY = 0;
		int iMinZ = 0;
		int iMaxX = 0;
		int iMaxY = 0;
		int iMaxZ = 0;
		int iDepth = 0;
		float *pA = nullptr;
		float *pB = nullptr;
		ZVector3 vDir{};
		ZVector3 vInvDir{};
		int vPos[3]{0,0,0};
	};
	RE_VERIFY_SIZE(SRecurseInfoCompiled, 0x5C);

	/**
	 * @brief Fixed-size spatial quadtree for storing 3D AABB objects by their X/Z position.
	 *
	 * The tree stores object bounds in object-coordinate space (OCS), where X/Y/Z are 16-bit
	 * integer coordinates. Subdivision is performed on the X/Z plane; Y is kept only in each
	 * object's AABB and is used by cube and line segment checks. Nodes are allocated from an
	 * internal CMemPool with a caller-provided maximum node count.
	 */
	struct CQuadtree
	{
		// methods
		/** @brief Creates a quadtree with room for up to @p nMaxNodes nodes. */
		CQuadtree(int nMaxNodes);
		/** @brief Releases the internal node pool. Stored objects should be deleted before destruction. */
		~CQuadtree();

		/** @brief Sets the world-space origin used by ConvToOCS and vector overloads. */
		void SetOrigin(const ZVector3& vOrigin);
		/** @brief Sets the world-to-OCS scale. Must be greater than zero. */
		void SetScale(float fScale);
		/** @brief Returns the target tree depth for an object whose largest AABB dimension is @p iDimension. */
		int GetDepth(int iDimension) const;
		/** @brief Adds an object with OCS min/max bounds and user id @p iID. */
		void AddMinMax(int iMinX, int iMinY, int iMinZ, int iMaxX, int iMaxY, int iMaxZ, unsigned int iID);
		/** @brief Adds an object with world-space min/max bounds and user id @p iID. */
		void AddMinMax(const ZVector3& pMin, const ZVector3& pMax, unsigned int iID);
		/** @brief Updates an existing object's OCS bounds and moves it to a suitable node if needed. */
		void Move(CQuadtreeObj* pObject, int* pMin, int* pMax);
		/** @brief Updates an existing object's world-space bounds and moves it if needed. */
		void Move(CQuadtreeObj* pObject, const ZVector3& pMin, const ZVector3& pMax);
		/** @brief Removes an object from the tree and destroys it. */
		void Delete(CQuadtreeObj *pObject);
		/** @brief Unlinks an object from its current node without destroying it. */
		void Detach(CQuadtreeObj* pObject);
		/** @brief Visits every object in the tree using SRecurseInfoCompiled::pChkFunc. */
		void GetEverything(SRecurseInfoCompiled* pInfo);
		/** @brief Visits objects overlapping the world-space query AABB. */
		void CheckCube(SRecurseInfoCompiled* pInfo, const ZVector3& pMin, const ZVector3& pMax);
		/** @brief Visits objects overlapping the OCS query AABB. */
		void CheckCube(SRecurseInfoCompiled* pInfo, int iMinX, int iMinY, int iMinZ, int iMaxX, int iMaxY, int iMaxZ);
		/** @brief Tests a world-space line segment against stored AABBs and calls pChkFunc for hits. */
		bool CheckLinesegment(SRecurseInfoCompiled *pInfo, const ZVector3& pA, const ZVector3& pB);
		/** @brief Converts a world-space float[3] position to integer OCS coordinates. */
		void ConvToOCS(int* pOutOcsPos, const float* pInWorldPos) const;
		/** @brief Converts a world-space vector position to integer OCS coordinates. */
		void ConvToOCS(int* pOutOcsPos, const ZVector3& pInWorldPos) const;
		/** @brief Allocates a new quadtree object wrapper for @p iID. */
		CQuadtreeObj* CreateObject(int iID) const;

		// members
		CNodeQuad* m_pRoot;
		CMemPool m_tPool;
		int m_iObjects;
		float m_fScale;
		ZVector3 m_vOrigin;
	};
	RE_VERIFY_SIZE(CQuadtree, 0x30);
}
