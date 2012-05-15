#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

// Windows Header Files
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <string.h>
#include <winsock.h>
#include <string>
#include <winbase.h>

//Miranda SDK headers
#include "newpluginapi.h"
#include "m_clist.h"
#include "m_clui.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_protomod.h"
#include "m_database.h"
#include "m_system.h"
#include "m_protocols.h"
#include "m_userinfo.h"
#include "m_options.h"
#include "m_protosvc.h"
#include "m_utils.h"
#include "m_ignore.h"
#include "m_clc.h"
#include <m_variables.h>

#include "..\Resource.h"
#include "..\Version.h"

#define protocolname		"SimpleAutoReplier" //no spaces here :)

#define KEY_ENABLED			"Enabled"
#define KEY_HEADING			"Heading"
#define KEY_REPEATINTERVAL	"RepeatInterval"

//General
extern HINSTANCE hinstance;
extern BOOL fEnabled;
extern INT interval;
extern TCHAR* ptszDefaultMsg[6];

extern INT OptInit(WPARAM wParam,LPARAM lParam);
