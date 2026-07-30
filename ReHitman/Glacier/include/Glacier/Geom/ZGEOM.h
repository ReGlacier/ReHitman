#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/ZOldTypeInfo.h>
#include <Glacier/Fysix/eGlobalTreeType.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/Runtime/ZRuntimeMacro.h>
#include <Glacier/Runtime/ZFactory.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    // fwds
    class ZRTTI;
    class ZBaseGeom;
    class ZTreeGroup;
    class ZBaseConRout;

    extern const char* g_pGeomName;
    extern ZBaseGeom* g_pBaseGeom;

    /**
     * @brief Reversed from cRTP::Properties container
     */
    enum EBoundingBox 
    {
        BOUNDING_Static = 0,
        BOUNDING_Dynamic = 1,
        BOUNDING_DynamicAutoAssign = 2,
    };

    class ZGEOM : public RTP::cBase
    {
    public:
#       pragma region " --- Factory<T> stuff --- "
        struct ZGeomCreator
        {
            struct Create
            {
                template <typename T>
                static T* Do(const ZGEOMCLASSINFO& sGeomClassInfo)
                {
                    ZGEOM* pExtraGeom = AllocExtraGeomHelper(sizeof(T));
                    if (pExtraGeom)
                    {
                        return znew_placement<T>(reinterpret_cast<T*>(pExtraGeom), g_pGeomName, g_pBaseGeom);
                    }

                    return nullptr;
                }
            };
        };

        DECLARE_FACTORY(ZGEOMCLASSINFO, uint32_t, ZGeomCreator);
#       pragma endregion

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

        using ActiveAnimCB = bool(ZGEOM::*)(Animation::ActiveAnimation *,float,float,uint32_t);

#       pragma region " --- Static members --- "
        // static
        STATIC_CLASS_VAR(ZGEOM, ZFactory<ZGEOM>, m_Factory);
        STATIC_CLASS_VAR(ZGEOM, const char*, FactoryName);
        STATIC_CLASS_VAR(ZGEOM, RTP::ZPropertyInfo, Info);
        STATIC_CLASS_VAR(ZGEOM, ZGEOMCLASSINFO*, m_OldClassInfo);
        STATIC_CLASS_VAR(ZGEOM, ZGEOM::EStatus, m_PreferedStatus);
        STATIC_CLASS_VAR_EZ(ZGEOM, ZGEOM*, s_pLastSendCommandCaller);
        DECLARE_ID_AND_MASK(ZGEOM);
#       pragma endregion

        // Data
        ZBaseGeom* m_baseGeom; //+0x4
        ExGeomData* m_pExData; //+0x8
        uint16_t m_lGeomControl; //+0xC
        EStatus m_eStatus; //+0xE
        uint8_t m_fieldE; //+0xF

        // VFTable
        ~ZGEOM() override;
        // ZSerializable
        void LoadSave(ISerializerStream& stream, bool bSaving) override;
        void LoadObject(IInputSerializerStream& stream) override;
        void SaveObject(IOutputSerializerStream& stream) override;
        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;
        // ZGEOM
        virtual uint32_t GetObjectId() const;
        virtual void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const;
        virtual ZGEOMCLASSINFO* GetOldClassInfo() const;
        virtual bool DoInit();
        virtual void CalcCenSize() = 0;
        virtual void PreSaveGame();
        virtual void CheckPointSave(ZCheckPointBuffer &);
        virtual void CheckPointLoad(ZCheckPointBuffer &);
        virtual void Activate(bool);
        virtual void MakeInactive();
        virtual void MakeActive();
        virtual eGlobalTreeType GetBoundTreeType() const;
        virtual int AnimCallBackToId(ActiveAnimCB pCallback);
        virtual ActiveAnimCB AnimCallBackFromId(int);
        virtual void CameraMessages(bool);
        virtual void CreateExData();
        virtual void FreeExData();
        virtual void CopyExData(const ZGEOM* Source);
        virtual CHUNKFILE* GetChunkData(int Name) const;
        virtual void RecurGetNext(ZBaseGeom**);
        virtual bool DisableParentBoundAdjust();
        virtual void ExpandBounds(ZMat3x3& mMat, ZVector3& vCen, ZVector3& vSize, ZBaseGeom* pBaseGeom);
        virtual void SetMat(const Glacier::ZMat3x3& mMat);
        virtual void SetPos(const Glacier::ZVector3& vPos);
        virtual void SetMatPos(const Glacier::ZMat3x3& mat, const Glacier::Vector3& pos);
        virtual void SetWorldPosition(const Glacier::Vector3& pos);
        virtual void SetRootTM(const ZMat3x3& RTMat, const ZVector3& RTPos);
        virtual void Display(bool OnOff);
        virtual void Select(bool OnOff);
        virtual void Hide(bool OnOff);
        virtual void UNKNOWN_FUCKING_METHOD(); // - IDK
        virtual void HideRecursive(bool bHide);
        virtual void Freeze(bool OnOff);
        virtual void DispBound(bool OnOff);
        virtual void SetMoving(bool bMoving);
        virtual bool RequestCustomDraw();
        virtual bool HasOwnerDraw() const;
        virtual void OwnerDraw(ZBaseGeom *,uint,ZDrawBuffer *,ZCAMERA *,float const*,float const*,void const*);
        virtual void CorrectOwnerDrawMatrix(ZMat3x3& mMat, ZVector3& vPos, ZBaseGeom *pOwnerBaseGeom, uint32_t lBoneId);
        virtual void CorrectOwnerDrawPartMatrix(ZMat4x4*,ZBaseGeom*);
        virtual bool WantDrawBufferControl() const;
        virtual void DrawUpdate();
        virtual bool DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace);
        virtual bool DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZBaseGeom* pOwnerBaseGeom, uint32_t lBoneId);
        virtual bool WantViewPrimHideUnhideRequest() const;
        virtual bool WantViewPrimHide(uint8_t lLODControl) const;
        virtual bool WantViewPrimHideMirrors(uint8_t lLODControl) const;
        virtual bool WantViewUseAllLOD() const;
        virtual uint8_t GetLODMaskOverride() const;
        virtual uint8_t GetSortPriority() const;
        virtual void CopyEvents(const ZGEOM* Source);
        virtual ZEventBase* FindEvent(char const* pEventName) const;
        virtual void* GetEventData(const char* pEventName) const;
        virtual ZBaseConRout* AddEvent(char const* pEventName);
        virtual void AttachEvent(ZEventBase& event);
        virtual void EnableClassCall(uint32_t Cases);
        virtual void DisableClassCall(uint32_t Cases);
        virtual void SetClassTimerInterval(float);
        virtual int32_t CallEvents(int Type, void* Param1, int16_t Param2, ZGEOM* pCaller);
        virtual void SendCommand(ZMSGID Msg, void* pData, ZGEOM* pTarget);
        virtual void SendCommand(ZREF TargetRef, ZMSGID Msg, void* pData);
        virtual void SendCommand(ZGEOM* pTarget, ZMSGID Msg, void* pData);
        virtual void SendCommandRecursive(ZREF rTarget, ZMSGID Msg, void* pData);
        virtual void SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget);
        virtual void FreeEvents();
        virtual void FreeEvent(char const* pEventName);
        virtual void FreeEvent(ZEventBase* pEvent);
        virtual void RemoveEventFromList(ZEventBase* pEvent);
        virtual void RegisterInstance(uint32_t lInstanceCount);
        virtual void ClassInit();
        virtual void ClassInit2();
        virtual void PostClassInit();
        virtual void PostClassInit2();
        virtual void ClassDisplay();
        virtual void ClassFrameUpdate();
        virtual void ClassTimeUpdate();
        virtual int32_t ClassCommand(ZMSGID Msg, void* pData);
        virtual void ClassPushPopScene(bool);
        virtual bool ChkLineColi(COLI* pColi, bool bWantInvisible);
        virtual bool CheckPointInside(ZVector3& pPoint, float fDotDist);
        virtual bool CheckPointInsideBound(ZVector3& pPoint);
        virtual bool CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0);
        virtual float GetPointInsideDistance(const ZVector3& vPos); /// OK
        virtual void Visible();
        virtual void Invisible();
        virtual void PushState();
        // On iOS and other platforms there are weird method PopState, but there are nothing lol
        virtual bool DuplicateData(ZGEOM* pFromGeom);
        virtual ZGEOM* Duplicate(ZGROUP* DestGroup, const char* DupName, bool Recursive);
        virtual ZGEOM* DuplicateInit(ZGROUP* DestGroup, const Glacier::ZMat3x3* pMat, const Glacier::ZVector3* pPos, char const* DupName, bool Recursive);
        virtual ZGEOM* DuplicateToResource(ZGROUP* DestGroup, uint32_t lGeomResourceId, const char* DupName, bool Recursive);
        virtual ZGEOM* DuplicateToResourceInit(ZGROUP* DestGroup, uint32_t lGeomResourceId, const ZMat3x3* mMat, const ZVector3* vPos, const char* DupName, bool Recursive);
        virtual void CopyData(const ZGEOM* Source);
        virtual void AddActionMap(SInputActionDefinition *,char *);
        virtual void RemoveActionMap(char *);
        virtual void CheckAction(int,void *);
        virtual void CheckAction(char *,void *);
        virtual void CheckAction(SInputAction *,void *);
        virtual int GetActionID(const char* psActionName);
        virtual SInputAction* GetActionPtr(int iActionId);
        virtual SInputAction* GetActionPtr(const char* psActionName);
        virtual bool IsAttached() const;
        virtual void SetAttachedTo(ZLNKOBJ* pAttachedTo);
        virtual ZLNKOBJ* GetAttachedTo() const;
        virtual bool IsLinkBaseObj() const;
        virtual void OnCameraEnter();
       
        // methods
        ZGEOM(const char* pName, ZBaseGeom* pBaseGeom);
        bool IsInitialized() const;
        void GetRootTM(ZMat3x3& mat, ZVector3& pos) const;
        void GetMatPos(ZMat3x3& mat, ZVector3& pos) const;
        void GetRootPoint(Glacier::ZVector3& pos) const;
        void GetCen(Glacier::ZVector3& pos) const;
        void GetRootMatPos(Glacier::ZMat3x3& mat, Glacier::ZVector3& pos) const;
        void GetLocalPoint(Glacier::ZVector3& pos) const;
        void Zvmmul(Glacier::ZVector3& pos) const;
        uint32_t GetGeomType() const;
        ZREF GetRef() const;
        ZBaseGeom* BaseGeom() const { return m_baseGeom; }
        MYSTR CalcTotalName(bool bRoot) const;
        void CalcTotalNameRecur(MYSTR& sName, bool bRoot) const;
        void Delete();
        ZGROUP* Parent() const;
        ZTreeGroup* GetTreeGroup() const;
        ZTreeGroup* GetDynamicTreeGroup() const;
        void ChangeEventsActivity();
        void CopyEvent(const ZEventBase* pEvent);
        void VerifyPointers() const;
        bool Active() const;
        uint32_t Prim() const;
        const ZMat3x3& Mat() const;
        const ZVector3& Pos() const;
        const ZVector3& Cen() const;
        const ZVector3& Size() const;
        const float Radius() const;
        const char* Name() const;
        ZGROUP* GetOwner(bool bCheckWorldGroups) const;
        bool ChkEvents() const;
        void SetName(char const* pName);
        void SetMatSimple(const ZMat3x3& mMat);
        void SetPosSimple(const ZVector3& vPos);
        void SetMatPosSimple(const ZMat3x3& mMat, const ZVector3& vPos);
        void SetRootPos(const ZVector3& vPos);
        void SetAutoRoomAssign(bool bAutoAssign);
        void MakeDynamic(bool bDynamic);
        uint32_t Control() const;
        uint16_t GeomControl() const;
        void SetGeomControl(uint16_t lBitsAdd, uint16_t lBitsRem);
        void SetControl(uint32_t lBitsAdd, uint32_t lBitsRem);

        // RTTI custom methods
