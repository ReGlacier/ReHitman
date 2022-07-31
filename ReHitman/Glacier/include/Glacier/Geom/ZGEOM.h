#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZOldTypeInfo.h>

namespace Glacier
{
    class ZRTTI;
    class ZEntityLocator;

    /**
     * @brief Reversed from cRTP::Properties container
     */
    enum EBoundingBox {
        BOUNDING_Static = 0,
        BOUNDING_Dynamic = 1,
        BOUNDING_DynamicAutoAssign = 2,
    };

    class ZGEOM
    {
    public:
        // Data
        ZEntityLocator* m_baseGeom; //+0x4
        int m_field8;               //+0x8
        uint16_t m_fieldC;          //+0xC
        uint16_t m_fieldE;          //+0xE

        // VFTable
        virtual void Release(bool);
        virtual void PreSave(ISerializerStream &);
        virtual void PostSave(ISerializerStream &);
        virtual void PreLoad(ISerializerStream &);
        virtual void PostLoad(ISerializerStream &);
        virtual void PostProcess(uint,uint);
        virtual void LoadSave(ISerializerStream &,bool);
        virtual void LoadObject(IInputSerializerStream &);
        virtual void SaveObject(IOutputSerializerStream &);
        virtual void ExchangeObject(ISerializerStream &);
        virtual void SetToDefault();
        virtual void* GetTypeID(); // Always nullptr, better to use GetOldClassInfo
        virtual void GetProperties();
        virtual int GetObjectId();
        virtual void GetObjectIdAndMask(uint &,uint &);
        virtual ZRTTI* GetOldClassInfo();
        virtual void DoInit();
        virtual void CalcCenSize(); // #17, pure virtual
        virtual void PreSaveGame();
        virtual void CheckPointSave(ZCheckPointBuffer &);
        virtual void CheckPointLoad(ZCheckPointBuffer &);
        virtual void Activate(bool);
        virtual void MakeInactive();
        virtual void MakeActive();
        virtual void GetBoundTreeType();
        virtual void AnimCallBackToId(void* callback /* bool (ZGEOM::*);(Animation::ActiveAnimation *,float,float,uint); */);
        virtual void AnimCallBackFromId(int);
        virtual void CameraMessages(bool);
        virtual void CreateExData();
        virtual void FreeExData();
        virtual void CopyExData(ZGEOM const*);
        virtual void GetChunkData(int);
        virtual void RecurGetNext(ZEntityLocator**);
        virtual void DisableParentBoundAdjust();
        virtual void ExpandBounds(float *,float *,float *,ZEntityLocator *);
        virtual void SetMat(const Glacier::ZMat3x3*);                    //#35 at 0x0076A200
        virtual void SetPos(const Glacier::ZVector3*);                   //#36 at 0x0076A204
        virtual void SetMatPos(const Glacier::ZMat3x3* mat, const Glacier::Vector3* pos);               //#37 at 0x0076A208
        virtual void SetWorldPosition(float const*);                     //#38 at 0x0076A20C
        virtual void SetRootTM(float const*,float const*);               //#39 at 0x0076A210
        virtual void Display(bool);                                      //#40 at 0x0076A214
        virtual void Select(bool);                                       //#41 at 0x0076A218
        virtual void Hide(bool);                                         //#42 at 0x0076A21C
        virtual void UnknownMethod(bool flag);
        virtual void HideRecursive(bool);                                //#43 at 0x0076A220
        virtual void Freeze(bool);                                       //#44 at 0x0076A224
        virtual void DispBound(bool);                                    //#45 at 0x0076A228
        virtual void SetMoving(bool);
        virtual void RequestCustomDraw();
        virtual void HasOwnerDraw();
        virtual void OwnerDraw(ZEntityLocator *,uint,ZDrawBuffer *,ZCAMERA *,float const*,float const*,void const*);
        virtual void CorrectOwnerDrawMatrix(float *,float *,ZEntityLocator *,uint);
        virtual void CorrectOwnerDrawPartMatrix(ZMat4x4 *,ZEntityLocator *,ZBone *);
        virtual void WantDrawBufferControl();
        virtual void DrawUpdate();
        virtual void DrawBufferViewUpdate(ZDrawBuffer *,ZCameraSpace *);
        virtual void DrawBufferViewUpdate(ZDrawBuffer *,ZEntityLocator *,uint);
        virtual void WantViewPrimHideUnhideRequest();
        virtual void WantViewPrimHide(uchar,bool);
        virtual void WantViewPrimHideMirrors(uchar);
        virtual void WantViewUseAllLOD();
        virtual void GetLODMaskOverride();
        virtual void GetSortPriority();
        virtual void CopyEvents(ZGEOM const*);
        virtual ZEventBase* FindEvent(char const*);
        virtual int GetEventData(const char *);
        virtual void AddEvent(char const*);
        virtual void AttachEvent(ZEventBase &);
        virtual void EnableClassCall(uint);
        virtual void DisableClassCall(uint);
        virtual void SetClassTimerInterval(float);
        virtual void CallEvents(int,void *,int,ZGEOM*);
        virtual void SendCommand(ZGEOM*,ushort,void *);
        virtual void SendCommand(uint,ushort,void *);
        virtual void SendCommand(ushort,void *,ZGEOM*);
        virtual void SendCommandRecursive(ushort,void *,ZGEOM*);
        virtual void SendCommandRecursive(uint,ushort,void *);
        virtual void FreeEvents();
        virtual void FreeEvent(ZEventBase *);
        virtual void FreeEvent(char const*);
        virtual void RemoveEventFromList(ZEventBase *);
        virtual void RegisterInstance(uint);
        virtual void ClassInit();
        virtual void ClassInit2();
        virtual void PostClassInit();
        virtual void PostClassInit2();
        virtual void ClassDisplay();
        virtual void ClassFrameUpdate();
        virtual void ClassTimeUpdate();
        virtual void ClassCommand(ushort,void *);
        virtual void ClassPushPopScene(bool);
        virtual void ChkLineColi(COLI *,bool);
        virtual void CheckPointInside(float const*,float);
        virtual void CheckPointInsideBound(float const*);
        virtual void CheckBoxInside(float const*,float const*,float const*);
        virtual float GetPointInsideDistance(float const*); /// OK
        virtual void Visible();   /// NOP
        virtual void Invisible(); /// NOP
        virtual void PushState(); /// NOP
        //virtual void PopState();  /// NOP
        virtual void DuplicateData(ZGEOM*);
        virtual ZGEOM* Duplicate(ZGROUP *,char const*,bool);
        virtual ZGEOM* DuplicateInit(ZGROUP* to, const Glacier::ZMat3x3* mat, const Glacier::ZVector3* pos, char const* name, bool f4);
        virtual void DuplicateToResource(ZGROUP *,uint,char const*,bool);
        virtual void DuplicateToResourceInit(ZGROUP *,uint,float const*,float const*,char const*,bool);
        virtual void CopyData(ZGEOM const*);
        virtual void AddActionMap(SInputActionDefinition *,char *);
        virtual void RemoveActionMap(char *);
        virtual void CheckAction(int,void *);
        virtual void CheckAction(char *,void *);
        virtual void CheckAction(SInputAction *,void *);
        virtual int GetActionID(char *);
        virtual void* GetActionPtr(int);
        virtual void* GetActionPtr(char *);
        virtual bool IsAttached();
        virtual void SetAttachedTo(ZLNKOBJ *);
        virtual ZLNKOBJ* GetAttachedTo();
        virtual bool IsLinkBaseObj();
        virtual void OnCameraEnter();

        // API
        void GetRootTM(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos);
        void GetMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos);
        void GetRootPoint(Glacier::ZVector3* pos);
        void GetCen(Glacier::ZVector3* pos);
        void GetRootMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos);
        void GetLocalPoint(Glacier::ZVector3* pos);
        void Zvmmul(Glacier::ZVector3* pos);
        ZREF GetRef();

        // Static API
        static ZGEOM* RefToPtr(unsigned int REF);
        static ZOldTypeInfo** GetFactory();
    };
}