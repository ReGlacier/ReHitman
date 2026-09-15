#include <Glacier/Component/ZComponentProducerData.h>
#include <Glacier/Component/ZComponentGlobalList.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>
#include <cstdio>


namespace Glacier
{
    namespace 
    {
        // PS2 at 0x0098C9C0
        static constexpr uint8_t kLUT[] =
        {
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 
            0x28, 0x28, 0x28, 0x28, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
            0x20, 0x20, 0x88, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
            0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x04, 0x04, 
            0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x10, 0x10, 
            0x10, 0x10, 0x10, 0x10, 0x10, 0x41, 0x41, 0x41, 0x41, 0x41, 
            0x41, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
            0x01, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x42, 0x42, 0x42, 
            0x42, 0x42, 0x42, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
            0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
            0x02, 0x02, 0x02, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        inline bool isspace(char ch)
        {
            return kLUT[static_cast<uint8_t>(ch)] & 0x8u;
        }

        inline bool isalnum(char ch)
        {
            return kLUT[static_cast<uint8_t>(ch)] & 0x7u;
        }

        inline char tolower(char ch)
        {
            return ((kLUT[static_cast<uint8_t>(ch)] & 1) != 0) ? static_cast<char>(static_cast<uint8_t>(ch) + 32) : ch;
        }

        const char* GetToken(const char*& pstr)
        {
            while (*pstr && isspace(static_cast<unsigned char>(*pstr)))
            {
                ++pstr;
            }

            const char* tokenStart = pstr;

            while (*pstr && isalnum(static_cast<unsigned char>(*pstr)))
            {
                ++pstr;
            }

            if (*pstr)
            {
                ++pstr;
            }

            return tokenStart;
        }
    }

    ZComponentProducerData::ZComponentProducerData(ZComponentId componentId, uint16_t componentGroup, ZComponentCreator pCreator, ZComponentDestructor pDestructor, ZComponentSetInstance pSetInstance, const char* creationParameters)
        : m_ComponentId(componentId)
        , m_ComponentGroup(componentGroup)
        , m_Type(ZComponentProducerData::Types::DEPENDENT)
        , m_IsInitialized(false)
        , m_pComponent(nullptr)
        , m_pComponentPointer(&m_pComponent)
        , m_pNext(nullptr)
        , m_pCreator(pCreator)
        , m_pDestructor(pDestructor)
        , m_pSetInstance(pSetInstance)
    {
        const char* pParams = creationParameters;

        if (pParams)
        {
            const char* pCurrentToken = GetToken(pParams);
            if (pCurrentToken && *pCurrentToken)
            {
                if (CompareIds(pCurrentToken, "depends"))
                {
                    m_Type = ZComponentProducerData::Types::DEPENDENT;
                    while (true)
                    {
                        const char* pDependentOn = GetToken(pParams);
                        if (!pDependentOn || !*pDependentOn)
                            break;

                        AddDependency(pDependentOn);
                    }
                }
                else if (CompareIds(pCurrentToken, "parent"))
                {
                    m_Type = ZComponentProducerData::Types::CHILD;

                    const char* pParentOn = GetToken(pParams);
                    if (pParentOn && *pParentOn)
                    {
                        SetParent(pParentOn);
                    }
                }
                else
                {
                    printf("Invalid component parameters %s\n", pCurrentToken);
                    ZASSERT(false);
                }
            }
        }

        ZComponentGlobalList::Instance().Add(this);
    }

    ZComponentProducerData::~ZComponentProducerData()
    {
        // Do nothing?
    }

    bool ZComponentProducerData::CompareIds(ZComponentId a, ZComponentId b)
    {
        while (*a && isalnum(*a) && *b && isalnum(*b))
        {
            if (tolower(*a) != tolower(*b))
            {
                return false;
            }

            ++a;
            ++b;
        }

        if ((!*a || isspace(*a)) && (!*b || isspace(*b)))
            return true;

        return false;
    }

    ZComponentId ZComponentProducerData::GetComponentId() const
    {
        return m_ComponentId;
    }

    uint16_t ZComponentProducerData::GetComponentGroup() const
    {
        return m_ComponentGroup;
    }

    ZComponentProducerData::Types ZComponentProducerData::GetType() const
    {
        return static_cast<Types>(m_Type);
    }

    ZComponentId ZComponentProducerData::GetDependency(int iDependencyIdx) const
    {
        ZASSERT(iDependencyIdx < DependencySize());

        return m_DependentIds[iDependencyIdx];
    }

    uint32_t ZComponentProducerData::DependencySize() const
    {
        return m_DependencySize;
    }

    ZComponentId ZComponentProducerData::GetParent() const
    {
        ZASSERT(m_Type == Types::CHILD);
        ZASSERT(m_DependentIds[0]);

        return m_DependentIds[0];
    }

    ZComponentProducerData* ZComponentProducerData::GetNext() const
    {
        return m_pNext;
    }

    bool ZComponentProducerData::IsCopy() const
    {
        return m_pComponentPointer != &m_pComponent;
    }

    bool ZComponentProducerData::IsInitialized() const
    {
        return m_IsInitialized;
    }

    void ZComponentProducerData::AddDependency(ZComponentId id)
    {
        ZASSERT(m_DependencySize < NDEPENDENCIES);
        ZASSERT(m_Type == DEPENDENT);

        m_DependentIds[m_DependencySize++] = id;
    }

    void ZComponentProducerData::SetParent(ZComponentId parent)
    {
        ZASSERT(m_DependencySize == 0);
        ZASSERT(m_Type == CHILD);

        m_DependentIds[m_DependencySize++] = parent;
    }

    void ZComponentProducerData::SetComponent(ZComponentBase* pComponent)
    {
        ZASSERT(!IsCopy());

        m_pComponent = pComponent;
    }

    void ZComponentProducerData::SetComponentPointer(ZComponentProducerData* ptr)
    {
        if (IsCopy())
        {
            m_pComponentPointer = ptr->m_pComponentPointer;
        }
        else
        {
            m_pComponentPointer = &ptr->m_pComponent;
        }
    }

    ZComponentBase* ZComponentProducerData::GetComponent()
    {
        return *m_pComponentPointer;
    }

    ZComponentBase* ZComponentProducerData::Create()
    {
        ZASSERT(!m_IsInitialized);

        m_IsInitialized = true;
        return m_pCreator();
    }

    void ZComponentProducerData::SetInstance()
    {
        ZASSERT(!m_IsInitialized);

        auto* pComponent = GetComponent();

        m_pSetInstance(pComponent);
        m_IsInitialized = true;
    }

    void ZComponentProducerData::Destroy(ZComponentBase* pComponent)
    {
        m_pDestructor(pComponent);
        m_IsInitialized = false;
    }
}