#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>

#include <malloc.h>
#include <time.h>

#include <map>
#include <algorithm>

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
#include <m_gui.h>
#include <m_chat.h>
#include <m_http.h>
#include <m_json.h>
#include <m_metacontacts.h>
#include <m_protoint.h>
#include <m_protosvc.h>
#include <m_contacts.h>

#include <m_cloudfile.h>

#include "version.h"
#include "resource.h"

class CCloudService;

#include "options.h"

extern HNETLIBUSER hNetlibConnection;
extern PLUGININFOEX pluginInfoEx;

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

#define MODULENAME "CloudFile"

#define FILE_CHUNK_SIZE 1024 * 1024 //1 MB

#include "http_request.h"
#include "file_transfer.h"

// services
#include "cloud_file.h"
#include "oauth.h"
#include "Services\dropbox_service.h"
#include "Services\google_service.h"
#include "Services\microsoft_service.h"
#include "Services\yandex_service.h"
extern LIST<CCloudService> Services;
void InitializeServices();

// events
int OnModulesLoaded(WPARAM, LPARAM);

// icons
void InitializeIcons();

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

INT_PTR SendFileInterceptor(WPARAM wParam, LPARAM lParam);
UINT UploadAndReportProgressThread(void *owner, void *arg);

// utils
void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact = NULL);
void ShowNotification(const wchar_t *message, int flags, MCONTACT hContact = NULL);
bool CanSendToContact(MCONTACT hContact);
void SendToContact(MCONTACT hContact, const wchar_t *data);
void PasteToInputArea(MCONTACT hContact, const wchar_t *data);
void Report(MCONTACT hContact, const wchar_t *data);

CCloudService* FindService(const char *szProto);

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#endif //_COMMON_H_