#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    struct ZAnimVariationHandle {
        int m_field0;
        int m_field4;
        int m_field8;
    };

    struct ActiveAnimation {
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
        int m_field28;
    }; //Size 0x2C

    struct Model {
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        ActiveAnimation m_animationsPool[4];
        //Not completed!
    };

    enum class ESuitMask : int32_t {
        NoActor					 = 0b0000'0000'0000'0000'0000'0000'0000'0000,	///< This is not an actor (M13, wheelchair gui subactor kind)
        SkinChangerNotSupported	 = 0b0000'0000'0000'0000'0000'0000'0000'0001,	///< This actor does not supports skin changer
        Nude					 = 0b0000'0000'0000'0000'0000'0000'0000'0010,	///< Nude view of model (works only if the actor can share their suit, in other case works as Inivisble)
        Invisible				 = 0b0000'0000'0000'0000'0000'0000'0000'0100,	///< Invisible view of the actor (possible model loading failure, I don't know)
        OriginalView			 = 0b0000'0000'0000'0000'0000'0000'0000'0101,	///< Just original view of actor
        Agent47_WithoutHeaddress = 0b0000'0000'0000'0000'0000'0000'0000'0110,	///< Sometimes works if suit have headdress (in other case will work as invisible)
        Agent47_WithHeaddress	 = 0b0000'0000'0000'0000'0000'0000'0000'0111	///< Works in 99% of situations with actors who can share their suit
    };

    class ZLNKOBJ : public ZSTDOBJ
    {
    public:
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
        virtual void GetBoneMatPos(float *,float *, unsigned int);
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
        virtual void GetBoneNrFromName(char const*);
        virtual void GetBoneName(int);
        virtual void GetOrigLocalBones(void);
        virtual void GetBoneDefinitions(void);
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
        virtual void GetBoneSize(int,float *);
        virtual void GetBoneCenter(int,float *);
        virtual void AnimEnd(Animation::ActiveAnimation *,int);
        virtual void UpdateAnimationsAndGroundLink(float);
        virtual void UpdatePoseAnimation();
        virtual void StartAnim(Animation::ActiveAnimation*, int);
        virtual bool IsInElevator();
        virtual float GetElevatorDeltaY();
        virtual bool WantBloodOnHit();
        virtual void LoadSaveAnimations(ZPackedInput*, bool);

        //data (total size is 0xF4, base size is 0x10)
        int field_10;
        int field_14;
        int field_18;
        ZMat3x3 m_matrix1C;
        int field_40;
        int field_44;
        int field_48;
        ESuitMask m_suitMask;
        int field_50;
        int field_54;
        int field_58;
        int field_5C;
        int field_60;
        int field_64;
        char field_68;
        char field_69;
        char field_6A;
        char field_6B;
        Model* m_animationModel;
        int field_70;
        int field_74;
        int field_78;
        int field_7C;
        char m_iAnimVariationFlags;
        char field_81;
        char field_82;
        char field_83;
        int m_boneModify;
        char m_bIsInCutSequence;
        char field_89;
        char field_8A;
        char field_8B;
        int field_8C;
        int field_90;
        int field_94;
        int field_98;
        int field_9C;
        int field_A0;
        int field_A4;
        int field_A8;
        int field_AC;
        int field_B0;
        int field_B4;
        int field_B8;
        int field_BC;
        int field_C0;
        int field_C4;
        int field_C8;
        int field_CC;
        int field_D0;
        int field_D4;
        int field_D8;
        int field_DC;
        int m_GroundAnimDestStartFrame;
        int m_GroundAnimDestEndFrame;
        int m_fPelvisBoneOffset;
        int field_EC;
        char m_LODMask;
        char m_bUseLODMASK;
        char m_FramesReset;
        char m_bMetaKeyCallBacks;
    };
}