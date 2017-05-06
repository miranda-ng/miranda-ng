#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>

#include <malloc.h>
#include <time.h>

#include <map>

#include <newpluginapi.h>

#include <m_options.h>
#include <m_database.h>
#include <m_netlib.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_popup.h>
#include <m_file.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_string.h>
#include <m_gui.h>
#include <m_chat.h>
#include <m_http.h>
#include <m_json.h>
#include <m_metacontacts.h>
#include <m_protoint.h>
#include <m_protosvc.h>

#include <m_dropbox.h>

#include "version.h"
#include "resource.h"
#include "options.h"

extern HINSTANCE hInstance;
extern HNETLIBUSER hNetlibConnection;

class Exception
{
	CMStringA message;

public:
	Exception(const char *message) :
		message(message)
	{
	}

	const char* what() const throw()
	{
		return message.c_str();
	}
};

#define MODULE "CloudFile"

#define FILE_CHUNK_SIZE 1024 * 1024 //1 MB

#include "http_request.h"
#include "file_transfer.h"

// services
#include "cloud_service.h"
#include "oauth.h"
#include "Services\dropbox_service.h"
#include "Services\google_service.h"
#include "Services\microsoft_service.h"
#include "Services\yandex_service.h"
extern LIST<CCloudService> Services;
void InitServices();

// events
int OnModulesLoaded(WPARAM, LPARAM);
int OnProtoAck(WPARAM, LPARAM);
int OnFileDialogCanceled(void* obj, WPARAM hContact, LPARAM);

// icons
void InitializeIcons();
HANDLE GetIconHandle(int iconId);
HANDLE GetIconHandle(const char *name);
HICON LoadIconEx(int iconId, bool big = false);

// menus
extern HGENMENU hContactMenu;
void InitializeMenus();
int OnPrebuildContactMenu(WPARAM, LPARAM);

// srmm
#define BBB_ID_FILE_SEND 10001
int OnSrmmToolbarLoaded(WPARAM, LPARAM);
int OnSrmmWindowOpened(WPARAM, LPARAM);
int OnSrmmButtonPressed(WPARAM, LPARAM);

// options
int OnOptionsInitialized(WPARAM wParam, LPARAM);

// transfers
extern LIST<FileTransferParam> Transfers;
INT_PTR ProtoSendFile(void *obj, WPARAM, LPARAM lParam);
INT_PTR ProtoSendFileInterceptor(void *obj, WPARAM wParam, LPARAM lParam);
INT_PTR ProtoCancelFile(WPARAM, LPARAM lParam);
UINT UploadAndReportProgressThread(void *owner, void *arg);

// utils
void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact = NULL);
void ShowNotification(const wchar_t *message, int flags, MCONTACT hContact = NULL);
MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD flags, DWORD cbBlob, PBYTE pBlob);
bool CanSendToContact(MCONTACT hContact);
void SendToContact(MCONTACT hContact, const wchar_t *data);
void PasteToInputArea(MCONTACT hContact, const wchar_t *data);
void PasteToClipboard(const wchar_t *data);

#endif //_COMMON_H_