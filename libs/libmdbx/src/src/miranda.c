#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../config.h"
#include "../mdbx.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	mdbx_dll_handler(hModule, ul_reason_for_call, lpReserved);
	return TRUE;
}
