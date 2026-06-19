#pragma once

#include <Glacier/CCom.h>
#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZEntityLocator.h> // ZBaseGeom
#include <Glacier/ZLinkedListHeader.hpp>
#include <Glacier/ZListNodeBase.h>
#include <Glacier/ZScheduledUpdate.h>
#include <Glacier/AnimationManager.h>

#include <Glacier/PF4/Fwd.h>

#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/ZSTL/ZHash.h>
#include <Glacier/ZSTL/MYSTR.h>

#include <Glacier/Geom/ZGEOM.h>

namespace Glacier
{
    class ZIOInputStream;
    struct ZGEOMCLASSINFO;

    struct SDynBlock
    {
        unsigned int* pNode1;
        unsigned int* pNode2;
        unsigned int* pNode3;
        unsigned int* pNextBlock;
    };
    RE_VERIFY_SIZE(SDynBlock, 0x10);

    struct CListUser : CMemPool
    {
        // vtbl
        virtual void AnalyzeCatch(unsigned int, void*);
        virtual bool IsNodeInList(unsigned int, void*);
        virtual unsigned int* UnfoldList(unsigned int*, unsigned int);
        virtual unsigned int* GetCatchBuffer(unsigned int*);
        virtual void DisconnectFromAllMembers(void*);
        virtual void NotifyAllMembers(void*);
        virtual bool DisconnectNodeFromNode(ZBaseGeom*, ZBaseGeom*);
        virtual void ConvertOffsetsToRefs(const unsigned int*);
        virtual unsigned int GetTotalBufferSize();
        virtual unsigned int* GetFullBuffer();
        virtual unsigned int AddRuntimeMember(void*);
        virtual void RemoveRuntimeMember(void*);

        // members
        unsigned int m_uSizeOfStaticBuf;
        unsigned int* m_pStaticBuf;
        unsigned int m_uSizeOfDynamicBuf;
        SDynBlock* m_pDynamicBuf;
        unsigned int m_uSizeOfRuntimeBuf;
        CMemPool* m_pGetDynEntryPool;
        unsigned int m_uMaxLength;
        unsigned int* m_pCollidedWith;
        unsigned int* m_pInternalListUnfold;
        unsigned int* m_pInternalOutBuf;
    };
    RE_VERIFY_SIZE(CListUser, 0x44);

    struct ZEventList
    {
        unsigned int m_NumberOfEvents;
        ZList<ZEventBase, 0, 0> m_EventLists[10];
        ZListIterator<ZEventBase, 0> m_Iterator;
        ZEventBase* m_pCurrentEvent;
    };
    RE_VERIFY_SIZE(ZEventList, 0x5C);

    struct ZScene
    {
        bool m_Changing;
        bool m_Loaded;
        char m_SceneName[260];
        LINKREFTAB* _pBigFiles;
        ZROOM* _pRoot;
        unsigned int _rEnvionmentRef;
        REFTAB* _pDisplayRouts;
        ZGEOMCLASSINFO* _pClassFirst;
        ZGEOMCLASSINFO* _pClassLast;
        CCom* _pSceneCom;
        float _FrameTime;
        float _PreFrameTime;
        float _ActTime;
        struct ZInputActions* _pInputActions;
        struct MMCHK* _pAllGroupsMMChk;
        uint8_t* _pStaticBuffer;
        int _lStaticBufferLength;
        CHUNKFILE* _pPackedAnims;
        int _lPackedAnimsLength;
        ZGeomBuffer* _pGeomBuffer;
        int _lLockMinMax;
        uint8_t* _pPackedTreeData;
        ZEventList* _pFrameUpdateList;
        uint8_t* m_WordTable;
    };

    struct ZEntityTracker
    {
        PF4::ZInterface* m_PathFinder;
        int m_ActorTypeId;
        int m_HeroTypeId;
        int m_ReservedPointId;
    };
    RE_VERIFY_SIZE(ZEntityTracker, 0x10);

    class ZEngineDataBase
    {
    public:
        /// === members ===
        bool m_SavingGame;
        bool m_LoadingGame;
        struct ILoadCallBack* m_pLoadCallBack;
        struct ZSaveClass* m_pSaveObject;
        struct ZGeomBuffer* m_pGeomBuffer;
        PF4::ZInterface* m_pPathfinder4Data;
        Animation::Manager* m_AnimationManager;
        ZEntityTracker* m_pEntityTracker;
        uint32_t m_rParticleControllerGeom;
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
        ZPStrHash* m_pZMessageHash;
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

