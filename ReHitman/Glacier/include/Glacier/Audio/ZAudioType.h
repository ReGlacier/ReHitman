#pragma once

#include <Glacier/Audio/ZAudioTypes.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>

#include <cstddef>
#include <cstdint>

namespace Glacier
{
    struct IAudioTypeList
    {
        uint32_t m_lEntryOffset;
        uint16_t m_lEntryCount;
        uint16_t m_lEntryCapacity;
    };

    template <typename TEntry, typename TOwner>
    struct TAudioTypeList : IAudioTypeList
    {
    };

    class ZAudioTypeBase
    {
    public:
        struct ZPackedBase
        {
            ZPackedBase();

            template <typename T>
            T& GetAs()
            {
                return *static_cast<T*>(this);
            }

            template <typename T>
            const T& GetAs() const
            {
                return *static_cast<const T*>(this);
            }

            ZAudioTypes::EType m_Type;
            uint8_t m_Filler1;
            RE_ADD_PADDING(3);
            uint32_t m_ResourceNameOffset;
            uint16_t m_lControllerCount;
            RE_ADD_PADDING(2);
            uint32_t m_lControllerOffset;
        };

        // vtbl
        virtual ~ZAudioTypeBase();
        virtual ZAudioTypes::EType GetType() const;
        virtual ZPackedBase& GetPackedTemplate();
        virtual ZPackedBase* GetNewPackedTemplate();
        virtual size_t GetPackedTemplateSize() const;
        virtual bool IsPackingUnique() const;
    };

    template <ZAudioTypes::EType Type>
    class ZAudioType;

    template <ZAudioTypes::EType Type>
    class ZAudioTypeTemplate : public ZAudioTypeBase
    {
    public:
        struct TPacked : ZPackedBase
        {
            TPacked()
            {
                this->m_Type = Type;
            }

            static ZAudioTypes::EType GetStaticType()
            {
                return Type;
            }
        };

        ZAudioTypes::EType GetType() const override
        {
            return Type;
        }

        ZPackedBase& GetPackedTemplate() override
        {
            static typename ZAudioType<Type>::ZPacked packed;
            return packed;
        }

        ZPackedBase* GetNewPackedTemplate() override
        {
            return ZUniMemory::New<typename ZAudioType<Type>::ZPacked>();
        }

        size_t GetPackedTemplateSize() const override
        {
            return sizeof(typename ZAudioType<Type>::ZPacked);
        }
    };

    template <ZAudioTypes::EType Type>
    class ZAudioType : public ZAudioTypeTemplate<Type>
    {
    public:
        struct ZPacked : ZAudioTypeTemplate<Type>::TPacked
        {
        };
    };

    struct SSequenceKey
    {
        int32_t m_lType;
        int32_t m_lTime;
        uint32_t m_lSoundOffset;
    };

    template <>
    class ZAudioType<ZAudioTypes::Sound> : public ZAudioTypeTemplate<ZAudioTypes::Sound>
    {
    public:
        struct ZPacked : TPacked
        {
            uint32_t m_lWaveOffset;
            float m_fMinDist;
            int16_t m_lPitch;
            int16_t m_InnerConeAngle;
            int16_t m_OuterConeAngle;
            int16_t m_OuterConeVolume;
            int16_t m_LPCutOffFreq;
            int16_t m_lPan;
            uint8_t m_SourceType;
            uint8_t m_Volume;
            uint8_t m_lFlags;
            uint8_t m_iMaxDistanceModel;
            uint8_t m_lLoops;
            RE_ADD_PADDING(3);
        };

        bool m_bLoadingSound;
    };

