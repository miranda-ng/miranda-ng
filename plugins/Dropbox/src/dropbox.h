#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

#include "singleton.h"
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
	CMI_API_REQUEST_AUTH,
	CMI_SEND_FILES,
	CMI_MAX   // this item shall be the last one
};

struct FileTransferParam
{
	HANDLE hProcess;
	PROTOFILETRANSFERSTATUS pfts;

	int totalFolders;
	char **pszFolders;
	int relativePathStart;

	FileTransferParam()
	{
		totalFolders = 0;
		pszFolders = NULL;
		relativePathStart = 0;

		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_UTF;
		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.pszFiles = NULL;
		pfts.tszWorkingDir = NULL;
		pfts.wszCurrentFile = NULL;
	}

	~FileTransferParam()
	{
		if (pfts.pszFiles)
		{
			for (int i = 0; pfts.pszFiles[i]; i++)
			{
				if (pfts.pszFiles[i]) mir_free(pfts.pszFiles[i]);
			}
			delete pfts.pszFiles;
		}

		if (pszFolders)
		{
			for (int i = 0; pszFolders[i]; i++)
			{
				if (pszFolders[i]) mir_free(pszFolders[i]);
			}
			delete pszFolders;
		}
	}
};

class CDropbox
{
public:
	void Init();
	void Uninit() { };

private:
	HANDLE hNetlibUser;
	ULONG  hFileProcess;

	static HGENMENU ContactMenuItems[CMI_MAX];

	// hooks
	static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	static int OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// services
	static INT_PTR ProtoGetCaps(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendFile(WPARAM wParam, LPARAM lParam);
	static INT_PTR ProtoSendMessage(WPARAM wParam, LPARAM lParam);

	static INT_PTR RequestApiAuthorization(WPARAM wParam, LPARAM lParam);

	static INT_PTR SendFilesToDropbox(WPARAM wParam, LPARAM lParam);

	// access token
	static bool HasAccessToken();

	void RequestAcceessToken(MCONTACT hContact);
	void DestroyAcceessToken(MCONTACT hContact);

	static void RequestApiAuthorizationAsync(void *arg);

	// transrers
	HttpRequest *CreateFileSendChunkedRequest(const char *data, int length);
	void SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset);
	void SendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset);
	void SendFileChunkedLast(const char *fileName, const char *uploadId, MCONTACT hContact);

	void CreateFolder(const char *folderName, MCONTACT hContact);

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

#endif //_DROPBOX_PROTO_H_