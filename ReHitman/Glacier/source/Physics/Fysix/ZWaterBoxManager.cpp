#include <Glacier/Physics/Fysix/ZWaterBoxManager.h>
#include <Glacier/Physics/Fysix/ZWaterBox.h>


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

    uint32_t ZWaterBoxManager::Count() const
    {
        return m_WaterBoxList.Count();
    }

    bool ZWaterBoxManager::Exists(const ZWaterBox* pWaterBox) const
    {
        return false;
    }

    bool ZWaterBoxManager::Exists(ZREF rWaterBox) const
    {
        return Find(rWaterBox) != nullptr;
    }

    ZWaterBox* ZWaterBoxManager::Find(ZGEOM* pGeom) const
    {
        const ZREF rTarget = pGeom->GetRef();

        // IOI WTF? Junior style coding in 2k04?
        for (auto rGeom : m_WaterBoxList.As<ZREF>())
        {
            if (rGeom == rTarget)
            {
                return ref_cast<ZWaterBox>(rGeom);
            }
        }

        return nullptr;
    }

    ZWaterBox* ZWaterBoxManager::Find(uint32_t lIndex) const
    {
        if (lIndex >= m_WaterBoxList.Count())
            return nullptr;

        auto rWaterBox = m_WaterBoxList[lIndex];
        return ref_cast<ZWaterBox>(rWaterBox);
    }

    bool ZWaterBoxManager::Add(ZWaterBox* pWaterBox)
    {
        if (!pWaterBox)
            return false;

        return Add(pWaterBox->GetRef());
    }

    bool ZWaterBoxManager::Add(ZREF rWaterBox)
    {
        if (rWaterBox && !m_WaterBoxList.Exists(rWaterBox))
        {
            m_WaterBoxList.Add(rWaterBox);
            return true;
        }

        return false;
    }

    void ZWaterBoxManager::Remove(const ZWaterBox* pWaterBox)
    {
        if (!pWaterBox)
            return;

        Remove(pWaterBox->GetRef());
    }

    void ZWaterBoxManager::Remove(ZREF rWaterBox)
    {
        if (rWaterBox)
        {
            m_WaterBoxList.Remove(rWaterBox);
        }
    }

    ZWaterBox* ZWaterBoxManager::operator[](uint32_t lIndex)
    {
        if (lIndex >= m_WaterBoxList.Count())
            return nullptr;

        return ref_cast<ZWaterBox>(m_WaterBoxList[lIndex]);
    }

    const ZWaterBox* ZWaterBoxManager::operator[](uint32_t lIndex) const
    {
        if (lIndex >= m_WaterBoxList.Count())
            return nullptr;

        return ref_cast<ZWaterBox>(m_WaterBoxList[lIndex]);
    }

    ZWaterBox* ZWaterBoxManager::operator[](ZGEOM* pGeom)
    {
        return Find(pGeom);
    }

    const ZWaterBox* ZWaterBoxManager::operator[](ZGEOM* pGeom) const
    {
        return Find(pGeom);
    }

    template <>
    ZWaterBoxManager* ZComponentSingleton<ZWaterBoxManager, ZRuntimeComponentBase>::m_pInstance = nullptr;
}
