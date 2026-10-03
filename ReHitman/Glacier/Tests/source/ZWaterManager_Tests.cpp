#include <Glacier/Render/ZWaterManager.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <cmath>

using namespace Glacier;

namespace
{
    // ZWaterManager has no destructor, so tests release the constructor
    // allocations explicitly to keep the runner clean.
    void ReleaseManager(ZWaterManager& manager)
    {
        ZUniMemory::Free(manager.m_pPatches);
        ZUniMemory::Free(manager.m_pPatchLinearMem);
        manager.m_pPatches = nullptr;
        manager.m_pPatchLinearMem = nullptr;
    }
}

TEST(ZWaterManager, DefaultConstructorAllocatesFullCapacity)
{
    ZWaterManager manager;

    EXPECT_EQ(manager.m_iTotalPatches, 256);
    EXPECT_EQ(manager.m_iNumPatches, 0);
    EXPECT_EQ(manager.m_iNumRipples, 0);
    ASSERT_NE(manager.m_pPatchLinearMem, nullptr);
    ASSERT_NE(manager.m_pPatches, nullptr);

    for (int i = 0; i < manager.m_iTotalPatches; ++i)
    {
        EXPECT_EQ(manager.m_pPatches[i], &manager.m_pPatchLinearMem[i]);
    }

    ReleaseManager(manager);
}

TEST(ZWaterManager, ConstructorKeepsRequestedCapacity)
{
    ZWaterManager manager(4);

    EXPECT_EQ(manager.m_iTotalPatches, 4);
    EXPECT_EQ(manager.m_iNumPatches, 0);
    ASSERT_NE(manager.m_pPatches, nullptr);

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(manager.m_pPatches[i], &manager.m_pPatchLinearMem[i]);
    }

    ReleaseManager(manager);
}

TEST(ZWaterManager, ConstructorClampsCapacityAboveLimit)
{
    ZWaterManager manager(1000);

    EXPECT_EQ(manager.m_iTotalPatches, 256);

    ReleaseManager(manager);
}

TEST(ZWaterManager, AllocWaterPatchHandsOutSequentialPatches)
{
    ZWaterManager manager(4);

    int keyA = 0;
    int keyB = 0;

    SWaterPatchInst* pA = manager.AllocWaterPatch(&keyA);
    SWaterPatchInst* pB = manager.AllocWaterPatch(&keyB);

    ASSERT_NE(pA, nullptr);
    ASSERT_NE(pB, nullptr);
    EXPECT_EQ(pA, &manager.m_pPatchLinearMem[0]);
    EXPECT_EQ(pB, &manager.m_pPatchLinearMem[1]);
    EXPECT_EQ(pA->pKey, &keyA);
    EXPECT_EQ(pB->pKey, &keyB);
    EXPECT_EQ(pA->iRipples, 0);
    EXPECT_EQ(manager.m_iNumPatches, 2);

    ReleaseManager(manager);
}

TEST(ZWaterManager, AllocWaterPatchReturnsNullptrWhenFull)
{
    ZWaterManager manager(2);

    int keyA = 0;
    int keyB = 0;
    int keyC = 0;

    ASSERT_NE(manager.AllocWaterPatch(&keyA), nullptr);
    ASSERT_NE(manager.AllocWaterPatch(&keyB), nullptr);
    EXPECT_EQ(manager.m_iNumPatches, 2);

    EXPECT_EQ(manager.AllocWaterPatch(&keyC), nullptr);
    EXPECT_EQ(manager.m_iNumPatches, 2);

    ReleaseManager(manager);
}

