#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <shlwapi.h>

#include <malloc.h>
#include <time.h>

#include <newpluginapi.h>

#include <m_options.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_popup.h>
#include <m_file.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_string.h>
#include <m_msg_buttonsbar.h>

#include <m_protoint.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <m_netlib.h>

#include <m_dropbox.h>

#include "version.h"
#include "resource.h"

#define MODULE "Dropbox"

extern HINSTANCE g_hInstance;

#include "dropbox.h"

// icons
void   InitializeIcons();
HANDLE GetIconHandle(int iconId);
HICON  LoadIconEx(int iconId, bool big = false);

#endif //_COMMON_H_