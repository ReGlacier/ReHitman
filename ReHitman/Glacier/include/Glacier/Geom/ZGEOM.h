#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZOldTypeInfo.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/RTP/Base.h>

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
        // Types
        enum EStatus : uint8_t
        {
            STATUS_New = 0x0,
            STATUS_Init = 0x1,
            STATUS_Init2 = 0x2,
            STATUS_Loaded = 0x3,
            STATUS_PostInit = 0x4,
            STATUS_PostInit2 = 0x5,
            STATUS_OK = 0x5, // Rly, same to PostInit2, weird shit
            STATUS_Remove = 0x6,
            STATUS_End = 0x7,
        };

        // Data
        ZBaseGeom* m_baseGeom; //+0x4
        ExGeomData* m_pExData; //+0x8
        uint16_t m_lGeomControl; //+0xC
        EStatus m_eStatus; //+0xE
        uint8_t m_fieldE; //+0xF

        // VFTable
        virtual void Release(bool);
        virtual void PreSave(ISerializerStream &);
        virtual void PostSave(ISerializerStream &);
        virtual void PreLoad(ISerializerStream &);
        virtual bool PostLoad(ISerializerStream &);
        virtual bool PostProcess(uint,uint);
        virtual void LoadSave(ISerializerStream &,bool);
        virtual void LoadObject(IInputSerializerStream &);
        virtual void SaveObject(IOutputSerializerStream &);
        virtual void ExchangeObject(ISerializerStream &);
        virtual void SetToDefault();
        virtual uint32_t GetTypeID();

        virtual const RTP::ZPropertyInfo* GetProperties();
        virtual uint32_t GetObjectId();
        virtual void GetObjectIdAndMask(uint &,uint &);
        virtual struct ZGEOMCLASSINFO* GetOldClassInfo();
        virtual bool DoInit();
        virtual void CalcCenSize();
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
        virtual void RecurGetNext(ZBaseGeom**);
        virtual void DisableParentBoundAdjust();
        virtual void ExpandBounds(float *,float *,float *, ZBaseGeom*);
        virtual void SetMat(const Glacier::ZMat3x3*);
        virtual void SetPos(const Glacier::ZVector3*);
        virtual void SetMatPos(const Glacier::ZMat3x3* mat, const Glacier::Vector3* pos);
        virtual void SetWorldPosition(float const*);
        virtual void SetRootTM(float const*,float const*);
        virtual void Display(bool);
        virtual void Select(bool);
        virtual void Hide(bool);
        virtual void UNKNOWN_FUCKING_METHOD(); // - IDK
        virtual void HideRecursive(bool);
        virtual void Freeze(bool);
        virtual void DispBound(bool);
        virtual void SetMoving(bool);
        virtual void RequestCustomDraw();
        virtual bool HasOwnerDraw();
        virtual void OwnerDraw(ZEntityLocator *,uint,ZDrawBuffer *,ZCAMERA *,float const*,float const*,void const*);
        virtual void CorrectOwnerDrawMatrix(float *,float *,ZEntityLocator *,uint);
        virtual void CorrectOwnerDrawPartMatrix(ZMat4x4 *,ZEntityLocator *,ZBone *);
        virtual void WantDrawBufferControl();
        virtual void DrawUpdate();
        virtual bool DrawBufferViewUpdate(ZDrawBuffer *,ZCameraSpace *);
        virtual bool DrawBufferViewUpdate(ZDrawBuffer *,ZEntityLocator *,uint);
        virtual bool WantViewPrimHideUnhideRequest();
        virtual bool WantViewPrimHide(uchar,bool);
        virtual bool WantViewPrimHideMirrors(uchar);
        virtual bool WantViewUseAllLOD();
        virtual uint8_t GetLODMaskOverride();
        virtual uint8_t GetSortPriority();
        virtual void CopyEvents(ZGEOM const*);
        virtual ZEventBase* FindEvent(char const*);
        virtual int GetEventData(const char *);
        virtual void AddEvent(char const*);
        virtual void AttachEvent(ZEventBase &);
        virtual void EnableClassCall(uint);
        virtual void DisableClassCall(uint);
        virtual void SetClassTimerInterval(float);
        virtual int CallEvents(int,void *,int,ZGEOM*);
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
        virtual void Visible();
        virtual void Invisible();
        virtual void PushState();
        //virtual void PopState();
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
		template <typename T> static T* RefCast(Glacier::ZREF rRef) { return reinterpret_cast<T*>(RefToPtr(rRef)); }
        static ZOldTypeInfo** GetFactory();
    };
    RE_VERIFY_SIZE(ZGEOM, 0x10);
    RE_VERIFY_OFFSET(ZGEOM, m_baseGeom, 0x4);
    RE_VERIFY_OFFSET(ZGEOM, m_pExData, 0x8);
    RE_VERIFY_OFFSET(ZGEOM, m_lGeomControl, 0xC);
    RE_VERIFY_OFFSET(ZGEOM, m_eStatus, 0xE);
    RE_VERIFY_OFFSET(ZGEOM, m_fieldE, 0xF);
}