#if !defined(AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _CRT_SECURE_NO_WARNINGS

#if defined( UNICODE ) && !defined( _UNICODE )
	#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define WINVER       0x0502
#define _WIN32_WINNT 0x0502

#define MIRANDA_VER 0x0A00

#include <windows.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <PrSht.h>
#include <Commdlg.h>
#include <Ntsecapi.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>
#include <uxtheme.h>

// miranda API
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_cluiframes.h>
#include <m_clui.h>
#include <m_genmenu.h>
#include <m_ignore.h>
#include <m_message.h>
#include <m_options.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_updater.h>
#include <m_icolib.h>
#include <win2k.h>
#include <m_popup.h>
#include <m_popup2.h>
#include <m_nudge.h>
#include <m_folders.h>
#include <m_avatars.h>
#include <m_chat.h>
#include <m_extraicons.h>
#include <m_music.h>
#include <m_proto_listeningto.h>

//sdk
#include <timefuncs.h>
#include "zlib.h"
#include <DebugFunctions.h>
#include <SocketFunctions.h>
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

#include "resource.h"
#include ".\xstatus_MRA\resource.h"
#include "proto.h"
#include "MraSendQueue.h"
#include "MraFilesQueue.h"
#include "MraPopUp.h"
#include "MraAvatars.h"
#include "MraIcons.h"
#include "MraSelectEMail.h"
#include "MraAntiSpam.h"
#include "MraMPop.h"
#include "MraChat.h"

typedef int (*PUNCOMPRESS)(unsigned char*,DWORD*,unsigned char*,DWORD);
typedef int (*PCOMPRESS2)(unsigned char*,DWORD*,unsigned char*,DWORD,int);

