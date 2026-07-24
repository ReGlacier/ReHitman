#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/EventBase/ZEventList.h>
#include <Glacier/CCom.h>

#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZScene::ZScene() = default;

    const char* ZScene::GetSceneName() const
    {
        ZASSERT(m_SceneName[0]);
        return &m_SceneName[0];
    }

    void ZScene::Load(const char* scene_name)
    {
        m_Changing = true;

        constexpr size_t max_capacity = sizeof(m_SceneName) - 1; 
        size_t i = 0;

        for (; scene_name[i] != '\0' && i < max_capacity; ++i)
        {
            m_SceneName[i] = (scene_name[i] == '/') ? '\\' : scene_name[i];
        }

        m_SceneName[i] = '\0';
    }

    void ZScene::LoadDoneNotify(uint8_t* word_table)
    {
        m_WordTable = word_table;
        m_Changing = true;
    }

    void ZScene::Unload()
    {
        m_SceneName[0] = '\0'; // maybe memset?
        m_Changing = m_Loaded;
    }

    void ZScene::UnloadDoneNotify()
    {
        m_Changing = false;
        m_Loaded = false;

        ZUniMemory::Free(m_WordTable);
        m_WordTable = nullptr;
    }
    
    ZScene::EToDo ZScene::GetToDo() const
    {
        if (m_Changing)
        {
            if (m_Loaded)
            {
                if (m_SceneName[0])
                {
                    return ZScene::EToDo::TODO_UnloadAndLoad;
                }
                else
                {
                    return ZScene::EToDo::TODO_Unload;
                }
            }
            else
            {
                ZASSERT(m_SceneName[0]);
                return ZScene::EToDo::TODO_Load;
            }
        }
        else
        {
            return ZScene::EToDo::TODO_Nothing;
        }
    }
}