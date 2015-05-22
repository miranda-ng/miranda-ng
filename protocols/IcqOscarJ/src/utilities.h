// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#ifndef __UTILITIES_H
#define __UTILITIES_H

struct icq_ack_args
{
	MCONTACT hContact;
	int    nAckType;
	int    nAckResult;
	HANDLE hSequence;
	LPARAM pszMessage;
};

struct icq_contacts_cache
{
	MCONTACT hContact;
	DWORD dwUin;
	const char *szUid;
};


/*---------* Functions *---------------*/

void MoveDlgItem(HWND hwndDlg, int iItem, int left, int top, int width, int height);
void EnableDlgItem(HWND hwndDlg, UINT control, int state);
void ShowDlgItem(HWND hwndDlg, UINT control, int state);
void icq_EnableMultipleControls(HWND hwndDlg, const UINT* controls, int cControls, int state);
void icq_ShowMultipleControls(HWND hwndDlg, const UINT* controls, int cControls, int state);
int IcqStatusToMiranda(WORD wStatus);
WORD MirandaStatusToIcq(int nStatus);
int MirandaStatusToSupported(int nMirandaStatus);
char *MirandaStatusToStringUtf(int);

int AwayMsgTypeToStatus(int nMsgType);

void   SetGatewayIndex(HANDLE hConn, DWORD dwIndex);
DWORD  GetGatewayIndex(HANDLE hConn);
void   FreeGatewayIndex(HANDLE hConn);

char *NickFromHandle(MCONTACT hContact);
char *NickFromHandleUtf(MCONTACT hContact);
char *strUID(DWORD dwUIN, char *pszUID);

char* __fastcall strstrnull(const char *str, const char *substr);
char* __fastcall null_strdup(const char *string);
char* __fastcall null_strcpy(char *dest, const char *src, size_t maxlen);
size_t __fastcall null_strcut(char *string, size_t maxlen);

WCHAR* __fastcall null_strdup(const WCHAR *string);
WCHAR* __fastcall null_strcpy(WCHAR *dest, const WCHAR *src, size_t maxlen);

void parseServerAddress(char *szServer, WORD* wPort);

char *DemangleXml(const char *string, size_t len);
char *MangleXml(const char *string, size_t len);
char *EliminateHtml(const char *string, size_t len);
char *ApplyEncoding(const char *string, const char *pszEncoding);

int RandRange(int nLow, int nHigh);

bool IsStringUIN(const char *pszString);

char* time2text(time_t time);

void __fastcall SAFE_FREE(void** p);
void* __fastcall SAFE_MALLOC(size_t size);
void* __fastcall SAFE_REALLOC(void* p, size_t size);

__inline static void SAFE_FREE(char** str) { SAFE_FREE((void**)str); }
__inline static void SAFE_FREE(WCHAR** str) { SAFE_FREE((void**)str); }

void __fastcall SAFE_DELETE(MZeroedObject **p);

DWORD ICQWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds, int bWaitAlways = FALSE);

HANDLE NetLib_OpenConnection(HANDLE hUser, const char* szIdent, NETLIBOPENCONNECTION* nloc);
void NetLib_CloseConnection(HANDLE *hConnection, int bServerConn);
void NetLib_SafeCloseHandle(HANDLE *hConnection);

char* __fastcall ICQTranslateUtf(const char *src);
char* __fastcall ICQTranslateUtfStatic(const char *src, char *buf, size_t bufsize);

WORD GetMyStatusFlags();

/* Unicode FS utility functions */

int IsValidRelativePath(const char *filename);
const char* ExtractFileName(const char *fullname);
char* FileNameToUtf(const TCHAR *filename);

int FileAccessUtf(const char *path, int mode);
int FileStatUtf(const char *path, struct _stati64 *buffer);
int MakeDirUtf(const char *dir);
int OpenFileUtf(const char *filename, int oflag, int pmode);

/* Unicode UI utility functions */
WCHAR* GetWindowTextUcs(HWND hWnd);
void SetWindowTextUcs(HWND hWnd, WCHAR *text);
char *GetWindowTextUtf(HWND hWnd);
char *GetDlgItemTextUtf(HWND hwndDlg, int iItem);
void SetWindowTextUtf(HWND hWnd, const char *szText);
void SetDlgItemTextUtf(HWND hwndDlg, int iItem, const char *szText);

int  ComboBoxAddStringUtf(HWND hCombo, const char *szString, DWORD data);
int  ListBoxAddStringUtf(HWND hList, const char *szString);

int  MessageBoxUtf(HWND hWnd, const char *szText, const char *szCaption, UINT uType);

void InitXStatusIcons();
void setContactExtraIcon(MCONTACT hContact, int xstatus);
int  OnReloadIcons(WPARAM wParam, LPARAM lParam);

#endif /* __UTILITIES_H */