// LPS
typedef struct
{
	SIZE_T	dwSize;

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

// структура содержащая информацию по построению меню или расширеных иконок
struct GUI_DISPLAY_ITEM
{
	LPSTR			lpszName;		// имя сервиса, оно же имя в иколибе
	LPWSTR			lpwszDescr;		// текстовое описание отображаемое юзеру
	LONG			defIcon;		// иконка из ресурсов
	MIRANDASERVICE	lpFunc;			// функция вызываемая меню
};

// структура содержащая информацию о сервисах/функциях
struct SERVICE_ITEM
{
	LPSTR			lpszName;		// имя сервиса, оно же имя в иколибе
	MIRANDASERVICE	lpFunc;			// функция вызываемая
};

#include "MraConstans.h"
#include "MraSendCommand.h"
#include "MraMRIMProxy.h"



/////////////////////////////////////////////////////////////////////////////////////////
//	plugin options


#define MAIN_MENU_ITEMS_COUNT		15
#define CONTACT_MENU_ITEMS_COUNT	10

typedef struct
{
	HANDLE				hHeap;
	HINSTANCE			hInstance;
	HMODULE				hDLLZLib;
	HANDLE				lpfnCompress2;
	HANDLE				lpfnUncompress;
	HMODULE				hDLLXStatusIcons;
	char				szModuleName[MAXMODULELABELLENGTH];
	WCHAR				wszModuleName[MAXMODULELABELLENGTH];
	SIZE_T				dwModuleNameLen;
	char				szDisplayModuleName[MAX_PATH];
	WCHAR				wszDisplayModuleName[MAX_PATH];
	SIZE_T				dwDisplayModuleNameLen;

	DWORD				dwGlobalPluginRunning;// 
	BOOL				bLoggedIn;
	BOOL				bChatExist;
	DWORD				dwStatusMode;
	DWORD				dwXStatusMode;
	DWORD				dwDesiredStatusMode;

	HWND				hWndEMailPopupStatus;
	DWORD				dwEmailMessagesTotal;
	DWORD				dwEmailMessagesUnRead;

	BOOL				bHideXStatusUI;
	HANDLE				hXStatusMenuItems[MRA_XSTATUS_COUNT+4];
	HANDLE				hXStatusAdvancedStatusIcons[MRA_XSTATUS_COUNT+4];
	HANDLE				hXStatusAdvancedStatusItems[MRA_XSTATUS_COUNT+4];
	HANDLE				hExtraXstatusIcon;

	HANDLE				hMainMenuIcons[MAIN_MENU_ITEMS_COUNT+4];
	HANDLE				hMainMenuItems[MAIN_MENU_ITEMS_COUNT+4];

	HANDLE				hContactMenuIcons[CONTACT_MENU_ITEMS_COUNT+4];
	HANDLE				hContactMenuItems[CONTACT_MENU_ITEMS_COUNT+4];

	HANDLE				hAdvancedStatusIcons[ADV_ICON_MAX+4];
	HANDLE				hAdvancedStatusItems[ADV_ICON_MAX+4];
	HANDLE				hExtraInfo;

	HANDLE				hThreadAPC;				// APC thread, for queue tasks
	HANDLE				hWaitEventThreadAPCHandle;
	DWORD				dwAPCThreadRunning;
	FIFO_MT				ffmtAPCQueue;
	HANDLE				hMPopSessionQueue;
	HANDLE				hSendQueueHandle;
	HANDLE				hFilesQueueHandle;
	HANDLE				hAvatarsQueueHandle;
	
	HANDLE				hNetlibUser;
	HANDLE				hThreadWorker;
	HANDLE				hConnection;
	DWORD				dwThreadWorkerLastPingTime;
	DWORD				dwThreadWorkerRunning;
	DWORD				dwCMDNum;
	CRITICAL_SECTION	csCriticalSectionSend;

	HANDLE				hHookModulesLoaded;
	HANDLE				hHookPreShutdown;
	HANDLE				hHookOptInitialize;
	HANDLE				hHookContactDeleted;
	HANDLE				hHookSettingChanged;
	HANDLE				hHookRebuildCMenu;
	HANDLE				hHookExtraIconsApply;
	HANDLE				hHookExtraIconsRebuild;
	HANDLE				hHookIconsChanged;
	HANDLE				heNudgeReceived;
	HANDLE				hHookRebuildStatusMenu;
	HANDLE				hWATrack;

	char				szNewMailSound[MAX_PATH];

	SIZE_T				dwMirWorkDirPathLen;
	WCHAR				szMirWorkDirPath[MAX_FILEPATH];
} MRA_SETTINGS;


/////////////////////////////////////////////////////////////////////////////////////////
//	External variables
extern MRA_SETTINGS masMraSettings;


INT_PTR			LoadServices	();
INT_PTR			LoadModules		();
void			UnloadModules	();
void			UnloadServices	();
void			InitExtraIcons	();
void			SetExtraIcons	(HANDLE hContact);
DWORD			MraSetXStatusInternal	(DWORD dwXStatus);
DWORD			MraGetXStatusInternal	();

int				OptInit			(WPARAM wParam,LPARAM lParam);


INT_PTR CALLBACK AdvancedSearchDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam);
DWORD			AdvancedSearchFromDlg(HWND hWndDlg);


DWORD			MraRecvCommand_Message			(DWORD dwTime,DWORD dwFlags,MRA_LPS *plpsFrom,MRA_LPS *plpsText,MRA_LPS *plpsRFTText,MRA_LPS *plpsMultiChatData);


DWORD			MraGetSelfVersionString		(LPSTR lpszSelfVersion,SIZE_T dwSelfVersionSize,SIZE_T *pdwSelfVersionSizeRet);

DWORD			MraAddrListGetFromBuff		(LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_ADDR_LIST *pmalAddrList);
DWORD			MraAddrListGetToBuff		(MRA_ADDR_LIST *pmalAddrList,LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet);
void			MraAddrListStoreToContact	(HANDLE hContact,MRA_ADDR_LIST *pmalAddrList);
void			MraAddrListFree				(MRA_ADDR_LIST *pmalAddrList);



#define GetContactNameA(Contact) (LPSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)Contact,0)
#define GetContactNameW(Contact) (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)Contact,GCDNF_UNICODE)

#define GetStatusModeDescriptionA(Status) (LPSTR)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)Status,0)
#define GetStatusModeDescriptionW(Status) (LPWSTR)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)Status,GSMDF_UNICODE)

