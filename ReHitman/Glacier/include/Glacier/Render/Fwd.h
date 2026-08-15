#pragma once

// D3D9
enum _D3DXINCLUDE_TYPE;
using D3DXINCLUDE_TYPE = _D3DXINCLUDE_TYPE;

enum _D3DPOOL;
using D3DPOOL = _D3DPOOL;

struct _D3DXMACRO;
using D3DXMACRO = _D3DXMACRO;

struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DTexture9;
struct IDirect3DSurface9;
struct IDirect3DCubeTexture9;
struct IDirect3DPixelShader9;
struct IDirect3DIndexBuffer9;
struct IDirect3DVertexBuffer9;
struct IDirect3DVertexShader9;
struct IDirect3DVertexDeclaration9;


namespace Glacier
{
    // Prims
    struct SPrims;
    struct SPrimLight;
    struct SPrimObject;
    struct SVertexWintel;
    struct SVertexColorD3D;

    // Prim Access
    class ZPrimAccess;
    class ZPrimAccessMesh;
    struct ZPrimHandle;

    // Legacy
    class ZOldDrawInfo;
    class ZRender;
    struct IView;
    struct IDraw;

    // Light
    struct SUpdateLightData;

    // Render entries
    class ZRenderEntry;
    class ZRenderEntryGeom;
    class ZRenderEntryBones;
    class ZRenderEntrySprite;
    class ZRenderEntryLists;
    struct SRenderEntryInstance;

    // Others
    class ZRenderBaseDll;
    struct ZRenderDrawBase;
    struct ZBoneModifyBase;
    class ZTextureBase;
    class ZBitmap;
    class ZBoneModifyBase; // rly here?
    class ZRagdollContainer;
    class ZPrimControlBase;
    class ZRenderMaterialBuffer;
    class ZRenderMaterialBufferD3D;
    class ZTexturePackBase;
    class ZRenderMaterialPacker;
    class ZRenderMaterialInstance;
    class ZRenderContext;
    class ZRenderObjectInstance;
    class ZRenderObject;
    class ZRenderMaterialLayer;
    class ZRenderMaterialSubClass;
    class ZRenderMaterialBinderList;
    struct SRMaterialProperties;

    class ZRenderBinder;
    class ZRenderMaterialEffectD3DFX;
    class ZRenderBinderList;
    class ZRenderMaterialBinderTextureD3DFX;
    class ZDirect3DDevice;
    class ZTextureBase;
    class ZTextureD3D;
    class ZTextureManagerD3D;
    class ZRenderWintelD3D;
    class ZPrimControlWintel;
    struct SRenderEntryNotifyInfo;
    struct SRenderEntryInstance;
    class ZCmdList;
    class ZViewSpace;
    class ZRenderView;
    class ZRenderMaterialSubClassD3D;
    class ZRenderMaterialLayerD3DFX;
    class ZRPropertyReader;
    class ZPostFilter;
    class ZRenderViewBase;
    class ZRenderMaterialClass;
    class ZRenderMaterialClassD3D;
    class ZRMaterialObject;
    class ZRenderDrawD3D;
    class ZRenderMaterialResourceD3DFX;
    class ZRIndexContainer;
    class ZRVertexContainer;
    class ZRD3DStaticVB;
    class ZRD3DStaticIB;
    class ZRD3DDynamicVB;
    class ZRD3DDynamicIB;
    class ZRD3DDynamicIB32;
    class ZRenderViewD3D;

    struct SBonesArray;
    struct SSpriteArray;
    struct SDrawArray;
}