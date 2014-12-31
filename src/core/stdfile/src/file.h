/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define VIRUSSCAN_DISABLE  0
#define VIRUSSCAN_AFTERDL  1
#define VIRUSSCAN_DURINGDL 2

#define FILERESUME_ASK         0
//1, 2, 3, 4: resume, overwrite, rename, skip: from proto library
#define FILERESUMEF_ALL         0x80
#define FILERESUME_RESUMEALL    (FILERESUME_RESUME|FILERESUMEF_ALL)
#define FILERESUME_OVERWRITEALL (FILERESUME_OVERWRITE|FILERESUMEF_ALL)
#define FILERESUME_RENAMEALL    (FILERESUME_RENAME|FILERESUMEF_ALL)
#define FILERESUME_CANCEL       0xFFFFFFFF

#define M_FILEEXISTSDLGREPLY   (WM_USER+200)
#define M_PRESHUTDOWN		   (WM_USER+201)

struct FileSendData
{
	MCONTACT hContact;
	const TCHAR **ppFiles;
};

#define BYTESRECVEDHISTORYCOUNT  10   //the number of bytes recved is sampled once a second and the last 10 are used to get the transfer speed
struct FileDlgData
{
	HWND hwndTransfer;
	HANDLE fs;
	MCONTACT hContact;
	HANDLE hDbEvent;
	HANDLE hNotifyEvent;
	TCHAR **files;
	int send;
	int closeIfFileChooseCancelled;
	int resumeBehaviour;
	int bytesRecvedHistory[BYTESRECVEDHISTORYCOUNT];
	int bytesRecvedHistorySize;
	int waitingForAcceptance;
	PROTOFILETRANSFERSTATUS transferStatus;
	int *fileVirusScanned;
	HANDLE hPreshutdownEvent;
	DWORD dwTicks;

	TCHAR szSavePath[MAX_PATH];
	TCHAR szMsg[450], szFilenames[1024];
	HICON hIcon, hIconFolder;
};

//file.c
#define UNITS_BYTES     1   // 0 <= size<1000: "%d bytes"
#define UNITS_KBPOINT1	2	// 1000 <= size<100*1024: "%.1f KB"
#define UNITS_KBPOINT0  3   // 100*1024 <= size<1024*1024: "%d KB"
#define UNITS_MBPOINT2  4   // 1024*1024 <= size: "%.2f MB"
#define UNITS_GBPOINT3  5   // 1024*1024*1024 <= size: "%.3f GB"

void GetSensiblyFormattedSize(__int64 size, TCHAR *szOut, int cchOut, int unitsOverride, int appendUnits, int *unitsUsed);
void FreeFilesMatrix(TCHAR ***files);	  //loving that triple indirection
void FreeProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *fts);
void CopyProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src);
void UpdateProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src);
int SRFile_GetRegValue(HKEY hKeyBase, const TCHAR *szSubKey, const TCHAR *szValue, TCHAR *szOutput, int cbOutput);
//filesenddlg.c
INT_PTR CALLBACK DlgProcSendFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//filerecv.c
INT_PTR CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void RemoveInvalidFilenameChars(TCHAR *tszString);
void RemoveInvalidPathChars(TCHAR *tszString);
void GetContactReceivedFilesDir(MCONTACT hContact, TCHAR *szDir, int cchDir, BOOL substVars);
void GetReceivedFilesDir(TCHAR *szDir, int cchDir);
int BrowseForFolder(HWND hwnd, TCHAR *szPath);
//fileexistsdlg.c
struct TDlgProcFileExistsParam
{
	HWND hwndParent;
	PROTOFILETRANSFERSTATUS *fts;
};
INT_PTR CALLBACK DlgProcFileExists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//filexferdlg.c
INT_PTR CALLBACK DlgProcFileTransfer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//fileopts.c
int FileOptInitialise(WPARAM wParam, LPARAM lParam);
//ftmanager.c
#define WM_FT_ADD			(WM_USER+701)
#define WM_FT_RESIZE		(WM_USER+702)
#define WM_FT_REMOVE		(WM_USER+703)
#define WM_FT_SELECTPAGE	(WM_USER+704)
#define WM_FT_CLEANUP		(WM_USER+705)
#define WM_FT_COMPLETED		(WM_USER+706)

HWND FtMgr_Show(bool bForceActivate, bool bFromMenu);
void FtMgr_Destroy();
HWND FtMgr_AddTransfer(FileDlgData *dat);

void FreeFileDlgData(FileDlgData* dat);

TCHAR *GetContactID(MCONTACT hContact);

extern HANDLE hDlgSucceeded, hDlgCanceled;
