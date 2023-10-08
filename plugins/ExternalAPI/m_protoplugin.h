#ifndef __M_PROTOPLUGIN_H
#define __M_PROTOPLUGIN_H

#include <windows.h>
#include "m_account.h"			// for account import functions
#include "m_mails.h"	// for mail import functions

// 
// ================================== OTHER DEFINITIONS ========================================
// 

// structure is used to give parameters to Check, Synchro or Timeout function
struct CheckParam
{
	// Your plugin should use this definition
#define YAMN_CHECKVERSION	2
	// Version of this structure. Please verify your version in your plugin
	int Ver;
	// Event that new Check thread must set to signal calling thread that "I've copied all parameters from stack"
	// IMPORTANT!!!: Although version #defined in your plugin is not the same, your plugin MUST signal this event
	// in any way. YAMN is waiting for this event. If you do not signal it, YAMN is blocked.
	HANDLE ThreadRunningEV;
	// ActualAccount- the only parameter used in Check function and should contain all needed information I think :)
	CAccount *AccountParam;

	// I thought it, but this is needed, too
#define YAMN_NORMALCHECK	0
#define YAMN_FORCECHECK		1
	int Flags;

	// YAMN writes here some informations that are needed to pass to mail browser function (or bad connection)
	void *BrowserParam;
	// Calling thread (protocol plugin) can write here its own parameters. Usefull when protocol calls its own check function. YAMN always sets this parameter to NULL
	void *CustomParam;
};

// structure is used to give parameters to DeleteMails function
struct DeleteParam
{
	// Your plugin should use this definition
#define YAMN_DELETEVERSION	1
	// Version of this structure. Please verify your version in your plugin
	DWORD Ver;
	// Event that new Delete thread must set to signal calling thread that it copied all parameters from stack
	// IMPORTANT!!!: Although version #defined in your plugin is not the same, your plugin MUST signal this event
	// in any way. YAMN is waiting for this event. If you do not signal it, YAMN is blocked.
	HANDLE ThreadRunningEV;
	// ActualAccount- which account to delete
	CAccount *AccountParam;
	// YAMN writes here some informations that are needed to pass to mail browser function (or bad connection or no new mail)
	void *BrowserParam;
	// Calling thread can write here its own parameter. Usefull when protocol calls its own delete function. YAMN always sets this parameter to NULL
	void *CustomParam;
};

// 
// ================================== IMPORTED FUNCTIONS ==================================
// 

#ifndef YAMN_STANDARDFCN
typedef DWORD(WINAPI *YAMN_STANDARDFCN)(LPVOID);
#endif
typedef struct CYAMNVariables *(WINAPI *YAMN_GETVARIABLESFCN)(DWORD);
typedef CAccount *(WINAPI *YAMN_NEWACCOUNTFCN)(struct YAMN_PROTOPLUGIN *);
typedef void (WINAPI *YAMN_STOPACCOUNTFCN)(CAccount *);
typedef void (WINAPI *YAMN_DELETEACCOUNTFCN)(CAccount *);
typedef DWORD(WINAPI *YAMN_WRITEPLUGINOPTS)(HANDLE File, CAccount *);
typedef DWORD(WINAPI *YAMN_READPLUGINOPTS)(CAccount *, char **, char *);
typedef DWORD(WINAPI *YAMN_CHECKFCN)(struct CheckParam *);
typedef void(__cdecl *YAMN_DELETEFCN)(void *);
typedef TCHAR* (WINAPI *YAMN_GETERRORSTRINGWFCN)(DWORD);
typedef char* (WINAPI *YAMN_GETERRORSTRINGAFCN)(DWORD);
typedef void (WINAPI *YAMN_DELETEERRORSTRINGFCN)(LPVOID);
typedef DWORD(WINAPI *YAMN_WRITEACCOUNTSFCN)();

struct YAMN_PROTOIMPORTFCN
{
	// Note: not all of these functions are needed to be implemented in your protocol plugin. Those
	// functions, which are not implemented, you have to set to NULL.

	// Function is called to construct protocol defined account
	// This is VERY IMPORTANT for YAMN and plugin to cooperate:
	// Imagine following situation. YAMN wants to add new account (it is possible e.g.
	// when loading accounts from file), so it has to call protocol constructor.
	// It calls NewAccount function and plugin creates new account and returns
	// its handle (pointer in fact). That means new account is created with plugin features
	// (it is created inherited account, not base class).
	YAMN_NEWACCOUNTFCN		NewAccountFcnPtr;

	// Function is called to delete protocol defined variables to inherited CAccount structure
	YAMN_DELETEACCOUNTFCN		DeleteAccountFcnPtr;

	// Function is called when user requests not tu run account longer. (E.g. when closing Miranda)
	YAMN_STOPACCOUNTFCN		StopAccountFcnPtr;

	// Function is called when plugin should write its own info into book file
	YAMN_WRITEPLUGINOPTS		WritePluginOptsFcnPtr;

	// Function is called when plugin should read its own info from book file
	YAMN_READPLUGINOPTS		ReadPluginOptsFcnPtr;

	// Function is called to synchronise account (delete old mails and get the new ones)
	YAMN_CHECKFCN			SynchroFcnPtr;

