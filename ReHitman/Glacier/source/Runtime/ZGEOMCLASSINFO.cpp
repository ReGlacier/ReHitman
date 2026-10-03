#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZGEOMCLASSINFO::ZGEOMCLASSINFO(const ZGEOMCLASSINFO& copy) = default;
	ZGEOMCLASSINFO::ZGEOMCLASSINFO(const char *szClassInfoName, uint32_t lSize, const char *szParentClass, uint32_t lType, uint32_t lGeomCases, const char *szParameters, SetTypeIdAndMaskFunc_t pSetter, uint32_t* pClassId, uint32_t* pMaskId)
        : ZNonResourceClassInfo(szClassInfoName, 0, lSize, szParameters)
        , m_pSetTypeIDAndMask(pSetter)
        , m_lType(lType)
        , m_lGeomCases(lGeomCases)
        , m_szParentClass(szParentClass)
        , m_pClassId(pClassId)
        , m_pMaskId(pMaskId)
    {
    }
		
    ZGEOMCLASSINFO::~ZGEOMCLASSINFO() = default;

    bool ZGEOMCLASSINFO::IsDerivedFrom(const ZGEOMCLASSINFO* pOther) const
    {
        if (!pOther) return false;

        const auto iSelfClassId = GetClassId();
        const auto iSelfMaskId = GetMaskId();

        return iSelfMaskId == pOther->GetClassId();
    }

    uint32_t ZGEOMCLASSINFO::GetClassId() const
    {
        ZASSERT(m_pClassId != nullptr);
        return *m_pClassId;
    }

    uint32_t ZGEOMCLASSINFO::GetMaskId() const
    {
        ZASSERT(m_pMaskId != nullptr);
        return *m_pMaskId;
    }

    const char* ZGEOMCLASSINFO::ParentClass() const
    {
        return m_szParentClass;
    }

    void ZGEOMCLASSINFO::SetTypeIDAndMask(uint32_t lType, uint32_t lMask)
    {
        ZASSERT(m_pSetTypeIDAndMask);
        m_pSetTypeIDAndMask(lType, lMask);
    }

    uint32_t ZGEOMCLASSINFO::Type() const
    {
        return m_lType;
    }
}