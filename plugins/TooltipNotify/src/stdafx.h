// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500

// Windows header files
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

// C/C++ header files
#include <cassert>
#include <algorithm>
#include <vector>

// Miranda IM header files
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_database.h>
#include <m_clui.h>
#include <m_ignore.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_clc.h>
#include <m_fontservice.h>

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))