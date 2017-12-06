#ifndef __M_PROTOPLUGIN_H
#define __M_PROTOPLUGIN_H

#include <windows.h>
#include "m_account.h"			//for account import functions
#include "m_mails.h"	//for mail import functions

//
//================================== OTHER DEFINITIONS ========================================
//

//structure is used to give parameters to Check, Synchro or Timeout function
struct CheckParam
{
	//Your plugin should use this definition
#define YAMN_CHECKVERSION	2
	//Version of this structure. Please verify your version in your plugin
	DWORD Ver;
	//Event that new Check thread must set to signal calling thread that "I've copied all parameters from stack"
	//IMPORTANT!!!: Although version #defined in your plugin is not the same, your plugin MUST signal this event
	//in any way. YAMN is waiting for this event. If you do not signal it, YAMN is blocked.
	HANDLE ThreadRunningEV;
	//ActualAccount- the only parameter used in Check function and should contain all needed information I think :)
	HACCOUNT AccountParam;

	//I thought it, but this is needed, too
#define YAMN_NORMALCHECK	0
#define YAMN_FORCECHECK		1
	DWORD Flags;

	//YAMN writes here some informations that are needed to pass to mail browser function (or bad connection)
	void *BrowserParam;
	//Calling thread (protocol plugin) can write here its own parameters. Usefull when protocol calls its own check function. YAMN always sets this parameter to NULL
	void *CustomParam;
};

//structure is used to give parameters to DeleteMails function
struct DeleteParam
{
	//Your plugin should use this definition
#define YAMN_DELETEVERSION	1
	//Version of this structure. Please verify your version in your plugin
	DWORD Ver;
	//Event that new Delete thread must set to signal calling thread that it copied all parameters from stack
	//IMPORTANT!!!: Although version #defined in your plugin is not the same, your plugin MUST signal this event
	//in any way. YAMN is waiting for this event. If you do not signal it, YAMN is blocked.
	HANDLE ThreadRunningEV;
	//ActualAccount- which account to delete
	HACCOUNT AccountParam;
	//YAMN writes here some informations that are needed to pass to mail browser function (or bad connection or no new mail)
	void *BrowserParam;
	//Calling thread can write here its own parameter. Usefull when protocol calls its own delete function. YAMN always sets this parameter to NULL
	void *CustomParam;
};

//
//================================== IMPORTED FUNCTIONS ==================================
//

#ifndef YAMN_STANDARDFCN
typedef DWORD(WINAPI *YAMN_STANDARDFCN)(LPVOID);
#endif
typedef struct CYAMNVariables *(WINAPI *YAMN_GETVARIABLESFCN)(DWORD);
typedef HACCOUNT(WINAPI *YAMN_NEWACCOUNTFCN)(struct CYAMNProtoPlugin *, DWORD);
typedef void (WINAPI *YAMN_STOPACCOUNTFCN)(HACCOUNT);
typedef void (WINAPI *YAMN_DELETEACCOUNTFCN)(HACCOUNT);
typedef DWORD(WINAPI *YAMN_WRITEPLUGINOPTS)(HANDLE File, HACCOUNT);
typedef DWORD(WINAPI *YAMN_READPLUGINOPTS)(HACCOUNT, char **, char *);
typedef DWORD(WINAPI *YAMN_CHECKFCN)(struct CheckParam *);
typedef void(__cdecl *YAMN_DELETEFCN)(void *);
typedef TCHAR* (WINAPI *YAMN_GETERRORSTRINGWFCN)(DWORD);
typedef char* (WINAPI *YAMN_GETERRORSTRINGAFCN)(DWORD);
typedef void (WINAPI *YAMN_DELETEERRORSTRINGFCN)(LPVOID);
typedef DWORD(WINAPI *YAMN_WRITEACCOUNTSFCN)();

