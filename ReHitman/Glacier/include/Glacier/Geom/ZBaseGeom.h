#pragma once

#include <Glacier/ZUniMemory.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <Glacier/Geom/ZGeomEventList.h>
#include <Glacier/Geom/ExGeomData.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <Glacier/Fysix/eGlobalTreeType.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstdint>
#include <cstddef>


namespace Glacier
{
    //fwds
    class ZTreeGroup;
    class ZGROUP;

    class ZBaseGeom
    {
    public:
        // constants
        static constexpr float MIN_GEOM_SIZE = 1.f / 4096.f;

        // static members

        // members
        ZMat3x3 m_mMat;
        ZVector3 m_vPos;
        ZVector3 m_vCen;
        unsigned int m_lControl;
        ZVector3 m_vSize;
        float m_fRadius;
        ZBaseGeom * m_pParent;
        uint32_t m_uListID : 24;
        uint32_t m_lPotentialLightListChange : 7;
        uint32_t m_bFreezeLightList : 1;

        union
        {
            uint16_t m_lDrawId;
            uint16_t m_lDrawEntryId;
        };

        union
        {
            uint16_t m_iRoomListNr;
            uint16_t m_iDynamicParentNr;
        };

        uint16_t m_iPrev;
        uint16_t m_iNext;

        class ZGEOM* m_pExtraGeom;
        CQuadtreeObj* m_pDynId;
        unsigned int m_lPrim;
        const char* m_Name;

        // vtbl - empty
        // static methods
        static ZBaseGeom* RefToPtr(ZREF rRef);
        static ZBaseGeom* GetBaseGeomPtrFromOffset(uint32_t lOffset);
        static uint32_t GetBaseGeomOffsetFromPtr(ZBaseGeom* pBaseGeom);

        // methods
        ZBaseGeom();
        ~ZBaseGeom();

        // allocation & deallocation
        static void* operator new(std::size_t size);
        static void* operator new(std::size_t size, ZREF ref);
        static void* operator new(std::size_t size, ZREF ref, const char* file, uint32_t line);
        static void operator delete(void* ptr) noexcept;
        static void operator delete(void* ptr, ZREF ref) noexcept;
        static void operator delete(void* ptr, ZREF ref, const char* file, uint32_t line) noexcept;

