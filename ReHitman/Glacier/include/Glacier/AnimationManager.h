#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    namespace Animation
    {
        class Header
        {
            uint16_t m_OrgStartFrame;
            uint16_t m_Frames;
            uint16_t m_RealFPS;
            uint16_t m_MetaDataEntries;
            int m_Mask;
            int m_NewAnimPos;
            int m_MetaDataOffset;
            float m_BlendFrames;
            float m_vGroundDisplacement[3];
            float m_fGroundRange;
            int m_Size;
            int m_OldControl;
            int m_SoundIndex;
            float m_AimDirection[3];
            float m_AimPosition[3];
            char* m_Name;
        };
        RE_VERIFY_SIZE(Header, 0x50); // NOT VERIFIED, IN PS2 struct different and have size 0x40

        class CrowdHeader
        {
            char m_szName[32];
            int m_iOffset;
            int m_iFrames;
        };
        RE_VERIFY_SIZE(CrowdHeader, 0x28);

        struct ZNameList
        {
            char* m_Names;
            int m_Size;
            int m_Count;
        };
        RE_VERIFY_SIZE(ZNameList, 0xC);

        class Manager
        {
        public:
            // Data
            Header* m_Headers;
            int m_Animcount;
            CrowdHeader* m_pCrowdHeaders;
            int m_iCrowdAnimCount;
            char* m_Data;
            int m_Pos;
            int m_MaxPos;
            int m_OwnsBuffers;
            int m_StateSize;
            int m_HumanQuatSize;
            int m_QuatSize;
            int m_PoseSize;
            bool m_PlayUncompressed;
            bool m_pad25[3];
            ZNameList m_BoneNames;
            ZNameList m_AnimNames;
            ZNameList m_PoseNames;
            void* m_Cache;
            int m_SizeUncompressed;
            char* m_pMetaKeyData;
            char* m_pMetaKeyDataStrings;
        };
        RE_VERIFY_SIZE(Manager, 0x68); // Verified by ZEngineDataBase::AllocSequence method
    }
}