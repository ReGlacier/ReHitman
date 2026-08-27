#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    class MeshNrs; // Unused in release?

    class ZLIST : public ZGEOM
    {
    public:
        // static
        STATIC_CLASS_VAR(ZLIST, ZLIST*, m_TrackLinkObjectsInstance);

        // RTTI
        DECLARE_GEOM_CLASS(ZLIST, 0x800001Au);

        // vtbl
        ~ZLIST() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget) override;
        bool CheckPointInside(ZVector3& pPoint, float fDotDist) override;

        // ZLIST
        virtual void SendCommandToList(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual void AddGeom(ZREF rGeom);
        virtual void AddGeom(ZGEOM* pGeom);
        virtual void RemoveGeomById(ZREF rGeom);
        virtual void RemoveGeomByPtr(ZGEOM* pGeom);
        virtual void AddMeshNrs(MeshNrs*);
        virtual int  Script_GetRefNr(int lNr);
        virtual int  Script_GetCount();

        template <typename T> T* At(size_t index)
        {
            if (index >= m_pZList->Count()) {
                return nullptr;
            }
            return reinterpret_cast<T*>(m_pZList->operator[](index));
        }

#       pragma region " --- RTTI Methods --- "
        void GetGeomList(REFTAB32& rRefTab);
        void SetGeomList(const REFTAB32& rRefTab);
#       pragma endregion

        // methods
        ZLIST(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        REFTAB* m_pZList;  //+0x10
    };
}
