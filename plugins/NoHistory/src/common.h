#define MIRANDA_VER 0x800

#ifndef _COMMON_INC
#define _COMMON_INC

#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0500

#include <stdio.h>

#include <windows.h>
#include <commctrl.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_options.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_extraicons.h>

#include <m_utils.h>

#define MODULE						"NoHistory"
#define DBSETTING_REMOVE			"RemoveHistory"

extern HINSTANCE hInst;

#ifndef MIID_NOHISTORY
#define MIID_NOHISTORY	{0xe9ba130e, 0x9927, 0x4469, { 0xb6, 0x19, 0x95, 0x61, 0xa8, 0xc0, 0x57, 0xa7}}
#endif

#endif


void SrmmMenu_Load();
void SrmmMenu_Unload();


