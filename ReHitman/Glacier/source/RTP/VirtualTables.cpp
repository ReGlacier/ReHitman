#define GLACIER_RTP_VIRTUALTABLES_NO_XEXE_ALIASES
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <type_traits>


namespace Glacier::RTP
{
    namespace
    {
        template <typename T>
        struct ArrayElementCount
        {
            static constexpr uint32_t Value = 1;
        };

        template <typename T, size_t N>
        struct ArrayElementCount<T[N]>
        {
            static constexpr uint32_t Value = static_cast<uint32_t>(N) * ArrayElementCount<T>::Value;
        };

        template <typename TPMF>
        auto MethodPointerAddress(TPMF method)
        {
            uintptr_t address = 0;
            static_assert(sizeof(address) <= sizeof(method));
            memcpy(&address, &method, sizeof(address));
            return address;
        }

        template <typename T>
        ZSerializableBase* ApplyDelta(ZSerializableBase& object, int delta)
        {
            return reinterpret_cast<ZSerializableBase*>(reinterpret_cast<uint8_t*>(&object) + delta);
        }

        template <typename T>
        T* PropertyAddress(ZDataProperty<T>* pProperty, ZSerializableBase& object)
        {
            const auto offset = reinterpret_cast<uintptr_t>(pProperty->m_Offset);
            return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(&object) + offset);
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZRTString& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, ZGEOMREF& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, ZCOLOR& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, ZANIM& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, ZFILENAME& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, ZRawData& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, TIMETYPE& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, REFTAB& value);
        void ExchangeValue(ISerializerStream& stream, const char* name, REFTAB32& value);

        template <typename T>
        void ExchangeValue(ISerializerStream& stream, const char* name, ZBitfield<T>& value);

        template <typename T>
        void ExchangeValue(ISerializerStream& stream, const char* name, T& value)
        {
            if constexpr (std::is_array_v<T>)
            {
                using Element = std::remove_all_extents_t<T>;
                if constexpr (std::is_same_v<Element, ZGEOMREF>)
                {
                    for (uint32_t i = 0; i < ArrayElementCount<T>::Value; ++i)
                    {
                        ExchangeValue(stream, name, reinterpret_cast<ZGEOMREF*>(&value)[i]);
                    }
                }
                else
                {
                    stream.ExchangeArray(name, reinterpret_cast<Element*>(&value), ArrayElementCount<T>::Value);
                }
            }
            else
            {
                stream.Exchange(name, value);
            }
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZRTString& value)
        {
            if (stream.IsSaving())
            {
                SaveConstString(stream, name, value);
            }
            else
            {
                LoadConstString(stream, name, value);
            }
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZGEOMREF& value)
        {
            if (stream.TestStreamFilter(1u << ISerializerStream::CONTENT_SavedGame))
            {
                stream.Exchange(name, value.m_Value);
                return;
            }

            if (stream.IsSaving())
            {
                ZASSERT(false);
                return;
            }

            const char* refName = nullptr;
            stream.Exchange(name, refName);
            value.m_Value = g_pEngineData->GetREFByName(refName);
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZCOLOR& value)
        {
            stream.Exchange(name, value.m_Value);
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZANIM& value)
        {
            ExchangeValue(stream, name, static_cast<ZRTString&>(value));
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZFILENAME& value)
        {
            ExchangeValue(stream, name, static_cast<ZRTString&>(value));
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, ZRawData& value)
        {
            if (stream.IsSaving())
            {
                ZASSERT(false);
                return;
            }

            uint32_t size = 0;
            stream.ExchangeContainer(name, size);
            value.SetSize(size);

            if (size)
            {
                stream.ExchangeRaw(ZToken::Void, value.m_Data, size);
            }
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, TIMETYPE& value)
        {
            stream.Exchange(name, value.secs);
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, REFTAB& value)
        {
            stream.ExchangeArray(stream.GetToken(name), &value, 1);
        }

        void ExchangeValue(ISerializerStream& stream, const char* name, REFTAB32& value)
        {
            stream.ExchangeArray(name, &value, 1);
        }

        template <typename T>
        void ExchangeValue(ISerializerStream& stream, const char* name, ZBitfield<T>& value)
        {
            stream.Exchange(name, value.m_Bitfield);
        }

        template <typename T>
        tVirtualTable<ZDataProperty<T>> MakeDataTable()
        {
            return {
                .Load = LoadDataProperty<T>,
                .Save = SaveDataProperty<T>
            };
        }

