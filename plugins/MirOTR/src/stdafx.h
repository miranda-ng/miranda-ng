// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // Selten verwendete Teile der Windows-Header nicht einbinden.

#ifndef WINVER                  // Gibt an, dass Windows 2000 die mindestens erforderliche Plattform ist.
#define WINVER 0x0500           // Ändern Sie den entsprechenden Wert, um auf andere Versionen von Windows abzuzielen.
#endif

#ifndef _WIN32_WINNT            // Gibt an, dass Windows 2000 die mindestens erforderliche Plattform ist.
#define _WIN32_WINNT 0x0500     // Ändern Sie den entsprechenden Wert, um auf andere Versionen von Windows abzuzielen.
#endif

#ifndef _WIN32_IE               // Gibt an, dass Internet Explorer 7.0 die mindestens erforderliche Plattform ist.
#define _WIN32_IE 0x0700        // Ändern Sie den entsprechenden Wert, um auf andere Versionen von IE abzuzielen.
#endif

#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <stack>
#include <gcrypt.h>
extern "C" {
	#include <privkey.h>
	#include <proto.h>
	#include <tlv.h>
	#include <message.h>
	#include <userstate.h>
}
using namespace std;

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_message.h>
#include <m_protomod.h>
#include <m_popup.h>
#include <m_contacts.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <win2k.h>
#include <m_genmenu.h>
#include <m_options.h>

//ExternalAPI
#include <m_folders.h>
#include <m_msg_buttonsbar.h>
#include <m_metacontacts.h>

#include "otr.h"
#include "dbfilter.h"
#include "language.h"
#include "options.h"
#include "utils.h"
#include "svcs_menu.h"
#include "svcs_proto.h"
#include "svcs_srmm.h"
#include "resource.h"
#include "otr.h"
#include "icons.h"
#include "dialogs.h"
#include "version.h"
#include "entities.h"
#include "icons.h"
#include "mirotrmenu.h"
#include "striphtml.h"
#include "ekhtml.h"
#include "svcs_proto.h"

#ifdef _DEBUG
	#define DEBUGOUTA(x)	OutputDebugStringA(x);
	#define DEBUGOUT(x)		OutputDebugString(x);
	#define DEBUGOUT_T(x)	OutputDebugString(__T(x));
#else
	#define DEBUGOUTA(x);
	#define DEBUGOUT(x)		
	#define DEBUGOUT_T(x)	
#endif

// modified manual policy - so that users set to 'opportunistic' will automatically start OTR with users set to 'manual'
#define OTRL_POLICY_MANUAL_MOD		(OTRL_POLICY_MANUAL | OTRL_POLICY_WHITESPACE_START_AKE | OTRL_POLICY_ERROR_START_AKE)

#define MODULENAME "MirOTR"

#define PREF_BYPASS_OTR 0x8000
#define PREF_NO_HISTORY  0x10000

extern HINSTANCE hInst;

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
