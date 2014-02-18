#ifndef _DROPBOX_PROTO_H_
#define _DROPBOX_PROTO_H_

//#include "common.h"
#include "http_request.h"

#define DROPBOX_API_VER "1"

#define DROPBOX_API_URL "https://api.dropbox.com/" DROPBOX_API_VER
#define DROPBOX_APICONTENT_URL "https://api-content.dropbox.com/" DROPBOX_API_VER

#define DROPBOX_API_KEY "fa8du7gkf2q8xzg"
#include "..\..\DropBoxApi\sekret_key.h"

#define DROPBOX_FILE_CHUNK_SIZE 100 * 1024 //100 KB
//4 * 1024 * 1024  // 4 MB

extern ULONG  g_fileId;

void SetContactStatus(MCONTACT hContact, int newStatus);
bool DropBoxLogIn();

int OnOptionsInit(WPARAM wParam, LPARAM lParam);
int OnModulesLoaded(WPARAM wParam, LPARAM lParam);

INT_PTR DropBoxGetCaps(WPARAM wParam, LPARAM lParam);

INT_PTR DropBoxSendMessage(WPARAM wParam, LPARAM lParam);
INT_PTR DropBoxReceiveMessage(WPARAM wParam, LPARAM lParam);

HttpRequest *DropBoxCreateFileChunkedRequest(const char *data, int length);
bool DropBoxSendFileChunkedStart(const char *data, int length, char *uploadId, int &offset);
bool DropBoxSendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset);
bool DropBoxSendFileChunkedEnd(const char *fileName, const char *uploadId, MCONTACT hContact);

void DropBoxAsyncFileSend(void *args);

INT_PTR DropBoxSendFile(WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //_DROPBOX_PROTO_H_