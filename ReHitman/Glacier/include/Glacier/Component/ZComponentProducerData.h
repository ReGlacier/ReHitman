#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/Fwds.h>
#include <Glacier/Component/Component.h>
#include <cstdint>


namespace Glacier
{
    class ZComponentProducerData
    {
    public:
        // constants
        static constexpr int NDEPENDENCIES = 5;

        // types
        enum Types { DEPENDENT = 1, CHILD = 2 };

        // methods
        ZComponentProducerData(
            ZComponentId componentId, 
            uint16_t componentGroup, 
            ZComponentCreator pCreator, 
            ZComponentDestructor pDestructor, 
            ZComponentSetInstance pSetInstance, 
            const char* creationParameters);
        ~ZComponentProducerData();

        static bool CompareIds(ZComponentId a, ZComponentId b);
        ZComponentId GetComponentId() const;
        uint16_t GetComponentGroup() const;
        Types GetType() const;
        ZComponentId GetDependency(int iDependencyIdx) const;
        uint32_t DependencySize() const;
        ZComponentId GetParent() const;
        ZComponentProducerData* GetNext() const;
        bool IsCopy() const;
        bool IsInitialized() const;
        void AddDependency(ZComponentId id);
        void SetParent(ZComponentId parent);
        void SetComponent(ZComponentBase* pComponent);
        void SetComponentPointer(ZComponentProducerData* ptr);
        ZComponentBase* GetComponent();
        ZComponentBase* Create();
        void SetInstance();
        void Destroy(ZComponentBase* pComponent);

        // members
        ZComponentId m_ComponentId;
        uint16_t m_ComponentGroup;
        uint8_t m_Type;
        uint8_t m_DependencySize;
        ZComponentId m_DependentIds[NDEPENDENCIES];
        bool m_IsInitialized;
        ZComponentBase* m_pComponent { nullptr };
        ZComponentBase** m_pComponentPointer { nullptr };
        ZComponentProducerData* m_pNext { nullptr };
        ZComponentCreator m_pCreator { nullptr };
        ZComponentDestructor m_pDestructor { nullptr };
        ZComponentSetInstance m_pSetInstance { nullptr };
    };
}