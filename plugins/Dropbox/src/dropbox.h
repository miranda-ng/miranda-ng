#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

#include "http_request.h"
#include "file_transfer.h"

#define DROPBOX_API_VER "1"
#define DROPBOX_API_ROOT "sandbox"
#define DROPBOX_WWW_URL "https://www.dropbox.com/"
#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_APP_KEY "fa8du7gkf2q8xzg"
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

	LIST<FileTransferParam> transfers;

	// hooks
	static int OnProtoAck(WPARAM wParam, LPARAM lParam);
	static int OnPreShutdown(WPARAM wParam, LPARAM lParam);
	int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	int OnContactDeleted(WPARAM wParam, LPARAM lParam);
	int OnOptionsInitialized(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int OnSrmmWindowOpened(WPARAM wParam, LPARAM lParam);
	int OnTabSrmmButtonPressed(WPARAM wParam, LPARAM lParam);
	int OnFileDialogCancelled(WPARAM wParam, LPARAM lParam);
	int OnFileDialogSuccessed(WPARAM wParam, LPARAM lParam);

	// services
	static HANDLE CreateProtoServiceFunctionObj(const char *szService, MIRANDASERVICEOBJ serviceProc, void *obj);

	static INT_PTR ProtoGetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoGetName(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoLoadIcon(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoGetStatus(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoSendFile(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoCancelFile(WPARAM wParam, LPARAM lParam);
	INT_PTR ProtoSendMessage(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoReceiveMessage(WPARAM wParam, LPARAM lParam);

	INT_PTR SendFileToDropbox(WPARAM wParam, LPARAM lParam);

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
	void SendFile(const char *path, const char *data, int length);
	void SendFileChunkedFirst(const char *data, int length, char *uploadId, size_t &offset);
	void SendFileChunkedNext(const char *data, int length, const char *uploadId, size_t &offset);
	void SendFileChunkedLast(const char *path, const char *uploadId);

	void CreateFolder(const char *encodedPath);

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

	template<int(CDropbox::*Event)(WPARAM, LPARAM)>
	static int GlobalEvent(void *obj, WPARAM wParam, LPARAM lParam)
	{
		CDropbox *instance = (CDropbox*)obj;
		return instance ? (instance->*Event)(wParam, lParam) : 0;
	}

	template<INT_PTR(CDropbox::*Service)(WPARAM, LPARAM)>
	static INT_PTR GlobalService(void *obj, WPARAM wParam, LPARAM lParam)
	{
		CDropbox *instance = (CDropbox*)obj;
		return instance ? (instance->*Service)(wParam, lParam) : 0;
	}
};

#endif //_DROPBOX_PROTO_H_