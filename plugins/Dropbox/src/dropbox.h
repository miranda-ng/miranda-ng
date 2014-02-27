#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

#include <map>
#include "singleton.h"
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

#define INSTANCE Singleton<CDropbox>::GetInstance()

enum
{
	CMI_API_REQUEST_AUTH,
	CMI_SEND_FILES,
	CMI_MAX   // this item shall be the last one
};

class CDropbox
{
public:
	void Init();
	void Uninit() { };

private:
	HANDLE hNetlibUser;
	ULONG  hFileProcess;
	MCONTACT hContactTransfer;

	static MCONTACT hContactDefault;
	static std::map<HWND, MCONTACT> dcftp;

	static HGENMENU ContactMenuItems[CMI_MAX];

	// hooks
	static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	static int OnOptionsInit(WPARAM wParam, LPARAM lParam);
	static int OnContactDeleted(WPARAM wParam, LPARAM lParam);
	static int OnPrebuildContactMenu(WPARAM wParam, LPARAM);
	static int OnSrmmWindowOpened(WPARAM wParam, LPARAM);
	static int OnSrmmButtonPressed(WPARAM wParam, LPARAM);
	static int OnFileDoalogCancelled(WPARAM wParam, LPARAM);
	static int OnFileDoalogSuccessed(WPARAM wParam, LPARAM);

	// services
	static INT_PTR ProtoGetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendFile(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendMessage(WPARAM wParam, LPARAM lParam);

	static INT_PTR RequestApiAuthorization(WPARAM wParam, LPARAM lParam);

	static INT_PTR SendFilesToDropbox(WPARAM wParam, LPARAM lParam);

	// access token
	static bool HasAccessToken();

	void RequestAcceessToken();
	void DestroyAcceessToken();

	static void RequestApiAuthorizationAsync(void *arg);

	// transrers
	int HandleFileTransferError(NETLIBHTTPREQUEST *response, MCONTACT hContact);

	int SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset, MCONTACT hContact);
	int SendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset, MCONTACT hContact);
	int SendFileChunkedLast(const char *fileName, const char *uploadId, MCONTACT hContact);

	int CreateFolder(const char *folderName, MCONTACT hContact);

	static void _cdecl SendFileAsync(void *arg);

	// contacts
	static MCONTACT GetDefaultContact();

	// icons
	static void InitIcons();

	// menus
	static void InitMenus();
	static void Menu_DisableItem(HGENMENU hMenuItem, BOOL bDisable);

	// dialogs
	static INT_PTR CALLBACK TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// utils
	static wchar_t *HttpStatusToText(HTTP_STATUS status);

	void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
};

#endif //_DROPBOX_PROTO_H_