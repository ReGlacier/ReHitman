#pragma once

#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Items/EWeaponOperation.h>
#include <Glacier/Items/ITEMSTATE.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZBitfield.h>


namespace Glacier::RTP
{
    template <typename T>
    void LoadDataProperty(ZDataProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template <typename T>
    void SaveDataProperty(ZDataProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template <typename T>
    void LoadVirtualProperty(ZVirtualProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template <typename T>
    void SaveVirtualProperty(ZVirtualProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    void LoadEnumProperty(ZEnumProperty* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    void SaveEnumProperty(ZEnumProperty* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template <typename T>
    void LoadVirtualEnumProperty(ZVirtualEnumProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template <typename T>
    void SaveVirtualEnumProperty(ZVirtualEnumProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    namespace VirtualTables
    {
        extern tVirtualTable<ZEnumProperty> Enum;

        extern tVirtualTable<ZDataProperty<bool>> Data_bool;
        extern tVirtualTable<ZDataProperty<char>> Data_char;
        extern tVirtualTable<ZDataProperty<uchar>> Data_uchar;
        extern tVirtualTable<ZDataProperty<short>> Data_short;
        extern tVirtualTable<ZDataProperty<ushort>> Data_ushort;
        extern tVirtualTable<ZDataProperty<int>> Data_int;
        extern tVirtualTable<ZDataProperty<uint>> Data_uint;
        extern tVirtualTable<ZDataProperty<float>> Data_float;
        extern tVirtualTable<ZDataProperty<TIMETYPE>> Data_TIMETYPE;
        extern tVirtualTable<ZDataProperty<::Glacier::ZRTString>> Data_ZRTString;
        extern tVirtualTable<ZDataProperty<ZGEOMREF>> Data_ZGEOMREF;
        extern tVirtualTable<ZDataProperty<ZANIM>> Data_ZANIM;
        extern tVirtualTable<ZDataProperty<ZCOLOR>> Data_ZCOLOR;
        extern tVirtualTable<ZDataProperty<ZFILENAME>> Data_ZFILENAME;
        extern tVirtualTable<ZDataProperty<ZRawData>> Data_ZRawData;
        extern tVirtualTable<ZDataProperty<REFTAB>> Data_REFTAB;
        extern tVirtualTable<ZDataProperty<REFTAB32>> Data_REFTAB32;
        extern tVirtualTable<ZDataProperty<ZAUDIOREF>> Data_ZAUDIOREF;
        extern tVirtualTable<ZDataProperty<ZBitfield<ITEMSTATE>>> Data_ZBitfield_ITEMSTATE;
        extern tVirtualTable<ZDataProperty<ZBitfield<WEAPONOPERATION>>> Data_ZBitfield_WEAPONOPERATION;
        extern tVirtualTable<ZDataProperty<ZBitfield<ESecurityZone>>> Data_ZBitfield_ESecurityZone;

        extern tVirtualTable<ZDataProperty<int[3]>> Data_int_3;
        extern tVirtualTable<ZDataProperty<int[21]>> Data_int_21;

        extern tVirtualTable<ZDataProperty<uint[2]>> Data_uint_2;
        extern tVirtualTable<ZDataProperty<uint[4]>> Data_uint_4;
        extern tVirtualTable<ZDataProperty<uint[15]>> Data_uint_15;
        extern tVirtualTable<ZDataProperty<uint[32]>> Data_uint_32;
        extern tVirtualTable<ZDataProperty<uint[64]>> Data_uint_64;
        extern tVirtualTable<ZDataProperty<uint[128]>> Data_uint_128;

        extern tVirtualTable<ZDataProperty<float[2]>> Data_float_2;
        extern tVirtualTable<ZDataProperty<float[3]>> Data_float_3;
        extern tVirtualTable<ZDataProperty<float[4]>> Data_float_4;
        extern tVirtualTable<ZDataProperty<float[9]>> Data_float_9;
        extern tVirtualTable<ZDataProperty<float[12]>> Data_float_12;
        extern tVirtualTable<ZDataProperty<float[2][3]>> Data_float_2_3;
        extern tVirtualTable<ZDataProperty<float[8][4]>> Data_float_8_4;
        extern tVirtualTable<ZDataProperty<ZGEOMREF[6]>> Data_ZGEOMREF_6;
        extern tVirtualTable<ZDataProperty<ZGEOMREF[64]>> Data_ZGEOMREF_64;

        extern tVirtualTable<ZVirtualProperty<bool>> Virtual_bool;
        extern tVirtualTable<ZVirtualProperty<int>> Virtual_int;
        extern tVirtualTable<ZVirtualProperty<uint>> Virtual_uint;
        extern tVirtualTable<ZVirtualProperty<float>> Virtual_float;
        extern tVirtualTable<ZVirtualProperty<::Glacier::ZRTString>> ZRTString;
        extern tVirtualTable<ZVirtualProperty<::Glacier::ZRTString>> Virtual_ZRTString;
        extern tVirtualTable<ZVirtualProperty<ZGEOMREF>> Virtual_ZGEOMREF;
        extern tVirtualTable<ZVirtualProperty<ZANIM>> Virtual_ZANIM;
        extern tVirtualTable<ZVirtualProperty<ZCOLOR>> Virtual_ZCOLOR;
        extern tVirtualTable<ZVirtualProperty<ZFILENAME>> Virtual_ZFILENAME;
        extern tVirtualTable<ZVirtualProperty<ZRawData>> Virtual_ZRawData;
        extern tVirtualTable<ZVirtualProperty<REFTAB32>> Virtual_REFTAB32;
        extern tVirtualTable<ZVirtualProperty<ZBitfield<ITEMSTATE>>> Virtual_ZBitfield_ITEMSTATE;
        extern tVirtualTable<ZVirtualProperty<ZBitfield<WEAPONOPERATION>>> Virtual_ZBitfield_WEAPONOPERATION;
        extern tVirtualTable<ZVirtualProperty<ZBitfield<EAnimMode>>> Virtual_ZBitfield_EAnimMode;

        extern tVirtualTable<ZVirtualProperty<int[3]>> Virtual_int_3;
        extern tVirtualTable<ZVirtualProperty<float[3]>> Virtual_float_3;
        extern tVirtualTable<ZVirtualProperty<float[4]>> Virtual_float_4;
        extern tVirtualTable<ZVirtualProperty<float[9]>> Virtual_float_9;

        extern tVirtualTable<ZVirtualEnumProperty<EBoundingBox>> VirtualEnum_EBoundingBox;
    }
}

//
// NOTE: This code is autogenerated by XeXe and AI. Please, do not edit this
//
extern decltype(&Glacier::RTP::VirtualTables::Data_bool) VirtualTable_DP__1;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZCOLOR) VirtualTable_DP__2;
extern decltype(&Glacier::RTP::VirtualTables::Data_REFTAB) VirtualTable_DP__3;
extern decltype(&Glacier::RTP::VirtualTables::Data_REFTAB32) VirtualTable_DP__4;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZGEOMREF) VirtualTable_DP__5;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint) VirtualTable_DP__6;
extern decltype(&Glacier::RTP::VirtualTables::Data_int) VirtualTable_DP__7;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZBitfield_ESecurityZone) VirtualTable_DP__9;
extern decltype(&Glacier::RTP::VirtualTables::Data_float) VirtualTable_DP__11;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_3) VirtualTable_DP__12;
extern decltype(&Glacier::RTP::VirtualTables::Data_short) VirtualTable_DP__13;
extern decltype(&Glacier::RTP::VirtualTables::Data_int) VirtualTable_DP__14;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__15) VirtualTable_DP__15;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZRTString) VirtualTable_DP__16;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__17) VirtualTable_DP__17;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__18) VirtualTable_DP__18;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__19) VirtualTable_DP__19;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__20) VirtualTable_DP__20;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZAUDIOREF) VirtualTable_DP__21;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_4) VirtualTable_DP__24;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__25) VirtualTable_DP__25;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__26) VirtualTable_DP__26;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_4) VirtualTable_DP__27;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_9) VirtualTable_DP__28;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZFILENAME) VirtualTable_DP__30;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_8_4) VirtualTable_DP__31;
extern decltype(&Glacier::RTP::VirtualTables::Data_uchar) VirtualTable_DP__32;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__33) VirtualTable_DP__33;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__34) VirtualTable_DP__34;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__35) VirtualTable_DP__35;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__37) VirtualTable_DP__37;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__38) VirtualTable_DP__38;
extern decltype(&Glacier::RTP::VirtualTables::Data_TIMETYPE) VirtualTable_DP__39;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__40) VirtualTable_DP__40;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZRawData) VirtualTable_DP__44;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__47) VirtualTable_DP__47;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__51) VirtualTable_DP__51;
extern decltype(&Glacier::RTP::VirtualTables::Data_int_3) VirtualTable_DP__56;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint) VirtualTable_DP__58;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_64) VirtualTable_DP__59;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_32) VirtualTable_DP__60;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_32) VirtualTable_DP__61;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_32) VirtualTable_DP__62;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__64) VirtualTable_DP__64;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__67) VirtualTable_DP__67;
extern decltype(&Glacier::RTP::VirtualTables::Data_float) VirtualTable_DP__68;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__69) VirtualTable_DP__69;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_12) VirtualTable_DP__71;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__73) VirtualTable_DP__73;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__74) VirtualTable_DP__74;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__75) VirtualTable_DP__75;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_2) VirtualTable_DP__76;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__77) VirtualTable_DP__77;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_4) VirtualTable_DP__79;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__88) VirtualTable_DP__88;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_2_3) VirtualTable_DP__89;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__90) VirtualTable_DP__90;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZBitfield_WEAPONOPERATION) VirtualTable_DP__114;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZBitfield_ITEMSTATE) VirtualTable_DP__118;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__122) VirtualTable_DP__122;
extern decltype(&Glacier::RTP::VirtualTables::Data_char) VirtualTable_DP__123;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__124) VirtualTable_DP__124;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__125) VirtualTable_DP__125;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_4) VirtualTable_DP__126;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__127) VirtualTable_DP__127;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__128) VirtualTable_DP__128;
extern decltype(&Glacier::RTP::VirtualTables::Data_ushort) VirtualTable_DP__136;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__137) VirtualTable_DP__137;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__140) VirtualTable_DP__140;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__147) VirtualTable_DP__147;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__148) VirtualTable_DP__148;
extern decltype(&Glacier::RTP::VirtualTables::Data_float_3) VirtualTable_DP__149;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__159) VirtualTable_DP__159;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__160) VirtualTable_DP__160;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__161) VirtualTable_DP__161;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__163) VirtualTable_DP__163;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__164) VirtualTable_DP__164;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__165) VirtualTable_DP__165;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__166) VirtualTable_DP__166;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__167) VirtualTable_DP__167;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_2) VirtualTable_DP__171;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__172) VirtualTable_DP__172;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__173) VirtualTable_DP__173;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__174) VirtualTable_DP__174;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_15) VirtualTable_DP__175;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__176) VirtualTable_DP__176;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__178) VirtualTable_DP__178;
extern decltype(&Glacier::RTP::VirtualTables::Data_char) VirtualTable_DP__182;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__183) VirtualTable_DP__183;
extern decltype(&Glacier::RTP::VirtualTables::Data_int_21) VirtualTable_DP__184;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__186) VirtualTable_DP__186;
extern decltype(&Glacier::RTP::VirtualTables::Data_uchar) VirtualTable_DP__187;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZANIM) VirtualTable_DP__188;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__190) VirtualTable_DP__190;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZGEOMREF_64) VirtualTable_DP__191;
extern decltype(&Glacier::RTP::VirtualTables::Data_ZGEOMREF_6) VirtualTable_DP__192;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__193) VirtualTable_DP__193;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__194) VirtualTable_DP__194;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__196) VirtualTable_DP__196;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__200) VirtualTable_DP__200;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__201) VirtualTable_DP__201;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__209) VirtualTable_DP__209;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__210) VirtualTable_DP__210;
extern decltype(&Glacier::RTP::VirtualTables::Data_int) VirtualTable_DP__215;
extern decltype(&Glacier::RTP::VirtualTables::Data_uint_128) VirtualTable_DP__216;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__217) VirtualTable_DP__217;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_DP__218) VirtualTable_DP__218;
extern decltype(&Glacier::RTP::VirtualTables::Enum) VirtualTable_EP;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_bool) VirtualTable_VP__1;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZCOLOR) VirtualTable_VP__2;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_REFTAB32) VirtualTable_VP__4;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZGEOMREF) VirtualTable_VP__5;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_uint) VirtualTable_VP__6;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_int) VirtualTable_VP__7;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__10) VirtualTable_VP__10;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_float) VirtualTable_VP__11;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_float_3) VirtualTable_VP__12;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_int) VirtualTable_VP__14;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZRTString) VirtualTable_VP__16;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_float_9) VirtualTable_VP__28;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZFILENAME) VirtualTable_VP__30;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZRawData) VirtualTable_VP__44;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_int_3) VirtualTable_VP__56;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__78) VirtualTable_VP__78;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_float_4) VirtualTable_VP__79;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_float_3) VirtualTable_VP__95;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__104) VirtualTable_VP__104;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__120) VirtualTable_VP__120;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__133) VirtualTable_VP__133;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__143) VirtualTable_VP__143;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__144) VirtualTable_VP__144;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__145) VirtualTable_VP__145;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__168) VirtualTable_VP__168;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZBitfield_EAnimMode) VirtualTable_VP__169;
// extern decltype(&Glacier::RTP::VirtualTables::VirtualTable_VP__170) VirtualTable_VP__170;
extern decltype(&Glacier::RTP::VirtualTables::Virtual_ZANIM) VirtualTable_VP__188;
extern decltype(&Glacier::RTP::VirtualTables::VirtualEnum_EBoundingBox) VirtualTable_VP__189;