typedef struct CAccountImportFcn
{
	//If changes are made in this structure, version is changed. 
	//So then YAMN does not initialize your structure, if version does not match.
#define YAMN_PROTOIMPORTFCNVERSION	3

	//Note: not all of these functions are needed to be implemented in your protocol plugin. Those
	//functions, which are not implemented, you have to set to NULL.

	//Function is called to construct protocol defined account
	//This is VERY IMPORTANT for YAMN and plugin to cooperate:
	//Imagine following situation. YAMN wants to add new account (it is possible e.g.
	//when loading accounts from file), so it has to call protocol constructor.
	//It calls NewAccount function and plugin creates new account and returns
	//its handle (pointer in fact). That means new account is created with plugin features
	//(it is created inherited account, not base class).
	YAMN_NEWACCOUNTFCN		NewAccountFcnPtr;

	//Function is called to delete protocol defined variables to inherited CAccount structure
	YAMN_DELETEACCOUNTFCN		DeleteAccountFcnPtr;

	//Function is called when user requests not tu run account longer. (E.g. when closing Miranda)
	YAMN_STOPACCOUNTFCN		StopAccountFcnPtr;

	//Function is called when plugin should write its own info into book file
	YAMN_WRITEPLUGINOPTS		WritePluginOptsFcnPtr;

	//Function is called when plugin should read its own info from book file
	YAMN_READPLUGINOPTS		ReadPluginOptsFcnPtr;

	//Function is called to synchronise account (delete old mails and get the new ones)
	YAMN_CHECKFCN			SynchroFcnPtr;

	//Function is called when timer timed out- it can be the same as SynchroFcnPtr
	YAMN_CHECKFCN			TimeoutFcnPtr;

	//Function is called when forced checking- it can be the same as SynchroFcnPtr
	YAMN_CHECKFCN			ForceCheckFcnPtr;

	//Function is called when user wants to delete mails
	YAMN_DELETEFCN			DeleteMailsFcnPtr;

	//Function is called when YAMN wants to get error description. Note the parameter given in
	//this function is in fact the same as your CheckFcnPtr, DeleteMailsFcnPtr etc. returns to YAMN.
	//If you want, you may return pointer to some structure, which includes more information about
	//error than only one DWORD. And then, you can in your function create Unicode string containing
	//all your error code. YAMN copies this string into its own buffer. Your error code and pointer
	//can be deleted in DeleteErrorStringFcnPtr, which is called by YAMN
	YAMN_GETERRORSTRINGWFCN		GetErrorStringWFcnPtr;

	//This is the same as previous one, but plugin returns normal string (not Unicode). YAMN first
	//looks, if your plugin has implemented GetErrorStringWFcnPtr. If not, it looks for this function
	//So as you (of course) wait, you implemnt only one of these functions or no one of them.
	YAMN_GETERRORSTRINGAFCN		GetErrorStringAFcnPtr;

	//Deletes error string that was allocated in your GetErrorStringXFcnPtr. Parameter to this fcn is
	//Unicode or normal string. Therefore parameter is defined as LPVOID, but your plugin knows if it is
	//Unicode or not...
	//If NULL, YAMN does nothing with string
	YAMN_DELETEERRORSTRINGFCN	DeleteErrorStringFcnPtr;

	//Function is called to notify plugin, that it is quite good to store account status (and mails)
	YAMN_WRITEACCOUNTSFCN		WriteAccountsFcnPtr;

	//Function is called when user wants to view mails
	//not used now, in the future
	YAMN_STANDARDFCN		ViewMailsFcnPtr;

	//Function is called when application exits. Plugin should unload
	YAMN_STANDARDFCN		UnLoadFcn;
} YAMN_PROTOIMPORTFCN, *PYAMN_PROTOIMPORTFCN;

typedef HYAMNMAIL(WINAPI *YAMN_NEWMAILFCN)(HACCOUNT, DWORD);
typedef void (WINAPI *YAMN_DELETEMAILFCN)(HYAMNMAIL);
typedef DWORD(WINAPI *YAMN_WRITEMAILOPTS)(HANDLE File, HYAMNMAIL);
typedef DWORD(WINAPI *YAMN_READMAILOPTS)(HYAMNMAIL, char **, char *);

typedef struct CMailImportFcn
{
	//If changes are made in this structure, version is changed. 
	//So then YAMN does not initialize your structure, if version does not match.
#define	YAMN_MAILIMPORTFCNVERSION	1

	//Note: not all of these functions are needed to be implemented in your protocol plugin. Those
	//functions, which are not implemented, you have to set to NULL.

	//Function is called to construct protocol defined account
	//This is VERY IMPORTANT for YAMN and plugin to cooperate:
	//Imagine following situation. YAMN wants to add new account (it is possible e.g.
	//when loading accounts from file), so it has to call protocol constructor.
	//It calls NewAccount function and plugin creates new account and returns
	//its handle (pointer in fact). That means new account is created with plugin features
	//(it is created inherited account, not base class).
	YAMN_NEWMAILFCN			NewMailFcnPtr;

	//Function is called to delete protocol defined variables to inherited CAccount structure
	YAMN_DELETEMAILFCN		DeleteMailFcnPtr;

	//Function is called when plugin should write its own info into book file
	YAMN_WRITEMAILOPTS		WriteMailOptsFcnPtr;

	//Function is called when plugin should read its own info from book file
	YAMN_READMAILOPTS		ReadMailOptsFcnPtr;
} YAMN_MAILIMPORTFCN, *PYAMN_MAILIMPORTFCN;