	// Function is called when timer timed out- it can be the same as SynchroFcnPtr
	YAMN_CHECKFCN			TimeoutFcnPtr;

	// Function is called when forced checking- it can be the same as SynchroFcnPtr
	YAMN_CHECKFCN			ForceCheckFcnPtr;

	// Function is called when user wants to delete mails
	YAMN_DELETEFCN			DeleteMailsFcnPtr;

	// Function is called when YAMN wants to get error description. Note the parameter given in
	// this function is in fact the same as your CheckFcnPtr, DeleteMailsFcnPtr etc. returns to YAMN.
	// If you want, you may return pointer to some structure, which includes more information about
	// error than only one DWORD. And then, you can in your function create Unicode string containing
	// all your error code. YAMN copies this string into its own buffer. Your error code and pointer
	// can be deleted in DeleteErrorStringFcnPtr, which is called by YAMN
	YAMN_GETERRORSTRINGWFCN		GetErrorStringWFcnPtr;

	// This is the same as previous one, but plugin returns normal string (not Unicode). YAMN first
	// looks, if your plugin has implemented GetErrorStringWFcnPtr. If not, it looks for this function
	// So as you (of course) wait, you implemnt only one of these functions or no one of them.
	YAMN_GETERRORSTRINGAFCN		GetErrorStringAFcnPtr;

	// Deletes error string that was allocated in your GetErrorStringXFcnPtr. Parameter to this fcn is
	// Unicode or normal string. Therefore parameter is defined as LPVOID, but your plugin knows if it is
	// Unicode or not...
	// If NULL, YAMN does nothing with string
	YAMN_DELETEERRORSTRINGFCN	DeleteErrorStringFcnPtr;

	// Function is called to notify plugin, that it is quite good to store account status (and mails)
	YAMN_WRITEACCOUNTSFCN		WriteAccountsFcnPtr;

	// Function is called when user wants to view mails
	// not used now, in the future
	YAMN_STANDARDFCN		ViewMailsFcnPtr;

	// Function is called when application exits. Plugin should unload
	YAMN_STANDARDFCN		UnLoadFcn;
};

typedef HYAMNMAIL(WINAPI *YAMN_NEWMAILFCN)(CAccount *);
typedef void (WINAPI *YAMN_DELETEMAILFCN)(HYAMNMAIL);
typedef DWORD(WINAPI *YAMN_WRITEMAILOPTS)(HANDLE File, HYAMNMAIL);
typedef DWORD(WINAPI *YAMN_READMAILOPTS)(HYAMNMAIL, char **, char *);

struct YAMN_MAILIMPORTFCN
{
	// Note: not all of these functions are needed to be implemented in your protocol plugin. Those
	// functions, which are not implemented, you have to set to NULL.

	// Function is called to construct protocol defined account
	// This is VERY IMPORTANT for YAMN and plugin to cooperate:
	// Imagine following situation. YAMN wants to add new account (it is possible e.g.
	// when loading accounts from file), so it has to call protocol constructor.
	// It calls NewAccount function and plugin creates new account and returns
	// its handle (pointer in fact). That means new account is created with plugin features
	// (it is created inherited account, not base class).
	YAMN_NEWMAILFCN			NewMailFcnPtr;

	// Function is called to delete protocol defined variables to inherited CAccount structure
	YAMN_DELETEMAILFCN		DeleteMailFcnPtr;

	// Function is called when plugin should write its own info into book file
	YAMN_WRITEMAILOPTS		WriteMailOptsFcnPtr;

	// Function is called when plugin should read its own info from book file
	YAMN_READMAILOPTS		ReadMailOptsFcnPtr;
};

// 
// ================================== PROTOCOL PLUGIN REGISTRATION STRUCTURES ==================================
// 

struct YAMN_PROTOREGISTRATION
{
	// Name of plugin
	// this member CANNOT be NULL. Just write here description, i.e. "Yahoo Mail 1.2"
	char *Name;

	// The version of plugin. CANNOT be NULL.
	char *Ver;

	// Plugin copyright
	// Write here your copyright if you want (or NULL)
	char *Copyright;

	// Plugin description. Can be NULL.
	char *Description;

	// Your contact (email). Can be NULL.
	char *Email;

	// The web page. Can be NULL.
	char *WWW;
};

struct YAMN_PROTOPLUGIN
{
	// Pointer to first protocol plugin account
	CAccount *FirstAccount = 0;

	// We prevent browsing through accounts (chained list) from deleting or adding any account
	// If we want to delete or add, we must have "write" access to AccountBrowserSO
	// Note that accounts can be changed during AccountBrowser is in "read" mode, because we do not add or delete account.
	SWMRG AccountBrowserSO;

	// All needed other info from plugin
	YAMN_PROTOREGISTRATION *PluginInfo;

	// Imported functions
	YAMN_PROTOIMPORTFCN *Fcn = 0;
	YAMN_MAILIMPORTFCN *MailFcn = 0;
};

struct YAMN_PROTOPLUGINQUEUE
{
	YAMN_PROTOPLUGIN *Plugin;
	YAMN_PROTOPLUGINQUEUE *Next;
};

#endif
