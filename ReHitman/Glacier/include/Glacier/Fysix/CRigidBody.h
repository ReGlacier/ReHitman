#pragma once

#include <Glacier/EventBase/ZEventBase.h>

#include <Glacier/Fysix/SRigidBodyVelocity.h>
#include <Glacier/Fysix/SExplosionInfo.h>
#include <Glacier/Fysix/SHitInfo.h>

namespace Glacier
{
    class ZCollisionBox;

    class CRigidBody : public ZEventBase
    {
    public:
        //data
        static constexpr const char* Name = "QRigidBody";
        static constexpr const char* ClassName = "ZGEOM_QRigidBody";

        //vftable
        virtual void SetVelocity(const SRigidBodyVelocity& velocity);

        //api
        void Enable();
        void Disable();
        void DisableRemove(bool);
        void SetPos(const ZVector3* position);
        void SetVelocity(const SRigidBodyVelocity* velocity);
        void SetupTransform();
        void HandleHit(SHitInfo* hitInfo);
        void HandleExplodeBomb(SExplosionInfo* explosionInfo);
        void PlaySound();
        void CheckCollision4a(ZCollisionBox* collisionBox);
        void CheckCollision4b(ZCollisionBox* collisionBox);

        //data (total size is 0xF4, ZEventBase size is 0x30)
        uint16_t m_bodyId;
        uint16_t field_32;
        int m_pConstrainedParticleSystem;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_field44;
        int m_field48;
        int m_field4C;
        int m_field50;
        int m_field54;
        int m_field58;
        int m_field5C;
        int m_field60;
        int m_field64;
        ZVector3 m_position;
        int m_field74;
        int m_field78;
        int m_field7C;
        int m_field80;
        int m_field84;
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_field98;
        int m_field9C;
        int m_fieldA0;
        int m_fieldA4;
        int m_fieldA8;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        int m_fieldB8;
        int m_fieldBC;
        int m_fieldC0;
        int m_fieldC4;
        int m_fieldC8;
        int m_fieldCC;
        int m_fieldD0;
        int m_fieldD4;
        int m_fieldD8;
        int m_fieldDC;
        int m_fieldE0;
        int m_material;
        int m_fieldE8;
        int m_fieldEC;
        int m_fieldF0;
    };

}