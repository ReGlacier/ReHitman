#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    void Zlink__Stopallanims(ZREF rLink);
    void Zlink__Stopubanims(ZREF rLink);
    anim Zlink__Getanim(ZREF rLink, const char* name);
    bool Zlink__Checkanimrunning(ZREF rLink, int animId);
    int Zlink__Getnumberofframes(ZREF rLink, anim animId);
    int Zlink__Playanimsegment(ZREF rLink, anim animId, int segment, float start, float end, float blendTime);
    bool Zlink__Canplayanim(ZREF rLink, anim animId, int segment);
    bool Zlink__Canplayanimsegment(ZREF rLink, anim animId, float start, float end, float blendIn, float blendOut);
    int Zlink__Playaniminterpolated(ZREF rLink, anim animId, v3 position, v3 direction, float start, float end, float blendIn, float blendOut, int flags);
    int Zlink__Setactiveanimdest(ZREF rLink, v3 position, v3 direction, float blendTime);
    int Zlink__Getactiveanimdest(ZREF rLink, v3& position, v3& direction);
    void Zlink__Lockfacing(ZREF rLink, ZREF rTarget);
    int Zlink__Getmetakeyframenr(ZREF rLink, anim animId, eMeta meta);
    int Zlink__Getmetakeynrframenr(ZREF rLink, anim animId, eMeta meta, int metaIndex);
    bool Zlink__Getanimendpoint(ZREF rLink, anim animId, ZREF rBase, v3& endpoint);
    int Zlink__Getmetakeyframenrfrommetastring(ZREF rLink, anim animId, const char* meta);
    void Zlink__Setholdweaponubanim(ZREF rLink, anim animId);
}
