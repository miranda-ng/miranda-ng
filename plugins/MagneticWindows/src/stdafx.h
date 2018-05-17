#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_message.h>
#include <m_system.h>
#include <m_options.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_MagneticWindows.h>

#include "SnapToListService.h"
#include "Options.h"

#include "resource.h"
#include "Version.h"

struct TWorkingVariables
{
	int MouseX, MouseY;
	bool SnappedX, SnappedY;
};

#define MODULE_NAME "MagneticWindows"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin() :
		PLUGIN<CMPlugin>(MODULE_NAME)
	{}
};

void WindowStart();
bool WindowOpen(HWND);
bool WindowClose(HWND);
void WindowCloseAll();
