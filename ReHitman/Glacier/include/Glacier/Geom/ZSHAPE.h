#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/SSplineMover.h> // ParametricCurve


namespace Glacier
{
    struct BasicShape
    {
        // vtbl
        virtual bool Save(struct ZSaveGame*);
        virtual bool Load(struct ZLoadGame*);

        // methods
        BasicShape();
        bool IsNormalized() const;
        void DefineBezierSpline(ParametricCurve* pCurves, const float (*aVertices)[3], int lNrSegments, float fTess);
        float GetLen() const;
        float CalcShapeLen() const;
        void GetSplinePos(float (*pPos)[3], float t) const;
        void GetSplineVelocity(float (*pVel)[3], float t) const;
        void GetSplinePosVelocity(float (*pPos)[3], float (*pVel)[3], float t) const;

        // data
        float m_fLen;
        float m_fMinSegLen;
        ParametricCurve* m_pSegments;
        int32_t m_lNrSegment;
    };

    class ZSHAPE : public ZGEOM
    {
    public:
        // types
        struct SShapeParametricCurve
        {
            uint32_t m_iReferences;
            ParametricCurve m_aCurves[1];   // segments follow inline; allocation is larger
        };

        // RTTI
        DECLARE_GEOM_CLASS(ZSHAPE, 0x4000022u);

        // vtbl
        ~ZSHAPE() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void CopyData(const ZGEOM* Source) override;

        // methods
        ZSHAPE(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        SShapeParametricCurve* m_pShapeSegments;
        BasicShape             m_S;
    };
    RE_VERIFY_SIZE(ZSHAPE, 0x28);  // Verified PC alloc
}
