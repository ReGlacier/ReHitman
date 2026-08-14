#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Animation/ZBone.h>

#include <tuple>
#include <bit>


namespace Glacier
{
    namespace Animation
    {
        struct Model;
    }

    RE_PACKED_STRUCT(1)
    struct SPoseKey {
        uint16_t Frame;             //+0x00
        uint8_t Strength;           //+0x02
    } RE_PACKED_STRUCT_END;
    RE_VERIFY_SIZE(SPoseKey, 0x3);

    struct alignas(4) ZPoseModel
    {
        // methods
        ZPoseModel();

        void ResetIndex();
        uint16_t GetIndex();
        void SetIndex(uint16_t index);
        float GetWeight(float fWeight);
        void SetName(char cName);
        void SetActive(bool bIsActive);
        void SetKeys(SPoseKey* pKey);
        void SetSize(uint16_t size);
        uint16_t Size() const;
        char Name() const;
        bool Active() const;

        // members
        uint8_t m_cName;
        SPoseKey* m_pPoseKeys;
        uint16_t m_dwIndex;
        uint16_t m_dwSize;
        bool m_bActive;
        bool m_bLoaded;
    };
    RE_VERIFY_SIZE(ZPoseModel, 0x10);

    struct ZPoseAnim
    {
        // methods
        ZPoseAnim();
        ZPoseAnim(uint32_t rHost);
        ~ZPoseAnim();

        void SetID(uint32_t lId);
        uint32_t GetID() const;
        bool Create(char* pBuffer);
        void UpdateData(char* pBuffer);
        void UpdateFrame();
        int8_t GetExpressionID() const;
        uint8_t Count() const;
        uint8_t StartCount() const;
        float StartTime() const;
        float TimeMultiplier() const;
        float EndTime() const;
        float FrameTime() const;
        float FrameStartTime() const;
        uint8_t ActivePoses() const;
        uint32_t PoseSize() const;
        void SetStartTime(float);
        void SetTimeMultiplier(float);
        void SetEndTime(float);
        void SetCount(int);
        void SetStartCount(int);
        void SetActivePoses(uint8_t);
        void SetPoseSize(uint32_t);
        void SetFrameStartTime(float);
        void SetFrameTime(float);
        void SetEmotionID(uint16_t);
        float GetPoseWeight(int lIdx, float fWeight);
        char GetPoseName(int lIdx);
        void SetHost(uint32_t rHost);
        uint32_t GetHost() const;

        // members
        uint32_t m_rHost;
        uint32_t m_dwID;
        uint8_t m_lCount;
        uint8_t m_lStartCount;
        uint8_t m_lActivePoses;
        uint8_t m_padB;
        float m_fStartTime;
        float m_fTimeMultiplier;
        float m_fEndTime;
        uint32_t m_lPoseSize;
        float m_fFrameTime;
        float m_fFrameStartTime;
        uint8_t m_iUpdate;
        uint8_t m_pad25;
        uint16_t m_lEmotionID;
        ZPoseModel* m_pPoseList;
    };
    RE_VERIFY_SIZE(ZPoseAnim, 0x2C);

    struct ActBoneMotion2
    {
        union
        {
            char _DefaultLocalQuat[4];
            uint32_t m_PackedQuat;
        };
    };
    RE_VERIFY_SIZE(ActBoneMotion2, 0x4);
    
    struct SFlagOffsetPair
    {
        // methods
        SFlagOffsetPair() = default;

        SFlagOffsetPair(const SFlagOffsetPair& copy)
            : iFlags(copy.iFlags)
            , iOffset(copy.iOffset)
        {}

        SFlagOffsetPair& operator=(const SFlagOffsetPair& copy)
        {
            iFlags = copy.iFlags;
            iOffset = copy.iOffset;
            return *this;
        }

        int GetNumSetFlags(uint32_t flags) const
        {
            const uint32_t iMasked = iFlags & flags;
            return std::popcount(iMasked);
        }

        bool operator<(const SFlagOffsetPair& rhs) const
        {
            return std::tie(iFlags, iOffset) < std::tie(rhs.iFlags, rhs.iOffset);
        }

        // members
        uint32_t iFlags { 0u };
        int32_t iOffset { 0 };
    };
    RE_VERIFY_SIZE(SFlagOffsetPair, 0x8);

