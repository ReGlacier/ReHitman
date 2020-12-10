#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZGROUP
    {
    public:
        //vftable
        virtual void Release(bool);
        virtual void PreSave(int&);
        virtual void PostSave(int&);
        virtual void PreLoad(int&);
        virtual void PostLoad(int&);
        virtual void PostProcess(unsigned int, unsigned int);
        virtual void LoadSave(int&, bool);
        virtual void LoadObject(int&);
        virtual void SaveObject(int&);
        virtual void ExchangeObject(int&);
        virtual void SetToDefault();
        virtual int* GetTypeID();
        virtual int* GetProperties() const;
        virtual int* GetObjectId() const;
        virtual int* GetObjectIdAndMask(unsigned int&, unsigned int&) const;
        virtual int* GetOldClassInfo(); const
        virtual void DoInit();
        virtual void CalcCenSize();
        virtual void PreSaveGame();
        virtual void CheckPointSave(int&);
        virtual void CheckPointLoad(int&);
        virtual void Activate(bool);
        virtual void MakeInactive();
        virtual void MakeActive();
        virtual int* GetBoundTreeType();
        virtual void AnimCallBackToId(void* cb);
        virtual void AnimCallBackFromId(int);
        virtual void CameraMessages(bool);
        virtual void CreateExData();
        virtual void FreeExData();
        virtual void CopyExData(ZGEOM const*);
        virtual int* GetChunkData(int);
        virtual void RecurGetNext(ZEntityLocator const**);
        virtual void DisableParentBoundAdjust();
        virtual void ExpandBounds(float*, float*, float*, ZEntityLocator*);
        virtual void SetMat(float const*);
        virtual void SetPos(float const*);
        virtual void SetMatPos(float const*, float const*);
        virtual void SetWorldPosition(float const*);
        virtual void SetRootTM(float const*, float const*);
        virtual void Display(bool);
        virtual void Select(bool);
        virtual void Hide(bool);
        virtual void HideRecursive(bool);
        virtual void Freeze(bool);
        virtual void DispBound(bool);
        virtual void SetMoving(bool);
        virtual void RequestCustomDraw();
        virtual void HasOwnerDraw();
        virtual void OwnerDraw(ZEntityLocator*, unsigned int, int*, int*, float const*, float const*, void const*);
        virtual void CorrectOwnerDrawMatrix(float*, float*, ZEntityLocator*, unsigned int);
        virtual void CorrectOwnerDrawPartMatrix(Glacier::ZMat3x3*, ZEntityLocator*, int*);
        virtual void WantDrawBufferControl();
        virtual void DrawUpdate();
        virtual void DrawBufferViewUpdate(int*, int*);
        virtual void DrawBufferViewUpdate_2(int*, ZEntityLocator*, unsigned int);
        virtual void WantViewPrimHideUnhideRequest();
        virtual void WantViewPrimHide(unsigned char, bool);
        virtual void WantViewPrimHideMirrors(unsigned char);
        virtual void WantViewUseAllLOD();
        virtual int* GetLODMaskOverride();
        virtual int* GetSortPriority();
        virtual void CopyEvents(ZGEOM const*);
        virtual int* FindEvent(char const*);
        virtual int* GetEventData(char*);
        virtual void AddEvent(char const*);
        virtual void AttachEvent(ZEventBase&);
        virtual void EnableClassCall(unsigned int);
        virtual void DisableClassCall(unsigned int);
        virtual void SetClassTimerInterval(float);
        virtual void CallEvents(int, void*, int, ZGEOM*);
        virtual void SendCommand(ZGEOM*, unsigned short, void*);
        virtual void SendCommand_2(unsigned int, unsigned short, void*);
        virtual void SendCommand_3(unsigned short, void*, ZGEOM*);
        virtual void SendCommandRecursive(unsigned short, void*, ZGEOM*);
        virtual void SendCommandRecursive_2(unsigned int, unsigned short, void*);
        virtual void FreeEvents();
        virtual void FreeEvent(ZEventBase*);
        virtual void FreeEvent_2(char const*);
        virtual void RemoveEventFromList(ZEventBase*);
        virtual void RegisterInstance(unsigned int);
        virtual void ClassInit();
        virtual void ClassInit2();
        virtual void PostClassInit();
        virtual void PostClassInit2();
        virtual void ClassDisplay();
        virtual void ClassFrameUpdate();
        virtual void ClassTimeUpdate();
        virtual void ClassCommand(unsigned short, void*);
        virtual void ClassPushPopScene(bool);
        virtual void ChkLineColi(int*, bool);
        virtual void CheckPointInside(float const*, float);
        virtual void CheckPointInsideBound(float const*);
        virtual void CheckBoxInside(float const*, float const*, float const*);
        virtual int* GetPointInsideDistance(float const*);
        virtual void Visible();
        virtual void Invisible();
        virtual void PushState();
        virtual void PopState();
        virtual void DuplicateData(ZGEOM*);
        virtual void Duplicate(ZGROUP*, char const*, bool);
        virtual void DuplicateInit(ZGROUP*, float const*, float const*, char const*, bool);
        virtual void DuplicateToResource(ZGROUP*, unsigned int, char const*, bool);
        virtual void DuplicateToResourceInit(ZGROUP*, unsigned int, float const*, float const*, char const*, bool);
        virtual void CopyData(ZGEOM const*);
        virtual void AddActionMap(int*, char*);
        virtual void RemoveActionMap(char*);
        virtual void CheckAction(int, void*);
        virtual void CheckAction_2(char*, void*);
        virtual void CheckAction_3(int*, void*);
        virtual int* GetActionID(char*);
        virtual int* GetActionPtr(int);
        virtual int* GetActionPtr_CStr(char*);
        virtual bool IsAttached();
        virtual void SetAttachedTo(ZLNKOBJ*);
        virtual int* GetAttachedTo();
        virtual bool IsLinkBaseObj();
        virtual void OnCameraEnter();
        virtual bool IsRecursiveActivateAllowed();
        virtual void DynamicGroupOnScreen();
        virtual void CheckBoxInside_2(float const*, float const*, float const*);
        virtual int* FindLoadWorldGeom(char const*);
        virtual int* FindMasterGeom(char const*);
        virtual ZGEOM* FindGeom(char const*, ZEntityLocator*);
        virtual int GroupDepth();
        virtual float GetPFResMultiplier();
        virtual void LinkBound(unsigned int);
        virtual void RemoveBound(unsigned int);
        virtual int* GetAmbientSettings(float const*, float*, float*, float*);
        virtual void CreateParentsRecur(ZGROUP*, ZGROUP**, bool);
        virtual void SetOverRideNearFar(float*);
        virtual void CorrectCenSizeRecur();
        virtual void CorrectCenSize();
        virtual void InvalidateBounds();
        virtual void AttachGeom(ZEntityLocator*, bool);
        virtual void AttachGeom_2(ZGEOM*, bool);
        virtual void DetachGeom(ZEntityLocator*, bool);
        virtual void RecurGetNextGroup(ZEntityLocator const**);
        virtual void RecurGetNextExclRoom(ZEntityLocator const**);
        virtual void SetGroupControl(unsigned int, unsigned int);
        virtual int GroupControl();
        virtual void ResetGroupPosition(bool);
        virtual void MakeActiveRecursive();
        virtual int* GetStaticLights(ZEntityLocator**, ZEntityLocator**);
        virtual void CalcCenSizeRecur();
        virtual int* GetCenSizeRecur(float*, float*, bool);
        virtual int* FindMaskGeom(char const*, int);

        //data
        ZEntityLocator* m_pEntityLocator; //0x0004
        int32_t m_field8; //0x0008
        int32_t m_memchk; //0x000C
        float m_field10; //0x0010
        float m_field14; //0x0014
        float m_field18; //0x0018
        float m_field1C; //0x001C
        float m_field20; //0x0020
        float m_field24; //0x0024
        int32_t m_control; //0x0028
        int32_t m_field2C; //0x002C
        int32_t m_field30; //0x0030
        int32_t m_field34; //0x0034
        int32_t m_field38; //0x0038
        ZEntityLocator* m_pEntity0; //0x003C
        ZEntityLocator* m_pEntity1; //0x0040
        int32_t m_field44; //0x0044
        int32_t m_field48; //0x0048

        //API
        ZGEOM* CreateGeom(const char* name, int typeId, bool unk3);
        bool IsRoot();
    };
}