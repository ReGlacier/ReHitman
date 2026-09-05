#include <BloodMoney/Game/Globals.h>

// Macro to speedup codegen
#define RE_MAKE_GLOBAL(type, name, addr) type*& name = *reinterpret_cast<type**>(Hitman::BloodMoney::Globals::addr);

RE_MAKE_GLOBAL(Glacier::ZSysInterfaceWintel, g_pSysInterface, kSysInterfaceAddr);

// End of globals
#undef RE_MAKE_GLOBAL
