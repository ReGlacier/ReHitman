#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Decal/ZDecalMarkControllerBase.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/ZSTL/ZFixedArray.h>
#include <Glacier/ZSTL/ZHash.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    using DECALCALLBACK = void(*)(ZDecalCallBack* pDecal, void* pData, uint32_t lValue);

    class ZDecalMarkController : public ZDecalMarkControllerBase
    {
    public:
        // types
        struct ZLink;

        struct ZDecalMark
        {
            ZVector3 m_vPosition;
            ZVector3 m_vDirection;
            float m_fRotation;
            uint32_t m_lSourcePrim;
            ZVector2 m_fRadius;
            ZVector4 m_fExtraTextureSize;
            float m_fOpacity;
            uint32_t m_lStoredUVSize;
            DECALCALLBACK m_CallBackDecal;
            void* m_pCallBackData;
            uint32_t m_lCallBackValue;
            bool m_bStoreUV;
            ZDecalMarkController::ZDecalMark* m_pPrev;
            ZDecalMarkController::ZDecalMark* m_pNext;
            ZDecalMarkController::ZDecalMark* m_pNextCallBack;
            ZDecalMarkController::ZDecalMark* m_pNextToCreate;
            ZDecalMarkController::ZDecalMark* m_pNextToRemove;
            ZDecalMarkController::ZLink* m_pLinks;
            bool m_bRemoved;
        };

        struct ZLink
        {
            ZBaseGeom* m_pBaseGeom;
            ZPrimAccessMesh* m_pPrimAccessMesh;
            float* m_pStoredUV;
            ZDecalMarkController::ZDecalMark* m_pDecalMark;
            ZDecalMarkController::ZLink* m_pPrev;
            ZDecalMarkController::ZLink* m_pNext;
            ZDecalMarkController::ZLink* m_pPrevSameGeom;
            ZDecalMarkController::ZLink* m_pNextSameGeom;
        };

        // methods
        ZDecalMarkController();
        ~ZDecalMarkController();

        void Init();
        void End();
        void Flush();
        void BeginFrame();

        ZDecalMarkController::ZDecalMark* Add(const float* pvPosition, const float* pvDirection, uint32_t lSourcePrim, float fRadiusX, float fRadiusY, const float* pvExtraTextureSize, bool bStoreUV, float fRotation);
        void Modify(ZDecalMarkController::ZDecalMark* pDecalMark, float fOpacity);
        void Remove(ZDecalMarkController::ZDecalMark* pDecalMark);
        void SetCallBack(ZDecalMarkController::ZDecalMark* pDecalMark, DECALCALLBACK pCallBackDecal, void* pCallBackData, uint32_t lCallBackValue);
        uint32_t GetAttachedObjects(ZBaseGeom* pBaseGeom, ZPrimHandle* pObjects, uint32_t lMaxNumObjects);
        void RemoveBaseGeom(ZBaseGeom* pBaseGeom);
        void RemoveAllDecals();

        void Create(ZDecalMarkController::ZDecalMark* pDecalMark);
        ZPrimAccessMesh* CreateDecal(ZBaseGeom* pBaseGeom, uint32_t lSourcePrim, const float* pvPosition, const float* pvDirection, float fRadius, float fRotation, const float* pvExtraTextureSize);
        bool RegisterDecalMesh(ZDecalMarkController::ZDecalMark* pDecalMark, ZBaseGeom* pBaseGeom, ZPrimAccessMesh* pMesh);
        void RemoveDecal(ZDecalMarkController::ZDecalMark* pDecalMark);
        ZDecalMarkController::ZLink* RemoveLink(ZBaseGeom* pBaseGeom);
        void RecreateObjects(ZBaseGeom* pBaseGeom);

        // members
        ZFixedArray<ZDecalMarkController::ZDecalMark, 128> m_DecalMarks; // PC/iOS - 128, XBox - 512
        ZFixedArray<ZDecalMarkController::ZLink, 256> m_Links; // PC/iOS - 256, XBox - 768
        ZDecalMarkController::ZDecalMark* m_pFirst;
        ZDecalMarkController::ZDecalMark* m_pFirstCallBack;
        ZDecalMarkController::ZDecalMark* m_pFirstCreated;
        ZDecalMarkController::ZDecalMark* m_pFirstRemoved;
        ZHash<int, ZDecalMarkController::ZLink*> m_BaseGeomToLink; // Original type: ZIntHash<ZLink*> (ZHash<int, V> subclass with IntValue override)
    };
    RE_VERIFY_SIZE(ZDecalMarkController::ZDecalMark, 0x6C); // Approved by PC ctor (108 byte stride)
    RE_VERIFY_SIZE(ZDecalMarkController::ZLink, 0x20); // Approved by PC ctor (32 byte stride)
    RE_VERIFY_OFFSET(ZDecalMarkController::ZDecalMark, m_bStoreUV, 0x4C); // Approved by PC RegisterDecalMesh
    RE_VERIFY_OFFSET(ZDecalMarkController::ZDecalMark, m_pLinks, 0x64); // Approved by PC RegisterDecalMesh
    RE_VERIFY_OFFSET(ZDecalMarkController::ZDecalMark, m_bRemoved, 0x68); // Approved by PC ZRenderDraw::RemoveMark
    RE_VERIFY_OFFSET(ZDecalMarkController, m_DecalMarks, 0x280C); // Approved by PC ctor
    RE_VERIFY_OFFSET(ZDecalMarkController, m_Links, 0x5E10); // Approved by PC ctor
    RE_VERIFY_OFFSET(ZDecalMarkController, m_pFirst, 0x7E14); // Approved by PC ctor
    RE_VERIFY_OFFSET(ZDecalMarkController, m_pFirstCallBack, 0x7E18); // Approved by PC ZRenderDraw::SetMarkCallBack
    RE_VERIFY_OFFSET(ZDecalMarkController, m_pFirstCreated, 0x7E1C); // Approved by PC Add
    RE_VERIFY_OFFSET(ZDecalMarkController, m_pFirstRemoved, 0x7E20); // Approved by PC ZRenderDraw::RemoveMark
    RE_VERIFY_OFFSET(ZDecalMarkController, m_BaseGeomToLink, 0x7E24); // Approved by PC ctor
    RE_VERIFY_SIZE(ZDecalMarkController, 0x7E44); // Approved by PC ZRenderDraw::Ctor (controller at +0x12C, next member at +0x27F70)
}
