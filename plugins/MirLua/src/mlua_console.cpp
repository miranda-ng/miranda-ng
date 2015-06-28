#include "stdafx.h"

BOOL WINAPI ConsoleHandler(DWORD)
{
	return TRUE;
}

CMLuaConsole::CMLuaConsole(lua_State *L)
	: L(L), hConsole(NULL)
{
	if (db_get_b(NULL, MODULE, "ShowConsole", 0))
	{
		if (!AttachConsole(ATTACH_PARENT_PROCESS))
		{
			if (AllocConsole())
			{
				SetConsoleTitle(_T("MirLua Console"));
				freopen("CONOUT$", "w", stdout);
				hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
				if (HWND hConsoleWindow = GetConsoleWindow())
				{
					HMENU hConsoleMenu = GetSystemMenu(hConsoleWindow, FALSE);
					DeleteMenu(hConsoleMenu, SC_CLOSE, MF_BYCOMMAND);
				}
				SetConsoleCtrlHandler(ConsoleHandler, true);
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