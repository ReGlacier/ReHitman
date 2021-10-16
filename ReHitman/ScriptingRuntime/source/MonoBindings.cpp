#include <MonoBindings.h>
#include <spdlog/spdlog.h>
#include <mono/jit/jit.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZROOM.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/Globals.h>

namespace API {
    namespace DeveloperConsole {
        static void Info(MonoString* psMessage) {
            if (psMessage) {
                spdlog::info("(Managed) {}", mono_string_to_utf8(psMessage));
            }
        }
        static void Warning(MonoString* psMessage) {
            if (psMessage) {
                spdlog::warn("(Managed) {}", mono_string_to_utf8(psMessage));
            }
        }
        static void Error(MonoString* psMessage) {
            if (psMessage) {
                spdlog::error("(Managed) {}", mono_string_to_utf8(psMessage));
            }
        }
    }
    namespace Game {
        static float GetTimeScale() {
            auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(
                    Hitman::BloodMoney::Globals::kSysInterfaceAddr);
            if (!pSysInterface) {
                return .0f;
            }
            //TODO: Refactor this place
            return *reinterpret_cast<float *>(reinterpret_cast<int>(pSysInterface) + 0xB24);
        }

        static void SetTimeScale(float value) {
            auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(
                    Hitman::BloodMoney::Globals::kSysInterfaceAddr);
            if (!pSysInterface) {
                return;
            }

            //TODO: Refactor this place
            *reinterpret_cast<float *>(reinterpret_cast<int>(pSysInterface) + 0xB24) = value;
        }

        static MonoString* GetSceneName() {
            auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(
                    Hitman::BloodMoney::Globals::kSysInterfaceAddr);
            if (!pSysInterface) {
                return nullptr;
            }

            auto pEngineDb = pSysInterface->m_engineDataBase;
            if (!pEngineDb) {
                return nullptr;
            }

            if (!pEngineDb->GetSceneName()) {
                return nullptr;
            }

            return mono_string_new(mono_domain_get(), pEngineDb->GetSceneName());
        }

        static MonoString* GetProfileName() {
            auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(
                    Hitman::BloodMoney::Globals::kGameDataAddr);
            if (!pGameData) {
                return nullptr;
            }

            return mono_string_new(mono_domain_get(), &pGameData->m_ProfileName[0]);
        }

        static int32_t GetProfileMoney() {
            auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(
                    Hitman::BloodMoney::Globals::kGameDataAddr);
            if (!pGameData) {
                return 0;
            }

            return pGameData->m_PlayerMoney;
        }

        static void SetProfileMoney(int32_t money) {
            auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(
                    Hitman::BloodMoney::Globals::kGameDataAddr);
            if (pGameData) {
                pGameData->m_PlayerMoney = money;
            }
        }

        static uint32_t GetRootGroup() {
            auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Hitman::BloodMoney::Globals::kSysInterfaceAddr);
            if (!pSysInterface) {
                return 0;
            }

            if (!pSysInterface->m_engineDataBase || !pSysInterface->m_engineDataBase->m_root) {
                return 0;
            }

