#pragma once

#include <windows.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_string.h>

#include <m_variables.h>

#include "Resource.h"
#include "version.h"

#define protocolname		"SimpleAutoReplier" //no spaces here :)

#define KEY_ENABLED			"Enabled"
#define KEY_HEADING			"Heading"
#define KEY_REPEATINTERVAL	"RepeatInterval"

// General

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin() :
		PLUGIN<CMPlugin>(protocolname)
	{}
};

extern INT interval;
extern wchar_t *ptszDefaultMsg[6];
extern HGENMENU hEnableMenu;
extern IconItem iconList[];

extern INT OptInit(WPARAM wParam, LPARAM lParam);
