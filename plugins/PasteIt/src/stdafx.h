// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
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
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_xml.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_chat_int.h>

#include "PasteToWeb.h"
#include "PasteToWeb1.h"
#include "PasteToWeb2.h"
#include "version.h"
#include "resource.h"
#include "Options.h"

extern PasteToWeb* pasteToWebs[PasteToWeb::pages];

#define MODULENAME "PasteIt"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
