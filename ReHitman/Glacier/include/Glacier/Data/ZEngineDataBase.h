#pragma once

#include <Glacier/Com/CCom.h>
#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZLinkedListHeader.hpp>
#include <Glacier/ZEntityTracker.h>
#include <Glacier/Data/ZScene.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/EventBase/ZEventList.h>

#include <Glacier/PF4/Fwd.h>

#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/ZSTL/ZHash.h>
#include <Glacier/ZSTL/MYSTR.h>

#include <Glacier/Geom/ZGEOM.h>

namespace Glacier
{
    // fwds
    class ZSaveClass;
    class STRREFTAB;
    class ZGeomBuffer;
    struct IInputStream;
    struct ZGEOMCLASSINFO;
    struct ZROUTCLASSINFO;
    struct CGlobalCom;
    class ZScheduledUpdate;
    class ZSoundObject;

    enum EGizmoType : uint32_t
    {
        eGIZMO_LIGHT = 0,
        eGIZMO_PRIMITIVE = 1,
        eGIZMO_STDOBJ = 2,
        eGIZMO_SNDOBJ = 3,
        eGIZMO_GATE = 4,
        eGIZMO__MAX = 5,
    };

    struct SGeomTypeCount
    {
        uint32_t m_lGeomType;
        uint32_t m_lGeomCount;
        uint32_t m_lNoNeedExtraGeom;
    };
    RE_VERIFY_SIZE(SGeomTypeCount, 0xC);

    enum ALLOCSEQUENCESTATUS
    {
        AS_ZIPLOAD = 0,
        AS_DLCLOAD = 1,
        AS_TEXTURE = 2,
        AS_GEOMS = 3,
        AS_DATABLOCKS = 4,
        AS_INIT = 5,
        AS_INCLUDESCENE = 6,
    };

    struct ILoadCallBack
    {
        virtual void CallMe() = 0;
    };

    struct SMakeGeomDynamic
    {
        ZREF rGeom;
        uint32_t lControl;
    };

    class ZEngineDataBase : public ZSerializable
    {
    public:
        // types
        // constants
        static constexpr size_t MAXNRREGISTERMESSAGES = 0x400;
        // static
        // vtable
        ~ZEngineDataBase() override;
        void PreLoad(ISerializerStream& stream) override;
        void ExchangeObject(ISerializerStream& stream) override;

        // ZEngineDataBase :
        virtual void InitAllocSequencePercent(ZSWScene* pSceneWrapper, bool bPacked);
        virtual void EndAllocSequencePercent(ZSWScene* pSceneWrapper);
        virtual float SetAllocSequencePercent(ALLOCSEQUENCESTATUS Status, char const* pText, float fPercent);
        virtual void SoundUpdate();
        virtual void MainLoop(bool bUpdateViews);
        virtual const char* GetSceneName();
        virtual ZROOM* AllocRootGroup();
        virtual void AllocSequence(struct ZSWScene* __formal);
        virtual bool ForceExtraGeom();
        virtual void CountNrGeoms(uint32_t& lNrBaseGeoms, uint32_t& lExtraGeomsSize, SGeomTypeCount& pGeomTypeCount, uint32_t lNrGeomTypes);
        virtual void DeleteAllGeoms();
        virtual void LoadBoundTrees();
        virtual void CreateBoundTrees();
        virtual void CreateRoomTrees();
        virtual void LoadRoomTrees();
        virtual void CreateSoundGraph();
        virtual void LoadSoundGraph();
        virtual void RegisterZDefine(char const* pName, char*, int);
        virtual ZMSGID RegisterZMsg(char const* pMsgName, uint32_t lForcedValue, const char* pFile, int Line);
        virtual const char* GetZMsgName(ZMSGID lMsgNumber);
        virtual void CreateObjectFactories();
        virtual bool StartUp();
        virtual void CloseDown();
        virtual void AddDlc(const char* pDllName);
        virtual void FreeDlcFiles();
        virtual void GetDefaultDLCFiles(STRREFTAB* pDlcFiles);
        virtual void ControlSceneChange();
        virtual void UnloadScene();
        virtual void LoadScene(char const* scene_name);
        virtual void CheckAndMakeStaticContainer();
        virtual void DoUnloadScene();
        virtual void FreeSceneMemory();
        virtual void PushValues(ZScene* pNewScene);
        virtual void InstallTextureBuffer();
        virtual uint32_t GetPrimsSize();
        virtual void GetPrimsData(void* pData, uint32_t lSize);
        virtual uint32_t GetGeomsSize();
        virtual void GetGeomsData(void* pData, uint32_t lSize);
        virtual IInputStream* CreatePropertyInputStream();
        virtual void CleanupPropertyData();
        virtual uint32_t GetStaticSize();
        virtual void GetStaticData(void* pData, uint32_t lSize);
        virtual uint32_t GetTextureSize();
        virtual void GetTextureData(void* pData, uint32_t lSize);
        virtual uint32_t GetMaterialsSize(void);
        virtual void GetMaterialsData(void* pData, uint32_t lSize);
        virtual uint32_t GetSoundDataSize();
        virtual void GetSoundData(void* pData, uint32_t lSize);
        virtual uint32_t GetWaveDataSize();
        virtual void GetWaveData(void* pData, uint32_t lSize);
        virtual uint32_t GetWaveHeaderDataSize();
        virtual void GetWaveHeaderData(void* pData, uint32_t lSize);
        virtual uint32_t GetAnimsSize();
        virtual void GetAnimsData(void* pData, uint32_t lSize);
        virtual uint32_t GetGeomFilesSize();
        virtual void GetGeomFilesData(void* pData, uint32_t lSize);
        virtual uint32_t GetRoomColiTreeSize();
        virtual void GetRoomColiTreeData(void* pData, uint32_t lSize);
        virtual uint32_t GetRoomInsideTreeSize();
        virtual void GetRoomInsideTreeData(void* pData, uint32_t lSize);
        virtual uint32_t GetGlobalColiTreeSize();
        virtual void GetGlobalColiTreeData(void* pData, uint32_t lSize);
        virtual uint32_t GetGlobalStripColiTreeSize();
        virtual void GetGlobalStripColiTreeData(void* pData, uint32_t lSize);
        virtual ZCAMERA* CreateDefaultCam(ZCAMERA* pCamera);
        virtual void CorrectEditorDestGroup(SCompiledGeom* pCompiledGeom, ZGROUP* pCurrentDestGroup);
        virtual void PackHookMissingOnlyInitialize();
        virtual void CreatePackedStaticGameLevelData();
        virtual void LoadPackedStaticGameLevelData();