        template <typename T>
        tVirtualTable<ZVirtualProperty<T>> MakeVirtualTable()
        {
            return {
                .Load = LoadVirtualProperty<T>,
                .Save = SaveVirtualProperty<T>
            };
        }

        template <typename T>
        tVirtualTable<ZVirtualEnumProperty<T>> MakeVirtualEnumTable()
        {
            return {
                .Load = LoadVirtualEnumProperty<T>,
                .Save = SaveVirtualEnumProperty<T>
            };
        }
    }

    template <typename T>
    void LoadDataProperty(ZDataProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        ExchangeValue(stream, pProperty->m_Node.m_Name, *PropertyAddress(pProperty, object));
    }

    template <typename T>
    void SaveDataProperty(ZDataProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        ExchangeValue(stream, pProperty->m_Node.m_Name, *PropertyAddress(pProperty, object));
    }

    template <typename T>
    void LoadVirtualProperty(ZVirtualProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        T value{};
        ExchangeValue(stream, pProperty->m_Node.m_Name, value);

        using Setter = void(__thiscall*)(void*, const T&);
        auto* pThis = ApplyDelta<T>(object, pProperty->m_Set.__delta);
        auto setter = reinterpret_cast<Setter>(MethodPointerAddress(pProperty->m_Set.__pfn));
        setter(pThis, value);
    }

    template <typename T>
    void SaveVirtualProperty(ZVirtualProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        T value{};

        using Getter = void(__thiscall*)(void*, T&);
        auto* pThis = ApplyDelta<T>(object, pProperty->m_Get.__delta);
        auto getter = reinterpret_cast<Getter>(MethodPointerAddress(pProperty->m_Get.__pfn));
        getter(pThis, value);

        ExchangeValue(stream, pProperty->m_Node.m_Name, value);
    }

    void LoadEnumProperty(ZEnumProperty* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        auto* value = reinterpret_cast<uint8_t*>(&object) + reinterpret_cast<uintptr_t>(pProperty->m_Offset);
        stream.ExchangeEnum(pProperty->m_Node.m_Name, value, 1, *pProperty->m_Info);
    }

    void SaveEnumProperty(ZEnumProperty* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        auto* value = reinterpret_cast<uint8_t*>(&object) + reinterpret_cast<uintptr_t>(pProperty->m_Offset);
        stream.ExchangeEnum(pProperty->m_Node.m_Name, value, 1, *pProperty->m_Info);
    }

    template <typename T>
    void LoadVirtualEnumProperty(ZVirtualEnumProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        T value{};
        stream.ExchangeEnum(pProperty->m_Node.m_Name, &value, 1, *pProperty->m_Info);

        using Setter = void(__thiscall*)(void*, const T&);
        auto* pThis = ApplyDelta<T>(object, pProperty->m_Set.__delta);
        auto setter = reinterpret_cast<Setter>(MethodPointerAddress(pProperty->m_Set.__pfn));
        setter(pThis, value);
    }

    template <typename T>
    void SaveVirtualEnumProperty(ZVirtualEnumProperty<T>* pProperty, ISerializerStream& stream, ZSerializableBase& object)
    {
        T value{};

        using Getter = void(__thiscall*)(void*, T&);
        auto* pThis = ApplyDelta<T>(object, pProperty->m_Get.__delta);
        auto getter = reinterpret_cast<Getter>(MethodPointerAddress(pProperty->m_Get.__pfn));
        getter(pThis, value);

        stream.ExchangeEnum(pProperty->m_Node.m_Name, &value, 1, *pProperty->m_Info);
    }

    template void LoadDataProperty<bool>(ZDataProperty<bool>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<bool>(ZDataProperty<bool>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<char>(ZDataProperty<char>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<char>(ZDataProperty<char>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uchar>(ZDataProperty<uchar>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uchar>(ZDataProperty<uchar>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<short>(ZDataProperty<short>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<short>(ZDataProperty<short>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ushort>(ZDataProperty<ushort>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ushort>(ZDataProperty<ushort>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<int>(ZDataProperty<int>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<int>(ZDataProperty<int>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint>(ZDataProperty<uint>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint>(ZDataProperty<uint>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float>(ZDataProperty<float>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float>(ZDataProperty<float>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<TIMETYPE>(ZDataProperty<TIMETYPE>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<TIMETYPE>(ZDataProperty<TIMETYPE>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZRTString>(ZDataProperty<ZRTString>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZRTString>(ZDataProperty<ZRTString>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZGEOMREF>(ZDataProperty<ZGEOMREF>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZGEOMREF>(ZDataProperty<ZGEOMREF>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZANIM>(ZDataProperty<ZANIM>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZANIM>(ZDataProperty<ZANIM>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZCOLOR>(ZDataProperty<ZCOLOR>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZCOLOR>(ZDataProperty<ZCOLOR>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZFILENAME>(ZDataProperty<ZFILENAME>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZFILENAME>(ZDataProperty<ZFILENAME>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZRawData>(ZDataProperty<ZRawData>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZRawData>(ZDataProperty<ZRawData>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<REFTAB>(ZDataProperty<REFTAB>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<REFTAB>(ZDataProperty<REFTAB>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<REFTAB32>(ZDataProperty<REFTAB32>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<REFTAB32>(ZDataProperty<REFTAB32>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZAUDIOREF>(ZDataProperty<ZAUDIOREF>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZAUDIOREF>(ZDataProperty<ZAUDIOREF>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZBitfield<ITEMSTATE>>(ZDataProperty<ZBitfield<ITEMSTATE>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZBitfield<ITEMSTATE>>(ZDataProperty<ZBitfield<ITEMSTATE>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZBitfield<WEAPONOPERATION>>(ZDataProperty<ZBitfield<WEAPONOPERATION>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZBitfield<WEAPONOPERATION>>(ZDataProperty<ZBitfield<WEAPONOPERATION>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZBitfield<ESecurityZone>>(ZDataProperty<ZBitfield<ESecurityZone>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZBitfield<ESecurityZone>>(ZDataProperty<ZBitfield<ESecurityZone>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template void LoadDataProperty<int[3]>(ZDataProperty<int[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<int[3]>(ZDataProperty<int[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<int[21]>(ZDataProperty<int[21]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<int[21]>(ZDataProperty<int[21]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint[2]>(ZDataProperty<uint[2]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint[2]>(ZDataProperty<uint[2]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint[4]>(ZDataProperty<uint[4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint[4]>(ZDataProperty<uint[4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint[15]>(ZDataProperty<uint[15]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint[15]>(ZDataProperty<uint[15]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint[32]>(ZDataProperty<uint[32]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint[32]>(ZDataProperty<uint[32]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint[64]>(ZDataProperty<uint[64]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint[64]>(ZDataProperty<uint[64]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<uint[128]>(ZDataProperty<uint[128]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<uint[128]>(ZDataProperty<uint[128]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[2]>(ZDataProperty<float[2]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[2]>(ZDataProperty<float[2]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[3]>(ZDataProperty<float[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[3]>(ZDataProperty<float[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[4]>(ZDataProperty<float[4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[4]>(ZDataProperty<float[4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[9]>(ZDataProperty<float[9]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[9]>(ZDataProperty<float[9]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[12]>(ZDataProperty<float[12]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[12]>(ZDataProperty<float[12]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[2][3]>(ZDataProperty<float[2][3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[2][3]>(ZDataProperty<float[2][3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<float[8][4]>(ZDataProperty<float[8][4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<float[8][4]>(ZDataProperty<float[8][4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZGEOMREF[6]>(ZDataProperty<ZGEOMREF[6]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZGEOMREF[6]>(ZDataProperty<ZGEOMREF[6]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadDataProperty<ZGEOMREF[64]>(ZDataProperty<ZGEOMREF[64]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveDataProperty<ZGEOMREF[64]>(ZDataProperty<ZGEOMREF[64]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    template void LoadVirtualProperty<bool>(ZVirtualProperty<bool>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<bool>(ZVirtualProperty<bool>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<int>(ZVirtualProperty<int>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<int>(ZVirtualProperty<int>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<uint>(ZVirtualProperty<uint>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<uint>(ZVirtualProperty<uint>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<float>(ZVirtualProperty<float>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<float>(ZVirtualProperty<float>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZRTString>(ZVirtualProperty<ZRTString>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZRTString>(ZVirtualProperty<ZRTString>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZGEOMREF>(ZVirtualProperty<ZGEOMREF>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZGEOMREF>(ZVirtualProperty<ZGEOMREF>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZANIM>(ZVirtualProperty<ZANIM>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZANIM>(ZVirtualProperty<ZANIM>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZCOLOR>(ZVirtualProperty<ZCOLOR>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZCOLOR>(ZVirtualProperty<ZCOLOR>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZFILENAME>(ZVirtualProperty<ZFILENAME>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZFILENAME>(ZVirtualProperty<ZFILENAME>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZRawData>(ZVirtualProperty<ZRawData>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZRawData>(ZVirtualProperty<ZRawData>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<REFTAB32>(ZVirtualProperty<REFTAB32>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<REFTAB32>(ZVirtualProperty<REFTAB32>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZBitfield<ITEMSTATE>>(ZVirtualProperty<ZBitfield<ITEMSTATE>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZBitfield<ITEMSTATE>>(ZVirtualProperty<ZBitfield<ITEMSTATE>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZBitfield<WEAPONOPERATION>>(ZVirtualProperty<ZBitfield<WEAPONOPERATION>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZBitfield<WEAPONOPERATION>>(ZVirtualProperty<ZBitfield<WEAPONOPERATION>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<ZBitfield<EAnimMode>>(ZVirtualProperty<ZBitfield<EAnimMode>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<ZBitfield<EAnimMode>>(ZVirtualProperty<ZBitfield<EAnimMode>>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<int[3]>(ZVirtualProperty<int[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<int[3]>(ZVirtualProperty<int[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<float[3]>(ZVirtualProperty<float[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<float[3]>(ZVirtualProperty<float[3]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<float[4]>(ZVirtualProperty<float[4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<float[4]>(ZVirtualProperty<float[4]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualProperty<float[9]>(ZVirtualProperty<float[9]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualProperty<float[9]>(ZVirtualProperty<float[9]>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void LoadVirtualEnumProperty<EBoundingBox>(ZVirtualEnumProperty<EBoundingBox>* pProperty, ISerializerStream& stream, ZSerializableBase& object);
    template void SaveVirtualEnumProperty<EBoundingBox>(ZVirtualEnumProperty<EBoundingBox>* pProperty, ISerializerStream& stream, ZSerializableBase& object);

    namespace VirtualTables
    {
        tVirtualTable<ZEnumProperty> Enum {
            .Load = LoadEnumProperty,
            .Save = SaveEnumProperty
        };

        tVirtualTable<ZDataProperty<bool>> Data_bool = MakeDataTable<bool>();
        tVirtualTable<ZDataProperty<char>> Data_char = MakeDataTable<char>();
        tVirtualTable<ZDataProperty<uchar>> Data_uchar = MakeDataTable<uchar>();
        tVirtualTable<ZDataProperty<short>> Data_short = MakeDataTable<short>();
        tVirtualTable<ZDataProperty<ushort>> Data_ushort = MakeDataTable<ushort>();
        tVirtualTable<ZDataProperty<int>> Data_int = MakeDataTable<int>();
        tVirtualTable<ZDataProperty<uint>> Data_uint = MakeDataTable<uint>();
        tVirtualTable<ZDataProperty<float>> Data_float = MakeDataTable<float>();
        tVirtualTable<ZDataProperty<TIMETYPE>> Data_TIMETYPE = MakeDataTable<TIMETYPE>();
        tVirtualTable<ZDataProperty<::Glacier::ZRTString>> Data_ZRTString = MakeDataTable<::Glacier::ZRTString>();
        tVirtualTable<ZDataProperty<ZGEOMREF>> Data_ZGEOMREF = MakeDataTable<ZGEOMREF>();
        tVirtualTable<ZDataProperty<ZANIM>> Data_ZANIM = MakeDataTable<ZANIM>();
        tVirtualTable<ZDataProperty<ZCOLOR>> Data_ZCOLOR = MakeDataTable<ZCOLOR>();
        tVirtualTable<ZDataProperty<ZFILENAME>> Data_ZFILENAME = MakeDataTable<ZFILENAME>();
        tVirtualTable<ZDataProperty<ZRawData>> Data_ZRawData = MakeDataTable<ZRawData>();
        tVirtualTable<ZDataProperty<REFTAB>> Data_REFTAB = MakeDataTable<REFTAB>();
        tVirtualTable<ZDataProperty<REFTAB32>> Data_REFTAB32 = MakeDataTable<REFTAB32>();
        tVirtualTable<ZDataProperty<ZAUDIOREF>> Data_ZAUDIOREF = MakeDataTable<ZAUDIOREF>();
        tVirtualTable<ZDataProperty<ZBitfield<ITEMSTATE>>> Data_ZBitfield_ITEMSTATE = MakeDataTable<ZBitfield<ITEMSTATE>>();
        tVirtualTable<ZDataProperty<ZBitfield<WEAPONOPERATION>>> Data_ZBitfield_WEAPONOPERATION = MakeDataTable<ZBitfield<WEAPONOPERATION>>();
        tVirtualTable<ZDataProperty<ZBitfield<ESecurityZone>>> Data_ZBitfield_ESecurityZone = MakeDataTable<ZBitfield<ESecurityZone>>();

        tVirtualTable<ZDataProperty<int[3]>> Data_int_3 = MakeDataTable<int[3]>();
        tVirtualTable<ZDataProperty<int[21]>> Data_int_21 = MakeDataTable<int[21]>();

        tVirtualTable<ZDataProperty<uint[2]>> Data_uint_2 = MakeDataTable<uint[2]>();
        tVirtualTable<ZDataProperty<uint[4]>> Data_uint_4 = MakeDataTable<uint[4]>();
        tVirtualTable<ZDataProperty<uint[15]>> Data_uint_15 = MakeDataTable<uint[15]>();
        tVirtualTable<ZDataProperty<uint[32]>> Data_uint_32 = MakeDataTable<uint[32]>();
        tVirtualTable<ZDataProperty<uint[64]>> Data_uint_64 = MakeDataTable<uint[64]>();
        tVirtualTable<ZDataProperty<uint[128]>> Data_uint_128 = MakeDataTable<uint[128]>();

        tVirtualTable<ZDataProperty<float[2]>> Data_float_2 = MakeDataTable<float[2]>();
        tVirtualTable<ZDataProperty<float[3]>> Data_float_3 = MakeDataTable<float[3]>();
        tVirtualTable<ZDataProperty<float[4]>> Data_float_4 = MakeDataTable<float[4]>();
        tVirtualTable<ZDataProperty<float[9]>> Data_float_9 = MakeDataTable<float[9]>();
        tVirtualTable<ZDataProperty<float[12]>> Data_float_12 = MakeDataTable<float[12]>();
        tVirtualTable<ZDataProperty<float[2][3]>> Data_float_2_3 = MakeDataTable<float[2][3]>();
        tVirtualTable<ZDataProperty<float[8][4]>> Data_float_8_4 = MakeDataTable<float[8][4]>();
        tVirtualTable<ZDataProperty<ZGEOMREF[6]>> Data_ZGEOMREF_6 = MakeDataTable<ZGEOMREF[6]>();
        tVirtualTable<ZDataProperty<ZGEOMREF[64]>> Data_ZGEOMREF_64 = MakeDataTable<ZGEOMREF[64]>();

        tVirtualTable<ZVirtualProperty<bool>> Virtual_bool = MakeVirtualTable<bool>();
        tVirtualTable<ZVirtualProperty<int>> Virtual_int = MakeVirtualTable<int>();
        tVirtualTable<ZVirtualProperty<uint>> Virtual_uint = MakeVirtualTable<uint>();
        tVirtualTable<ZVirtualProperty<float>> Virtual_float = MakeVirtualTable<float>();
        tVirtualTable<ZVirtualProperty<::Glacier::ZRTString>> ZRTString = MakeVirtualTable<::Glacier::ZRTString>();
        tVirtualTable<ZVirtualProperty<::Glacier::ZRTString>> Virtual_ZRTString = MakeVirtualTable<::Glacier::ZRTString>();
        tVirtualTable<ZVirtualProperty<ZGEOMREF>> Virtual_ZGEOMREF = MakeVirtualTable<ZGEOMREF>();
        tVirtualTable<ZVirtualProperty<ZANIM>> Virtual_ZANIM = MakeVirtualTable<ZANIM>();
        tVirtualTable<ZVirtualProperty<ZCOLOR>> Virtual_ZCOLOR = MakeVirtualTable<ZCOLOR>();
        tVirtualTable<ZVirtualProperty<ZFILENAME>> Virtual_ZFILENAME = MakeVirtualTable<ZFILENAME>();
        tVirtualTable<ZVirtualProperty<ZRawData>> Virtual_ZRawData = MakeVirtualTable<ZRawData>();
        tVirtualTable<ZVirtualProperty<REFTAB32>> Virtual_REFTAB32 = MakeVirtualTable<REFTAB32>();
        tVirtualTable<ZVirtualProperty<ZBitfield<ITEMSTATE>>> Virtual_ZBitfield_ITEMSTATE = MakeVirtualTable<ZBitfield<ITEMSTATE>>();
        tVirtualTable<ZVirtualProperty<ZBitfield<WEAPONOPERATION>>> Virtual_ZBitfield_WEAPONOPERATION = MakeVirtualTable<ZBitfield<WEAPONOPERATION>>();
        tVirtualTable<ZVirtualProperty<ZBitfield<EAnimMode>>> Virtual_ZBitfield_EAnimMode = MakeVirtualTable<ZBitfield<EAnimMode>>();

        tVirtualTable<ZVirtualProperty<int[3]>> Virtual_int_3 = MakeVirtualTable<int[3]>();
        tVirtualTable<ZVirtualProperty<float[3]>> Virtual_float_3 = MakeVirtualTable<float[3]>();
        tVirtualTable<ZVirtualProperty<float[4]>> Virtual_float_4 = MakeVirtualTable<float[4]>();
        tVirtualTable<ZVirtualProperty<float[9]>> Virtual_float_9 = MakeVirtualTable<float[9]>();

        tVirtualTable<ZVirtualEnumProperty<EBoundingBox>> VirtualEnum_EBoundingBox = MakeVirtualEnumTable<EBoundingBox>();
    }
}

decltype(&Glacier::RTP::VirtualTables::Data_bool) VirtualTable_DP__1 = &Glacier::RTP::VirtualTables::Data_bool;
decltype(&Glacier::RTP::VirtualTables::Data_ZCOLOR) VirtualTable_DP__2 = &Glacier::RTP::VirtualTables::Data_ZCOLOR;
decltype(&Glacier::RTP::VirtualTables::Data_REFTAB) VirtualTable_DP__3 = &Glacier::RTP::VirtualTables::Data_REFTAB;
decltype(&Glacier::RTP::VirtualTables::Data_REFTAB32) VirtualTable_DP__4 = &Glacier::RTP::VirtualTables::Data_REFTAB32;
decltype(&Glacier::RTP::VirtualTables::Data_ZGEOMREF) VirtualTable_DP__5 = &Glacier::RTP::VirtualTables::Data_ZGEOMREF;
decltype(&Glacier::RTP::VirtualTables::Data_uint) VirtualTable_DP__6 = &Glacier::RTP::VirtualTables::Data_uint;
decltype(&Glacier::RTP::VirtualTables::Data_int) VirtualTable_DP__7 = &Glacier::RTP::VirtualTables::Data_int;
decltype(&Glacier::RTP::VirtualTables::Data_ZBitfield_ESecurityZone) VirtualTable_DP__9 = &Glacier::RTP::VirtualTables::Data_ZBitfield_ESecurityZone;
decltype(&Glacier::RTP::VirtualTables::Data_float) VirtualTable_DP__11 = &Glacier::RTP::VirtualTables::Data_float;
decltype(&Glacier::RTP::VirtualTables::Data_float_3) VirtualTable_DP__12 = &Glacier::RTP::VirtualTables::Data_float_3;
decltype(&Glacier::RTP::VirtualTables::Data_short) VirtualTable_DP__13 = &Glacier::RTP::VirtualTables::Data_short;
decltype(&Glacier::RTP::VirtualTables::Data_int) VirtualTable_DP__14 = &Glacier::RTP::VirtualTables::Data_int;
decltype(&Glacier::RTP::VirtualTables::Data_ZRTString) VirtualTable_DP__16 = &Glacier::RTP::VirtualTables::Data_ZRTString;
decltype(&Glacier::RTP::VirtualTables::Data_ZAUDIOREF) VirtualTable_DP__21 = &Glacier::RTP::VirtualTables::Data_ZAUDIOREF;
decltype(&Glacier::RTP::VirtualTables::Data_float_4) VirtualTable_DP__24 = &Glacier::RTP::VirtualTables::Data_float_4;
decltype(&Glacier::RTP::VirtualTables::Data_uint_4) VirtualTable_DP__27 = &Glacier::RTP::VirtualTables::Data_uint_4;
decltype(&Glacier::RTP::VirtualTables::Data_float_9) VirtualTable_DP__28 = &Glacier::RTP::VirtualTables::Data_float_9;
decltype(&Glacier::RTP::VirtualTables::Data_ZFILENAME) VirtualTable_DP__30 = &Glacier::RTP::VirtualTables::Data_ZFILENAME;
decltype(&Glacier::RTP::VirtualTables::Data_float_8_4) VirtualTable_DP__31 = &Glacier::RTP::VirtualTables::Data_float_8_4;
decltype(&Glacier::RTP::VirtualTables::Data_uchar) VirtualTable_DP__32 = &Glacier::RTP::VirtualTables::Data_uchar;
decltype(&Glacier::RTP::VirtualTables::Data_TIMETYPE) VirtualTable_DP__39 = &Glacier::RTP::VirtualTables::Data_TIMETYPE;
decltype(&Glacier::RTP::VirtualTables::Data_ZRawData) VirtualTable_DP__44 = &Glacier::RTP::VirtualTables::Data_ZRawData;
decltype(&Glacier::RTP::VirtualTables::Data_int_3) VirtualTable_DP__56 = &Glacier::RTP::VirtualTables::Data_int_3;
decltype(&Glacier::RTP::VirtualTables::Data_uint) VirtualTable_DP__58 = &Glacier::RTP::VirtualTables::Data_uint;
decltype(&Glacier::RTP::VirtualTables::Data_uint_64) VirtualTable_DP__59 = &Glacier::RTP::VirtualTables::Data_uint_64;
decltype(&Glacier::RTP::VirtualTables::Data_uint_32) VirtualTable_DP__60 = &Glacier::RTP::VirtualTables::Data_uint_32;
decltype(&Glacier::RTP::VirtualTables::Data_uint_32) VirtualTable_DP__61 = &Glacier::RTP::VirtualTables::Data_uint_32;
decltype(&Glacier::RTP::VirtualTables::Data_uint_32) VirtualTable_DP__62 = &Glacier::RTP::VirtualTables::Data_uint_32;
decltype(&Glacier::RTP::VirtualTables::Data_float) VirtualTable_DP__68 = &Glacier::RTP::VirtualTables::Data_float;
decltype(&Glacier::RTP::VirtualTables::Data_float_12) VirtualTable_DP__71 = &Glacier::RTP::VirtualTables::Data_float_12;
decltype(&Glacier::RTP::VirtualTables::Data_float_2) VirtualTable_DP__76 = &Glacier::RTP::VirtualTables::Data_float_2;
decltype(&Glacier::RTP::VirtualTables::Data_float_4) VirtualTable_DP__79 = &Glacier::RTP::VirtualTables::Data_float_4;
decltype(&Glacier::RTP::VirtualTables::Data_float_2_3) VirtualTable_DP__89 = &Glacier::RTP::VirtualTables::Data_float_2_3;
decltype(&Glacier::RTP::VirtualTables::Data_ZBitfield_WEAPONOPERATION) VirtualTable_DP__114 = &Glacier::RTP::VirtualTables::Data_ZBitfield_WEAPONOPERATION;
decltype(&Glacier::RTP::VirtualTables::Data_ZBitfield_ITEMSTATE) VirtualTable_DP__118 = &Glacier::RTP::VirtualTables::Data_ZBitfield_ITEMSTATE;
decltype(&Glacier::RTP::VirtualTables::Data_char) VirtualTable_DP__123 = &Glacier::RTP::VirtualTables::Data_char;
decltype(&Glacier::RTP::VirtualTables::Data_float_4) VirtualTable_DP__126 = &Glacier::RTP::VirtualTables::Data_float_4;
decltype(&Glacier::RTP::VirtualTables::Data_ushort) VirtualTable_DP__136 = &Glacier::RTP::VirtualTables::Data_ushort;
decltype(&Glacier::RTP::VirtualTables::Data_float_3) VirtualTable_DP__149 = &Glacier::RTP::VirtualTables::Data_float_3;
decltype(&Glacier::RTP::VirtualTables::Data_uint_2) VirtualTable_DP__171 = &Glacier::RTP::VirtualTables::Data_uint_2;
decltype(&Glacier::RTP::VirtualTables::Data_uint_15) VirtualTable_DP__175 = &Glacier::RTP::VirtualTables::Data_uint_15;
decltype(&Glacier::RTP::VirtualTables::Data_char) VirtualTable_DP__182 = &Glacier::RTP::VirtualTables::Data_char;
decltype(&Glacier::RTP::VirtualTables::Data_int_21) VirtualTable_DP__184 = &Glacier::RTP::VirtualTables::Data_int_21;
decltype(&Glacier::RTP::VirtualTables::Data_uchar) VirtualTable_DP__187 = &Glacier::RTP::VirtualTables::Data_uchar;
decltype(&Glacier::RTP::VirtualTables::Data_ZANIM) VirtualTable_DP__188 = &Glacier::RTP::VirtualTables::Data_ZANIM;
decltype(&Glacier::RTP::VirtualTables::Data_ZGEOMREF_64) VirtualTable_DP__191 = &Glacier::RTP::VirtualTables::Data_ZGEOMREF_64;
decltype(&Glacier::RTP::VirtualTables::Data_ZGEOMREF_6) VirtualTable_DP__192 = &Glacier::RTP::VirtualTables::Data_ZGEOMREF_6;
decltype(&Glacier::RTP::VirtualTables::Data_int) VirtualTable_DP__215 = &Glacier::RTP::VirtualTables::Data_int;
decltype(&Glacier::RTP::VirtualTables::Data_uint_128) VirtualTable_DP__216 = &Glacier::RTP::VirtualTables::Data_uint_128;
decltype(&Glacier::RTP::VirtualTables::Enum) VirtualTable_EP = &Glacier::RTP::VirtualTables::Enum;
decltype(&Glacier::RTP::VirtualTables::Virtual_bool) VirtualTable_VP__1 = &Glacier::RTP::VirtualTables::Virtual_bool;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZCOLOR) VirtualTable_VP__2 = &Glacier::RTP::VirtualTables::Virtual_ZCOLOR;
decltype(&Glacier::RTP::VirtualTables::Virtual_REFTAB32) VirtualTable_VP__4 = &Glacier::RTP::VirtualTables::Virtual_REFTAB32;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZGEOMREF) VirtualTable_VP__5 = &Glacier::RTP::VirtualTables::Virtual_ZGEOMREF;
decltype(&Glacier::RTP::VirtualTables::Virtual_uint) VirtualTable_VP__6 = &Glacier::RTP::VirtualTables::Virtual_uint;
decltype(&Glacier::RTP::VirtualTables::Virtual_int) VirtualTable_VP__7 = &Glacier::RTP::VirtualTables::Virtual_int;
decltype(&Glacier::RTP::VirtualTables::Virtual_float) VirtualTable_VP__11 = &Glacier::RTP::VirtualTables::Virtual_float;
decltype(&Glacier::RTP::VirtualTables::Virtual_float_3) VirtualTable_VP__12 = &Glacier::RTP::VirtualTables::Virtual_float_3;
decltype(&Glacier::RTP::VirtualTables::Virtual_int) VirtualTable_VP__14 = &Glacier::RTP::VirtualTables::Virtual_int;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZRTString) VirtualTable_VP__16 = &Glacier::RTP::VirtualTables::Virtual_ZRTString;
decltype(&Glacier::RTP::VirtualTables::Virtual_float_9) VirtualTable_VP__28 = &Glacier::RTP::VirtualTables::Virtual_float_9;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZFILENAME) VirtualTable_VP__30 = &Glacier::RTP::VirtualTables::Virtual_ZFILENAME;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZRawData) VirtualTable_VP__44 = &Glacier::RTP::VirtualTables::Virtual_ZRawData;
decltype(&Glacier::RTP::VirtualTables::Virtual_int_3) VirtualTable_VP__56 = &Glacier::RTP::VirtualTables::Virtual_int_3;
decltype(&Glacier::RTP::VirtualTables::Virtual_float_4) VirtualTable_VP__79 = &Glacier::RTP::VirtualTables::Virtual_float_4;
decltype(&Glacier::RTP::VirtualTables::Virtual_float_3) VirtualTable_VP__95 = &Glacier::RTP::VirtualTables::Virtual_float_3;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZBitfield_EAnimMode) VirtualTable_VP__169 = &Glacier::RTP::VirtualTables::Virtual_ZBitfield_EAnimMode;
decltype(&Glacier::RTP::VirtualTables::Virtual_ZANIM) VirtualTable_VP__188 = &Glacier::RTP::VirtualTables::Virtual_ZANIM;
decltype(&Glacier::RTP::VirtualTables::VirtualEnum_EBoundingBox) VirtualTable_VP__189 = &Glacier::RTP::VirtualTables::VirtualEnum_EBoundingBox;
