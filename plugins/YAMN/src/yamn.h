
#ifndef __YAMN_H
#define __YAMN_H

#define _CRT_SECURE_NO_WARNINGS
#define VC_EXTRALEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_icolib.h>
#include <m_popup.h>
#include <m_messages.h>
#include <m_netlib.h>
#include <m_hotkeys.h>

#include <m_toptoolbar.h>
#include <m_kbdnotify.h>
#include <m_filterplugin.h>
#include <m_yamn.h>
#include <m_protoplugin.h>
#include <m_folders.h>

#include "main.h"
#include "mails/decode.h"
#include "browser/browser.h"
#include "resource.h"
#include "debug.h"
#include "version.h"
#include "proto\netclient.h"
#include "proto\netlib.h"
#include "proto\pop3\pop3.h"
#include "proto\pop3\pop3comm.h"
#include "proto\pop3\pop3opt.h"


//From services.cpp
void CreateServiceFunctions(void);
void DestroyServiceFunctions(void);
void HookEvents(void);
void UnhookEvents(void);
void RefreshContact(void);
void ContactDoubleclicked(WPARAM wParam, LPARAM lParam);
INT_PTR ClistContactDoubleclicked(WPARAM wParam, LPARAM lParam);

extern CRITICAL_SECTION PluginRegCS;
extern SCOUNTER *AccountWriterSO;
extern HANDLE ExitEV;
extern HANDLE WriteToFileEV;

//From debug.cpp
#ifdef _DEBUG
void InitDebug();
void UnInitDebug();
#endif

//From synchro.cpp
//struct CExportedFunctions SynchroExported[];

//From yamn.cpp
INT_PTR GetFcnPtrSvc(WPARAM wParam, LPARAM lParam);
INT_PTR GetVariablesSvc(WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
INT_PTR ForceCheckSvc(WPARAM, LPARAM);

extern struct YAMNExportedFcns *pYAMNFcn;

//From account.cpp
extern CRITICAL_SECTION AccountStatusCS;
extern CRITICAL_SECTION FileWritingCS;

INT_PTR CreatePluginAccountSvc(WPARAM wParam, LPARAM lParam);
INT_PTR DeletePluginAccountSvc(WPARAM wParam, LPARAM);
int InitAccount(HACCOUNT Which);
void DeInitAccount(HACCOUNT Which);
void StopSignalFcn(HACCOUNT Which);
void CodeDecodeString(char *Dest, BOOL Encrypt);
DWORD FileToMemory(TCHAR *FileName, char **MemFile, char **End);

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemory(char **Parser,char *End,char **StoreTo,char *DebugString);
#endif
DWORD ReadStringFromMemory(char **Parser, char *End, char **StoreTo);
DWORD ReadMessagesFromMemory(HACCOUNT Which, char **Parser, char *End);
DWORD ReadAccountFromMemory(HACCOUNT Which, char **Parser, TCHAR *End);
INT_PTR AddAccountsFromFileSvc(WPARAM wParam, LPARAM lParam);

DWORD WriteStringToFile(HANDLE File, char *Source);
DWORD WriteStringToFileW(HANDLE File, WCHAR *Source);


DWORD WriteMessagesToFile(HANDLE File, HACCOUNT Which);
DWORD WINAPI WritePOP3Accounts();
INT_PTR WriteAccountsToFileSvc(WPARAM wParam, LPARAM lParam);
INT_PTR FindAccountByNameSvc(WPARAM wParam, LPARAM lParam);
INT_PTR GetNextFreeAccountSvc(WPARAM wParam, LPARAM lParam);

INT_PTR DeleteAccountSvc(WPARAM wParam, LPARAM);
void __cdecl DeleteAccountInBackground(void *Which);
int StopAccounts(HYAMNPROTOPLUGIN Plugin);
int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin, BOOL GetAccountBrowserAccess = FALSE);
int DeleteAccounts(HYAMNPROTOPLUGIN Plugin);