        void LoadSave(ISerializerStream& stream, bool bSaving);
        const char* Name() const;
        bool DoInit();
        ZGROUP* ParentGroup() const;
        ZROOM* GetOwnerRoom() const;
        void SetName(const char* name);
        bool SetPrim(int primId);
        void GetMatPos(Glacier::ZMat3x3& mat, Glacier::ZVector3& pos);
        uint32_t DrawId() const;
        uint32_t ListId() const;
        ZBaseGeom* Next();
        void SetNext(ZBaseGeom* next);
        ZBaseGeom* GetPrev();
        void SetPrev(ZBaseGeom* prev);
        ZGEOM* GetGeom() const { return m_pExtraGeom; }
        uint32_t ColiId() const;
        uint32_t GetColiBits() const;
        void SetColiId(uint32_t lColi);
        ZBaseGeom* Parent() const;
        void SetParent(ZBaseGeom* pBaseGeom);
        void SetMat(const float* m0);
        void SetPos(const float* p);
        void SetMatPos(const float* m, const float* p);
        void SetPosSimple(const float* p);
        void SetMatSimple(const float* m0);
        void SetRootPos(const float* p);
        void GetRootTM(ZMat3x3& mat, ZVector3& pos) const;
        void SetRootTM(const ZMat3x3& mat, const ZVector3& pos);
        void GetRootPoint(ZVector3& point) const;
        void GetRootMat(ZMat3x3& mat) const;
        void GetRootVect(ZVector3& vect) const;
        void GetPos(ZVector3& pos) const;
        void GetCen(ZVector3& cen) const;
        void GetSize(ZVector3& size) const;
        void GetRootMatPos(ZMat3x3& mat, ZVector3& pos) const;
        void GetLocalMat(ZMat3x3& mat);
        void GetLocalMatPos(ZMat3x3& mat, ZVector3& pos) const;
        void GetLocalPoint(ZVector3& point) const;
        void SetCen(const ZVector3& vCenter);
        void SetCen(float fX, float fY, float fZ);
        void SetCenSimple(const ZVector3& vCenter);
        void SetSize(const ZVector3& vSize);
        void SetSizeSimple(const ZVector3& vSize);
        void SetRadius(float fRadius);
        void SetRadiusSimple(float fRadius);
        const float* Mat() const;
        const float* Pos() const;
        const float* Cen() const;
        const float* Size() const;
        uint32_t GetRef() const;
        float Radius() const;
        void SetControlDirect(uint32_t lAddBits, uint32_t lRemBits);
        uint32_t Control() const;
        uint32_t GeomControl() const;
        void SetControl(uint32_t lControl);
        void SetControl(uint32_t lAddBits, uint32_t lRemBits);
        uint32_t Prim() const;
        void UpdateMovedGeom();
        void DynamicPosChanged();
        ZGROUP* GetOwner(bool bCheckWorldGroup) const;
        ZBaseGeom* GetDynamicParent() const;
        ZBaseGeom* GetDynamicParentPtr() const;
        ZBaseGeomRoomList* GetRoomList() const;
        ZBaseGeomRoomList* GetRoomListPtr() const;
        ZTreeGroup* GetTreeGroup() const;
        ZTreeGroup* GetDynamicTreeGroup() const;
        eGlobalTreeType GetBoundTreeType() const;
        void FreeRoomList();
        void SetRoomList(ZBaseGeomRoomList* pRoomList);
        void RemoveFromRoomList(ZROOM* pRoom);
        void AddToRoomList(ZROOM* pRoom);
        void AttachToRoomsDrawLists(ZROOM* pRoom);
        void DetachFromRoomsDrawLists(ZROOM* pRoom);
        void AttachToDynamicContainer();
        void DetachFromDynamicContainer(ZGROUP* pOldParent);
        bool Active() const;
        void MakeInactive();
        void MakeActive();
        void MakeDynamic(bool bDynamic);
        void Hide(bool bHide);
        void Freeze(bool bFreeze);
        void HideRecursive(bool bHide);
        bool ChkEvents() const;
        bool ChkUpdateMinMax() const;
        void SendCommand(ZMSGID Msg, void* pData, ZGEOM* pTarget);
        void SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget);
        bool CheckPointInside(ZVector3& vPoint, float fDotDist) const;
        void CopyData(const ZBaseGeom* Source);
        void CopyGeometry(const ZBaseGeom* Source);
        void CopyCenSizeRadius(const ZBaseGeom* Source);
        void CopyMatPos(const ZBaseGeom* Source);
        void SetDynamicParent(ZBaseGeom* pParent);
        void SetDynamicParentPtr(const ZBaseGeom* pParent);
        void SetAutoRoomAssign(bool bAutoAssign);
        bool IsHidden() const;
        bool IsMovingObject() const;
        bool WantCameraMsg() const;
        MYSTR CalcTotalName(bool bRoot) const;
        bool RequestCustomDraw() const;
        void CreateUniquePrim();
        void SetOwnerDraw(bool bOwnerDraw);
        void SetUpdateLight(bool bUpdateLight);
        void SetIsMoving(bool bMoving);
        void SetMainRoom(ZROOM* pRoom);
        void GetLocalVect(ZVector3& vLocalVect);
        void GetLocalPointVect(ZVector3& point, ZVector3& vect);
        void GetRootPointVect(float*, float*);
        void Zvmmul(ZVector3& v) const;
        void AssignToRooms();
        void AutoAssignToRooms();
        void AdjustRoomList(ZROOM** ppRooms, uint32_t lNrRooms);
        void CalcCenSize(bool bCalledByZGEOM);
        void ForceCalcMaxMin();
        void AdjustMinMax(ZBaseGeom* pChildBaseGeom);
        bool DisableParentBoundAdjust() const;
        bool GetParentBox(ZVector3& vCen, ZVector3& vSize) const;
        bool CalcPrimCenSizeAlongMat(ZVector3& vCen, ZVector3& vSize, ZMat3x3& mMat) const;
        void LightNotifyPotentialDetachment(bool bIncrease);
        void FixLightList();
        void UpdateLightListForLight();
        void UpdateLightListForGeom();
        void SetAttachUpdate();

        template <typename TGeom> bool IsDerivedFrom() const;
        bool IsDerivedFromStdObj(uint32_t lClassId) const;
    }; //Size: 0x0070
    RE_VERIFY_SIZE(ZBaseGeom, 0x70);
    RE_VERIFY_OFFSET(ZBaseGeom, m_mMat, 0x0);
    RE_VERIFY_OFFSET(ZBaseGeom, m_vPos, 0x24);
    RE_VERIFY_OFFSET(ZBaseGeom, m_vCen, 0x30);
    RE_VERIFY_OFFSET(ZBaseGeom, m_lControl, 0x3C);
    RE_VERIFY_OFFSET(ZBaseGeom, m_vSize, 0x40);
    RE_VERIFY_OFFSET(ZBaseGeom, m_fRadius, 0x4C);
    RE_VERIFY_OFFSET(ZBaseGeom, m_pParent, 0x50);
    RE_VERIFY_OFFSET(ZBaseGeom, m_lDrawId, 0x58);
    RE_VERIFY_OFFSET(ZBaseGeom, m_iPrev, 0x5C);
    RE_VERIFY_OFFSET(ZBaseGeom, m_iNext, 0x5E);
    RE_VERIFY_OFFSET(ZBaseGeom, m_pExtraGeom, 0x60);
    RE_VERIFY_OFFSET(ZBaseGeom, m_pDynId, 0x64);
    RE_VERIFY_OFFSET(ZBaseGeom, m_lPrim, 0x68);
    RE_VERIFY_OFFSET(ZBaseGeom, m_Name, 0x6C);
}
