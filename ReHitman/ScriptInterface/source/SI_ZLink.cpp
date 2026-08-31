#include <SI/SI_Common.h>
#include <SI/SI_ZLink.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier
{
    void Zlink__Stopallanims(ZREF rLink)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return;

        pLnkObj->StopAllAnims(false);
    }

    void Zlink__Stopubanims(ZREF rLink)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return;

        pLnkObj->StopUBAnims();
    }

    anim Zlink__Getanim(ZREF rLink, const char* name)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return -1;

        auto rAnim = pLnkObj->GetAnimVariationHandle(name);
        if (!rAnim.IsValid())
            return -1;

        return rAnim.iIndex;
    }

    bool Zlink__Checkanimrunning(ZREF rLink, int animId)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return false;

        ZAnimVariationHandle hAnim;
        hAnim.iIndex = animId;

        if (!hAnim.IsValid())
        {
            ZERROR("Invalid animation handle used by script on %s", pLnkObj->Name());
            return false;
        }

        return pLnkObj->IsAnimationRunning(animId) != nullptr;
    }

    int Zlink__Getnumberofframes(ZREF rLink, anim animId)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return -1;

        ZAnimVariationHandle hAnim;
        hAnim.iIndex = animId;

        if (!hAnim.IsValid())
        {
            ZERROR("Invalid animation handle used by script on %s", pLnkObj->Name());
            return -1;
        }

        auto* pHeader = pLnkObj->GetAnimHeaderFromVariation(hAnim, 0, 0.0f);
        if (!pHeader)
        {
            return -1;
        }

        return pHeader->Length();
    }

    int Zlink__Playanimsegment(ZREF rLink, anim animId, int dwmode, float start, float end, float speed)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return -1;

        ZAnimVariationHandle hAnim;
        hAnim.iIndex = animId;

        if (!hAnim.IsValid())
        {
            ZERROR("Invalid animation handle used by script on %s", pLnkObj->Name());
            return -1;
        }

        auto* pHeader = pLnkObj->GetAnimHeaderFromVariation(hAnim, 0, 0.0f);
        if (!pHeader)
        {
            return -1;
        }

        return pLnkObj->PlayAnimSegment(pHeader, dwmode, start, end, speed);
    }

    bool Zlink__Canplayanim(ZREF rLink, anim animId, int segment)
    {
        auto* pLnkObj = GetValidLnkObj(rLink);
        if (!pLnkObj)
            return false;

        if (!pLnkObj->IsDerivedFrom<ZIKLNKOBJ>())
        {
            ZERROR("%s is not derived from ZIKLNKOBJ!", pLnkObj->Name());
            return false;
        }

        ZAnimVariationHandle hAnim;
        hAnim.iIndex = animId;

        if (!hAnim.IsValid())
        {
            ZERROR("Invalid animation handle used by script on %s", pLnkObj->Name());
            return false;
        }

        auto* pHeader = pLnkObj->GetAnimHeaderFromVariation(hAnim, 0, 0.0f);
        if (!pHeader)
            return false;

        auto* pIKLnkObj = static_cast<ZIKLNKOBJ*>(pLnkObj);
        return pIKLnkObj->CanPlayAnimSegment(pHeader, 0.0f, static_cast<float>(pHeader->Length()), segment < 0);
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