#define DB_Mra_DeleteValue(Contact,valueName) DBDeleteContactSetting(Contact,PROTOCOL_NAMEA,valueName)

#define DB_Mra_GetDword(Contact,valueName,parDefltValue) DBGetContactSettingDword(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_Mra_SetDword(Contact,valueName,parValue) DBWriteContactSettingDword(Contact,PROTOCOL_NAMEA,valueName,parValue)

#define DB_Mra_GetWord(Contact,valueName,parDefltValue) DBGetContactSettingWord(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_Mra_SetWord(Contact,valueName,parValue) DBWriteContactSettingWord(Contact,PROTOCOL_NAMEA,valueName,parValue)

#define DB_Mra_GetByte(Contact,valueName,parDefltValue) DBGetContactSettingByte(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_Mra_SetByte(Contact,valueName,parValue) DBWriteContactSettingByte(Contact,PROTOCOL_NAMEA,valueName,parValue)

void	DB_Mra_CreateResidentSetting(LPSTR lpszValueName);

BOOL	DB_GetStaticStringA(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize);
BOOL	DB_GetStaticStringW(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize);
#define DB_Mra_GetStaticStringA(Contact,ValueName,Ret,RetBuffSize,pRetBuffSize) DB_GetStaticStringA(Contact,PROTOCOL_NAMEA,ValueName,Ret,RetBuffSize,pRetBuffSize)
#define DB_Mra_GetStaticStringW(Contact,ValueName,Ret,RetBuffSize,pRetBuffSize) DB_GetStaticStringW(Contact,PROTOCOL_NAMEA,ValueName,Ret,RetBuffSize,pRetBuffSize)

BOOL	DB_SetStringExA(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPSTR lpszValue,SIZE_T dwValueSize);
BOOL	DB_SetStringExW(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszValue,SIZE_T dwValueSize);
#define DB_SetStringA(Contact,Module,valueName,parValue) DB_SetStringExA(Contact,Module,valueName,parValue,lstrlenA(parValue))
#define DB_SetStringW(Contact,Module,valueName,parValue) DBWriteContactSettingWString(Contact,Module,valueName,parValue)
#define DB_SetLPSStringA(Contact,Module,valueName,parValue) if ((parValue)) DB_SetStringExA(Contact,Module,valueName,(parValue)->lpszData,(parValue)->dwSize)
#define DB_SetLPSStringW(Contact,Module,valueName,parValue) if ((parValue)) DB_SetStringExW(Contact,Module,valueName,(parValue)->lpwszData,((parValue)->dwSize/sizeof(WCHAR)))
#define DB_Mra_SetStringA(Contact,valueName,parValue) DB_SetStringExA(Contact,PROTOCOL_NAMEA,valueName,parValue,lstrlenA(parValue))
#define DB_Mra_SetStringW(Contact,valueName,parValue) DBWriteContactSettingWString(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_Mra_SetLPSStringA(Contact,valueName,parValue) DB_SetLPSStringA(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_Mra_SetLPSStringW(Contact,valueName,parValue) DB_SetLPSStringW(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_Mra_SetStringExA(Contact,valueName,parValue,parValueSize) DB_SetStringExA(Contact,PROTOCOL_NAMEA,valueName,parValue,parValueSize)
#define DB_Mra_SetStringExW(Contact,valueName,parValue,parValueSize) DB_SetStringExW(Contact,PROTOCOL_NAMEA,valueName,parValue,parValueSize)

int DB_WriteContactSettingBlob(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPVOID lpValue,SIZE_T dwValueSize);
#define DB_Mra_WriteContactSettingBlob(hContact,lpszValueName,lpbValue,dwValueSize) DB_WriteContactSettingBlob(hContact,PROTOCOL_NAMEA,lpszValueName,lpbValue,dwValueSize)
BOOL DB_GetContactSettingBlob(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPVOID lpRet,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize);
#define DB_Mra_GetContactSettingBlob(hContact,lpszValueName,lpbRet,dwRetBuffSize,pdwRetBuffSize) DB_GetContactSettingBlob(hContact,PROTOCOL_NAMEA,lpszValueName,lpbRet,dwRetBuffSize,pdwRetBuffSize)

