#include <Glacier/ZBaseConRout.h>
#include <G1ConfigurationService.h>


namespace Glacier
{
	ZBaseConRoutTypeInfo** ZBaseConRout::GetFactory()
	{
		return ((ZBaseConRoutTypeInfo**(__stdcall*)())G1ConfigurationService::G1API_FunctionAddress_ZBaseConRout_GetFactory)();
	}
}