#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>

#include <BloodMoney/Game/UI/UIFWD.h>

namespace Hitman::BloodMoney
{
    class ZWINDOW
    {
    public:
        /// === members ===
        Glacier::ZBaseGeom* m_baseGeom; //0x0004
        char pad_0008[64]; //0x0008
        int32_t m_topWindowID; //0x0048
        char pad_004C[52]; //0x004C

        /// === vftable ===
        virtual void Release(bool);
        virtual void PreSave(Glacier::ISerializerStream &);
        virtual void PostSave(Glacier::ISerializerStream &);
        virtual void PreLoad(Glacier::ISerializerStream &);
        virtual void PostLoad(Glacier::ISerializerStream &);
        virtual void PostProcess(Glacier::uint,Glacier::uint);
        virtual void LoadSave(Glacier::ISerializerStream &,bool);
        virtual void LoadObject(Glacier::IInputSerializerStream &);
        virtual void SaveObject(Glacier::IOutputSerializerStream &);
        virtual void ExchangeObject(Glacier::ISerializerStream &);
        virtual void SetToDefault();
        virtual int GetTypeID();
        virtual void* GetProperties();
        virtual int GetObjectId();
        virtual void GetObjectIdAndMask(Glacier::uint &,Glacier::uint &);
        virtual Glacier::ZRTTI* GetOldClassInfo();
        virtual void DoInit();
        virtual int CalcCenSize();
        virtual void PreSaveGame();
        virtual void CheckPointSave(Glacier::ZCheckPointBuffer &);
        virtual void CheckPointLoad(Glacier::ZCheckPointBuffer &);
        virtual void Activate(bool);
        virtual void MakeInactive();
        virtual void MakeActive();
        virtual void GetBoundTreeType();
        virtual void AnimCallBackToId(Glacier::AnimCallback_t callback);
        virtual void AnimCallBackFromId(int);
        virtual void CameraMessages(bool);
        virtual void CreateExData();
        virtual void FreeExData();
        virtual void CopyExData(Glacier::ZGEOM const*);
        virtual void GetChunkData(int);
        virtual void RecurGetNext(Glacier::ZBaseGeom const**);
        virtual void DisableParentBoundAdjust();
        virtual void ExpandBounds(float *,float *,float *,Glacier::ZBaseGeom *);
        virtual void SetMat(float const*);
        virtual void SetPos(float const*);
        virtual void SetMatPos(float const*,float const*);
        virtual void SetWorldPosition(float const*);
        virtual void SetRootTM(float const*,float const*);
        virtual void Display(bool);
        virtual void Select(bool);
        virtual void Hide(bool);
        virtual void HideRecursive(bool);
        virtual void Freeze(bool);
        virtual void DispBound(bool);
        virtual void SetMoving(bool);
        virtual void RequestCustomDraw();
        virtual bool HasOwnerDraw();
        virtual void OwnerDraw(Glacier::ZBaseGeom *, Glacier::uint, Glacier::ZDrawBuffer*, Glacier::ZCAMERA*, float const*, float const*, void const*);
        virtual void CorrectOwnerDrawMatrix(float *,float *, Glacier::ZBaseGeom*, Glacier::uint);
        virtual void CorrectOwnerDrawPartMatrix(Glacier::ZMat3x3 *,Glacier::ZBaseGeom *,Glacier::ZBone *);
        virtual bool WantDrawBufferControl();
        virtual void DrawUpdate();
        virtual void DrawBufferViewUpdate(Glacier::ZDrawBuffer *,Glacier::ZCameraSpace *);
        virtual void DrawBufferViewUpdate(Glacier::ZDrawBuffer *,Glacier::ZBaseGeom *,Glacier::uint);
        virtual void WantViewPrimHideUnhideRequest();
        virtual void WantViewPrimHide(Glacier::uchar,bool);
        virtual void WantViewPrimHideMirrors(Glacier::uchar);
        virtual void WantViewUseAllLOD();
        virtual void GetLODMaskOverride();
        virtual int GetSortPriority();
        virtual void CopyEvents(Glacier::ZGEOM const*);
        virtual Glacier::ZEventBase* FindEvent(char const*);
        virtual void* GetEventData(char *);
        virtual void AddEvent(char const*);
        virtual void AttachEvent(Glacier::ZEventBase &);
        virtual void EnableClassCall(Glacier::uint);
        virtual void DisableClassCall(Glacier::uint);
        virtual void SetClassTimerInterval(float);
        virtual void CallEvents(int,void *,int, Glacier::ZGEOM*);
        virtual void SendCommand(Glacier::ZGEOM*, Glacier::ZMSGID, void *);
        virtual void SendCommand(Glacier::uint, Glacier::ZMSGID, void *);
        virtual void SendCommand(Glacier::ZMSGID, void*, Glacier::ZGEOM*);
        virtual void SendCommandRecursive(Glacier::ZMSGID, void *, Glacier::ZGEOM *);
        virtual void SendCommandRecursive(Glacier::uint, Glacier::ZMSGID,void *);
        virtual void FreeEvents();
        virtual void FreeEvent(Glacier::ZEventBase *);
        virtual void FreeEvent(char const*);
        virtual void RemoveEventFromList(Glacier::ZEventBase *);
        virtual void RegisterInstance(Glacier::uint);
        virtual void ClassInit();
        virtual void ClassInit2();
        virtual void PostClassInit();
        virtual void PostClassInit2();
        virtual void ClassDisplay();
        virtual void ClassFrameUpdate();
        virtual void ClassTimeUpdate();
        virtual void ClassCommand(Glacier::ZMSGID, void *);
        virtual void ClassPushPopScene(bool);
        virtual bool ChkLineColi(Glacier::COLI *,bool);
        virtual bool CheckPointInside(float const*,float);
        virtual bool CheckPointInsideBound(float const*);
        virtual bool CheckBoxInside(float const*,float const*,float const*);
        virtual void GetPointInsideDistance(float const*);
        virtual void Visible();
        virtual void Invisible();
        virtual void PushState();
        virtual void PopState();
        virtual void DuplicateData(Glacier::ZGEOM*);
        virtual void Duplicate(Glacier::ZGROUP*,char const*,bool);
        virtual void DuplicateInit(Glacier::ZGROUP*,float const*,float const*,char const*,bool);
        virtual void DuplicateToResource(Glacier::ZGROUP*,Glacier::uint,char const*,bool);
        virtual void DuplicateToResourceInit(Glacier::ZGROUP*,Glacier::uint,float const*,float const*,char const*,bool);
        virtual void CopyData(Glacier::ZGEOM const*);
        virtual void AddActionMap(Glacier::SInputActionDefinition *,char *);
        virtual void RemoveActionMap(char *);
        virtual void CheckAction(int,void *);
        virtual void CheckAction(char *,void *);
        virtual void CheckAction(Glacier::SInputAction*,void *);
        virtual int GetActionID(char *);
        virtual Glacier::SInputAction* GetActionPtr(int);
        virtual Glacier::SInputAction* GetActionPtr(char *);
        virtual bool IsAttached();
        virtual void SetAttachedTo(Glacier::ZLNKOBJ*);
        virtual Glacier::ZLNKOBJ* GetAttachedTo();
        virtual bool IsLinkBaseObj();
        virtual void OnCameraEnter();
        virtual bool IsRecursiveActivateAllowed();
        virtual void DynamicGroupOnScreen(void);
        virtual bool CheckBoxInside2(float const*,float const*,float const*);
        virtual void FindLoadWorldGeom(char const*);
        virtual Glacier::ZGEOM* FindMasterGeom(char const*);
        virtual Glacier::ZGEOM* FindGeom(char const*, Glacier::ZBaseGeom *);
        virtual int GroupDepth();
        virtual float GetPFResMultiplier();
        virtual void LinkBound(Glacier::uint);
        virtual void RemoveBound(Glacier::uint);
        virtual void GetAmbientSettings(float const*,float *,float *,float *);
        virtual void CreateParentsRecur(Glacier::ZGROUP*, Glacier::ZGROUP**, bool);
        virtual void SetOverRideNearFar(float *);
        virtual void* CorrectCenSizeRecur();
        virtual void* CorrectCenSize();
        virtual void InvalidateBounds();
        virtual void AttachGeom(Glacier::ZBaseGeom*, bool);
        virtual void AttachGeom(Glacier::ZGEOM*, bool);
        virtual void DetachGeom(Glacier::ZBaseGeom* ,bool);
        virtual void RecurGetNextGroup(Glacier::ZBaseGeom**);
        virtual void RecurGetNextExclRoom(Glacier::ZBaseGeom**);
        virtual void SetGroupControl(Glacier::uint, Glacier::uint);
        virtual void* GroupControl();
        virtual void ResetGroupPosition(bool);
        virtual void MakeActiveRecursive();
        virtual void GetStaticLights(Glacier::ZBaseGeom **, Glacier::ZBaseGeom **);
        virtual void* CalcCenSizeRecur();
        virtual void GetCenSizeRecur(float *,float *,bool);
        virtual void FindMaskGeom(char const*,int);
        virtual ZWINDOWS* GetSystem();
        virtual void WndMessage(Glacier::ZWMEVENT*);
        virtual void GetMouseColi(Glacier::SMouseColi *,float *,float *);
        virtual void RecalcMaxMin();
        virtual void CalcRealCenSize(float *,float *);
        virtual void SetText(char const*);
        virtual void SetAlpha(int);
        virtual void SetLineSpacing(int);
        virtual void OnWindowOpen(Glacier::uint, bool);
        virtual void OnWindowClose(Glacier::uint, bool);
        virtual ZWINGROUP* GetDefaultFocus();
        virtual void SetDefaultFocus(ZWINGROUP *);
    };
}