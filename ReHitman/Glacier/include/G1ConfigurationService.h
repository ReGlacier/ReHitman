#pragma once

#include <cstdint>

namespace Glacier
{
    class G1ConfigurationService final
    {
    public:
        // Consts
        static constexpr std::intptr_t kNotConfiguredOption = 0x0;

        // API Configuration
        static std::intptr_t G1API_FunctionAddress_ZSysMem_Alloc;
        static std::intptr_t G1API_FunctionAddress_ZSysMem_Free;
        static std::intptr_t G1API_FunctionAddress_ZGROUP_CreateGeom;
        static std::intptr_t G1API_FunctionAddress_ZGROUP_IsRoot;
        static std::intptr_t G1API_FunctionAddress_ZGEOM_GetMatPos;
        static std::intptr_t G1API_FunctionAddress_ZGEOM_RefToPtr;
        static std::intptr_t G1API_FunctionAddress_ZHumanBoid_SetTarget;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_ParentGroup;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_SetName;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_SetPrim;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_GetMatPos;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_Next;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_SetNext;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_GetPrev;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_SetPrev;
        static std::intptr_t G1API_FunctionAddress_ZAction_GetActionArray;
        static std::intptr_t G1API_FunctionAddress_ZItemContainer_FreePos;
        static std::intptr_t G1API_FunctionAddress_ZItemContainer_OccupyPos;
        static std::intptr_t G1API_FunctionAddress_ZItemContainer_IsContainerFull;
        static std::intptr_t G1API_FunctionAddress_ZLnkActionQueue_DispatchNextAction;
        static std::intptr_t G1API_FunctionAddress_ZLnkActionQueue_RemoveAction;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_Enable;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_Disable;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_DisableRemove;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_SetPos;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_SetVelocity;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_SetupTransform;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_HandleHit;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_HandleExplodeBomb;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_PlaySound;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_CheckCollision4a;
        static std::intptr_t G1API_FunctionAddress_CRigidBody_CheckCollision4b;
        static std::intptr_t G1API_FunctionAddress_ZHash_int_SMatPos_Put;
        static std::intptr_t G1API_FunctionAddress_ZHash_int_SMatPos_Clear;
        static std::intptr_t G1API_FunctionAddress_ZHash_int_SMatPos_Find;
    };
}