//
//================================== PROTOCOL PLUGIN REGISTRATION STRUCTURES ==================================
//

typedef struct CProtoPluginRegistration
{
#define	YAMN_PROTOREGISTRATIONVERSION	1
	//Name of plugin
	//this member CANNOT be NULL. Just write here description, i.e. "Yahoo Mail 1.2"
	char *Name;

	//The version of plugin. CANNOT be NULL.
	char *Ver;

	//Plugin copyright
	//Write here your copyright if you want (or NULL)
	char *Copyright;

	//Plugin description. Can be NULL.
	char *Description;

	//Your contact (email). Can be NULL.
	char *Email;

	//The web page. Can be NULL.
	char *WWW;

} YAMN_PROTOREGISTRATION, *PYAMN_PROTOREGISTRATION;

typedef struct CYAMNProtoPlugin
{
	//Pointer to first protocol plugin account
	HACCOUNT FirstAccount;

	//We prevent browsing through accounts (chained list) from deleting or adding any account
	//If we want to delete or add, we must have "write" access to AccountBrowserSO
	//Note that accounts can be changed during AccountBrowser is in "read" mode, because we do not add or delete account.
	PSWMRG AccountBrowserSO;

	//All needed other info from plugin
	PYAMN_PROTOREGISTRATION PluginInfo;

	//Imported functions
	PYAMN_PROTOIMPORTFCN Fcn;
	PYAMN_MAILIMPORTFCN MailFcn;
} YAMN_PROTOPLUGIN, *PYAMN_PROTOPLUGIN, *HYAMNPROTOPLUGIN;

typedef struct CProtoPluginQueue
{
	HYAMNPROTOPLUGIN Plugin;
	struct CProtoPluginQueue *Next;
} YAMN_PROTOPLUGINQUEUE, *PYAMN_PROTOPLUGINQUEUE;

//
//================================== YAMN SERVICES FOR PROTOCOL PLUGIN ==================================
//

//RegisterProtoPlugin Service
//Your plugin can call this service to "connect to YAMN"- it means, that you
//give some parameters to YAMN and YAMN can then cooperate with your protocol plugins
//WPARAM- pointer to YAMN_PROTOREGISTRATION structure. Plugin must not delete this structure from memory.
//LPARAM- version of YAMN_PROTOREGISTRATION structure (use YAMN_PROTOREGISTRATIONVERSION definition)
//returns handle to plugin (HYAMNPROTOPLUGIN), if registration failed (plugin not registered) returns NULL
//Note, that your plugin should store returned plugin handle, because it will be usefull in next services.
//You need next to call SetProtocolPluginFcnImportFcn to have your plugin cooperated with YAMN.
#define	MS_YAMN_REGISTERPROTOPLUGIN		"YAMN/Service/RegisterProtocolPlugin"

//UnregisterProtoPlugin Service
//Removes plugin from YAMN and deltes its structures
//WPARAM- (HYAMNPROTOPLUGIN) handle of protocol plugin
//LPARAM- any value
//returns nonzero if success
#define	MS_YAMN_UNREGISTERPROTOPLUGIN		"YAMN/Service/UnregisterProtocolPlugin"

//CreateAccount Service
//Your plugin should call this to create new account for your plugin.
//WPARAM- (HYAMNPLUGIN) Plugin handle
//LPARAM- CAccount version (use YAMN_ACCOUNTVERSION definition)
//returns pointer to (HACCOUNT) or pointer to your structure returned from imported NewAccountFcnPtr, if implemented
#define	MS_YAMN_CREATEPLUGINACCOUNT	"YAMN/Service/CreateAccount"

//DeletePluginAccount Service
//Deletes plugin's account from memory. You probably won't use this service, because it deletes only account
//without any synchronization. Use MS_YAMN_DELETEACCOUNT instead.
//WPARAM- (HACCOUNT) to delete
//LPARAM- any value
//returns zero if failed, otherwise returns nonzero
#define	MS_YAMN_DELETEPLUGINACCOUNT	"YAMN/Service/DeletePluginAccount"

//FindAccountByName Service
//Searches accounts queue for first account that belongs to plugin
//WPARAM- (HYAMNPLUGIN) Plugin handle
//LPARAM- (TCHAR *)string, name of account to find
//returns found HACCOUNT handle or NULL if not found
#define	MS_YAMN_FINDACCOUNTBYNAME	"YAMN/Service/FindAccountByName"

