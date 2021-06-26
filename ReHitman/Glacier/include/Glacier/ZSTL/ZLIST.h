#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Glacier
{
    class MeshNrs; // Unused in release?

    class ZLIST : public ZGEOM
    {
    public:
        /// === data ===
        REFTAB* m_entries;  //+0x10

        /// === vftable ===
        virtual void SendCommandToList(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual void AddGeom(unsigned int);
        virtual void AddGeom(ZGEOM*);
        virtual void RemoveGeomById(unsigned int);
        virtual void RemoveGeomByPtr(ZGEOM*);
        virtual void AddMeshNrs(MeshNrs*);
        virtual int  Script_GetRefNr(int);
        virtual int  Script_GetCount();

        /// === helpers ===
        template <typename T> T* At(size_t index) {
            if (index >= m_entries->Count()) {
                return nullptr;
            }
            return reinterpret_cast<T*>(m_entries->operator[](index));
        }
    };
}