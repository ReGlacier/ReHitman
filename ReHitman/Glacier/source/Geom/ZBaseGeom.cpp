#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZGeomListTypeUtils.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Geom/ZSHAPE.h>
#include <Glacier/Geom/ZBOUND.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZEngineGeomControl.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRender.h>
#include <cstring>


namespace Glacier
{
    ZBaseGeom* ZBaseGeom::RefToPtr(ZREF rRef)
    {
        return ZGeomBuffer::Instance().GeomRefToBasePtr(rRef);
    }

    ZBaseGeom* ZBaseGeom::GetBaseGeomPtrFromOffset(uint32_t lOffset)
    {
        return lOffset ? ZGeomBuffer::Instance().BaseGeomBufferPtr() + lOffset - 1 : nullptr;
    }

    uint32_t ZBaseGeom::GetBaseGeomOffsetFromPtr(ZBaseGeom* pBaseGeom)
    {
        return static_cast<uint32_t>(pBaseGeom - ZGeomBuffer::Instance().BaseGeomBufferPtr() + 1);
    }

    void* ZBaseGeom::operator new(std::size_t size)
    {
        ZASSERT(size == sizeof(ZBaseGeom));

        ZBaseGeom* pBaseGeom = ZGeomBuffer::Instance().AllocBaseGeom();
        ZASSERT(pBaseGeom);

        return pBaseGeom;
    }

    void* ZBaseGeom::operator new(std::size_t size, ZREF ref)
    {
        ZASSERT(size == sizeof(ZBaseGeom));

        ZBaseGeom* pBaseGeom = ZGeomBuffer::Instance().AllocBaseGeomDirect(ref);
        ZASSERT(pBaseGeom);

        return pBaseGeom;
    }

    void* ZBaseGeom::operator new(std::size_t size, ZREF ref, const char* file, uint32_t line)
    {
        std::ignore = file;
        std::ignore = line;

        return operator new(size, ref);
    }

    void ZBaseGeom::operator delete(void* ptr) noexcept
    {
        std::ignore = ptr;
    }

    void ZBaseGeom::operator delete(void* ptr, ZREF ref) noexcept
    {
        std::ignore = ptr;
        std::ignore = ref;
    }

    void ZBaseGeom::operator delete(void* ptr, ZREF ref, const char* file, uint32_t line) noexcept
    {
        std::ignore = ptr;
        std::ignore = ref;
        std::ignore = file;
        std::ignore = line;
    }

    ZBaseGeom::ZBaseGeom()
    {
        std::memset(this, 0, sizeof(ZBaseGeom)); // LOL
        m_mMat.Reset();

        m_lControl |= 0x1000000; // TODO: Find this mask

        if (g_pEngineData->RunTime())
        {
            m_lControl |= ZCBOUNDSDIRTY;
        }
        else
        {
            SetControl(ZCNONRUNTIME, 0u);
        }
    }

    ZBaseGeom::~ZBaseGeom()
    {
        if (ListId())
        {
            if (g_pEngineData)
            {
                auto* pLightList = g_pEngineData->GetListUser();
                if (pLightList)
                {
                    pLightList->DisconnectFromAllMembers(this);
                }
            }
        }

        SetControl(0u, ZCCHKLIGHT);

        uint32_t lGeomType = 0x200002; // HBM specific

        if (m_pExtraGeom)
        {
            lGeomType = m_pExtraGeom->GetGeomType();

            if (m_lControl >= 0)
            {
                SetControl(0x80000000u, 0u); // TODO: Find this mask
                m_pExtraGeom->Delete();
            }
        }
        else
        {
            if (m_lControl & ZCDYNAMIC)
            {
                DetachFromDynamicContainer(nullptr);
                FreeRoomList();
            }

            if (ZGROUP* pParentGroup = ParentGroup())
            {
                pParentGroup->DetachGeom(this, true);
            }
        }

        if (Control() & ZCHASDYNAMICPARENT)
        {
            assert(GetDynamicParent());
        }
        else if (GetRoomListPtr())
        {
            assert(!GetRoomListPtr()||!GetRoomListPtr()->Count());
        }

        if (Control() & ZCDYNAMIC)
        {
            FreeRoomList();
        }

        assert(!m_pDynId); // from PS2 build
        ZGeomBuffer::SetDeleteGeomType(lGeomType);
        ZGeomBuffer::Instance().FreeBaseGeom(this);
        m_pExtraGeom = nullptr;

        g_pEngineData->DeleteCheck((void*)m_Name);
    }

    void ZBaseGeom::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        if (bSaving)
        {
            Control();
        }

        stream.ExchangeArray("Matrix", m_mMat.data, 9);
        stream.ExchangeArray("Position", &m_vPos.x, 3);
        stream.ExchangeArray("Center", &m_vCen.x, 3);
        stream.ExchangeArray("Size", &m_vSize.x, 3);
        stream.Exchange("Radius", m_fRadius);
        stream.Exchange("Prim", m_lPrim);

        uint32_t lListID = m_uListID;
        uint32_t lPotentialLightListChange = m_lPotentialLightListChange;
        uint32_t lFreezeLightList = m_bFreezeLightList;

        if (bSaving)
        {
            auto* pListUser = g_pEngineData->GetListUser();
            ZASSERT(pListUser);
            if (pListUser->IsRuntimeMember(lListID))
            {
                lListID = 0xFFFFFFFFu;
            }
        }

        stream.Exchange("ListID", lListID);
        stream.Exchange("PotentialLightListChange", lPotentialLightListChange);
        stream.Exchange("FreezeLightList", lFreezeLightList);

