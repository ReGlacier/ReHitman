#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZLinkedListHeader.hpp>
#include <Glacier/ZScheduledUpdate.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/CCom.h>

namespace Glacier
{
    class ZIOInputStream;

    class ZEngineDataBase
    {
    public:
        /// === members ===
        int32_t m_field4; //0x0004
        int32_t m_field8; //0x0008
        int32_t m_fieldC; //0x000C
        ZGeomBuffer* m_geomBuffer; //0x0010
        uint32_t m_pf4runtime__data; //0x0014
        AnimationManager* m_animUnknown; //0x0018
        uint32_t m_ppPF4Runtime__Data; //0x001C
        int32_t m_field20; //0x0020
        int32_t m_field24; //0x0024
        void* m_ANMBufferPtr; //0x0028
        int32_t m_ANMBufferSize; //0x002C
        int32_t m_field30; //0x0030
        /// ---{ LINKED LIST GOD RAY BEGIN }---

        ///TODO: Use ZLinkedList<T> for this we need to implement ZEventBase, CWinEvent<ZWINDOW>, ZLIST, ZGEOM, ZCAMERA, CBaseEvent<ZGEOM> and ZPostFilterBoxEvent

        //uint32_t m_ZEventBase_LinkedList_EndPtr; //0x0034
        //uint32_t m_ZEventBase_LinkedList_BeginPtr; //0x0038
        ZLinkedListHeader<ZEventBase> m_eventBase_LinkedList; //0x0034

        //uint32_t m_CWinEvent_LinkedList_EndPtr; //0x003C
        //uint32_t m_CWinEvent_LinkedList_BeginPtr; //0x0040
        ZLinkedListHeader<ZEventBase> m_cWinEvent_LinkedList; //0x003C

        //uint32_t m_UnknownClass0_LinkedListPtr_EndPtr_BP; //0x0044
        //uint32_t m_UnknownClass0_LinkedListPtr_BeginPtr_BP; //0x0048
        ZLinkedListHeader<void> m_unknownClass0_LinkedList; //0x0044

        //uint32_t m_ZLIST_LinkedList_EndPtr; //0x004C
        //uint32_t m_ZLIST_LinkedList_BeginPtr; //0x0050
        ZLinkedListHeader<ZLIST> m_zList_LinkedList; //0x004C

        //uint32_t m_ZGEOM_LinkedList_EndPtr; //0x0054
        //uint32_t m_ZGEOM_LinkedList_BeginPtr; //0x0058
        ZLinkedListHeader<ZGEOM> m_zGeom_LinkedList; //0x0054

        //uint32_t m_UnknownClass1_LinkedListPtr_EndPtr_BP; //0x005C
        //uint32_t m_UnknownClass1_LinkedListPtr_BeginPtr_BP; //0x0060
        ZLinkedListHeader<void> m_unknownClass1_LinkedList; //0x005C

        //uint32_t m_ZCAMERA_LinkedList_EndPtr; //0x0064
        //uint32_t m_ZCAMERA_LinkedList_BeginPtr; //0x0068
        ZLinkedListHeader<ZCAMERA> m_zCamera_LinkedList; //0x0064

        //uint32_t m_ZGEOMPostFilterEvent_LinkedList_EndPtr; //0x006C
        //uint32_t m_ZGEOMPostFilterEvent_LinkedList_BeginPtr; //0x0070
        ZLinkedListHeader<ZGEOM> m_zGeomPostFilterEvent_LinkedList; //0x006C

        //uint32_t m_ZCAMERAPostFilterEvent_LinkedList_EndPtr; //0x0074
        //uint32_t m_ZCAMERAPostFilterEvent_LinkedList_BeginPtr; //0x0078
        ZLinkedListHeader<ZCAMERA> m_zCameraPostFilterEvent_LinkedList; //0x0074

        //uint32_t ZGEOM2_LinkedList_EndPtr; //0x007C
        //uint32_t ZGEOM2_LinkedList_BeginPtr; //0x0080
        ZLinkedListHeader<ZGEOM> m_zGeom2_LinkedList; //0x0074

        //uint32_t m_unk_LinkedList_EndPtr; //0x0084
        //uint32_t m_unk_LinkedList_BeginPtr; //0x0088
        ZLinkedListHeader<void> m_unknownClass2_LinkedList; //0x0084

        /// ---{ LINKED LIST GOD RAY END }---
        ZScheduledUpdate* m_pEventScheduler; //0x008C
        ZROOM* m_root; //0x0090
        char pad_0094[4]; //0x0094
        ResourceCollection* m_resourceCollection; //0x0098
        char pad_009C[56]; //0x009C
        char* m_gms; //0x00D4

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
    };
}