#ifndef _COMMON_H
#define _COMMON_H

#define _WIN32_WINNT 	0x0500
#define _WIN32_IE 		0x0400

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN


//////#define _USE_32BIT_TIME_T
#define _CRT_SECURE_NO_WARNINGS

//#define UNICODE
//#define _UNICODE

/*#ifdef NDEBUG
	#pragma optimize("gsy",on)
	#pragma comment(linker,"/subsystem:Windows,5")
	#pragma comment(linker,"/version:5")

	#pragma comment(linker,"/nodefaultlib")
	//#pragma comment(linker,"/GS")
	//#pragma comment(linker,"/merge:.rdata=.text /merge:.data=.text")
	//#pragma comment(linker,"/verbose")
	//#pragma comment(linker,"/noentry")
	#pragma comment(linker,"/ENTRY:DllMain")

#endif //_DEBUG*/

	//#pragma comment(linker,"/nodefaultlib")
	//#pragma comment(linker,"/noentry")
	//#pragma comment(linker,"/ENTRY:DllMain")

//#define CRTDLL

#define MIRANDA_VER 0x0A00

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
/*#ifdef NDEBUG
	#include <..\minicrt\minicrt.h>
#else
	#include <..\minicrt\timefuncs.h>
#endif //_DEBUG*/

#include "AdditionalFunctions/ListMT.h"
#include "AdditionalFunctions/DebugFunctions.h"
#include "AdditionalFunctions/MemoryCompare.h"
#include "AdditionalFunctions/MemoryFind.h"
#include "AdditionalFunctions/MemoryFindByte.h"
#include "AdditionalFunctions/BuffToLowerCase.h"

#include <newpluginapi.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_history.h>
#include <m_protomod.h>
#include <m_autoreplacer.h>
#include <m_skin.h>
#include <m_protosvc.h>
#include <m_icq.h>
#include <m_protosvc.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_options.h>
#include <win2k.h>

#include "resource.h"
#include "version.h"

extern HINSTANCE hInst;

// структура содержащая информацию по построению меню или расширеных иконок
struct GUI_DISPLAY_ITEM
{
	LPSTR	lpszName;		// имя сервиса, оно же имя в иколибе
	LPWSTR	lpwszDescr;		// текстовое описание отображаемое юзеру
	LONG	defIcon;		// иконка из ресурсов
	LPVOID	lpFunc;			// функция вызываемая меню
};


// структура содержащая информацию о сервисах/функциях
struct SERVICE_ITEM
{
	LPSTR	lpszName;		// имя сервиса, оно же имя в иколибе
	LPVOID	lpFunc;			// функция вызываемая
};


#include "SMSConstans.h"
#include "senddlg.h"
#include "recvdlg.h"


#define MAIN_MENU_ITEMS_COUNT		1
#define CONTACT_MENU_ITEMS_COUNT	1


typedef struct
{
	HANDLE				hHeap;
	HINSTANCE			hInstance;

	//HANDLE			hMainMenuIcons[MAIN_MENU_ITEMS_COUNT+1];
	HANDLE				hMainMenuItems[MAIN_MENU_ITEMS_COUNT+1];

	//HANDLE			hContactMenuIcons[CONTACT_MENU_ITEMS_COUNT+1];
	HANDLE				hContactMenuItems[CONTACT_MENU_ITEMS_COUNT+1];

	HANDLE				hHookModulesLoaded;
	HANDLE				hHookPreShutdown;
	HANDLE				hHookOptInitialize;
	HANDLE				hHookRebuildCMenu;
	HANDLE				hHookDbAdd;
	HANDLE				hHookProtoAck;
	HANDLE				hHookAccListChanged;

	LIST_MT				lmtSendSMSWindowsListMT;
	LIST_MT				lmtRecvSMSWindowsListMT;

	PROTOACCOUNT		**ppaSMSAccounts;
	SIZE_T				dwSMSAccountsCount;

} SMS_SETTINGS;



extern SMS_SETTINGS ssSMSSettings;




#define MEMALLOC(Size)			HeapAlloc(ssSMSSettings.hHeap,HEAP_ZERO_MEMORY,(Size+sizeof(SIZE_T)))
#define MEMREALLOC(Mem,Size)	HeapReAlloc(ssSMSSettings.hHeap,(HEAP_ZERO_MEMORY),(LPVOID)Mem,(Size+sizeof(SIZE_T)))
#define MEMFREE(Mem)			if (Mem) {HeapFree(ssSMSSettings.hHeap,0,(LPVOID)Mem);Mem=NULL;}

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




#define GetContactNameA(Contact) (LPSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)Contact,0)
#define GetContactNameW(Contact) (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)Contact,GCDNF_UNICODE)

