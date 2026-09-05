#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/ZWINOBJSPRITEHOLDER.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Render/Draw/ZDrawBuffer.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SPrimSpritesArray.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>

#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        template <typename T>
        void LoadVirtualEnum(RTP::ZVirtualEnumProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
        {
            T value {};
            stream.ExchangeEnum(pProperty->m_Node.m_Name, &value, 1, *pProperty->m_Info);

            uintptr_t address = 0;
            std::memcpy(&address, &pProperty->m_Set.__pfn, sizeof(address));
            using Setter = void(__thiscall*)(void*, const T&);
            reinterpret_cast<Setter>(address)(reinterpret_cast<uint8_t*>(&object) + pProperty->m_Set.__delta, value);
        }

        template <typename T>
        void SaveVirtualEnum(RTP::ZVirtualEnumProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
        {
            T value {};
            uintptr_t address = 0;
            std::memcpy(&address, &pProperty->m_Get.__pfn, sizeof(address));
            using Getter = void(__thiscall*)(void*, T&);
            reinterpret_cast<Getter>(address)(reinterpret_cast<uint8_t*>(&object) + pProperty->m_Get.__delta, value);
            stream.ExchangeEnum(pProperty->m_Node.m_Name, &value, 1, *pProperty->m_Info);
        }
    }

    ZWINOBJ::ZWINOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_dwFaceColor(0xFFFFFFFF)
        , m_dwDrawMode(0)
        , m_v2Scale { 1.0f, 1.0f }
        , m_pSprites(nullptr)
        , m_iNumSprites(0)
        , m_iNumSpriteArrays(0)
        , m_pPrims(nullptr)
        , m_pSpriteArrays(nullptr)
        , m_vAlignmentOffset {}
        , m_vLastCamPos { 1.0e38f, 1.0e38f, 1.0e38f }
        , m_dwAlignment(0x11)
        , m_dwType(0x80)
        , m_dwAmount(0xFF)
        , m_iPriority(0x10)
        , m_bModified(true)
        , m_bStaticSprites(false)
        , m_bAnimateAlpha(false)
        , m_bScalePS2_512_448(true)
    {
    }

    ZWINOBJ::~ZWINOBJ()
    {
        RemoveGeometry();
    }

    bool ZWINOBJ::PostLoad(ISerializerStream& stream)
    {
        if (m_iPriority > 0x10)
            m_iPriority = 0x10;

        return true;
    }

    void ZWINOBJ::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZGEOM::LoadSave(stream, bSaving);

        bool bStaticSprites = m_bStaticSprites;
        stream.Exchange("m_bStaticSprites", bStaticSprites);
        if (!bSaving)
            m_bStaticSprites = bStaticSprites;

        LoadSaveGeometry(stream, bSaving);
    }

    const RTP::ZPropertyInfo& ZWINOBJ::GetProperties() const
    {
        return ZWINOBJ::Info;
    }

    uint32_t ZWINOBJ::GetObjectId() const
    {
        return ZWINOBJ::m_Id;
    }

    void ZWINOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWINOBJ::m_Id;
        mask = ZWINOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* ZWINOBJ::GetOldClassInfo() const
    {
        return ZWINOBJ::m_OldClassInfo;
    }

    void ZWINOBJ::CalcCenSize()
    {
        ZSTDOBJ::CalcCenSize();

        ZVector3 vCen;
        ZVector3 vSize;
        if (m_pSpriteArrays)
        {
            ZVector3 vMin { 1.0e38f, 1.0e38f, 1.0e38f };
            ZVector3 vMax { -1.0e38f, -1.0e38f, -1.0e38f };
            for (uint32_t i = 0; i < m_iNumSpriteArrays; ++i)
            {
                const SSpriteArray& array = m_pSpriteArrays[i];
                for (uint32_t j = 0; j < array.lNumSprites; ++j)
                {
                    const SSpriteArrayElementUV& sprite = array.pSpritesUV[j];
                    const ZVector3 vHalfSize { sprite.fScale.x * 0.5f, sprite.fScale.y * 0.5f, 0.0f };
                    ZVector3 vSpriteMin;
                    ZVector3 vSpriteMax;
                    vsub(vSpriteMin, sprite.p, vHalfSize);
                    vadd(vSpriteMax, sprite.p, vHalfSize);
                    vmin(vMin, vSpriteMin);
                    vmax(vMax, vSpriteMax);
                }
            }

            vsub(vSize, vMax, vMin);
            vadd(vCen, vMax, vMin);
            vCen.x *= m_v2Scale.x * 0.5f;
            vCen.y *= -m_v2Scale.y * 0.5f;
            vSize.x *= m_v2Scale.x * 0.5f;
            vSize.y *= m_v2Scale.y * 0.5f;
        }
        else
        {
            GetCen(vCen);
            GetSize(vSize);
            vCen.x *= m_v2Scale.x;
            vCen.y *= m_v2Scale.y;
            vSize.x = 0.0f;
            vSize.y = 0.0f;
        }

        SetCen(vCen);
        SetSize(vSize);
        SetRadius(vlen(vSize) + 1.0f);

        m_vAlignmentOffset = {};
        if (m_dwAlignment & 0xF)
        {
            float fX = -vCen.x;
            if (m_dwAlignment & 1)
                fX = BaseGeom()->m_vSize.x - vCen.x + 0.00012207031f;
            else if (m_dwAlignment & 2)
                fX -= BaseGeom()->m_vSize.x + 0.00012207031f;
            m_vAlignmentOffset.x = std::floor(fX);
        }
        if (m_dwAlignment & 0xF0)
        {
            float fY = -vCen.y;
            if (m_dwAlignment & 0x10)
                fY = BaseGeom()->m_vSize.y - vCen.y + 0.00012207031f;
            if (m_dwAlignment & 0x20)
                fY -= BaseGeom()->m_vSize.y + 0.00012207031f;
            m_vAlignmentOffset.y = std::floor(fY);
        }

        m_bModified = true;
    }

    void ZWINOBJ::SetMat(const ZMat3x3& mMat)
    {
        if (std::memcmp(&mMat, BaseGeom()->m_mMat.data, sizeof(mMat)) != 0)
        {
            ZGEOM::SetMat(mMat);
            m_bModified = true;
        }
    }

    void ZWINOBJ::SetPos(const ZVector3& vPos)
    {
        if (vPos != BaseGeom()->m_vPos)
        {
            ZGEOM::SetPos(vPos);
            m_bModified = true;
        }
    }

    void ZWINOBJ::Hide(bool bHide)
    {
        g_pEngineData->UnlockMinMax();
        ZGEOM::Hide(bHide);
        g_pEngineData->LockMinMax();
    }

    bool ZWINOBJ::WantDrawBufferControl() const
    {
        return true;
    }

    bool ZWINOBJ::DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace)
    {
        ZMat3x3 mMat;
        ZVector3 vPos;
        GetRootTM(mMat, vPos);
        pCameraSpace->GetLocalMatPos(&mMat, &vPos);
        Draw(pDrawBuffer, pCameraSpace, mMat, vPos, m_iPriority);
        return false;
    }

    void ZWINOBJ::ClassInit()
    {
        ZSTDOBJ::ClassInit();

        const ZMat3x3 mMat
        {
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        };
        SetMat(mMat);
        SetDrawing();
    }

    void ZWINOBJ::CopyData(const ZGEOM* pSource)
    {
        ZGEOM::CopyData(pSource);

        const ZWINOBJ* pWinSource = geom_cast<ZWINOBJ>(pSource);
        if (!pWinSource)
            return;

        RemoveGeometry();
        m_dwFaceColor = pWinSource->m_dwFaceColor;
        m_dwDrawMode = pWinSource->m_dwDrawMode;
        m_v2Scale = pWinSource->m_v2Scale;
        m_vAlignmentOffset = pWinSource->m_vAlignmentOffset;
        m_dwAlignment = pWinSource->m_dwAlignment;
        m_dwType = pWinSource->m_dwType;
        m_dwAmount = pWinSource->m_dwAmount;
        m_iPriority = pWinSource->m_iPriority;
        m_bStaticSprites = pWinSource->m_bStaticSprites;
        m_bAnimateAlpha = pWinSource->m_bAnimateAlpha;
        m_bScalePS2_512_448 = pWinSource->m_bScalePS2_512_448;

        m_iNumSprites = pWinSource->m_iNumSprites;
        m_pSprites = pWinSource->m_pSprites;
        if (!pWinSource->m_bStaticSprites && pWinSource->m_pSprites)
        {
            m_pSprites = g_pRenderDll->m_pPrimControl->AllocSpriteArrayUV(m_iNumSprites);
            std::memcpy(m_pSprites, pWinSource->m_pSprites, sizeof(SSpriteArrayElementUV) * m_iNumSprites);
        }

        if (pWinSource->m_pSpriteArrays)
        {
            m_iNumSpriteArrays = pWinSource->m_iNumSpriteArrays;
            m_pPrims = g_pRenderDll->m_pPrimControl->AllocPrimList(m_iNumSpriteArrays);
            m_pSpriteArrays = g_pRenderDll->m_pPrimControl->AllocSpriteArrays(m_iNumSpriteArrays);
            for (uint32_t i = 0; i < m_iNumSpriteArrays; ++i)
            {
                const SSpriteArray& sourceArray = pWinSource->m_pSpriteArrays[i];
                SSpriteArray& destArray = m_pSpriteArrays[i];
                destArray.lNumSprites = sourceArray.lNumSprites;
                destArray.pSpritesUV = m_pSprites + (sourceArray.pSpritesUV - pWinSource->m_pSprites);

                const auto* pSourcePrim = ZPrimControlBase::GetPrimitive<const SPrimSpritesArray>(pWinSource->m_pPrims[i]);
                ZASSERT(pSourcePrim->lType == 2);
                m_pPrims[i] = CreateSpriteArray(pSourcePrim->lTextureId, pSourcePrim->lDrawMode, SPRITETYPE(pSourcePrim->lSpriteType));
            }
        }

        m_bModified = true;
        m_vLastCamPos = { 1.0e38f, 1.0e38f, 1.0e38f };
        m_mCache.Reset();
        m_vCache = {};
        m_vCacheScale = {};
    }

    uint32_t ZWINOBJ::CreateSpriteArray(uint32_t lTextureId, uint32_t lDrawMode, SPRITETYPE lSpriteType)
    {
        ZASSERT((lTextureId & 0xFFFFF7FFu) != 0);

        REFTAB* pFreeSpriteArrays = ZWINOBJSPRITEHOLDER::GetFreeSpriteArraysList();
        if (pFreeSpriteArrays)
        {
            RefRun run;
            pFreeSpriteArrays->RunInitNxtRef(&run);
            while (run)
            {
                const uint32_t iPrimId = pFreeSpriteArrays->RunNxtRef(&run);
                auto* pPrim = ZPrimControlBase::GetPrimitive<SPrimSpritesArray>(iPrimId);
                if (pPrim->lDrawEntryId != 0)
                    continue;

                ZASSERT(pPrim->lType == 2);
                pPrim->lNextPrim = 0;
                pPrim->lType = 2;
                pPrim->lSpriteType = lSpriteType;
                pPrim->lTextureId = static_cast<uint16_t>(lTextureId);
                pPrim->lDrawMode = lDrawMode;
                pPrim->lPackType = 1;
                pPrim->lDrawEntryId = 0;
                pFreeSpriteArrays->RunDelRef(&run);
                return iPrimId;
            }
        }

        return g_pRenderDll->m_pPrimControl->CreateSpriteArray(lTextureId, lDrawMode, lSpriteType, false);
    }

    void ZWINOBJ::FreeSpriteArray(uint32_t iPrimId)
    {
        REFTAB* pFreeSpriteArrays = ZWINOBJSPRITEHOLDER::GetFreeSpriteArraysList();
        ZASSERT(pFreeSpriteArrays != nullptr);
        ZASSERT(!pFreeSpriteArrays->Find(iPrimId));
        pFreeSpriteArrays->Add(iPrimId);
    }

    void ZWINOBJ::DrawRaw(ZDrawBuffer* pDrawBuffer, const ZMat3x3& mMat, const ZVector3& vPos, const ZVector3&, float)
    {
        for (uint32_t i = 0; i < m_iNumSpriteArrays; ++i)
            pDrawBuffer->DrawSpriteArray(m_pPrims[i], &m_pSpriteArrays[i], mMat.data, vPos.Get(), m_iPriority, true, 0);
    }

    void ZWINOBJ::Draw(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace, const ZMat3x3& mMat, const ZVector3& vPos)
    {
        Draw(pDrawBuffer, pCameraSpace, mMat, vPos, m_iPriority);
    }

    void ZWINOBJ::Draw(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace, const ZMat3x3& mMat, const ZVector3& vPos, uint8_t lPriority)
    {
        if (!m_pSpriteArrays)
            return;

        if (m_vLastCamPos != vPos)
            m_bModified = true;

        if (m_bModified)
        {
            m_vLastCamPos = vPos;

            ZMat3x3 mCombined;
            mmtmul(mCombined, mMat, BaseGeom()->m_mMat);
            ZVector3 vAdjustedPos { vPos.x - 0.5f, vPos.y - 0.5f, vPos.z };
            CalcPosScale(pCameraSpace, mCombined, vAdjustedPos, m_vCache, m_vCacheScale, m_mCache, pDrawBuffer);
            vscalar(m_mCache.XAxis(), m_vCacheScale.x);
            vscalar(m_mCache.YAxis(), -g_pSysInterface->WindowFirst->PixelAspectXY() * m_vCacheScale.y);
            TransformRootVector(m_vCache, m_mCache);
            mmmul(m_mCache, BaseGeom()->m_mMat);
            m_vCache.y = -m_vCache.y;
            m_bModified = false;
        }

        ZASSERT(lPriority != 0);
        for (uint32_t i = 0; i < m_iNumSpriteArrays; ++i)
            pDrawBuffer->DrawSpriteArray(m_pPrims[i], &m_pSpriteArrays[i], m_mCache.data, m_vCache.Get(), lPriority, true, 0);
    }

    void ZWINOBJ::RecalcMaxMin()
    {
        BaseGeom()->SetControl(ZCHASMOVED, 0);
        BaseGeom()->ForceCalcMaxMin();

        ZVector3 vCen;
        GetCen(vCen);
        vCen.x += m_vAlignmentOffset.x;
        vCen.y += m_vAlignmentOffset.y;
        SetCen(vCen);

        ZGROUP* pParent = Parent();
        if (pParent && pParent->IsDerivedFrom<ZWINGROUP>())
            static_cast<ZWINGROUP*>(pParent)->RecalcMaxMin();

        vCen.x -= m_vAlignmentOffset.x;
        vCen.y -= m_vAlignmentOffset.y;
        SetCen(vCen);
    }

    void ZWINOBJ::GetMouseColi(SMouseColi& sColi, const ZVector3& vPos, const ZMat3x3& mMat)
    {
        if (m_dwType == 0x80)
            return;

        ZVector3 vInputPos;
        vadd(vInputPos, vPos, BaseGeom()->m_vPos);

        ZCameraSpace cameraSpace;
        cameraSpace = sColi.pCamera;
        ZVector3 vProjectedPos;
        ZVector3 vScale;
        ZMat3x3 mOutput;
        CalcPosScale(&cameraSpace, mMat, vInputPos, vProjectedPos, vScale, mOutput, nullptr);

        ZVector3 vCenter;
        vadd(vCenter, BaseGeom()->m_vCen, vProjectedPos);
        if (vCenter.x - BaseGeom()->m_vSize.x < 0.0f &&
            vCenter.x + BaseGeom()->m_vSize.x > 0.0f &&
            vCenter.y - BaseGeom()->m_vSize.y < 0.0f &&
            vCenter.y + BaseGeom()->m_vSize.y > 0.0f &&
            vProjectedPos.z < sColi.vColi.z)
        {
            sColi.rGeom = GetRef();
            sColi.vColi = vProjectedPos;
        }
    }

    void ZWINOBJ::RemoveGeometry()
    {
        m_bModified = false;
        if (!m_bStaticSprites)
        {
            if (m_pSprites)
                g_pRenderDll->m_pPrimControl->FreeSpriteArrayUV(m_pSprites, m_iNumSprites);
            if (m_pPrims)
            {
                for (uint32_t i = 0; i < m_iNumSpriteArrays; ++i)
                    FreeSpriteArray(m_pPrims[i]);
                g_pRenderDll->m_pPrimControl->FreePrimList(m_pPrims, m_iNumSpriteArrays);
            }
            if (m_pSpriteArrays)
                g_pRenderDll->m_pPrimControl->FreeSpriteArrays(m_pSpriteArrays, m_iNumSpriteArrays);

            m_pSprites = nullptr;
            m_iNumSprites = 0;
            m_iNumSpriteArrays = 0;
            m_pPrims = nullptr;
            m_pSpriteArrays = nullptr;
        }
    }

    void ZWINOBJ::LoadSaveGeometry(ISerializerStream&, bool bSaving)
    {
        ZASSERT(!m_bStaticSprites);
        if (!bSaving)
        {
            m_pSprites = nullptr;
            m_iNumSprites = 0;
            m_iNumSpriteArrays = 0;
            m_pPrims = nullptr;
            m_pSpriteArrays = nullptr;
        }
    }

    void ZWINOBJ::SetDrawMode(uint32_t mode, int pctval)
    {
        m_dwDrawMode = pctval == -1 ? mode : mode | 1;
        if (!m_pSpriteArrays)
            return;

        ZASSERT(m_pPrims != nullptr);
        for (uint32_t i = 0; i < m_iNumSpriteArrays; ++i)
        {
            auto* pPrim = ZPrimControlBase::GetPrimitive<SPrimSpritesArray>(m_pPrims[i]);
            if (!pPrim)
                continue;

            ZASSERT(pPrim->lType == 2);
            if (pPrim->lDrawMode & 0x10)
                m_dwDrawMode |= 0xC000;
            pPrim->lDrawMode = m_dwDrawMode;
        }
    }

    int32_t ZWINOBJ::CalcDrawMode(int dwDrawMode, int) const
    {
        switch (dwDrawMode)
        {
            case DRAWMODE_NORMAL: return 0x16C010;
            case DRAWMODE_ADD: return 0x16C012;
            case DRAWMODE_TRANS: return 0x16C011;
            case DRAWMODE_OPACITY: return 0x16C210;
            case DRAWMODE_HARDOPACITY: return 0x16C211;
            case DRAWMODE_SUB:
            case DRAWMODE_MUL:
                ZASSERT(false);
                return 0x16C010;
            default: return 0x16C010;
        }
    }

    void ZWINOBJ::SetDrawing()
    {
        SetDrawMode(GetDrawMode(), -1);
    }

    void ZWINOBJ::SetColor(uint32_t lColor)
    {
        SetColor(lColor, false);
    }

    void ZWINOBJ::SetColor(uint32_t lColor, bool bUseAlpha)
    {
        const uint32_t dwColor = bUseAlpha ? lColor : (lColor & 0xFFFFFFu) | (uint32_t(m_dwAmount) << 24);
        if (dwColor == m_dwFaceColor)
            return;

        m_dwFaceColor = dwColor;
        if (bUseAlpha)
            m_dwAmount = static_cast<uint8_t>(lColor >> 24);
        for (uint32_t i = 0; m_pSprites && i < m_iNumSprites; ++i)
            g_pRenderDll->m_pPrimControl->ColorUTOIU(&m_pSprites[i].lColor, &m_dwFaceColor);
    }

    void ZWINOBJ::SetColor(const ZVector3& vColor)
    {
        const uint32_t lColor = (uint32_t(vColor.x) << 16) | (uint32_t(vColor.y) << 8) | uint32_t(vColor.z);
        SetColor(lColor);
    }

    void ZWINOBJ::SetAlpha(uint8_t lAlpha)
    {
        SetDrawMode(lAlpha == 0xFF ? m_dwDrawMode & ~1u : m_dwDrawMode | 1u, -1);
        m_dwFaceColor = (m_dwFaceColor & 0xFFFFFFu) | (uint32_t(lAlpha) << 24);
        m_dwAmount = lAlpha;
        for (uint32_t i = 0; m_pSprites && i < m_iNumSprites; ++i)
            g_pRenderDll->m_pPrimControl->ColorUTOIU(&m_pSprites[i].lColor, &m_dwFaceColor);
    }

    void ZWINOBJ::SetScale(const Vector2& vScale, bool)
    {
        if (m_v2Scale != vScale)
        {
            m_v2Scale = vScale;
            m_bModified = true;
            CalcCenSize();
        }
    }

    void ZWINOBJ::SetAlignment(uint8_t lAlignment)
    {
        if (m_dwAlignment != lAlignment)
        {
            m_dwAlignment = lAlignment;
            CalcCenSize();
            m_bModified = true;
        }
    }

    uint8_t ZWINOBJ::GetAlignment() const
    {
        return m_dwAlignment;
    }

    void ZWINOBJ::SetPos(float x, float y, float z)
    {
        SetPos(ZVector3 { x, y, z });
    }

    void ZWINOBJ::SetType(uint8_t lType)
    {
        m_dwType = lType;
    }

    uint32_t ZWINOBJ::GetDrawMode() const
    {
        return m_dwDrawMode < 7 ? CalcDrawMode(m_dwDrawMode, m_dwAmount) : m_dwDrawMode;
    }

    void ZWINOBJ::SetPriority(uint8_t lPrio)
    {
        m_iPriority = lPrio;
    }

    void ZWINOBJ::GetType(EType&)
    {
        ZASSERT(false);
    }

    void ZWINOBJ::SetType(const EType& eType)
    {
        m_dwType = uint8_t(1u << eType);
    }

    void ZWINOBJ::GetDrawMode(EDrawMode& eDrawMode)
    {
        eDrawMode = static_cast<EDrawMode>(m_dwDrawMode);
    }

    void ZWINOBJ::SetDrawMode(const EDrawMode& eDrawMode)
    {
        m_dwDrawMode = eDrawMode;
    }

    void ZWINOBJ::GetAlignment(EAlignment&)
    {
        ZASSERT(false);
    }

    void ZWINOBJ::SetAlignment(const EAlignment& eAlignment)
    {
        static constexpr uint8_t aAlignment[] =
        {
            0x11, 0x12, 0x14, 0x21, 0x22, 0x24, 0x41, 0x42,
            0x44, 0x01, 0x02, 0x04, 0x10, 0x20, 0x40, 0x00
        };
        if (uint32_t(eAlignment) < STATIC_ARR_LEN(aAlignment))
            m_dwAlignment = aAlignment[eAlignment];
    }

    void ZWINOBJ::CalcPosScale(ZCameraSpace* pCameraSpace, const ZMat3x3& mMat, const ZVector3& vPos,
        ZVector3& vOutPos, ZVector3& vOutScale, ZMat3x3& mOutMat, ZDrawBuffer* pDrawBuffer)
    {
        mOutMat = mMat;
        vOutPos = vPos;

        ZVector4 vViewport;
        pCameraSpace->GetViewport(&vViewport);
        const int iSizeX = pDrawBuffer ? pDrawBuffer->GetSizeX() : g_pSysInterface->m_lResolution[0];
        const int iSizeY = pDrawBuffer ? pDrawBuffer->GetSizeY() : g_pSysInterface->m_lResolution[1];

        vOutPos.x -= float(iSizeX >> 1) * vViewport.z;
        vOutPos.y -= float(iSizeY >> 1) * vViewport.w;
        vOutPos.z += 6.0f;

        ZVector3 vAlignment { m_vAlignmentOffset.x, m_vAlignmentOffset.y, 0.0f };
        if (ZGROUP* pParent = Parent())
            pParent->GetRootVect(vAlignment);
        vOutPos.x += vAlignment.x;
        vOutPos.y += vAlignment.y;

        vOutPos.x = m_v2Scale.x == 0.0f ? 0.0f : vOutPos.x / m_v2Scale.x;
        vOutPos.y = m_v2Scale.y == 0.0f ? 0.0f : vOutPos.y / m_v2Scale.y;

        const float fViewportWidth = vViewport.z - vViewport.x;
        const float fViewportHeight = vViewport.w - vViewport.y;
        const float fPerspective = pCameraSpace->GetPers() / vOutPos.z;
        const float fViewAspect = (float(iSizeX) * fViewportWidth) / (float(iSizeY) * fViewportHeight);
        vOutScale.x = m_v2Scale.x / (float(iSizeX) * fPerspective * fViewportWidth);
        vOutScale.y = -m_v2Scale.y / (fViewAspect * float(iSizeY) * fPerspective * fViewportHeight);
        vOutScale.z = 1.0f;
    }

    void ZWINOBJ::SetModified(bool bModified)
    {
        m_bModified = bModified;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static ZEnumEntry TypeEntries[] =
        {
            { nullptr, ZWINOBJ::TYPE_Normal, "TYPE_Normal" },
            { &TypeEntries[0], ZWINOBJ::TYPE_Pushed, "TYPE_Pushed" },
            { &TypeEntries[1], ZWINOBJ::TYPE_Hover, "TYPE_Hover" },
            { &TypeEntries[2], ZWINOBJ::TYPE_Focused, "TYPE_Focused" },
            { &TypeEntries[3], ZWINOBJ::TYPE_Checked, "TYPE_Checked" },
            { &TypeEntries[4], ZWINOBJ::TYPE_Disabled, "TYPE_Disabled" },
            { &TypeEntries[5], ZWINOBJ::TYPE_Invisible, "TYPE_Invisible" },
            { &TypeEntries[6], ZWINOBJ::TYPE_Background, "TYPE_Background" }
        };
        static ZEnumInfo TypeInfo { &TypeEntries[7], "EType", sizeof(ZWINOBJ::EType) };

        static ZEnumEntry DrawModeEntries[] =
        {
            { nullptr, ZWINOBJ::DRAWMODE_NORMAL, "DRAWMODE_NORMAL" },
            { &DrawModeEntries[0], ZWINOBJ::DRAWMODE_ADD, "DRAWMODE_ADD" },
            { &DrawModeEntries[1], ZWINOBJ::DRAWMODE_SUB, "DRAWMODE_SUB" },
            { &DrawModeEntries[2], ZWINOBJ::DRAWMODE_TRANS, "DRAWMODE_TRANS" },
            { &DrawModeEntries[3], ZWINOBJ::DRAWMODE_MUL, "DRAWMODE_MUL" },
            { &DrawModeEntries[4], ZWINOBJ::DRAWMODE_OPACITY, "DRAWMODE_OPACITY" },
            { &DrawModeEntries[5], ZWINOBJ::DRAWMODE_HARDOPACITY, "DRAWMODE_HARDOPACITY" }
        };
        static ZEnumInfo DrawModeInfo { &DrawModeEntries[6], "EDrawMode", sizeof(ZWINOBJ::EDrawMode) };

        static ZEnumEntry AlignmentEntries[] =
        {
            { nullptr, ZWINOBJ::ALIGNMENT_Top_Left, "ALIGNMENT_Top_Left" },
            { &AlignmentEntries[0], ZWINOBJ::ALIGNMENT_Top_Right, "ALIGNMENT_Top_Right" },
            { &AlignmentEntries[1], ZWINOBJ::ALIGNMENT_Top_Center, "ALIGNMENT_Top_Center" },
            { &AlignmentEntries[2], ZWINOBJ::ALIGNMENT_Bottom_Left, "ALIGNMENT_Bottom_Left" },
            { &AlignmentEntries[3], ZWINOBJ::ALIGNMENT_Bottom_Right, "ALIGNMENT_Bottom_Right" },
            { &AlignmentEntries[4], ZWINOBJ::ALIGNMENT_Bottom_Center, "ALIGNMENT_Bottom_Center" },
            { &AlignmentEntries[5], ZWINOBJ::ALIGNMENT_Middle_Left, "ALIGNMENT_Middle_Left" },
            { &AlignmentEntries[6], ZWINOBJ::ALIGNMENT_Middle_Right, "ALIGNMENT_Middle_Right" },
            { &AlignmentEntries[7], ZWINOBJ::ALIGNMENT_Middle_Center, "ALIGNMENT_Middle_Center" },
            { &AlignmentEntries[8], ZWINOBJ::ALIGNMENT_Left, "ALIGNMENT_Left" },
            { &AlignmentEntries[9], ZWINOBJ::ALIGNMENT_Right, "ALIGNMENT_Right" },
            { &AlignmentEntries[10], ZWINOBJ::ALIGNMENT_Center, "ALIGNMENT_Center" },
            { &AlignmentEntries[11], ZWINOBJ::ALIGNMENT_Top, "ALIGNMENT_Top" },
            { &AlignmentEntries[12], ZWINOBJ::ALIGNMENT_Bottom, "ALIGNMENT_Bottom" },
            { &AlignmentEntries[13], ZWINOBJ::ALIGNMENT_Middle, "ALIGNMENT_Middle" },
            { &AlignmentEntries[14], ZWINOBJ::ALIGNMENT_None, "ALIGNMENT_None" }
        };
        static ZEnumInfo AlignmentInfo { &AlignmentEntries[15], "EAlignment", sizeof(ZWINOBJ::EAlignment) };

        static RTP::tVirtualTable<RTP::ZVirtualEnumProperty<ZWINOBJ::EType>> TypeTable
        {
            LoadVirtualEnum<ZWINOBJ::EType>, SaveVirtualEnum<ZWINOBJ::EType>
        };
        static RTP::tVirtualTable<RTP::ZVirtualEnumProperty<ZWINOBJ::EDrawMode>> DrawModeTable
        {
            LoadVirtualEnum<ZWINOBJ::EDrawMode>, SaveVirtualEnum<ZWINOBJ::EDrawMode>
        };
        static RTP::tVirtualTable<RTP::ZVirtualEnumProperty<ZWINOBJ::EAlignment>> AlignmentTable
        {
            LoadVirtualEnum<ZWINOBJ::EAlignment>, SaveVirtualEnum<ZWINOBJ::EAlignment>
        };

        static RTP::ZVirtualEnumProperty<ZWINOBJ::EType> Type
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "Type", .m_Filter = 1 },
            .m_VirtualTable = &TypeTable,
            .m_Get = &ZWINOBJ::GetType,
            .m_Set = &ZWINOBJ::SetType,
            .m_Info = &TypeInfo
        };

        static RTP::ZVirtualEnumProperty<ZWINOBJ::EDrawMode> DrawMode
        {
            .m_Node = { .m_Next = Type, .m_Name = "DrawMode", .m_Filter = 1 },
            .m_VirtualTable = &DrawModeTable,
            .m_Get = &ZWINOBJ::GetDrawMode,
            .m_Set = &ZWINOBJ::SetDrawMode,
            .m_Info = &DrawModeInfo
        };

        static RTP::ZVirtualEnumProperty<ZWINOBJ::EAlignment> Alignment
        {
            .m_Node = { .m_Next = DrawMode, .m_Name = "Alignment", .m_Filter = 1 },
            .m_VirtualTable = &AlignmentTable,
            .m_Get = &ZWINOBJ::GetAlignment,
            .m_Set = &ZWINOBJ::SetAlignment,
            .m_Info = &AlignmentInfo
        };

        static RTP::ZDataProperty<uint8_t> Priority
        {
            .m_Node = { .m_Next = Alignment, .m_Name = "m_iPriority", .m_Filter = 3 },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZWINOBJ, m_iPriority)
        };

        static RTP::ZDataProperty<uint8_t> Amount
        {
            .m_Node = { .m_Next = Priority, .m_Name = "m_dwAmount", .m_Filter = 3 },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZWINOBJ, m_dwAmount)
        };

        static RTP::ZDataProperty<uint8_t> TypeData
        {
            .m_Node = { .m_Next = Amount, .m_Name = "m_dwType", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZWINOBJ, m_dwType)
        };

        static RTP::ZDataProperty<uint8_t> AlignmentData
        {
            .m_Node = { .m_Next = TypeData, .m_Name = "m_dwAlignment", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZWINOBJ, m_dwAlignment)
        };

        static RTP::ZDataProperty<float[2]> AlignmentOffset
        {
            .m_Node = { .m_Next = AlignmentData, .m_Name = "m_vAlignmentOffset", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__76,
            .m_Offset = reinterpret_cast<float(*)[2]>(CLASS_PROPERTY(ZWINOBJ, m_vAlignmentOffset))
        };

        static RTP::ZDataProperty<float[2]> Scale
        {
            .m_Node = { .m_Next = AlignmentOffset, .m_Name = "m_v2Scale", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__76,
            .m_Offset = reinterpret_cast<float(*)[2]>(CLASS_PROPERTY(ZWINOBJ, m_v2Scale))
        };

        static RTP::ZDataProperty<uint32_t> DrawModeData
        {
            .m_Node = { .m_Next = Scale, .m_Name = "m_dwDrawMode", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZWINOBJ, m_dwDrawMode)
        };

        static RTP::ZDataProperty<uint32_t> FaceColor
        {
            .m_Node = { .m_Next = DrawModeData, .m_Name = "m_dwFaceColor", .m_Filter = 2 },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZWINOBJ, m_dwFaceColor)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZWINOBJ,
        ZSTDOBJ,
        0x009A2888,
        "ZWINOBJ",
        0x0077CF90,
        cProperties::FaceColor,
        0x00807574,
        0x009A27CC,
        0x009A27D0
    );
#   pragma endregion
}
