#pragma once

#include <BloodMoney/Editors/LevelEditorGeomsPool.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::size_t kCacheCapacity = 12'000;
    }

    LevelEditorGeomsPool& LevelEditorGeomsPool::GetInstance()
    {
        static LevelEditorGeomsPool* instance = nullptr;
        if (!instance)
        {
            instance = new LevelEditorGeomsPool();
        }
        instance->m_cache.reserve(Consts::kCacheCapacity);
        return *instance;
    }

    void LevelEditorGeomsPool::AddGeom(Glacier::ZGEOM* geom)
    {
        if (!geom)
        {
            spdlog::warn("LevelEditorGeomsPool::AddGeom| Somebody trying to add NULL GEOM!");
            return;
        }
        m_geoms.insert(geom);
        m_shouldRebuildCache = true;
    }

    void LevelEditorGeomsPool::RemoveGeom(Glacier::ZGEOM* geom)
    {
        if (!geom)
        {
            spdlog::warn("LevelEditorGeomsPool::AddGeom| Somebody trying to remove NULL GEOM!");
            return;
        }

        m_geoms.erase(geom);
        m_shouldRebuildCache = true;
    }

    void LevelEditorGeomsPool::ForEach(std::function<void(Glacier::ZGEOM*)> pred)
    {
        for (const auto& geom : m_geoms)
            pred(geom);
    }

    size_t LevelEditorGeomsPool::GetCount() const
    {
        return m_geoms.size();
    }

    const std::vector<Glacier::ZGEOM*>& LevelEditorGeomsPool::AsVector()
    {
        if (m_shouldRebuildCache)
        {
            m_shouldRebuildCache = false;
            m_cache.clear();

            for (const auto& geom : m_geoms)
            {
                m_cache.push_back(geom);
            }
        }

        return m_cache;
    }
}