TEST(ZWaterManager, FreeWaterPatchSwapsInLastPatch)
{
    ZWaterManager manager(4);

    int keyA = 0;
    int keyB = 0;
    int keyC = 0;
    int keyD = 0;

    SWaterPatchInst* pA = manager.AllocWaterPatch(&keyA);
    SWaterPatchInst* pB = manager.AllocWaterPatch(&keyB);
    SWaterPatchInst* pC = manager.AllocWaterPatch(&keyC);
    SWaterPatchInst* pD = manager.AllocWaterPatch(&keyD);
    ASSERT_NE(pA, nullptr);
    ASSERT_NE(pB, nullptr);
    ASSERT_NE(pC, nullptr);
    ASSERT_NE(pD, nullptr);

    manager.FreeWaterPatch(&keyB);

    EXPECT_EQ(manager.m_iNumPatches, 3);
    EXPECT_EQ(manager.m_pPatches[0], pA);
    EXPECT_EQ(manager.m_pPatches[1], pD);
    EXPECT_EQ(manager.m_pPatches[2], pC);

    ReleaseManager(manager);
}

TEST(ZWaterManager, FreeWaterPatchRemovingLastLeavesOrderIntact)
{
    ZWaterManager manager(3);

    int keyA = 0;
    int keyB = 0;
    int keyC = 0;

    SWaterPatchInst* pA = manager.AllocWaterPatch(&keyA);
    SWaterPatchInst* pB = manager.AllocWaterPatch(&keyB);
    SWaterPatchInst* pC = manager.AllocWaterPatch(&keyC);
    ASSERT_NE(pA, nullptr);
    ASSERT_NE(pB, nullptr);
    ASSERT_NE(pC, nullptr);

    manager.FreeWaterPatch(&keyC);

    EXPECT_EQ(manager.m_iNumPatches, 2);
    EXPECT_EQ(manager.m_pPatches[0], pA);
    EXPECT_EQ(manager.m_pPatches[1], pB);

    ReleaseManager(manager);
}

TEST(ZWaterManager, FreeWaterPatchIgnoresUnknownKeyAndEmptyManager)
{
    ZWaterManager manager(2);

    // Empty manager: must not touch memory or crash.
    manager.FreeWaterPatch(reinterpret_cast<void*>(0x1234));
    EXPECT_EQ(manager.m_iNumPatches, 0);

    int keyA = 0;
    int keyB = 0;
    ASSERT_NE(manager.AllocWaterPatch(&keyA), nullptr);
    ASSERT_NE(manager.AllocWaterPatch(&keyB), nullptr);

    int unknownKey = 0;
    manager.FreeWaterPatch(&unknownKey);

    EXPECT_EQ(manager.m_iNumPatches, 2);

    ReleaseManager(manager);
}

TEST(ZWaterManager, UpdateRippleInfoSelectsOverlappingRipples)
{
    ZWaterManager manager(1);

    manager.m_iNumRipples = 2;

    SWaterRippleInst& inside = manager.m_pRipples[0];
    inside = {};
    inside.fXCenter = 1.0f;
    inside.fZCenter = 1.0f;
    inside.fRadius = 0.5f;

    SWaterRippleInst& outside = manager.m_pRipples[1];
    outside = {};
    outside.fXCenter = 100.0f;
    outside.fZCenter = 1.0f;
    outside.fRadius = 0.5f;

    SWaterPatchInst patch{};
    patch.iXsegs = 2;
    patch.iZsegs = 2;
    patch.fSegSizeX = 1.0f;
    patch.fSegSizeZ = 1.0f;
    patch.fXPos = 0.0f;
    patch.fZPos = 0.0f;

    EXPECT_EQ(manager.UpdateRippleInfo(&patch), 1);
    EXPECT_EQ(patch.iRipples, 1);
    EXPECT_EQ(patch.ripples[0], &inside);

    ReleaseManager(manager);
}

TEST(ZWaterManager, UpdateRippleInfoCollectsAllOverlappingRipples)
{
    ZWaterManager manager(1);

    // m_pRipples holds at most 8 instances, matching the ripples[] capacity.
    manager.m_iNumRipples = 8;
    for (int i = 0; i < 8; ++i)
    {
        manager.m_pRipples[i] = {};
        manager.m_pRipples[i].fXCenter = 1.0f;
        manager.m_pRipples[i].fZCenter = 1.0f;
        manager.m_pRipples[i].fRadius = 0.1f;
    }

    SWaterPatchInst patch{};
    patch.iXsegs = 2;
    patch.iZsegs = 2;
    patch.fSegSizeX = 1.0f;
    patch.fSegSizeZ = 1.0f;

    EXPECT_EQ(manager.UpdateRippleInfo(&patch), 8);
    EXPECT_EQ(patch.iRipples, 8);
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(patch.ripples[i], &manager.m_pRipples[i]);
    }

    ReleaseManager(manager);
}

