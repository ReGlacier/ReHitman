#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace Glacier
{
    using uchar = unsigned char;
    using ushort = unsigned short;
    using uint = unsigned int;
    using ZMSGID = unsigned short;

    /// FWD
    class ISerializerStream {};
    class IInputSerializerStream {};
    class IOutputSerializerStream {};

    class ZScene;
    class ZLIST;
    class COLI;
    class CCom;
    class ZBone;
    class ZCameraSpace;
    class ZBaseGeom;
    class ZDrawBuffer;
    class ZItemState;
    class ZCAMERA;
    class ZScheduledScript;
    class ZGEOM;
    class ZLNKOBJ;
    class ZItem;
    class ZItemTemplate;
    class ZROOM;
    class ZGROUP;
    class ISerializerStream;
    class ZSWScene;
    class ZEventBase;
    class ZGeomBuffer;
    class ZGameStats;
    class ZAnimationInfo;
    class ResourceCollection;
    struct SGeomTypeCount;
    struct SCompiledGeom;
    struct SInputAction;
    struct SInputActionDefinition;
    enum ALLOCSEQUENCESTATUS {};

    class ZCheckPointBuffer; // NOT AVAILABLE FOR PC ?

    using zstring = std::string; /// I GUESS, CAUSE INSIDE ZSTRING USED STD::BASIC_STRING<CHAR, ...>
}