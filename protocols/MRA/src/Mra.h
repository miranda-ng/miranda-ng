#if !defined(AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _CRT_SECURE_NO_WARNINGS

#pragma warning(disable:4267)

//#define CRTDLL

#include "resource.h"
#include "..\xstatus_MRA\src\resource.h"
#include <SystemHeaders.h>
#include <Wincrypt.h>
#include <Ntsecapi.h>
#include <Commdlg.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <DebugFunctions.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <SocketFunctions.h>

#define bzero(pvDst, count) memset(pvDst, 0, count)
#include <timefuncs.h>

#include "../../plugins/zlib/src/zlib.h"

#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_clist.h>
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
#include <m_proto_listeningto.h>


#include <MemoryFind.h>
#include <MemoryFindByte.h>
#include <MemoryCompare.h>
#include <MemoryReplace.h>
#include <BuffToLowerCase.h>
#include <StrToNum.h>
#include <StrToNum.h>
#include <StrHexToNum.h>
#include <Base64.h>
#include <InternetTime.h>
#include <ListMT.h>
#include <FIFOMT.h>
#include <RC4.h>
#include <SHA1.h>


// LPS
typedef struct
{
	size_t	dwSize;

	union {
		LPSTR	lpszData;
		LPWSTR	lpwszData;
	};
} MRA_LPS;


// GUID
typedef struct
{
	DWORD	id[4];
} MRA_GUID;



struct MRA_ADDR_LIST_ITEM
{
	DWORD	dwPort;
	DWORD	dwAddr;
};

struct MRA_ADDR_LIST
{
	DWORD				dwAddrCount;
	MRA_ADDR_LIST_ITEM	*pmaliAddress;
};

#include "MraConstans.h"
#include "MraProto.h"

// структура содержащая информацию по построению меню или расширеных иконок
struct GUI_DISPLAY_ITEM
{
	LPSTR         lpszName;    // имя сервиса, оно же имя в иколибе
	LPWSTR        lpwszDescr;  // текстовое описание отображаемое юзеру
	LONG          defIcon;     // иконка из ресурсов
	CMraProto::MyServiceFunc lpFunc;      // функция вызываемая меню
	HANDLE        hIconHandle;
};

// структура содержащая информацию о сервисах/функциях
struct SERVICE_ITEM
{
	LPSTR         lpszName;		// имя сервиса, оно же имя в иколибе
	CMraProto::MyServiceFunc lpFunc;			// функция вызываемая
};

extern GUI_DISPLAY_ITEM gdiMenuItems[];
extern GUI_DISPLAY_ITEM gdiContactMenuItems[];
extern GUI_DISPLAY_ITEM gdiExtraStatusIconsItems[];

#include "proto.h"
#include "MraSendCommand.h"
#include "MraMRIMProxy.h"
#include "MraSendQueue.h"
#include "MraFilesQueue.h"
#include "MraPopup.h"
#include "MraAvatars.h"
#include "MraIcons.h"
#include "MraSelectEMail.h"
#include "MraAntiSpam.h"
#include "MraMPop.h"
#include "MraChat.h"


#ifdef MRA_EXPORTS
	#define MRA_API __declspec(dllexport)
#else
	#define MRA_API
	// __declspec(dllimport)
	#pragma comment(lib, "MRALib.lib")
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//	plugin options

typedef struct
{
	HANDLE                hHeap;
	HINSTANCE             hInstance;
	HMODULE               hDLLXStatusIcons;

	DWORD                 dwGlobalPluginRunning;
	BOOL                  bChatExist;

	size_t                dwMirWorkDirPathLen;
	WCHAR                 szMirWorkDirPath[MAX_FILEPATH];
} MRA_SETTINGS;


/////////////////////////////////////////////////////////////////////////////////////////
//	External variables
extern MRA_SETTINGS masMraSettings;
extern LIST<CMraProto> g_Instances;

extern HANDLE hXStatusAdvancedStatusIcons[];

INT_PTR LoadModules();
void    UnloadModules();
void    InitExtraIcons();