        /// VFTABLE
        virtual void Release(); //#0
        virtual void PreSave(ISerializerStream &); //#2
        virtual void PostSave(ISerializerStream &); //#3
        virtual void PreLoad(ISerializerStream &); //#4
        virtual void PostLoad(ISerializerStream &); //#5
        virtual void PostProcess(uint,uint); //#6
        virtual void LoadSave(ISerializerStream &,bool); //#7
        virtual void LoadObject(IInputSerializerStream &); //#8
        virtual void SaveObject(IOutputSerializerStream &); //#9
        virtual void ExchangeObject(ISerializerStream &); //#10
        virtual void SetToDefault(void); //#11
        virtual void GetTypeID(void); //#12
        virtual void InitAllocSequencePercent(ZSWScene *,bool); //#13
        virtual void EndAllocSequencePercent(ZSWScene *); //#14
        virtual void SetAllocSequencePercent(ALLOCSEQUENCESTATUS,char const*,float); //#15
        virtual void SoundUpdate(void); //#16
        virtual void MainLoop(bool); //#17
        virtual const char* GetSceneName(void); //#18
        virtual ZGROUP* AllocRootGroup(void); //#19
        virtual void AllocSequence(ZSWScene *); //#20
        virtual void ForceExtraGeom(void); //#21
        virtual void CountNrGeoms(uint &,uint &,SGeomTypeCount *,uint); //#22
        virtual void DeleteAllGeoms(void); //#23
        virtual void LoadBoundTrees(void); //#24
        virtual void CreateBoundTrees(void); //#25
        virtual void CreateRoomTrees(void); //#26
        virtual void LoadRoomTrees(void); //#27
        virtual void CreateSoundGraph(void); //#28
        virtual void LoadSoundGraph(void); //#29
        virtual ZMSGID RegisterZDefine(char const*,char *,int); //#30
        virtual ZMSGID RegisterZMsg(char const*,uint, const char *,int); //#31
        virtual const char* GetZMsgName(ZMSGID); //#32
        virtual void CreateObjectFactories(void); //#33
        virtual void StartUp(void); //#34
        virtual void CloseDown(void); //#35
        virtual void UnknownFunction_36(); //#36
        virtual void UnknownFunction_37(); //#37
        virtual void UnknownFunction_38(); //#38
        virtual void ControlSceneChange(void); //#39
        virtual void UnloadScene(void); //#40
        virtual void LoadScene(char const*); //#41
        virtual void CheckAndMakeStaticContainer(void); //#42
        virtual void UnknownFunction_43(); //#43
        virtual void FreeSceneMemory(void); //#44
        virtual void PushValues(ZScene *); //#45
        virtual void InstallTextureBuffer(void); //#46
        virtual void GetPrimsSize(void); //#47
        virtual void GetPrimsData(void *,uint); //#48
        virtual void GetGeomsSize(void); //#49
        virtual void GetGeomsData(void *,uint); //#50
        virtual ZIOInputStream* CreatePropertyInputStream(); //#51
        virtual void CleanupPropertyData(void); //#512
        virtual void GetStaticSize(void); //#53
        virtual void GetStaticData(void *,uint); //#54
        virtual void GetTextureSize(void); //#55
        virtual void GetTextureData(void *,uint); //#56
        virtual void GetMaterialsSize(void); //#57
        virtual void GetMaterialsData(void *,uint); //#58
        virtual void GetSoundDataSize(void); //#59
        virtual void GetSoundData(void *,uint); //#60
        virtual void GetWaveDataSize(void); //#61
        virtual void GetWaveData(void *,uint); //#62
        virtual void GetWaveHeaderDataSize(void); //#63
        virtual void GetWaveHeaderData(void *,uint); //#64
        virtual uint32_t GetAnimsSize(void); //#65
        virtual void* GetAnimsData(void *,uint); //#66
        virtual uint32_t GetGeomFilesSize(void); //#67
        virtual void* GetGeomFilesData(void *,uint); //#68
        virtual uint32_t GetRoomColiTreeSize(void); //#69
        virtual void* GetRoomColiTreeData(void *,uint); //#70
        virtual uint32_t GetRoomInsideTreeSize(void); //#71
        virtual uint32_t GetRoomInsideTreeSize_(void); //#72
        virtual void* GetRoomInsideTreeData(void *,uint); //#73
        virtual uint32_t GetGlobalColiTreeSize(void); //#74
        virtual void* GetGlobalColiTreeData(void *,uint); //#75
        virtual uint32_t GetGlobalStripColiTreeSize(void); //#76
        virtual void* GetGlobalStripColiTreeData(void *,uint); //#77
        virtual ZCAMERA* CreateDefaultCam(ZCAMERA *); //#78
        virtual void CorrectEditorDestGroup(SCompiledGeom *,ZGROUP *); //#79
        virtual void PackHookMissingOnlyInitialize(void); //#80
        virtual void CreatePackedStaticGameLevelData(void); //#81
        virtual void LoadPackedStaticGameLevelData(void); //#82

        // API
        CCom* GetSceneCom();
        std::intptr_t GetSceneVar(const char* varname);
        std::intptr_t SRefToPtr(Glacier::ZREF sref);
        ZScheduledUpdate* GetEventScheduler();
	    ZOldTypeInfo* GetGeomClassInfo(uint32_t typeId);

        // Static methods
        static CCom* GetGlobalCom();
    };
    RE_VERIFY_SIZE(ZEngineDataBase, 0x52D4);
}