#include <Glacier/Serializer/ZSerializerVisitor.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        void AssertInsideSkippedObject(uint32_t lDepth)
        {
            ZASSERT(lDepth != 0);
        }
    }

    ZSerializerVisitor_Skip::ZSerializerVisitor_Skip(uint32_t lDepth) : m_Depth(lDepth)
    {
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, const unsigned int)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, double*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, float*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, int32_t*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, uint32_t*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, int16_t*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, uint16_t*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, int8_t*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, uint8_t*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, bool*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Data(const ZToken, const char*)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Bitfield(const ZToken, const char *(*)[32])
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Reference(const ZToken, uint32_t)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::Container(const ZToken, const uint32_t)
    {
        AssertInsideSkippedObject(m_Depth);
    }

    void ZSerializerVisitor_Skip::BeginObject(const ZToken)
    {
        ++m_Depth;
    }

    void ZSerializerVisitor_Skip::EndObject(const ZToken)
    {
        --m_Depth;
    }

    void ZSerializerVisitor_Skip::End()
    {
        ZASSERT(m_Depth == 0);
    }

    ZSerializerVisitor_SkipObject::operator bool() const
    {
        return m_Depth != 0;
    }

    void ZSerializerVisitor_SkipToNextMark::Skip()
    {
        m_SkipFound = true;
    }

    void ZSerializerVisitor_SkipToNextMark::EndObject(const ZToken token)
    {
        ZSerializerVisitor_Skip::EndObject(token);
        ZASSERT(m_Depth != 0);
    }

    ZSerializerVisitor_SkipToNextMark::ZSerializerVisitor_SkipToNextMark()
        : ZSerializerVisitor_Skip(1u),
          m_SkipFound(false)
    {
    }

    ZSerializerVisitor_SkipToNextMark::operator bool() const
    {
        return !m_SkipFound;
    }
}
