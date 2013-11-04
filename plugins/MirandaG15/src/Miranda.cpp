
/*
 * Miranda IM LCD Plugin
 * Displays incoming text messages on an LCD.
 *
 * Copyright (c) 2003 Martin Rubli, mrubli@gmx.net
 *
 ******************************************************************************
 * This file is part of Miranda IM LCD Plugin.
 *
 * Miranda IM LCD Plugin is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * Miranda IM LCD Plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Miranda IM LCD Plugin; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ******************************************************************************
 *
 * Miranda.cpp: Miranda plugin initialisation
 */

#include "StdAfx.h"

#ifdef _DEBUG
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
#endif

#include "CAppletManager.h"
#include "CConfig.h"

#include "m_system.h"

// SETTINGS
#define LCD_FPS 10

#ifdef _WIN64
	#pragma comment(lib, "src\\LCDFramework\\hid\\x64\\hid.lib")
	#pragma comment(lib, "src\\LCDFramework\\g15sdk\\lib\\x64\\lgLcd.lib")
#else
	#pragma comment(lib, "src\\LCDFramework\\hid\\hid.lib")
	#pragma comment(lib, "src\\LCDFramework\\g15sdk\\lib\\x86\\lgLcd.lib")
#endif

//************************************************************************
// Variables
//************************************************************************
bool g_bInitialized;
// AppletManager object
 CAppletManager* g_AppletManager;

// Plugin Information

HINSTANCE hInstance;
int hLangpack;

// {58D63981-14C1-4099-A3F7-F4FAA4C8FC59}
#define MIID_G15APPLET	{ 0x58d63981, 0x14c1, 0x4099, { 0xa3, 0xf7, 0xf4, 0xfa, 0xa4, 0xc8, 0xfc, 0x59 } }

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_G15APPLET, MIID_LAST};
	
PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {798221E1-E47A-4dc8-9077-1E576F9C4307}
	{0x798221e1, 0xe47a, 0x4dc8, {0x90, 0x77, 0x1e, 0x57, 0x6f, 0x9c, 0x43, 0x7}}
};

// Function Prototypes
int Init(WPARAM,LPARAM);
void UnInit();

//************************************************************************
// Exported Functions
//************************************************************************
extern "C" {
	__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
	{
		return &pluginInfoEx;
	}
	
	// Called by Miranda to load the plugin.
	// We defer initialization until Miranda's module loading process completed and return 0 to
	// mark success, everything else will cause the plugin to be freed right away.
	int __declspec(dllexport) Load()
	{
		g_bInitialized = false;
		InitDebug();
		TRACE(_T("Plugin loaded\n"));
		// Schedule actual initialization for later
		HookEvent(ME_SYSTEM_MODULESLOADED, Init);
		return 0;
	}

	// Called by Miranda when the plugin should unload itself.
	int __declspec(dllexport) Unload(void)
	{
		if(!g_bInitialized) {
			TRACE(_T("ERROR: Unload requested, but plugin is not initialized?!\n"));		
			return 0;
		}
		TRACE(_T("-------------------------------------------\nUnloading started\n"));
		UnInit();
		TRACE(_T("Unloading successful\n"));
		TRACE(_T("Cleaning up: "));
		UnInitDebug();
		TRACE(_T("OK!\n"));
		return 0;
	}
}

//************************************************************************
// DllMain
//
// EntryPoint of the DLL
//************************************************************************
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	return TRUE;
}

//************************************************************************
// Init
//
// Called after Miranda has finished loading all modules
// This is where the main plugin initialization happens and the
// connection to the LCD is established,
//************************************************************************
int Init(WPARAM wParam,LPARAM lParam)
{
	g_AppletManager = new CAppletManager();
	// Memoryleak Detection
	#ifdef _DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		
	#endif
	
	 // set up the LCD context as non-autostart, non-persist, callbacked
	CConfig::Initialize();

	// Initialize the output object
	if(!g_AppletManager->Initialize(toTstring(APP_SHORTNAME)))
	{
		if(CConfig::GetBoolSetting(SKIP_DRIVER_ERROR)) {
			tstring text = _T("Failed to initialize the LCD connection\n Make sure you have the newest Logitech drivers installed (>=1.03).\n");
			tstring title = _T(APP_SHORTNAME);
			MessageBox(NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONEXCLAMATION);
		}

		TRACE(_T("Initialization failed!.\n"));
		return 0;
	}
	
	g_bInitialized = true;
	TRACE(_T("Initialization completed successfully.\n-------------------------------------------\n"));
	return 0;
}
//************************************************************************
// UnInit
//
// Called when the plugin is about to be unloaded
//************************************************************************
void UnInit(void)
{
	g_AppletManager->Shutdown();	
	delete g_AppletManager;
	
//#ifdef _DEBUG
//	_CrtDumpMemoryLeaks();
//#endif
}