TEST(ZWaterManager, FrameUpdateAdvancesLivingRipple)
{
    ZWaterManager manager(1);

    manager.m_iNumRipples = 1;
    SWaterRippleInst& ripple = manager.m_pRipples[0];
    ripple = {};
    ripple.fAmplitude = 1.0f;
    ripple.fDamping = 0.25f;
    ripple.fRadius = 1.0f;
    ripple.fRadiusSpeed = 0.5f;
    ripple.fSpeed = 1.0f;

    manager.FrameUpdate(0.5f);

    EXPECT_EQ(manager.m_iNumRipples, 1);
    EXPECT_FLOAT_EQ(ripple.fAmplitude, 0.75f);
    EXPECT_FLOAT_EQ(ripple.fRadius, 1.5f);
    EXPECT_FLOAT_EQ(ripple.fOneOverRadius, 1.0f / 1.5f);
    EXPECT_FLOAT_EQ(ripple.fPhase, 0.5f * 1.0f * 6.2831855f);

    ReleaseManager(manager);
}

TEST(ZWaterManager, FrameUpdateRemovesExpiredRipple)
{
    ZWaterManager manager(1);

    manager.m_iNumRipples = 1;
    SWaterRippleInst& ripple = manager.m_pRipples[0];
    ripple = {};
    ripple.fAmplitude = 0.2f;
    ripple.fDamping = 0.25f;
    ripple.fRadius = 1.0f;
    ripple.fRadiusSpeed = 1.0f;

    manager.FrameUpdate(0.1f);

    EXPECT_EQ(manager.m_iNumRipples, 0);

    ReleaseManager(manager);
}

TEST(ZWaterManager, FrameUpdateCompactsRipplesArray)
{
    ZWaterManager manager(1);

    manager.m_iNumRipples = 2;

    SWaterRippleInst& dying = manager.m_pRipples[0];
    dying = {};
    dying.fAmplitude = 0.1f;
    dying.fDamping = 0.2f;

    SWaterRippleInst& living = manager.m_pRipples[1];
    living = {};
    living.fAmplitude = 1.0f;
    living.fDamping = 0.0f;
    living.fRadius = 2.0f;
    living.fRadiusSpeed = 0.5f;

    manager.FrameUpdate(0.1f);

    EXPECT_EQ(manager.m_iNumRipples, 1);
    EXPECT_FLOAT_EQ(manager.m_pRipples[0].fAmplitude, 1.0f);
    EXPECT_FLOAT_EQ(manager.m_pRipples[0].fRadius, 2.5f);

    ReleaseManager(manager);
}

TEST(ZWaterManager, FrameUpdateRefreshesPatchRippleInfo)
{
    ZWaterManager manager(1);

    int key = 0;
    SWaterPatchInst* patch = manager.AllocWaterPatch(&key);
    ASSERT_NE(patch, nullptr);

    patch->iXsegs = 2;
    patch->iZsegs = 2;
    patch->fSegSizeX = 1.0f;
    patch->fSegSizeZ = 1.0f;
    patch->fXPos = 0.0f;
    patch->fZPos = 0.0f;

    manager.m_iNumRipples = 1;
    manager.m_pRipples[0] = {};
    manager.m_pRipples[0].fXCenter = 1.0f;
    manager.m_pRipples[0].fZCenter = 1.0f;
    manager.m_pRipples[0].fRadius = 0.5f;
    manager.m_pRipples[0].fAmplitude = 1.0f;
    manager.m_pRipples[0].fDamping = 0.0f;

    manager.FrameUpdate(0.1f);

    EXPECT_EQ(patch->iRipples, 1);
    EXPECT_EQ(patch->ripples[0], &manager.m_pRipples[0]);

    ReleaseManager(manager);
}
