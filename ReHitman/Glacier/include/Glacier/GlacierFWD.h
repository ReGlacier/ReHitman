#pragma once

#include <string>

namespace Glacier
{
    using uchar = unsigned char;
    using ushort = unsigned short;
    using uint = unsigned int;
    using ZMSGID = unsigned short;
    using ZDATA = void*;
    using ZREF = unsigned int;
    using SREF = unsigned int;
    using ZWM_MESSAGE = signed int; //Less that 0x1000!

    struct RefRun;
    using TabBlk = void*;

    /// FWD
    class ISerializerStream {};
    class IInputSerializerStream {};
    class IOutputSerializerStream {};

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
    class CHUNKFILE;
    class CHUNK;
    class REFTAB;
    class REFTAB32;
    class ZOffsetAlloc;
    class ZEventBuffer;
    class ZGeomBufferPoolEntity;
    class ZQElemsBuffer;
    class ZBaseGeomLists;

    struct SGeomTypeCount;
    struct SCompiledGeom;
    struct SInputAction;
    struct SInputActionDefinition;
    struct SExtendedImpactInfo;
    struct SRigidBodyVelocity;

    enum ALLOCSEQUENCESTATUS {};
    struct ITEMHANDS {};
    struct LocomotionInfo;

    using ITEMSTATE = int;
    struct WEAPONOPERATION {};
    enum EFootSide;

    class ZCheckPointBuffer; // NOT AVAILABLE FOR PC ?

    using zstring = std::string; /// I GUESS, CAUSE INSIDE ZSTRING USED STD::BASIC_STRING<CHAR, ...>

    namespace PF4
    {
        class Interface;
    }

    class ZEntityLocator;
    class ZBaseGeom; //ZEntityLocator implements this by old design
    class FsZip_t;

    using AnimCallback_t = bool(ZGEOM::*)(void*, float, float, unsigned int);
}