//GetNextFreeAccount Service
//Creates new account for plugin and adds it to plugin account queue.
//Note!!! you have to use AccountBrowserSO in your plugin before and after calling this service, because it is not synchronized
//So the normal way is like this:
//	WaitToWriteSO(MyPlugin->AccountBrowserSO);
//	CallService(MS_YAMN_GETNEXTFREEACCOUNT,MyPlugin,YAMN_ACCOUNTVERSION);
//	WriteDoneSO(MyPlugin->AccountBrowserSO);
//
//WPARAM- (HYAMNPLUGIN) Plugin handle
//LPARAM- CAccount version (use YAMN_ACCOUNTVERSION definition)
//returns new HACCOUNT handle or NULL if not found
#define	MS_YAMN_GETNEXTFREEACCOUNT	"YAMN/Service/GetNextFreeAccount"

//DeleteAccount Service
//Deletes account from plugin account queue. It also deletes it, but in background (when needed).
//This deleting is full synchronized and safe. It is recommended for plugins to use this service.
//WPARAM- (HYAMNPLUGIN) Plugin handle
//LPARAM- (HACCOUNT) Account to delete
#define MS_YAMN_DELETEACCOUNT	   	"YAMN/Service/DeleteAccount"

//ReadAccounts Service
//Reads standard accounts to file. Standard account means standard YAMN book format.
//WPARAM- (HYAMNPLUGIN) Plugin handle
//LPARAM- (TCHAR*)filename string. Put here your own desired filename.
//return value is one of the ones written in "account.h" file
#define	MS_YAMN_READACCOUNTS 		"YAMN/Service/ReadAccounts"

//WriteAccounts Service
//Writes standard accounts to file. Standard account means standard YAMN book format. It does not
//store special protocol features. It stores Account settings from CAccount struct and stores MIME mails
//from CMimeMsgQueue. If your Mails pointer does not point to CMimeMsgQueue structure,
//do not use this function. You are then forced to write your own function
//WPARAM- (HYAMNPLUGIN) Plugin handle
//LPARAM- (TCHAR*)filename string. Put here your own desired filename.
//return value is one of the ones written in "account.h" file
#define MS_YAMN_WRITEACCOUNTS 		"YAMN/Service/WriteAccounts"

//GetFileName Service
//Function makes original filename, when you add your protocol string
//From "yahoo" makes "yamn-accounts.yahoo.xxxxx.book" filename
//It is good to use this fcn to have similar filenames...
//WPARAM- (TCHAR*) plugin string
//LPARAM- ignored
//returns NULL when failed, otherwise returns (TCHAR*)string (!!! not char *) to filename!!!
//You can use MS_YAMN_DELETEFILENAME service to release allocated filename from memory
#define MS_YAMN_GETFILENAME		   "YAMN/Service/GetFileName"

//DeleteFileName Service
//deletes unicode string from memory
//WPARAM- (WCHAR *) pointer to unicode string
//LPARAM- any value
#define MS_YAMN_DELETEFILENAME		"YAMN/Service/DeleteFileName"

//
//================================== FUNCTIONS DEFINITIONS ========================================
//

typedef int (WINAPI *YAMN_SETPROTOCOLPLUGINFCNIMPORTFCN)(HYAMNPROTOPLUGIN Plugin, PYAMN_PROTOIMPORTFCN YAMNFcn, DWORD YAMNFcnVer, PYAMN_MAILIMPORTFCN YAMNMailFcn, DWORD YAMNMailFcnVer);

//
//================================== QUICK FUNCTION CALL DEFINITIONS ========================================
//

//These are defininitions for YAMN exported functions. Your plugin can use them.
//pYAMNFcn is global variable, it is pointer to your structure containing YAMN functions.
//It is something similar like pluginLink variable in Miranda plugin. If you use
//this name of variable, you have already defined these functions and you can use them.
//It's similar to Miranda's CreateService function.

//How to use YAMN functions:
//Create a structure containing pointer to functions you want to use in your plugin
//This structure can look something like this:
//
//	struct
//	{
//		YAMN_SETPROTOCOLPLUGINFCNIMPORTFCN	SetProtocolPluginFcnImportFcn;
//	} *pYAMNFcn;
//
//then you have to fill this structure with pointers...
//
//	pYAMNFcn->SetProtocolPluginFcnImportFcn=(YAMN_SETPROTOCOLPLUGINFCNIMPORTFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SETPROTOCOLPLUGINFCNIMPORTID,0);
//
//and in your plugin just simply use e.g.:
//
//	SetProtocolPluginFcnImport(...);
//

#define	YAMN_SETPROTOCOLPLUGINFCNIMPORTID	"YAMN/SetProtocolPluginFcnImport"

#define SetProtocolPluginFcnImport(a,b,c,d,e)		pYAMNFcn->SetProtocolPluginFcnImportFcn(a,b,c,d,e)

#endif
