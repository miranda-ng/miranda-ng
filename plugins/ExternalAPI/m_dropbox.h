#ifndef M_DROPBOX_H_
#define M_DROPBOX_H_

#define MIID_DROPBOX {0x9649d8e2, 0x7326, 0x4ec1, {0xb4, 0xa3, 0xf2, 0xec, 0x1a, 0x39, 0x84, 0x94}}

struct DropboxUploadInfo
{
	const TCHAR *localPath; // local path
	const TCHAR *serverFolder; // server folder in witch file will be placed (can be NULL)
};

// upload file on Dropbox
// wParam = (WPARAM)(char**) '\r\n' separated download links (can be NULL, otherwise should be manually free)
// lParam = (LPARAM)(const DropboxUploadInfo*)
// returns status of transfer.
// 0 on success otherwise fail
#define MS_DROPBOX_UPLOAD "Dropbox/Upload"

// upload file on Dropbox
// wParam = 0
// lParam = (LPARAM)(const DropboxUploadInfo*)
// returns file htansfer handle or NULL on failure
// returns immediately, without waiting for the send
// note, that you can track progress by using ME_PROTO_ACK
#define MS_DROPBOX_UPLOADASYNC "Dropbox/UploadAsync"

// if you want to get download links after upload
// use ME_DROPBOX_UPLOADED hook. you'll get:
struct DropboxUploadResult
{
	HANDLE hProcess;	// hProcess
	int status;			// status of transfer. 0 on success otherwise fail
	const char* data;	// '\r\n' separated download links
};

// notifies a caller that upload has been finished
// wParam = 0
// lParam = (LPARAM)(DropboxUploadResult*)
#define ME_DROPBOX_UPLOADED "Dropbox/Uploaded"

#endif //M_DROPBOX_H_