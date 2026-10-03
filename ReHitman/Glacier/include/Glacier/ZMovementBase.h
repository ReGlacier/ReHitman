#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct ZPlayer;
    struct ZSphereImpact;

    struct ZMovementBase
    {
        // vtbl
        virtual ~ZMovementBase();

        // ZSerializable
        virtual void PreSave(ISerializerStream&);
        virtual void PostSave(ISerializerStream&);
        virtual void PreLoad(ISerializerStream&);
        virtual bool PostLoad(ISerializerStream&);
        virtual bool PostProcess(unsigned int, unsigned int);
        virtual void LoadSave(ISerializerStream&, bool);
        virtual void LoadObject(IInputSerializerStream&);
        virtual void SaveObject(IOutputSerializerStream&);
        virtual void ExchangeObject(ISerializerStream&);
        virtual void SetToDefault();

        // RTP::cBase
        virtual uint32_t GetTypeID();
        virtual const RTP::ZPropertyInfo* GetProperties() const;

        // ZMovementBase
        virtual void ClassInit2();
        virtual void TerminateControl(bool);
        virtual void WantControl(const float*, const float*, const float*);
        virtual void GotControl();
        virtual void RemoveControl();
        virtual void Move(const float*, const float*, float*, float*, float*);
        virtual void FrameUpdate();

        // Anim
        virtual void AnimEnd(Animation::ActiveAnimation*);
        virtual void GetAnims();
        virtual void GetActiveAnim(Animation::Header**, const float*, const float*, const float*, const float*, int&);
        virtual void DisableUpperBody();
        virtual void DisableHeadDirection();
        virtual void BlendInSpeed(const Animation::Header*);
        virtual void GetCurrentUBAnim(unsigned int*);

        // Physics?
        virtual void CalcNewSpeed(const float*, const float*, float*, float*);
        virtual void CalcNewPosAndSpeed(const float*, const float*, float*, float*, float*);
        virtual void AddGravityToSpeed(float*, float);
        virtual void CheckGroundContact(float, float);
        virtual bool CheckCollision(const float*, const float*, float*, float*, float*) const;
        virtual bool CheckMovingSphereCollision(ZSphereImpact*, const float*, const float*, float*, float*, float*) const;
        virtual bool GetActiveAnimMovement(const float*, const float*, float*, float*) const;
        virtual Animation::Header* GetAnimHeaderFromVariation(ZAnimVariationHandle);
        virtual void InitAnimGroundMovement(Animation::Header*, float);
        virtual void GetAnimGroundMovement(Animation::Header*, float, const float*, const float*, float*, float*, float*, bool);
        virtual bool ChangeDimensions(float*, float*, const float*, const float*) const;

        // data
        ZMat3x3 m_mGroundMovement;
        ZVector3 m_vGroundMovement;
        float m_fGroundMovementLastFrame;
        struct ZPlayer* m_pPlayer;
        float m_fAnimPrc;
        unsigned int m_lType;
        float m_fFrameTime;
        float m_fDeltaFrameTime;
        float m_fOldDeltaFrameTime;
        bool m_bPushOut;
        RE_ADD_PADDING(3);
    }; // Actual size is unknown, but size is unchanged in Mini Ninjas
    RE_VERIFY_SIZE(ZMovementBase, 0x54); // Verified by PS2, iOS and Mini Ninjas. I guess that's enough
}
