#pragma once

#include <Glacier/ZUniMemory.h>
#include <Glacier/Runtime/ZEnum.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <utility>


namespace Glacier
{
    struct ISerializerStream;
    struct ZSerializableBase;
    struct ZRTString;
}

namespace Glacier::RTP
{
    // fwds
    template <typename T> struct ZDataProperty;

    struct cNode
    {
        struct cNode* m_Next{nullptr}; // +0x0
        const char* m_Name{nullptr}; // +0x4
        unsigned int m_Filter{0}; // +0x8
    };
    RE_VERIFY_OFFSET(cNode, m_Filter, 0x8);

    struct cVirtualTable
    {
        void(*Load)(RTP::cNode*, ISerializerStream*, ZSerializableBase*);
        void(*Save)(RTP::cNode*, ISerializerStream*, ZSerializableBase*);
    };

    struct cProperty
    {
        RTP::cNode m_Node;
        RTP::cVirtualTable *m_VirtualTable;
    };

    struct ZPropertyInfo
    {
        cNode* First{nullptr};
        ZPropertyInfo* Super{nullptr};
        const char* Name{nullptr};
    };

    template <typename TProperty>
    struct tVirtualTable
    {
        void(*Load)(TProperty* pProperty, Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object) = nullptr;
        void(*Save)(TProperty* pProperty, Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object) = nullptr;
    };

    template <typename T>
    struct ZDataProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZDataProperty<T>>* m_VirtualTable{nullptr};
        T* m_Offset{nullptr};

        static void Load(Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object);
        static void Save(Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object);

        // Layout-neutral helper: allows `.m_Next = Item` instead of `.m_Next = &Item.m_Node` in property lists.
        operator RTP::cNode*() { return &m_Node; }
        operator const RTP::cNode*() const { return &m_Node; }
    };

    struct ZEnumProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZEnumProperty>* m_VirtualTable{nullptr};
        void* m_Offset{nullptr};
        ZEnumInfo* m_Info{nullptr};

        // Layout-neutral helper: allows `.m_Next = Item` instead of `.m_Next = &Item.m_Node` in property lists.
        operator RTP::cNode*() { return &m_Node; }
        operator const RTP::cNode*() const { return &m_Node; }
    };

    template <typename T>
    struct ZVirtualProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZVirtualProperty<T>>* m_VirtualTable { nullptr };

        // Layout-neutral helper: allows `.m_Next = Item` instead of `.m_Next = &Item.m_Node` in property lists.
        operator RTP::cNode*() { return &m_Node; }
        operator const RTP::cNode*() const { return &m_Node; }

        struct GetPMF
        {
            void(ZSerializableBase::*__pfn)(T&) = nullptr;
            int __delta = 0;

            GetPMF() = default;

            template <typename TOwner>
            GetPMF(void(TOwner::*pfn)(T&))
            {
                *this = pfn;
            }

            template <typename TOwner>
            GetPMF& operator=(void(TOwner::*pfn)(T&))
            {
                static_assert(sizeof(__pfn) <= sizeof(pfn));

                __pfn = nullptr;
                __delta = 0;
                std::memcpy(&__pfn, &pfn, sizeof(__pfn));

                return *this;
            }
        } m_Get{};
        struct SetPMF 
        {
            void(ZSerializableBase::*__pfn)(const T&) = nullptr;
            int __delta = 0;

            SetPMF() = default;

            template <typename TOwner>
            SetPMF(void(TOwner::*pfn)(const T&))
            {
                *this = pfn;
            }

            template <typename TOwner>
            SetPMF& operator=(void(TOwner::*pfn)(const T&))
            {
                static_assert(sizeof(__pfn) <= sizeof(pfn));

                __pfn = nullptr;
                __delta = 0;
                std::memcpy(&__pfn, &pfn, sizeof(__pfn));

                return *this;
            }
        } m_Set{};
    };

    template <typename T>
    struct ZVirtualEnumProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZVirtualEnumProperty<T>>* m_VirtualTable { nullptr };

        // Layout-neutral helper: allows `.m_Next = Item` instead of `.m_Next = &Item.m_Node` in property lists.
        operator RTP::cNode*() { return &m_Node; }
        operator const RTP::cNode*() const { return &m_Node; }

        struct GetPMF
        {
            void(ZSerializableBase::*__pfn)(T&) = nullptr;
            int __delta = 0;

            GetPMF() = default;

            template <typename TOwner>
            GetPMF(void(TOwner::*pfn)(T&))
            {
                *this = pfn;
            }

            template <typename TOwner>
            GetPMF& operator=(void(TOwner::*pfn)(T&))
            {
                static_assert(sizeof(__pfn) <= sizeof(pfn));

                __pfn = nullptr;
                __delta = 0;
                std::memcpy(&__pfn, &pfn, sizeof(__pfn));

                return *this;
            }
        } m_Get{};
        struct SetPMF 
        {
            void(ZSerializableBase::*__pfn)(const T&) = nullptr;
            int __delta = 0;

            SetPMF() = default;

            template <typename TOwner>
            SetPMF(void(TOwner::*pfn)(const T&))
            {
                *this = pfn;
            }

            template <typename TOwner>
            SetPMF& operator=(void(TOwner::*pfn)(const T&))
            {
                static_assert(sizeof(__pfn) <= sizeof(pfn));

                __pfn = nullptr;
                __delta = 0;
                std::memcpy(&__pfn, &pfn, sizeof(__pfn));

                return *this;
            }
        } m_Set{};
        ZEnumInfo* m_Info { nullptr };
    };

    struct cBase : public ZSerializable
    {
        // vtbl
        void LoadObject(IInputSerializerStream& stream) override;
        void SaveObject(IOutputSerializerStream& stream) override;
        virtual const ZPropertyInfo& GetProperties() const;
        // methods
        // static
        STATIC_CLASS_VAR(cBase, ZPropertyInfo, Info);
    };

    void LoadSerializable(ZSerializableBase* serializable, const ZPropertyInfo* properties, ISerializerStream& stream);
    void SaveSerializable(ZSerializableBase* serializable, const ZPropertyInfo* properties, ISerializerStream& stream);
    void LoadConstString(ISerializerStream& stream, const char* name, ZRTString& value);
    void SaveConstString(ISerializerStream& stream, const char* name, ZRTString& value);
}

#define CLASS_PROPERTY(Class, Member) \
    reinterpret_cast<decltype(std::declval<Class>().Member)*>(offsetof(Class, Member))