void WINAPI GetStatusFcn(HACCOUNT Which, TCHAR *Value);
void WINAPI SetStatusFcn(HACCOUNT Which, TCHAR *Value);

INT_PTR UnregisterProtoPlugins();
INT_PTR RegisterProtocolPluginSvc(WPARAM, LPARAM);
INT_PTR UnregisterProtocolPluginSvc(WPARAM, LPARAM);
INT_PTR GetFileNameSvc(WPARAM, LPARAM);
INT_PTR DeleteFileNameSvc(WPARAM, LPARAM);

//From filterplugin.cpp
//struct CExportedFunctions FilterPluginExported[];
INT_PTR UnregisterFilterPlugins();
INT_PTR RegisterFilterPluginSvc(WPARAM, LPARAM);
INT_PTR UnregisterFilterPluginSvc(WPARAM, LPARAM);
INT_PTR FilterMailSvc(WPARAM, LPARAM);

//From mails.cpp (MIME)
//struct CExportedFunctions MailExported[];
INT_PTR CreateAccountMailSvc(WPARAM wParam, LPARAM lParam);
INT_PTR DeleteAccountMailSvc(WPARAM wParam, LPARAM lParam);
INT_PTR LoadMailDataSvc(WPARAM wParam, LPARAM lParam);
INT_PTR UnloadMailDataSvc(WPARAM wParam, LPARAM);
INT_PTR SaveMailDataSvc(WPARAM wParam, LPARAM lParam);

//From mime.cpp
//void WINAPI ExtractHeaderFcn(char *,int,WORD,HYAMNMAIL);	//already in MailExported
struct _tcptable
{
	char *NameBase, *NameSub;
	BOOLEAN isValid;
	unsigned short int CP;
};
extern struct _tcptable CodePageNamesAll[]; // in mime/decode.cpp
extern int CPLENALL;
extern struct _tcptable *CodePageNamesSupp; // in mime/decode.cpp
extern int CPLENSUPP;

extern int PosX, PosY, SizeX, SizeY;
extern int HeadPosX, HeadPosY, HeadSizeX, HeadSizeY, HeadSplitPos;

//#define CPDEFINDEX	63	//ISO-8859-1
#define CPDEFINDEX	0	//ACP

//From pop3comm.cpp
int RegisterPOP3Plugin(WPARAM, LPARAM);

//From mailbrowser.cpp
INT_PTR RunMailBrowserSvc(WPARAM, LPARAM);

//From badconnect.cpp
INT_PTR RunBadConnectionSvc(WPARAM, LPARAM);

//From YAMNopts.cpp
int YAMNOptInitSvc(WPARAM, LPARAM);

//From main.cpp
int PostLoad(WPARAM, LPARAM);				//Executed after all plugins loaded YAMN reads mails from file and notify every protocol it should set its functions
int Shutdown(WPARAM, LPARAM);				//Executed before Miranda is going to shutdown
int AddTopToolbarIcon(WPARAM, LPARAM);	//Executed when TopToolBar plugin loaded Adds bitmap to toolbar

extern TCHAR UserDirectory[];		//e.g. "F:\WINNT\Profiles\UserXYZ"
extern TCHAR ProfileName[];		//e.g. "majvan"
extern SWMRG *AccountBrowserSO;
extern YAMN_VARIABLES YAMNVar;
extern HANDLE hNewMailHook;
extern HANDLE hTTButton;
extern HCURSOR hCurSplitNS, hCurSplitWE;
extern UINT SecTimer;

HANDLE WINAPI g_GetIconHandle(int idx);
HICON  WINAPI g_LoadIconEx(int idx, bool big = false);
void   WINAPI g_ReleaseIcon(HICON hIcon);

