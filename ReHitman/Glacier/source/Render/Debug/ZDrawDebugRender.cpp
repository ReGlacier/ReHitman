#include <Glacier/Render/Debug/ZDrawDebugRender.h>

#include <Glacier/Camera/ZCameraSpace.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/Render/ZTextureBase.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>
#include <cstdint>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr float kHalfPi = 1.5707964f;
        constexpr float kArcStep = 0.19634955f; // 2*pi / 32

        // Unit-cube corners in the order expected by the wire/filled box helpers.
        const float s_fCorners[8][3] =
        {
            { -1.0f, -1.0f, -1.0f },
            { -1.0f,  1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f },
            {  1.0f, -1.0f, -1.0f },
            { -1.0f, -1.0f,  1.0f },
            { -1.0f,  1.0f,  1.0f },
            {  1.0f,  1.0f,  1.0f },
            {  1.0f, -1.0f,  1.0f },
        };

        // 12 wireframe edges of the unit cube.
        const int16_t s_lEdges[12][2] =
        {
            { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 3 },
            { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
            { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
        };

        // 12 triangles (36 indices) of the unit cube.
        const int16_t s_lBoxTriangles[12][3] =
        {
            { 0, 3, 2 }, { 2, 1, 0 }, { 5, 6, 4 }, { 7, 4, 6 },
            { 2, 5, 1 }, { 2, 6, 5 }, { 4, 7, 3 }, { 4, 3, 0 },
            { 7, 2, 3 }, { 7, 6, 2 }, { 1, 4, 0 }, { 1, 5, 4 },
        };

        // 21 line segments of the debug arrow gizmo (Xbox_KL1 fVertices, scaled by 0.5 at use).
        const float s_fArrow[21][2][3] =
        {
            { {  0.0f,  1.0f,  2.0f }, {  2.0f,  1.0f,  0.0f } },
            { {  2.0f,  1.0f,  0.0f }, {  1.0f,  1.0f,  0.0f } },
            { {  1.0f,  1.0f,  0.0f }, {  1.0f,  1.0f, -2.0f } },
            { {  1.0f,  1.0f, -2.0f }, { -1.0f,  1.0f, -2.0f } },
            { { -1.0f,  1.0f, -2.0f }, { -1.0f,  1.0f,  0.0f } },
            { { -1.0f,  1.0f,  0.0f }, { -2.0f,  1.0f,  0.0f } },
            { { -2.0f,  1.0f,  0.0f }, {  0.0f,  1.0f,  2.0f } },
            { {  0.0f, -1.0f,  2.0f }, {  2.0f, -1.0f,  0.0f } },
            { {  2.0f, -1.0f,  0.0f }, {  1.0f, -1.0f,  0.0f } },
            { {  1.0f, -1.0f,  0.0f }, {  1.0f, -1.0f, -2.0f } },
            { {  1.0f, -1.0f, -2.0f }, { -1.0f, -1.0f, -2.0f } },
            { { -1.0f, -1.0f, -2.0f }, { -1.0f, -1.0f,  0.0f } },
            { { -1.0f, -1.0f,  0.0f }, { -2.0f, -1.0f,  0.0f } },
            { { -2.0f, -1.0f,  0.0f }, {  0.0f, -1.0f,  2.0f } },
            { {  0.0f, -1.0f,  2.0f }, {  0.0f,  1.0f,  2.0f } },
            { {  2.0f, -1.0f,  0.0f }, {  2.0f,  1.0f,  0.0f } },
            { {  1.0f, -1.0f,  0.0f }, {  1.0f,  1.0f,  0.0f } },
            { {  1.0f, -1.0f, -2.0f }, {  1.0f,  1.0f, -2.0f } },
            { { -1.0f, -1.0f, -2.0f }, { -1.0f,  1.0f, -2.0f } },
            { { -1.0f, -1.0f,  0.0f }, { -1.0f,  1.0f,  0.0f } },
            { { -2.0f, -1.0f,  0.0f }, { -2.0f,  1.0f,  0.0f } },
        };

        // 8 line segments of the debug pyramid: apex -> base, then the base ring.
        const float s_fPyramid[8][2][3] =
        {
            { {  0.0f,  0.0f,  0.0f }, {  1.0f,  1.0f,  1.0f } },
            { {  0.0f,  0.0f,  0.0f }, { -1.0f,  1.0f,  1.0f } },
            { {  0.0f,  0.0f,  0.0f }, { -1.0f, -1.0f,  1.0f } },
            { {  0.0f,  0.0f,  0.0f }, {  1.0f, -1.0f,  1.0f } },
            { {  1.0f,  1.0f,  1.0f }, {  1.0f, -1.0f,  1.0f } },
            { {  1.0f, -1.0f,  1.0f }, { -1.0f, -1.0f,  1.0f } },
            { { -1.0f, -1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f } },
            { { -1.0f,  1.0f,  1.0f }, {  1.0f,  1.0f,  1.0f } },
        };
    }

    ZDrawDebugRender::ZDrawDebugRender()
    {
        m_pIView = nullptr;
        m_bInside_xBegin = false;
        m_lVertexCount = 0;
        m_lCurrentDrawMode = 0;
        m_lCurrentTextureId = 0;
        m_lWantedDrawMode = 0;
        m_lWantedTextureId = 0;
        m_bWantedTextureIsPtr = false;
        m_fFontSize.x = 9.0f;
        m_fFontSize.y = 14.0f;
    }

    ZDrawDebugRender::~ZDrawDebugRender() = default;

    void ZDrawDebugRender::DrawLines(const ZDrawDebugRender::SVertex*, uint32_t)
    {
        // Do nothing (implemented by the platform renderer)
    }

    void ZDrawDebugRender::DrawTriangles(const ZDrawDebugRender::SVertex*, uint32_t)
    {
        // Do nothing (implemented by the platform renderer)
    }

    void ZDrawDebugRender::SetDrawMode(uint32_t)
    {
        // Do nothing (implemented by the platform renderer)
    }

    void ZDrawDebugRender::SetTexture(ZTextureBase*)
    {
        // Do nothing (implemented by the platform renderer)
    }

    void ZDrawDebugRender::SetTexture(uint32_t)
    {
        // Do nothing (implemented by the platform renderer)
    }

    void ZDrawDebugRender::SetViewport(const ZDrawDebugRender::SViewport& viewport)
    {
        m_Viewport = viewport;
    }

    void ZDrawDebugRender::SetOrtho(bool bOrtho, float fScale)
    {
        ZASSERT(!std::isnan(fScale) && std::isfinite(fScale));

        m_fOrthoScale = fScale;
        m_bOrtho = bOrtho;
    }

    void ZDrawDebugRender::Begin(IView* pIView, ZCameraSpace* pCameraSpace)
    {
        m_pIView = pIView;

        if (pCameraSpace)
        {
            bool bOrtho = false;
            float fScale = 1.0f;

            if (pCameraSpace->IsOrthogonal())
            {
                bOrtho = true;
                fScale = pCameraSpace->m_fScaleX;
            }

            SetOrtho(bOrtho, fScale);

            const uint32_t* pViewport = pIView->Viewport();
            SViewport viewport;
            viewport.x = pViewport[0];
            viewport.y = pViewport[1];
            viewport.w = pViewport[2] - pViewport[0];
            viewport.h = pViewport[3] - pViewport[1];
            SetViewport(viewport);

            m_ViewToWorld = pCameraSpace->m_CameraToRoot;
            tmat(m_WorldToView.m0.data, m_ViewToWorld.m0.data);

            ZVector3 p;
            vmtmul(p.Get(), m_ViewToWorld.p0.Get(), m_ViewToWorld.m0.Get());
            m_WorldToView.p0 = p * -1.0f;
        }
        else
        {
            SetOrtho(false, 1.0f);

            mreset(m_ViewToWorld.m0.data);
            m_ViewToWorld.p0.Reset();

            mreset(m_WorldToView.m0.data);
            m_WorldToView.p0.Reset();
        }
    }

    void ZDrawDebugRender::End()
    {
        xFlush();
    }

    void ZDrawDebugRender::xBegin(ZDrawDebugRender::PRIMTYPE Type)
    {
        ZASSERT(!m_bInside_xBegin);

        bool bFlush = (Type != m_CurrentType)
            || (m_lCurrentDrawMode != m_lWantedDrawMode)
            || (m_lCurrentTextureId != m_lWantedTextureId);

        m_bInside_xBegin = true;

        if (bFlush)
            xFlush();

        m_CurrentType = Type;
    }

    void ZDrawDebugRender::xEnd()
    {
        ZASSERT(m_bInside_xBegin);

        m_bInside_xBegin = false;

        ZASSERT(xCanFlush());
    }

    void ZDrawDebugRender::xReset()
    {
        m_lWantedDrawMode = 0;
        xSetTexture(0u);
        xFlush();
    }

    void ZDrawDebugRender::xColor(uint32_t c)
    {
        if ((c & 0xFF000000) != 0)
            m_CurrentVertex.c = c;
        else
            m_CurrentVertex.c = c | 0xFF000000;
    }

    void ZDrawDebugRender::xTexCoords3v(const float* v)
    {
        m_CurrentVertex.t.x = v[0];
        m_CurrentVertex.t.y = v[1];
        m_CurrentVertex.t.z = v[2];
    }

    void ZDrawDebugRender::xTexCoords2s(float s, float t)
    {
        m_CurrentVertex.t.x = s;
        m_CurrentVertex.t.y = t;
        m_CurrentVertex.t.z = 0.0f;
    }

    void ZDrawDebugRender::xTexCoords2v(float s, float t)
    {
        m_CurrentVertex.t.x = s;
        m_CurrentVertex.t.y = t;
        m_CurrentVertex.t.z = 0.0f;
    }

    void ZDrawDebugRender::xSetDrawMode(uint32_t lDrawMode)
    {
        m_lWantedDrawMode = lDrawMode;
    }

    void ZDrawDebugRender::xSetTexture(uint32_t lTextureId)
    {
        ZASSERT(!m_bInside_xBegin);

        m_lWantedTextureId = lTextureId;
        m_bWantedTextureIsPtr = false;
    }

    void ZDrawDebugRender::xSetTexture(ZTextureBase* pTexture)
    {
        ZASSERT(!m_bInside_xBegin);

        m_lWantedTextureId = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pTexture));
        m_bWantedTextureIsPtr = true;
    }

    bool ZDrawDebugRender::xCanFlush() const
    {
        if (m_CurrentType == PT_LINES)
            return (m_lVertexCount & 1) == 0;

        if (m_CurrentType == PT_TRIANGLES)
            return (m_lVertexCount % 3) == 0;

        ZASSERT(false);
        return true;
    }

    void ZDrawDebugRender::xFlush()
    {
        if (m_lVertexCount)
        {
            ZASSERT(xCanFlush());

            if (m_CurrentType == PT_LINES)
                DrawLines(m_Vertices, m_lVertexCount >> 1);
            else if (m_CurrentType == PT_TRIANGLES)
                DrawTriangles(m_Vertices, m_lVertexCount / 3);
            else
                ZASSERT(false);

            m_lVertexCount = 0;
        }

        if (m_lCurrentDrawMode != m_lWantedDrawMode)
        {
            m_lCurrentDrawMode = m_lWantedDrawMode;
            SetDrawMode(m_lCurrentDrawMode);
        }

        if (m_lCurrentTextureId != m_lWantedTextureId)
        {
            m_lCurrentTextureId = m_lWantedTextureId;

            if (m_bWantedTextureIsPtr)
                SetTexture(reinterpret_cast<ZTextureBase*>(static_cast<uintptr_t>(m_lWantedTextureId)));
            else
                SetTexture(m_lWantedTextureId);
        }
    }

    void ZDrawDebugRender::xVertex3(float x, float y, float z)
    {
        ZASSERT(!std::isnan(x) && std::isfinite(x));
        ZASSERT(!std::isnan(y) && std::isfinite(y));
        ZASSERT(!std::isnan(z) && std::isfinite(z));
        ZASSERT(m_bInside_xBegin);

        if (m_lVertexCount > 0x1FC && xCanFlush())
            xFlush();

        m_CurrentVertex.p.x = x;
        m_CurrentVertex.p.y = y;
        m_CurrentVertex.p.z = z;

        m_Vertices[m_lVertexCount] = m_CurrentVertex;
        ++m_lVertexCount;
    }

    void ZDrawDebugRender::xVertex3v(const float* v)
    {
        xVertex3(v[0], v[1], v[2]);
    }

    void ZDrawDebugRender::xVertex2(float x, float y)
    {
        xVertex3(x, y, 0.0f);
    }

    void ZDrawDebugRender::xVertex2v(const float* v)
    {
        xVertex3(v[0], v[1], 0.0f);
    }

    void ZDrawDebugRender::Transform(float* v, const float* m, const float* p, const float* s) const
    {
        v[0] *= s[0];
        v[1] *= s[1];
        v[2] *= s[2];

        vmmul(v, m);

        v[0] += p[0];
        v[1] += p[1];
        v[2] += p[2];
    }

    void ZDrawDebugRender::TransformWorldToView(float* vOut, const float* vIn) const
    {
        vmmul(vOut, vIn, m_WorldToView.m0.data);
        vOut[0] += m_WorldToView.p0.x;
        vOut[1] += m_WorldToView.p0.y;
        vOut[2] += m_WorldToView.p0.z;
    }

    void ZDrawDebugRender::TransformViewToWorld(float* vOut, const float* vIn) const
    {
        vsub(vOut, vIn, m_WorldToView.p0.Get());
        vmtmul(vOut, m_WorldToView.m0.Get());
    }

    void ZDrawDebugRender::DrawLine(const ZVector3& vStart, const ZVector3& vEnd, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);
        xVertex3v(vStart.Get());
        xVertex3v(vEnd.Get());
        xEnd();
    }

    void ZDrawDebugRender::DrawTriangle(const ZVector3& v0, const ZVector3& v1, const ZVector3& v2, uint32_t lColor)
    {
        xBegin(PT_TRIANGLES);
        xColor(lColor);
        xVertex3v(v0.Get());
        xVertex3v(v1.Get());
        xVertex3v(v2.Get());
        xEnd();
    }

    void ZDrawDebugRender::DrawCircle(const ZMat3x3& m, const ZVector3& v, float fRadius, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);

        const float s[3] = { fRadius, fRadius, fRadius };
        float fAngle = 0.0f;

        for (int i = 0; i < 32; ++i)
        {
            float a[3] = { std::sin(fAngle), std::cos(fAngle), 0.0f };
            float b[3] = { std::sin(fAngle + kArcStep), std::cos(fAngle + kArcStep), 0.0f };

            Transform(a, m.data, v.Get(), s);
            Transform(b, m.data, v.Get(), s);

            xVertex3v(a);
            xVertex3v(b);

            fAngle += kArcStep;
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawSphere3(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);

        float fAngle = 0.0f;

        for (int i = 0; i < 32; ++i)
        {
            const float sinA = std::sin(fAngle);
            const float cosA = std::cos(fAngle);
            fAngle += kArcStep;
            const float sinB = std::sin(fAngle);
            const float cosB = std::cos(fAngle);

            float a[3] = { sinA, cosA, 0.0f };
            float b[3] = { sinB, cosB, 0.0f };
            Transform(a, m.data, v.Get(), s.Get());
            Transform(b, m.data, v.Get(), s.Get());
            xVertex3v(a);
            xVertex3v(b);

            float c[3] = { sinA, 0.0f, cosA };
            float d[3] = { sinB, 0.0f, cosB };
            Transform(c, m.data, v.Get(), s.Get());
            Transform(d, m.data, v.Get(), s.Get());
            xVertex3v(c);
            xVertex3v(d);

            float e[3] = { 0.0f, sinA, cosA };
            float f[3] = { 0.0f, sinB, cosB };
            Transform(e, m.data, v.Get(), s.Get());
            Transform(f, m.data, v.Get(), s.Get());
            xVertex3v(e);
            xVertex3v(f);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawSphereArcs(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);

        float fAngle = -kHalfPi;

        for (int i = 0; i < 16; ++i)
        {
            const float sinA = std::sin(fAngle);
            const float cosA = std::cos(fAngle);
            fAngle += kArcStep;
            const float sinB = std::sin(fAngle);
            const float cosB = std::cos(fAngle);

            float a[3] = { sinA, 0.0f, cosA };
            float b[3] = { sinB, 0.0f, cosB };
            Transform(a, m.data, v.Get(), s.Get());
            Transform(b, m.data, v.Get(), s.Get());
            xVertex3v(a);
            xVertex3v(b);

            float c[3] = { 0.0f, sinA, cosA };
            float d[3] = { 0.0f, sinB, cosB };
            Transform(c, m.data, v.Get(), s.Get());
            Transform(d, m.data, v.Get(), s.Get());
            xVertex3v(c);
            xVertex3v(d);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawBoxWire(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        float aCorners[8][3];

        for (int i = 0; i < 8; ++i)
        {
            aCorners[i][0] = s_fCorners[i][0];
            aCorners[i][1] = s_fCorners[i][1];
            aCorners[i][2] = s_fCorners[i][2];
            Transform(aCorners[i], m.data, v.Get(), s.Get());
        }

        xBegin(PT_LINES);
        xColor(lColor);

        for (int i = 0; i < 12; ++i)
        {
            xVertex3v(aCorners[s_lEdges[i][0]]);
            xVertex3v(aCorners[s_lEdges[i][1]]);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawBoxFilled(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        float aCorners[8][3];

        for (int i = 0; i < 8; ++i)
        {
            aCorners[i][0] = s_fCorners[i][0];
            aCorners[i][1] = s_fCorners[i][1];
            aCorners[i][2] = s_fCorners[i][2];
            Transform(aCorners[i], m.data, v.Get(), s.Get());
        }

        xBegin(PT_TRIANGLES);
        xColor(lColor);

        for (int i = 0; i < 12; ++i)
        {
            xVertex3v(aCorners[s_lBoxTriangles[i][0]]);
            xVertex3v(aCorners[s_lBoxTriangles[i][1]]);
            xVertex3v(aCorners[s_lBoxTriangles[i][2]]);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawGrid(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lDivisionsX, uint32_t lDivisionsZ, uint32_t lColor)
    {
        const float fDivX = static_cast<float>(lDivisionsX);
        const float fDivZ = static_cast<float>(lDivisionsZ);
        const float fInvX = 1.0f / fDivX;
        const float fInvZ = 1.0f / fDivZ;
        const float fHalfX = (fDivX * fInvX) * 0.5f;
        const float fHalfZ = (fDivZ * fInvZ) * 0.5f;

        xBegin(PT_LINES);
        xColor(lColor);

        float x = -fHalfX;
        for (uint32_t i = 0; i <= lDivisionsX; ++i)
        {
            float p1[3] = { x, 0.0f, -fHalfZ };
            float p2[3] = { x, 0.0f,  fHalfZ };
            Transform(p1, m.data, v.Get(), s.Get());
            Transform(p2, m.data, v.Get(), s.Get());
            xVertex3v(p1);
            xVertex3v(p2);
            x += fInvX;
        }

        float z = -fHalfZ;
        for (uint32_t i = 0; i <= lDivisionsZ; ++i)
        {
            float p1[3] = { -fHalfX, 0.0f, z };
            float p2[3] = {  fHalfX, 0.0f, z };
            Transform(p1, m.data, v.Get(), s.Get());
            Transform(p2, m.data, v.Get(), s.Get());
            xVertex3v(p1);
            xVertex3v(p2);
            z += fInvZ;
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawPyramid(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);

        for (int i = 0; i < 8; ++i)
        {
            float p1[3] = { s_fPyramid[i][0][0], s_fPyramid[i][0][1], s_fPyramid[i][0][2] };
            float p2[3] = { s_fPyramid[i][1][0], s_fPyramid[i][1][1], s_fPyramid[i][1][2] };
            Transform(p1, m.data, v.Get(), s.Get());
            Transform(p2, m.data, v.Get(), s.Get());
            xVertex3v(p1);
            xVertex3v(p2);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawCappedPyramid(const ZMat3x3& m, const ZVector3& v, float fAngleRad, float fLength, float fAspect, uint32_t lColor)
    {
        ZVector3 scale;
        scale.x = fAngleRad * fLength;
        scale.y = (fAngleRad / fAspect) * fLength;
        scale.z = fLength;

        DrawPyramid(m, v, scale, lColor);
    }

    void ZDrawDebugRender::DrawArrow(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);

        for (int i = 0; i < 21; ++i)
        {
            float p1[3] = { s_fArrow[i][0][0] * 0.5f, s_fArrow[i][0][1] * 0.5f, s_fArrow[i][0][2] * 0.5f };
            float p2[3] = { s_fArrow[i][1][0] * 0.5f, s_fArrow[i][1][1] * 0.5f, s_fArrow[i][1][2] * 0.5f };
            Transform(p1, m.data, v.Get(), s.Get());
            Transform(p2, m.data, v.Get(), s.Get());
            xVertex3v(p1);
            xVertex3v(p2);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawCone(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        xBegin(PT_LINES);
        xColor(lColor);

        float fAngle = 0.0f;
        for (int i = 0; i < 32; ++i)
        {
            float a[3] = { std::sin(fAngle), std::cos(fAngle), 1.0f };
            float b[3] = { std::sin(fAngle + kArcStep), std::cos(fAngle + kArcStep), 1.0f };
            Transform(a, m.data, v.Get(), s.Get());
            Transform(b, m.data, v.Get(), s.Get());
            xVertex3v(a);
            xVertex3v(b);
            fAngle += kArcStep;
        }

        const float aApex[3] = { 0.0f, 0.0f, 0.0f };
        const float aTips[4][3] =
        {
            { 0.0f,  1.0f, 1.0f },
            { 0.0f, -1.0f, 1.0f },
            { 1.0f,  0.0f, 1.0f },
            { -1.0f, 0.0f, 1.0f },
        };

        for (int i = 0; i < 4; ++i)
        {
            float apex[3] = { aApex[0], aApex[1], aApex[2] };
            float tip[3] = { aTips[i][0], aTips[i][1], aTips[i][2] };
            Transform(apex, m.data, v.Get(), s.Get());
            Transform(tip, m.data, v.Get(), s.Get());
            xVertex3v(apex);
            xVertex3v(tip);
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawConeSolid(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor)
    {
        xBegin(PT_TRIANGLES);
        xColor(lColor);

        float fAngle = 0.0f;
        for (int i = 0; i < 32; ++i)
        {
            float a[3] = { std::sin(fAngle), std::cos(fAngle), 1.0f };
            float b[3] = { std::sin(fAngle + kArcStep), std::cos(fAngle + kArcStep), 1.0f };
            float apex[3] = { 0.0f, 0.0f, 0.0f };
            Transform(b, m.data, v.Get(), s.Get());
            Transform(a, m.data, v.Get(), s.Get());
            Transform(apex, m.data, v.Get(), s.Get());
            xVertex3v(b);
            xVertex3v(a);
            xVertex3v(apex);
            fAngle += kArcStep;
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawCylinder(const ZMat3x3& m, const ZVector3& v, float fHeight, float fRadius, uint32_t lColor)
    {
        const float fHalfHeight = fHeight * 0.5f;
        const float s[3] = { fRadius, 1.0f, fRadius };
        const float aCenters[2][3] =
        {
            { v.x, v.y + fHalfHeight, v.z },
            { v.x, v.y - fHalfHeight, v.z },
        };

        xBegin(PT_LINES);
        xColor(lColor);

        float fAngle = 0.0f;
        for (int i = 0; i < 32; ++i)
        {
            float aDir[3] = { std::sin(fAngle), 1.0f, std::cos(fAngle) };
            float bDir[3] = { std::sin(fAngle + kArcStep), 1.0f, std::cos(fAngle + kArcStep) };

            float a0[3] = { aDir[0], aDir[1], aDir[2] };
            float b0[3] = { bDir[0], bDir[1], bDir[2] };
            Transform(a0, m.data, aCenters[0], s);
            Transform(b0, m.data, aCenters[0], s);
            xVertex3v(a0);
            xVertex3v(b0);

            float a1[3] = { aDir[0], -aDir[1], aDir[2] };
            float b1[3] = { bDir[0], -bDir[1], bDir[2] };
            Transform(a1, m.data, aCenters[1], s);
            Transform(b1, m.data, aCenters[1], s);
            xVertex3v(a1);
            xVertex3v(b1);

            if ((i & 7) == 0)
            {
                float side0[3] = { aDir[0],  aDir[1], aDir[2] };
                float side1[3] = { aDir[0], -aDir[1], aDir[2] };
                Transform(side0, m.data, aCenters[0], s);
                Transform(side1, m.data, aCenters[1], s);
                xVertex3v(side0);
                xVertex3v(side1);
            }

            fAngle += kArcStep;
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawCappedCone(const ZMat3x3& m, const ZVector3& v, float fAngleRad, float fLength, uint32_t lColor)
    {
        ZVector3 scale;
        scale.x = std::sin(fAngleRad) * fLength;
        scale.y = scale.x;
        scale.z = std::cos(fAngleRad) * fLength;

        DrawCone(m, v, scale, lColor);

        ZVector3 capPos;
        ZVector3 capScale;

        if (fAngleRad <= kHalfPi)
        {
            const float fAlong = std::cos(fAngleRad) * fLength;
            capPos.x = v.x + m.data[0] * fAlong;
            capPos.y = v.y + m.data[1] * fAlong;
            capPos.z = v.z + m.data[2] * fAlong;
            capScale.x = fLength;
            capScale.y = fLength;
            capScale.z = fLength - std::cos(fAngleRad) * fLength;
        }
        else
        {
            capPos = v;
            capScale.x = fLength;
            capScale.y = fLength;
            capScale.z = fLength;
        }

        DrawSphereArcs(m, capPos, capScale, lColor);
    }

    void ZDrawDebugRender::DrawAxis(const ZVector3& vStart, const ZVector3& vDir, float fScale, uint32_t lColor)
    {
        ZVector3 vEnd;
        vEnd.x = vStart.x + vDir.x * (fScale * 0.7f);
        vEnd.y = vStart.y + vDir.y * (fScale * 0.7f);
        vEnd.z = vStart.z + vDir.z * (fScale * 0.7f);
        DrawLine(vStart, vEnd, lColor);

        ZVector3 vTip;
        vTip.x = vStart.x + vDir.x * fScale;
        vTip.y = vStart.y + vDir.y * fScale;
        vTip.z = vStart.z + vDir.z * fScale;

        ZVector3 scale;
        scale.x = fScale * 0.05f;
        scale.y = scale.x;
        scale.z = fScale * 0.3f;

        ZVector3 vNegDir;
        vNegDir.x = -vDir.x;
        vNegDir.y = -vDir.y;
        vNegDir.z = -vDir.z;

        ZMat3x3 mat;
        createmat(mat.data, vNegDir.Get(), nullptr);
        DrawConeSolid(mat, vTip, scale, lColor);
    }

    void ZDrawDebugRender::DrawMatrix(ZMat3x3& m, const ZVector3& v, int8_t lAxisMask)
    {
        float fScale = 1.0f;
        float vEnd[3];

        TransformWorldToView(vEnd, v.Get());

        if (m_bOrtho)
            fScale = 1.0f / (m_fOrthoScale * 15.0f);
        else
            vsetlen(vEnd, 10.0f);

        TransformViewToWorld(vEnd, vEnd);

        const uint32_t lXColor = (lAxisMask & 1) ? 0xFFFF0000u : 0xFF7F0000u;
        const uint32_t lYColor = (lAxisMask & 2) ? 0xFF00FF00u : 0xFF007F00u;
        const uint32_t lZColor = (lAxisMask & 4) ? 0xFF0000FFu : 0xFF00007Fu;

        const ZVector3 vEndPos(vEnd);

        ZVector3 axis;
        axis.x = m.data[6];
        axis.y = m.data[7];
        axis.z = m.data[8];
        DrawAxis(vEndPos, axis, fScale, lXColor);

        axis.x = m.data[3];
        axis.y = m.data[4];
        axis.z = m.data[5];
        DrawAxis(vEndPos, axis, fScale, lYColor);

        axis.x = m.data[0];
        axis.y = m.data[1];
        axis.z = m.data[2];
        DrawAxis(vEndPos, axis, fScale, lZColor);
    }

    void ZDrawDebugRender::DrawArc(const ZMat3x3& m, const ZVector3& v, const float* p1, const float* p2, const float* s, uint32_t lColor)
    {
        ZVector3 vA(p1);
        ZVector3 vB(p2);
        vnorm(vA.Get());
        vnorm(vB.Get());

        float fDot = vdot(vA.Get(), vB.Get());
        if (fDot > 1.0f)
            fDot = 1.0f;
        if (fDot < -1.0f)
            fDot = -1.0f;

        const float fAngle = std::acos(fDot);

        float vAxis[3];
        vcross(vAxis, vA.Get(), vB.Get());
        if (vnorm(vAxis) < 0.00012207031f)
        {
            vAxis[0] = 0.0f;
            vAxis[1] = 1.0f;
            vAxis[2] = 0.0f;
        }

        const float fStepDeg = (fAngle * 57.295776f) / 20.0f;

        xBegin(PT_LINES);
        xColor(lColor);

        float vPrev[3] = { p1[0], p1[1], p1[2] };
        Transform(vPrev, m.data, v.Get(), s);

        for (int i = 0; i < 20; ++i)
        {
            float rot[9];
            mrotaxis(rot, fStepDeg * static_cast<float>(i + 1), vAxis[0], vAxis[1], vAxis[2]);

            float vCur[3] = { p1[0], p1[1], p1[2] };
            vmmul(vCur, rot);
            Transform(vCur, m.data, v.Get(), s);

            xVertex3v(vPrev);
            xVertex3v(vCur);

            vPrev[0] = vCur[0];
            vPrev[1] = vCur[1];
            vPrev[2] = vCur[2];
        }

        xEnd();
    }

    void ZDrawDebugRender::DrawCapsule(const ZVector3& p0, const ZVector3& p1, float fRadius, uint32_t lColor)
    {
        ZVector3 vDir = p1 - p0;
        const float fLen = vDir.Length();
        ZASSERT(fLen >= 0.00024414062f);

        ZVector3 vAxis;
        if (fLen >= 0.00024414062f)
        {
            vAxis.x = vDir.x / fLen;
            vAxis.y = vDir.y / fLen;
            vAxis.z = vDir.z / fLen;
        }
        else
        {
            vAxis = ZVector3(0.0f, 1.0f, 0.0f);
        }

        const ZVector3 vScale(fRadius, fRadius, fRadius);

        float mat[9];
        createmat(mat, vAxis.Get(), nullptr);

        const ZMat3x3 m(mat);
        DrawSphereArcs(m, p0, vScale, lColor);
        DrawSphereArcs(m, p1, vScale, lColor);

        xBegin(PT_LINES);
        xColor(lColor);

        float fAngle = 0.0f;
        for (int i = 0; i < 32; ++i)
        {
            const float sinA = std::sin(fAngle);
            const float cosA = std::cos(fAngle);
            fAngle += kArcStep;
            const float sinB = std::sin(fAngle);
            const float cosB = std::cos(fAngle);

            float a0[3] = { sinA, cosA, 0.0f };
            float b0[3] = { sinB, cosB, 0.0f };
            Transform(a0, mat, p0.Get(), vScale.Get());
            Transform(b0, mat, p0.Get(), vScale.Get());

            float a1[3] = { sinA, cosA, 0.0f };
            float b1[3] = { sinB, cosB, 0.0f };
            Transform(a1, mat, p1.Get(), vScale.Get());
            Transform(b1, mat, p1.Get(), vScale.Get());

            xVertex3v(a0);
            xVertex3v(b0);
            xVertex3v(a1);
            xVertex3v(b1);
        }

        xEnd();
    }

    void ZDrawDebugRender::xDrawText(const float* m, const float* v, const float* s, const char* pszText,
        uint32_t lTextLength, uint32_t lColor, int8_t lAlignFlags)
    {
        ZASSERT(!m_bInside_xBegin);

        if (!lTextLength)
            return;

        uint32_t lFirst = 0;
        while (lFirst < lTextLength)
        {
            const int c = pszText[lFirst];
            if (c > 32 && c < 127)
                break;
            ++lFirst;
        }

        if (lFirst == lTextLength)
            return;

        ZTextureBase* pPrevTexture = reinterpret_cast<ZTextureBase*>(m_lWantedTextureId);
        const bool bPrevTextureIsPtr = m_bWantedTextureIsPtr;

        xSetTexture(static_cast<uint32_t>(TEXTURE_FONT));
        xColor(lColor);
        xBegin(PT_TRIANGLES);

        float mText[12];
        if ((lAlignFlags & TEXTALIGN_VIEW) != 0)
        {
            tmat(mText, m_WorldToView.m0.data);
            mText[9] = 0.0f;
            mText[10] = 0.0f;
            mText[11] = 0.0f;
        }
        else
        {
            std::memcpy(mText, m, sizeof(mText));
        }

        const float fFontW = m_fFontSize.x;
        const float fFontH = m_fFontSize.y;

        const float aPos[4][2] =
        {
            { 0.0f,                          0.0f },
            { 0.0f,                          1.0f - 1.0f / fFontH },
            { 1.0f - 1.0f / fFontW,          1.0f - 1.0f / fFontH },
            { 1.0f - 1.0f / fFontW,          0.0f },
        };

        const float aUV[4][2] =
        {
            { 0.0f,     0.0f },
            { 0.0f,     0.1015625f },
            { 0.0625f,  0.1015625f },
            { 0.0625f,  0.0f },
        };

        static const int aIndices[6] = { 0, 3, 1, 3, 2, 1 };

        const bool bCenterBlock = (lAlignFlags & TEXTALIGN_CENTERBLOCK) != 0;
        const bool bCenterX = (lAlignFlags & TEXTALIGN_CENTERX) != 0;
        const bool bRight = (lAlignFlags & TEXTALIGN_RIGHT) != 0;

        uint32_t lMaxLineWidth = 0;
        if (bCenterBlock)
        {
            uint32_t lWidth = 0;
            for (uint32_t i = 0; i < lTextLength; ++i)
            {
                const int c = pszText[i];
                if (c == 10)
                    lWidth = 0;
                if (c == 9)
                    lWidth += 8;
                else
                    ++lWidth;
                if (lWidth > lMaxLineWidth)
                    lMaxLineWidth = lWidth;
            }
        }

        float fYOffset = 0.0f;

        do
        {
            uint32_t lLineWidth = 0;
            uint32_t i = 0;
            while (i < lTextLength)
            {
                const int c = pszText[i];
                if (c == 10)
                    break;
                if (c == 9)
                    lLineWidth += 8;
                else
                    ++lLineWidth;
                ++i;
            }

            float fXOffset = 0.0f;
            if (bCenterBlock)
                fXOffset = static_cast<float>(lMaxLineWidth) * -0.5f;
            if (bCenterX)
                fXOffset = static_cast<float>(lLineWidth) * -0.5f;
            if (bRight)
                fXOffset = static_cast<float>(lLineWidth) * -1.0f;

            bool bDone = false;
            while (!bDone)
            {
                const int c = *pszText;
                if (c == 10)
                {
                    ++pszText;
                    --lTextLength;
                    break;
                }

                if (c == 9)
                {
                    fXOffset += 8.0f;
                }
                else
                {
                    if (c > 32 && c != 127)
                    {
                        const int lCharCol = c % 16;
                        const int lCharRow = c / 16;
                        const float fU = static_cast<float>(lCharCol) * 0.0625f;
                        const float fV = static_cast<float>(lCharRow) * 0.1015625f;

                        float aVertices[4][5];
                        for (int k = 0; k < 4; ++k)
                        {
                            aVertices[k][0] = aPos[k][0] + fXOffset;
                            aVertices[k][1] = aPos[k][1] + fYOffset;
                            aVertices[k][2] = 0.0f;
                            Transform(aVertices[k], mText, v, s);
                            aVertices[k][3] = aUV[k][0] + fU;
                            aVertices[k][4] = aUV[k][1] + fV;
                        }

                        for (int k = 0; k < 6; ++k)
                        {
                            const int idx = aIndices[k];
                            m_CurrentVertex.t.x = aVertices[idx][3];
                            m_CurrentVertex.t.y = aVertices[idx][4];
                            m_CurrentVertex.t.z = 0.0f;
                            xVertex3(aVertices[idx][0], aVertices[idx][1], aVertices[idx][2]);
                        }
                    }

                    fXOffset += 1.0f;
                }

                --lTextLength;
                ++pszText;
                if (!lTextLength)
                {
                    bDone = true;
                    break;
                }
            }

            fYOffset -= 1.0f;
        }
        while (lTextLength);

        xEnd();

        if (bPrevTextureIsPtr)
            xSetTexture(pPrevTexture);
        else
            xSetTexture(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pPrevTexture)));
    }

    void ZDrawDebugRender::xDrawText(const ZVector3& v, const char* pszText, uint32_t lColor, int8_t lAlignFlags)
    {
        float m[12];
        mreset(m);

        const float s[3] = { 6.0f, 8.0f, 8.0f };

        const char* pEnd = pszText;
        while (*pEnd++)
            ;

        xDrawText(m, v.Get(), s, pszText, static_cast<uint32_t>(pEnd - pszText - 1), lColor, lAlignFlags);
    }
}
