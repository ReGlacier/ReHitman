#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZMath.h>

#include <vector>

namespace Glacier
{
    enum class EBoidState : int32_t { Moving = 0, Staying = 1, ParentActorDestroyed = 2, Sitting = 5, Unk6 = 6 };

    class ZBoid;

    class ZHumanBoid
    {
    public:
        using BoidsVector = std::vector<ZBoid*, std::allocator<ZBoid*>>;

        //vftable
        virtual void FrameUpdate(const BoidsVector& a1, float);
        virtual void Draw(); //nullstub
        virtual float GetSpeed();
        virtual void Move(const BoidsVector& a1, float a2);
        virtual void AvoidBoids_Base(const BoidsVector& a1, float a2);
        virtual void HardAvoidBoids(const BoidsVector& a1, float a2);
        virtual void MovePosition(Glacier::ZVector3* position);
        virtual void TeleportPosition(Glacier::ZVector3* position);
        virtual void SetSpeed(float speed);
        virtual void LoadSave(ZPackedInput* packedInput);
        virtual void AvoidBoids(const BoidsVector& boids, float a2, float& a3);
        virtual void LoadSaveSubTarget(ZPackedInput* packedInput, bool);

        //api
        void SetTarget(Glacier::ZVector3* position, Glacier::Vector3* rotation, float a4, bool force);

        // data
        EBoidState m_boidState; //0x0004
        float m_speed;          //0x008
        int m_TrackerDist;      //0x00C
        ZGEOM* m_owner;         //0x010
        int m_Mask;             //0x014
        int field_18;           //0x018
        ZVector3 m_Position;    //0x01C
        ZVector3 m_Tracker;     //0x028
        float m_Weight;         //0x034
        int m_PushRadius;       //0x038
        int field_3C;           //0x03C
        int field_40;
        int field_44;
        int field_48;
        int field_4C;
        int field_50;
        int field_54;
        int field_58;
        int field_5C;
        int field_60;
        int field_64;
        int field_68;
        int field_6C;
        int field_70;
        int field_74;
        int field_78;
        int field_7C;
        int field_80;
        int field_84;
        int field_88;
        int field_8C;
        int field_90;
        int field_94;
        int field_98;
        int field_9C;
        int field_A0;
        int field_A4;
        int field_A8;
        int field_AC;
        float m_ActualSpeed;
        float m_fMaxSpeed;
        float m_speed2;
        int m_PF4RunTime_ZData;
        int m_PathRemaining;
        ZVector3 m_subTarget;
        float m_fSpeedMultiplier;
        int m_fEndSpeed;
        ZVector3 m_vEndDir;
        int m_fPauseMovementAtDistance;
        int m_PathEndsInObstacle;
        int field_EC;
    };
}