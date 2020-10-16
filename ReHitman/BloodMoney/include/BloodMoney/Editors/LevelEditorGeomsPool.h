#pragma once

#include <Glacier/GlacierFWD.h>

#include <functional>
#include <vector>
#include <set>

namespace Hitman::BloodMoney
{
    class LevelEditorGeomsPool final
    {
    private:
        std::set<Glacier::ZGEOM*> m_geoms;
        bool m_shouldRebuildCache { true };
        std::vector<Glacier::ZGEOM*> m_cache;

    public:
        static LevelEditorGeomsPool& GetInstance();

        void AddGeom(Glacier::ZGEOM* geom);
        void RemoveGeom(Glacier::ZGEOM* geom);
        void ForEach(std::function<void(Glacier::ZGEOM*)> pred);
        size_t GetCount() const;
        const std::vector<Glacier::ZGEOM*>& AsVector();
    };
}