/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org),
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

#pragma once

#define SRFILEMODULE "SRFile"

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

#define BYTESRECVEDHISTORYCOUNT  10   //the number of bytes recved is sampled once a second and the last 10 are used to get the transfer speed

struct FileDlgData : public MZeroedObject
{
	~FileDlgData();

	HWND hwndTransfer;
	HANDLE fs;
	MCONTACT hContact;
	MEVENT hDbEvent;
	HANDLE hNotifyEvent;
	HICON hIcon, hIconFolder;
	wchar_t **files;
	int send;
	int closeIfFileChooseCancelled;
	int resumeBehaviour;
	int bytesRecvedHistory[BYTESRECVEDHISTORYCOUNT];
	int bytesRecvedHistorySize;
	int waitingForAcceptance;
	PROTOFILETRANSFERSTATUS transferStatus;
	int *fileVirusScanned;
	uint32_t dwTicks;

	wchar_t szSavePath[MAX_PATH];
	wchar_t szMsg[450], szFilenames[1024];
};

// file.c
MEVENT Proto_RecvFile(MCONTACT hContact, PROTORECVFILE *pre);

// filerecv.c
void LaunchRecvDialog(CLISTEVENT *cle);
void RemoveInvalidFilenameChars(wchar_t *tszString);
void RemoveInvalidPathChars(wchar_t *tszString);
void GetContactSentFilesDir(MCONTACT hContact, wchar_t *szDir, int cchDir);
void GetReceivedFilesDir(wchar_t *szDir, int cchDir);
int  BrowseForFolder(HWND hwnd, wchar_t *szPath);

// fileexistsdlg.c
struct TDlgProcFileExistsParam
{
	HWND hwndParent;
	PROTOFILETRANSFERSTATUS *fts;
};
INT_PTR CALLBACK DlgProcFileExists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// filexferdlg.c
INT_PTR CALLBACK DlgProcFileTransfer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// fileopts.c
int SRFileOptInitialise(WPARAM wParam, LPARAM lParam);

// ftmanager.c
#define WM_FT_ADD			(WM_USER+701)
#define WM_FT_RESIZE		(WM_USER+702)
#define WM_FT_REMOVE		(WM_USER+703)
#define WM_FT_SELECTPAGE	(WM_USER+704)
#define WM_FT_CLEANUP		(WM_USER+705)
#define WM_FT_COMPLETED		(WM_USER+706)

#define HM_RECVEVENT		(WM_USER+10)

HWND FtMgr_Show(bool bForceActivate, bool bFromMenu);
void FtMgr_Destroy();
void FtMgr_AddTransfer(FileDlgData *dat);

extern HANDLE hDlgSucceeded, hDlgCanceled;
extern MWindowList g_hFileWindows;

namespace File
{
	extern CMOption<bool> bAutoMin, bAutoClear, bAutoClose, bAutoAccept, bReverseOrder;
	extern CMOption<bool> bWarnBeforeOpening, bOfflineAuto;
	extern CMOption<uint8_t> iIfExists, iUseScanner;
	extern CMOption<uint32_t> iOfflineSize;
	extern CMOption<wchar_t*> wszSaveDir, wszScanCmdLine;
};

// fieutils.c
void FreeFilesMatrix(wchar_t ***files);	  //loving that triple indirection
void FreeProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *fts);
void CopyProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src);
void UpdateProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src);

wchar_t *PFTS_StringToTchar(int flags, const MAllStrings s);
int  PFTS_CompareWithTchar(PROTOFILETRANSFERSTATUS *ft, const MAllStrings s, wchar_t *r);

MFilePath CreateUniqueFileName(const wchar_t *pszOriginalFile);
MFilePath FindUniqueFileName(const wchar_t *pszOriginalFile);

int  GetRegValue(HKEY hKeyBase, const wchar_t *szSubKey, const wchar_t *szValue, wchar_t *szOutput, int cbOutput);
void GetSensiblyFormattedSize(__int64 size, wchar_t *szOut, int cchOut, int unitsOverride, int appendUnits, int *unitsUsed);

// downloads or launches cloud file

struct OFD_CopyUrl : public OFD_Callback
{
	CMStringW wszUrl;

	OFD_CopyUrl(const CMStringW &url) :
		wszUrl(url)
	{}

	void Invoke(const OFDTHREAD &ofd) override
	{
		Utils_ClipboardCopy(ofd.wszPath.IsEmpty() ? wszUrl : ofd.wszPath);
	}
};

struct OFD_Download : public OFD_Callback
{
	void Invoke(const OFDTHREAD &ofd) override
	{
		if (ofd.bOpen)
			ShellExecuteW(nullptr, L"open", ofd.wszPath, nullptr, nullptr, SW_SHOWDEFAULT);
	}
};

class OFD_SaveAs : public OFD_Callback
{
	ptrW m_path;

public:
	OFD_SaveAs(const wchar_t *pwszPath) :
		m_path(mir_wstrdup(pwszPath))
	{}

	~OFD_SaveAs() {}

	void Invoke(const OFDTHREAD &ofd) override
	{
		CopyFileW(ofd.wszPath, m_path, false);
	}
};

void DownloadOfflineFile(MCONTACT, MEVENT, DB::EventInfo &dbei, int iCommand, OFD_Callback *pCallback);
