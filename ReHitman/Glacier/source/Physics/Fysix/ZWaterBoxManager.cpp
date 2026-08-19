#include <Glacier/Physics/Fysix/ZWaterBoxManager.h>


namespace Glacier
{
    ZWaterBoxManager::ZWaterBoxManager()
        : m_WaterBoxList()
    {
        // Do nothing
    }

    ZWaterBoxManager::~ZWaterBoxManager()
    {
        // Do nothing
    }

    void ZWaterBoxManager::NukeAndRestart()
    {
        m_WaterBoxList.Clear();
    }

    template <>
    ZWaterBoxManager* ZComponentSingleton<ZWaterBoxManager, ZRuntimeComponentBase>::m_pInstance = nullptr;
}