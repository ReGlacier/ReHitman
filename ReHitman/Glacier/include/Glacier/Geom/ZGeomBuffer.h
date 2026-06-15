#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZRTTI.h>

namespace Glacier
{
    // Class definition ZGeomBuffer
    class ZGeomBuffer // total size is 0x3C
    {
    public:
        // vftable
        virtual void Release(bool);
        virtual void PreSave(ISerializerStream&);
        virtual void PostSave(ISerializerStream&);
        virtual void PreLoad(ISerializerStream&);
        virtual bool PostLoad(ISerializerStream&);
        virtual bool PostProcess(unsigned int, unsigned int);
        virtual void LoadSave(ISerializerStream&, bool);
        virtual void LoadObject(IInputSerializerStream&);
        virtual void SaveObject(IOutputSerializerStream&);
        virtual void ExchangeObject(ISerializerStream&);
        virtual void SetToDefault();
        virtual unsigned int GetTypeID();

        // member methods
        uint32_t GeomPtrToRef(ZGEOM*);
        uint32_t GeomPtrToRef(ZEntityLocator*);

        // data
        ZEventBuffer* m_pEventBuffer;
        unsigned int m_MaxNumberOfBaseGeoms;
        void* m_BaseGeomMemoryManager;
        void* m_pExtraGeomElems;
        void* m_pBaseGeomLists;
        void* m_pGeomResourcesHeaders;
        void* m_pGeomResources;
        int m_lNrGeomResourcesHeaders;
        int m_lNrResources;
        bool m_bGeomCreationLock;
        bool padding[3];
        void* m_pRoomListsBuffer;
        void* m_pFreeRoomLists;
        int m_lNrRoomLists;
        int m_lMaxTotalNrBaseGeoms;
    }; //End of ZGeomBuffer from 00550E38
}