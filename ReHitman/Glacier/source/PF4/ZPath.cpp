#include <Glacier/PF4/ZPath.h>

#include <Glacier/PF4/ZInterface.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier::PF4
{
    // Appends a "custom vertex" path entry (type 3) that references m_Vertices
    // and returns the encoded ZDataRef for it (PC 004D8FF0).
    ZDataRef ZPath::AddVertex(const float* pvPos)
    {
        ZASSERT(m_CustomVertices < 4);
        const int iVertexIndex = m_CustomVertices;

        ZDataRef ref{};
        ref.m_Type = 3;
        ref.m_Id = static_cast<unsigned int>(iVertexIndex);
        ref.m_Pos.x = pvPos[0];
        ref.m_Pos.y = pvPos[2];

        m_Vertices[iVertexIndex] = ZVector3(pvPos[0], pvPos[1], pvPos[2]);
        m_pathIdx[m_Size] = ref;
        ++m_CustomVertices;
        ++m_Size;
        if (m_Size >= 2)
        {
            ZVector3 vLast;
            ZVector3 vPrev;
            GetPosition(m_Size - 1, vLast);
            GetPosition(m_Size - 2, vPrev);
            m_Cost += (vLast - vPrev).Length();
        }

        return ref;
    }

    // Appends an already encoded ZDataRef to the path and extends the cost (PC 004D8F70).
    void ZPath::AddRef(ZDataRef rRef)
    {
        if (m_Size == 199)
        {
            return;
        }

        m_pathIdx[m_Size] = rRef;
        ++m_Size;
        if (m_Size >= 2)
        {
            ZVector3 vLast;
            ZVector3 vPrev;
            GetPosition(m_Size - 1, vLast);
            GetPosition(m_Size - 2, vPrev);
            m_Cost += (vLast - vPrev).Length();
        }
    }

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

    // ZPath (PC 004D8CC0)
    ZPath::ZPath(ZDataRef* pDataRef, int iMaxLen)
    {
        m_pathIdx = pDataRef;
        m_iMaxSize = iMaxLen;
        m_Cost = 0.0f;
        m_Size = 0;
        m_PathFinder = nullptr;
        m_CustomVertices = 0;
    }

    // ZPath (PC 004D8CE0)
    ZPath::ZPath()
    {
        m_pathIdx = nullptr;
        m_iMaxSize = 0;
        m_Cost = 0.0f;
        m_Size = 0;
        m_PathFinder = nullptr;
        m_CustomVertices = 0;
    }

    // ZPath::Clear (PC 004D8D00)
    void ZPath::Clear()
    {
        m_Size = 0;
        m_Cost = 0.0f;
        m_CustomVertices = 0;
    }

    // ZPath::GetAction (PC 004D8D40): the path-way action of the entry at iPos,
    // or 0 for entries that are not traversed (e.g. custom vertices).
    int ZPath::GetAction(int iPos) const
    {
        ZASSERT(iPos >= 0 && iPos < m_Size);

        const ZDataRef& pathEntry = m_pathIdx[iPos];
        if (pathEntry.m_Type == 3)
        {
            return 0;
        }

        ZVector3 vPosition;
        ZVector3 vNormal;
        EPathWayActions action = PWA_DEFAULT;
        unsigned int lData = 0;
        m_PathFinder->MapNodeIdx(pathEntry, &vPosition.x, &vNormal.x, action, lData);
        return static_cast<int>(action);
    }

    // ZPath::GetEndPosition (PC 004D8ED0)
    void ZPath::GetEndPosition(ZVector3& vPosition) const
    {
        vPosition.x = 0.0f;
        vPosition.y = 0.0f;
        vPosition.z = 0.0f;
        if (m_Size > 0)
        {
            GetPosition(m_Size - 1, vPosition);
        }
    }

    // ZPath::CutAtDistance (PC 004D90E0): cuts the path at the requested travelled
    // distance. The cutting point is stored as a custom vertex in m_Vertices[3].
    bool ZPath::CutAtDistance(float fDistance)
    {
        if (fDistance < 1.0f)
        {
            m_Cost = 0.0f;
            m_Size = 1;
            return false;
        }

        if (m_Size < 2)
        {
            return false;
        }

        if (fDistance < 0.0f)
        {
            m_pathIdx[1] = m_pathIdx[0];
            m_Size = 2;
            m_Cost = 0.0f;
            return true;
        }

        if (m_Cost < fDistance)
        {
            return false;
        }

        int iPos = 0;
        for (;;)
        {
            ++iPos;
            ZASSERT(iPos < m_Size);
            if (GetDistanceFromStart(iPos) >= fDistance)
            {
                break;
            }
        }

        if (GetAction(iPos) != PWA_DEFAULT)
        {
            // Stop in front of the gate/action instead of crossing it.
            m_Cost = GetDistanceFromStart(iPos - 1);
            m_Size = iPos;
            return true;
        }

        // Split the plain segment that contains the requested distance.
        ZVector3 vEnd;
        ZVector3 vStart;
        GetPosition(iPos, vEnd);
        GetPosition(iPos - 1, vStart);

        const float fRemaining = fDistance - GetDistanceFromStart(iPos - 1);
        const float fSegLength = GetDistanceFromStart(iPos) - GetDistanceFromStart(iPos - 1);
        const float fT = fRemaining / fSegLength;
        ZASSERT(fT >= 0.0f && fT <= 1.0f);

        ZVector3 vDelta;
        vsub(&vDelta.x, &vEnd.x, &vStart.x);
        vaddscalar(&m_Vertices[3].x, &vEnd.x, &vDelta.x, fT - 1.0f);
        m_pathIdx[iPos].SetPosition(3, m_Vertices[3].x, m_Vertices[3].z);

        m_Size = iPos + 1;
        m_Cost = fDistance;
        return true;
    }
}
