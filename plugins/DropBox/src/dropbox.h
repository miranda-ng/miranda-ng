#ifndef _Dropbox_PROTO_H_
#define _Dropbox_PROTO_H_

//#include "common.h"
#include "http_request.h"

#define DROPBOX_API_VER "1"
#define DROPBOX_API_ROOT "sandbox"
#define DROPBOX_WWW_URL "https://www.dropbox.com/"
#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_API_KEY "fa8du7gkf2q8xzg"
#include "..\..\..\Dropbox\secret_key.h"

#define DROPBOX_FILE_CHUNK_SIZE 1024 * 1024 //1 MB

enum
{
	CMI_API_ACCESS_REQUERIED,
	CMI_URL_OPEN_ROOT,
	CMI_MAX   // this item shall be the last one
};

struct FileTransferParam
{
	HANDLE hProcess;
	PROTOFILETRANSFERSTATUS pfts;

	FileTransferParam() 
	{
		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_UTF;
		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.currentFileTime = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.tszWorkingDir = NULL;
		pfts.wszCurrentFile = NULL;
	}

	~FileTransferParam() 
	{
		for (int i = 0; pfts.pszFiles[pfts.totalFiles]; i++)
		{
			delete pfts.pszFiles[i];
		}
		delete pfts.pszFiles;
	}
};

class CDropbox
{
public:
	CDropbox();
	~CDropbox() { }

private:
	HANDLE hNetlibUser;
	ULONG  hFileProcess;

	static HGENMENU ContactMenuItems[CMI_MAX];

	// hooks
	static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	static int OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// services
	static INT_PTR GetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR SendFile(WPARAM wParam, LPARAM lParam);
	static INT_PTR SendMessage(WPARAM wParam, LPARAM lParam);

	static INT_PTR RequeriedApiAccess(WPARAM wParam, LPARAM lParam);

	// access token
	static bool HasAccessToken();

	void RequestAcceessToken();
	void DestroyAcceessToken();

	// transrers
	HttpRequest *CreateFileSendChunkedRequest(const char *data, int length);
	void SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset);
	void SendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset);
	void SendFileChunkedLast(const char *fileName, const char *uploadId, MCONTACT hContact);

	static void _cdecl SendFileAsync(void *arg);

	// contacts
	static MCONTACT GetDefaultContact();

	// icons
	static void InitIcons();

	// menus
	static void InitMenus();

	// dialogs
	static INT_PTR CALLBACK TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// utils
	void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
};

#endif //_Dropbox_PROTO_H_