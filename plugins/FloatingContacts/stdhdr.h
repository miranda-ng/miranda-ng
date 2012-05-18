
#ifndef __STDHDR_H__
#define __STDHDR_H__

// disable security warnings about "*_s" functions
#define _CRT_SECURE_NO_DEPRECATE

// disable warnings about underscore in stdc functions
#pragma warning(disable: 4996)

// Unreferenced formal parameter
#pragma warning(disable: 4100)

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//

#define STRICT

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#pragma warning ( disable : 4201 ) //nonstandard extension used : nameless struct/union
#include <commctrl.h>

#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_message.h>
//#include "../../include/msgs.h"
#include <m_file.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_clc.h>
//#include "../../include/clc.h"
#include <m_clistint.h>
#include <m_hotkeys.h>

#include "bitmap_funcs.h"

#include "fltcont.h"
#include "thumbs.h"
#include "filedrop.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////

#endif	// #ifndef __STDHDR_H__

/////////////////////////////////////////////////////////////////////////////
// End Of File stdhdr.h
