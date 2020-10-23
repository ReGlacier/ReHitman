#pragma once

#include <string>

namespace Glacier
{
    using uchar = unsigned char;
    using ushort = unsigned short;
    using uint = unsigned int;
    using ZMSGID = unsigned short;

    using RefRun = void*;
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
    class ZLNKOBJ;
    class ZItem;
    class ZItemWeapon;
    class ZItemTemplate;
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
    class ResourceCollection;
    class ZAnimVariationHandle;
    class ZLnkAction;
    class ZIKHAND;
    class ZBodyInfo;
    class ZActionDispatcher;
    class ZROUTCLASSINFO;

    struct SGeomTypeCount;
    struct SCompiledGeom;
    struct SInputAction;
    struct SInputActionDefinition;
    struct SExtendedImpactInfo;
    struct SRigidBodyVelocity;
    struct SMouseColi;

    enum ALLOCSEQUENCESTATUS {};
    struct ITEMHANDS {};
    struct LocomotionInfo;

    struct ITEMSTATE {};
    enum EFootSide {};

    class ZCheckPointBuffer; // NOT AVAILABLE FOR PC ?

    using zstring = std::string; /// I GUESS, CAUSE INSIDE ZSTRING USED STD::BASIC_STRING<CHAR, ...>

    namespace PF4
    {
        class Interface;
    }

    class ZBaseGeom; //ZEntityLocator implements this by old design
}