#if 0 // Just for reference
#ifndef GLACIER_RTP_VIRTUALTABLES_NO_XEXE_ALIASES
// **************
// DataProperties
// **************
inline auto VirtualTable_DP__1 = &Glacier::RTP::VirtualTables::Data_bool;
inline auto VirtualTable_DP__2 = &Glacier::RTP::VirtualTables::Data_ZCOLOR;
inline auto VirtualTable_DP__3 = &Glacier::RTP::VirtualTables::Data_REFTAB;
inline auto VirtualTable_DP__4 = &Glacier::RTP::VirtualTables::Data_REFTAB32;
inline auto VirtualTable_DP__5 = &Glacier::RTP::VirtualTables::Data_ZGEOMREF;
inline auto VirtualTable_DP__6 = &Glacier::RTP::VirtualTables::Data_uint;
inline auto VirtualTable_DP__7 = &Glacier::RTP::VirtualTables::Data_int;
inline auto VirtualTable_DP__9 = &Glacier::RTP::VirtualTables::Data_ZBitfield_ESecurityZone;
inline auto VirtualTable_DP__11 = &Glacier::RTP::VirtualTables::Data_float;
inline auto VirtualTable_DP__12 = &Glacier::RTP::VirtualTables::Data_float_3;
inline auto VirtualTable_DP__13 = &Glacier::RTP::VirtualTables::Data_short;
inline auto VirtualTable_DP__14 = &Glacier::RTP::VirtualTables::Data_int;
// inline auto VirtualTable_DP__15 = &Glacier::RTP::VirtualTables::VirtualTable_DP__15;
inline auto VirtualTable_DP__16 = &Glacier::RTP::VirtualTables::Data_ZRTString;
// inline auto VirtualTable_DP__17 = &Glacier::RTP::VirtualTables::VirtualTable_DP__17;
// inline auto VirtualTable_DP__18 = &Glacier::RTP::VirtualTables::VirtualTable_DP__18;
// inline auto VirtualTable_DP__19 = &Glacier::RTP::VirtualTables::VirtualTable_DP__19;
// inline auto VirtualTable_DP__20 = &Glacier::RTP::VirtualTables::VirtualTable_DP__20;
inline auto VirtualTable_DP__21 = &Glacier::RTP::VirtualTables::Data_ZAUDIOREF;
inline auto VirtualTable_DP__24 = &Glacier::RTP::VirtualTables::Data_float_4;
// inline auto VirtualTable_DP__25 = &Glacier::RTP::VirtualTables::VirtualTable_DP__25;
// inline auto VirtualTable_DP__26 = &Glacier::RTP::VirtualTables::VirtualTable_DP__26;
inline auto VirtualTable_DP__27 = &Glacier::RTP::VirtualTables::Data_uint_4;
inline auto VirtualTable_DP__28 = &Glacier::RTP::VirtualTables::Data_float_9;
inline auto VirtualTable_DP__30 = &Glacier::RTP::VirtualTables::Data_ZFILENAME;
inline auto VirtualTable_DP__31 = &Glacier::RTP::VirtualTables::Data_float_8_4;
inline auto VirtualTable_DP__32 = &Glacier::RTP::VirtualTables::Data_uchar;
// inline auto VirtualTable_DP__33 = &Glacier::RTP::VirtualTables::VirtualTable_DP__33;
// inline auto VirtualTable_DP__34 = &Glacier::RTP::VirtualTables::VirtualTable_DP__34;
// inline auto VirtualTable_DP__35 = &Glacier::RTP::VirtualTables::VirtualTable_DP__35;
// inline auto VirtualTable_DP__37 = &Glacier::RTP::VirtualTables::VirtualTable_DP__37;
// inline auto VirtualTable_DP__38 = &Glacier::RTP::VirtualTables::VirtualTable_DP__38;
inline auto VirtualTable_DP__39 = &Glacier::RTP::VirtualTables::Data_TIMETYPE;
// inline auto VirtualTable_DP__40 = &Glacier::RTP::VirtualTables::VirtualTable_DP__40;
inline auto VirtualTable_DP__44 = &Glacier::RTP::VirtualTables::Data_ZRawData;
// inline auto VirtualTable_DP__47 = &Glacier::RTP::VirtualTables::VirtualTable_DP__47;
// inline auto VirtualTable_DP__51 = &Glacier::RTP::VirtualTables::VirtualTable_DP__51;
inline auto VirtualTable_DP__56 = &Glacier::RTP::VirtualTables::Data_int_3;
inline auto VirtualTable_DP__58 = &Glacier::RTP::VirtualTables::Data_uint;
inline auto VirtualTable_DP__59 = &Glacier::RTP::VirtualTables::Data_uint_64;
inline auto VirtualTable_DP__60 = &Glacier::RTP::VirtualTables::Data_uint_32;
inline auto VirtualTable_DP__61 = &Glacier::RTP::VirtualTables::Data_uint_32;
inline auto VirtualTable_DP__62 = &Glacier::RTP::VirtualTables::Data_uint_32;
// inline auto VirtualTable_DP__64 = &Glacier::RTP::VirtualTables::VirtualTable_DP__64;
// inline auto VirtualTable_DP__67 = &Glacier::RTP::VirtualTables::VirtualTable_DP__67;
inline auto VirtualTable_DP__68 = &Glacier::RTP::VirtualTables::Data_float;
// inline auto VirtualTable_DP__69 = &Glacier::RTP::VirtualTables::VirtualTable_DP__69;
inline auto VirtualTable_DP__71 = &Glacier::RTP::VirtualTables::Data_float_12;
// inline auto VirtualTable_DP__73 = &Glacier::RTP::VirtualTables::VirtualTable_DP__73;
// inline auto VirtualTable_DP__74 = &Glacier::RTP::VirtualTables::VirtualTable_DP__74;
// inline auto VirtualTable_DP__75 = &Glacier::RTP::VirtualTables::VirtualTable_DP__75;
inline auto VirtualTable_DP__76 = &Glacier::RTP::VirtualTables::Data_float_2;
// inline auto VirtualTable_DP__77 = &Glacier::RTP::VirtualTables::VirtualTable_DP__77;
inline auto VirtualTable_DP__79 = &Glacier::RTP::VirtualTables::Data_float_4;
// inline auto VirtualTable_DP__88 = &Glacier::RTP::VirtualTables::VirtualTable_DP__88;
inline auto VirtualTable_DP__89 = &Glacier::RTP::VirtualTables::Data_float_2_3;
// inline auto VirtualTable_DP__90 = &Glacier::RTP::VirtualTables::VirtualTable_DP__90;
inline auto VirtualTable_DP__114 = &Glacier::RTP::VirtualTables::Data_ZBitfield_WEAPONOPERATION;
inline auto VirtualTable_DP__118 = &Glacier::RTP::VirtualTables::Data_ZBitfield_ITEMSTATE;
// inline auto VirtualTable_DP__122 = &Glacier::RTP::VirtualTables::VirtualTable_DP__122;
inline auto VirtualTable_DP__123 = &Glacier::RTP::VirtualTables::Data_char;
// inline auto VirtualTable_DP__124 = &Glacier::RTP::VirtualTables::VirtualTable_DP__124;
// inline auto VirtualTable_DP__125 = &Glacier::RTP::VirtualTables::VirtualTable_DP__125;
inline auto VirtualTable_DP__126 = &Glacier::RTP::VirtualTables::Data_float_4;
// inline auto VirtualTable_DP__127 = &Glacier::RTP::VirtualTables::VirtualTable_DP__127;
// inline auto VirtualTable_DP__128 = &Glacier::RTP::VirtualTables::VirtualTable_DP__128;
inline auto VirtualTable_DP__136 = &Glacier::RTP::VirtualTables::Data_ushort;
// inline auto VirtualTable_DP__137 = &Glacier::RTP::VirtualTables::VirtualTable_DP__137;
// inline auto VirtualTable_DP__140 = &Glacier::RTP::VirtualTables::VirtualTable_DP__140;
// inline auto VirtualTable_DP__147 = &Glacier::RTP::VirtualTables::VirtualTable_DP__147;
// inline auto VirtualTable_DP__148 = &Glacier::RTP::VirtualTables::VirtualTable_DP__148;
inline auto VirtualTable_DP__149 = &Glacier::RTP::VirtualTables::Data_float_3;
// inline auto VirtualTable_DP__159 = &Glacier::RTP::VirtualTables::VirtualTable_DP__159;
// inline auto VirtualTable_DP__160 = &Glacier::RTP::VirtualTables::VirtualTable_DP__160;
// inline auto VirtualTable_DP__161 = &Glacier::RTP::VirtualTables::VirtualTable_DP__161;
// inline auto VirtualTable_DP__163 = &Glacier::RTP::VirtualTables::VirtualTable_DP__163;
// inline auto VirtualTable_DP__164 = &Glacier::RTP::VirtualTables::VirtualTable_DP__164;
// inline auto VirtualTable_DP__165 = &Glacier::RTP::VirtualTables::VirtualTable_DP__165;
// inline auto VirtualTable_DP__166 = &Glacier::RTP::VirtualTables::VirtualTable_DP__166;
// inline auto VirtualTable_DP__167 = &Glacier::RTP::VirtualTables::VirtualTable_DP__167;
inline auto VirtualTable_DP__171 = &Glacier::RTP::VirtualTables::Data_uint_2;
// inline auto VirtualTable_DP__172 = &Glacier::RTP::VirtualTables::VirtualTable_DP__172;
// inline auto VirtualTable_DP__173 = &Glacier::RTP::VirtualTables::VirtualTable_DP__173;
// inline auto VirtualTable_DP__174 = &Glacier::RTP::VirtualTables::VirtualTable_DP__174;
inline auto VirtualTable_DP__175 = &Glacier::RTP::VirtualTables::Data_uint_15;
// inline auto VirtualTable_DP__176 = &Glacier::RTP::VirtualTables::VirtualTable_DP__176;
// inline auto VirtualTable_DP__178 = &Glacier::RTP::VirtualTables::VirtualTable_DP__178;
inline auto VirtualTable_DP__182 = &Glacier::RTP::VirtualTables::Data_char;
// inline auto VirtualTable_DP__183 = &Glacier::RTP::VirtualTables::VirtualTable_DP__183;
inline auto VirtualTable_DP__184 = &Glacier::RTP::VirtualTables::Data_int_21;
// inline auto VirtualTable_DP__186 = &Glacier::RTP::VirtualTables::VirtualTable_DP__186;
inline auto VirtualTable_DP__187 = &Glacier::RTP::VirtualTables::Data_uchar;
inline auto VirtualTable_DP__188 = &Glacier::RTP::VirtualTables::Data_ZANIM;
// inline auto VirtualTable_DP__190 = &Glacier::RTP::VirtualTables::VirtualTable_DP__190;
inline auto VirtualTable_DP__191 = &Glacier::RTP::VirtualTables::Data_ZGEOMREF_64;
inline auto VirtualTable_DP__192 = &Glacier::RTP::VirtualTables::Data_ZGEOMREF_6;
// inline auto VirtualTable_DP__193 = &Glacier::RTP::VirtualTables::VirtualTable_DP__193;
// inline auto VirtualTable_DP__194 = &Glacier::RTP::VirtualTables::VirtualTable_DP__194;
// inline auto VirtualTable_DP__196 = &Glacier::RTP::VirtualTables::VirtualTable_DP__196;
// inline auto VirtualTable_DP__200 = &Glacier::RTP::VirtualTables::VirtualTable_DP__200;
// inline auto VirtualTable_DP__201 = &Glacier::RTP::VirtualTables::VirtualTable_DP__201;
// inline auto VirtualTable_DP__209 = &Glacier::RTP::VirtualTables::VirtualTable_DP__209;
// inline auto VirtualTable_DP__210 = &Glacier::RTP::VirtualTables::VirtualTable_DP__210;
inline auto VirtualTable_DP__215 = &Glacier::RTP::VirtualTables::Data_int;
inline auto VirtualTable_DP__216 = &Glacier::RTP::VirtualTables::Data_uint_128;
// inline auto VirtualTable_DP__217 = &Glacier::RTP::VirtualTables::VirtualTable_DP__217;
// inline auto VirtualTable_DP__218 = &Glacier::RTP::VirtualTables::VirtualTable_DP__218;
// **************
// EnumProperties
// **************
inline auto VirtualTable_EP = &Glacier::RTP::VirtualTables::Enum;
// *****************
// VirtualProperties
// *****************
inline auto VirtualTable_VP__1 = &Glacier::RTP::VirtualTables::Virtual_bool;
inline auto VirtualTable_VP__2 = &Glacier::RTP::VirtualTables::Virtual_ZCOLOR;
inline auto VirtualTable_VP__4 = &Glacier::RTP::VirtualTables::Virtual_REFTAB32;
inline auto VirtualTable_VP__5 = &Glacier::RTP::VirtualTables::Virtual_ZGEOMREF;
inline auto VirtualTable_VP__6 = &Glacier::RTP::VirtualTables::Virtual_uint;
inline auto VirtualTable_VP__7 = &Glacier::RTP::VirtualTables::Virtual_int;
// inline auto VirtualTable_VP__10 = &Glacier::RTP::VirtualTables::VirtualTable_VP__10;
inline auto VirtualTable_VP__11 = &Glacier::RTP::VirtualTables::Virtual_float;
inline auto VirtualTable_VP__12 = &Glacier::RTP::VirtualTables::Virtual_float_3;
inline auto VirtualTable_VP__14 = &Glacier::RTP::VirtualTables::Virtual_int;
inline auto VirtualTable_VP__16 = &Glacier::RTP::VirtualTables::Virtual_ZRTString;
inline auto VirtualTable_VP__28 = &Glacier::RTP::VirtualTables::Virtual_float_9;
inline auto VirtualTable_VP__30 = &Glacier::RTP::VirtualTables::Virtual_ZFILENAME;
inline auto VirtualTable_VP__44 = &Glacier::RTP::VirtualTables::Virtual_ZRawData;
inline auto VirtualTable_VP__56 = &Glacier::RTP::VirtualTables::Virtual_int_3;
// inline auto VirtualTable_VP__78 = &Glacier::RTP::VirtualTables::VirtualTable_VP__78;
inline auto VirtualTable_VP__79 = &Glacier::RTP::VirtualTables::Virtual_float_4;
inline auto VirtualTable_VP__95 = &Glacier::RTP::VirtualTables::Virtual_float_3;
// inline auto VirtualTable_VP__104 = &Glacier::RTP::VirtualTables::VirtualTable_VP__104;
// inline auto VirtualTable_VP__120 = &Glacier::RTP::VirtualTables::VirtualTable_VP__120;
// inline auto VirtualTable_VP__133 = &Glacier::RTP::VirtualTables::VirtualTable_VP__133;
// inline auto VirtualTable_VP__143 = &Glacier::RTP::VirtualTables::VirtualTable_VP__143;
// inline auto VirtualTable_VP__144 = &Glacier::RTP::VirtualTables::VirtualTable_VP__144;
// inline auto VirtualTable_VP__145 = &Glacier::RTP::VirtualTables::VirtualTable_VP__145;
// inline auto VirtualTable_VP__168 = &Glacier::RTP::VirtualTables::VirtualTable_VP__168;
inline auto VirtualTable_VP__169 = &Glacier::RTP::VirtualTables::Virtual_ZBitfield_EAnimMode;
// inline auto VirtualTable_VP__170 = &Glacier::RTP::VirtualTables::VirtualTable_VP__170;
inline auto VirtualTable_VP__188 = &Glacier::RTP::VirtualTables::Virtual_ZANIM;
inline auto VirtualTable_VP__189 = &Glacier::RTP::VirtualTables::VirtualEnum_EBoundingBox;
#endif
#endif