//From synchro.cpp
void  WINAPI DeleteMessagesToEndFcn(HACCOUNT Account, HYAMNMAIL From);
DWORD WINAPI WaitToWriteFcn(PSWMRG SObject, PSCOUNTER SCounter = NULL);
void  WINAPI WriteDoneFcn(PSWMRG SObject, PSCOUNTER SCounter = NULL);
DWORD WINAPI WaitToReadFcn(PSWMRG SObject);
void  WINAPI ReadDoneFcn(PSWMRG SObject);
DWORD WINAPI SCIncFcn(PSCOUNTER SCounter);
DWORD WINAPI SCDecFcn(PSCOUNTER SCounter);
BOOL  WINAPI SWMRGInitialize(PSWMRG, TCHAR *);
void  WINAPI SWMRGDelete(PSWMRG);
DWORD WINAPI SWMRGWaitToWrite(PSWMRG pSWMRG, DWORD dwTimeout);
void  WINAPI SWMRGDoneWriting(PSWMRG pSWMRG);
DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout);
void  WINAPI SWMRGDoneReading(PSWMRG pSWMRG);

//From mails.cpp
void WINAPI DeleteMessageFromQueueFcn(HYAMNMAIL *From, HYAMNMAIL Which, int mode);
void WINAPI SetRemoveFlagsInQueueFcn(HYAMNMAIL From, DWORD FlagsSet, DWORD FlagsNotSet, DWORD FlagsToSet, int mode);

//From mime.cpp
void ExtractHeader(struct CMimeItem *items, int &CP, struct CHeader *head);
void ExtractShortHeader(struct CMimeItem *items, struct CShortHeader *head);
void DeleteHeaderContent(struct CHeader *head);
void DeleteShortHeaderContent(struct CShortHeader *head);
char *ExtractFromContentType(char *ContentType, char *value);
WCHAR *ParseMultipartBody(char *src, char *bond);

//From account.cpp
void WINAPI GetStatusFcn(HACCOUNT Which, TCHAR *Value);

extern HYAMNPROTOPLUGIN POP3Plugin;

//from decode.cpp
int DecodeQuotedPrintable(char *Src, char *Dst, int DstLen, BOOL isQ);
int DecodeBase64(char *Src, char *Dst, int DstLen);

//From filterplugin.cpp
extern PYAMN_FILTERPLUGINQUEUE FirstFilterPlugin;

//From protoplugin.cpp
extern PYAMN_PROTOPLUGINQUEUE FirstProtoPlugin;

extern struct CExportedFunctions ProtoPluginExportedFcn[1];
extern struct CExportedServices ProtoPluginExportedSvc[5];
//From filterplugin.cpp
extern struct CExportedFunctions FilterPluginExportedFcn[1];
extern struct CExportedServices FilterPluginExportedSvc[2];
//From synchro.cpp
extern struct CExportedFunctions SynchroExportedFcn[7];
//From account.cpp
extern struct CExportedFunctions AccountExportedFcn[2];
extern struct CExportedServices AccountExportedSvc[9];
//From mails.cpp (MIME)
extern struct CExportedFunctions MailExportedFcn[8];
extern struct CExportedServices MailExportedSvc[5];

extern char *iconDescs[];
extern char *iconNames[];
extern HIMAGELIST CSImages;

extern void __stdcall	SSL_DebugLog(const char *fmt, ...);

extern int YAMN_STATUS;

extern struct WndHandles *MessageWnd;

extern int GetCharsetFromString(char *input, size_t size);
extern void SendMsgToRecepients(struct WndHandles *FirstWin, UINT msg, WPARAM wParam, LPARAM lParam);
extern void ConvertCodedStringToUnicode(char *stream, WCHAR **storeto, DWORD cp, int mode);
extern void __cdecl MailBrowser(void *Param);
extern DWORD WINAPI NoNewMailProc(LPVOID Param);
extern void __cdecl BadConnection(void *Param);
extern PVOID TLSCtx;
extern PVOID SSLCtx;

extern HGENMENU hMenuItemMain, hMenuItemCont, hMenuItemContApp;
extern PYAMN_VARIABLES pYAMNVar;

#endif
