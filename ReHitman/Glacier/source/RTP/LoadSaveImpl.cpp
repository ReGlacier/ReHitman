#include <Glacier/RTP/Base.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZSTL/ZRTStringObject.h>


namespace Glacier::RTP
{
    namespace 
    {
        inline cProperty* AsProperty(cNode* node)
        {
            return reinterpret_cast<cProperty*>(node);
        }
        
        inline bool IsAllowedProperty(const RTP::cNode* node, const ISerializerStream& stream)
        {
            // NOTE: PC version is a little bit different of PS2.
            //       Not all properties are accepted by default, we need to check m_Filter before apply property from stream
            return ((1u << stream.m_Content) & node->m_Filter) != 0;
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
            if (IsAllowedProperty(property, stream))
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
            if (IsAllowedProperty(property, stream))
            {
                AsProperty(property)->m_VirtualTable->Save(property, &stream, serializable);
            }
        }
    }

    void LoadConstString(ISerializerStream& stream, const char* name, ZRTString& value)
    {
        const char* stringValue = value;
        stream.Exchange(name, stringValue);
        value = stringValue;
    }

    void SaveConstString(ISerializerStream& stream, const char* name, ZRTString& value)
    {
        const char* stringValue = value;
        stream.Exchange(name, stringValue);
    }
}
