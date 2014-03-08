#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

#include <map>
#include <string>
#include "http_request.h"
#include "file_transfer.h"

#define DROPBOX_API_VER "1"
#define DROPBOX_API_ROOT "sandbox"
#define DROPBOX_WWW_URL "https://www.dropbox.com/"
#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_API_KEY "fa8du7gkf2q8xzg"
#include "..\..\..\Dropbox\secret_key.h"

#define DROPBOX_FILE_CHUNK_SIZE 1024 * 1024 //1 MB

#define BBB_ID_FILE_SEND 10001

enum
{
	CMI_SEND_FILES,
	CMI_MAX   // this item shall be the last one
};

struct CommandParam
{
	CDropbox *instance;
	HANDLE hProcess;
	MCONTACT hContact;
	void *data;
};

class CDropbox
{
public:
	CDropbox();
	virtual ~CDropbox() { }

private:
	HANDLE hNetlibUser;
	ULONG  hFileProcess;
	ULONG  hMessageProcess;
	
	MCONTACT hDefaultContact;
	MCONTACT hTransferContact;

	std::map<HWND, MCONTACT> dcftp;
	std::map<std::string, pThreadFunc> commands;

	HGENMENU contactMenuItems[CMI_MAX];

	// hooks
	static int OnModulesLoaded(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnPreShutdown(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnContactDeleted(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnOptionsInitialized(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnPrebuildContactMenu(void *obj, WPARAM wParam, LPARAM lParam);	
	static int OnSrmmWindowOpened(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnTabSrmmButtonPressed(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnFileDoalogCancelled(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnFileDoalogSuccessed(void *obj, WPARAM wParam, LPARAM lParam);

	// services
	static INT_PTR ProtoGetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendFile(void *obj, WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendMessage(void *obj, WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoReceiveMessage(void *obj, WPARAM wParam, LPARAM lParam);

	static INT_PTR SendFilesToDropbox(void *obj, WPARAM wParam, LPARAM lParam);

	// commands
	static void CommandHelp(void *arg);
	static void CommandContent(void *arg);
	static void CommandShare(void *arg);
	static void CommandDelete(void *arg);

	// access token
	bool HasAccessToken();

	void RequestAcceessToken();
	void DestroyAcceessToken();

	static UINT RequestAcceessTokenAsync(void *owner, void* param);

	// account info
	void RequestAccountInfo();

	// transrers
	static int HandleFileTransferError(NETLIBHTTPREQUEST *response, MCONTACT hContact);

	int SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset, MCONTACT hContact);
	int SendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset, MCONTACT hContact);
	int SendFileChunkedLast(const char *fileName, const char *uploadId, MCONTACT hContact);

	int CreateFolder(const char *folderName, MCONTACT hContact);

	static void _cdecl SendFileAsync(void *arg);

	// contacts
	MCONTACT GetDefaultContact();

	// icons
	void InitializeIcons();

	// menus
	void InitializeMenus();
	static void Menu_DisableItem(HGENMENU hMenuItem, BOOL bDisable);

	// dialogs
	static INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// utils
	static HANDLE CreateProtoServiceFunctionObj(const char *szService, MIRANDASERVICEOBJ serviceProc, void *obj);

	static wchar_t *HttpStatusToText(HTTP_STATUS status);

	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
};

#endif //_DROPBOX_PROTO_H_