#define MEMALLOC(Size)			HeapAlloc(masMraSettings.hHeap,HEAP_ZERO_MEMORY,((Size)+sizeof(void*)))
#define MEMREALLOC(Mem,Size)	HeapReAlloc(masMraSettings.hHeap,(HEAP_ZERO_MEMORY),(void*)(Mem),((Size)+sizeof(void*)))
#define MEMFREE(Mem)			if ((Mem)) {HeapFree(masMraSettings.hHeap,0,(void*)(Mem));(Mem)=NULL;}

#define SetBit(bytes,bitpos) bytes|=(1<<bitpos)
#define GetBit(bytes,bitpos) ((bytes&(1<<bitpos))? TRUE:FALSE)


#define IsXStatusValid(XStatus) (((XStatus) && (XStatus)<MRA_XSTATUS_COUNT))


#define SEND_DLG_ITEM_MESSAGEW(hDlg,nIDDlgItem,Msg,wParam,lParam)	SendMessageW(GetDlgItem(hDlg,nIDDlgItem),Msg,wParam,lParam)
#define SEND_DLG_ITEM_MESSAGEA(hDlg,nIDDlgItem,Msg,wParam,lParam)	SendMessageA(GetDlgItem(hDlg,nIDDlgItem),Msg,wParam,lParam)
#define SEND_DLG_ITEM_MESSAGE(hDlg,nIDDlgItem,Msg,wParam,lParam)	SendMessage(GetDlgItem(hDlg,nIDDlgItem),Msg,wParam,lParam)

#define IS_DLG_BUTTON_CHECKED(hDlg,nIDDlgItem)						SEND_DLG_ITEM_MESSAGE(hDlg,nIDDlgItem,BM_GETCHECK,NULL,NULL)
#define CHECK_DLG_BUTTON(hDlg,nIDDlgItem,uCheck)					SEND_DLG_ITEM_MESSAGE(hDlg,nIDDlgItem,BM_SETCHECK,(WPARAM)uCheck,NULL)

#define SET_DLG_ITEM_TEXTW(hDlg,nIDDlgItem,lpString)				SEND_DLG_ITEM_MESSAGEW(hDlg,nIDDlgItem,WM_SETTEXT,0,(LPARAM)lpString)
#define SET_DLG_ITEM_TEXTA(hDlg,nIDDlgItem,lpString)				SEND_DLG_ITEM_MESSAGEA(hDlg,nIDDlgItem,WM_SETTEXT,0,(LPARAM)lpString)
#define SET_DLG_ITEM_TEXT(hDlg,nIDDlgItem,lpString)					SEND_DLG_ITEM_MESSAGE(hDlg,nIDDlgItem,WM_SETTEXT,0,(LPARAM)lpString)

#define GET_DLG_ITEM_TEXTW(hDlg,nIDDlgItem,lpString,nMaxCount)		SEND_DLG_ITEM_MESSAGEW(hDlg,nIDDlgItem,WM_GETTEXT,(WPARAM)nMaxCount,(LPARAM)lpString)
#define GET_DLG_ITEM_TEXTA(hDlg,nIDDlgItem,lpString,nMaxCount)		SEND_DLG_ITEM_MESSAGEA(hDlg,nIDDlgItem,WM_GETTEXT,(WPARAM)nMaxCount,(LPARAM)lpString)
#define GET_DLG_ITEM_TEXT(hDlg,nIDDlgItem,lpString,nMaxCount)		SEND_DLG_ITEM_MESSAGE(hDlg,nIDDlgItem,WM_GETTEXT,(WPARAM)nMaxCount,(LPARAM)lpString)

#define GET_DLG_ITEM_TEXT_LENGTH(hDlg,nIDDlgItem)					SEND_DLG_ITEM_MESSAGE(hDlg,nIDDlgItem,WM_GETTEXTLENGTH,NULL,NULL)
#define GET_WINDOW_TEXT_LENGTH(hDlg)								SendMessage(hDlg,WM_GETTEXTLENGTH,NULL,NULL)

