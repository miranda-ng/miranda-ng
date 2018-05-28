#include "stdafx.h"

HNETLIBUSER g_hNetlib = nullptr;

void LoadNetlib()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szDescriptiveName.a = MODULENAME;
	nlu.szSettingsModule = MODULENAME;
	g_hNetlib = Netlib_RegisterUser(&nlu);
}

void UnloadNetlib()
{
	if (g_hNetlib) {
		Netlib_CloseHandle(g_hNetlib);
		g_hNetlib = nullptr;
	}
}