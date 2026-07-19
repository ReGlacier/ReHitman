#include <Glacier/RTP/Base.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier::RTP
{
    namespace 
    {
        inline cProperty* AsProperty(cNode* node)
        {
            return reinterpret_cast<cProperty*>(node);
        }
    }

    void LoadSerializable(ZSerializableBase* serializable, const ZPropertyInfo* properties, ISerializerStream& stream)
    {
        if (properties->Super)
        {
            LoadSerializable(serializable, properties->Super, stream);
        }

        for (auto* property = properties->First; property; property = property->m_Next)
        {
            if (stream.TestStreamFilter(ISerializerStream::EContent::CONTENT_SavedGame))
            {
                AsProperty(property)->m_VirtualTable->Load(property, &stream, serializable);
            }
        }
    }

    void SaveSerializable(ZSerializableBase* serializable, const ZPropertyInfo* properties, ISerializerStream& stream)
    {
        if (properties->Super)
        {
            SaveSerializable(serializable, properties->Super, stream);
        }

        for (auto* property = properties->First; property; property = property->m_Next)
        {
            if (stream.TestStreamFilter(ISerializerStream::EContent::CONTENT_SavedGame))
            {
                AsProperty(property)->m_VirtualTable->Save(property, &stream, serializable);
            }
        }
    }
}