#define GET_CURRENT_COMBO_DATA(hWndDlg,ControlID)					SEND_DLG_ITEM_MESSAGE(hWndDlg,ControlID,CB_GETITEMDATA,SEND_DLG_ITEM_MESSAGE(hWndDlg,ControlID,CB_GETCURSEL,0,0),0)


#define IsFileExist(FileName) (GetFileAttributes(FileName)!=INVALID_FILE_ATTRIBUTES)
#define IsFileExistA(FileName) (GetFileAttributesA(FileName)!=INVALID_FILE_ATTRIBUTES)
#define IsFileExistW(FileName) (GetFileAttributesW(FileName)!=INVALID_FILE_ATTRIBUTES)


#define IsThreadAlive(hThread) (GetThreadPriority(hThread)!=THREAD_PRIORITY_ERROR_RETURN)



DWORD			MraGetVersionStringFromFormated	(LPSTR lpszUserAgentFormated,SIZE_T dwUserAgentFormatedSize,LPSTR lpszVersion,SIZE_T dwVersionSize,SIZE_T *pdwVersionSizeRet);
HANDLE			MraHContactFromEmail			(LPSTR lpszEMail,SIZE_T dwEMailSize,BOOL bAddIfNeeded,BOOL bTemporary,BOOL *pbAdded);
BOOL			MraUpdateContactInfo			(HANDLE hContact);
DWORD			MraContactCapabilitiesGet		(HANDLE hContact);
void			MraContactCapabilitiesSet		(HANDLE hContact,DWORD dwFutureFlags);
DWORD			MraGetContactStatus				(HANDLE hContact);
DWORD			MraSetContactStatus				(HANDLE hContact,DWORD dwNewStatus);
void			MraUpdateEmailStatus			(LPSTR lpszFrom,SIZE_T dwFromSize,LPSTR lpszSubject,SIZE_T dwSubjectSize,DWORD dwDate,DWORD dwUIDL);
BOOL			IsUnicodeEnv					();
BOOL			IsHTTPSProxyUsed				(HANDLE hNetlibUser);
BOOL			IsContactMra					(HANDLE hContact);
BOOL			IsContactMraProto				(HANDLE hContact);
BOOL			IsEMailMy						(LPSTR lpszEMail,SIZE_T dwEMailSize);
BOOL			IsEMailChatAgent				(LPSTR lpszEMail,SIZE_T dwEMailSize);
BOOL			IsContactChatAgent				(HANDLE hContact);
BOOL			IsEMailMR						(LPSTR lpszEMail,SIZE_T dwEMailSize);
BOOL			GetEMailFromString				(LPSTR lpszBuff,SIZE_T dwBuffSize,LPSTR *plpszEMail,SIZE_T *pdwEMailSize);
DWORD			GetContactEMailCount			(HANDLE hContact,BOOL bMRAOnly);
BOOL			GetContactFirstEMail			(HANDLE hContact,BOOL bMRAOnly,LPSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize);
DWORD			MraAPCQueueAdd					(PAPCFUNC pfnAPC,PFIFO_MT pffmtAPCQueue,ULONG_PTR dwData);
void			MraAPCQueueDestroy				(PFIFO_MT pffmtAPCQueue);
void			ShowFormatedErrorMessage		(LPWSTR lpwszErrText,DWORD dwErrorCode);
DWORD			ProtoBroadcastAckAsynchEx		(const char *szModule,HANDLE hContact,int type,int result,HANDLE hProcess,LPARAM lParam,SIZE_T dwLparamSize);
void			MraUserAPCThreadProc			(LPVOID lpParameter);

