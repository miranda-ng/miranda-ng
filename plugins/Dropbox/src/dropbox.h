#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

#include "http_request.h"
#include "file_transfer.h"

#define DROPBOX_API_VER "1"
#define DROPBOX_API_ROOT "sandbox"
#define DROPBOX_WWW_URL "https://www.dropbox.com/"
#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_API_KEY "fa8du7gkf2q8xzg"
#include "..\..\..\miranda-private-keys\Dropbox\secret_key.h"

#define DROPBOX_FILE_CHUNK_SIZE 4 * 1024 * 1024 //4 MB

#define BBB_ID_FILE_SEND 10001

enum
{
	CMI_SEND_FILES,
	CMI_MAX   // this item shall be the last one
};

class CDropbox : public MZeroedObject
{
	struct CommandParam
	{
		CDropbox *instance;
		HANDLE hProcess;
		MCONTACT hContact;
		void *data;
	};

public:
	CDropbox();
	virtual ~CDropbox();

private:
	HANDLE hNetlibConnection;
	ULONG  hFileProcess;
	ULONG  hMessageProcess;

	HANDLE hFileSentEventHook;

	MCONTACT hDefaultContact;
	MCONTACT hTransferContact;
	HWND     hTransferWindow;

	HGENMENU contactMenuItems[CMI_MAX];

	// hooks
	static int OnProtoAck(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnPreShutdown(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnModulesLoaded(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnContactDeleted(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnOptionsInitialized(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnPrebuildContactMenu(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnSrmmWindowOpened(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnTabSrmmButtonPressed(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnFileDialogCancelled(void *obj, WPARAM wParam, LPARAM lParam);
	static int OnFileDialogSuccessed(void *obj, WPARAM wParam, LPARAM lParam);

	// services
	static HANDLE CreateProtoServiceFunctionObj(const char *szService, MIRANDASERVICEOBJ serviceProc, void *obj);

	static INT_PTR ProtoGetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoGetName(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoLoadIcon(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoGetStatus(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSetStatus(void *obj, WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendFile(void *obj, WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendMessage(void *obj, WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoReceiveMessage(void *obj, WPARAM wParam, LPARAM lParam);

	static INT_PTR SendFileToDropbox(void *obj, WPARAM wParam, LPARAM lParam);

	// commands
	static void CommandHelp(void *arg);
	static void CommandContent(void *arg);
	static void CommandShare(void *arg);
	static void CommandDelete(void *arg);

	// access token
	bool HasAccessToken();

	void RequestAccessToken();
	void DestroyAccessToken();

	static UINT RequestAccessTokenAsync(void *owner, void *param);

	// account info
	void RequestAccountInfo();

	// transfers
	void SendFile(const char *fileName, const char *data, int length);
	void SendFileChunkedFirst(const char *data, int length, char *uploadId, size_t &offset);
	void SendFileChunkedNext(const char *data, int length, const char *uploadId, size_t &offset);
	void SendFileChunkedLast(const char *fileName, const char *uploadId);

	void CreateFolder(const char *folderName);

	void CreateDownloadUrl(const char *path, char *url);

	static UINT SendFilesAsync(void *owner, void *arg);
	static UINT SendFilesAndEventAsync(void *owner, void *arg);
	static UINT SendFilesAndReportAsync(void *owner, void *arg);

	// contacts
	MCONTACT GetDefaultContact();

	// menus
	void InitializeMenus();
	static void Menu_DisableItem(HGENMENU hMenuItem, BOOL bDisable);

	static INT_PTR SendFilesToDropboxCommand(void *obj, WPARAM wParam, LPARAM lParam);

	// dialogs
	static INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// SRMM
	static void DisableSrmmButton(MCONTACT hContact);

	// utils
	static char* HttpStatusToText(HTTP_STATUS status);
	static void HandleHttpResponseError(NETLIBHTTPREQUEST *response);
};

#endif //_DROPBOX_PROTO_H_