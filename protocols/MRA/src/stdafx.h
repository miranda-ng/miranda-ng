#if !defined(AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <Commdlg.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>

#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#include <crtdbg.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>

#include "../../plugins/zlib/src/zlib.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_cluiframes.h>
#include <m_clui.h>
#include <m_genmenu.h>
#include <m_ignore.h>
#include <m_message.h>
#include <m_options.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_string.h>
#include <m_netlib.h>
#include <m_icolib.h>
#include <win2k.h>
#include <m_popup.h>
#include <m_nudge.h>
#include <m_folders.h>
#include <m_avatars.h>
#include <m_chat.h>
#include <m_extraicons.h>
#include <m_music.h>
#include <m_xstatus.h>
#include <m_nudge.h>
#include <m_proto_listeningto.h>

#define PROTO_VERSION_MAJOR     1
#define PROTO_VERSION_MINOR     21

#include "resource.h"
#include "..\xstatus_MRA\src\resource.h"

#include "Sdk/MemoryFind.h"
#include "Sdk/MemoryFindByte.h"
#include "Sdk/BuffToLowerCase.h"
#include "Sdk/StrHexToNum.h"
#include "Sdk/timefuncs.h"
#include "Sdk/InternetTime.h"
#include "Sdk/ListMT.h"
#include "Sdk/FIFOMT.h"
#include "Sdk/RC4.h"
#include "Sdk/SHA1.h"

// LPS
struct MRA_LPS
{
	size_t dwSize;

	union {
		LPSTR  lpszData;
		LPWSTR lpwszData;
	};
};


// GUID
struct MRA_GUID
{
	DWORD	id[4];
};

struct MRA_ADDR_LIST_ITEM
{
	DWORD	dwPort;
	DWORD	dwAddr;
};

struct MRA_ADDR_LIST
{
	DWORD	dwAddrCount;
	MRA_ADDR_LIST_ITEM *pMailAddress;
};

/////////////////////////////////////////////////////////////////////////////

class BinBuffer
{
	LPBYTE m_data;
	size_t m_len;

public:
	BinBuffer(LPBYTE data, size_t len) : m_data(data), m_len(len) {}

	DWORD     getDword();
	DWORDLONG getInt64();
	MRA_GUID  getGuid();
	void      getStringA(CMStringA&);
	void      getStringW(CMStringW&);

	__forceinline bool eof() const { return m_len == 0; }
};

__forceinline BinBuffer& operator >>(BinBuffer& buf, DWORD &dwVar)     { dwVar = buf.getDword(); return buf; }
__forceinline BinBuffer& operator >>(BinBuffer& buf, DWORDLONG &llVar) { llVar = buf.getInt64(); return buf; }
__forceinline BinBuffer& operator >>(BinBuffer& buf, MRA_GUID &guid)   { guid = buf.getGuid();   return buf; }
__forceinline BinBuffer& operator >>(BinBuffer& buf, CMStringA &sVar)  { buf.getStringA(sVar);   return buf; }
__forceinline BinBuffer& operator >>(BinBuffer& buf, CMStringW &sVar)  { buf.getStringW(sVar);   return buf; }

/////////////////////////////////////////////////////////////////////////////

bool DB_GetStaticStringA(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize);
bool DB_GetStaticStringW(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPWSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize);

bool DB_GetStringA(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, CMStringA &Ret);
bool DB_GetStringW(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, CMStringW &Ret);

bool DB_SetStringExA(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, const CMStringA &value);
bool DB_SetStringExW(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, const CMStringW &value);

bool DB_GetContactSettingBlob(MCONTACT hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPVOID lpRet, size_t dwRetBuffSize, size_t *pdwRetBuffSize);

#include "MraConstans.h"
#include "MraProto.h"

extern IconItem gdiMenuItems[];
extern IconItem gdiContactMenuItems[];
extern IconItem gdiExtraStatusIconsItems[];

#include "proto.h"
#include "MraMRIMProxy.h"
#include "MraSendQueue.h"
#include "MraFilesQueue.h"
#include "MraPopup.h"
#include "MraAvatars.h"
#include "MraIcons.h"
#include "MraSelectEMail.h"
#include "MraMPop.h"

