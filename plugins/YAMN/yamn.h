
#ifndef __YAMN_H
#define __YAMN_H
#ifndef _WIN32_IE
	#define _WIN32_IE 0x0400
#endif
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif

#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <direct.h>			//For _chdir()

#define MIRANDA_VER 0x0A00

#include <commctrl.h>		//For hotkeys
#include "win2k.h"
#include "newpluginapi.h"	//CallService,UnHookEvent
#include "m_utils.h"		//window broadcasting
#include "m_system.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_clist.h"
#include "m_clui.h"
#include "m_options.h"
#include "m_database.h"		//database
#include "m_contacts.h"		//contact
#include "m_protocols.h"	//protocols
#include "m_protomod.h"		//protocols module
#include "m_protosvc.h"
#include "m_toptoolbar.h"
#include "m_icolib.h"
#include "m_kbdnotify.h"
#include "m_popup.h"
#include "m_updater.h"
#include "m_account.h"	//Account structure and all needed structures to cooperate with YAMN
#include "m_messages.h"	//Messages sent to YAMN windows
#include "m_mails.h"	//use YAMN's mails
#include "mails/m_decode.h"	//use decoding macros (needed for header extracting)
#include "browser/m_browser.h"	//we want to run YAMN mailbrowser, no new mail notification and bad connect window
#include "resources/resource.h"
#include "m_protoplugin.h"
#include "m_filterplugin.h"
#include "m_yamn.h"	//Main YAMN's variables
#include "m_protoplugin.h"	//Protocol registration and so on
#include "m_synchro.h"	//Synchronization
#include "debug.h"
#include <m_folders.h>


//icons definitions
#define ICONSNUMBER 8
#define ICONSDESCS "Neutral","YAMN","New Mail","Connect Fail","Launch Application","TopToolBar UP","TopToolBar Down","Offline"
#define ICONSNAMES "YAMN_Neutral",iconDescs[1],"YAMN_NewMail","YAMN_ConnectFail","YAMN_ApplExec","YAMN_TopToolBarUp","YAMN_TopToolBarDown","YAMN_Offline"
#define ICONSINDS IDI_ONLINE,IDI_ICOYAMN1,IDI_ICOYAMN2,IDI_NA,IDI_OCCUPIED,IDI_ICOTTBUP,IDI_OCCUPIED,IDI_OFFLINE

//From services.cpp
void CreateServiceFunctions(void);
void DestroyServiceFunctions(void);
void HookEvents(void);
void UnhookEvents(void);
void RefreshContact(void);
void ContactDoubleclicked(WPARAM wParam,LPARAM lParam);
INT_PTR ClistContactDoubleclicked(WPARAM wParam, LPARAM lParam);

//From debug.cpp
#undef YAMN_DEBUG
#ifdef YAMN_DEBUG
void InitDebug();
void UnInitDebug();
#endif

//From synchro.cpp
//struct CExportedFunctions SynchroExported[];

//From yamn.cpp
INT_PTR GetFcnPtrSvc(WPARAM wParam,LPARAM lParam);
INT_PTR GetVariablesSvc(WPARAM,LPARAM);
//INT_PTR AddWndToYAMNWindowsSvc(WPARAM,LPARAM);
//INT_PTR RemoveWndFromYAMNWindowsSvc(WPARAM,LPARAM);
DWORD WINAPI YAMNHotKeyThread(LPVOID);
void CALLBACK TimerProc(HWND,UINT,UINT,DWORD);
INT_PTR ForceCheckSvc(WPARAM,LPARAM);
// int ExitProc(WPARAM,LPARAM);

//From account.cpp
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//From account.cpp
extern CRITICAL_SECTION AccountStatusCS;
extern CRITICAL_SECTION FileWritingCS;

INT_PTR CreatePluginAccountSvc(WPARAM wParam,LPARAM lParam);
INT_PTR DeletePluginAccountSvc(WPARAM wParam,LPARAM);
int InitAccount(HACCOUNT Which);
void DeInitAccount(HACCOUNT Which);
void StopSignalFcn(HACCOUNT Which);
void CodeDecodeString(char *Dest,BOOL Encrypt);
static DWORD PostFileToMemory(HANDLE File,char **MemFile,char **End);
DWORD FileToMemoryA(char *FileName,char **MemFile,char **End);
DWORD FileToMemoryW(WCHAR *FileName,char **MemFile,char **End);

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemory(char **Parser,TCHAR *End,char **StoreTo,char *DebugString);
#endif
DWORD ReadStringFromMemory(char **Parser,TCHAR *End,char **StoreTo);
#ifndef UNICODE
	#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo,WCHAR *DebugString);
	#endif  //if defined(DEBUG...)
DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo);
#endif	//ifdef Unicode

static DWORD ReadNotificationFromMemory(char **Parser,TCHAR *End,YAMN_NOTIFICATION *Which);
DWORD ReadMessagesFromMemory(HACCOUNT Which,char **Parser,char *End);
static INT_PTR PerformAccountReading(HYAMNPROTOPLUGIN Plugin,char *MemFile,char *End);
DWORD ReadAccountFromMemory(HACCOUNT Which,char **Parser,TCHAR *End);
INT_PTR AddAccountsFromFileASvc(WPARAM wParam,LPARAM lParam);
INT_PTR AddAccountsFromFileWSvc(WPARAM,LPARAM);

DWORD WriteStringToFile(HANDLE File,char *Source);
#ifndef UNICODE
#define WriteStringToFileW	WriteStringToFile
#else
DWORD WriteStringToFileW(HANDLE File,WCHAR *Source);
#endif

DWORD WriteMessagesToFile(HANDLE File,HACCOUNT Which);
static INT_PTR PerformAccountWriting(HYAMNPROTOPLUGIN Plugin,HANDLE File);
INT_PTR WriteAccountsToFileASvc(WPARAM wParam,LPARAM lParam);
INT_PTR WriteAccountsToFileWSvc(WPARAM wParam,LPARAM lParam);
INT_PTR FindAccountByNameSvc(WPARAM wParam,LPARAM lParam);
INT_PTR GetNextFreeAccountSvc(WPARAM wParam,LPARAM lParam);

INT_PTR DeleteAccountSvc(WPARAM wParam,LPARAM);
DWORD WINAPI DeleteAccountInBackground(LPVOID Which);
int StopAccounts(HYAMNPROTOPLUGIN Plugin);
int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin,BOOL GetAccountBrowserAccess=FALSE);
int DeleteAccounts(HYAMNPROTOPLUGIN Plugin);

void WINAPI GetStatusFcn(HACCOUNT Which,TCHAR *Value);
void WINAPI SetStatusFcn(HACCOUNT Which,TCHAR *Value);

INT_PTR UnregisterProtoPlugins();
INT_PTR RegisterProtocolPluginSvc(WPARAM,LPARAM);
INT_PTR UnregisterProtocolPluginSvc(WPARAM,LPARAM);
INT_PTR GetFileNameWSvc(WPARAM,LPARAM);
INT_PTR GetFileNameASvc(WPARAM,LPARAM);
INT_PTR DeleteFileNameSvc(WPARAM,LPARAM);

//From filterplugin.cpp
//struct CExportedFunctions FilterPluginExported[];
INT_PTR UnregisterFilterPlugins();
INT_PTR RegisterFilterPluginSvc(WPARAM,LPARAM);
INT_PTR UnregisterFilterPluginSvc(WPARAM,LPARAM);
INT_PTR FilterMailSvc(WPARAM,LPARAM);

//From mails.cpp (MIME)
//struct CExportedFunctions MailExported[];
INT_PTR CreateAccountMailSvc(WPARAM wParam,LPARAM lParam);
INT_PTR DeleteAccountMailSvc(WPARAM wParam,LPARAM lParam);
INT_PTR LoadMailDataSvc(WPARAM wParam,LPARAM lParam);
INT_PTR UnloadMailDataSvc(WPARAM wParam,LPARAM);
INT_PTR SaveMailDataSvc(WPARAM wParam,LPARAM lParam);

//From mime.cpp
//void WINAPI ExtractHeaderFcn(char *,int,WORD,HYAMNMAIL);	//already in MailExported
struct _tcptable
{
	char *NameBase,*NameSub;
	BOOLEAN isValid;
	unsigned short int CP;
};
extern struct _tcptable CodePageNamesAll[]; // in mime/decode.cpp
extern int CPLENALL;
extern struct _tcptable *CodePageNamesSupp; // in mime/decode.cpp
extern int CPLENSUPP;
//#define CPDEFINDEX	63	//ISO-8859-1
#define CPDEFINDEX	0	//ACP

//From pop3comm.cpp
int RegisterPOP3Plugin(WPARAM,LPARAM);

//From mailbrowser.cpp
INT_PTR RunMailBrowserSvc(WPARAM,LPARAM);

//From badconnect.cpp
INT_PTR RunBadConnectionSvc(WPARAM,LPARAM);

