#ifndef _COMMON_H
#define _COMMON_H

#define _WIN32_WINNT 	0x0500
#define _WIN32_IE 		0x0400

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_history.h>
#include <m_skin.h>
#include <m_protosvc.h>
#include <m_icq.h>
#include <m_options.h>
#include <win2k.h>

#include "AdditionalFunctions/InterlockedFunctions.h"
#include "AdditionalFunctions/ListMT.h"
#include "AdditionalFunctions/MemoryCompare.h"
#include "AdditionalFunctions/MemoryFindByte.h"
#include "resource.h"
#include "version.h"
#include "recvdlg.h"
#include "SMSConstans.h"
#include "senddlg.h"

extern HINSTANCE hInst;

// структура содержащая информацию по построению меню или расширеных иконок
struct GUI_DISPLAY_ITEM
{
	LPSTR  lpszName;    // имя сервиса, оно же имя в иколибе
	LPWSTR lpwszDescr;  // текстовое описание отображаемое юзеру
	LONG   defIcon;     // иконка из ресурсов
	LPVOID lpFunc;      // функция вызываемая меню
};


#define MAIN_MENU_ITEMS_COUNT		1
#define CONTACT_MENU_ITEMS_COUNT	1


typedef struct
{
	HANDLE         hHeap;
	HINSTANCE      hInstance;

	HGENMENU       hMainMenuItems[MAIN_MENU_ITEMS_COUNT+1];
	HGENMENU       hContactMenuItems[CONTACT_MENU_ITEMS_COUNT+1];

	LIST_MT        lmtSendSMSWindowsListMT;
	LIST_MT        lmtRecvSMSWindowsListMT;

	PROTOACCOUNT **ppaSMSAccounts;
	size_t         dwSMSAccountsCount;

} SMS_SETTINGS;



extern SMS_SETTINGS ssSMSSettings;




#define MEMALLOC(Size)			HeapAlloc(ssSMSSettings.hHeap,HEAP_ZERO_MEMORY,(Size+sizeof(size_t)))
#define MEMREALLOC(Mem,Size)	HeapReAlloc(ssSMSSettings.hHeap,(HEAP_ZERO_MEMORY),(LPVOID)Mem,(Size+sizeof(size_t)))
#define MEMFREE(Mem)			if (Mem) {HeapFree(ssSMSSettings.hHeap,0,(LPVOID)Mem);Mem=NULL;}


#define GET_DLG_ITEM_TEXT_LENGTH(hDlg,nIDDlgItem) SendDlgItemMessage(hDlg,nIDDlgItem,WM_GETTEXTLENGTH,NULL,NULL)
#define GET_CURRENT_COMBO_DATA(hWndDlg,ControlID) SendDlgItemMessage(hWndDlg,ControlID,CB_GETITEMDATA,SendDlgItemMessage(hWndDlg,ControlID,CB_GETCURSEL,0,0),0)


#define GetContactNameW(Contact) (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)Contact,GCDNF_UNICODE)

#define DB_SMS_DeleteValue(Contact,valueName) db_unset(Contact,PROTOCOL_NAMEA,valueName)
#define DB_SMS_GetDword(Contact,valueName,parDefltValue) db_get_dw(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_SMS_SetDword(Contact,valueName,parValue) db_set_dw(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_SMS_GetWord(Contact,valueName,parDefltValue) db_get_w(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_SMS_SetWord(Contact,valueName,parValue) db_set_w(Contact,PROTOCOL_NAMEA,valueName,parValue)
#define DB_SMS_GetByte(Contact,valueName,parDefltValue) db_get_b(Contact,PROTOCOL_NAMEA,valueName,parDefltValue)
#define DB_SMS_SetByte(Contact,valueName,parValue) db_set_b(Contact,PROTOCOL_NAMEA,valueName,parValue)
BOOL	DB_GetStaticStringW(MCONTACT hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpszRetBuff,size_t dwRetBuffSize,size_t *pdwRetBuffSize);
#define DB_SMS_GetStaticStringW(Contact,ValueName,Ret,RetBuffSize,pRetBuffSize) DB_GetStaticStringW(Contact,PROTOCOL_NAMEA,ValueName,Ret,RetBuffSize,pRetBuffSize)
#define DB_SetStringW(Contact,Module,valueName,parValue) db_set_ws(Contact,Module,valueName,parValue)
#define DB_SMS_SetStringW(Contact,valueName,parValue) db_set_ws(Contact,PROTOCOL_NAMEA,valueName,parValue)

LRESULT CALLBACK MessageSubclassProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

LPSTR  GetModuleName(MCONTACT hContact);
void   EnableControlsArray(HWND hWndDlg,WORD *pwControlsList,size_t dwControlsListCount,BOOL bEnabled);

// Declaration of function that returns received string with only numbers
size_t CopyNumberA(LPSTR lpszOutBuff,LPSTR lpszBuff,size_t dwLen);
size_t CopyNumberW(LPWSTR lpcOutBuff,LPWSTR lpcBuff,size_t dwLen);
bool   IsPhoneW(LPWSTR lpwszString,size_t dwStringLen);
DWORD  GetContactPhonesCount(MCONTACT hContact);
BOOL   IsContactPhone(MCONTACT hContact,LPWSTR lpwszPhone,size_t dwPhoneSize);

// Declaration of function that returns HANDLE of contact by his cellular number
MCONTACT HContactFromPhone(LPWSTR lpwszPhone,size_t dwPhoneSize);
BOOL     GetDataFromMessage(LPSTR lpszMessage,size_t dwMessageSize,DWORD *pdwEventType,LPWSTR lpwszPhone,size_t dwPhoneSize,size_t *pdwPhoneSizeRet,UINT *piIcon);

// Declaration of function that gets a XML string and return the asked tag.
BOOL   GetXMLFieldEx(LPSTR lpszXML,size_t dwXMLSize,LPSTR *plpszData,size_t *pdwDataSize,const char *tag1,...);
BOOL   GetXMLFieldExBuff(LPSTR lpszXML,size_t dwXMLSize,LPSTR lpszBuff,size_t dwBuffSize,size_t *pdwBuffSizeRet,const char *tag1,...);
DWORD  DecodeXML(LPTSTR lptszMessage,size_t dwMessageSize,LPTSTR lptszMessageConverted,size_t dwMessageConvertedBuffSize,size_t *pdwMessageConvertedSize);
DWORD  EncodeXML(LPTSTR lptszMessage,size_t dwMessageSize,LPTSTR lptszMessageConverted,size_t dwMessageConvertedBuffSize,size_t *pdwMessageConvertedSize);
void   LoadMsgDlgFont(int i,LOGFONT *lf,COLORREF *colour);
int    RefreshAccountList(WPARAM eventCode,LPARAM lParam);
void   FreeAccountList();

int    OptInitialise(WPARAM wParam,LPARAM lParam);

int    LoadServices();
int    LoadModules();

int    handleAckSMS(WPARAM wParam,LPARAM lParam);
int    handleNewMessage(WPARAM wParam,LPARAM lParam);
void   RestoreUnreadMessageAlerts();

// Declaration of Menu SMS send click function
int    SmsRebuildContactMenu(WPARAM wParam,LPARAM lParam);

void   StartSmsSend(HWND hWndDlg,size_t dwModuleIndex,LPWSTR lpwszPhone,size_t dwPhoneSize,LPWSTR lpwszMessage,size_t dwMessageSize);

#endif