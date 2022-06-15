/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

void UnloadLangPackModule(void);

int  InitialiseModularEngine(void);
void DestroyModularEngine(void);

int  InitPathUtils(void);

extern HINSTANCE g_hInst;
extern HWND hAPCWindow;
extern HANDLE hThreadQueueEmpty;
extern HCURSOR g_hCursorNS, g_hCursorWE;
extern bool g_bEnableDpiAware;

/////////////////////////////////////////////////////////////////////////////////////////
// modules.cpp

struct THookSubscriber
{
	HINSTANCE hOwner;
	int type;
	union {
		struct {
			union {
				MIRANDAHOOK pfnHook;
				MIRANDAHOOKPARAM pfnHookParam;
				MIRANDAHOOKOBJ pfnHookObj;
				MIRANDAHOOKOBJPARAM pfnHookObjParam;
			};
			void* object;
			LPARAM lParam;
		};
		struct {
			HWND hwnd;
			UINT message;
		};
	};
};

#define HOOK_SECRET_SIGNATURE 0xDEADBABA

struct THook : public MZeroedObject
{
	char name[MAXMODULELABELLENGTH];
	int  id;
	int  subscriberCount;
	THookSubscriber* subscriber;
	MIRANDAHOOK pfnHook;
	uint32_t secretSignature = HOOK_SECRET_SIGNATURE;
	mir_cs csHook;
};

extern LIST<CMPluginBase> pluginListAddr;

/////////////////////////////////////////////////////////////////////////////////////////
// langpack.cpp

char* LangPackTranslateString(const MUUID *pUuid, const char *szEnglish, const int W);

/////////////////////////////////////////////////////////////////////////////////////////
// miranda.cpp

EXTERN_C MIR_CORE_DLL(void) BeginMessageLoop(void);
EXTERN_C MIR_CORE_DLL(void) EnterMessageLoop(void);
EXTERN_C MIR_CORE_DLL(void) LeaveMessageLoop(void);

/////////////////////////////////////////////////////////////////////////////////////////
// threads.cpp

extern uint32_t mir_tls;

/////////////////////////////////////////////////////////////////////////////////////////
// utils.cpp

typedef BOOL(MIR_SYSCALL *PGENRANDOM)(void*, uint32_t);
extern PGENRANDOM pfnRtlGenRandom;
