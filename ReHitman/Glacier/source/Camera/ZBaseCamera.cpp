#include <Glacier/Camera/ZBaseCamera.h>
#include <Glacier/Camera/KeyControlGeom.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>

#include <cstdlib>


namespace Glacier
{
    namespace
    {
        Action::ZStaticBinds CameraControlDefsDebugBinding {
            "ccc={"
                "TiltCamera=rel(ms,y);"
                "TurnCamera=rel(ms,x);};"
                "DebugAlways={Mode1=|hold(kb,rshift) hold(kb,lshift);"
                "Mode2=|hold(kb,rctrl) hold(kb,lctrl);"
                "Mode3=&!hold(kb,rshift) &!hold(kb,lshift) &!hold(kb,rctrl) !hold(kb,lctrl);"
                "Mode4=0;MoveX=-hold(kb,right) hold(kb,left);"
                "MoveY=-hold(kb,down) hold(kb,up);"
                "SubSpeed=tap(ms,wheeldown);"
                "AddSpeed=tap(ms,wheelup);"
                "Mode1MoveX=-hold(kb,right) hold(kb,left);"
                "Mode1MoveY=-hold(kb,rshift) hold(kb,rctrl);"
                "Mode1MoveZ=-hold(kb,up) hold(kb,down);"
                "Mode1MouseL=hold(ms,left);"
                "Mode1MouseM=hold(ms,middle);"
                "Mode1MouseR=hold(ms,right);"
            "};"
        };
        Action::ZHandle a_BaseCameraToggleFreeCam { "ToggleFreeCam" }; // PC 009ACE60
        Action::ZHandle a_BaseCameraToggleGates { "ToggleGates" }; // PC 009ACE6C
        Action::ZHandle a_BaseCameraToggleAlignment { "ToggleAlignment" }; // PC 009ACE78
        Action::ZHandle a_SubSpeed { "SubSpeed" };
        Action::ZHandle a_AddSpeed { "AddSpeed" };
        ZDebugInt g_iInvertFreeCam { "InvertFreeCam", "Inverts Y axis of freecam", 0, 0, 1, 1, "Camera/" };

        ZVector3 g_vOldFreeCamMouse; // PC dword_9ACF64 / flt_9ACF68 / unk_9ACF6C
    }

    ZBaseCamera::ZBaseCamera()
        : Glacier::CBaseEvent<ZCAMERA>()
    {
        Initialize();
    }

    ZBaseCamera::~ZBaseCamera() = default;

    const RTP::ZPropertyInfo& ZBaseCamera::GetProperties() const
    {
        return ZBaseCamera::Info;
    }

	ZBaseCamera::EEventPriority ZBaseCamera::GetEventPriority()
	{
	    return ZBaseCamera::EEventPriority::PRIORITY_BaseCamera;
	}

	void ZBaseCamera::FrameUpdate()
	{
        FreeCam();

        if (a_BaseCameraToggleGates.Digital())
        {
            ZGEOM* pGeom = GetGeom();
            ZASSERT(pGeom);
            if (pGeom)
            {
                // Toggle the "gates" render control on the camera geometry (PC: geom+0x2C ^= 0x10000)
                auto* pCamera = reinterpret_cast<ZCAMERA*>(pGeom);
                pCamera->CameraCon ^= 0x10000u;
            }
        }
	}

	void ZBaseCamera::CopyData(const ZEventBase* pEvent)
	{
	}

	void ZBaseCamera::Init()
	{
        auto* pSceneCom = g_pEngineData->GetSceneCom();
        ZASSERT(pSceneCom);

        float fFogBegin = pSceneCom->Get("CameraFogBegin");
        float fFogFull = pSceneCom->Get("CameraFogFull");
        if (fFogBegin == 0.0f)
        {
            fFogBegin = 0.65f;
        }
        if (fFogFull == 0.0f)
        {
            fFogFull = 0.95f;
        }

        ZGEOM* pGeom = GetGeom();
        ZASSERT(pGeom);
        if (pGeom)
        {
            auto* pCamera = reinterpret_cast<ZCAMERA*>(pGeom);

            pCamera->SetNear(pSceneCom->Get("CameraNear"));
            pCamera->SetFar(pSceneCom->Get("CameraFar"));
            pCamera->SetBackColor(static_cast<int>(pSceneCom->Get("CameraBackgroundColor")));
            pCamera->SetTargetLen(pSceneCom->Get("CameraTargetLen"));
            pCamera->SetFOV(pSceneCom->Get("CameraFOV"));
            pCamera->m_fFogNear = fFogBegin;
            pCamera->m_fFogFar = fFogFull;
            pCamera->CameraType = 0;
        }

        m_bIsActive = false;
        m_bEnabled = false;
        m_bAlignment = false;
        m_KeyConOn = false;

        // Register the free-camera debug controls (PC 0x007FEB9C = CameraControlDefsDebugBinding)
        Action::instance->AddBindings(CameraControlDefsDebugBinding.GetBinds());
	}

