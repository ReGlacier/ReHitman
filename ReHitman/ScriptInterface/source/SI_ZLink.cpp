#include <SI/SI_ZLink.h>

namespace Glacier
{
    void Zlink__Stopallanims(ZREF rLink)
    {
        // TODO: Finish me
    }

    void Zlink__Stopubanims(ZREF rLink)
    {
        // TODO: Finish me
    }

    anim Zlink__Getanim(ZREF rLink, const char* name)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zlink__Checkanimrunning(ZREF rLink, int animId)
    {
        // TODO: Finish me
        return false;
    }

    int Zlink__Getnumberofframes(ZREF rLink, anim animId)
    {
        // TODO: Finish me
        return 0;
    }

    int Zlink__Playanimsegment(ZREF rLink, anim animId, int segment, float start, float end, float blendTime)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zlink__Canplayanim(ZREF rLink, anim animId, int segment)
    {
        // TODO: Finish me
        return false;
    }

    bool Zlink__Canplayanimsegment(ZREF rLink, anim animId, float start, float end, float blendIn, float blendOut)
    {
        // TODO: Finish me
        return false;
    }

    int Zlink__Playaniminterpolated(ZREF rLink, anim animId, v3 position, v3 direction, float start, float end, float blendIn, float blendOut, int flags)
    {
        // TODO: Finish me
        return 0;
    }

    int Zlink__Setactiveanimdest(ZREF rLink, v3 position, v3 direction, float blendTime)
    {
        // TODO: Finish me
        return 0;
    }

    int Zlink__Getactiveanimdest(ZREF rLink, v3& position, v3& direction)
    {
        // TODO: Finish me
        return 0;
    }

    void Zlink__Lockfacing(ZREF rLink, ZREF rTarget)
    {
        // TODO: Finish me
    }

    int Zlink__Getmetakeyframenr(ZREF rLink, anim animId, eMeta meta)
    {
        // TODO: Finish me
        return 0;
    }

    int Zlink__Getmetakeynrframenr(ZREF rLink, anim animId, eMeta meta, int metaIndex)
    {
        // TODO: Finish me
        return 0;
    }

    bool Zlink__Getanimendpoint(ZREF rLink, anim animId, ZREF rBase, v3& endpoint)
    {
        // TODO: Finish me
        return false;
    }

    int Zlink__Getmetakeyframenrfrommetastring(ZREF rLink, anim animId, const char* meta)
    {
        // TODO: Finish me
        return 0;
    }

    void Zlink__Setholdweaponubanim(ZREF rLink, anim animId)
    {
        // TODO: Finish me
    }
}
