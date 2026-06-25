#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <Glacier/Geom/ZBaseGeomLists.h>
#include <Glacier/Geom/ZEntityLocator.h> // ZBaseGeom
#include <Glacier/ZSTL/ZFixedSizeMemoryManager.h>
#include <Glacier/ZSTL/ZRTTI.h>

namespace Glacier
{
    struct SGeomResourcesHeader
    {
        uint32_t m_lId;
        uint32_t m_lGeomResourcesStart;
        uint32_t m_lNrGeomResources;
    };
    RE_VERIFY_SIZE(SGeomResourcesHeader, 0xC);

    struct SGeomResources
    {
        uint32_t m_lGeomType;
        uint32_t m_lNrUnusedGeoms;
    };
    RE_VERIFY_SIZE(SGeomResources, 0x8);

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
        const uint32_t m_MaxNumberOfBaseGeoms;
        ZFixedSizeMemoryManager<ZBaseGeom>* m_BaseGeomMemoryManager;
        ZQElemsBuffer* m_pExtraGeomElems;
        ZBaseGeomLists* m_pBaseGeomLists;
        SGeomResourcesHeader* m_pGeomResourcesHeaders;
        SGeomResources* m_pGeomResources;
        uint32_t m_lNrGeomResourcesHeaders;
        uint32_t m_lNrResources;
        bool m_bGeomCreationLock;
        bool padding[3];
        ZBaseGeomRoomList* m_pRoomListsBuffer;
        uint16_t* m_pFreeRoomLists;
        uint32_t m_lNrRoomLists;
        uint32_t m_lMaxTotalNrBaseGeoms;
    }; // Verified size is 0x3C
    RE_VERIFY_SIZE(ZGeomBuffer, 0x3C); // verified
}