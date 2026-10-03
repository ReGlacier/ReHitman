#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    struct SGeomEvent_Direct_Id
    {
        // members
        uint8_t m_iDirect_Value[3];

        // methods
        void SetVal(uint32_t iValue) 
        {
            ZASSERT((iValue & 0xFF000000) == 0);

            m_iDirect_Value[0] = static_cast<uint8_t>(iValue & 0xFF);
            m_iDirect_Value[1] = static_cast<uint8_t>((iValue >> 8) & 0xFF);
            m_iDirect_Value[2] = static_cast<uint8_t>((iValue >> 16) & 0xFF);
        }

        uint32_t GetVal() const 
        { 
            return 
                (static_cast<uint32_t>(m_iDirect_Value[0]))       | 
                (static_cast<uint32_t>(m_iDirect_Value[1]) << 8)  | 
                (static_cast<uint32_t>(m_iDirect_Value[2]) << 16);  
        }
    };

    struct ZGeomEventListBuffers
    {
        // constatns
        static constexpr size_t MAX_NR_BUFFERS = 256;

        // types
        struct SGeomEventListBufferEntity_Id
        {
            uint8_t m_iBuffer_Id;
            uint8_t m_iEntity_Id;
        };

        struct SGeomEventListBufferEntity_Used
        {
            SGeomEvent_Direct_Id m_iValue;
            SGeomEventListBufferEntity_Id m_Next_In_List;
        };

        struct SGeomEventListBufferEntity_Free
        {
            uint8_t m_iPrev_Free;
            uint8_t m_iNext_Free;
        };

        struct SGeomEventListBufferEntity
        {
            union
            {
                SGeomEventListBufferEntity_Used m_GeomEventListBufferEntity_Used;
                SGeomEventListBufferEntity_Free m_GeomEventListBufferEntity_Free;
            };
        };

        struct ValueRun
        {
            SGeomEventListBufferEntity *m_pEntity;
            SGeomEventListBufferEntity_Id m_Entity_Id;
            bool m_bFin;
            bool m_bRunRemove;
        };

        struct ZGeomEventListBuffer
        {
            // constants
            static constexpr uint32_t NO_ENTITY_ID = 0;
            static constexpr uint32_t MAX_NR_ENTRIES = 255;
            
            // members
            uint8_t m_iBuffer_Id;
            uint8_t m_iFirst_Free_Entity;
            uint8_t m_iNr_Free_Entities;
            SGeomEventListBufferEntity m_Buffer_Entities[MAX_NR_ENTRIES];

            SGeomEventListBufferEntity* Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id);
            const SGeomEventListBufferEntity* Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id) const;
            uint8_t Alloc_Entity();
            bool Free_Entity(uint8_t iEntity_Id);
        };

        // methods
        ZGeomEventListBuffers();
        ~ZGeomEventListBuffers();
        uint32_t Get_List_Size(const SGeomEventListBufferEntity_Id& First_Entity_Id) const;
        SGeomEventListBufferEntity* Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id);
        const SGeomEventListBufferEntity* Get_Buffer_Entity(const SGeomEventListBufferEntity_Id& Entity_Id) const;
        void Free_All(const SGeomEventListBufferEntity_Id& First_Entity_Id);
        bool Free_Entity(const SGeomEventListBufferEntity_Id& Entity_Id);
        void Free_Buffer(ZGeomEventListBuffer* pBuffer);
        void Free_Buffer_Id(uint8_t iBuffer_Id);
        uint32_t Get_Value(const SGeomEventListBufferEntity_Id& Entity_Id) const;
        void Set_Value(const SGeomEventListBufferEntity_Id& Entity_Id, uint32_t iValue);
        SGeomEventListBufferEntity_Id Alloc_Value(uint32_t iValue);
        SGeomEventListBufferEntity_Id Alloc_Link_Value(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue);
        SGeomEventListBufferEntity_Id Alloc_Entity();
        uint8_t Get_Buffer_With_Space();
        ZGeomEventListBuffer* Alloc_Buffer();
        uint8_t New_Buffer_Id();
        void NextValueRun(ValueRun& Run);
        void InitValueRun(ValueRun& Run, const SGeomEventListBufferEntity_Id& First_Entity_Id);
        uint32_t GetValueFromValueRun(ValueRun& Run) const;
        uint32_t Get_Value_Nr(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue_Nr) const;
        bool Exists(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue) const;
        void Link_Entity_At_End(const SGeomEventListBufferEntity_Id& First_Entity_Id, const SGeomEventListBufferEntity_Id& New_Entity_Id);
        SGeomEventListBufferEntity_Id Free_Value(const SGeomEventListBufferEntity_Id& First_Entity_Id, uint32_t iValue);

        // members
        uint8_t m_Free_Buffer_Ids[MAX_NR_BUFFERS];
        uint16_t m_iNr_Free_Buffer_Ids;
        uint16_t m_iLast_Added_To_Buffer_Id;
        ZGeomEventListBuffer *m_GeomEventListBuffers[MAX_NR_BUFFERS];
    };

    struct ZGeomEventList
    {
        uint8_t m_bListEntityOffset_Or_DirectOffset;
        union 
        {
            ZGeomEventListBuffers::SGeomEventListBufferEntity_Id m_Control_Routine_List_Entity_Id;
            SGeomEvent_Direct_Id m_iControl_Routine_Direct_Id;
        };

        // methods
        ZGeomEventList();
        ~ZGeomEventList();
       
        void RunRemoveEvent(ZGeomEventListBuffers::ValueRun& Run);
        void Remove(uint32_t iValue);
        void NextValueRun(ZGeomEventListBuffers::ValueRun& Run);
        void InitValueRun(ZGeomEventListBuffers::ValueRun& Run);
        uint32_t GetValueNr(uint32_t iValue_Nr);
        uint32_t GetValueFromValueRun(ZGeomEventListBuffers::ValueRun &);
        bool Exists(uint32_t iValue);
        uint32_t Count() const;
        void Clear();
        bool ChkEvents() const;
        void Add(uint32_t iValue);
        
        static void Create_Geom_Event_Buffer_Lists();
        static void Destroy_Geom_Event_Buffer_Lists();
        
        // static
        STATIC_CLASS_VAR(ZGeomEventList, ZGeomEventListBuffers*, m_pGeomEventListBuffers);
    };
    RE_VERIFY_SIZE(ZGeomEventList, 0x4);
}
