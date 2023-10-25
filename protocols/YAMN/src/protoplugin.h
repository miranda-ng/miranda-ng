#ifndef __M_PROTOPLUGIN_H
#define __M_PROTOPLUGIN_H

// 
// ================================== OTHER DEFINITIONS ========================================
// 

// structure is used to give parameters to Check, Synchro or Timeout function
struct CheckParam
{
	CheckParam(CAccount *_1, bool _2) :
		AccountParam(_1),
		bParam(_2)
	{}

	CAccount *AccountParam;
	bool bParam;
};

// structure is used to give parameters to DeleteMails function
typedef struct CheckParam DeleteParam;

// 
// ================================== IMPORTED FUNCTIONS ==================================
// 

typedef DWORD     (MIR_CDECL *YAMN_STANDARDFCN)(LPVOID);
typedef CAccount* (MIR_CDECL *YAMN_NEWACCOUNTFCN)(struct YAMN_PROTOPLUGIN *);
typedef void      (MIR_CDECL *YAMN_STOPACCOUNTFCN)(CAccount *);
typedef void      (MIR_CDECL *YAMN_DELETEACCOUNTFCN)(CAccount *);
typedef DWORD     (MIR_CDECL *YAMN_WRITEPLUGINOPTS)(HANDLE File, CAccount *);
typedef DWORD     (MIR_CDECL *YAMN_READPLUGINOPTS)(CAccount *, char **, char *);
typedef void      (MIR_CDECL *YAMN_CHECKFCN)(CheckParam *);
typedef void      (MIR_CDECL *YAMN_DELETEFCN)(void *);
typedef TCHAR*    (MIR_CDECL *YAMN_GETERRORSTRINGWFCN)(DWORD);
typedef char*     (MIR_CDECL *YAMN_GETERRORSTRINGAFCN)(DWORD);
typedef void      (MIR_CDECL *YAMN_DELETEERRORSTRINGFCN)(LPVOID);
typedef DWORD     (MIR_CDECL *YAMN_WRITEACCOUNTSFCN)();

typedef struct CYAMNVariables* (MIR_CDECL *YAMN_GETVARIABLESFCN)(DWORD);

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

typedef YAMNMAIL * (MIR_CDECL *YAMN_NEWMAILFCN)(CAccount *);
typedef void  (MIR_CDECL *YAMN_DELETEMAILFCN)(YAMNMAIL *);
typedef DWORD (MIR_CDECL *YAMN_WRITEMAILOPTS)(HANDLE File, YAMNMAIL *);
typedef DWORD (MIR_CDECL *YAMN_READMAILOPTS)(YAMNMAIL *, char **, char *);

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
