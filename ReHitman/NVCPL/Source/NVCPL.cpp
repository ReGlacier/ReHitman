#include <Windows.h>
#include <cstdint>

constexpr std::intptr_t kFreeLibraryCodeAddr = 0x004884D9;
constexpr uint32_t kWaitMillisecondsUntilTargetInited = 2000;
constexpr const char* kTargetDllName = "ReHitman.dll";

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }

extern "C" __declspec(dllexport) bool __cdecl NvCplGetDataInt(int, int*) {
	const DWORD targetDllAttribs = GetFileAttributesA(kTargetDllName);

	if (targetDllAttribs == INVALID_FILE_ATTRIBUTES || (targetDllAttribs & FILE_ATTRIBUTE_DIRECTORY)) {
		MessageBoxA(nullptr, "No ReHitman.dll file found on work directory.\n"
		                     "Please, make sure that you put ReHitman.dll into the same folder to HitmanBloodMoney.exe.\n"
		                     "\n"
		                     "This process will be started in normal mode.", "ReHitman injection failed", MB_ICONERROR | MB_OK);
		return false;
	}

	// Patch executable (remove FreeLibrary call from game executable)
	BYTE patch[7] = {
			0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90
	};

	WriteProcessMemory(GetCurrentProcess(), (LPVOID*)kFreeLibraryCodeAddr, &patch[0], 7, nullptr);

	// Load library
	LoadLibraryA(kTargetDllName);

	// Wait for init
	Sleep(kWaitMillisecondsUntilTargetInited);

	// Return execution order
	return false;
}