    struct ZAnimVariation
    {
        // methods
        ZAnimVariation();
        
        void SetData(const char* pData);
        int32_t GetNumVariations() const;
        uint32_t GetAnimOffset(uint32_t lId, float fRand) const;
        bool IsValid() const;
        float AnimWeight(uint32_t mask) const;
        SFlagOffsetPair* GetPairs() const;

        // members
        const char* m_Data{ nullptr };
    };
    RE_VERIFY_SIZE(ZAnimVariation, 0x4); // Not confirmed yet

    struct GameEntity
    {
        ZGEOM* m_pGeom;
        // Need finish this part
    };
    // RE_VERIFY_SIZE(GameEntity, 0x18);

    struct ZAnimVariationBuffer
    {
        // methods
        ZAnimVariationBuffer();
        void SetBuffer(const char* pBuffer);
        bool IsValid() const;
        bool GetAnimVariation(ZAnimVariation& variation, ZAnimVariationHandle& handle);
        void FindAnimListStart();
        const char* GetBuffer() const;
         
        // members
        const char* m_pBuffer{ nullptr };
        int* m_AnimList{ nullptr };
    };
    RE_VERIFY_SIZE(ZAnimVariationBuffer, 0x8);

    struct ZAnimTemplatesNames
    {
        // methods
        ZAnimTemplatesNames();
        bool FindAnimVariationHandle(ZAnimVariationHandle& handle, const char* pszName);
        void SetBuffer(const char* pBuffer);
        bool Init();
        int32_t GetAnimCount() const;
        bool GetAnim(int, ZAnimVariationHandle& handle);
        const char* GetNameFromAnimVariationHandle(const ZAnimVariationHandle& handle);

        // members
        const char* m_pBuffer{ nullptr };
    };
    RE_VERIFY_SIZE(ZAnimTemplatesNames, 0x4);

    struct ZDeltaBone
    {
        // methods
        ZDeltaBone();
        ZDeltaBone(const ZDeltaBone& copy);
        ZDeltaBone& operator=(const ZDeltaBone& copy);
        void LoadSave(ISerializerStream& stream, bool bSaving);

        // members
        float m_OffsetQuat[4];
        float m_OffsetPos[3];
    };
    RE_VERIFY_SIZE(ZDeltaBone, 0x1C);

    class ZLNKOBJ : public ZSTDOBJ
    {
    public:
        // types
        struct SAnimSound
        {
            unsigned int m_rAnimationSound;
            int m_lSequenceID;
        };
        RE_VERIFY_SIZE(SAnimSound, 0x8);

