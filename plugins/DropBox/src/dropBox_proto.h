#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

//#include "common.h"
#include "http_request.h"

#define DROPBOX_API_VER "1"
#define DROPBOX_API_ROOT "sandbox"
#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_API_KEY "fa8du7gkf2q8xzg"
#include "..\..\DropBox\secret_key.h"

#define DROPBOX_FILE_CHUNK_SIZE 1024 * 1024 //1 MB

extern ULONG  g_fileId;

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

bool HasAccessToken();

void SetContactStatus(MCONTACT hContact, int newStatus);
bool DropBoxLogIn();

int OnOptionsInit(WPARAM wParam, LPARAM lParam);
int OnModulesLoaded(WPARAM wParam, LPARAM lParam);

INT_PTR DropBoxGetCaps(WPARAM wParam, LPARAM lParam);

INT_PTR DropBoxSendMessage(WPARAM wParam, LPARAM lParam);

HttpRequest *DropBoxCreateFileChunkedRequest(const char *data, int length);
bool DropBoxSendFileChunkedStart(const char *data, int length, char *uploadId, int &offset);
bool DropBoxSendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset);
bool DropBoxSendFileChunkedEnd(const char *fileName, const char *uploadId, MCONTACT hContact);

void DropBoxAsyncFileSend(void *arg);

INT_PTR DropBoxSendFile(WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //_DROPBOX_PROTO_H_