/////////////////////////////////////////////////////////////////////////////////////////
//	plugin options

extern HINSTANCE g_hInstance;
extern HMODULE   g_hDLLXStatusIcons;
extern HICON     g_hMainIcon;

extern bool      g_bChatExist, g_bShutdown;

extern size_t    g_dwMirWorkDirPathLen;
extern WCHAR     g_szMirWorkDirPath[MAX_FILEPATH];

/////////////////////////////////////////////////////////////////////////////////////////
//	External variables

extern LIST<CMraProto> g_Instances;

extern HANDLE hXStatusAdvancedStatusIcons[];

void    InitExtraIcons();

CMStringA MraGetSelfVersionString();

#define GetContactNameA(Contact) (LPSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)Contact, 0)
#define GetContactNameW(Contact) (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)Contact, GCDNF_UNICODE)

#define SetBit(bytes, bitpos) bytes |= (1<<bitpos)
#define GetBit(bytes, bitpos) ((bytes&(1<<bitpos))? TRUE:FALSE)

#define IsXStatusValid(XStatus) (((XStatus) && (XStatus) < MRA_XSTATUS_COUNT))

#define GET_CURRENT_COMBO_DATA(hWndDlg, ControlID)	SendDlgItemMessage(hWndDlg, ControlID, CB_GETITEMDATA, SendDlgItemMessage(hWndDlg, ControlID, CB_GETCURSEL, 0, 0), 0)

#define IsFileExist(FileName) (GetFileAttributes(FileName) != INVALID_FILE_ATTRIBUTES)

void      MraAddrListFree(MRA_ADDR_LIST *pmalAddrList);
DWORD     MraAddrListGetFromBuff(const CMStringA &szAddresses, MRA_ADDR_LIST *pmalAddrList);
CMStringA MraAddrListGetToBuff(MRA_ADDR_LIST *pmalAddrList);

void      InitXStatusIcons();
void      DestroyXStatusIcons();

CMStringA MraGetVersionStringFromFormatted(const CMStringA& szUserAgentFormatted);
CMStringA CopyNumber(const CMStringA&);

CMStringW DecodeXML(const CMStringW &lptszMessage);
CMStringW EncodeXML(const CMStringW &lptszMessage);

bool      IsHTTPSProxyUsed(HANDLE hConnection);
bool      IsContactMraProto(MCONTACT hContact);
bool      IsEMailMR(const CMStringA& szEmail);
bool      GetEMailFromString(const CMStringA& szBuff, CMStringA& szEmail);

bool      IsPhone(LPSTR lpszString, size_t dwStringSize);
bool      IsContactPhone(MCONTACT hContact, LPSTR lpszPhone, size_t dwPhoneSize);
HANDLE    MraHContactFromPhone(LPSTR lpszPhone, size_t dwPhoneSize, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded);
void      EnableControlsArray(HWND hWndDlg, WORD *pwControlsList, size_t dwControlsListCount, BOOL bEnabled);
bool      MraRequestXStatusDetails(DWORD dwXStatus);
bool      MraSendReplyBlogStatus(MCONTACT hContact);
DWORD     GetYears(CONST PSYSTEMTIME pcstSystemTime);
DWORD     FindFile(LPWSTR lpszFolder, DWORD dwFolderLen, LPWSTR lpszFileName, DWORD dwFileNameLen, LPWSTR lpszRetFilePathName, DWORD dwRetFilePathLen, DWORD *pdwRetFilePathLen);

DWORD     GetMraStatusFromMiradaStatus(DWORD dwMirandaStatus, DWORD dwXStatusMir, DWORD *pdwXStatusMra);
DWORD     GetMirandaStatusFromMraStatus(DWORD dwMraStatus, DWORD dwXStatusMra, DWORD *pdwXStatusMir);
DWORD     GetMraXStatusIDFromMraUriStatus(const char *lpszStatusUri);

INT_PTR CALLBACK DlgProcAccount(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

#define LPS2ANSI(var, p, size) char *var = (char*)alloca(size+1); memcpy(var, p, size); var[size]=0;

#define NETLIB_CLOSEHANDLE(hConnection) {Netlib_CloseHandle(hConnection); hConnection = NULL;}

#endif // !defined(AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
