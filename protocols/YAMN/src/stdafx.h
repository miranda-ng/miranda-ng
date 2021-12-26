
#ifndef __YAMN_H
#define __YAMN_H

#define VC_EXTRALEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_messages.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_timezones.h>

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
#include "proto/netclient.h"
#include "proto/netlib.h"
#include "proto/pop3/pop3.h"
#include "proto/pop3/pop3comm.h"
#include "proto/pop3/pop3opt.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
 
// From services.cpp
void CreateServiceFunctions(void);
void HookEvents(void);
void RefreshContact(void);
void ContactDoubleclicked(WPARAM wParam, LPARAM lParam);
INT_PTR ClistContactDoubleclicked(WPARAM wParam, LPARAM lParam);

extern mir_cs PluginRegCS;
extern SCOUNTER *AccountWriterSO;
extern HANDLE ExitEV;
extern HANDLE WriteToFileEV;

// From debug.cpp
#ifdef _DEBUG
void InitDebug();
void UnInitDebug();
#endif

// From yamn.cpp
INT_PTR GetFcnPtrSvc(WPARAM wParam, LPARAM lParam);
INT_PTR GetVariablesSvc(WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
INT_PTR ForceCheckSvc(WPARAM, LPARAM);

extern struct YAMNExportedFcns *pYAMNFcn;

// From account.cpp
INT_PTR CreatePluginAccountSvc(WPARAM wParam, LPARAM lParam);
INT_PTR DeletePluginAccountSvc(WPARAM wParam, LPARAM);
int InitAccount(CAccount *Which);
void DeInitAccount(CAccount *Which);
void StopSignalFcn(CAccount *Which);
void CodeDecodeString(char *Dest, BOOL Encrypt);
uint32_t FileToMemory(wchar_t *FileName, char **MemFile, char **End);

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
uint32_t ReadStringFromMemory(char **Parser,char *End,char **StoreTo,char *DebugString);
#endif
uint32_t ReadStringFromMemory(char **Parser, char *End, char **StoreTo);
uint32_t ReadMessagesFromMemory(CAccount *Which, char **Parser, char *End);
uint32_t ReadAccountFromMemory(CAccount *Which, char **Parser, wchar_t *End);
INT_PTR AddAccountsFromFileSvc(WPARAM wParam, LPARAM lParam);

uint32_t WriteStringToFile(HANDLE File, char *Source);
uint32_t WriteStringToFileW(HANDLE File, wchar_t *Source);


DWORD WriteMessagesToFile(HANDLE File, CAccount *Which);
DWORD WINAPI WritePOP3Accounts();
INT_PTR WriteAccountsToFileSvc(WPARAM wParam, LPARAM lParam);
INT_PTR FindAccountByNameSvc(WPARAM wParam, LPARAM lParam);
INT_PTR GetNextFreeAccountSvc(WPARAM wParam, LPARAM lParam);

INT_PTR DeleteAccountSvc(WPARAM wParam, LPARAM);
void __cdecl DeleteAccountInBackground(void *Which);
int StopAccounts(HYAMNPROTOPLUGIN Plugin);
int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin, BOOL GetAccountBrowserAccess = FALSE);
int DeleteAccounts(HYAMNPROTOPLUGIN Plugin);

void WINAPI GetStatusFcn(CAccount *Which, wchar_t *Value);
void WINAPI SetStatusFcn(CAccount *Which, wchar_t *Value);

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
//void WINAPI ExtractHeaderFcn(char *,int,uint16_t,HYAMNMAIL);	//already in MailExported
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

extern wchar_t UserDirectory[];		//e.g. "F:\WINNT\Profiles\UserXYZ"
extern wchar_t ProfileName[];		//e.g. "majvan"
extern SWMRG *AccountBrowserSO;
extern YAMN_VARIABLES YAMNVar;
extern HANDLE hNewMailHook;
extern HANDLE hTTButton;
extern HCURSOR hCurSplitNS, hCurSplitWE;
extern UINT SecTimer;

//From synchro.cpp
void  WINAPI DeleteMessagesToEndFcn(CAccount *Account, HYAMNMAIL From);
uint32_t WINAPI WaitToWriteFcn(PSWMRG SObject, PSCOUNTER SCounter = nullptr);
void  WINAPI WriteDoneFcn(PSWMRG SObject, PSCOUNTER SCounter = nullptr);
uint32_t WINAPI WaitToReadFcn(PSWMRG SObject);
void  WINAPI ReadDoneFcn(PSWMRG SObject);
uint32_t WINAPI SCIncFcn(PSCOUNTER SCounter);
uint32_t WINAPI SCDecFcn(PSCOUNTER SCounter);
BOOL  WINAPI SWMRGInitialize(PSWMRG, wchar_t *);
void  WINAPI SWMRGDelete(PSWMRG);
uint32_t WINAPI SWMRGWaitToWrite(PSWMRG pSWMRG, uint32_t dwTimeout);
void  WINAPI SWMRGDoneWriting(PSWMRG pSWMRG);
uint32_t WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, uint32_t dwTimeout);
void  WINAPI SWMRGDoneReading(PSWMRG pSWMRG);

//From mails.cpp
void WINAPI DeleteMessageFromQueueFcn(HYAMNMAIL *From, HYAMNMAIL Which, int mode);
void WINAPI SetRemoveFlagsInQueueFcn(HYAMNMAIL From, uint32_t FlagsSet, uint32_t FlagsNotSet, uint32_t FlagsToSet, int mode);

//From mime.cpp
void ExtractHeader(struct CMimeItem *items, int &CP, struct CHeader *head);
void ExtractShortHeader(struct CMimeItem *items, struct CShortHeader *head);
void DeleteHeaderContent(struct CHeader *head);
void DeleteShortHeaderContent(struct CShortHeader *head);
char *ExtractFromContentType(char *ContentType, char *value);
wchar_t *ParseMultipartBody(char *src, char *bond);

//From account.cpp
void WINAPI GetStatusFcn(CAccount *Which, wchar_t *Value);

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
extern void ConvertCodedStringToUnicode(char *stream, wchar_t **storeto, uint32_t cp, int mode);
extern void __cdecl MailBrowser(void *Param);
extern uint32_t WINAPI NoNewMailProc(LPVOID Param);
extern void __cdecl BadConnection(void *Param);
extern PVOID TLSCtx;
extern PVOID SSLCtx;

extern HGENMENU hMenuItemMain, hMenuItemCont, hMenuItemContApp;
extern PYAMN_VARIABLES pYAMNVar;

#endif
