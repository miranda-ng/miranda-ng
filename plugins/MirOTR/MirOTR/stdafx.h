// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Teile der Windows-Header nicht einbinden.

// Windows-Headerdateien:
#include <windows.h>
#include <string.h>
#include <assert.h>

#ifdef _DEBUG
	#define DEBUGOUTA(x)	OutputDebugStringA(x);
	#define DEBUGOUT(x)		OutputDebugString(x);
	#define DEBUGOUT_T(x)	OutputDebugString(__T(x));
#else
	#define DEBUGOUTA(x);
	#define DEBUGOUT(x)		
	#define DEBUGOUT_T(x)	
#endif
#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

//include
#include <newpluginapi.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_message.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_popup.h>
#include <m_contacts.h>
#include <m_ignore.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <m_skin.h>

//ExternalAPI
#include <m_folders.h>
#include <m_updater.h>
#include <m_msg_buttonsbar.h>
#include <m_metacontacts.h>

#include <gcrypt.h>
extern "C" {
	#include <privkey.h>
	#include <proto.h>
	#include <tlv.h>
	#include <message.h>
	#include <userstate.h>
}

#include "dllmain.h"
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

// modified manual policy - so that users set to 'opportunistic' will automatically start OTR with users set to 'manual'
#define OTRL_POLICY_MANUAL_MOD		(OTRL_POLICY_MANUAL | OTRL_POLICY_WHITESPACE_START_AKE | OTRL_POLICY_ERROR_START_AKE)

// {030F37D6-DD32-434d-BC64-5B6541EB9299}
static const MUUID MIID_OTR = { 0x30f37d6, 0xdd32, 0x434d, { 0xbc, 0x64, 0x5b, 0x65, 0x41, 0xeb, 0x92, 0x99 } };
#ifdef _UNICODE
	// {12D8FAAD-78AB-4e3c-9854-320E9EA5CC9F}
	static const MUUID MIID_OTRPLUGIN = { 0x12d8faad, 0x78ab, 0x4e3c, { 0x98, 0x54, 0x32, 0xe, 0x9e, 0xa5, 0xcc, 0x9f } };
#else
	// {16E2E7B0-D398-4ea8-A5CC-A0F3689C7608}
	static const MUUID MIID_OTRPLUGIN = { 0x16e2e7b0, 0xd398, 0x4ea8, { 0xa5, 0xcc, 0xa0, 0xf3, 0x68, 0x9c, 0x76, 0x8 } };
#endif

#define MODULENAME "MirOTR"

#define PREF_BYPASS_OTR 0x8000
#define PREF_NO_HISTORY  0x10000

#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
