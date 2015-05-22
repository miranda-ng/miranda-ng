/*
    ICQ Corporate protocol plugin for Miranda IM.
    Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "corp.h"

///////////////////////////////////////////////////////////////////////////////

HINSTANCE hInstance;
char protoName[64];
int hLangpack;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE
};

///////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID lpReserved)
{
    hInstance = hModule;
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hModule);
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload()
{
    UnloadServices();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&pluginInfo);

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = protoName;
	pd.type = PROTOTYPE_PROTOCOL;

    char fileName[MAX_PATH];
    WIN32_FIND_DATA findData;

    GetModuleFileName(hInstance, fileName, MAX_PATH);
    FindClose(FindFirstFile(fileName, &findData));
    findData.cFileName[mir_strlen(findData.cFileName) - 4] = 0;
    strcpy(protoName, findData.cFileName);

    CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

    LoadServices();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD Version)
{
    return &pluginInfo;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void T(char *format, ...)
{
    char buffer[8196], bufferTime[64];
    va_list list;
    SYSTEMTIME t;

    va_start(list, format);
    vsprintf(buffer, format, list);
    va_end(list);

    GetLocalTime(&t);
    sprintf(bufferTime, "%.2d:%.2d:%.2d.%.3d ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

    static HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD result;

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hFile = CreateFile("ICQ Corp.log", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
        SetFilePointer(hFile, 0, 0, FILE_END);
    }
    WriteFile(hFile, bufferTime, (DWORD)mir_strlen(bufferTime), &result, NULL);
	WriteFile(hFile, buffer, (DWORD)mir_strlen(buffer), &result, NULL);
}
#endif

///////////////////////////////////////////////////////////////////////////////
