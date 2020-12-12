#include <Glacier/Fysix/CRigidBody.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    void CRigidBody::Enable()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Enable != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Enable != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Enable))(this);
        }
    }

    void CRigidBody::Disable()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Disable != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Disable != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Disable))(this);
        }
    }

    void CRigidBody::DisableRemove(bool a1)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_DisableRemove != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_DisableRemove != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*, bool))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_DisableRemove))(this, a1);
        }
    }

    void CRigidBody::SetPos(const ZVector3* position)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetPos != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetPos != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*, const ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetPos))(this, position);
        }
    }

    void CRigidBody::SetVelocity(const SRigidBodyVelocity* velocity)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetVelocity != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetVelocity != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*, const SRigidBodyVelocity*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetVelocity))(this, velocity);
        }
    }

    void CRigidBody::SetupTransform()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetupTransform != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetupTransform != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetupTransform))(this);
        }
    }

    void CRigidBody::HandleHit(SHitInfo* hitInfo)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleHit != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleHit != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*,SHitInfo*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleHit))(this, hitInfo);
        }
    }

    void CRigidBody::HandleExplodeBomb(SExplosionInfo* explosionInfo)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleExplodeBomb != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleExplodeBomb != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*,SExplosionInfo*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleExplodeBomb))(this, explosionInfo);
        }
    }

    void CRigidBody::PlaySound()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_PlaySound != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_PlaySound != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_PlaySound))(this);
        }
    }

    void CRigidBody::CheckCollision4a(ZCollisionBox* collisionBox)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4a != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4a != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*,ZCollisionBox*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4a))(this, collisionBox);
        }
    }

    void CRigidBody::CheckCollision4b(ZCollisionBox* collisionBox)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4b != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4b != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CRigidBody*,ZCollisionBox*))(G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4b))(this, collisionBox);
        }
    }
}