//From YAMNopts.cpp
void WordToModAndVk(WORD,UINT *,UINT *);
int YAMNOptInitSvc(WPARAM,LPARAM);

//From main.cpp
int PostLoad(WPARAM,LPARAM);				//Executed after all plugins loaded YAMN reads mails from file and notify every protocol it should set its functions
int Shutdown(WPARAM,LPARAM);				//Executed before Miranda is going to shutdown
int AddTopToolbarIcon(WPARAM,LPARAM);		//Executed when TopToolBar plugin loaded Adds bitmap to toolbar
void LoadPlugins();							//Loads plugins located in MirandaDir/Plugins/YAMN/*.dll
int UninstallQuestionSvc(WPARAM,LPARAM);	//Ask information when user wants to uninstall plugin

extern WCHAR UserDirectory[];		//e.g. "F:\WINNT\Profiles\UserXYZ"
extern WCHAR ProfileName[];		//e.g. "majvan"
extern SWMRG *AccountBrowserSO;
extern CRITICAL_SECTION PluginRegCS;
extern YAMN_VARIABLES YAMNVar;
extern HANDLE hNewMailHook;
extern HANDLE WriteToFileEV;
extern HICON hYamnIcons[];

//From synchro.cpp
extern void WINAPI DeleteMessagesToEndFcn(HACCOUNT Account,HYAMNMAIL From);
extern DWORD WINAPI WaitToWriteFcn(PSWMRG SObject,PSCOUNTER SCounter=NULL);
extern void WINAPI WriteDoneFcn(PSWMRG SObject,PSCOUNTER SCounter=NULL);
extern DWORD WINAPI WaitToReadFcn(PSWMRG SObject);
extern void WINAPI ReadDoneFcn(PSWMRG SObject);
extern DWORD WINAPI SCIncFcn(PSCOUNTER SCounter);
extern DWORD WINAPI SCDecFcn(PSCOUNTER SCounter);
extern BOOL WINAPI SWMRGInitialize(PSWMRG,TCHAR *);
extern void WINAPI SWMRGDelete(PSWMRG);
extern DWORD WINAPI SWMRGWaitToWrite(PSWMRG pSWMRG,DWORD dwTimeout);
extern void WINAPI SWMRGDoneWriting(PSWMRG pSWMRG);
extern DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout);
extern void WINAPI SWMRGDoneReading(PSWMRG pSWMRG);

//From mails.cpp
extern void WINAPI DeleteMessageFromQueueFcn(HYAMNMAIL *From,HYAMNMAIL Which,int mode);
extern void WINAPI SetRemoveFlagsInQueueFcn(HYAMNMAIL From,DWORD FlagsSet,DWORD FlagsNotSet,DWORD FlagsToSet,int mode);

//From mime.cpp
void ExtractHeader(struct CMimeItem *items,int &CP,struct CHeader *head);
void ExtractShortHeader(struct CMimeItem *items,struct CShortHeader *head); 
void DeleteHeaderContent(struct CHeader *head);
void DeleteShortHeaderContent(struct CShortHeader *head);
char *ExtractFromContentType(char *ContentType,char *value);
WCHAR *ParseMultipartBody(char *src, char *bond);

//From account.cpp
void WINAPI GetStatusFcn(HACCOUNT Which,TCHAR *Value);
extern int StopAccounts(HYAMNPROTOPLUGIN Plugin);
extern int DeleteAccounts(HYAMNPROTOPLUGIN Plugin);
extern int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin,BOOL GetAccountBrowserAccess);

extern char *ProtoName;
extern HYAMNPROTOPLUGIN POP3Plugin;

//from decode.cpp
int DecodeQuotedPrintable(char *Src,char *Dst,int DstLen, BOOL isQ);
int DecodeBase64(char *Src,char *Dst,int DstLen);

//From maild.cpp
extern INT_PTR LoadMailDataSvc(WPARAM wParam,LPARAM lParam);
extern INT_PTR UnloadMailDataSvc(WPARAM wParam,LPARAM);
extern INT_PTR SaveMailDataSvc(WPARAM wParam,LPARAM lParam);

///////////////////////////////////////////////////////////////////////
class _A2T
{
	TCHAR* buf;

public:
	_A2T( const char* s ) : buf( mir_a2t( s )) {}
	_A2T( const char* s, int cp ) : buf( mir_a2t_cp( s, cp )) {}
	~_A2T() { mir_free(buf); }

	__forceinline operator TCHAR*() const
	{	return buf;
	}
};

#endif
