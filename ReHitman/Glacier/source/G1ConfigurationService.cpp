#include <G1ConfigurationService.h>

namespace Glacier
{
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Alloc                       = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Free                        = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGROUP_CreateGeom                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGROUP_IsRoot                       = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetMatPos                     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootTM                     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_RefToPtr                      = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRef                        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootPoint                  = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetCen                        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootMatPos                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetLocalPoint                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_Zvmmul                        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetFactory                    = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZHumanBoid_SetTarget                = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_DoInit                    = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_ParentGroup               = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetName                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrim                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetMatPos                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_Next                      = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetNext                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetPrev                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrev                   = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray              = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_FreePos              = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_OccupyPos            = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_IsContainerFull      = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_DispatchNextAction  = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_RemoveAction        = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Enable                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Disable                  = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_DisableRemove            = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetPos                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetupTransform           = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetVelocity              = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleHit                = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleExplodeBomb        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_PlaySound                = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4a         = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4b         = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Put               = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Clear             = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Find              = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom         = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_SRefToPtr           = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetEventScheduler   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GeomGeomClassInfo   = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem                  = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem               = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom         = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_GetGQC                              = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript               = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZAction_AddAction                   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZAction_Show                        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZAction_Hide                        = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_AssignOperator         = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMirror               = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMain                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj2D                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj3D                 = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetLocalMatPos         = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_TransformInversMatPos  = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetViewport            = G1ConfigurationService::kNotConfiguredOption;
	std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_SetFogEnabled               = G1ConfigurationService::kNotConfiguredOption;
	std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_IsFogEnabled                = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CCom_GetpVal                        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_CCom_GetVal                         = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_ToggleColor            = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_SetDiffuseColor        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGameStats_IncreaseCurrentShotCount = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGameStats_DecreaseCurrentShotCount = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZActorHeroCheckInside_IsInside      = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_AddEvent           = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_RemoveEvent        = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ActivateFrameUpdate      = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEventBase_DeactivateFrameUpdate    = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ChangeEventActivity      = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ActivateTimeUpdate       = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZActorCommunication_RegisterRadioUser = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseConRout_GetFactory             = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_InstanceAddress_ZEngineGeomControl                  = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_InstanceAddress_CConfiguration_bSubtitles           = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_InstanceAddress_ZEventBase_m_DefaultStatus          = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_InstanceAddress_CCom_g_globalCom                    = G1ConfigurationService::kNotConfiguredOption;
}