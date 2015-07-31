#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <winsock.h>
#include <Shlobj.h>
#include <commdlg.h>
#include <windowsx.h>
#include <time.h>
#include <string>
#include <map>
#include <list>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_xml.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_chat.h>
#include <m_http.h>
#include <m_string.h>

#include <m_msg_buttonsbar.h>

#include "version.h"
#include "resource.h"

#include "http_request.h"
#include "paste_to_web.h"
#include "pastebin_com.h"
#include "wklej_to.h"
#include "privnote_com.h"

#include "options.h"

extern HINSTANCE hInst;
extern HANDLE g_hNetlibUser;
extern PasteToWeb* pasteToWebs[PasteToWeb::pages];

#define MODULE				"PasteIt"

#include "..\..\..\miranda-private-keys\PasteIt\pastebin.h"

#endif //_COMMON_H_