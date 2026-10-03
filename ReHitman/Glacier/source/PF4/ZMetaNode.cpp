#include <Glacier/PF4/ZMetaNode.h>


namespace Glacier::PF4
{
    ZMetaNode::ZMetaNode() = default;

    ZMetaNode& ZMetaNode::operator=(const ZMetaNode& other)
    {
        m_Data = other.m_Data;
        m_Type = other.m_Type;
        m_pBoid = other.m_pBoid;
        m_Next = other.m_Next;
        m_Prev = other.m_Prev;
        m_Location = other.m_Location;
        return *this;
    }

    const ZVector3& ZMetaNode::GetPosition() const
    {
        return m_Location.m_vPos;
    }

    const ZLocation& ZMetaNode::GetLocation() const
    {
        return m_Location;
    }

    bool ZMetaNode::IsRegistered() const
    {
        return m_Data != nullptr;
    }
}
