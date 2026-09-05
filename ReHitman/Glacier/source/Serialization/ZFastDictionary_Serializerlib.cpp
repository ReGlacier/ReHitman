#include <Glacier/Serializer/ZFastDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZTokenTable_Serializerlib.h>
#include <Glacier/Serializer/ZPackedDictionary.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZInputStreamBase.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZFastDictionary_Serializerlib::~ZFastDictionary_Serializerlib() = default;
    ZFastDictionary_Serializerlib::ZFastDictionary_Serializerlib() = default;

    void ZFastDictionary_Serializerlib::SaveAsPackedDictionary(ZOutputStreamBase& pStream)
    {
        ZPackedDictionary_Serializerlib serializer { *reinterpret_cast<ZFastDictionary*>(this) };
        serializer.Save(pStream); 
    }

    void ZFastDictionary_Serializerlib::SaveAsTokenTable(ZOutputStreamBase& pStream)
    {
        ZTokenTable_Serializerlib serializer { *reinterpret_cast<ZFastDictionary*>(this) };
        serializer.Save(pStream);
    }
}