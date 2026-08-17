#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    class ZDrawDebugRender
    {
    public:
        // constants
        static constexpr int MAX_NUM_VERTICES = 0x200;

        // types
        struct SViewport
        {
            uint32_t x;
            uint32_t y;
            uint32_t w;
            uint32_t h;
        };

        struct SVertex
        {
            ZVector3 p;
            uint32_t c;
            ZVector3 t;
        };

        enum PRIMTYPE
        {
            PT_LINES = 0,
            PT_TRIANGLES = 1
        };

        enum AXIS
        {
            AXIS_X = 0x0,
            AXIS_Y = 0x1,
            AXIS_Z = 0x2,
            AXIS_YZ = 0x3,
            AXIS_YMZ = 0x4,
            AXIS_XZ = 0x5,
            AXIS_XMZ = 0x6,
            AXIS_XY = 0x7,
            AXIS_XMY = 0x8,
            AXIS_XYZ = 0x9,
            AXIS_XYMZ = 0xA,
            AXIS_XMYZ = 0xB,
            AXIS_XMYMZ = 0xC,
        };

        enum TEXTURETYPE
        {
            TEXTURE_NONE  = 0x0,
            TEXTURE_FONT  = 0x80000000,
            TEXTURE_MOUSE = 0x80000001,
            TEXTURE_LAST  = 0x80000002,
        };

        enum TEXTALIGN
        {
            TEXTALIGN_CENTERX     = 0x1,
            TEXTALIGN_CENTERY     = 0x2,
            TEXTALIGN_CENTER      = 0x3,
            TEXTALIGN_RIGHT       = 0x4,
            TEXTALIGN_VIEW        = 0x8,
            TEXTALIGN_CENTERBLOCK = 0x10,
        };

        // vtbl
        virtual ~ZDrawDebugRender();
        virtual void xFlush();
        virtual void Begin(IView* pView, ZCameraSpace* pCameraSpace);
        virtual void End();
        virtual void DrawLines();
        virtual void DrawTriangles();
        virtual void SetDrawMode(uint32_t lMode);
        virtual void SetTexture(ZTextureBase* pTexture);
        virtual void SetTexture(uint32_t lTextureId);
        virtual void SetViewport(const ZDrawDebugRender::SViewport& viewport);

        // methods
        ZDrawDebugRender();

        void xBegin(ZDrawDebugRender::PRIMTYPE Type);
        void xEnd();
        void xReset();
        void xColor(uint32_t c);
        void xTexCoords3v(const float* v);
        void xTexCoords2s(float s, float t);
        void xTexCoords2v(float s, float t);
        void xSetDrawMode(uint32_t lDrawMode);
        void xSetTexture(uint32_t lTexture);
        void xSetTexture(ZTextureBase* pTexture);
        bool xCanFlush();
        void xVertex3(float x, float y, float z);
        void xVertex3v(const float* v);
        void xVertex2(float x, float y);
        void xVertex2v(const float* v);
        void xDrawText(const float* m, const float* v, const float* s, const char* pszText, uint32_t lTextLength, uint32_t lColor, int8_t lAlignFlags);
        void xDrawText(const ZVector3& v, const char* pszText, uint32_t lColor, int8_t lAlignFlags);
        
        void SetOrtho(const float* v);
        void TransformWorldToView(float* vOut, const float* vIn);
        void TransformViewToWorld(float* vOut, const float* vIn);

        void DrawLine(const ZVector3& vStart, const ZVector3& vEnd, uint32_t lColor);
        void DrawTriangle(const ZVector3& v0, const ZVector3& v1, const ZVector3& v2, uint32_t lColor);
        void DrawArc(const ZMat3x3& m, const ZVector3& v, const float* p1, const float* p2, const float* s, uint32_t lColor);
        void DrawCircle(const ZMat3x3& m, const ZVector3& v, float fRadius, uint32_t lColor);
        void DrawBoxWire(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawBoxFilled(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawGrid(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lDivisionsZ, uint32_t lColor);
        void DrawArrow(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawPyramid(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawSphere3(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawCappedPyramid(const ZMat3x3& m, const ZVector3& v, float fAngleRad, float fAspect, uint32_t lColor);
        void DrawCylinder(const ZMat3x3& m, const ZVector3& v, float fHeight, float fRadius, uint32_t lColor);
        void DrawCone(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawConeSolid(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawCappedCone(const ZMat3x3& m, const ZVector3& v, float fAngleRad, float fLength, uint32_t lColor);
        void DrawSphereArcs(const ZMat3x3& m, const ZVector3& v, const ZVector3& s, uint32_t lColor);
        void DrawAxis(const ZVector3& vStart, const ZVector3& vDir, float fScale, uint32_t lColor);
        void DrawCapsule(const ZVector3& p0, const ZVector3& p1, float fRadius, uint32_t lColor);
        void DrawMatrix(ZMat3x3& m, const ZVector3& v, int8_t lAxisMask);

        // members
        const IView* m_pIView;
        ZMatrix m_WorldToView;
        ZMatrix m_ViewToWorld;
        ZDrawDebugRender::SViewport m_Viewport;
        bool m_bOrtho;
        RE_ADD_PADDING(3);
        float m_fOrthoScale;
        bool m_bInside_xBegin;
        ZDrawDebugRender::PRIMTYPE m_CurrentType;
        ZDrawDebugRender::SVertex m_CurrentVertex;
        uint32_t m_lVertexCount;
        ZDrawDebugRender::SVertex m_Vertices[MAX_NUM_VERTICES];
        uint32_t m_lCurrentDrawMode;
        uint32_t m_lCurrentTextureId;
        uint32_t m_lWantedDrawMode;
        uint32_t m_lWantedTextureId;
        bool m_bWantedTextureIsPtr;
        RE_ADD_PADDING(3);
        ZVector2 m_fFontSize;
    };
}