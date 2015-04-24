// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <commctrl.h>
#include <cassert>
#include <algorithm>
#include <vector>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_clui.h>
#include <m_ignore.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_clc.h>
#include <m_fontservice.h>
#include <win2k.h>

#define MODULENAME "TooltipNotify"

#include "version.h"
#include "TooltipNotify.h"
#include "DbHelpers.h"
#include "Tooltip.h"
#include "Settings.h"
#include "resource.h"
#include "Utils.h"

extern HINSTANCE g_hInstDLL;