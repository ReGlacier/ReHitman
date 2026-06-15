#include <Glacier/Geom/ZGeomBuffer.h>
#include <G1ConfigurationService.h>


namespace Glacier
{
	uint32_t ZGeomBuffer::GeomPtrToRef(ZGEOM* geom)
	{
		return ((uint32_t(__thiscall*)(ZGeomBuffer*, ZGEOM*))(G1ConfigurationService::G1API_FunctionAddress_ZGeomBuffer_GeomPtrToRef_ZGEOM))(this, geom);
	}

	uint32_t ZGeomBuffer::GeomPtrToRef(ZEntityLocator* geomBase)
	{
		return ((uint32_t(__thiscall*)(ZGeomBuffer*, ZEntityLocator*))(G1ConfigurationService::G1API_FunctionAddress_ZGeomBuffer_GeomPtrToRef_ZGeomBase))(this, geomBase);
	}
}