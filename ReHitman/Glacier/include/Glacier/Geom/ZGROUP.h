#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    // fwds
    class ZBaseGeom;

    class ZGROUP : public ZGEOM
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZGROUP, 0x100001u);

        // ZGROUP Control flags
        static constexpr uint32_t ZGRPCF_OVERRIDE_NEAR_FAR = 0x200u;
        static constexpr uint32_t ZGRPCF_INVALID_BOUNDS = 0x8000u;
        static constexpr uint32_t ZGRPCF_GROUP_CONTAINS_LIGHT = 0x10000u;
        static constexpr uint32_t ZGRPCF_LIGHT_SHINES_IN = 0x1000000u;
        static constexpr uint32_t ZGRPCF_LIGHT_SHINES_OUT = 0x2000000u;
        static constexpr uint32_t ZGRPCF_RESET = 0x40000000u;

        // vtbl
        ~ZGROUP() override;

        // ZSerializable
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        bool DoInit() override;
        void CalcCenSize() override;
        void PreSaveGame() override;
        eGlobalTreeType GetBoundTreeType() const override;
        void RecurGetNext(ZBaseGeom** ZGeom) override;
        void SetRootTM(const ZMat3x3& RTMat, const ZVector3& RTPos) override;
        void Freeze(bool OnOff) override;
        void SetMoving(bool bMoving) override;
        void SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget) override;
        bool CheckPointInside(ZVector3& pPoint, float fDotDist) override;
        bool CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* vHalfSize) override;
        float GetPointInsideDistance(const ZVector3& vPos) override;
        ZGEOM* Duplicate(ZGROUP* DestGroup, const char* DupName, bool Recursive) override;
        ZGEOM* DuplicateToResource(ZGROUP* DestGroup, uint32_t lGeomResourceId, const char* DupName, bool Recursive) override;
        ZGEOM* DuplicateToResourceInit(ZGROUP* DestGroup, uint32_t lGeomResourceId, const ZMat3x3* mMat, const ZVector3* vPos, const char* DupName, bool Recursive) override;
        void CopyData(const ZGEOM* Source) override;

        // ZGROUP
        virtual bool IsRecursiveActivateAllowed();
        virtual bool DynamicGroupOnScreen();
        virtual ZGEOM* FindLoadWorldGeom(const char* pSearchName) const;
        virtual ZGEOM* FindMasterGeom(const char* pSearchName) const;
        virtual ZGEOM* FindGeom(const char* GName, ZBaseGeom* pZGeomContinue);
        virtual int GroupDepth();
        virtual float GetPFResMultiplier() const;
        virtual void LinkBound(ZREF rBound);
        virtual void RemoveBound(ZREF rBound);
        virtual void GetAmbientSettings(const ZVector3& p0, float* AmbientDir, float* AmbientLowColor, float* AmbientHiColor) const;
        virtual void CreateParentsRecur(ZGROUP* ZParent, ZGROUP** ZDest, bool bAllowLoadWorlds);
        virtual void SetOverRideNearFar(const ZVector2& NearFar);
        virtual void CorrectCenSizeRecur();
        virtual void CorrectCenSize();
        virtual void InvalidateBounds();
        virtual void AttachGeom(ZGEOM* pGeom, bool bCalcMinMax);
        virtual void AttachGeom(ZBaseGeom* pBaseGeom, bool bCalcMinMax);
        virtual void DetachGeom(ZBaseGeom* pBaseGeom, bool bDestroying);
        virtual void RecurGetNextGroup(const ZBaseGeom** pGroup) const;
        virtual void RecurGetNextExclRoom(const ZBaseGeom** ZGeom) const;
        virtual void SetGroupControl(uint32_t lAddBits, uint32_t lRemBits);
        virtual uint32_t GroupControl() const;
        virtual void ResetGroupPosition(bool bReset);
        virtual void MakeActiveRecursive();
        virtual ZBaseGeom** GetStaticLights(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd);
        virtual void CalcCenSizeRecur();
        virtual void GetCenSizeRecur(ZVector3& vCen, ZVector3& vSize, bool bIgnoreHidden);
        virtual ZGEOM* FindMaskGeom(const char* pSearchName, int32_t lMask) const;

        // methods
        ZGROUP(const char* psName, ZBaseGeom* pBaseGeom);

        ZGEOM* CreateResourceGeom(const char* pName, uint32_t iGeomResourceId, uint32_t lGeomClassType, bool bCalcMinMax);
        ZGEOM* CreateGeom(const char* pName, uint32_t iGeomClassId, bool bCalcMinMax);
        bool IsRoot() const;
        void GroupContainsLight();

#       pragma region " --- RTTI Methods --- "
        void GetLightShinesIn(bool& bLightShinesIn);
        void SetLightShinesIn(const bool& bLightShinesIn);
        void GetLightShinesOut(bool& bLightShinesOut);
        void SetLightShinesOut(const bool& bLightShinesOut);
#       pragma endregion

        // members
        float m_vSizeInsideCheck[3];
        float m_vCenInsideCheck[3];
        uint32_t m_lGroupCon;
        float m_OverRideNearFar[2];
        REFTAB* m_pZBounds;
        uint32_t m_LightList;
        ZBaseGeom* m_pGroupFirst;
        ZBaseGeom* m_pGroupLast;
        float m_fPFResMultiplier;
        uint16_t m_NrAttachGeom;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZGROUP, 0x4C); // Verified

    bool ForGroupsCheck(ZBaseGeom* pBaseGeom);
}
