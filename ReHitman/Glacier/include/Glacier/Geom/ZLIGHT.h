#pragma once

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZLIGHT : public ZGEOM
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZLIGHT, 0x80800004u);

        // constants
        static constexpr uint32_t ZL_EFFECTS_LIGHT = 0x2u;
        static constexpr uint32_t ZL_USE_PARENT_BOUNT_ADJUST = 0x4u;
        static constexpr uint32_t ZL_ATTRIBUTE_CHANGED = 0x8u;

        // vtbl
        ~ZLIGHT() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void Activate(bool bActive) override;
        void CalcCenSize() override;
        bool DisableParentBoundAdjust() override;
        void ClassInit() override;
        void CopyData(const ZGEOM* Source) override;
        eGlobalTreeType GetBoundTreeType() const override;

        // ZLIGHT
        virtual void Enable();
        virtual void Disable();
        virtual void SetMultiplier(float fValue); // In default implementation it's do nothing
        virtual void SetDiffuseColor(unsigned int rgba); // In default implementation it's do nothing
        virtual void MakeLightUnique();

        // methods
        ZLIGHT(const char* psName, ZBaseGeom* pBaseGeom);
        void SetLightCon(uint32_t lAdd, uint32_t lRemove);
        uint32_t LightCon() const;
        void ActivateStaticShadows(bool bActive);
        void SetLightAttributeChanged(bool bChanged);
        void SetEffectLight(bool bEffect);
        bool LightAttributeChanged() const;
        bool EffectLight() const;
        bool IsGeomExcluded(ZREF rGeom) const;

#       pragma region " --- RTTI Methods --- "
        void GetParentBound(bool& bUseParentBound);
        void SetParentBound(const bool& bUseParentBound);
        void GetExcludeList(REFTAB32& aExcludeList);
        void SetExcludeList(const REFTAB32& aExcludeList);
        void GetStaticShadowList(REFTAB32& aStaticList);
        void SetStaticShadowList(const REFTAB32& aStaticList);
#       pragma endregion

        // members
        uint32_t m_lLightCon;
        uint32_t* m_pExcludeList;
        uint32_t m_rMasterLight;
        uint32_t* m_pStaticShadowList;
    };
    RE_VERIFY_SIZE(ZLIGHT, 0x20); // Verified
}
