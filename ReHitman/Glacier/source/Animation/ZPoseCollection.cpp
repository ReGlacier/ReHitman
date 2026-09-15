#include <Glacier/Animation/ZPoseCollection.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPoseBoneHeader.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>


namespace Glacier::Animation
{
    ZPoseCollection::ZPoseCollection() = default;

    ZPoseBone* ZPoseCollection::poseData()
    {
        auto* hdr = ZPrimControlBase::GetPrimitive<ZPoseBoneHeader>(m_PoseIdx);
        ZASSERT(hdr);

        return ZPrimControlBase::GetPrimitive<ZPoseBone>(hdr->m_PoseData);
    }

    int32_t ZPoseCollection::poseCount()
    {
        auto* hdr = ZPrimControlBase::GetPrimitive<ZPoseBoneHeader>(m_PoseIdx);
        ZASSERT(hdr);

        return hdr ? hdr->m_PoseCount : 0;
    }

    int32_t* ZPoseCollection::indexToOffsetLookup()
    {
        auto* hdr = ZPrimControlBase::GetPrimitive<ZPoseBoneHeader>(m_PoseIdx);
        ZASSERT(hdr);

        return ZPrimControlBase::GetPrimitive<int32_t>(hdr->m_PoseIndexToOffsetLookup);
    }

    int16_t* ZPoseCollection::idToPosLookup()
    {
        auto* hdr = ZPrimControlBase::GetPrimitive<ZPoseBoneHeader>(m_PoseIdx);
        ZASSERT(hdr);

        return ZPrimControlBase::GetPrimitive<int16_t>(hdr->m_PoseIdToPosLookup);
    }
}