#define DB_SMS_DeleteValue(Contact,valueName) DBDeleteContactSetting(Contact,PROTOCOL_NAMEA,valueName)
#define DB_SMS_GetDword(Contact,valueName,parDefltValue) DBGetContactSettingDword(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_SMS_SetDword(Contact,valueName,parValue) DBWriteContactSettingDword(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_SMS_GetWord(Contact,valueName,parDefltValue) DBGetContactSettingWord(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_SMS_SetWord(Contact,valueName,parValue) DBWriteContactSettingWord(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_SMS_GetByte(Contact,valueName,parDefltValue) DBGetContactSettingByte(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_SMS_SetByte(Contact,valueName,parValue) DBWriteContactSettingByte(Contact,PROTOCOL_NAMEA,valueName,parValue)
BOOL	DB_GetStaticStringW(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize);
#define DB_SMS_GetStaticStringW(Contact,ValueName,Ret,RetBuffSize,pRetBuffSize) DB_GetStaticStringW(Contact,PROTOCOL_NAMEA,ValueName,Ret,RetBuffSize,pRetBuffSize)
BOOL	DB_SetStringExW(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszValue,SIZE_T dwValueSize);
#define DB_SetStringW(Contact,Module,valueName,parValue) DBWriteContactSettingWString(Contact,Module,valueName,parValue)
#define DB_SMS_SetStringW(Contact,valueName,parValue) DBWriteContactSettingWString(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_SMS_SetStringExW(Contact,valueName,parValue,parValueSize) DB_SetStringExW(Contact,PROTOCOL_NAMEA,valueName,parValue,parValueSize)


LPSTR			GetModuleName					(HANDLE hContact);
void			EnableControlsArray				(HWND hWndDlg,WORD *pwControlsList,SIZE_T dwControlsListCount,BOOL bEnabled);
void			CListShowMenuItem				(HANDLE hMenuItem,BOOL bShow);
//Decleration of function that returns received string with only numbers
SIZE_T			CopyNumberA						(LPSTR lpszOutBuff,LPSTR lpszBuff,SIZE_T dwLen);
SIZE_T			CopyNumberW						(LPWSTR lpcOutBuff,LPWSTR lpcBuff,SIZE_T dwLen);
BOOL			IsPhoneW						(LPWSTR lpwszString,SIZE_T dwStringLen);
DWORD			GetContactPhonesCount			(HANDLE hContact);
BOOL			IsContactPhone					(HANDLE hContact,LPWSTR lpwszPhone,SIZE_T dwPhoneSize);
//Decleration of function that returns HANDLE of contact by his cellular number
HANDLE			HContactFromPhone				(LPWSTR lpwszPhone,SIZE_T dwPhoneSize);
BOOL			GetDataFromMessage				(LPSTR lpszMessage,SIZE_T dwMessageSize,DWORD *pdwEventType,LPWSTR lpwszPhone,SIZE_T dwPhoneSize,SIZE_T *pdwPhoneSizeRet,UINT *piIcon);
//Decleration of function that gets a XML string and return the asked tag.
BOOL			GetXMLFieldEx					(LPSTR lpszXML,SIZE_T dwXMLSize,LPSTR *plpszData,SIZE_T *pdwDataSize,const char *tag1,...);
BOOL			GetXMLFieldExBuff				(LPSTR lpszXML,SIZE_T dwXMLSize,LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet,const char *tag1,...);
//BOOL			GetXMLFieldExW					(LPWSTR lpwszXML,SIZE_T dwXMLSize,LPWSTR *plpwszData,SIZE_T *pdwDataSize,const WCHAR *tag1,...);
//BOOL			GetXMLFieldExBuffW				(LPWSTR lpwszXML,SIZE_T dwXMLSize,LPWSTR lpwszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet,const WCHAR *tag1,...);
DWORD			DecodeXML						(LPTSTR lptszMessage,SIZE_T dwMessageSize,LPTSTR lptszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize);
DWORD			EncodeXML						(LPTSTR lptszMessage,SIZE_T dwMessageSize,LPTSTR lptszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize);
void			LoadMsgDlgFont					(int i,LOGFONT *lf,COLORREF *colour);
LRESULT CALLBACK MessageSubclassProc			(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
int				RefreshAccountList				(WPARAM eventCode,LPARAM lParam);
void			FreeAccountList					();
//BOOL			IsModuleInAccountList			(LPSTR lpszModule);




int				OptInitialise	(WPARAM wParam,LPARAM lParam);

int				LoadServices	();
int				LoadModules		();
void			UnloadModules	();
void			UnloadServices	();

int handleAckSMS(WPARAM wParam,LPARAM lParam);
int handleNewMessage(WPARAM wParam,LPARAM lParam);
void RestoreUnreadMessageAlerts();

//Decleration of Menu SMS send click function
int SmsRebuildContactMenu(WPARAM wParam,LPARAM lParam);


void StartSmsSend(HWND hWndDlg,SIZE_T dwModuleIndex,LPWSTR lpwszPhone,SIZE_T dwPhoneSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize);




#endif