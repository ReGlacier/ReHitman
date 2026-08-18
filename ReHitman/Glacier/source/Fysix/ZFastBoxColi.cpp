#include <Glacier/Fysix/ZFastBoxColi.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/Fysix/ZCollisionBase.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ExGeomData.h>
#include <cstring>


namespace Glacier
{
    ZFastBoxColi::ZFastBoxColi(float fExtraSpace, int lGeomConMask)
    {
        m_fExtraSpace = fExtraSpace;
        m_lGeomConMask = lGeomConMask;
        m_bBoxSet = 0;
        m_bExtendedMode = 0;
        m_bIgnoreMovingObjects = 0;
        m_vCen = { 0.0f };
        m_vSize = { -1.f };
        m_pFaceList = ZUniMemory::New<REFTAB>(64, 30);
    }

    ZFastBoxColi::~ZFastBoxColi()
    {
        ZUniMemory::Delete(m_pFaceList);
    }

    void ZFastBoxColi::SetBox(ZVector3& vPos, ZMat3x3& mMat, ZVector3& vSize, bool bRefresh)
    {
        m_bBoxSet = true;

        bool bRebuild = bRefresh;

        if (!bRebuild)
        {
            // Check whether the new box (expressed in the old box local space) still fits into the old one
            ZMat3x3 mRel;
            ZVector3 vRelSize;
            ZVector3 vRelCen;
            ZVector3 vCenMin;
            ZVector3 vCenMax;

            mmtmul(mRel.data, mMat.data, m_m.data);
            vcpy(vRelSize.Get(), vSize.Get());
            TransformBox(mRel.data, vRelSize.Get());
            vsub(vRelCen.Get(), vPos.Get(), m_vCen.Get());
            vmtmul(vRelCen.Get(), mRel.data);
            vsub(vCenMin.Get(), vRelCen.Get(), vRelSize.Get());
            vadd(vCenMax.Get(), vRelCen.Get(), vRelSize.Get());

            for (int i = 0; i < 3; ++i)
            {
                if (-m_vSize.Get()[i] > vCenMin.Get()[i] || vCenMax.Get()[i] > m_vSize.Get()[i])
                {
                    bRebuild = true;
                    break;
                }
            }
        }

        if (bRebuild)
        {
            vcpy(m_vCen.Get(), vPos.Get());
            mcpy(m_m.data, mMat.data);
            vcpy(m_vSize.Get(), vSize.Get());

            const ZVector3 vExtraSpace(m_fExtraSpace, m_fExtraSpace, m_fExtraSpace);
            vadd(m_vSize.Get(), vExtraSpace.Get());

            Update();
        }
    }
    
    bool ZFastBoxColi::ChkLineColi(const ZVector3& v0, const ZVector3& v1, bool)
    {
        // TODO: This method has no implementation in any known build (XBOX_KL2/PC/iOS/PS2) — it was most likely stripped by the linker.
        return false;
    }
    
    void ZFastBoxColi::Update()
    {
        if (m_bBoxSet)
        {
            m_pFaceList->Clear();
            GetFaces();
        }
    }

    void ZFastBoxColi::EnableExtendedMode(bool value)
    {
        m_bExtendedMode = value;
    }
    
    void ZFastBoxColi::IgnoreMovingObjects(bool value)
    {
        m_bIgnoreMovingObjects = value;
    }

    REFTAB* ZFastBoxColi::GetFacesPtr()
    {
        return m_pFaceList;
    }
    
    uint32_t ZFastBoxColi::GetNumFaces()
    {
        return m_lNrTrisInAll;
    }
    