DWORD   MraGetSelfVersionString(LPSTR lpszSelfVersion, size_t dwSelfVersionSize, size_t *pdwSelfVersionSizeRet);

#define GetContactNameA(Contact) (LPSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)Contact, 0)
#define GetContactNameW(Contact) (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)Contact, GCDNF_UNICODE)

#define GetStatusModeDescriptionA(Status) (LPSTR)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)Status, 0)
#define GetStatusModeDescriptionW(Status) (LPWSTR)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)Status, GSMDF_UNICODE)

#define SetBit(bytes, bitpos) bytes |= (1<<bitpos)
#define GetBit(bytes, bitpos) ((bytes&(1<<bitpos))? TRUE:FALSE)

#define IsXStatusValid(XStatus) (((XStatus) && (XStatus)<MRA_XSTATUS_COUNT))

#define SEND_DLG_ITEM_MESSAGEW(hDlg, nIDDlgItem, Msg, wParam, lParam)	SendMessageW(GetDlgItem(hDlg, nIDDlgItem), Msg, wParam, lParam)
#define SEND_DLG_ITEM_MESSAGEA(hDlg, nIDDlgItem, Msg, wParam, lParam)	SendMessageA(GetDlgItem(hDlg, nIDDlgItem), Msg, wParam, lParam)
#define SEND_DLG_ITEM_MESSAGE(hDlg, nIDDlgItem, Msg, wParam, lParam)	SendMessage(GetDlgItem(hDlg, nIDDlgItem), Msg, wParam, lParam)

#define IS_DLG_BUTTON_CHECKED(hDlg, nIDDlgItem)						SEND_DLG_ITEM_MESSAGE(hDlg, nIDDlgItem, BM_GETCHECK, NULL, NULL)
#define CHECK_DLG_BUTTON(hDlg, nIDDlgItem, uCheck)					SEND_DLG_ITEM_MESSAGE(hDlg, nIDDlgItem, BM_SETCHECK, (WPARAM)uCheck, NULL)

#define SET_DLG_ITEM_TEXTW(hDlg, nIDDlgItem, lpString)				SEND_DLG_ITEM_MESSAGEW(hDlg, nIDDlgItem, WM_SETTEXT, 0, (LPARAM)lpString)
#define SET_DLG_ITEM_TEXTA(hDlg, nIDDlgItem, lpString)				SEND_DLG_ITEM_MESSAGEA(hDlg, nIDDlgItem, WM_SETTEXT, 0, (LPARAM)lpString)
#define SET_DLG_ITEM_TEXT(hDlg, nIDDlgItem, lpString)					SEND_DLG_ITEM_MESSAGE(hDlg, nIDDlgItem, WM_SETTEXT, 0, (LPARAM)lpString)

#define GET_DLG_ITEM_TEXTW(hDlg, nIDDlgItem, lpString, nMaxCount)		SEND_DLG_ITEM_MESSAGEW(hDlg, nIDDlgItem, WM_GETTEXT, (WPARAM)nMaxCount, (LPARAM)lpString)
#define GET_DLG_ITEM_TEXTA(hDlg, nIDDlgItem, lpString, nMaxCount)		SEND_DLG_ITEM_MESSAGEA(hDlg, nIDDlgItem, WM_GETTEXT, (WPARAM)nMaxCount, (LPARAM)lpString)
#define GET_DLG_ITEM_TEXT(hDlg, nIDDlgItem, lpString, nMaxCount)		SEND_DLG_ITEM_MESSAGE(hDlg, nIDDlgItem, WM_GETTEXT, (WPARAM)nMaxCount, (LPARAM)lpString)

#define GET_DLG_ITEM_TEXT_LENGTH(hDlg, nIDDlgItem)					SEND_DLG_ITEM_MESSAGE(hDlg, nIDDlgItem, WM_GETTEXTLENGTH, NULL, NULL)
#define GET_WINDOW_TEXT_LENGTH(hDlg)								SendMessage(hDlg, WM_GETTEXTLENGTH, NULL, NULL)

