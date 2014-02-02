#pragma once


// Windows headers
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <commctrl.h>
#include <assert.h>



// Miranda headers
//LotusNotify.h
#include <m_core.h>
#include <win2k.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_system.h>
//debug.h 
#include <m_netlib.h>



// Notesapi headers
#define W32
#if defined(_WIN64)
#define _AMD64_
#endif
#include "cnotesapi\include\global.h"
#include "cnotesapi\include\osmisc.h"
#include "cnotesapi\include\nsfdb.h"
#include "cnotesapi\include\nsfsearc.h"
#include "cnotesapi\include\names.h"
#include "cnotesapi\include\osenv.h"
#include "cnotesapi\include\extmgr.h"
#include "cnotesapi\include\bsafeerr.h"
#include "cnotesapi\include\nsferr.h"


