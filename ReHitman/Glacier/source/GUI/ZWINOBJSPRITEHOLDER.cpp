#include <Glacier/GUI/ZWINOBJSPRITEHOLDER.h>
#include <Glacier/Com/CComRead.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    ZWINOBJSPRITEHOLDER::ZWINOBJSPRITEHOLDER(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIST(psName, pBaseGeom)
    {
        CCom* pSceneCom = g_pEngineData->GetSceneCom();
        const ZREF rExisting = CComRead(pSceneCom, "rWINOBJSPRITEHOLDER");
        ZGEOM* pExisting = ZGEOM::RefToPtr(rExisting);
        ZASSERT(!pExisting || pExisting == this);
        ZASSERT(!rExisting || rExisting == GetRef());
        pSceneCom->SetVal("rWINOBJSPRITEHOLDER", GetRef(), CCOM_TYPE_REF);
    }

    ZWINOBJSPRITEHOLDER::~ZWINOBJSPRITEHOLDER() = default;

    const RTP::ZPropertyInfo& ZWINOBJSPRITEHOLDER::GetProperties() const
    {
        return ZWINOBJSPRITEHOLDER::Info;
    }

    uint32_t ZWINOBJSPRITEHOLDER::GetObjectId() const
    {
        return ZWINOBJSPRITEHOLDER::m_Id;
    }

    void ZWINOBJSPRITEHOLDER::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWINOBJSPRITEHOLDER::m_Id;
        mask = ZWINOBJSPRITEHOLDER::m_Mask;
    }

    ZGEOMCLASSINFO* ZWINOBJSPRITEHOLDER::GetOldClassInfo() const
    {
        return ZWINOBJSPRITEHOLDER::m_OldClassInfo;
    }

    REFTAB* ZWINOBJSPRITEHOLDER::GetFreeSpriteArraysList()
    {
        CCom* pSceneCom = g_pEngineData->GetSceneCom();
        const ZREF rHolder = CComRead(pSceneCom, "rWINOBJSPRITEHOLDER");
        ZASSERT(rHolder != 0);

        auto* pHolder = geom_cast<ZWINOBJSPRITEHOLDER>(ZGEOM::RefToPtr(rHolder));
        return pHolder ? &pHolder->m_rtFreeSpriteArrays : nullptr;
    }

    void ZWINOBJSPRITEHOLDER::SaveCleanup(bool)
    {
    }


#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(ZWINOBJSPRITEHOLDER, ZLIST, 0x009A2978, "ZWINOBJSPRITEHOLDER", 0x0077CFAC, nullptr, 0x0080E54C, 0x009A27E4, 0x009A27E8);
#   pragma endregion
}
