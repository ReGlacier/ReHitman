#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct ZPlayer;

    // NOT FINISHED!!!
	// Actually, it's inherited of RTP::cBase... I don't care
	struct ZMovementBase
	{
		// vtbl
        virtual ~ZMovementBase();
        virtual void PreSave(void* ISerializerStream);
        virtual void PostSave(void* ISerializerStream);
        virtual void PreLoad(void* ISerializerStream);
        virtual bool PostLoad(void* ISerializerStream);
        virtual bool PostProcess(const unsigned int unk1, const unsigned int unk2);
        virtual void LoadObject(void* IInputSerializerStream);
        virtual void SaveObject(void* IOutputSerializerStream);
        virtual void ExchangeObject(void* ISerializerStream);
        virtual void SetToDefault();
        virtual unsigned int GetTypeID();
        virtual const RTP::ZPropertyInfo* GetProperties();
        virtual void ClassInit2();
        virtual bool TerminateControl(bool bUnk);
        virtual bool WantControl(const float* dir1, const float* dir2, const float* dir3);
        virtual void GotControl();
        virtual void RemoveControl();
        virtual void Move(const float* currentPos, const float* targetPos, float* outVelocity, float* outUnk1, float* outUnk2);
        virtual void FrameUpdate();
        virtual void OnAnimationOverride(const float* unk1, const float* unk2);
        virtual void AnimEnd(int animId);
        virtual void GetAnims();
        virtual float GetActiveAnimPair(void* ZAnimVariationPair, const float* unk1, const float* unk2, const float* unk3, const float* unk4, int* outInt);
        virtual float GetActiveAnimHeader(void** AnimationHeader, const float* unk1, const float* unk2, const float* unk3, const float* unk4, int* outInt);
        virtual bool DisableUpperBody();
        virtual bool DisableHeadDirection();
        virtual float BlendInSpeed(const void* AnimationHeader);
        virtual void* GetCurrentUBAnim(unsigned int* outId); // ret Animation::Header*
        virtual float GetMaxWalkableHeight();
        virtual float GetMaxWalkOver();
        virtual void GetPreferredCamDist(float* outDist);
        virtual void GetPreferredCamValues(float* outVal1, float* outVal2, float* outVal3);
        virtual bool HandleActorCollision(void* ZActor);
        virtual bool AllowFollowerSwitch();
        virtual bool AllowItemUse();
        virtual void PlayerModelChanged();
        virtual void CheckPointSave(void* ZCheckPointBuffer);
        virtual void CheckPointLoad(void* ZCheckPointBuffer);
        virtual void CalcNewSpeed(const float* unk1, const float* unk2, float* outSpeed1, float* outSpeed2);
        virtual void CalcNewPosAndSpeed(const float* unk1, const float* unk2, float* outPos, float* outSpeed1, float* outSpeed2);
        virtual void CheckGroundContact(float unk1, float unk2);
        virtual unsigned int GetCollisionFlags();
        virtual bool CheckCollision(const float* unk1, const float* unk2, float* outPos, float* outUnk1, float* outUnk2);
        virtual bool CheckMovingSphereCollision(void* ZSphereImpact, const float* unk1, const float* unk2, float* outPos, float* outUnk1, float* outUnk2);
        virtual bool GetActiveAnimMovement(const float* unk1, const float* unk2, float* outVec1, float* outVec2);
        virtual void* GetAnimHeaderFromVariation(uint32_t ZAnimVariationHandle); // ret Animation::Header*
        virtual void InitAnimGroundMovement(void* AnimationHeader, float unk);
        virtual void GetAnimGroundMovement(void* AnimationHeader, float unk1, const float* unk2, const float* unk3, float* outVec1, float* outVec2, float* outVec3, bool bUnk);
        virtual bool ChangeDimensions(float* outWidth, float* outHeight, const float* inWidth, const float* inHeight);

        // data
        ZMat3x3 m_mGroundMovement;
        ZVector3 m_vGroundMovement;
        float m_fGroundMovementLastFrame;
        ZPlayer* m_pPlayer;
        float m_fAnimPrc;
        uint32_t m_lType;
        float m_fFrameTime;
        float m_fDeltaFrameTime;
        float m_fOldDeltaFrameTime;
        bool m_bPushOut;
        RE_ADD_PADDING(3);
	};
	RE_VERIFY_SIZE(ZMovementBase, 0x54); // Actually, I'm not sure about that. Need test later
}