        // methods
        ZEngineDataBase(const char* pFileName);

        // TODO: Make complete list of actual methods!!!
        void Initialize(const char* pFileName);
        void NewEventClass(ZEventBase* pEvent);
        void DeleteEventClass(ZEventBase* pEvent);
        ZScheduledUpdate& GetEventScheduler();
        void SetOnlyEventUpdate(ZEventBase* pEvent);
        ZEventBase* GetOnlyEventUpdate() const;
        bool IsPaused() const;
        bool CheckInPackBuffer(const void* ptr) const;
        bool CheckInAnimBuffer(const void* ptr) const;
        ZEventBase* AllocGeomCallEvent(ZGEOM* pGeom);
        bool ResourcesDisabled() const;
        void EnableResources();
        void DisableResources();
        CListUser* GetListUser() const;
        void DeleteCheck(void* ptr) const;
        bool RunTime() const;
        void MarkRunTime();
        void MarkNonRunTime();
        void UnlockMinMax();
        void LockMinMax();
        bool MinMaxLocked() const;
        void UnlockScene();
        ZREF GetREFByName(const char* pszName) const;
        ZGEOM* GeomRefToPtr(ZREF rGeom) const;
        CCom* GetSceneCom();
        ZREF GetSceneVar(const char* varname) const;
        ZSoundObject* SRefToPtr(Glacier::ZREF sref);
	    ZGEOMCLASSINFO* GetGeomClassInfo(uint32_t lTypeId);
        ZROUTCLASSINFO* GetRoutClassInfo(const char* pszRoutInfo);
        void FreeMsgValues();
        uint32_t GetTreesSize();
        void GetTreesData(void* pData, unsigned int lSize);
        uint32_t GetStaticGameLevelDataSize();
        void GetStaticGameLevelData(void* pData, unsigned int lSize);
        void DoLoadScene();
        MYSTR CalcCacheFileName(MYSTR sFileName, const char* pExt);
        void NetworkUpdate();
        uint32_t GetNextAnimId();
        void SetLoadCallBack(ILoadCallBack* pCallBack);
        bool IsLoadingGame() const;
        bool IsSavingGame() const;
        void ScheduledUpdate();
        void FrameUpdate();
        bool IsFrozen() const;
        void PauseScene(bool bPause);
        void FreezeScene(bool bFreeze);
        ZREF SPtrToRef(ZSoundObject* pSoundObject) const;
        void PurgePrimBuffer();
        void InitPathfinder4Data(const char* pBuffer);
        bool InitPhysicsData(const char* pBuffer);
        bool IsDrawGizmoEnabled(EGizmoType eType) const;
        void EnableDrawGizmo(EGizmoType eType, bool bEnabled);
        void FreeRoutsLists();
        void FreeScheduledUpdate();
        void FreeLightTable();
        void DeleteBoundTrees();
        char* GetStaticBuffer();
        char* GetAnimBuffer();
        void CreateGeoms(REFTAB* prtCreatedGeoms, ZStackArray<1000, SMakeGeomDynamic>* pMakeDynArray, const char* pGeomsData, const char* pStaticBuffer, IInputStream& property_in_stream);
        void LoadProperties(uint32_t lNrPackedGeoms, ZBaseGeom** BaseGeoms, IInputSerializerStream& in);
        void LoadPropertiesRecursive(IInputSerializerStream& in, ZBaseGeom**& BaseGeoms, ZBaseGeom* pObject);
        void SetSaveObject(ZSaveClass* pSaveObj);
        void LoadZDefines(IInputSerializerStream& stream);
        uint32_t GetSoundGraphSize();
        void GetSoundGraphData(void* pData, uint32_t lSize);

#if 0   // TO FILTER & IMPL
        public: bool SaveGame(unsigned int);
        public: bool LoadGame(unsigned int);
        public: bool SaveGame(IOutputStream&, IOutputStream&);
        public: bool LoadGame();
        public: bool DeleteSaveGame(unsigned int);
        public: REF GetREFByName(const char*);
        public: REF GeomPtrToRef(const ZBaseGeom*);
        public: REF GeomPtrToRef(const ZGEOM*);
        public: ZBaseGeom* GeomRefToBasePtr(unsigned int);
        public: ZGEOM* GeomRefToPtr(unsigned int);
        public: void CalcAllMinMax();
        public: void CalcAllMinMax(REFTAB*);
        public: void ClearSaveLoadFlags();
        public: int32 GetSceneDepth();
        public: void MakeDynamicGeomsDynamic(MakeDynArray*);
        public: void MakeAutoAssignGeomsAutoAssign(MakeDynArray*);
        public: void DumpUsedResources();
        public: void Init();
        public: void PostInit();
        public: void PostInit2();
        protected: uint32 GetStaticGameLevelDataSize();
        protected: void GetStaticGameLevelData(void*, unsigned int);
#endif