        // vtbl
        virtual void InitObjMatBone();
        virtual void CloseObjMatBone();
        virtual void* GetAnim(const char*);
        virtual void StopAllAnims(bool);
        virtual void StopUBAnims();
        virtual void StopAnim(Animation::ActiveAnimation *);
        virtual void ActivateAnimSegment(Animation::Header*, int, float, float, float);
        virtual void ActivateAnimSegment(ZAnimVariationHandle, int, float, float, float);
        virtual void ActivateAnim(Animation::Header*, int);
        virtual void SetActiveAnimDestination(float const*, float const*, float);
        virtual bool CheckActiveAnim(Animation::Header*, int);
        virtual void SetBoneFrameBlend(Animation::Header*, float, float, bool, unsigned int);
        virtual void SetDualFrame(Animation::Header*, float, Animation::Header*, float, float, float);
        virtual bool GroundAnimated();
        virtual void OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, char const*);
        virtual void ActivatePoseAnim(char*, float, unsigned int, float);
        virtual void StopPoseAnim(unsigned int, bool);
        virtual void StopAudio();
        virtual void StopAnimSound(bool, int, bool);
        virtual void StopAllAnimSounds(void);
        virtual void CheckLineCollision(float *,float const*,float const*);
        virtual void CheckLineCollision(float *,float const*,float const*,float *);
        virtual void CheckBoxCollision(float const*,float const*,float const*);
        virtual void Use2Skeletons(void);
        virtual void LocalStateIK(void);
        virtual void GetDefaultBones(ZBone*, unsigned int);
        virtual void SetDefaultBones(ZBone const*,SBoneDefinition const*);
        virtual void GetAnimDeltaBones(Animation::Header *);
        virtual void GetBoneMatPos(Glacier::ZMat3x3*,Glacier::ZVector3*, unsigned int);
        virtual void AttachBaseGeomToBone(ZBaseGeom const*,unsigned int,float const*,float const*);
        virtual void DetachBaseGeomFromBone(ZBaseGeom const*,unsigned int);
        virtual void GetAttachedBaseGeomBoneId(ZBaseGeom const*);
        virtual void GetGroundBoneAnimMatPos(ZMat3x3*, ZVector3*, Animation::Header *,float,bool);
        virtual void GetGroundBoneDeltaMatPos(ZMat3x3*, ZVector3*, Animation::Header *,float,float);
        virtual void GetAttachedGeomMatPos(ZBaseGeom const*,float *,float *);
        virtual void DisplayBone(unsigned int,bool);
        virtual void GetFocusMatPos(float *,float *);
        virtual void DisplayAllBones(bool);
        virtual void GetRootFocusMatPos(float *,float *);
        virtual void GetBoneNrFromId(unsigned short );
        virtual void GetBoneFromPoint(float const*);
        virtual int GetBoneNrFromName(char const*);
        virtual const char* GetBoneName(int);
        virtual void GetOrigLocalBones(void);
        virtual void* GetBoneDefinitions(void);
        virtual void CopyGeometryFrom(ZGEOM *);
        virtual void CopyGeometryFrom(unsigned int);
        virtual void CopyPoseFrom(ZLNKOBJ*);
        virtual void ChangeMesh(ZGROUP *);
        virtual void PrintAllBoneNames(void);
        virtual void SetCutSequence(bool);
        virtual void MoveToMatPos(float const*,float const*);
        virtual void SetRootTMParent(float *,float *);
        virtual void EventCallBack(Animation::ActiveAnimation *,float,float,void *);
        virtual void OnMoving(void);
        virtual void OnMoved(void);
        virtual void CalcTightCenSize(float *,float *);
        virtual void CalcShadowProjectPlane(float *,float const*,float const*);
        virtual void GetBoneControl(int);
        virtual void UpdateGeometry(bool);
        virtual void ResetAllAnimBones(void);
        virtual void ResetInactiveBones(void);
        virtual void ExecuteCallBack(char const*);
        virtual void GetBoneVolume(int);
        virtual void GetBoneSize(int, Glacier::Vector3*);
        virtual void GetBoneCenter(int, Glacier::Vector3*);
        virtual void AnimEnd(Animation::ActiveAnimation *,int);
        virtual void UpdateAnimationsAndGroundLink(float);
        virtual void UpdatePoseAnimation();
        virtual void StartAnim(Animation::ActiveAnimation*, int);
        virtual bool IsInElevator();
        virtual float GetElevatorDeltaY();
        virtual bool WantBloodOnHit();
        virtual void LoadSaveAnimations(ZPackedInput*, bool);

        // methods

        // members
        ZPoseAnim* m_pPoseAnim;
        ActBoneMotion2* m_pMotions2;
        int m_pMotions2BoneCount;
        ZBone m_Ground;
        unsigned int m_lVariantId;
        GameEntity* m_pGameEntity;
        SAnimSound m_AnimSound[2];
        unsigned int m_SoundMappingMaterial;
        unsigned char m_iVisionID;
        uint8_t m_pad69[3];
        Animation::Model* m_Model;
        ZRTString animCollectionName;
        ZAnimVariationBuffer m_AnimVariations;
        ZAnimTemplatesNames m_TemplateNames;
        uint32_t m_iAnimVariationFlags;
        int m_pBoneModify;
        bool m_bCutSequence;
        bool m_bHideInThisView;
        RE_ADD_PADDING(2);
        ZDeltaBone m_QGroundAnimDestOld;
        ZDeltaBone m_QGroundAnimDest;
        ZDeltaBone m_QGroundAnimCurrent;
        float m_GroundAnimDestStartFrame;
        float m_GroundAnimDestEndFrame;
        float m_fPelvisBoneOffset;
        struct ZLNKOBJ* m_pAttachedTo;
        uint8_t m_LODMask;
        bool m_bUseLODMASK;
        bool m_FramesReset;
        bool m_bMetaKeyCallBacks;
    };
    RE_VERIFY_SIZE(ZLNKOBJ, 0xF4);
}
