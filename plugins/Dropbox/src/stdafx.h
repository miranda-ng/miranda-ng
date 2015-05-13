#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>

#include <malloc.h>
#include <time.h>

#include <newpluginapi.h>

#include <m_options.h>
#include <m_database.h>
#include <m_netlib.h>
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
#include <m_gui.h>

#include <m_protoint.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <m_dropbox.h>

#include "version.h"
#include "resource.h"

class CDropbox;

#define DROPBOX_API_VER "1"
#define DROPBOX_API_ROOT "sandbox"
#define DROPBOX_WWW_URL "https://www.dropbox.com/"
#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_APP_KEY "fa8du7gkf2q8xzg"
#include "..\..\..\miranda-private-keys\Dropbox\secret_key.h"

#include "dropbox_dialogs.h"
#include "dropbox_options.h"
#include "http_request.h"
#include "api\account.h"
#include "api\upload.h"
#include "api\operations.h"
#include "file_transfer.h"
#include "dropbox.h"

#define MODULE "Dropbox"

extern HINSTANCE g_hInstance;

#define DROPBOX_FILE_CHUNK_SIZE 4 * 1024 * 1024 //4 MB

#define BBB_ID_FILE_SEND 10001

// icons
void   InitializeIcons();
HANDLE GetIconHandle(int iconId);
HICON  LoadIconEx(int iconId, bool big = false);

#endif //_COMMON_H_