#define GET_CURRENT_COMBO_DATA(hWndDlg, ControlID)					SEND_DLG_ITEM_MESSAGE(hWndDlg, ControlID, CB_GETITEMDATA, SEND_DLG_ITEM_MESSAGE(hWndDlg, ControlID, CB_GETCURSEL, 0, 0), 0)


#define IsFileExist(FileName) (GetFileAttributes(FileName) != INVALID_FILE_ATTRIBUTES)
#define IsFileExistA(FileName) (GetFileAttributesA(FileName) != INVALID_FILE_ATTRIBUTES)
#define IsFileExistW(FileName) (GetFileAttributesW(FileName) != INVALID_FILE_ATTRIBUTES)


#define IsThreadAlive(hThread) (GetThreadPriority(hThread) != THREAD_PRIORITY_ERROR_RETURN)

void   MraAddrListFree(MRA_ADDR_LIST *pmalAddrList);
DWORD  MraAddrListGetFromBuff(LPSTR lpszAddreses, size_t dwAddresesSize, MRA_ADDR_LIST *pmalAddrList);
DWORD  MraAddrListGetToBuff(MRA_ADDR_LIST *pmalAddrList, LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSizeRet);

void   InitXStatusIcons();
void   DestroyXStatusIcons();

DWORD  MraGetVersionStringFromFormatted (LPSTR dwUserAgentFormatted, size_t dwUserAgentFormattedSize, LPSTR lpszVersion, size_t dwVersionSize, size_t *pdwVersionSizeRet);
BOOL   IsUnicodeEnv                    ();
BOOL   IsHTTPSProxyUsed                (HANDLE hNetlibUser);
BOOL   IsContactMraProto               (HANDLE hContact);
BOOL   IsEMailMR                       (LPSTR lpszEMail, size_t dwEMailSize);
BOOL   GetEMailFromString              (LPSTR lpszBuff, size_t dwBuffSize, LPSTR *plpszEMail, size_t *pdwEMailSize);

DWORD  CreateBlobFromContact           (HANDLE hContact, LPWSTR lpwszRequestReason, size_t dwRequestReasonSize, LPBYTE lpbBuff, size_t dwBuffSize, size_t *pdwBuffSizeRet);
size_t CopyNumber                      (LPCVOID lpcOutBuff, LPCVOID lpcBuff, size_t dwLen);
BOOL   IsPhone                         (LPSTR lpszString, size_t dwStringSize);
BOOL   IsContactPhone                  (HANDLE hContact, LPSTR lpszPhone, size_t dwPhoneSize);
HANDLE MraHContactFromPhone            (LPSTR lpszPhone, size_t dwPhoneSize, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded);
void   EnableControlsArray             (HWND hWndDlg, WORD *pwControlsList, size_t dwControlsListCount, BOOL bEnabled);
BOOL   MraRequestXStatusDetails        (DWORD dwXStatus);
BOOL   MraSendReplyBlogStatus          (HANDLE hContact);
DWORD  GetYears                        (CONST PSYSTEMTIME pcstSystemTime);
DWORD  FindFile                        (LPWSTR lpszFolder, DWORD dwFolderLen, LPWSTR lpszFileName, DWORD dwFileNameLen, LPWSTR lpszRetFilePathName, DWORD dwRetFilePathLen, DWORD *pdwRetFilePathLen);
DWORD  MemFillRandom                   (LPVOID lpBuff, size_t dwBuffSize);
DWORD  DecodeXML                       (LPTSTR lptszMessage, size_t dwMessageSize, LPTSTR lptszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize);
DWORD  EncodeXML                       (LPTSTR lptszMessage, size_t dwMessageSize, LPTSTR lptszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize);

DWORD  GetMraStatusFromMiradaStatus    (DWORD dwMirandaStatus, DWORD dwXStatusMir, DWORD *pdwXStatusMra);
DWORD  GetMiradaStatusFromMraStatus    (DWORD dwMraStatus, DWORD dwXStatusMra, DWORD *pdwXStatusMir);
DWORD  GetMraXStatusIDFromMraUriStatus (LPSTR lpszStatusUri, size_t dwStatusUriSize);

INT_PTR CALLBACK DlgProcAccount(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

#endif // !defined(AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
