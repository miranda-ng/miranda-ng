#include <Windows.h>

#include "newpluginapi.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
};

static BOOL WINAPI RawDllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_plugin.setInst(hInstance);
	return TRUE;
}

extern "C" _pfnCrtInit _pRawDllMain = &RawDllMain;
