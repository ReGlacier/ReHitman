#pragma once

#include <cstdint>


namespace Glacier
{
    //fwds
    class LINKREFTAB;
    class ZROOM;
    class REFTAB;
    struct ZGEOMCLASSINFO;
    struct CCom;
    struct CHUNKFILE;
    class ZGeomBuffer;
    struct ZEventList;

    struct ZScene
    {
        // types
        enum EToDo : int32_t
        {
            TODO_Nothing = 0,
            TODO_Unload = 1,
            TODO_Load = 2,
            TODO_UnloadAndLoad = 3
        };

        // methods
        ZScene();
        const char* GetSceneName() const;
        void Load(const char* scene_name);
        void LoadDoneNotify(uint8_t* word_table);
        void Unload();
        void UnloadDoneNotify();
        EToDo GetToDo() const;

        // members
        bool m_Changing{false};
        bool m_Loaded{false};
        char m_SceneName[260]{'\0'};
        LINKREFTAB* _pBigFiles{nullptr};
        ZROOM* _pRoot{nullptr};
        uint32_t _rEnvionmentRef{0u};
        REFTAB* _pDisplayRouts{nullptr};
        ZGEOMCLASSINFO* _pClassFirst{nullptr};
        ZGEOMCLASSINFO* _pClassLast{nullptr};
        CCom* _pSceneCom{nullptr};
        float _FrameTime{0.f};
        float _PreFrameTime{0.f};
        float _ActTime{0.f};
        struct ZInputActions* _pInputActions{nullptr};
        struct MMCHK* _pAllGroupsMMChk{nullptr};
        uint8_t* _pStaticBuffer{nullptr};
        int _lStaticBufferLength{0};
        CHUNKFILE* _pPackedAnims{nullptr};
        int _lPackedAnimsLength{0};
        ZGeomBuffer* _pGeomBuffer{nullptr};
        int _lLockMinMax{0};
        uint8_t* _pPackedTreeData{nullptr};
        ZEventList* _pFrameUpdateList{nullptr};
        uint8_t* m_WordTable{nullptr};
    };
}