        if (!bSaving)
        {
            if (lListID != 0xFFFFFFFFu)
            {
                m_uListID = lListID & 0xFFFFFFu;
            }

            m_lPotentialLightListChange = lPotentialLightListChange & 0x7Fu;
            m_bFreezeLightList = lFreezeLightList & 1u;
        }
    }

    const char* ZBaseGeom::Name() const
    {
        return m_Name ? m_Name : "<NONAME>";
    }

    bool ZBaseGeom::DoInit()
    {
        return !m_pExtraGeom || m_pExtraGeom->DoInit();
    }

    ZGROUP* ZBaseGeom::ParentGroup() const
    {
        ZBaseGeom* pParent = Parent();
        if (pParent)
        {
            ZASSERT(pParent->GetGeom());
            ZASSERT(pParent->IsDerivedFrom<ZGROUP>());

            return reinterpret_cast<ZGROUP*>(pParent->GetGeom());
        }

        return nullptr;
    }

    ZROOM* ZBaseGeom::GetOwnerRoom() const
    {
        if (Control() & (ZCHASDYNAMICPARENT | ZCDYNAMIC))
        {
            ZBaseGeomRoomList* pRoomList = nullptr;

            if (Control() & ZCHASDYNAMICPARENT)
            {
                if (auto* pDynamicParent = GetDynamicParent())
                {
                    pRoomList = pDynamicParent->GetRoomList();
                }
            }
            else
            {
                pRoomList = GetRoomList();
            }

            if (pRoomList && pRoomList->Count())
            {
                return pRoomList->GetRoomNr(0);
            }

            return g_pEngineData->m_pRoot;
        }

        for (auto* pBaseGeom = const_cast<ZBaseGeom*>(this); pBaseGeom; pBaseGeom = pBaseGeom->Parent())
        {
            if (pBaseGeom->IsDerivedFrom<ZROOM>())
            {
                return static_cast<ZROOM*>(pBaseGeom->GetGeom());
            }
        }

        return nullptr;
    }

    void ZBaseGeom::SetName(const char* name)
    {
        g_pEngineData->DeleteCheck((void*)m_Name);

        if (g_pEngineData->CheckInPackBuffer((void*)name))
        {
            m_Name = name;
        }
        else
        {
            m_Name = (const char*)ZUniMemory::Allocate(sizeof(char) * strlen(name) + 1);
            std::strcpy(const_cast<char*>(m_Name), name);
        }
    }

    bool ZBaseGeom::SetPrim(int primId)
    {
        const auto lOldPrim = m_lPrim;
        m_lPrim = primId;

        if (Prim() && DrawId() && !IsDerivedFrom<ZLIGHT>())
        {
            g_pSysInterface->WindowFirst->ChangePrim(this, primId);
        }

        if (lOldPrim != m_lPrim)
        {
            SetControlDirect(ZCBOUNDSDIRTY, 0u);
        }

        return lOldPrim != m_lPrim;
    }

    void ZBaseGeom::GetMatPos(Glacier::ZMat3x3& mat, Glacier::ZVector3& pos)
    {
        mat = m_mMat;
        pos = m_vPos;
    }

    uint32_t ZBaseGeom::DrawId() const
    {
        return m_lDrawId;
    }

    uint32_t ZBaseGeom::ListId() const
    {
        return m_uListID & 0xFFFFFFu;
    }

    ZBaseGeom* ZBaseGeom::Next()
    {
        return m_iNext ? ZGeomBuffer::Instance().BaseGeomBufferPtr() + m_iNext - 1 : nullptr;
    }

    void ZBaseGeom::SetNext(ZBaseGeom* next)
    {
        m_iNext = next ? static_cast<uint16_t>(next - ZGeomBuffer::Instance().BaseGeomBufferPtr() + 1) : 0;
    }

    ZBaseGeom* ZBaseGeom::GetPrev()
    {
        return m_iPrev ? ZGeomBuffer::Instance().BaseGeomBufferPtr() + m_iPrev - 1 : nullptr;
    }

    void ZBaseGeom::SetPrev(ZBaseGeom* prev)
    {
        m_iPrev = prev ? static_cast<uint16_t>(prev - ZGeomBuffer::Instance().BaseGeomBufferPtr() + 1) : 0;
    }

    template <typename TGeom>
    bool ZBaseGeom::IsDerivedFrom() const
    {
        if (!m_pExtraGeom)
        {
            return TGeom::m_Id == ZSTDOBJ::m_Id;
        }

        return TGeom::m_Id == (m_pExtraGeom->GetObjectId() & TGeom::m_Mask);
    }

    uint32_t ZBaseGeom::ColiId() const
    {
        return 0u;
    }

    uint32_t ZBaseGeom::GetColiBits() const
    {
        return Control();
    }

    void ZBaseGeom::SetColiId(uint32_t lColi)
    {
        // Weird, I guess legacy stuff from H2SA
    }

    ZBaseGeom* ZBaseGeom::Parent() const
    {
        return m_pParent;
    }

    void ZBaseGeom::SetParent(ZBaseGeom* pBaseGeom)
    {
        ZASSERT(pBaseGeom && pBaseGeom->IsDerivedFrom<ZGROUP>());
        m_pParent = pBaseGeom;
    }

    void ZBaseGeom::SetMat(const float* m0)
    {
        if (std::memcmp(&m0[0], Mat(), sizeof(float) * 9))
        {
            SetMatSimple(m0);
            ZEngineGeomControl::GetInstance().GeomMoved(this);
        }
    }

    void ZBaseGeom::SetPos(const float* p)
    {
        if (std::memcmp(p, m_vPos.Get(), sizeof(float) * 3))
        {
            SetPosSimple(p);
            ZEngineGeomControl::GetInstance().GeomMoved(this);
        }
    }

    void ZBaseGeom::SetMatPos(const float* m, const float* p)
    {
        if (std::memcmp(m_vPos.Get(), p, sizeof(float) * 3) || std::memcmp(m_mMat.Get(), m, sizeof(float) * 9))
        {
            SetPosSimple(p);
            SetMatSimple(m);

            ZEngineGeomControl::GetInstance().GeomMoved(this);
        }
    }

    void ZBaseGeom::SetPosSimple(const float* p)
    {
        m_vPos = p;
        SetControlDirect(0x100000u, 0u);
    }

    void ZBaseGeom::SetMatSimple(const float* m0)
    {
        m_mMat = m0;
        SetControlDirect(0x100000u, 0u);
    }

    void ZBaseGeom::SetRootPos(const float* p)
    {
        auto* parent = Parent();
        if (!parent)
        {
            return;
        }

        ZVector3 localPos(p);
        parent->GetLocalPoint(localPos);
        SetPos(localPos.Get());
    }

    void ZBaseGeom::GetRootTM(ZMat3x3& mat, ZVector3& pos) const
    {
        mat.Reset();
        pos.Reset();

        GetRootMat(mat);
        GetRootPoint(pos);
    }

    void ZBaseGeom::SetRootTM(const ZMat3x3& mat, const ZVector3& pos)
    {
        auto* parent = Parent();
        if (!parent)
        {
            return;
        }

        ZMat3x3 localMat = mat;
        ZVector3 localPos = pos;

        parent->GetLocalMatPos(localMat, localPos);
        SetMatPos(localMat.Get(), localPos.Get());
    }

    void ZBaseGeom::GetRootPoint(ZVector3& point) const
    {
        for (const ZBaseGeom* geom = this; geom; geom = geom->Parent())
        {
            TransformRootVector(point, geom->m_mMat);

            point += geom->m_vPos;
        }
    }

    void ZBaseGeom::GetRootMat(ZMat3x3& mat) const
    {
        TransformRootVector(mat.XAxis(), m_mMat);
        if (m_pParent)
        {
            m_pParent->GetRootVect(mat.XAxis());
        }

        TransformRootVector(mat.YAxis(), m_mMat);
        if (m_pParent)
        {
            m_pParent->GetRootVect(mat.YAxis());
        }

        TransformRootVector(mat.ZAxis(), m_mMat);
        if (m_pParent)
        {
            m_pParent->GetRootVect(mat.ZAxis());
        }
    }

    void ZBaseGeom::GetRootVect(ZVector3& vec) const
    {
        for (const auto* geom = this; geom; geom = geom->Parent())
        {
            TransformRootVector(vec, geom->m_mMat);
        }
    }

    void ZBaseGeom::GetPos(ZVector3& pos) const
    {
        pos = m_vPos;
    }

    void ZBaseGeom::GetCen(ZVector3& cen) const
    {
        cen = m_vCen;
    }

    void ZBaseGeom::GetSize(ZVector3& size) const
    {
        size = m_vSize;
    }

    void ZBaseGeom::GetRootMatPos(ZMat3x3& mat, ZVector3& pos) const
    {
        auto* parent = Parent();
        if (!parent)
        {
            return;
        }

        parent->GetRootPoint(pos);
        parent->GetRootVect(mat.XAxis());
        parent->GetRootVect(mat.YAxis());
        parent->GetRootVect(mat.ZAxis());
    }

    void ZBaseGeom::GetLocalMat(ZMat3x3& mat)
    {
        ZMat3x3 mLocalMat;
        mLocalMat.Reset();

        GetRootMat(mLocalMat);
        mat *= mLocalMat;
    }

    void ZBaseGeom::GetLocalMatPos(ZMat3x3& mat, ZVector3& pos) const
    {
        ZMat3x3 rootMat;
        ZVector3 rootPos;

        GetRootTM(rootMat, rootPos);

        pos.x -= rootPos.x;
        pos.y -= rootPos.y;
        pos.z -= rootPos.z;

        TransformLocalVector(pos, rootMat);
        TransformLocalMatrix(mat, rootMat);
    }

    void ZBaseGeom::GetLocalPoint(ZVector3& point) const
    {
        if (!m_pParent) return;

        m_pParent->GetLocalPoint(point);
        point -= m_vPos;
        TransformLocalVector(point, m_mMat);
    }

    void ZBaseGeom::SetCen(const ZVector3& vCenter)
    {
        SetCenSimple(vCenter);
    }

    void ZBaseGeom::SetCen(float fX, float fY, float fZ)
    {
        ZVector3 vCenter { fX, fY, fZ };
        SetCenSimple(vCenter);
    }

    void ZBaseGeom::SetCenSimple(const ZVector3& vCenter)
    {
        m_vCen = vCenter;
        SetControlDirect(ZCBOUNDSDIRTY, 0u);
    }

    void ZBaseGeom::SetSize(const ZVector3& vSize)
    {
        SetSizeSimple(vSize);
    }

    void ZBaseGeom::SetSizeSimple(const ZVector3& vSize)
    {
        m_vSize = vSize;
        m_vSize.x = std::max(m_vSize.x, MIN_GEOM_SIZE);
        m_vSize.y = std::max(m_vSize.y, MIN_GEOM_SIZE);
        m_vSize.z = std::max(m_vSize.z, MIN_GEOM_SIZE);

        SetControlDirect(ZCBOUNDSDIRTY, 0u);
    }

    void ZBaseGeom::SetRadius(float fRadius)
    {
        SetRadiusSimple(fRadius);
    }

    void ZBaseGeom::SetRadiusSimple(float fRadius)
    {
        m_fRadius = fRadius;
        SetControlDirect(ZCBOUNDSDIRTY, 0u);
    }

    const float* ZBaseGeom::Mat() const
    {
        return m_mMat.Get();
    }

    const float* ZBaseGeom::Pos() const
    {
        return m_vPos.Get();
    }

    const float* ZBaseGeom::Cen() const
    {
        return m_vCen.Get();
    }

    const float* ZBaseGeom::Size() const
    {
        return m_vSize.Get();
    }

    uint32_t ZBaseGeom::GetRef() const
    {
        return ZGeomBuffer::Instance().GeomPtrToRef(this);
    }

    float ZBaseGeom::Radius() const
    {
        return m_fRadius;
    }

    void ZBaseGeom::SetControlDirect(uint32_t lAddBits, uint32_t lRemBits)
    {
        m_lControl &= ~lRemBits;
        m_lControl |= lAddBits;
    }

    void ZBaseGeom::SetControl(uint32_t lAddBits, uint32_t lRemBits)
    {
        ZASSERT((lAddBits & lRemBits) == 0);

        const uint32_t lColiBits = lAddBits & ZCCOLIMASK;
        if (lColiBits)
        {
            for (auto* pParent = Parent(); pParent; pParent = pParent->Parent())
            {
                pParent->m_lControl |= lColiBits;
            }
        }

        uint32_t lRealAddBits = lAddBits & ~m_lControl;
        uint32_t lRealRemBits = lRemBits & m_lControl;

        constexpr uint32_t ZCUPDATELIGHT = 0x1000000u; // TODO: Find this mask
        constexpr uint32_t kControlChangeMask = ZCOWNERDRAW | ZCDYNAMIC | ZCCHKLIGHT | ZCROOMASSIGN | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE;
        constexpr uint32_t kRoomsDrawListMask = ZCOWNERDRAW | ZCDYNAMIC | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE;
        constexpr uint32_t kDynamicContainerMask = ZCOWNERDRAW | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE;
        constexpr uint32_t kRoomAssignMask = ZCOWNERDRAW | ZCROOMASSIGN | ZCHIDDEN | ZCINACTIVE;
        constexpr uint32_t kRoomListMask = ZCOWNERDRAW | ZCHIDDEN | ZCINACTIVE;
        constexpr uint32_t kRecursiveGroupMask = ZCOWNERDRAW | ZCINACTIVE;

        if ((lRealAddBits | lRealRemBits) & kControlChangeMask)
        {
            if ((lRealAddBits & ZCDYNAMIC) && (Control() & ZCHASDYNAMICPARENT))
            {
                const MYSTR sName = CalcTotalName(true);
                const MYSTR sParentName = GetDynamicParent() ? GetDynamicParent()->CalcTotalName(true) : MYSTR("<none>");
                printf("ERROR: %s has dynamic parent %s", sName.String, sParentName.String);
                lRealAddBits &= ~ZCDYNAMIC;
            }

            if ((lRealRemBits & ZCINACTIVE) && Parent() && (Parent()->Control() & ZCINACTIVE))
            {
                if (Name() && Parent()->Name())
                {
                    printf("ERROR: Can not activate geom %s when parent %s is inactive", Name(), Parent()->Name());
                }

                lRealRemBits &= ~ZCINACTIVE;
            }

            if (lRealRemBits & ZCDYNAMIC)
            {
                lRealRemBits |= ZCROOMASSIGN;
            }

            const uint32_t lOldControl = m_lControl;
            const uint32_t lNewControl = (~lRealRemBits & m_lControl) | lRealAddBits;

            const auto CheckBitfieldChanges = [](uint32_t lOldValue, uint32_t lNewValue, uint32_t lRequiredBits, uint32_t lForbiddenBits) -> int32_t
            {
                const bool bOldEnabled = ((lOldValue & lRequiredBits) == lRequiredBits) && ((lOldValue & lForbiddenBits) == 0);
                const bool bNewEnabled = ((lNewValue & lRequiredBits) == lRequiredBits) && ((lNewValue & lForbiddenBits) == 0);

                if (bOldEnabled == bNewEnabled)
                {
                    return 0;
                }

                return bNewEnabled ? 1 : -1;
            };

            const int32_t lRoomsDrawListChange = CheckBitfieldChanges(lOldControl, lNewControl, 0u, kRoomsDrawListMask);
            const int32_t lDynamicContainerChange = CheckBitfieldChanges(lOldControl, lNewControl, ZCDYNAMIC, kDynamicContainerMask);
            const int32_t lDynamicRoomListChange = CheckBitfieldChanges(lOldControl, lNewControl, ZCDYNAMIC, kRoomListMask);
            const int32_t lStaticRoomListChange = CheckBitfieldChanges(lOldControl, lNewControl, ZCDYNAMIC, kRoomAssignMask);
            const int32_t lAutoRoomAssignChange = CheckBitfieldChanges(lOldControl, lNewControl, ZCDYNAMIC | ZCROOMASSIGN, kRoomListMask);

            if (lDynamicRoomListChange < 0)
            {
                DetachFromDynamicContainer(nullptr);
            }

            if (lStaticRoomListChange < 0 || lAutoRoomAssignChange < 0)
            {
                FreeRoomList();
            }

            if (lRoomsDrawListChange < 0 || lDynamicContainerChange < 0)
            {
                DetachFromRoomsDrawLists(nullptr);
            }

            m_lControl &= ~lRealRemBits;
            m_lControl |= lRealAddBits;

            // TODO: Finish after ZEngineDataBase/CListUser are reversed.
            // Original runtime flow registers/unregisters lit ZSTDOBJ/ZLIGHT geoms in CListUser
            // when ZCCHKLIGHT changes, updates m_uListID, and marks ZCUPDATELIGHT.
            if (lRealAddBits & ZCCHKLIGHT)
            {
                ZASSERT((m_lControl & ZCNONRUNTIME) == 0);
            }

            if ((lRealAddBits | lRealRemBits) & ZCCHKLIGHT)
            {
                m_lControl |= ZCUPDATELIGHT;
            }

            if (lRealAddBits & ZCDYNAMIC)
            {
                SetDynamicParent(this);
                if (auto* pParentGroup = ParentGroup())
                {
                    pParentGroup->InvalidateBounds();
                }
            }
            else if (lRealRemBits & ZCDYNAMIC)
            {
                if (IsDerivedFrom<ZGROUP>())
                {
                    auto* pGroup = static_cast<ZGROUP*>(GetGeom());
                    for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                    {
                        pChild->SetControl(0u, ZCHASDYNAMICPARENT);
                        pChild->m_iDynamicParentNr = 0;
                    }
                }
            }

            if (lRoomsDrawListChange > 0 || lDynamicContainerChange > 0)
            {
                AttachToRoomsDrawLists(nullptr);
            }

            if (lDynamicRoomListChange > 0)
            {
                ZASSERT(!m_pDynId);
                AttachToDynamicContainer();
            }

            if (lAutoRoomAssignChange > 0)
            {
                AutoAssignToRooms();
            }
            else if (lStaticRoomListChange > 0)
            {
                ZBaseGeom* pRoomBaseGeom = Parent();
                for (; pRoomBaseGeom; pRoomBaseGeom = pRoomBaseGeom->Parent())
                {
                    if (pRoomBaseGeom->IsDerivedFrom<ZROOM>())
                    {
                        break;
                    }
                }

                if (pRoomBaseGeom)
                {
                    AddToRoomList(static_cast<ZROOM*>(pRoomBaseGeom->GetGeom()));
                }
                else
                {
                    const MYSTR sName = CalcTotalName(true);
                    printf("WARNING: Geom %s will never be drawn as it is not attached under a room!", sName.String);
                }
            }

            if (lRealAddBits & ZCINACTIVE)
            {
                // TODO: Notify CListUser after ZEngineDataBase/CListUser are reversed.
                // Original removes this geom from ListUser when m_uListID is set.
                if (auto* pGeom = GetGeom())
                {
                    pGeom->Activate(false);
                }
            }
            else if ((lRealRemBits & ZCINACTIVE) && GetGeom())
            {
                GetGeom()->Activate(true);
            }

            const uint32_t lRecursiveAddBits = lRealAddBits & kRecursiveGroupMask;
            const uint32_t lRecursiveRemBits = lRealRemBits & kRecursiveGroupMask;
            if ((lRecursiveAddBits | lRecursiveRemBits) && IsDerivedFrom<ZGROUP>())
            {
                auto* pGroup = static_cast<ZGROUP*>(GetGeom());
                for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                {
                    pChild->SetControl(lRecursiveAddBits, lRecursiveRemBits);
                }
            }

            if ((lRealAddBits | lRealRemBits) & ZCOWNERDRAW)
            {
                SetPrim(Prim());
            }
        }
        else
        {
            m_lControl &= ~lRealRemBits;
            m_lControl |= lRealAddBits;
        }
    }

    void ZBaseGeom::SetControl(uint32_t lControl)
    {
        const uint32_t lOldControl = Control();
        SetControl(~lOldControl & lControl, ~lControl & lOldControl);
    }

    uint32_t ZBaseGeom::Control() const
    {
        return m_lControl;
    }

    uint32_t ZBaseGeom::GeomControl() const
    {
        if (auto* pGeom = GetGeom())
        {
            return pGeom->GeomControl();
        }

        return 0u;
    }

    uint32_t ZBaseGeom::Prim() const
    {
        return m_lPrim;
    }

    void ZBaseGeom::UpdateMovedGeom()
    {
        if ((Control() & ZCOWNERDRAW) == 0)
        {
            if ((Control() & ZCDYNAMIC) == 0)
            {
                DynamicPosChanged();
            }
            else
            {
                // TODO: Finish me
                // if (!ZEngineDataBase::MinMaxLocked)
                //{
                //    ZEngineDataBase::AdjustMinMax(false);
                //}
            }
        }
    }

    void ZBaseGeom::DynamicPosChanged()
    {
        ZASSERT(!(Control() & ZCOWNERDRAW));

        if (Control())
        {
            auto* pDynamicTreeGroup = ParentGroup()->GetDynamicTreeGroup();

            if (pDynamicTreeGroup)
            {
                pDynamicTreeGroup->MoveDynamicGeom(this);
            }
        }

        if (Control() & ZCROOMASSIGN)
        {
            AutoAssignToRooms();
        }
    }

    ZGROUP* ZBaseGeom::GetOwner(bool bCheckWorldGroup) const
    {
        if (Control() & ZCDYNAMIC)
        {
            printf("WARNING: ZBaseGeom::GetOwner() called on dynamic object\n");
            return g_pEngineData->m_pRoot;
        }
        else if (Parent())
        {
            for (auto* pParentGroup = ParentGroup(); pParentGroup; pParentGroup = pParentGroup->BaseGeom()->ParentGroup())
            {
                if (pParentGroup->IsDerivedFrom<ZROOM>())
                {
                    return pParentGroup;
                }

                if (bCheckWorldGroup && (pParentGroup->m_lGroupCon & 0x4000000))
                {
                    return pParentGroup;
                }
            }
        }
        else if (IsDerivedFrom<ZGROUP>())
        {
            return static_cast<ZGROUP*>(GetGeom());
        }

        return nullptr;
    }

    ZBaseGeom* ZBaseGeom::GetDynamicParent() const
    {
        if (Control() & ZCHASDYNAMICPARENT)
        {
            return GetDynamicParentPtr();
        }

        return nullptr;
    }

    ZBaseGeom* ZBaseGeom::GetDynamicParentPtr() const
    {
        ZASSERT(Control() & ZCHASDYNAMICPARENT);

        if (m_iDynamicParentNr)
        {
            return ZGeomBuffer::Instance().BaseGeomBufferPtr() + m_iDynamicParentNr - 1;
        }

        return nullptr;
    }

    ZBaseGeomRoomList* ZBaseGeom::GetRoomList() const
    {
        ZASSERT(Control() & ZCDYNAMIC);
        ZASSERT(!(Control() & ZCHASDYNAMICPARENT));

        return GetRoomListPtr();
    }

    ZBaseGeomRoomList* ZBaseGeom::GetRoomListPtr() const
    {
        ZASSERT(!(Control() & ZCHASDYNAMICPARENT));

        if (m_iRoomListNr)
        {
            return ZGeomBuffer::Instance().GetRoomList(m_iRoomListNr);
        }

        return nullptr;
    }

    ZTreeGroup* ZBaseGeom::GetTreeGroup() const
    {
        ZGROUP* pGroup = nullptr;

        if (IsDerivedFrom<ZGROUP>())
        {
            pGroup = static_cast<ZGROUP*>(GetGeom());
        }
        else
        {
            pGroup = ParentGroup();
        }

        while (pGroup)
        {
            if (pGroup->IsDerivedFrom<ZTreeGroup>())
            {
                return static_cast<ZTreeGroup*>(pGroup);
            }

            pGroup = pGroup->Parent();
        }

        return nullptr;
    }

    ZTreeGroup* ZBaseGeom::GetDynamicTreeGroup() const
    {
        ZGEOM* geom = nullptr;

        if (IsDerivedFrom<ZGROUP>())
        {
            geom = m_pExtraGeom;
        }
        else if (m_pParent)
        {
            geom = m_pParent->m_pExtraGeom;
        }

        for (; geom; geom = geom->BaseGeom()->Parent()->GetGeom())
        {
            if (!geom->IsDerivedFrom<ZTreeGroup>())
            {
                continue;
            }

            auto* treeGroup = static_cast<ZTreeGroup*>(geom);
            if (treeGroup->IsDynamicContainer() || treeGroup->IsPrivate())
            {
                return treeGroup;
            }
        }

        return nullptr;
    }

    eGlobalTreeType ZBaseGeom::GetBoundTreeType() const
    {
        if (auto* pGeom = GetGeom())
        {
            return pGeom->GetBoundTreeType();
        }

        return eGlobalTreeType::GT_StdObjs;
    }

    void ZBaseGeom::FreeRoomList()
    {
        if (auto* pRoomList = GetRoomList())
        {
            while (pRoomList->Count())
            {
                auto iRoomNr = pRoomList->GetRoomNr(0);
                RemoveFromRoomList(iRoomNr);
            }

            ZGeomBuffer::Instance().FreeRoomList(pRoomList);
            SetRoomList(nullptr);
        }
    }

    void ZBaseGeom::SetRoomList(ZBaseGeomRoomList* pRoomList)
    {
        ZASSERT(!(Control() & ZCHASDYNAMICPARENT));
        m_iRoomListNr = pRoomList ? ZGeomBuffer::Instance().GetRoomListNr(pRoomList) : 0u;
    }

    void ZBaseGeom::RemoveFromRoomList(ZROOM* pRoom)
    {
        if (auto* pRoomList = GetRoomListPtr())
        {
            if (pRoomList->Exists(pRoom))
            {
                pRoomList->Remove(pRoom);
            }
        }

        pRoom->RemoveDynamicGeomFromRoom(this);
        DetachFromRoomsDrawLists(pRoom);
    }

    void ZBaseGeom::AddToRoomList(ZROOM* pRoom)
    {
        if ((Control() & (ZCOWNERDRAW|0xC00u)) == 0) // TODO: Find mask
        {
            if (!GetRoomListPtr())
            {
                auto* pRoomList = ZGeomBuffer::Instance().AllocRoomList();
                SetRoomList(pRoomList);
            }

            auto* pRoomList = GetRoomListPtr();
            if (pRoomList->Add(pRoom))
            {
                pRoom->AddDynamicGeomToRoom(this);
                AttachToRoomsDrawLists(pRoom);
            }
        }
    }

    void ZBaseGeom::AttachToRoomsDrawLists(ZROOM* pRoom)
    {
        if (Control() & (ZCOWNERDRAW | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE))
        {
            return;
        }

        const auto eListType = GetBaseGeomListType(this);
        if (eListType == eBaseGeomListTypes::BGLT_Light)
        {
            if (Control() & (ZCHASDYNAMICPARENT | ZCDYNAMIC))
            {
                if (pRoom)
                {
                    if (!ZGeomBuffer::Instance().Exists(pRoom->m_lDynamicGeomsDrawList, this))
                    {
                        pRoom->m_lDynamicGeomsDrawList = ZGeomBuffer::Instance().AddGeoms(pRoom->m_lDynamicGeomsDrawList, this, this);
                    }
                }
                else
                {
                    auto* pDynamicParent = this;
                    if (Control() & ZCHASDYNAMICPARENT)
                    {
                        pDynamicParent = GetDynamicParent();
                    }

                    if (auto* pRoomList = pDynamicParent ? pDynamicParent->GetRoomListPtr() : nullptr)
                    {
                        const uint32_t lRoomCount = pRoomList->Count();
                        for (uint32_t i = 0; i < lRoomCount; ++i)
                        {
                            auto* pListRoom = pRoomList->GetRoomNr(i);
                            if (!ZGeomBuffer::Instance().Exists(pListRoom->m_lDynamicGeomsDrawList, this))
                            {
                                pListRoom->m_lDynamicGeomsDrawList = ZGeomBuffer::Instance().AddGeoms(pListRoom->m_lDynamicGeomsDrawList, this, this);
                            }
                        }
                    }
                }
            }
            else
            {
                ZBaseGeom* pRoomBaseGeom = Parent();
                for (; pRoomBaseGeom; pRoomBaseGeom = pRoomBaseGeom->Parent())
                {
                    if (pRoomBaseGeom->IsDerivedFrom<ZROOM>())
                    {
                        break;
                    }
                }

                if (pRoomBaseGeom)
                {
                    auto* pOwnerRoom = static_cast<ZROOM*>(pRoomBaseGeom->GetGeom());
                    uint32_t* pDrawList = RequestCustomDraw() ? &pOwnerRoom->m_lStaticGeomsCustomDrawList : &pOwnerRoom->m_lStaticGeomsPrimDrawList;

                    if (!ZGeomBuffer::Instance().Exists(*pDrawList, this))
                    {
                        *pDrawList = ZGeomBuffer::Instance().AddGeoms(*pDrawList, this, this);
                    }

                    g_pSysInterface->WindowFirst->UpdateBaseGeom(this);
                }
            }
        }
        else if (eListType == eBaseGeomListTypes::BGLT_Group)
        {
            auto* pGroup = static_cast<ZGROUP*>(GetGeom());
            for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
            {
                if (pChild->IsDerivedFrom<ZROOM>())
                {
                    continue;
                }

                if (pChild->IsDerivedFrom<ZTreeGroup>())
                {
                    auto* pTreeGroup = static_cast<ZTreeGroup*>(pChild->GetGeom());
                    if (pTreeGroup->IsPrivate())
                    {
                        continue;
                    }
                }

                pChild->AttachToRoomsDrawLists(pRoom);
            }
        }
    }

    void ZBaseGeom::DetachFromRoomsDrawLists(ZROOM* pRoom)
    {
        if (Control() & ZCOWNERDRAW)
        {
            return;
        }

        const auto eListType = GetBaseGeomListType(this);
        if (eListType == eBaseGeomListTypes::BGLT_Light)
        {
            if (Control() & (ZCHASDYNAMICPARENT | ZCDYNAMIC))
            {
                if (pRoom)
                {
                    if (ZGeomBuffer::Instance().Exists(pRoom->m_lDynamicGeomsDrawList, this))
                    {
                        ZASSERT((Control() & (ZCOWNERDRAW | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE)) == 0);
                        pRoom->m_lDynamicGeomsDrawList = ZGeomBuffer::Instance().RemoveGeoms(pRoom->m_lDynamicGeomsDrawList, this, this);
                    }
                }
                else
                {
                    auto* pDynamicParent = this;
                    if (Control() & ZCHASDYNAMICPARENT)
                    {
                        pDynamicParent = GetDynamicParent();
                    }

                    if (auto* pRoomList = pDynamicParent ? pDynamicParent->GetRoomListPtr() : nullptr)
                    {
                        const uint32_t lRoomCount = pRoomList->Count();
                        for (uint32_t i = 0; i < lRoomCount; ++i)
                        {
                            auto* pListRoom = pRoomList->GetRoomNr(i);
                            if (ZGeomBuffer::Instance().Exists(pListRoom->m_lDynamicGeomsDrawList, this))
                            {
                                ZASSERT((Control() & (ZCOWNERDRAW | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE)) == 0);
                                pListRoom->m_lDynamicGeomsDrawList = ZGeomBuffer::Instance().RemoveGeoms(pListRoom->m_lDynamicGeomsDrawList, this, this);
                            }
                        }
                    }
                }
            }
            else
            {
                ZBaseGeom* pRoomBaseGeom = Parent();
                for (; pRoomBaseGeom; pRoomBaseGeom = pRoomBaseGeom->Parent())
                {
                    if (pRoomBaseGeom->IsDerivedFrom<ZROOM>())
                    {
                        break;
                    }
                }

                if (pRoomBaseGeom)
                {
                    auto* pOwnerRoom = static_cast<ZROOM*>(pRoomBaseGeom->GetGeom());
                    uint32_t* pDrawList = RequestCustomDraw() ? &pOwnerRoom->m_lStaticGeomsCustomDrawList : &pOwnerRoom->m_lStaticGeomsPrimDrawList;

                    if (ZGeomBuffer::Instance().Exists(*pDrawList, this))
                    {
                        ZASSERT((Control() & (ZCOWNERDRAW | ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE)) == 0);
                        *pDrawList = ZGeomBuffer::Instance().RemoveGeoms(*pDrawList, this, this);
                    }

                    g_pSysInterface->WindowFirst->UpdateBaseGeom(this);
                }
            }
        }
        else if (eListType == eBaseGeomListTypes::BGLT_Group)
        {
            auto* pGroup = static_cast<ZGROUP*>(GetGeom());
            for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
            {
                if (pChild->IsDerivedFrom<ZROOM>())
                {
                    continue;
                }

                if (pChild->IsDerivedFrom<ZTreeGroup>())
                {
                    auto* pTreeGroup = static_cast<ZTreeGroup*>(pChild->GetGeom());
                    if (pTreeGroup->IsPrivate())
                    {
                        continue;
                    }
                }

                pChild->DetachFromRoomsDrawLists(pRoom);
            }
        }
    }

    void ZBaseGeom::AttachToDynamicContainer()
    {
        ZASSERT(Active());
        ZASSERT(Control() & ZCDYNAMIC);
        ZASSERT(!m_pDynId);

        if (Control())
        {
            auto* pDynamicTreeGroup = ParentGroup()->GetDynamicTreeGroup();
            if (pDynamicTreeGroup)
            {
                m_pDynId = pDynamicTreeGroup->AddDynamicGeom(this);
            }
        }
    }

    void ZBaseGeom::DetachFromDynamicContainer(ZGROUP* pOldParent)
    {
        if (m_pDynId && !m_lControl)
        {
            if (!pOldParent)
            {
                if (m_pParent)
                {
                    pOldParent = reinterpret_cast<ZGROUP*>(m_pParent->m_pExtraGeom);
                }
            }

            auto* pDynTreeGroup = pOldParent->GetDynamicTreeGroup();
            if (pDynTreeGroup)
            {
                pDynTreeGroup->RemoveDynamicGeom(this);
                m_pDynId = nullptr;
            }
        }
    }

    bool ZBaseGeom::Active() const
    {
        // Idk, but this correct
        // IOI, why...
        return (Control() & ZCINACTIVE) == 0;
    }

    void ZBaseGeom::MakeInactive()
    {
        SetControl(ZCINACTIVE, 0u);
    }

    void ZBaseGeom::MakeActive()
    {
        SetControl(0u, ZCINACTIVE);
    }

    void ZBaseGeom::MakeDynamic(bool bDynamic)
    {
        if (bDynamic)
        {
            SetControl(ZCDYNAMIC, 0u);
        }
        else
        {
            SetControl(0u, ZCDYNAMIC);
        }
    }

    void ZBaseGeom::Hide(bool bHide)
    {
        if (bHide)
        {
            SetControl(ZCHIDDEN, 0u);
        }
        else
        {
            SetControl(0u, ZCHIDDEN);
        }
    }

    void ZBaseGeom::HideRecursive(bool bHide)
    {
        const uint32_t iAddBits = bHide ? ZCHIDDEN : 0u;
        const uint32_t iRemBits = bHide ? 0u : ZCHIDDEN;

        if (!IsDerivedFrom<ZGROUP>())
        {
            SetControl(iAddBits, iRemBits);
            return;
        }

        auto* groupGeom = static_cast<ZGROUP*>(m_pExtraGeom);

        for (auto* current = groupGeom->BaseGeom(); current; groupGeom->RecurGetNext(&current))
        {
            current->SetControl(iAddBits, iRemBits);
        }
    }

    void ZBaseGeom::Freeze(bool bFreeze)
    {
        if (bFreeze)
        {
            ZASSERT(GetGeom());

            GetGeom()->Freeze(bFreeze);
        }
        else if (auto* pGeom = GetGeom())
        {
            pGeom->Freeze(bFreeze);
        }
    }

    bool ZBaseGeom::ChkEvents() const
    {
        if (!m_pExtraGeom)
            return false;

        if (!m_pExtraGeom->m_pExData)
            return false;

        return m_pExtraGeom->m_pExData->_Events.ChkEvents();
    }

    bool ZBaseGeom::ChkUpdateMinMax() const
    {
        return !g_pEngineData->MinMaxLocked() && ((Control() & ZCDYNAMIC) == 0);
    }

    void ZBaseGeom::SendCommand(ZMSGID Msg, void* pData, ZGEOM* pTarget)
    {
        if (auto* pGeom = GetGeom())
        {
            // Yep, in same in game
            pGeom->SendCommandRecursive(Msg, pData, pTarget);
        }
    }

    void ZBaseGeom::SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget)
    {
        if (auto* pGeom = GetGeom())
        {
            pGeom->SendCommandRecursive(Msg, pData, pTarget);
        }
    }

    bool ZBaseGeom::CheckPointInside(ZVector3& vPoint, float fDotDist) const
    {
        return false;
    }

    void ZBaseGeom::CopyData(const ZBaseGeom* Source)
    {
        CopyGeometry(Source);
        SetColiId(Source->ColiId());
        SetControl(Source->m_lControl & 0x97FBA3FFu, ~Source->m_lControl & 0x97FBA3FFu);
        CopyMatPos(Source);
        CopyCenSizeRadius(Source);

        if (Source->Control() & ZCINACTIVE) { MakeInactive(); }
        if (Source->Control() & ZCHIDDEN) { Hide(true); }
        if (Source->Control() & ZCDYNAMIC) { MakeDynamic(true); }

        if (auto* parent = Parent(); parent && (parent->Control() & (ZCHASDYNAMICPARENT | ZCDYNAMIC)))
        {
            SetControl(ZCHASDYNAMICPARENT, 0u);

            auto* pDynamicParent = (parent->Control() & ZCDYNAMIC) ? parent : parent->GetDynamicParent();
            SetDynamicParentPtr(pDynamicParent);
        }

        if (Source->Control() & ZCROOMASSIGN)
        {
            SetAutoRoomAssign(true);
        }

        if (ZGEOM* pSelfGeom = GetGeom())
        {
            if (ZGEOM* pSourceGeom = Source->GetGeom())
            {
                pSelfGeom->CopyData(pSourceGeom);
            }
        }
    }

    void ZBaseGeom::CopyGeometry(const ZBaseGeom* Source)
    {
        SetPrim(Source->Prim());
    }

    void ZBaseGeom::CopyCenSizeRadius(const ZBaseGeom* Source)
    {
        m_vCen = Source->m_vCen;
        m_lControl |= ZCBOUNDSDIRTY;

        SetSizeSimple(Source->m_vSize);

        m_fRadius = Source->m_fRadius;
    }

    void ZBaseGeom::CopyMatPos(const ZBaseGeom* Source)
    {
        SetPosSimple(Source->Pos());
        SetMatSimple(Source->Mat());

        ZEngineGeomControl::GetInstance().GeomMoved(this);
    }

    void ZBaseGeom::SetDynamicParent(ZBaseGeom* pParent)
    {
        if (this != pParent)
        {
            if (Control() & ZCDYNAMIC)
            {
                const MYSTR sName = CalcTotalName(true);
                printf("%s is a dynamic object inside dynamic group %s - this must not happen", sName.String, pParent->Name());
                return;
            }

            m_lControl |= ZCHASDYNAMICPARENT;
            SetDynamicParentPtr(pParent);
        }

        if (IsDerivedFrom<ZGROUP>())
        {
            auto* pGroup = static_cast<ZGROUP*>(GetGeom());
            for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
            {
                pChild->SetDynamicParent(pParent);
            }
        }
    }

    void ZBaseGeom::SetDynamicParentPtr(const ZBaseGeom* pParent)
    {
        ZASSERT(Control() & ZCHASDYNAMICPARENT);

        m_iDynamicParentNr = pParent ? static_cast<uint16_t>(pParent - ZGeomBuffer::Instance().BaseGeomBufferPtr() + 1) : 0;
    }

    void ZBaseGeom::SetAutoRoomAssign(bool bAutoAssign)
    {
        if (bAutoAssign)
        {
            SetControl(ZCROOMASSIGN, 0u);
        }
        else
        {
            SetControl(0u, ZCROOMASSIGN);
        }
    }

    bool ZBaseGeom::IsHidden() const
    {
        if (Control() & ZCHIDDEN)
        {
            return true;
        }

        if (auto* pParent = Parent())
        {
            return pParent->IsHidden();
        }

        return false;
    }

    bool ZBaseGeom::IsMovingObject() const
    {
        if (auto* pGeom = GetGeom(); pGeom && pGeom->m_pExData)
        {
            return pGeom->m_pExData->_lControl & ZCEXMOVINGOBJ;
        }

        return false;
    }

    bool ZBaseGeom::WantCameraMsg() const
    {
        if (auto* pGeom = GetGeom(); pGeom && pGeom->m_pExData)
        {
            return pGeom->m_pExData->_lControl & ZCEXWANTCAMERAMSG;
        }

        return false;
    }

    MYSTR ZBaseGeom::CalcTotalName(bool bRoot) const
    {
        if (auto* pGeom = GetGeom())
        {
            return pGeom->CalcTotalName(bRoot);
        }
        else
        {
            MYSTR sName { "<NONAME>" };
            return sName;
        }
    }

    bool ZBaseGeom::RequestCustomDraw() const
    {
        if (auto* pGeom = GetGeom())
        {
            return pGeom->RequestCustomDraw();
        }

        return false;
    }

    void ZBaseGeom::CreateUniquePrim()
    {
        if (Prim() && static_cast<int>(g_pSysInterface->m_fRealTime) <= 0x64 && ZPrimControlBase::Instance()->IsPrimUnique(Prim()))
        {
            uint32_t lClonedPrim = ZPrimControlBase::Instance()->CopyPrim(Prim(), 0);
            SetPrim(lClonedPrim);
        }
    }

    void ZBaseGeom::SetOwnerDraw(bool bOwnerDraw)
    {
        if (bOwnerDraw)
        {
            SetControl(ZCOWNERDRAW, 0u);
        }
        else
        {
            SetControl(0u, ZCOWNERDRAW);
        }
    }

    void ZBaseGeom::SetUpdateLight(bool bUpdateLight)
    {
        constexpr uint32_t ZCUPDATELIGHT = 0x1000000u; // TODO: Find this mask

        m_lControl &= ~ZCUPDATELIGHT;
        if (bUpdateLight)
        {
            m_lControl |= ZCUPDATELIGHT;
        }
    }

    void ZBaseGeom::SetIsMoving(bool bMoving)
    {
        if (bMoving)
        {
            if (GetGeom())
            {
                if (!GetGeom()->m_pExData)
                {
                    GetGeom()->CreateExData();
                }

                GetGeom()->m_pExData->_lControl |= ZCEXMOVINGOBJ;
            }
            else
            {
                printf("ERROR: Trying to set ZCEXMOVINGOBJ on a BaseGeom without an ExtraGeom\n");
            }
        }
        else if (GetGeom() && GetGeom()->m_pExData)
        {
            GetGeom()->m_pExData->_lControl &= ~ZCEXMOVINGOBJ;
        }
    }

    void ZBaseGeom::SetMainRoom(ZROOM* pRoom)
    {
        const uint32_t lControl = Control();
        if ((lControl & ZCDYNAMIC) && (lControl & ZCROOMASSIGN) && ((lControl & (ZCOWNERDRAW | ZCINACTIVE | ZCHIDDEN)) == 0))
        {
            if (auto* pRoomList = GetRoomListPtr())
            {
                while (pRoomList->Count())
                {
                    RemoveFromRoomList(pRoomList->GetRoomNr(0));
                }
            }
            else
            {
                SetRoomList(ZGeomBuffer::Instance().AllocRoomList());
            }

            AddToRoomList(pRoom);
        }
    }

    void ZBaseGeom::GetLocalVect(ZVector3& vLocalVect)
    {
        const ZBaseGeom* aParents[256];
        uint32_t lNrParents = 0;

        for (const auto* pBaseGeom = this; pBaseGeom; pBaseGeom = pBaseGeom->Parent())
        {
            if (!pBaseGeom->Parent())
            {
                break;
            }

            ZASSERT(lNrParents < 256);
            aParents[lNrParents++] = pBaseGeom;
        }

        for (int32_t i = static_cast<int32_t>(lNrParents) - 1; i >= 0; --i)
        {
            vmtmul(vLocalVect.Get(), aParents[i]->Mat());
        }
    }

    void ZBaseGeom::GetLocalPointVect(ZVector3& point, ZVector3& vect)
    {
        if (!m_pParent)
        {
            return;
        }

        const ZBaseGeom* aParents[256];
        uint32_t lNrParents = 0;

        for (const auto* pBaseGeom = this; pBaseGeom; pBaseGeom = pBaseGeom->Parent())
        {
            ZASSERT(lNrParents < 256);
            aParents[lNrParents++] = pBaseGeom;
        }

        for (int32_t i = static_cast<int32_t>(lNrParents) - 1; i >= 0; --i)
        {
            const auto* pBaseGeom = aParents[i];

            point -= pBaseGeom->m_vPos;
            vmtmul(point.Get(), pBaseGeom->Mat());
            vmtmul(vect.Get(), pBaseGeom->Mat());
        }
    }

    void ZBaseGeom::GetRootPointVect(float* pPoint, float* pVect)
    {
        if (!m_pParent)
        {
            return;
        }

        for (const auto* pBaseGeom = this; pBaseGeom->Parent(); pBaseGeom = pBaseGeom->Parent())
        {
            vmmul(pPoint, pBaseGeom->Mat());
            pPoint[0] += pBaseGeom->m_vPos.x;
            pPoint[1] += pBaseGeom->m_vPos.y;
            pPoint[2] += pBaseGeom->m_vPos.z;

            vmmul(pVect, pBaseGeom->Mat());
        }
    }

    void ZBaseGeom::Zvmmul(ZVector3& v) const
    {
        vmmul(v.Get(), Mat());
    }

    void ZBaseGeom::AssignToRooms()
    {
        ZASSERT(Control() & ZCDYNAMIC);

        if (Control() & ZCINACTIVE)
        {
            AutoAssignToRooms();
        }
        else
        {
            // NOTE: Need think about next code, maybe we should wrap it into func like `ZROOM* GetOwnerRoom() const` ?
            auto* pRoot = Parent();
            for (; pRoot && pRoot->IsDerivedFrom<ZROOM>(); pRoot = pRoot->Parent())
            {
            }

            if (pRoot)
            {
                AddToRoomList(static_cast<ZROOM*>(pRoot->GetGeom()));
            }
            else
            {
                MYSTR sName = CalcTotalName(true);
                printf("WARNING: Geom %s will never be drawn as it is not attached under a room!", sName.String);
            }
        }
    }

    void ZBaseGeom::AutoAssignToRooms()
    {
        ZASSERT(Control() & ZCROOMASSIGN);

        if (Control() & (ZCOWNERDRAW | ZCINACTIVE | ZCHIDDEN))
        {
            FreeRoomList();
            return;
        }

        // ZMat3x3 mMat;
        // mMat.Reset();

        // ZVector3 vPos;
        // GetCen(vPos);
        // ZASSERT((vPos.x * vPos.x + vPos.y * vPos.y + vPos.z * vPos.z) < 1.0e12f);

        // ZVector3 vSize;
        // GetSize(vSize);
        // GetRootMatPos(mMat, vPos);
        // ZASSERT((vPos.x * vPos.x + vPos.y * vPos.y + vPos.z * vPos.z) < 1.0e12f);

        // ZROOM* aRooms[128] {};
        // uint32_t lNrRooms = 0;

        // Original flow:
        // - ZCollisionBase::GetCollisionInterface()->GetRoomsInsideBox(aRooms, &lNrRooms, mMat, vPos, vSize, false)
        // - if at least one room was found, expand through open connected exits intersecting the box

        // AdjustRoomList(aRooms, lNrRooms);
    }

    void ZBaseGeom::AdjustRoomList(ZROOM** ppRooms, uint32_t lNrRooms)
    {
        ZASSERT(Control() & ZCDYNAMIC);
        ZASSERT(!(Control() & ZCHASDYNAMICPARENT));

        if (lNrRooms >= 7)
        {
            printf("ZBaseGeom::AdjustRoomList lNrInnerRooms>MAXNRROOMSINROOMLIST (%s) %d", Name(), lNrRooms);

            for (uint32_t i = 0; i < lNrRooms; ++i)
            {
                ZASSERT(ppRooms[i]);
                if (ppRooms[i])
                {
                    printf("- %d : %s", i, ppRooms[i]->Name());
                }
            }
        }

        auto* pRoomList = GetRoomList();
        uint32_t lNrNewRooms = lNrRooms >= 7 ? 6 : lNrRooms;
        ZROOM* aNewRooms[6] {};

        for (uint32_t i = 0; i < lNrNewRooms; ++i)
        {
            aNewRooms[i] = ppRooms[i];
        }

        if (pRoomList)
        {
            for (uint32_t i = 0; i < pRoomList->Count();)
            {
                ZROOM* pRoom = pRoomList->GetRoomNr(i);
                bool bRoomFound = false;

                for (uint32_t j = 0; j < lNrNewRooms; ++j)
                {
                    if (aNewRooms[j] == pRoom)
                    {
                        aNewRooms[j] = aNewRooms[--lNrNewRooms];
                        bRoomFound = true;
                        ++i;
                        break;
                    }
                }

                if (!bRoomFound)
                {
                    RemoveFromRoomList(pRoom);
                }
            }
        }

        for (uint32_t i = 0; i < lNrNewRooms; ++i)
        {
            AddToRoomList(aNewRooms[i]);
        }

        pRoomList = GetRoomList();
        if (pRoomList)
        {
            pRoomList->Clear();
            for (uint32_t i = 0; i < lNrRooms; ++i)
            {
                pRoomList->Add(ppRooms[i]);
            }
        }
    }

    void ZBaseGeom::CalcCenSize(bool bCalledByZGEOM)
    {
        if (bCalledByZGEOM || !GetGeom())
        {
            const auto* pCen = Cen();
            const auto* pSize = Size();

            if (ZPrimControlBase::Instance()->CalcPrimCenSize(Prim(), m_vCen.Get(), m_vSize.Get(), true))
            {
                SetRadius(m_vSize.Length());
            }
            else if (IsDerivedFrom<ZSTDOBJ>())
            {
                SetSize({ 50.f });
                SetCen({ 0.f });
                SetRadius(51.f);
            }
            else
            {
                SetRadius(0.f);
                SetCen({ 0.f });
                SetSize({ 0.f });
            }
        }
        else
        {
            GetGeom()->CalcCenSize();
        }
    }

    void ZBaseGeom::ForceCalcMaxMin()
    {
        CalcCenSize(false);
        AdjustMinMax(nullptr);
    }

    void ZBaseGeom::AdjustMinMax(ZBaseGeom* pChildBaseGeom)
    {
        bool bChanged = false;

        if (pChildBaseGeom)
        {
            ZASSERT(IsDerivedFrom<ZGROUP>());

            if (!pChildBaseGeom->DisableParentBoundAdjust())
            {
                ZVector3 vChildCen;
                ZVector3 vChildSize;

                if (pChildBaseGeom->GetParentBox(vChildCen, vChildSize))
                {
                    ZVector3 vMax;
                    ZVector3 vMin;

                    if (Radius() == 0.0f)
                    {
                        vMin = vChildCen - vChildSize;
                        vMax = vChildCen;
                        vMax += vChildSize;
                        bChanged = true;
                    }
                    else
                    {
                        vMax = m_vCen;
                        vMax += m_vSize;
                        vMin = m_vCen - m_vSize;

                        const ZVector3 vChildMin = vChildCen - vChildSize;
                        ZVector3 vChildMax = vChildCen;
                        vChildMax += vChildSize;

                        if (vMax.x < vChildMax.x) { vMax.x = vChildMax.x; bChanged = true; }
                        if (vMax.y < vChildMax.y) { vMax.y = vChildMax.y; bChanged = true; }
                        if (vMax.z < vChildMax.z) { vMax.z = vChildMax.z; bChanged = true; }
                        if (vChildMin.x < vMin.x) { vMin.x = vChildMin.x; bChanged = true; }
                        if (vChildMin.y < vMin.y) { vMin.y = vChildMin.y; bChanged = true; }
                        if (vChildMin.z < vMin.z) { vMin.z = vChildMin.z; bChanged = true; }
                    }

                    if (bChanged)
                    {
                        ZVector3 vCen = vMax;
                        vCen += vMin;
                        vCen.x *= 0.5f;
                        vCen.y *= 0.5f;
                        vCen.z *= 0.5f;
                        SetCen(vCen);

                        ZVector3 vSize = vMax - vCen;
                        SetSize(vSize);
                        SetRadius(vSize.Length() + 1.0f);

                        auto* pGroup = static_cast<ZGROUP*>(GetGeom());
                        std::memcpy(pGroup->m_vSizeInsideCheck, Size(), sizeof(pGroup->m_vSizeInsideCheck));
                        std::memcpy(pGroup->m_vCenInsideCheck, Cen(), sizeof(pGroup->m_vCenInsideCheck));
                    }
                }
            }
        }

        if (!pChildBaseGeom || bChanged)
        {
            if (ChkUpdateMinMax())
            {
                if (auto* pParent = Parent())
                {
                    pParent->AdjustMinMax(this);
                }
            }
        }
    }

    bool ZBaseGeom::DisableParentBoundAdjust() const
    {
        if (Control() & ZCDYNAMIC)
        {
            return true;
        }
        else if (auto* pGeom = GetGeom())
        {
            return pGeom->DisableParentBoundAdjust();
        }

        return false;
    }

    bool ZBaseGeom::GetParentBox(ZVector3& vCen, ZVector3& vSize) const
    {
        if (ChkUpdateMinMax() && Radius() != 0.f) // Compare with 0.0f is not safe, need check in BUGFIX
        {
            if (Parent())
            {
                vCen = Cen();
                vSize = Size();

                TransformBox(Mat(), vSize.Get());
                vmmul(vCen.Get(), Mat());
                vCen += m_vPos;

                return true;
            }
        }

        return false;
    }

    bool ZBaseGeom::CalcPrimCenSizeAlongMat(ZVector3& vCen, ZVector3& vSize, ZMat3x3& mMat) const
    {
        return ZPrimControlBase::Instance()->CalcPrimCenSizeAlongMat(Prim(), vCen.Get(), vSize.Get(), mMat.Get(), false);
    }

    void ZBaseGeom::LightNotifyPotentialDetachment(bool bIncrease)
    {
        // TODO: Finish me
    }

    void ZBaseGeom::FixLightList()
    {
        // TODO: Finish me
    }

    void ZBaseGeom::UpdateLightListForLight()
    {
        // TODO: Finish me
    }

    void ZBaseGeom::UpdateLightListForGeom()
    {
        // TODO: Finish me
    }

    bool ZBaseGeom::IsDerivedFromStdObj(uint32_t lClassId) const
    {
        return ZSTDOBJ::m_Id == lClassId; // NOTE: In PS2 used ZSTDOBJ::GetClassId() call, but it's same
    }

    void ZBaseGeom::SetAttachUpdate()
    {
        if (!m_lDrawId)
        {
            return;
        }

        // TODO: Finish me after ZRenderDraw reversed!
        // ZRenderEntry* pDrawEntry = IDraw::Instance<ZRenderDraw>()->m_DrawIdToPointer[m_lDrawId];
        // pDrawEntry->m_lControl |= ZRenderEntry::RENDERENTRY_FLAGS::RE_ATTACH_UPDATE;
    }

    template bool ZBaseGeom::IsDerivedFrom<ZSTDOBJ>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZSHAPE>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZBOUND>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZSNDOBJ>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZGROUP>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZLIGHT>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZLNKOBJ>() const;
}
