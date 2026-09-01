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
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/Geom/ZGateLightOmni.h>
#include <Glacier/Geom/ZGateLightSpot.h>
#include <Glacier/Geom/ZGateLightSpotSquare.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Geom/ZEngineGeomControl.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRender.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        /**
         * @brief Collects the "top groups" of a light, i.e. the chain of groups the light
         *        shines out of, terminated by the first group it does not shine out of.
         *        (PC 0x00432A90)
         */
        int FindLightTopGroups(const ZBaseGeom* pBaseGeom, ZGROUP** paGroups)
        {
            ZASSERT(pBaseGeom->IsDerivedFrom<ZLIGHT>());

            int lNrGroups = 0;

            if ((pBaseGeom->Control() & (ZCOWNERDRAW | ZCROOMASSIGN | ZCHASDYNAMICPARENT | ZCDYNAMIC)) != 0)
            {
                const ZBaseGeom* pCur = pBaseGeom;
                while (pCur)
                {
                    if ((pCur->Control() & ZCDYNAMIC) != 0)
                    {
                        if (auto* pRoomList = pCur->GetRoomList())
                        {
                            if (pRoomList->Count())
                            {
                                paGroups[lNrGroups++] = static_cast<ZGROUP*>(pRoomList->GetRoomNr(0));
                            }
                        }
                        return lNrGroups;
                    }

                    pCur = pCur->Parent();
                    ZASSERT(pCur && pCur->IsDerivedFrom<ZGROUP>());

                    auto* pGroupGeom = static_cast<ZGROUP*>(pCur->GetGeom());
                    if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_LIGHT_SHINES_OUT) == 0)
                    {
                        paGroups[lNrGroups] = pGroupGeom;
                        return lNrGroups + 1;
                    }

                    if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_LIGHT_SHINES_IN) == 0)
                    {
                        paGroups[lNrGroups++] = pGroupGeom;
                    }
                }

                return lNrGroups;
            }

            const ZBaseGeom* pCur = pBaseGeom->Parent();
            ZASSERT(pCur);

            while (true)
            {
                auto* pGroupGeom = static_cast<ZGROUP*>(pCur->GetGeom());
                if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_LIGHT_SHINES_OUT) == 0)
                {
                    break;
                }

                if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_LIGHT_SHINES_IN) == 0)
                {
                    paGroups[lNrGroups++] = pGroupGeom;
                }

                pCur = pCur->Parent();
                if (!pCur)
                {
                    return lNrGroups;
                }
            }

            paGroups[lNrGroups] = static_cast<ZGROUP*>(pCur->GetGeom());
            return lNrGroups + 1;
        }

        /**
         * @brief Checks whether an object is affected by a light, i.e. whether any of its
         *        ancestor groups belongs to the light's top groups.
         *        (PC 0x00432B90)
         */
        bool IsObjectAffectedByLight(const ZBaseGeom* pObject, const ZBaseGeom* pLightBase, int lNumGroups, ZGROUP** paGroups)
        {
            ZASSERT(pLightBase->IsDerivedFrom<ZLIGHT>());

            auto* pLightGeom = static_cast<ZLIGHT*>(pLightBase->GetGeom());
            if (pLightGeom->IsGeomExcluded(pObject->GetRef()))
            {
                return false;
            }

            if (lNumGroups == 0)
            {
                return false;
            }

            if ((pObject->Control() & (ZCOWNERDRAW | ZCROOMASSIGN | ZCHASDYNAMICPARENT | ZCDYNAMIC)) != 0)
            {
                const ZBaseGeom* pCur = pObject;
                while (pCur)
                {
                    if ((pCur->Control() & ZCDYNAMIC) != 0)
                    {
                        if (auto* pRoomList = pCur->GetRoomList())
                        {
                            if (pRoomList->Count())
                            {
                                auto* pRoomGeom = static_cast<ZGROUP*>(pRoomList->GetRoomNr(0));
                                for (int i = 0; i < lNumGroups; ++i)
                                {
                                    if (paGroups[i] == pRoomGeom)
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        return false;
                    }

                    pCur = pCur->Parent();
                    ZASSERT(pCur && pCur->IsDerivedFrom<ZGROUP>());

                    auto* pGroupGeom = static_cast<ZGROUP*>(pCur->GetGeom());
                    for (int i = 0; i < lNumGroups; ++i)
                    {
                        if (paGroups[i] == pGroupGeom)
                        {
                            return true;
                        }
                    }

                    if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_LIGHT_SHINES_IN) == 0)
                    {
                        return false;
                    }
                }

                return false;
            }

            const ZBaseGeom* pCur = pObject->Parent();
            ZASSERT(pCur);

            while (true)
            {
                auto* pGroupGeom = static_cast<ZGROUP*>(pCur->GetGeom());
                for (int i = 0; i < lNumGroups; ++i)
                {
                    if (paGroups[i] == pGroupGeom)
                    {
                        return true;
                    }
                }

                if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_LIGHT_SHINES_IN) == 0)
                {
                    break;
                }

                pCur = pCur->Parent();
                if (!pCur)
                {
                    return false;
                }
            }

            return false;
        }

        /**
         * @brief Recursively collects all lights below a group tree that affect the object.
         *        (PC 0x00432CF0)
         */
        int GetLightsRecur(int iLights, uint32_t* pCatch, const ZBaseGeom* pObject, ZBaseGeom* pBaseGeom)
        {
            if (pBaseGeom->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroup = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
                for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                {
                    if ((pChild->Control() & (ZCINVISIBLE | ZCHIDDEN | ZCINACTIVE)) == 0)
                    {
                        iLights = GetLightsRecur(iLights, pCatch, pObject, pChild);
                    }
                }
            }
            else if (pBaseGeom->IsDerivedFrom<ZLIGHT>() && pBaseGeom->m_uListID != 0)
            {
                ZASSERT(!pBaseGeom->IsDerivedFrom<ZENVIRONMENT>());

                ZGROUP* aGroups[128] {};
                const int lNumGroups = FindLightTopGroups(pBaseGeom, aGroups);
                ZASSERT(lNumGroups <= 128);

                if (IsObjectAffectedByLight(pObject, pBaseGeom, lNumGroups, aGroups))
                {
                    ZASSERT(iLights < 512);
                    pCatch[iLights++] = reinterpret_cast<uint32_t>(pBaseGeom);
                }
            }

            return iLights;
        }
    }

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

        m_lControl |= ZCUPDATELIGHT;

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
                SetControl(0x80000000u, 0u); // NOTE: Find this mask
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

    ZBaseGeom* ZBaseGeom::Next() const
    {
        return m_iNext ? ZGeomBuffer::Instance().BaseGeomBufferPtr() + m_iNext - 1 : nullptr;
    }

    void ZBaseGeom::SetNext(ZBaseGeom* next)
    {
        m_iNext = next ? static_cast<uint16_t>(next - ZGeomBuffer::Instance().BaseGeomBufferPtr() + 1) : 0;
    }

    ZBaseGeom* ZBaseGeom::GetPrev() const
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

            if (g_pEngineData && g_pEngineData->RunTime())
            {
                if (lRealAddBits & ZCCHKLIGHT)
                {
                    ZASSERT((m_lControl & ZCNONRUNTIME) == 0);

                    if (m_lPrim && (IsDerivedFrom<ZSTDOBJ>() || IsDerivedFrom<ZLIGHT>()))
                    {
                        if (auto* pListUser = g_pEngineData->GetListUser())
                        {
                            ZASSERT(!m_uListID);
                            m_uListID = pListUser->AddRuntimeMember(this);
                            m_lControl |= ZCUPDATELIGHT;
                        }
                    }
                }
                else if ((lRealRemBits & ZCCHKLIGHT)
                    && (m_lControl & ZCNONRUNTIME) == 0
                    && m_lPrim
                    && (IsDerivedFrom<ZSTDOBJ>() || IsDerivedFrom<ZLIGHT>()))
                {
                    if (auto* pListUser = g_pEngineData->GetListUser())
                    {
                        ZASSERT(m_uListID);

                        if (IsDerivedFrom<ZLIGHT>())
                        {
                            LightNotifyPotentialDetachment(false);
                        }

                        pListUser->RemoveRuntimeMember(this);
                        m_uListID = 0;
                        m_lControl |= ZCUPDATELIGHT;
                    }
                }
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
                if (m_uListID)
                {
                    if (auto* pListUser = g_pEngineData->GetListUser())
                    {
                        pListUser->DisconnectFromAllMembers(this);
                    }
                }

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
            else if (!g_pEngineData->MinMaxLocked())
            {
                AdjustMinMax(nullptr);
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
        if ((Control() & (ZCOWNERDRAW|0xC00u)) == 0) // NOTE: Find mask
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

void ZBaseGeom::RemoveDynamicParent()
    {
        if (Control() & ZCHASDYNAMICPARENT)
        {
            m_iRoomListNr = 0;
            m_lControl &= ~ZCHASDYNAMICPARENT;
        }

        if (IsDerivedFrom<ZGROUP>())
        {
            auto* pGroup = static_cast<ZGROUP*>(GetGeom());

            for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
            {
                pChild->RemoveDynamicParent();
            }
        }
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

        ZMat3x3 mMat;
        mMat.Reset();

        ZVector3 vPos;
        GetCen(vPos);
        ZASSERT(vlen(vPos.Get()) < 1000000.0f);

        ZVector3 vSize;
        GetSize(vSize);

        if (Parent())
        {
            GetRootPoint(vPos);
            GetRootMat(mMat);
        }

        ZASSERT(vlen(vPos.Get()) < 1000000.0f);

        ZROOM* aRooms[128] {};
        uint32_t lNrRooms = 0;

        if (auto* pColi = ZCollisionBase::GetCollisionInterface())
        {
            lNrRooms = pColi->GetInnerRoomsLst(aRooms, &aRooms[128], mMat.data, vPos.Get(), vSize.Get(), false);
        }

        if (lNrRooms)
        {
            auto* pRoom = aRooms[0];

            // Enlarge the query box to also reach rooms behind open exits
            ZVector3 vExitBoxSize;
            vExitBoxSize.x = vSize.x * 2.75f;
            vExitBoxSize.y = vSize.y;
            vExitBoxSize.z = vSize.z * 2.75f;

            uint16_t aExitIndices[16] {};
            const uint32_t lNrExitIndices = pRoom->GetExitsIndicesInsideBox(aExitIndices, 16, mMat, vPos, vExitBoxSize);

            for (uint32_t i = 0; i < lNrExitIndices; ++i)
            {
                const auto* pExit = &pRoom->m_pExits[aExitIndices[i]];

                if ((pExit->m_lControl & 0x3u) == 0x2u) // connected && open
                {
                    bool bAlreadyInList = false;
                    for (uint32_t j = 0; j < lNrRooms; ++j)
                    {
                        if (aRooms[j] == pExit->m_pNeighbor)
                        {
                            bAlreadyInList = true;
                            break;
                        }
                    }

                    if (!bAlreadyInList)
                    {
                        aRooms[lNrRooms++] = pExit->m_pNeighbor;
                    }
                }
            }
        }

        AdjustRoomList(aRooms, lNrRooms);
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
        ZASSERT(IsDerivedFrom<ZLIGHT>());

        if (bIncrease)
        {
            if (m_lPotentialLightListChange == 0)
            {
                if (auto* pListUser = g_pEngineData->GetListUser())
                {
                    uint32_t lNrMembers = 0;
                    if (auto* pMembers = pListUser->UnfoldList(&lNrMembers, m_uListID))
                    {
                        for (uint32_t i = 0; i < lNrMembers; ++i)
                        {
                            auto* pMember = reinterpret_cast<ZBaseGeom*>(pMembers[i]);
                            ++pMember->m_lPotentialLightListChange;
                            pMember->m_lControl |= ZCUPDATELIGHT;
                        }
                    }
                }

                m_lPotentialLightListChange = 1;
            }
        }
        else if (m_lPotentialLightListChange != 0)
        {
            if (auto* pListUser = g_pEngineData->GetListUser())
            {
                uint32_t lNrMembers = 0;
                if (auto* pMembers = pListUser->UnfoldList(&lNrMembers, m_uListID))
                {
                    for (uint32_t i = 0; i < lNrMembers; ++i)
                    {
                        auto* pMember = reinterpret_cast<ZBaseGeom*>(pMembers[i]);
                        if (pMember->m_lPotentialLightListChange != 0)
                        {
                            --pMember->m_lPotentialLightListChange;
                        }
                    }
                }
            }

            m_lPotentialLightListChange = 0;
        }
    }

    void ZBaseGeom::FixLightList()
    {
        if (auto* pListUser = g_pEngineData->GetListUser())
        {
            ZASSERT(m_lPotentialLightListChange != 0);

            uint32_t lNrLights = 0;
            if (uint32_t* pLights = pListUser->UnfoldList(&lNrLights, m_uListID))
            {
                ZASSERT(lNrLights <= 512);

                // Copy the unfolded list, as the updates below reuse the internal unfold buffer
                uint32_t aLightValues[512] {};
                std::memcpy(aLightValues, pLights, sizeof(uint32_t) * lNrLights);

                for (uint32_t i = 0; i < lNrLights; ++i)
                {
                    auto* pLight = reinterpret_cast<ZBaseGeom*>(aLightValues[i]);
                    if (pLight->Control() & ZCLIGHTCHANGED)
                    {
                        pLight->LightNotifyPotentialDetachment(false);
                        pLight->UpdateLightListForLight();
                    }
                }
            }

            m_lPotentialLightListChange = 0;
        }
    }

    void ZBaseGeom::UpdateLightListForLight()
    {
        if (IsDerivedFrom<ZENVIRONMENT>())
        {
            return;
        }

        auto* pListUser = g_pEngineData->GetListUser();
        if (!pListUser)
        {
            SetControl(0, ZCLIGHTCHANGED);
            return;
        }

        if (m_bFreezeLightList)
        {
            SetControl(0, ZCLIGHTCHANGED);
            pListUser->NotifyAllMembers(this);
            return;
        }

        if (m_uListID == 0)
        {
            return;
        }

        uint32_t lMaxElems = 0;
        uint32_t* pCatch = pListUser->GetCatchBuffer(&lMaxElems);
        ZASSERT(lMaxElems == 512);

        ZBaseGeomRoomList* pRoomList = nullptr;
        ZROOM* pSingleRoom = nullptr;
        uint32_t lNrRooms = 0;

        if ((Control() & (ZCOWNERDRAW | ZCROOMASSIGN | ZCHASDYNAMICPARENT | ZCDYNAMIC)) != 0)
        {
            auto* pDynamicParent = (Control() & ZCHASDYNAMICPARENT) ? GetDynamicParent() : this;
            pRoomList = pDynamicParent->GetRoomListPtr();
            if (pRoomList && pRoomList->Count())
            {
                lNrRooms = pRoomList->Count();
            }
            else
            {
                pRoomList = nullptr;
                pSingleRoom = g_pEngineData->m_pRoot;
                lNrRooms = 1;
            }
        }
        else
        {
            pSingleRoom = static_cast<ZROOM*>(GetGeom()->GetOwner(false));
            lNrRooms = 1;
        }

        ZGROUP* aGroups[128] {};
        int lNumGroups = 0;
        if ((Control() & ZCROOMASSIGN) == 0)
        {
            lNumGroups = FindLightTopGroups(this, aGroups);
            ZASSERT(lNumGroups <= 128);
        }

        ZMat3x3 mMat;
        mMat.Reset();

        ZVector3 vPos;
        GetCen(vPos);

        if (Parent())
        {
            GetRootPoint(vPos);
            GetRootMat(mMat);
        }

        ZBaseGeom* aGeoms[1024] {};
        uint32_t lNrGeoms = 0;

        if (pRoomList)
        {
            lNrGeoms = ZCollisionBase::GetCollisionInterface()->GetDynamicGeomsInBoxInRooms(
                aGeoms, &aGeoms[1024], eGlobalTreeType::GT_StdObjs, pRoomList->GetRoomList(), pRoomList->Count(),
                mMat.data, vPos.Get(), Size(), -1, true);

            if ((Control() & ZCROOMASSIGN) != 0)
            {
                lNumGroups = pRoomList->Count();
                for (int i = 0; i < lNumGroups; ++i)
                {
                    aGroups[i] = static_cast<ZGROUP*>(pRoomList->GetRoomNr(i));
                }
            }
        }
        else
        {
            lNumGroups = 1;
            aGroups[0] = static_cast<ZGROUP*>(pSingleRoom);

            lNrGeoms = ZCollisionBase::GetCollisionInterface()->GetDynamicGeomsInBoxInRooms(
                aGeoms, &aGeoms[1024], eGlobalTreeType::GT_StdObjs, &pSingleRoom, 1,
                mMat.data, vPos.Get(), Size(), -1, true);
        }

        uint32_t lNrLights = 0;

        // Dynamic geoms in the light's rooms
        for (uint32_t i = 0; i < lNrGeoms; ++i)
        {
            auto* pGeom = aGeoms[i];

            if (pGeom->IsDerivedFrom<ZLIGHT>())
            {
                continue;
            }

            if (pGeom->m_uListID != 0 && pGeom->m_lPrim != 0 && IsObjectAffectedByLight(pGeom, this, lNumGroups, aGroups))
            {
                pCatch[lNrLights++] = reinterpret_cast<uint32_t>(pGeom);
            }
        }

        // Static geoms in each of the light's rooms
        for (uint32_t lRoomNr = 0; lRoomNr < lNrRooms; ++lRoomNr)
        {
            ZROOM* pRoom = pSingleRoom;
            if (pRoomList)
            {
                pRoom = pRoomList->GetRoomNr(lRoomNr);
                lNumGroups = 1;
                aGroups[0] = static_cast<ZGROUP*>(pRoom);
            }

            ZMat3x3 mLocalMat;
            mLocalMat.Reset();

            ZVector3 vLocalPos;
            GetCen(vLocalPos);

            if (Parent())
            {
                GetRootPoint(vLocalPos);
                GetRootMat(mLocalMat);
            }

            pRoom->BaseGeom()->GetLocalMatPos(mLocalMat, vLocalPos);

            ZBaseGeom* aRoomGeoms[1024] {};
            const uint32_t lNrRoomGeoms = ZCollisionBase::GetCollisionInterface()->GetGeomsInBoxLocal(
                aRoomGeoms, &aRoomGeoms[1024], static_cast<ZTreeGroup*>(pRoom), eGlobalTreeType::GT_StdObjs,
                mLocalMat.data, vLocalPos.Get(), Size(), -1, true, false, true);

            for (uint32_t i = 0; i < lNrRoomGeoms; ++i)
            {
                auto* pGeom = aRoomGeoms[i];

                if (pGeom->IsDerivedFrom<ZLIGHT>())
                {
                    continue;
                }

                if (pGeom->m_uListID != 0 && pGeom->m_lPrim != 0 && IsObjectAffectedByLight(pGeom, this, lNumGroups, aGroups))
                {
                    pCatch[lNrLights++] = reinterpret_cast<uint32_t>(pGeom);
                }
            }
        }

        pListUser->AnalyzeCatch(lNrLights, this);
        SetControl(0, ZCLIGHTCHANGED);
    }

    void ZBaseGeom::UpdateLightListForGeom()
    {
        if (!g_pEngineData->GetListUser() || (Control() & ZCOWNERDRAW))
        {
            SetControl(0, ZCLIGHTCHANGED);
            return;
        }

        ZASSERT(m_uListID != 0);

        if (m_bFreezeLightList)
        {
            SetControl(ZCUPDATELIGHT, ZCLIGHTCHANGED);
            return;
        }

        auto* pListUser = g_pEngineData->GetListUser();
        uint32_t lMaxElems = 0;
        uint32_t* pCatch = pListUser->GetCatchBuffer(&lMaxElems);
        ZASSERT(lMaxElems == 512);

        ZGEOM* pRoomGeom = nullptr;
        ZBaseGeomRoomList* pRoomList = nullptr;

        if ((Control() & (ZCOWNERDRAW | ZCROOMASSIGN | ZCHASDYNAMICPARENT | ZCDYNAMIC)) != 0)
        {
            auto* pDynamicParent = (Control() & ZCHASDYNAMICPARENT) ? GetDynamicParent() : this;
            pRoomList = pDynamicParent->GetRoomListPtr();
            if (pRoomList && pRoomList->Count())
            {
                pRoomGeom = pRoomList->GetRoomNr(0);
            }
            else
            {
                pRoomGeom = g_pEngineData->m_pRoot;
            }
        }
        else
        {
            pRoomGeom = GetGeom()->GetOwner(false);
            ZASSERT(pRoomGeom && pRoomGeom->IsDerivedFrom<ZROOM>());
        }

        ZMat3x3 mMat;
        mMat.Reset();

        ZVector3 vPos;
        GetCen(vPos);

        if (Parent())
        {
            GetRootPoint(vPos);
            GetRootMat(mMat);
        }

        pRoomGeom->BaseGeom()->GetLocalMatPos(mMat, vPos);

        ZBaseGeom* aGeoms[1024] {};
        const uint32_t lNrGeoms = ZCollisionBase::GetCollisionInterface()->GetGeomsInBoxLocal(
            aGeoms, &aGeoms[1024], static_cast<ZTreeGroup*>(pRoomGeom), eGlobalTreeType::GT_Lights,
            mMat.data, vPos.Get(), Size(), -1, true, true, true);

        uint32_t lNrLights = 0;
        for (uint32_t i = 0; i < lNrGeoms; ++i)
        {
            auto* pGeom = aGeoms[i];

            if ((pGeom->Control() & ZCINACTIVE) != 0)
            {
                continue;
            }

            if (pGeom->IsDerivedFrom<ZENVIRONMENT>())
            {
                continue;
            }

            if (pGeom->IsDerivedFrom<ZLIGHT>())
            {
                if (pGeom->m_uListID == 0)
                {
                    continue;
                }

                ZGROUP* aGroups[128] {};
                const int lNumGroups = FindLightTopGroups(pGeom, aGroups);
                ZASSERT(lNumGroups <= 128);

                if (!IsObjectAffectedByLight(this, pGeom, lNumGroups, aGroups))
                {
                    continue;
                }

                ZASSERT(lNrLights < 512);
                pCatch[lNrLights++] = reinterpret_cast<uint32_t>(pGeom);

                // Also add the light's master light (fixture) when it resides in the same room(s)
                auto* pLightGeom = static_cast<ZLIGHT*>(pGeom->GetGeom());
                if (pLightGeom->m_rMasterLight)
                {
                    if (auto* pMasterGeom = ZGEOM::RefToPtr(pLightGeom->m_rMasterLight))
                    {
                        auto* pMasterRoom = pMasterGeom->BaseGeom()->GetOwnerRoom();

                        const bool bInTargetRooms = pRoomList
                            ? (pMasterRoom && pRoomList->Exists(pMasterRoom))
                            : (static_cast<ZGEOM*>(pMasterRoom) == pRoomGeom);

                        if (bInTargetRooms)
                        {
                            ZASSERT(lNrLights < 512);
                            pCatch[lNrLights++] = reinterpret_cast<uint32_t>(pMasterGeom->BaseGeom());
                        }
                    }
                }

                // Gate lights: also collect their slave lights that reside in the same room(s)
                REFTAB* pGateSlaves = nullptr;
                if (pGeom->IsDerivedFrom<ZGateLightOmni>())
                {
                    pGateSlaves = &static_cast<ZGateLightOmni*>(pGeom->GetGeom())->m_Slaves;
                }
                else if (pGeom->IsDerivedFrom<ZGateLightSpot>())
                {
                    pGateSlaves = &static_cast<ZGateLightSpot*>(pGeom->GetGeom())->m_Slaves;
                }
                else if (pGeom->IsDerivedFrom<ZGateLightSpotSquare>())
                {
                    pGateSlaves = &static_cast<ZGateLightSpotSquare*>(pGeom->GetGeom())->m_Slaves;
                }

                if (pGateSlaves)
                {
                    for (const uint32_t lSlaveRef : *pGateSlaves)
                    {
                        auto* pSlaveGeom = ZGEOM::RefToPtr(lSlaveRef);
                        if (!pSlaveGeom)
                        {
                            continue;
                        }

                        auto* pSlaveRoom = pSlaveGeom->BaseGeom()->GetOwnerRoom();

                        const bool bInTargetRooms = pRoomList
                            ? (pSlaveRoom && pRoomList->Exists(pSlaveRoom))
                            : (static_cast<ZGEOM*>(pSlaveRoom) == pRoomGeom);

                        if (bInTargetRooms)
                        {
                            ZASSERT(lNrLights < 512);
                            pCatch[lNrLights++] = reinterpret_cast<uint32_t>(pSlaveGeom->BaseGeom());
                        }
                    }
                }
            }
            else if ((pGeom->Control() & ZCDYNAMIC) != 0 && pGeom->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroupGeom = static_cast<ZGROUP*>(pGeom->GetGeom());
                if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_GROUP_CONTAINS_LIGHT) != 0)
                {
                    lNrLights = GetLightsRecur(lNrLights, pCatch, this, pGeom);
                }
            }
        }

        if (lNrLights >= 13)
        {
            printf("WARNING: Geom '%s' is hit by %d lights! Truncating list to %d lights!\n", Name(), lNrLights, 12);
            lNrLights = 12;
        }

        pListUser->AnalyzeCatch(lNrLights, this);
        SetControl(0, ZCLIGHTCHANGED);
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

        ZRenderEntry* pDrawEntry = IDraw::Instance<ZRenderDraw>()->m_apRenderEntryLookup[m_lDrawId];
        pDrawEntry->m_lControl |= ZRenderEntry::RENDERENTRY_FLAGS::RE_ATTACH_UPDATE;
    }

    void ZBaseGeom::GetRootBox(ZVector3& vMin, ZVector3& vMax) const
    {
        vMin = m_vCen;

        ZMat3x3 mMat {};
        mMat.Reset();

        if (m_pParent)
        {
            GetRootPoint(vMin);
            GetRootMat(mMat);
        }

        vMax = m_vSize;
        TransformBox(mMat.Get(), vMax.Get());
    }

    template bool ZBaseGeom::IsDerivedFrom<ZSTDOBJ>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZSHAPE>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZBOUND>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZSNDOBJ>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZGROUP>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZLIGHT>() const;
    template bool ZBaseGeom::IsDerivedFrom<ZLNKOBJ>() const;
}
