#include <Glacier/Render/Entry/ZRenderEntryBonesRigid.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    namespace
    {
        uint32_t ReadHiddenBoneIndices(const ZBaseGeom* pBaseGeom)
        {
            if (!pBaseGeom)
                return 0;

            auto* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->Is<ZLNKOBJ>())
                return 0;

            const auto* pBoneModifier = static_cast<const ZLNKOBJ*>(pGeom)->GetBoneModifier();
            return pBoneModifier ? pBoneModifier->m_lHiddenBoneIds : 0;
        }
    }

    ZRenderEntryBonesRigid::ZRenderEntryBonesRigid(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryBones(sInfo)
        , m_lHiddenBoneIndices(ReadHiddenBoneIndices(m_pBaseGeom))
    {
    }

    ZRenderEntryBonesRigid::~ZRenderEntryBonesRigid() = default;

    void __thiscall ZRenderEntryBonesRigid::Update()
    {
        if (!m_lLODLevelsWanted)
            return;

        ZRenderEntryGeom::Update();
        m_lHiddenBoneIndices = ReadHiddenBoneIndices(m_pBaseGeom);
    }

    void ZRenderEntryBonesRigid::SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance)
    {
        ZASSERT(pContext != nullptr);
        ZASSERT(pObjInstance != nullptr);
        if (!pContext || !pObjInstance || !pObjInstance->m_pRenderObject || !m_pBoneData)
            return;

        const auto* pPrim = pObjInstance->m_pRenderObject->m_hPrim.Get<const uint8_t>();
        if (!pPrim)
            return;

        const uint32_t lTransformIndex = *reinterpret_cast<const uint32_t*>(pPrim + 0x3C);
        const auto* pTransform = reinterpret_cast<const float*>(
            m_pBoneData + 48u * (lTransformIndex + 3u));

        ZMat3x3 mBone;
        for (uint32_t i = 0; i < 9; ++i)
            mBone.data[i] = pTransform[i];

        ZMat3x3 mRotation;
        tmat(mRotation.Get(), mBone.Get());
        ZMatrix result;
        mmmul(result.m0.Get(), mRotation.Get(), m_ObjectToWorldMatrix.m0.Get());

        ZVector3 position{
            pTransform[3], pTransform[7], pTransform[11] };
        vmmul(position, m_ObjectToWorldMatrix.m0);
        position += m_ObjectToWorldMatrix.p0;
        result.p0 = position;

        pContext->m_ObjectToWorldMatrix = result;
        pContext->m_pDeformBones = nullptr;
        pContext->m_pBonesLight = m_pLightData
            ? m_pLightData
            : reinterpret_cast<const float*>(m_pBoneData + 32);

        const float fFade = (m_lFade == 0xFF ? 2.0f : static_cast<float>(m_lFade) * 0.0039370079f)
            * 0.85000002f + 0.15000001f;
        pContext->m_fObjectFade = fFade > 1.0f ? 1.0f : fFade;
    }

    uint32_t ZRenderEntryBonesRigid::GetHiddenBoneIndices() const
    {
        return m_lHiddenBoneIndices;
    }

    uint32_t ZRenderEntryBonesRigid::GetBoneIndexMask(const SPrimObject* pPrimObject)
    {
        if (!pPrimObject)
            return 0u;

        // PC receives the larger primitive-info record here and reads its
        // bone index at +0x3C; SPrimObject is only the compact public view.
        const auto lBoneIndex = *reinterpret_cast<const uint32_t*>(
            reinterpret_cast<const char*>(pPrimObject) + 0x3C);
        return 1u << lBoneIndex;
    }

    ZRenderEntryBonesRigid* ZRenderEntryBonesRigid::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        return ZUniMemory::New<ZRenderEntryBonesRigid>(sInfo);
    }
}