	int ZBaseCamera::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
	{
        if (command == ZBaseCamera::m_msgRequestQuality.m_MessageID)
        {
            if (data)
            {
                *static_cast<float*>(data) = GetQuality();
            }
            return 0;
        }

        if (command == ZBaseCamera::m_msgActivateCamera.m_MessageID)
        {
            ActivateCamera();
            return 0;
        }

        if (command == ZBaseCamera::m_msgDeactivateCamera.m_MessageID)
        {
            DeactivateCamera();
            return 0;
        }

        if (command == ZBaseCamera::m_msgEnableCamera.m_MessageID)
        {
            EnableCamera(true);
            return 0;
        }

        if (command == ZBaseCamera::m_msgDisableCamera.m_MessageID)
        {
            EnableCamera(false);
            return 0;
        }

        if (command == ZBaseCamera::m_msgGetFollowGeom.m_MessageID)
        {
            if (data)
            {
                *static_cast<uint32_t*>(data) = m_rFollowGeom;
            }
            return 0;
        }

        if (command == ZBaseCamera::m_msgFollowGeom.m_MessageID)
        {
            m_rFollowGeom = static_cast<ZREF>(reinterpret_cast<uintptr_t>(data));
            return 0;
        }

        if (command == ZBaseCamera::m_msgResetCamToBestPos.m_MessageID)
        {
            ResetCamToBestPos();
            return 0;
        }

        if (command == ZBaseCamera::m_msgIsFreeCam.m_MessageID)
        {
            if (data)
            {
                *static_cast<bool*>(data) = IsFreeCam();
            }
            return 0;
        }

		return 0;
	}

	bool ZBaseCamera::Is1stPerson()
	{
		return true; // PC sub_580400 (shared return-true), PS2 _ZN11ZBaseCamera11Is1stPersonEv
	}

	void ZBaseCamera::SetFollowGeom(Glacier::ZREF rGeom)
	{
        m_rFollowGeom = rGeom;
	}

	void ZBaseCamera::SetFreeCam(bool bIsFree)
	{
        if (bIsFree)
        {
            ResetCamToBestPos();
            m_bAlignment = true;
            m_KeyConOn = true;
        }
        else
        {
            EnableCamera(true);
            m_bAlignment = false;
            m_KeyConOn = false;
        }
	}

	bool ZBaseCamera::IsFreeCam()
	{
        return m_KeyConOn;
	}

	void ZBaseCamera::GetFocalPos(Glacier::Vector3* pvFocalPos)
	{
        // Only the derived cameras know their focal point (PS2: INT3 in basecamera.h:65, PC: 0x4636F0)
        ZASSERT(false);
	}

	void ZBaseCamera::GetFocalOffset(Glacier::Vector3* pvFocalOffset)
	{
        // Only the derived cameras know their focal offset (PS2: INT3 in basecamera.h:66, PC: 0x4636F0)
        ZASSERT(false);
	}

	void ZBaseCamera::GetCameraPosition(Glacier::Vector3* pvCameraPosition)
	{
        // Only the derived cameras know their camera position (PS2: INT3 in basecamera.h:67, PC: 0x4636F0)
        ZASSERT(false);
	}

	float ZBaseCamera::GetQuality()
	{
        return 0.0f; // PC 0x6F80C0, PS2 _ZN11ZBaseCamera10GetQualityEv
	}

	void ZBaseCamera::ActivateCamera()
	{
        ZGEOM* pGeom = GetGeom();
        ZASSERT(pGeom);
        if (pGeom)
        {
            auto* pCamera = reinterpret_cast<ZCAMERA*>(pGeom);
            pCamera->CameraCon &= ~0x2000000u;
        }

        m_bIsActive = true;
	}

