#pragma once

#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZREF.h>
#include <string>
#include <cassert>

namespace Glacier
{
    using uchar = unsigned char;
    using ushort = unsigned short;
    using uint = unsigned int;
    using ZMSGID = unsigned short;
    using ZDATA = void*;
    using SREF = unsigned int;
    enum ZWM_MESSAGE;

    struct RefRun;
    
    /// FWD
    struct ISerializerStream;
    struct IInputSerializerStream;
    struct IOutputSerializerStream;

    class ZPackedInput;
    class ZScene;
    class ZLIST;
    class COLI;
    class CCom;
    class ZBone;
    class ZCameraSpace;
    class ZDrawBuffer;
    class ZItemState;
    class ZCAMERA;
    class ZScheduledScript;
    class ZGEOM;
    class ZSTDOBJ;
    class ZLNKOBJ;
    class ZItem;
    class ZItemWeapon;
    class ZItemTemplate;
    class ZItemTemplateAmmo;
    class ZItemWeaponTemplate;
    class ZROOM;
    class ZGROUP;
    class ZAction;
    class ZLNKWHANDS;
    class ISerializerStream;
    class ZSWScene;
    class ZEventBase;
    class ZGeomBuffer;
    class ZGameStats;
    class ZAnimationInfo;
    class AnimationManager;
    class ResourceCollection;
    class ZAnimVariationHandle;
    class ZLnkAction;
    class ZIKHAND;
    class ZItemContainer;
    class ZItemTemplateWeapon;
    class ZBodyInfo;
    class ZActionDispatcher;
    class ZROUTCLASSINFO;
    struct CHUNKFILE;
    class CHUNK;
    class REFTAB;
    class REFTAB32;
    class ZOffsetAlloc;
    class ZEventBuffer;
    class ZGeomBufferPoolEntity;
    class ZQElemsBuffer;
    class ZBaseGeomLists;
    class CInventory;
    class ZParticleEmitter;

    struct SGeomTypeCount;
    struct SCompiledGeom;
    struct SInputAction;
    struct SInputActionDefinition;
    struct SExtendedImpactInfo;
    struct SRigidBodyVelocity;
    struct ZPoolAllocRefTab;
    struct ZPoolAllocLinkSortRefTab;

    enum ALLOCSEQUENCESTATUS;
    struct LocomotionInfo;

    class ZCheckPointBuffer; // NOT AVAILABLE FOR PC ?

    class ZBaseGeom;
    class FsZip_t;

    using AnimCallback_t = bool(ZGEOM::*)(void*, float, float, unsigned int);

    class ZRX86AllocIf;
    struct SBoneDefinition;
}