#       pragma region " --- RTTI generated stuff --- "
        void GetBoundingBox(EBoundingBox& /*bbox*/) { /* there are no code, IOI did this */ }
        void SetBoundingBox(const EBoundingBox& /*bbox*/) { /* there are no code, IOI did this */ }
        void GetMatrix(float (&mMat)[9]);
        void SetMatrix(const float (&mMat)[9]);
        void GetPosition(float (&)[3]);
        void SetPosition(float const (&)[3]);
        void GetInactive(bool&);
        void SetInactive(bool const&);
        void GetPrim(uint32_t&);
        void SetPrim(uint32_t const&);
#       pragma endregion

        template <typename T>
        bool IsDerivedFrom() const
        {
            static_assert(std::is_base_of_v<ZGEOM, T>, "T must be derived from ZGEOM");
            return T::m_Id == (GetObjectId() & T::m_Mask);
        }

        template <typename T>
        bool Is() const
        {
            return GetObjectId() == T::m_Id;
        }

        // Static API
        static ZGEOM* RefToPtr(unsigned int REF);
		template <typename T> static T* RefCast(Glacier::ZREF rRef) { return reinterpret_cast<T*>(RefToPtr(rRef)); }
        static ZFactory<ZGEOM>& GetFactory();
        static ZGEOM* AllocExtraGeomHelper(uint32_t iExtraGeomBlockSize);
        static const RTP::ZPropertyInfo& Properties();
        static uint32_t GetClassId();
        static void GetClassIdAndMask(uint32_t& lClassId, uint32_t& lMask);
        static void SetPreferedStatus(EStatus status);
    };
    RE_VERIFY_SIZE(ZGEOM, 0x10);
    RE_VERIFY_OFFSET(ZGEOM, m_baseGeom, 0x4);
    RE_VERIFY_OFFSET(ZGEOM, m_pExData, 0x8);
    RE_VERIFY_OFFSET(ZGEOM, m_lGeomControl, 0xC);
    RE_VERIFY_OFFSET(ZGEOM, m_eStatus, 0xE);
    RE_VERIFY_OFFSET(ZGEOM, m_fieldE, 0xF);
}

// Macro from IOI assert stuff
#pragma region " --- Macros --- "
#   define GREFTOPTR(r) Glacier::ZGEOM::RefToPtr(r)
#   define GREFTOBASEPTR(r) Glacier::ZBaseGeom::RefToPtr(r)
#pragma endregion