    void ZFastBoxColi::GetFaces()
    {
        uint32_t lNrStrips = 0;
        char aStripsBuffer[kMaxStripsInsideBox];

        // TODO: Finish this place after ZCollisionBase will be reversed.
        // Expected decompiled call (XBOX_KL2):
        // if (!ZCollisionBase::GetCollisionInterface(this)->GetStripsInsideBox(&lNrStrips, aStripsBuffer, sizeof(aStripsBuffer),
        //         m_m.data, m_vCen.Get(), m_vSize.Get(), m_lGeomConMask, true, true, GT_StdObjs))
        // {
        //     ZMSG_CH(chCollision, "ERROR: Stack buffer size %d too small for amount of strips within box!", sizeof(aStripsBuffer));
        // }

        m_lNrTrisInAll = 0;

        const char* pStripData = aStripsBuffer;

        for (uint32_t iStrip = 0; iStrip != lNrStrips; ++iStrip)
        {
            const uint32_t* pStripHeader = reinterpret_cast<const uint32_t*>(pStripData);

            ZBaseGeom* pBaseGeom = reinterpret_cast<ZBaseGeom*>(pStripHeader[4]);
            const float* pVerts = reinterpret_cast<const float*>(pStripData) + 6;

            uint32_t lNrVerts = pStripHeader[0];
            bool bIsFan = false;

            if ((lNrVerts & 0x80000000) != 0)
            {
                bIsFan = true;
                lNrVerts &= ~0x80000000;
            }

            ZASSERT(pBaseGeom->IsDerivedFrom<ZSTDOBJ>());

            const ZGEOM* pGeom = pBaseGeom->GetGeom();

            if (m_bIgnoreMovingObjects
                && pGeom && pGeom->m_pExData
                && (pGeom->m_pExData->_lControl & ZCEXMOVINGOBJ) != 0)
            {
                // skip moving objects
                pStripData = reinterpret_cast<const char*>(pVerts + (bIsFan ? 9 : 3) * lNrVerts);
                continue;
            }

            // transform vertices into box space
            float* pTransformedVerts = const_cast<float*>(pVerts);

            for (uint32_t iVert = (bIsFan ? 3 * lNrVerts : lNrVerts); iVert != 0; --iVert)
            {
                vmmul(pTransformedVerts, m_m.data);
                vadd(pTransformedVerts, m_vCen.Get());
                pTransformedVerts += 3;
            }

            if (bIsFan)
            {
                // triangle fan: (v0, v1, v2) per vertex triplet
                for (uint32_t iTri = 0; iTri != lNrVerts; ++iTri)
                {
                    if (!vcmp(pVerts, pVerts + 3) && !vcmp(pVerts, pVerts + 6) && !vcmp(pVerts + 3, pVerts + 6))
                    {
                        AddFace(pBaseGeom, pVerts, pVerts + 3, pVerts + 6);
                    }

                    pVerts += 9;
                }

                pStripData = reinterpret_cast<const char*>(pVerts);
            }
            else
            {
                // triangle strip: (v0, v1, v2), (v2, v1, v3), (v2, v3, v4), ...
                for (uint32_t iTri = 0; iTri != lNrVerts - 2; ++iTri)
                {
                    if (!vcmp(pVerts, pVerts + 3) && !vcmp(pVerts, pVerts + 6) && !vcmp(pVerts + 3, pVerts + 6))
                    {
                        if ((iTri & 1) != 0)
                        {
                            AddFace(pBaseGeom, pVerts + 6, pVerts + 3, pVerts);
                        }
                        else
                        {
                            AddFace(pBaseGeom, pVerts, pVerts + 3, pVerts + 6);
                        }
                    }

                    pVerts += 3;
                }

                pStripData = reinterpret_cast<const char*>(pVerts + 6);
            }
        }
    }

    void ZFastBoxColi::AddFace(const ZBaseGeom* pBaseGeom, const float* pVert0, const float* pVert1, const float* pVert2)
    {
        if (!TriangleAABBOverlap(m_vCen.Get(), m_vSize.Get(), m_m.data, pVert0, pVert1, pVert2))
        {
            return;
        }

        ZVector3 vNormal;
        CalcNormal(vNormal.Get(), pVert0, pVert1, pVert2);

        const ZREF geomRef = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);

        // build the invertible 3x3 system: [ edge0 | edge1 | normal ]
        float mSys[9];
        vsub(&mSys[0], pVert1, pVert0);
        vsub(&mSys[3], pVert2, pVert0);
        vcpy(&mSys[6], vNormal.Get());

        SFace face;

        if (!Invert3x3System(face.m_aInvSys, mSys))
        {
            return;
        }

        bool bFaceValid = true;

        face.m_GeomRef = geomRef;
        face.m_vNormal = vNormal;
        face.m_vVert0 = *reinterpret_cast<const ZVector3*>(pVert0);

        if (m_bExtendedMode)
        {
            face.m_vVert1 = *reinterpret_cast<const ZVector3*>(pVert1);
            face.m_vVert2 = *reinterpret_cast<const ZVector3*>(pVert2);

            float vEdge0[3];
            float vEdge1[3];

            vsub(vEdge0, pVert1, pVert0);
            vsub(vEdge1, pVert2, pVert0);
            vcross(face.m_vEdgeNormal1.Get(), vNormal.Get(), vEdge0); // cross(N, E0)
            vcross(face.m_vEdgeNormal2.Get(), vEdge1, vNormal.Get()); // cross(E1, N)
            vadd(face.m_vEdgeNormal0.Get(), face.m_vEdgeNormal1.Get(), face.m_vEdgeNormal2.Get());

            if (vlen2(face.m_vEdgeNormal1.Get()) <= 9.9999997e-10f)
            {
                bFaceValid = false;
            }
            else
            {
                vnorm(face.m_vEdgeNormal1.Get());
            }

            if (vlen2(face.m_vEdgeNormal2.Get()) <= 9.9999997e-10f)
            {
                bFaceValid = false;
            }
            else
            {
                vnorm(face.m_vEdgeNormal2.Get());
            }

            vscalar(face.m_vEdgeNormal0.Get(), -1.0f);

            if (vlen2(face.m_vEdgeNormal0.Get()) <= 9.9999997e-10f)
            {
                bFaceValid = false;
            }
            else
            {
                vnorm(face.m_vEdgeNormal0.Get());
            }
        }

        if (bFaceValid)
        {
            // Add() reserves the leading uint32_t of the element for the ref, our payload goes right after it
            uint32_t* pFaceData = m_pFaceList->Add(0);
            memcpy(pFaceData, &face, sizeof(SFace));
            ++m_lNrTrisInAll;
        }
    }
}