	void ZBaseCamera::DeactivateCamera()
	{
        ZGEOM* pGeom = GetGeom();
        ZASSERT(pGeom);
        if (pGeom)
        {
            auto* pCamera = reinterpret_cast<ZCAMERA*>(pGeom);
            pCamera->CameraCon |= 0x2000000u;
        }

        m_bIsActive = false;
	}

	void ZBaseCamera::EnableCamera(bool bIsEnabled)
	{
        m_bEnabled = bIsEnabled;
	}

	void ZBaseCamera::ResetCamToBestPos()
	{
	}

	void ZBaseCamera::Initialize()
	{
        ZEngineDataBase* pEngineData = g_pEngineData;
        ZASSERT(pEngineData);

        pEngineData->RegisterZDefine("float CameraNear=0.1;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float CameraFar=8000;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("color CameraBackgroundColor;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float CameraTargetLen=0;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float CameraFOV=67.4;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float CameraFogBegin=0.65;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float CameraFogFull=0.95;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float PS2_Dot3TurnoffDistance=2000.0;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float PS2_SpecularTurnoffDistance=3000.0;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("bool PS2_EnableStaticShadowSeamsFixup=false;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("bool PS2_DisableStaticShadowAdditive=false;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("bool PC_XBOX_EnableDotFadeDropShadows=false;", const_cast<char*>(__FILE__), __LINE__);
        pEngineData->RegisterZDefine("float StaticShadowSampleHeightLimit=10000;", const_cast<char*>(__FILE__), __LINE__);

        // Register this camera controller as the active "CameraControl" receiver
        pEngineData->GetSceneCom()->SetVal("CameraControl", GetRef(), CCOM_TYPE_REF);

        m_rFollowGeom = 0;
        m_fCameraRotSpeed = 1.0f;

        char* pszOption = nullptr;
        if (ZSysInterface::GetOption("FreeCamMode", &pszOption))
        {
            g_iFreeCamMode.m_iValue = std::atoi(pszOption);
        }
        else
        {
            g_iFreeCamMode.m_iValue = 0;
        }

        if (ZSysInterface::GetOption("FreeCamMouseSensitivity", &pszOption))
        {
            m_fCameraRotSpeed = static_cast<float>(std::atof(pszOption));
        }

        m_fCameraMoveSpeed = 100.0f;
	}

    ZGEOM* ZBaseCamera::FollowGeom()
    {
        return m_rFollowGeom ? ZGEOM::RefToPtr(m_rFollowGeom) : nullptr;
    }

    bool ZBaseCamera::FreeCam()
    {
        if (a_BaseCameraToggleFreeCam.Digital())
        {
            if (m_KeyConOn)
            {
                m_KeyConOn = false;
                ActivateCamera();
            }
            else
            {
                m_KeyConOn = true;
                DeactivateCamera();

                const int iMouse = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eMOUSE_TYPE);
                g_vOldFreeCamMouse.x = SysInput::instance->Analog(iMouse, 0);
                g_vOldFreeCamMouse.y = SysInput::instance->Analog(iMouse, 1);
                g_vOldFreeCamMouse.z = 0.0f;
            }
        }

        if (!m_KeyConOn)
        {
            return false;
        }

        if (a_SubSpeed.Digital())
        {
            m_fCameraMoveSpeed *= 0.71428567f;
            if (m_fCameraMoveSpeed < 1.0f)
            {
                m_fCameraMoveSpeed = 1.0f;
            }
        }
        else if (a_AddSpeed.Digital())
        {
            m_fCameraMoveSpeed *= 1.4f;
            if (m_fCameraMoveSpeed > 2000.0f)
            {
                m_fCameraMoveSpeed = 2000.0f;
            }
        }

        if (a_BaseCameraToggleAlignment.Digital())
        {
            m_bAlignment = !m_bAlignment;
        }

        ZGEOM* pGeom = GetGeom();
        ZASSERT(pGeom);
        if (pGeom)
        {
            KeyControlGeom(pGeom->BaseGeom(), m_fCameraMoveSpeed);

            ZMat3x3 matrix;
            ZVector3 vPos;
            pGeom->GetRootTM(matrix, vPos);

            const int iMouse = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eMOUSE_TYPE);
            ZVector3 vMouse;
            vMouse.x = SysInput::instance->Analog(iMouse, 0);
            vMouse.y = SysInput::instance->Analog(iMouse, 1);
            vMouse.z = 0.0f;

            if (!m_bAlignment
                && (vMouse.x != 0.0f || vMouse.y != 0.0f)
                && (vMouse.x - g_vOldFreeCamMouse.x) * (vMouse.x - g_vOldFreeCamMouse.x)
                 + (vMouse.y - g_vOldFreeCamMouse.y) * (vMouse.y - g_vOldFreeCamMouse.y) > 0.00012207031f)
            {
                // PC: MouseConVect1(&vForward, &g_vOldFreeCamMouse, &vMouse, fRotSpeed, fRotSpeed);
                //     fRotSpeed = m_fCameraRotSpeed * 0.0060000001f
                ZVector3 vForward(matrix.ZAxis());
                const float fRotSpeed = m_fCameraRotSpeed * 0.0060000001f;
                MouseConVect1(&vForward.x, &g_vOldFreeCamMouse.x, &vMouse.x, fRotSpeed, fRotSpeed);
                createmat(&matrix.data[0], &vForward.x, nullptr);
                pGeom->SetRootTM(matrix, vPos);
            }

            g_vOldFreeCamMouse = vMouse;
        }

        return true;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // Reversed from PS2 ZBaseCamera::Info chain at 0x00950168 (names/filter/order).
        static RTP::ZDataProperty<ZGEOMREF> ZBaseCameraFollowGeom {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_rFollowGeom",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__5,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZBaseCamera, m_rFollowGeom))
        };

        static RTP::ZDataProperty<float> ZBaseCameraMoveSpeed {
            .m_Node = {
                .m_Next = &ZBaseCameraFollowGeom.m_Node,
                .m_Name = "m_fCameraMoveSpeed",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__11,
            .m_Offset = CLASS_PROPERTY(ZBaseCamera, m_fCameraMoveSpeed)
        };

        static RTP::ZDataProperty<float> ZBaseCameraRotSpeed {
            .m_Node = {
                .m_Next = &ZBaseCameraMoveSpeed.m_Node,
                .m_Name = "m_fCameraRotSpeed",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__11,
            .m_Offset = CLASS_PROPERTY(ZBaseCamera, m_fCameraRotSpeed)
        };

        static RTP::ZDataProperty<bool> ZBaseCameraKeyConOn {
            .m_Node = {
                .m_Next = &ZBaseCameraRotSpeed.m_Node,
                .m_Name = "m_KeyConOn",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZBaseCamera, m_KeyConOn)
        };

        static RTP::ZDataProperty<bool> ZBaseCameraAlignment {
            .m_Node = {
                .m_Next = &ZBaseCameraKeyConOn.m_Node,
                .m_Name = "m_bAlignment",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZBaseCamera, m_bAlignment)
        };

        static RTP::ZDataProperty<bool> ZBaseCameraEnabled {
            .m_Node = {
                .m_Next = &ZBaseCameraAlignment.m_Node,
                .m_Name = "m_bEnabled",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZBaseCamera, m_bEnabled)
        };

        static RTP::ZDataProperty<bool> ZBaseCameraActivated {
            .m_Node = {
                .m_Next = &ZBaseCameraEnabled.m_Node,
                .m_Name = "m_bActivated",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZBaseCamera, m_bIsActive)
        };
    }

    DEFINE_ROUT_CLASS(
        ZBaseCamera,
        ZCAMERA,
        BaseCamera,
        304,
        0,
        0x008122A4,
        cProperties::ZBaseCameraActivated,
        ZCAMERA
    );
#   pragma endregion

    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgRequestQuality, 0x009ACDD0, {"Camera_RequestQuality"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgSendQuality, 0x009ACDDC, {"Camera_SendQuality"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgActivateCamera, 0x009ACDE8, {"MainCameraControl_ActivateCamera"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgDeactivateCamera, 0x009ACDF4, {"MainCameraControl_DeactivateCamera"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgEnableCamera, 0x009ACE00, {"EnableCamera"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgDisableCamera, 0x009ACE0C, {"DisableCamera"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgAddCamToSort, 0x009ACE18, {"Camera_Add"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgRemoveCamFromSort, 0x009ACE24, {"Camera_Remove"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgResetCamToBestPos, 0x009ACE30, {"MSG_RESETCAMTOBESTPOS"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgFollowGeom, 0x009ACE3C, {"CamFollowGeom"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgGetFollowGeom, 0x009ACE48, {"MainCameraControl_GetFollowGeom"});
    STATIC_CLASS_VAR_IMPL(ZBaseCamera, ZMessageResolver, m_msgIsFreeCam, 0x009ACE54, {"MainCameraControl_IsFreeCam"});
}
