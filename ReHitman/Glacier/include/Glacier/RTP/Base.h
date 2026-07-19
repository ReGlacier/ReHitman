#pragma once

#include <Glacier/ZUniMemory.h>
#include <Glacier/Runtime/ZEnum.h>
#include <Glacier/Serializer/ZSerializable.h>


namespace Glacier
{
    struct ISerializerStream;
    struct ZSerializableBase;
}

namespace Glacier::RTP
{
    // fwds
    template <typename T> struct ZDataProperty;

    struct cNode
    {
        struct cNode* m_Next{nullptr};
        const char* m_Name{nullptr};
        unsigned int m_Filter{0};
    };

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

    template <typename T>
    struct tVirtualTable
    {
        void(*Load)(RTP::ZDataProperty<T>* pProperty, Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object) = nullptr;
        void(*Save)(RTP::ZDataProperty<T>* pProperty, Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object) = nullptr;
    };

    template <typename T>
    struct ZDataProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZDataProperty<T>>* m_VirtualTable{nullptr};
        T* m_Offset{nullptr};

        static void Load(Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object);
        static void Save(Glacier::ISerializerStream& stream, Glacier::ZSerializableBase& object);
    };

    template <typename T>
    struct ZVirtualProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZVirtualProperty<T>>* m_VirtualTable { nullptr };
        struct GetPMF 
        {
            void(ZSerializableBase::*__pfn)(T&) = nullptr;
            int __delta = 0;
        } m_Get{};
        struct SetPMF 
        {
            void(ZSerializableBase::*__pfn)(const T&) = nullptr;
            int __delta = 0;
        } m_Set{};
    };

    template <typename T>
    struct ZVirtualEnumProperty
    {
        RTP::cNode m_Node{};
        RTP::tVirtualTable<RTP::ZVirtualEnumProperty<T>>* m_VirtualTable { nullptr };
        struct GetPMF 
        {
            void(ZSerializableBase::*__pfn)(T&) = nullptr;
            int __delta = 0;
        } m_Get{};
        struct SetPMF 
        {
            void(ZSerializableBase::*__pfn)(const T&) = nullptr;
            int __delta = 0;
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
}
