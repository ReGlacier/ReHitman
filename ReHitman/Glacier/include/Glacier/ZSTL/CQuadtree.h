#pragma once

#include <Glacier/ZSTL/CTreeObject.h>
#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ZSTL/CMemPool.h>


namespace Glacier
{
	struct CNodeQuad
	{
		uint16_t m_aiChildren[4];
		uint16_t m_iParent;
		uint16_t m_iDepth;
		CTreeObjectList m_tObjectList;
	};
	RE_VERIFY_SIZE(CNodeQuad, 0x10);

	struct CQuadtreeObj : CTreeObject
	{
		CNodeQuad* m_pNode;
	};
	RE_VERIFY_SIZE(CQuadtreeObj, 0x1C);

	struct CQuadtree
	{
		CNodeQuad* m_pRoot;
		CMemPool m_tPool;
		int m_iObjects;
		float m_fScale;
		ZVector3 m_vOrigin;
	};

	RE_VERIFY_SIZE(CQuadtree, 0x30);
}