#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct ZBone;
    struct SBoneDefinition;
    struct ZBoneQuat;
    struct PPart_vtbl;

    template <typename T1, typename T2>
    struct vector_if
    {
        T1* m_pDataStart;
        T2 m_iSize;
    };

    struct PPart
    {
        // vtbl
        virtual void SetBonePtr(ZBone*);
        virtual void SetBonePtr(ZBoneQuat*);
        virtual void SetBoneDefsPtr(SBoneDefinition*);
        virtual void SetOriginalBonePtr(const ZBone*);

        // members
        union
        {
            vector_if<ZBoneQuat,unsigned short> BonesL;
            vector_if<ZBone,unsigned short> BonesG;
        };
        vector_if<SBoneDefinition,unsigned short> BoneDefs;
        vector_if<ZBone,unsigned short> OriginalBonesG;
    };
    RE_VERIFY_SIZE(PPart, 0x1C); // Verified by PS2 & XBox360 Mini Ninjas
}