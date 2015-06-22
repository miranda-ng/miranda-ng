#include "stdafx.h"

CMLuaConsole::CMLuaConsole(lua_State *L)
	: L(L), hConsole(NULL)
{
	if (db_get_b(NULL, MODULE, "ShowConsole", 0))
	{
		if (!AttachConsole(ATTACH_PARENT_PROCESS))
		{
			if (AllocConsole())
			{
				freopen("CONOUT$", "w", stdout);
				hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
				SetConsoleTitle(_T("MirLua Console"));
			}
		}
	}
}

CMLuaConsole::~CMLuaConsole()
{
	if (hConsole)
		CloseHandle(hConsole);
	FreeConsole();
}