        // Static methods
        static CGlobalCom* GetGlobalCom();

        // members
        bool m_SavingGame;
        bool m_LoadingGame;
        ILoadCallBack* m_pLoadCallBack;
        ZSaveClass* m_pSaveObject;
        ZGeomBuffer* m_pGeomBuffer;
        PF4::ZInterface* m_pPathfinder4Data;
        Animation::Manager* m_AnimationManager;
        ZEntityTracker* m_pEntityTracker;
        ZREF m_rParticleControllerGeom;
        ZScene* m_pScene;
        CHUNKFILE* m_pPackedAnims;
        uint32_t m_lPackedAnimsLength;
        ZEventList m_EventList;
        ZScheduledUpdate* m_pScheduledUpdate;
        ZROOM* m_pRoot;
        float m_fEvenOutTimers;
        ResourceCollection* m_pLocaleResources;
        uint32_t m_lPackedTreeDataLength;
        uint8_t* m_pPackedTreeData;
        uint32_t m_lGlobalColiTreeLength;
        uint8_t* m_pGlobalColiTreeData;
        uint32_t m_lGlobalStripColiTreeLength;
        uint8_t* m_pGlobalStripColiTreeData;
        bool m_bInfAmmo;
        bool m_bLightDisplay;
        bool m_bCameraDisplay;
        bool m_bSoundDisplay;
        int m_lDontDrawFrame;
        bool m_bPause;
        bool m_bFrozen;
        uint8_t* m_pRoomColiTreeData;
        uint8_t* m_pRoomInsideTreeData;
        bool m_bRunTime;
        bool m_bPackTime;
        float m_fDisplayPercent;
        int m_lLockMinMax;
        MYSTR m_FileName;
        ZRTString m_ZMsgStrings[1024];
        ZPStrHash<unsigned int>* m_pZMessageHash;
        uint32_t m_iNumRegisteredMessages;
        uint8_t* m_pStaticBuffer;
        int m_lStaticBufferLength;
        CCom m_SceneCom;
        int m_lSceneDepth;
        int m_lNrEvents;
        bool m_bDrawGizmoEnabled[5];
        int m_iDrawColiMask;
        bool m_bStripViewEnabled;
        bool m_bWaterRenderingEnabled;
        CListUser* m_pListUser;
        int m_lDisableResources;
        int m_AnimIdCount;
        ZEventBase* m_pOnlyEventUpdate;
    };
    RE_VERIFY_SIZE(ZEngineDataBase, 0x52D4); // verified
    RE_VERIFY_OFFSET(ZEngineDataBase, m_FileName, 0xD4); // Verified ZEngineDataBase::ZEngineDataBase
    RE_VERIFY_OFFSET(ZEngineDataBase, m_SceneCom, 0x1164); // Verified ZEngineDataBase::ZEngineDataBase

    // Really weird, but it is
    #define g_pEngineData g_pSysInterface->m_pEngineData
    #define ZROOT g_pEngineData->m_pRoot
}
