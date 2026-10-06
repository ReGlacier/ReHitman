#include <Glacier/Geom/ZGeomEventList.h>


namespace Glacier
{
    ZGeomEventListBuffers::ZGeomEventListBuffers()
    {
        for (uint32_t i = 0; i != MAX_NR_BUFFERS; ++i)
        {
            m_Free_Buffer_Ids[i] = static_cast<uint8_t>(i);
            m_GeomEventListBuffers[i] = nullptr;
        }

        m_iNr_Free_Buffer_Ids = MAX_NR_BUFFERS;
        m_iLast_Added_To_Buffer_Id = 0;
    }

    ZGeomEventListBuffers::~ZGeomEventListBuffers()
    {
        for (uint32_t i = 0; i != MAX_NR_BUFFERS; ++i)
        {
            if (m_GeomEventListBuffers[i])
            {
                ZUniMemory::Free(m_GeomEventListBuffers[i]);
                m_GeomEventListBuffers[i] = nullptr;
            }
        }
    }

    ZGeomEventListBuffers::SGeomEventListBufferEntity* ZGeomEventListBuffers::ZGeomEventListBuffer::Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id)
    {
        return &m_Buffer_Entities[Entity_Id.m_iEntity_Id];
    }

    const ZGeomEventListBuffers::SGeomEventListBufferEntity* ZGeomEventListBuffers::ZGeomEventListBuffer::Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id) const
    {
        return &m_Buffer_Entities[Entity_Id.m_iEntity_Id];
    }

    uint8_t ZGeomEventListBuffers::ZGeomEventListBuffer::Alloc_Entity()
    {
        ZASSERT(m_iFirst_Free_Entity != 0xFF);
        ZASSERT(m_iNr_Free_Entities);

        uint8_t iEntity_Id = m_iFirst_Free_Entity;
        SGeomEventListBufferEntity& Entity = m_Buffer_Entities[m_iFirst_Free_Entity];
        m_iFirst_Free_Entity = Entity.m_GeomEventListBufferEntity_Free.m_iNext_Free;
        Entity.m_GeomEventListBufferEntity_Used.m_iValue.SetVal(0);
        Entity.m_GeomEventListBufferEntity_Used.m_Next_In_List = { 0, 0xFF };

        if (m_iFirst_Free_Entity != 0xFF)
        {
            m_Buffer_Entities[m_iFirst_Free_Entity].m_GeomEventListBufferEntity_Free.m_iPrev_Free = 0xFF;
        }

        --m_iNr_Free_Entities;
        return iEntity_Id;
    }

    bool ZGeomEventListBuffers::ZGeomEventListBuffer::Free_Entity(uint8_t iEntity_Id)
    {
        ZASSERT(m_iNr_Free_Entities != MAX_NR_ENTRIES);

        SGeomEventListBufferEntity& Entity = m_Buffer_Entities[iEntity_Id];
        Entity.m_GeomEventListBufferEntity_Free.m_iNext_Free = m_iFirst_Free_Entity;
        Entity.m_GeomEventListBufferEntity_Free.m_iPrev_Free = 0xFF;

        if (m_iFirst_Free_Entity != 0xFF)
        {
            m_Buffer_Entities[m_iFirst_Free_Entity].m_GeomEventListBufferEntity_Free.m_iPrev_Free = iEntity_Id;
        }

        m_iFirst_Free_Entity = iEntity_Id;
        ++m_iNr_Free_Entities;

        return m_iNr_Free_Entities == MAX_NR_ENTRIES;
    }

    ZGeomEventListBuffers::SGeomEventListBufferEntity* ZGeomEventListBuffers::Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id)
    {
        if (Entity_Id.m_iEntity_Id == 0xFF)
        {
            return nullptr;
        }

        ZASSERT(m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]);
        ZASSERT(m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]->Get_Buffer_Entity(Entity_Id));

        return m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]->Get_Buffer_Entity(Entity_Id);
    }

    const ZGeomEventListBuffers::SGeomEventListBufferEntity* ZGeomEventListBuffers::Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id) const
    {
        if (Entity_Id.m_iEntity_Id == 0xFF)
        {
            return nullptr;
        }

        ZASSERT(m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]);
        ZASSERT(m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]->Get_Buffer_Entity(Entity_Id));

        return m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]->Get_Buffer_Entity(Entity_Id);
    }

    uint32_t ZGeomEventListBuffers::Get_List_Size(const SGeomEventListBufferEntity_Id& First_Entity_Id) const
    {
        const SGeomEventListBufferEntity* pBuffer = Get_Buffer_Entity(First_Entity_Id);
        uint32_t iSize = 0;

        while (pBuffer)
        {
            ++iSize;

            SGeomEventListBufferEntity_Id Next_Entity_Id = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
            pBuffer = Get_Buffer_Entity(Next_Entity_Id);
        }

        return iSize;
    }

    bool ZGeomEventListBuffers::Free_Entity(const SGeomEventListBufferEntity_Id& Entity_Id)
    {
        ZGeomEventListBuffer* pBuffer = m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id];
        bool bFreeBuffer = pBuffer->Free_Entity(Entity_Id.m_iEntity_Id);

        if (bFreeBuffer)
        {
            Free_Buffer(pBuffer);
        }

        return bFreeBuffer;
    }

    void ZGeomEventListBuffers::Free_Buffer(ZGeomEventListBuffer* pBuffer)
    {
        m_GeomEventListBuffers[pBuffer->m_iBuffer_Id] = nullptr;
        Free_Buffer_Id(pBuffer->m_iBuffer_Id);
        ZUniMemory::Free(pBuffer);
    }

    void ZGeomEventListBuffers::Free_Buffer_Id(uint8_t iBuffer_Id)
    {
        m_Free_Buffer_Ids[m_iNr_Free_Buffer_Ids++] = iBuffer_Id;
    }

    void ZGeomEventListBuffers::Free_All(const SGeomEventListBufferEntity_Id& First_Entity_Id)
    {
        SGeomEventListBufferEntity_Id Entity_Id = First_Entity_Id;
        SGeomEventListBufferEntity* pBuffer = Get_Buffer_Entity(Entity_Id);

        while (pBuffer)
        {
            SGeomEventListBufferEntity_Id Next_Entity_Id = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
            Free_Entity(Entity_Id);
            Entity_Id = Next_Entity_Id;
            pBuffer = Get_Buffer_Entity(Entity_Id);
        }
    }

    uint32_t ZGeomEventListBuffers::Get_Value(const SGeomEventListBufferEntity_Id& Entity_Id) const
    {
        return Get_Buffer_Entity(Entity_Id)->m_GeomEventListBufferEntity_Used.m_iValue.GetVal();
    }

    void ZGeomEventListBuffers::Set_Value(const SGeomEventListBufferEntity_Id& Entity_Id, uint32_t iValue)
    {
        Get_Buffer_Entity(Entity_Id)->m_GeomEventListBufferEntity_Used.m_iValue.SetVal(iValue);
    }

    ZGeomEventListBuffers::SGeomEventListBufferEntity_Id ZGeomEventListBuffers::Alloc_Value(uint32_t iValue)
    {
        SGeomEventListBufferEntity_Id Entity_Id = Alloc_Entity();
        Set_Value(Entity_Id, iValue);
        return Entity_Id;
    }

    ZGeomEventListBuffers::SGeomEventListBufferEntity_Id ZGeomEventListBuffers::Alloc_Link_Value(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue)
    {
        SGeomEventListBufferEntity_Id Entity_Id = Alloc_Value(iValue);
        Link_Entity_At_End(First_Entity_Id, Entity_Id);
        return Entity_Id;
    }

    ZGeomEventListBuffers::SGeomEventListBufferEntity_Id ZGeomEventListBuffers::Alloc_Entity()
    {
        ZASSERT(m_iLast_Added_To_Buffer_Id < MAX_NR_BUFFERS);

        SGeomEventListBufferEntity_Id Entity_Id;
        Entity_Id.m_iBuffer_Id = Get_Buffer_With_Space();
        Entity_Id.m_iEntity_Id = m_GeomEventListBuffers[Entity_Id.m_iBuffer_Id]->Alloc_Entity();
        return Entity_Id;
    }

    uint8_t ZGeomEventListBuffers::Get_Buffer_With_Space()
    {
        ZGeomEventListBuffer* pBuffer = m_GeomEventListBuffers[m_iLast_Added_To_Buffer_Id];

        if (!pBuffer || pBuffer->m_iFirst_Free_Entity == 0xFF)
        {
            for (uint32_t i = 0; i != MAX_NR_BUFFERS; ++i)
            {
                pBuffer = m_GeomEventListBuffers[i];

                if (pBuffer && pBuffer->m_iFirst_Free_Entity != 0xFF)
                {
                    m_iLast_Added_To_Buffer_Id = static_cast<uint16_t>(i);
                    return static_cast<uint8_t>(m_iLast_Added_To_Buffer_Id);
                }
            }

            m_iLast_Added_To_Buffer_Id = Alloc_Buffer()->m_iBuffer_Id;
        }

        return static_cast<uint8_t>(m_iLast_Added_To_Buffer_Id);
    }

    ZGeomEventListBuffers::ZGeomEventListBuffer* ZGeomEventListBuffers::Alloc_Buffer()
    {
        ZGeomEventListBuffer* pBuffer = static_cast<ZGeomEventListBuffer*>(ZUniMemory::Allocate(sizeof(ZGeomEventListBuffer)));
        pBuffer->m_iBuffer_Id = New_Buffer_Id();

        for (uint32_t i = 0; i != ZGeomEventListBuffer::MAX_NR_ENTRIES; ++i)
        {
            pBuffer->m_Buffer_Entities[i].m_GeomEventListBufferEntity_Free.m_iPrev_Free = static_cast<uint8_t>(i - 1);
            pBuffer->m_Buffer_Entities[i].m_GeomEventListBufferEntity_Free.m_iNext_Free = static_cast<uint8_t>(i + 1);
        }

        pBuffer->m_Buffer_Entities[0].m_GeomEventListBufferEntity_Free.m_iPrev_Free = 0xFF;
        pBuffer->m_Buffer_Entities[ZGeomEventListBuffer::MAX_NR_ENTRIES - 1].m_GeomEventListBufferEntity_Free.m_iNext_Free = 0xFF;
        pBuffer->m_iFirst_Free_Entity = 0;
        pBuffer->m_iNr_Free_Entities = ZGeomEventListBuffer::MAX_NR_ENTRIES;
        m_GeomEventListBuffers[pBuffer->m_iBuffer_Id] = pBuffer;
        return pBuffer;
    }

    uint8_t ZGeomEventListBuffers::New_Buffer_Id()
    {
        ZASSERT(m_iNr_Free_Buffer_Ids);
        return m_Free_Buffer_Ids[--m_iNr_Free_Buffer_Ids];
    }

    void ZGeomEventListBuffers::NextValueRun(ValueRun& Run)
    {
        ZASSERT(Run.m_pEntity);

        SGeomEventListBufferEntity_Id Entity_Id = Run.m_pEntity->m_GeomEventListBufferEntity_Used.m_Next_In_List;

        if (Entity_Id.m_iEntity_Id == 0xFF)
        {
            Run.m_bFin = true;
            Run.m_pEntity = nullptr;
        }
        else
        {
            Run.m_pEntity = Get_Buffer_Entity(Entity_Id);
            Run.m_Entity_Id = Entity_Id;
        }
    }

    void ZGeomEventListBuffers::InitValueRun(ValueRun& Run, const SGeomEventListBufferEntity_Id& First_Entity_Id)
    {
        Run.m_Entity_Id = First_Entity_Id;
        Run.m_pEntity = Get_Buffer_Entity(Run.m_Entity_Id);
    }

    uint32_t ZGeomEventListBuffers::GetValueFromValueRun(ValueRun& Run) const
    {
        return Run.m_pEntity->m_GeomEventListBufferEntity_Used.m_iValue.GetVal();
    }

    uint32_t ZGeomEventListBuffers::Get_Value_Nr(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue_Nr) const
    {
        uint32_t iListSize = Get_List_Size(First_Entity_Id);

        if (iValue_Nr >= iListSize)
        {
            return 0;
        }

        SGeomEventListBufferEntity_Id Entity_Id = First_Entity_Id;
        const SGeomEventListBufferEntity* pBuffer = Get_Buffer_Entity(Entity_Id);

        while (pBuffer && iValue_Nr)
        {
            --iValue_Nr;
            Entity_Id = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
            pBuffer = Get_Buffer_Entity(Entity_Id);
        }

        if (pBuffer)
        {
            return pBuffer->m_GeomEventListBufferEntity_Used.m_iValue.GetVal();
        }

        return 0;
    }

    bool ZGeomEventListBuffers::Exists(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue) const
    {
        SGeomEventListBufferEntity_Id Entity_Id = First_Entity_Id;
        const SGeomEventListBufferEntity* pBuffer = Get_Buffer_Entity(Entity_Id);

        while (pBuffer)
        {
            if (pBuffer->m_GeomEventListBufferEntity_Used.m_iValue.GetVal() == iValue)
            {
                return true;
            }

            Entity_Id = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
            pBuffer = Get_Buffer_Entity(Entity_Id);
        }

        return false;
    }

    void ZGeomEventListBuffers::Link_Entity_At_End(const SGeomEventListBufferEntity_Id& First_Entity_Id, const SGeomEventListBufferEntity_Id& New_Entity_Id)
    {
        SGeomEventListBufferEntity* pBuffer = Get_Buffer_Entity(First_Entity_Id);

        while (pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List.m_iEntity_Id != 0xFF)
        {
            pBuffer = Get_Buffer_Entity(pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List);
        }

        pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List = New_Entity_Id;
    }

    ZGeomEventListBuffers::SGeomEventListBufferEntity_Id ZGeomEventListBuffers::Free_Value(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue)
    {
        SGeomEventListBufferEntity_Id Entity_Id = First_Entity_Id;
        SGeomEventListBufferEntity_Id New_First_Entity_Id = { 0, 0xFF };
        SGeomEventListBufferEntity* pBuffer = Get_Buffer_Entity(Entity_Id);
        SGeomEventListBufferEntity* pFirstEntity = pBuffer;
        SGeomEventListBufferEntity* pLastEntity = nullptr;

        while (pBuffer)
        {
            if (pBuffer->m_GeomEventListBufferEntity_Used.m_iValue.GetVal() == iValue)
            {
                break;
            }

            pLastEntity = pBuffer;
            Entity_Id = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
            pBuffer = Get_Buffer_Entity(Entity_Id);
        }

        if (!pBuffer)
        {
            return New_First_Entity_Id;
        }

        if (pBuffer == pFirstEntity)
        {
            ZASSERT(!pLastEntity);
            New_First_Entity_Id = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
        }
        else
        {
            ZASSERT(pLastEntity);
            pLastEntity->m_GeomEventListBufferEntity_Used.m_Next_In_List = pBuffer->m_GeomEventListBufferEntity_Used.m_Next_In_List;
        }

        Free_Entity(Entity_Id);
        return New_First_Entity_Id;
    }

    ZGeomEventList::ZGeomEventList()
    {
        m_bListEntityOffset_Or_DirectOffset = 1u; // or true?
        m_iControl_Routine_Direct_Id.SetVal(0u);
    }

    ZGeomEventList::~ZGeomEventList() = default;

    void ZGeomEventList::RunRemoveEvent(ZGeomEventListBuffers::ValueRun& Run)
    {
        switch (m_bListEntityOffset_Or_DirectOffset)
        {
            case 1:
                ZASSERT(m_bListEntityOffset_Or_DirectOffset != 1);
                break;

            case 2:
                m_iControl_Routine_Direct_Id.SetVal(0);
                m_bListEntityOffset_Or_DirectOffset = 1;
                Run.m_bFin = true;
                break;

            case 3:
            {
                uint32_t iValue = Run.m_pEntity->m_GeomEventListBufferEntity_Used.m_iValue.GetVal();
                NextValueRun(Run);
                Remove(iValue);
                break;
            }
        }

        Run.m_bRunRemove = true;
    }

    void ZGeomEventList::Remove(uint32_t iValue)
    {
        if (m_bListEntityOffset_Or_DirectOffset == 1)
        {
            return;
        }

        if (m_bListEntityOffset_Or_DirectOffset == 2)
        {
            ZASSERT(Count() == 1);

            if (iValue == m_iControl_Routine_Direct_Id.GetVal())
            {
                m_iControl_Routine_Direct_Id.SetVal(0);
                m_bListEntityOffset_Or_DirectOffset = 1;
            }

            return;
        }

        ZASSERT(m_bListEntityOffset_Or_DirectOffset == 3);

        ZGeomEventListBuffers::SGeomEventListBufferEntity_Id Entity_Id = m_Control_Routine_List_Entity_Id;
        ZGeomEventListBuffers::SGeomEventListBufferEntity_Id New_Entity_Id = ZGeomEventList::m_pGeomEventListBuffers->Free_Value(Entity_Id, iValue);

        if (New_Entity_Id.m_iEntity_Id != 0xFF)
        {
            m_Control_Routine_List_Entity_Id = New_Entity_Id;
        }

        uint32_t iCount = Count();
        ZASSERT(iCount != 0);

        if (iCount == 1)
        {
            Entity_Id = m_Control_Routine_List_Entity_Id;
            uint32_t iRemainingValue = ZGeomEventList::m_pGeomEventListBuffers->Get_Value(Entity_Id);
            ZGeomEventList::m_pGeomEventListBuffers->Free_Value(Entity_Id, iRemainingValue);
            m_iControl_Routine_Direct_Id.SetVal(iRemainingValue);
            m_bListEntityOffset_Or_DirectOffset = 2;
        }
    }

    void ZGeomEventList::NextValueRun(ZGeomEventListBuffers::ValueRun& Run)
    {
        if (Run.m_bRunRemove)
        {
            return;
        }

        switch (m_bListEntityOffset_Or_DirectOffset)
        {
            case 1:
                ZASSERT(m_bListEntityOffset_Or_DirectOffset != 1);
                break;

            case 3:
                ZASSERT(Run.m_pEntity);
                ZGeomEventList::m_pGeomEventListBuffers->NextValueRun(Run);
                break;

            case 2:
                Run.m_bFin = true;
                break;
        }
    }
        
    void ZGeomEventList::InitValueRun(ZGeomEventListBuffers::ValueRun& Run)
    {
        Run.m_pEntity = nullptr;
        Run.m_bFin = false;
        Run.m_bRunRemove = false;

        if (m_bListEntityOffset_Or_DirectOffset == 3)
        {
            ZGeomEventList::m_pGeomEventListBuffers->InitValueRun(Run, m_Control_Routine_List_Entity_Id);
        }
        else if (m_bListEntityOffset_Or_DirectOffset == 1)
        {
            Run.m_bFin = true;
        }
    }
    
    uint32_t ZGeomEventList::GetValueNr(uint32_t iValue_Nr)
    {
        if (m_bListEntityOffset_Or_DirectOffset == 1)
        {
            return 0;
        }

        if (m_bListEntityOffset_Or_DirectOffset == 2)
        {
            if (!iValue_Nr)
            {
                return m_iControl_Routine_Direct_Id.GetVal();
            }
        }
        else if (m_bListEntityOffset_Or_DirectOffset == 3)
        {
            return ZGeomEventList::m_pGeomEventListBuffers->Get_Value_Nr(m_Control_Routine_List_Entity_Id, iValue_Nr);
        }

        return 0;
    }

    uint32_t ZGeomEventList::GetValueFromValueRun(ZGeomEventListBuffers::ValueRun& Run)
    {
        if (Run.m_bFin)
        {
            return 0;
        }

        if (m_bListEntityOffset_Or_DirectOffset == 1)
        {
            ZASSERT(m_bListEntityOffset_Or_DirectOffset != 1);
        }

        if (m_bListEntityOffset_Or_DirectOffset == 3)
        {
            return ZGeomEventList::m_pGeomEventListBuffers->GetValueFromValueRun(Run);
        }

        return m_iControl_Routine_Direct_Id.GetVal();
    }
    
    bool ZGeomEventList::Exists(uint32_t iValue)
    {
        if (m_bListEntityOffset_Or_DirectOffset == 1)
        {
            return false;
        }

        if (m_bListEntityOffset_Or_DirectOffset == 2)
        {
            return m_iControl_Routine_Direct_Id.GetVal() == iValue;
        }

        return ZGeomEventList::m_pGeomEventListBuffers->Exists(m_Control_Routine_List_Entity_Id, iValue);
    }
    
    uint32_t ZGeomEventList::Count() const
    {
        if (m_bListEntityOffset_Or_DirectOffset == 1)
        {
            return 0;
        }
        else if (m_bListEntityOffset_Or_DirectOffset == 2)
        {
            return 1;
        }
        else
        {
            ZGeomEventListBuffers::SGeomEventListBufferEntity_Id bufferEntity { m_Control_Routine_List_Entity_Id };
            return ZGeomEventList::m_pGeomEventListBuffers->Get_List_Size(bufferEntity);
        }
    }
    
    void ZGeomEventList::Clear()
    {
        if (m_bListEntityOffset_Or_DirectOffset == 3)
        {
            ZGeomEventListBuffers::SGeomEventListBufferEntity_Id sEntityId { m_Control_Routine_List_Entity_Id };
            ZGeomEventList::m_pGeomEventListBuffers->Free_All(sEntityId);
        }

        m_iControl_Routine_Direct_Id.SetVal(0u);
        m_bListEntityOffset_Or_DirectOffset = 1;
    }
    
    bool ZGeomEventList::ChkEvents() const
    {
        return m_bListEntityOffset_Or_DirectOffset != 1;
    }
    
    void ZGeomEventList::Add(uint32_t iValue)
    {
        if (Exists(iValue))
        {
            return;
        }

        switch (m_bListEntityOffset_Or_DirectOffset)
        {
            case 1:
                m_iControl_Routine_Direct_Id.SetVal(iValue);
                m_bListEntityOffset_Or_DirectOffset = 2;
                break;

            case 2:
            {
                uint32_t iOldValue = m_iControl_Routine_Direct_Id.GetVal();
                ZGeomEventListBuffers::SGeomEventListBufferEntity_Id Entity_Id = ZGeomEventList::m_pGeomEventListBuffers->Alloc_Value(iOldValue);
                m_Control_Routine_List_Entity_Id = Entity_Id;
                ZGeomEventList::m_pGeomEventListBuffers->Alloc_Link_Value(m_Control_Routine_List_Entity_Id, iValue);
                m_bListEntityOffset_Or_DirectOffset = 3;
                break;
            }

            case 3:
                ZGeomEventList::m_pGeomEventListBuffers->Alloc_Link_Value(m_Control_Routine_List_Entity_Id, iValue);
                break;

            default:
                ZASSERT(false);
                break;
        }
    }

    
    void ZGeomEventList::Create_Geom_Event_Buffer_Lists()
    {
        ZGeomEventList::m_pGeomEventListBuffers = ZUniMemory::New<ZGeomEventListBuffers>();
    }

    void ZGeomEventList::Destroy_Geom_Event_Buffer_Lists()
    {
        if (ZGeomEventList::m_pGeomEventListBuffers)
        {
            ZUniMemory::Delete(ZGeomEventList::m_pGeomEventListBuffers);
        }

        ZGeomEventList::m_pGeomEventListBuffers = nullptr;
    }
    
    STATIC_CLASS_VAR_IMPL(ZGeomEventList, ZGeomEventListBuffers*, m_pGeomEventListBuffers, 0x00972974, nullptr);
}