DWORD			CreateBlobFromContact			(HANDLE hContact,LPWSTR lpwszRequestReason,SIZE_T dwRequestReasonSize,LPBYTE lpbBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet);
void			CListCreateMenu					(LONG lPosition,LONG lPopupPosition,HANDLE hMainIcon,LPSTR pszContactOwner,LPSTR lpszCListMenuType,const GUI_DISPLAY_ITEM *pgdiItems,HANDLE *hIcoLibIcons,SIZE_T dwCount,HANDLE *hResult);
void			CListDestroyMenu				(const GUI_DISPLAY_ITEM *pgdiItems,SIZE_T dwCount);
void			CListShowMenuItem				(HANDLE hMenuItem,BOOL bShow);
//void			CListSetMenuItemIcon			(HANDLE hMenuItem,HANDLE hIcon);
int				ExtraSetIcon					(HANDLE hExtraIcon,HANDLE hContact,HANDLE hImage,int iColumnType);
SIZE_T			CopyNumber						(LPCVOID lpcOutBuff,LPCVOID lpcBuff,SIZE_T dwLen);
BOOL			IsPhone							(LPSTR lpszString,SIZE_T dwStringSize);
BOOL			IsContactPhone					(HANDLE hContact,LPSTR lpszPhone,SIZE_T dwPhoneSize);
HANDLE			MraHContactFromPhone			(LPSTR lpszPhone,SIZE_T dwPhoneSize,BOOL bAddIfNeeded,BOOL bTemporary,BOOL *pbAdded);
void			EnableControlsArray				(HWND hWndDlg,WORD *pwControlsList,SIZE_T dwControlsListCount,BOOL bEnabled);
BOOL			MraRequestXStatusDetails		(DWORD dwXStatus);
BOOL			MraSendReplyBlogStatus			(HANDLE hContact);
DWORD			GetYears						(CONST PSYSTEMTIME pcstSystemTime);
DWORD			FindFile						(LPWSTR lpszFolder,DWORD dwFolderLen,LPWSTR lpszFileName,DWORD dwFileNameLen,LPWSTR lpszRetFilePathName,DWORD dwRetFilePathLen,DWORD *pdwRetFilePathLen);
DWORD			MemFillRandom					(LPVOID lpBuff,SIZE_T dwBuffSize);
BOOL			SetPassDB						(LPSTR lpszBuff,SIZE_T dwBuffSize);
BOOL			GetPassDB						(LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSize);
DWORD			DecodeXML						(LPTSTR lptszMessage,SIZE_T dwMessageSize,LPTSTR lptszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize);
DWORD			EncodeXML						(LPTSTR lptszMessage,SIZE_T dwMessageSize,LPTSTR lptszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize);


DWORD			StartConnect					();
void			MraThreadClean					();

DWORD			GetMraStatusFromMiradaStatus	(DWORD dwMirandaStatus,DWORD dwXStatusMir,DWORD *pdwXStatusMra);
DWORD			GetMiradaStatusFromMraStatus	(DWORD dwMraStatus,DWORD dwXStatusMra,DWORD *pdwXStatusMir);
DWORD			GetMraXStatusIDFromMraUriStatus	(LPSTR lpszStatusUri,SIZE_T dwStatusUriSize);


DWORD			GetContactFlags					(HANDLE hContact);
DWORD			SetContactFlags					(HANDLE hContact,DWORD dwContactFlag);

DWORD			GetContactBasicInfoW			(HANDLE hContact,DWORD *pdwID,DWORD *pdwGroupID,DWORD *pdwContactFlag,DWORD *pdwContactSeverFlags,DWORD *pdwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,SIZE_T *pdwEMailSize,LPWSTR lpwszNick,SIZE_T dwNickSize,SIZE_T *pdwNickSize,LPSTR lpszPhones,SIZE_T dwPhonesSize,SIZE_T *pdwPhonesSize);

#define SCBIFSI_LOCK_CHANGES_EVENTS	1
#define SCBIF_ID			1
#define SCBIF_GROUP_ID		2
#define SCBIF_FLAG			4
#define SCBIF_SERVER_FLAG	8
#define SCBIF_STATUS		16
#define SCBIF_EMAIL			32
#define SCBIF_NICK			64
#define SCBIF_PHONES		128
DWORD			SetContactBasicInfoW			(HANDLE hContact,DWORD dwSetInfoFlags,DWORD dwFlags,DWORD dwID,DWORD dwGroupID,DWORD dwContactFlag,DWORD dwContactSeverFlags,DWORD dwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszNick,SIZE_T dwNickSize,LPSTR lpszPhones,SIZE_T dwPhonesSize);




#endif // !defined(AFX_MRA_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