            return pSysInterface->m_engineDataBase->m_root->GetRef();
        }
    }
    namespace ZGEOM {
        static bool IsNull(uint32_t zref) {
            return Glacier::ZGEOM::RefToPtr(zref) == nullptr;
        }

        static void DetachAndDestroy(uint32_t zref) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return;
            }

            auto pParentGroup = pGeom->m_baseGeom->ParentGroup();
            pParentGroup->DetachGeom(pGeom->m_baseGeom, true); /* NOTE: true? What is it? */

            //TODO: Free mem of pGeom!
        }

        static MonoString* GetName(uint32_t zref) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return nullptr;
            }

            return mono_string_new(mono_domain_get(), &pGeom->m_baseGeom->entityName[0]);
        }

        static MonoArray* GetPosition(uint32_t zref) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return nullptr;
            }

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;

            pGeom->GetMatPos(&mat, &pos);

            /// Build managed instance
            MonoArray* ret = mono_array_new(mono_domain_get(), mono_get_single_class(), 3);
            if (ret) {
                mono_array_set(ret, float, 0, pos.x);
                mono_array_set(ret, float, 1, pos.y);
                mono_array_set(ret, float, 2, pos.z);
            }

            return ret;
        }

        static void SetPosition(uint32_t zref, float x, float y, float z) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return;
            }

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;

            pGeom->GetMatPos(&mat, &pos);
            pos.x = x;
            pos.y = y;
            pos.z = z;

            pGeom->SetMatPos(&mat, &pos);
        }

        static MonoArray* GetTransform(uint32_t zref) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return nullptr;
            }

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;

            pGeom->GetMatPos(&mat, &pos);

            MonoArray* ret = mono_array_new(mono_domain_get(), mono_get_single_class(), 9);
            if (ret) {
                for (int i = 0; i < 9; i++) {
                    mono_array_set(ret, float, i, mat.data[i]);
                }
            }

            return ret;
        }

        static void SetTransform(uint32_t zref, float m00, float m10, float m20, float m01, float m11, float m21, float m02, float m12, float m22) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return;
            }

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;

            pGeom->GetMatPos(&mat, &pos);

            mat.data[0] = m00;
            mat.data[1] = m10;
            mat.data[2] = m20;
            mat.data[3] = m01;
            mat.data[4] = m11;
            mat.data[5] = m21;
            mat.data[6] = m02;
            mat.data[7] = m12;
            mat.data[8] = m22;

            pGeom->SetMatPos(&mat, &pos);
        }

        static uint32_t GetPrimitiveId(uint32_t zref) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return 0;
            }

            return pGeom->m_baseGeom->m_primitive;
        }

        static void SetPrimitiveId(uint32_t zref, uint32_t primId, bool shouldUpdateView) {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zref);
            if (!pGeom) {
                return;
            }

            pGeom->m_baseGeom->SetPrim(primId);
            if (shouldUpdateView) {
                //TODO: Think how to make it for any kind of ZGEOM
            }
        }
    }
    namespace ZGROUP {
        static bool ZGeomIsZGroup(Glacier::ZGEOM* pGeom) {
            //TODO: Add check for type of pGeom
            return true;
        }

        static uint32_t CreateGeom(uint32_t zgroupRef, MonoString* psName, MonoArray* pfPosition, MonoArray* pfTransform, uint32_t geomClassId)
        {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(zgroupRef);
            if (!pGeom) {
                return 0;
            }

            if (!ZGeomIsZGroup(pGeom)) {
                return 0;
            }

            Glacier::ZGROUP* pGroup = reinterpret_cast<Glacier::ZGROUP*>(pGeom);
            Glacier::ZGEOM* pNewGeom = pGroup->CreateGeom(mono_string_to_utf8(psName), geomClassId, true);
            if (!pNewGeom) {
                return 0;
            }

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;

            pos.x = mono_array_get(pfPosition, float, 0);
            pos.y = mono_array_get(pfPosition, float, 1);
            pos.z = mono_array_get(pfPosition, float, 2);

            for (int i = 0; i < 9; i++) {
                mat.data[i] = mono_array_get(pfTransform, float, i);
            }

            pNewGeom->SetMatPos(&mat, &pos);
            return pNewGeom->GetRef();
        }

        static void DetachGeom(uint32_t group, uint32_t geom)
        {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(group);
            if (!pGeom) {
                return;
            }

            if (!ZGeomIsZGroup(pGeom)) {
                return;
            }

            Glacier::ZGEOM* pGeomToRemove = Glacier::ZGEOM::RefToPtr(geom);
            if (!pGeomToRemove) {
                return;
            }

            reinterpret_cast<Glacier::ZGROUP*>(pGeom)->DetachGeom(pGeomToRemove->m_baseGeom, true);
        }

        static void AttachGeom(uint32_t group, uint32_t geom)
        {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(group);
            if (!pGeom) {
                return;
            }

            if (!ZGeomIsZGroup(pGeom)) {
                return;
            }

            Glacier::ZGEOM* pGeomToAttach = Glacier::ZGEOM::RefToPtr(group);
            if (!pGeomToAttach) {
                return;
            }

            reinterpret_cast<Glacier::ZGROUP*>(pGeom)->AttachGeom(pGeomToAttach->m_baseGeom, true);
        }
    }
}

namespace Hitman::BloodMoney::Scripting {
    void DeclareDeveloperConsole();
    void DeclareGame();
    void DeclareZGeom();
    void DeclareZGroup();

    void MonoBindings::DeclareFunctions() {
        DeclareDeveloperConsole();
        DeclareGame();
        DeclareZGeom();
        DeclareZGroup();
    }

    void DeclareDeveloperConsole()
    {
        // DeveloperConsole
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.DeveloperConsole::Info", API::DeveloperConsole::Info);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.DeveloperConsole::Warning", API::DeveloperConsole::Warning);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.DeveloperConsole::Error", API::DeveloperConsole::Error);
    }

    void DeclareGame()
    {
        // Game
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetTimeScale", API::Game::GetTimeScale);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::SetTimeScale", API::Game::SetTimeScale);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetSceneName", API::Game::GetSceneName);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetProfileName", API::Game::GetProfileName);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetProfileMoney", API::Game::GetProfileMoney);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::SetProfileMoney", API::Game::SetProfileMoney);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetRootGroup", API::Game::GetRootGroup);
    }

    void DeclareZGeom()
    {
        // ZGEOM
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::IsNull", API::ZGEOM::IsNull);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::DetachAndDestroy", API::ZGEOM::DetachAndDestroy);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::GetName", API::ZGEOM::GetName);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::GetPosition", API::ZGEOM::GetPosition);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::SetPosition", API::ZGEOM::SetPosition);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::GetTransform", API::ZGEOM::GetTransform);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::SetTransform", API::ZGEOM::SetTransform);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::GetPrimitiveId", API::ZGEOM::GetPrimitiveId);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGEOM::SetPrimitiveId", API::ZGEOM::SetPrimitiveId);
    }

    void DeclareZGroup()
    {
        //ZGROUP
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGROUP::CreateGeom", API::ZGROUP::CreateGeom);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGROUP::AttachGeom", API::ZGROUP::AttachGeom);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.ZGROUP::DetachGeom", API::ZGROUP::DetachGeom);
    }
}