#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Animation/ZBone.h>

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

    struct GameEntity
    {
        ZGEOM* m_pGeom;
        // Need finish this part
    };
    // RE_VERIFY_SIZE(GameEntity, 0x18);

    struct ZAnimVariationBuffer
    {
        const char* m_pBuffer;
        int* m_AnimList;
    };
    RE_VERIFY_SIZE(ZAnimVariationBuffer, 0x8);

    struct ZAnimTemplatesNames
    {
        const char* m_pBuffer;
    };
    RE_VERIFY_SIZE(ZAnimTemplatesNames, 0x4);

    struct ZDeltaBone
    {
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

        //vftable
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
        virtual void AttachBaseGeomToBone(ZEntityLocator const*,unsigned int,float const*,float const*);
        virtual void DetachBaseGeomFromBone(ZEntityLocator const*,unsigned int);
        virtual void GetAttachedBaseGeomBoneId(ZEntityLocator const*);
        virtual void GetGroundBoneAnimMatPos(ZMat3x3*, ZVector3*, Animation::Header *,float,bool);
        virtual void GetGroundBoneDeltaMatPos(ZMat3x3*, ZVector3*, Animation::Header *,float,float);
        virtual void GetAttachedGeomMatPos(ZEntityLocator const*,float *,float *);
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

        //data (total size is 0xF4, base size is 0x10)
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
        bool m_pad8A[2];
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