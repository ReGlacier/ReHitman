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

        // data
        float m_fLen;
        float m_fMinSegLen;
        ParametricCurve* m_pSegments;
        int32_t m_lNrSegment;
    };

    class ZSHAPE : public ZGEOM
    {
    public:
        STATIC_CLASS_VAR(ZSHAPE, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZSHAPE, uint32_t, m_Mask);

        // types
        struct SShapeParametricCurve 
        {
            uint32_t         m_iReferences;
            ParametricCurve* m_pSegments;
        };

        // vtbl (no changes)
        // data        
        SShapeParametricCurve* m_pShapeSegments;
        BasicShape             m_S;
    };
    RE_VERIFY_SIZE(ZSHAPE, 0x28);
}
