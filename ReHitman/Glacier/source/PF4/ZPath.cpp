#include <Glacier/PF4/ZPath.h>

#include <Glacier/PF4/ZInterface.h>


namespace Glacier::PF4
{
    void ZPath::GetPosition(int iPathEntryIndex, ZVector3& vPosition) const
    {
        ZASSERT(iPathEntryIndex >= 0 && iPathEntryIndex < m_Size);

        const ZDataRef& pathEntry = m_pathIdx[iPathEntryIndex];
        if (pathEntry.m_Type == 3)
        {
            vPosition = m_Vertices[pathEntry.m_Id];
        }
        else
        {
            ZVector3 vNormal;
            EPathWayActions action;
            unsigned int lData;
            m_PathFinder->MapNodeIdx(pathEntry, vPosition, vNormal, action, lData);
        }

        vPosition.x = pathEntry.m_Pos.x;
        vPosition.z = pathEntry.m_Pos.y;
    }

    bool ZPath::GetNormal(int iPathEntryIndex, ZVector3& vNormal) const
    {
        ZASSERT(iPathEntryIndex >= 0 && iPathEntryIndex < m_Size);

        const ZDataRef& pathEntry = m_pathIdx[iPathEntryIndex];
        if (pathEntry.m_Type != 1)
            return false;

        ZVector3 vPosition;
        EPathWayActions action;
        unsigned int lData;
        m_PathFinder->MapNodeIdx(pathEntry, vPosition, vNormal, action, lData);
        return true;
    }

    float ZPath::GetDistanceFromStart(int iPathEntryIndex) const
    {
        ZASSERT(iPathEntryIndex >= 0 && iPathEntryIndex < m_Size);

        float fDistance = 0.0f;
        for (int i = 0; i < iPathEntryIndex; ++i)
        {
            ZVector3 vFrom;
            ZVector3 vTo;
            GetPosition(i, vFrom);
            GetPosition(i + 1, vTo);
            fDistance += (vTo - vFrom).Length();
        }

        return fDistance;
    }
}