    template <>
    class ZAudioType<ZAudioTypes::Set> : public ZAudioTypeTemplate<ZAudioTypes::Set>
    {
    public:
        struct ZPacked : TPacked
        {
            TAudioTypeList<uint32_t, ZPackedBase> m_Entries;
            uint32_t m_PlayedBitTable;
            uint32_t m_PlayType;
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::Seq> : public ZAudioTypeTemplate<ZAudioTypes::Seq>
    {
    public:
        struct ZPacked : TPacked
        {
            TAudioTypeList<SSequenceKey, SSequenceKey> m_Keys;
            uint8_t m_MasterVolume;
            uint8_t m_lLoops;
            RE_ADD_PADDING(2);
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::Wave> : public ZAudioTypeTemplate<ZAudioTypes::Wave>
    {
    public:
        struct ZPacked : TPacked
        {
            enum EFlags
            {
                eStreamed = 0x1,
                eLocalized = 0x2,
                eLoadingSound = 0x4,
                eIsLocalizedArray = 0x8
            };

            uint32_t m_lHeaderOffset;
            float m_fDuration;
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::Def> : public ZAudioTypeTemplate<ZAudioTypes::Def>
    {
    public:
        struct ZPacked : TPacked
        {
            TAudioTypeList<uint32_t, uint32_t> m_Entries;
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::Mapping> : public ZAudioTypeTemplate<ZAudioTypes::Mapping>
    {
    public:
        struct ZPacked : TPacked
        {
            uint32_t m_lSoundOffset;
            uint32_t m_lPart1Offset;
            uint32_t m_lPart2Offset;
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::ObjectProperty> : public ZAudioTypeTemplate<ZAudioTypes::ObjectProperty>
    {
    public:
        struct ZPacked : TPacked
        {
            uint32_t m_lMaterialId;
            uint32_t m_lSize;
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::SurfaceProperty> : public ZAudioTypeTemplate<ZAudioTypes::SurfaceProperty>
    {
    public:
        struct ZPacked : TPacked
        {
            uint32_t m_lMaterialId;
            int32_t m_lOcclusion;
            float m_fOcclusionLFRatio;
            float m_fOcclusionRoomRatio;
        };
    };

    template <>
    class ZAudioType<ZAudioTypes::Group> : public ZAudioTypeTemplate<ZAudioTypes::Group>
    {
    public:
        struct ZPacked : TPacked
        {
            struct ZValues
            {
                float m_fVolumeAttenuation;
                float m_fPitch;
                uint16_t m_lMaxChannels;
                RE_ADD_PADDING(2);
            };

            TAudioTypeList<uint32_t, ZPacked> m_Childs;
            TAudioTypeList<uint32_t, uint32_t> m_Actives;
            ZValues m_Values;
            ZValues m_CalculatedValues;
            uint32_t m_NameOffset;
            uint32_t m_MasterOffset;
            int16_t m_lChannelsPlaying;
            bool m_bExposeToUser;
            bool m_bIsDefault;
            bool m_bUseFade;
            bool m_bCrossFade;
            bool m_bUseMasterPitch;
            bool m_bUseMaxChannelsPlaying;
        };
    };

    using SSound = ZAudioType<ZAudioTypes::Sound>::ZPacked;
    using SSoundSet = ZAudioType<ZAudioTypes::Set>::ZPacked;
    using SSoundSequence = ZAudioType<ZAudioTypes::Seq>::ZPacked;
    using SWave = ZAudioType<ZAudioTypes::Wave>::ZPacked;
    using SSoundDef = ZAudioType<ZAudioTypes::Def>::ZPacked;
    using SMapping = ZAudioType<ZAudioTypes::Mapping>::ZPacked;
    using SObjectProperty = ZAudioType<ZAudioTypes::ObjectProperty>::ZPacked;
    using SSurfaceProperty = ZAudioType<ZAudioTypes::SurfaceProperty>::ZPacked;
    using SSoundGroup = ZAudioType<ZAudioTypes::Group>::ZPacked;

    RE_VERIFY_SIZE(IAudioTypeList, 0x08);
    RE_VERIFY_SIZE(ZAudioTypeBase::ZPackedBase, 0x14);
    RE_VERIFY_SIZE(SSequenceKey, 0x0C);
    RE_VERIFY_SIZE(SSound, 0x30);
    RE_VERIFY_SIZE(SSoundSet, 0x24);
    RE_VERIFY_SIZE(SSoundSequence, 0x20);
    RE_VERIFY_SIZE(SWave, 0x1C);
    RE_VERIFY_SIZE(SSoundDef, 0x1C);
    RE_VERIFY_SIZE(SMapping, 0x20);
    RE_VERIFY_SIZE(SObjectProperty, 0x1C);
    RE_VERIFY_SIZE(SSurfaceProperty, 0x24);
    RE_VERIFY_SIZE(SSoundGroup::ZValues, 0x0C);
    RE_VERIFY_SIZE(SSoundGroup, 0x4C);
}
