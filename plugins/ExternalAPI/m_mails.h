#ifndef __MAILS_H
#define __MAILS_H

#include <windows.h>
#include <tchar.h>
#include "m_account.h"

//
//================================== OTHER DEFINITIONS ========================================
//

typedef struct CShortNames
{
	char *Value;
	char *ValueNick;
	struct CShortNames *Next;	
} YAMN_MIMESHORTNAMES,*PYAMN_MIMESHORTNAMES;

typedef struct CNames
{
	WCHAR *Value;
	WCHAR *ValueNick;
	struct CNames *Next;	
} YAMN_MIMENAMES,*PYAMN_MIMENAMES;

struct CShortHeader
//this header is used in to get non-unicode data from mime header
{
	char *From;
	char *FromNick;
	char *ReturnPath;
	char *ReturnPathNick;
	char *Subject;
	PYAMN_MIMESHORTNAMES To;
	PYAMN_MIMESHORTNAMES Cc;
	PYAMN_MIMESHORTNAMES Bcc;
	char *Date;
	char Priority;
	char *Body;

	int CP;

	CShortHeader() {}
	~CShortHeader() {}
};

struct CHeader
//this header is used in miranda to store final results of mime reading in Unicode
{
	WCHAR *From;
	WCHAR *FromNick;
	WCHAR *ReturnPath;
	WCHAR *ReturnPathNick;
	WCHAR *Subject;
	PYAMN_MIMENAMES To;
	PYAMN_MIMENAMES Cc;
	PYAMN_MIMENAMES Bcc;
	WCHAR *Date;
	TCHAR Priority;
	WCHAR *Body;

	CHeader() {}
	~CHeader() {}
};

struct CMimeItem
{
	char *name;
	char *value;
	struct CMimeItem *Next;
	CMimeItem(): name(NULL), value(NULL), Next(NULL){}
};

typedef struct CMailData	//this is plugin-independent
{
#define	YAMN_MAILDATAVERSION	3

	DWORD Size;
	int CP;

	struct CMimeItem *TranslatedHeader;		//MIME items
	struct CMimeItem *Additional;			//MIME items not read from server (custom, for filter plugins etc.)
	char *Body;					//Message body

	CMailData(): CP(-1), Size(0), TranslatedHeader(NULL), Body(NULL){}
} MAILDATA,*PMAILDATA;

typedef struct CMimeMsgQueue
{
#define	YAMN_MAILVERSION	3
	char *ID;					//The ID of mail. This ID identifies every mail in the account, so plugin should set it

	DWORD Number;

#define YAMN_MSG_ANY		0xffffffff	//any mail

//The difference between new and unseen: when new mail is found in account, it becomes unseen and new. But in the next check, if the same mail is found, it is not new.
//However, when user was not near computer, he does not know about this mail- it is unseen. After user accepts, that he saw new mails, it becomes seen.
#define YAMN_MSG_NEW		0x80000000	//this mail is new
#define YAMN_MSG_UNSEEN		0x40000000	//this mail is mailbrowser unseen
#define YAMN_MSG_DISPLAY	0x20000000	//this mail can be displayed in mailbrowser
#define YAMN_MSG_POPUP		0x10000000	//this mail can be displayed in popup and can invoke a popup
#define YAMN_MSG_SYSTRAY	0x08000000	//this mail can invoke systray icon
#define	YAMN_MSG_BROWSER	0x04000000	//this mail can run mailbrowser
#define	YAMN_MSG_DISPLAYC	0x02000000	//this mail is inserted to browser mail counter system (the "Account - xx new mails, yy total" phrase)
#define	YAMN_MSG_POPUPC		0x01000000	//this mail is inserted to popup counter system (the "Account - xx new mails, yy total" phrase)

#define YAMN_MSG_SOUND		0x00800000	//this mail can "play sound"
#define YAMN_MSG_APP		0x00400000	//this mail can "launch application"
#define	YAMN_MSG_NEVENT		0x00100000	//this mail can launch Miranda "new mail" event

#define	YAMN_MSG_VIRTUAL	0x00080000	//this mail is not real- does not exists

#define YAMN_MSG_FILTERED	0x00040000	//this mail has been filtered

#define YAMN_MSG_DELETETRASH	0x00020000	//this mail should be moved to the trash bin rather than really deleting from mailbox (this is only switch doing nothing, perhaps usefull for filter plugins)
#define YAMN_MSG_DELETED	0x00010000	//this mail is already deleted from server (also must be set virtual flag)  (when doing synchronizations between 2 queues, YAMN then does not touch this mail)
#define YAMN_MSG_MEMDELETE	0x00008000	//this mail will be deleted immidiatelly from memory (and disk) when deleted from server (some opposite of YAMN_MSG_DELETED)
#define YAMN_MSG_USERDELETE	0x00004000	//this mail is about to delete from server (user deletes manually)
#define YAMN_MSG_AUTODELETE	0x00002000	//this mail is about to delete from server (plugin marks it for deleting)
#define YAMN_MSG_DELETEOK	0x00001000	//this mail is confirmed to delete (this flag must be set to delete this mail)

#define YAMN_MSG_BODYREQUESTED	0x00000800	//user requested (part of) the body. In POP3 it should be (TOP <nr> <lines>)
#define YAMN_MSG_BODYRECEIVED	0x00000200	//(part of) the body.received;
#define YAMN_MSG_STAYUNSEEN	0x00000400	//this mail stays unseen while user does not really see it

#define YAMN_MSG_DELETE		(YAMN_MSG_USERDELETE | YAMN_MSG_AUTODELETE)

#define YAMN_MSG_NORMALNEW	(YAMN_MSG_NEW | YAMN_MSG_UNSEEN | YAMN_MSG_BROWSER | YAMN_MSG_DISPLAY | YAMN_MSG_DISPLAYC | YAMN_MSG_POPUP | YAMN_MSG_POPUPC | YAMN_MSG_SYSTRAY | YAMN_MSG_SOUND | YAMN_MSG_APP | YAMN_MSG_NEVENT | YAMN_MSG_MEMDELETE | YAMN_MSG_STAYUNSEEN)

#define YAMN_MSG_FLAGSSET(maildata,flag)	((maildata & flag)==flag)

#define YAMN_MSG_SPAML1		1		//spam level 1: notify, show in another color in mail browser
#define YAMN_MSG_SPAML2		2		//spam level 2: do not notify, show in another color in mail browser
#define YAMN_MSG_SPAML3		3		//spam level 3: delete, show in another color in mail browser that it was deleted, you do not need to set YAMN_MSG_AUTODELETE
#define YAMN_MSG_SPAML4		4		//spam level 4: delete, do not show, you do not need to set YAMN_MSG_AUTODELETE
#define YAMN_MSG_SPAMMASK	0x0000000F

#define YAMN_MSG_SPAML(maildata,level)	((maildata & YAMN_MSG_SPAMMASK)==level)
	DWORD Flags;
//Plugins can read mail data, but it can be NULL!!! So plugin should use Load and Save services to load or save data and Unload to release data from memory
	PMAILDATA MailData;
//Here YAMN stores its own informations about this mail. Not usefull for plugins...
//	void *YAMNData;
	HWND MsgWindow;
//plugins can store here its own data
	void *PluginData;

	CMimeMsgQueue(): ID(NULL), Number(0), Flags(0), MailData(NULL), MsgWindow(NULL), PluginData(NULL), Next(NULL){}
	~CMimeMsgQueue() {}

	struct CMimeMsgQueue *Next;
} YAMNMAIL,*HYAMNMAIL;
#define	LoadedMailData(x)	(x->MailData!=NULL)

//
//================================== YAMN MAIL SERVICES ==================================
//

//CreateAccountMail Service
//Your plugin should call this to create new mail for your plugin.
//WPARAM- (HACCOUNT) Account handle
//LPARAM- CMailData version (use YAMN_MAILVERSION definition)
//returns pointer to (HYAMNMAIL) or pointer to your structure returned from imported NewMailFcnPtr, if implemented
#define	MS_YAMN_CREATEACCOUNTMAIL	"YAMN/Service/CreateMail"
#define CreateAccountMail(x)	(HYAMNMAIL)CallService(MS_YAMN_CREATEACCOUNTMAIL,(WPARAM)x,(LPARAM)YAMN_MAILVERSION)

//DeleteAccountMail Service
//Deletes plugin's mail from memory. You probably won't use this service, because it deletes only account
//without any synchronization. Use MS_YAMN_DELETEACCOUNT instead. Note that deleting mail is something like "this mail is
//not more in the account".
//WPARAM- (HYAMNPROTOPLUGIN) handle of plugin, which is going to delete mail
//LPARAM- (HYAMNMAIL) mail going to delete
//returns zero if failed, otherwise returns nonzero
#define	MS_YAMN_DELETEACCOUNTMAIL	"YAMN/Service/DeletePluginMail"
#define DeleteAccountMail(x,y)	CallService(MS_YAMN_DELETEACCOUNTMAIL,(WPARAM)x,(LPARAM)y)

//LoadMailData Service
//This service loads mail from standard YAMN storage (now it is 1 file, from which mails are loaded once at startup, but
//in the future it can be Miranda profile file or separate file (1 file per 1 mail). It depends on YAMN implementation...
//Use this function if you want to read or write to MailData member of mail structure. Please use synchronization obejcts
//before calling this service (so you must have read or write access to mails)
//WPARAM- (HYAMNMAIL) mail where to load data
//LPARAM- (DWORD) version of MAILDATA structure (use YAMN_MAILDATAVERSION definition)
//returns pointer to new allocated MailData structure (the same value as MailData member)
#define MS_YAMN_LOADMAILDATA		"YAMN/Service/LoadMailData"
#define LoadMailData(x)		(PMAILDATA)CallService(MS_YAMN_LOADMAILDATA,(WPARAM)x,(LPARAM)YAMN_MAILDATAVERSION)

//UnloadMailData Service
//This service frees mail data from memory. It does not care if data were saved or not. So you should save mail before you
//release data from memory.
//WPARAM- (HYAMNMAIL) mail whose data are about to free
//LPARAM- nothing yet
//returns nonzero if success
#define MS_YAMN_UNLOADMAILDATA		"YAMN/Service/UnloadMailData"
#define UnloadMailData(x)	CallService(MS_YAMN_UNLOADMAILDATA,(WPARAM)x,0)

//SaveMailData Service
//This service saves mail to standard YAMN storage (when using now 1 book file, it does nothing, because save is done when
//using MS_YAMN_WRITEACCOUNT service. In the future, mail can be saved to Miranda profile or to separate file...)
//WPARAM- (HYAMNMAIL) mail to save
//LPARAM- (DWORD) version of MAILDATA structure (use YAMN_MAILDATAVERSION definition)
//returns ZERO! if succes
#define MS_YAMN_SAVEMAILDATA		"YAMN/Service/SaveMailData"
#define SaveMailData(x)		CallService(MS_YAMN_SAVEMAILDATA,(WPARAM)x,(LPARAM)YAMN_MAILDATAVERSION)

//
//================================== FUNCTIONS DEFINITIONS ========================================
//

//typedef void (WINAPI *YAMN_SENDMESSAGEFCN)(UINT,WPARAM,LPARAM);
typedef void (WINAPI *YAMN_SYNCHROMIMEMSGSFCN)(HACCOUNT,HYAMNMAIL *,HYAMNMAIL *,HYAMNMAIL *,HYAMNMAIL *);
typedef void (WINAPI *YAMN_TRANSLATEHEADERFCN)(char *,int,struct CMimeItem **);
typedef void (WINAPI *YAMN_APPENDQUEUEFCN)(HYAMNMAIL,HYAMNMAIL);
typedef void (WINAPI *YAMN_DELETEMIMEQUEUEFCN)(HACCOUNT,HYAMNMAIL);
typedef void (WINAPI *YAMN_DELETEMIMEMESSAGEFCN)(HYAMNMAIL *,HYAMNMAIL,int);
typedef HYAMNMAIL (WINAPI *YAMN_FINDMIMEMESSAGEFCN)(HYAMNMAIL,char *);
typedef HYAMNMAIL (WINAPI *YAMN_CREATENEWDELETEQUEUEFCN)(HYAMNMAIL);
typedef void (WINAPI *YAMN_SETREMOVEQUEUEFLAGSFCN)(HYAMNMAIL,DWORD,DWORD,DWORD,int);

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
//		YAMN_SYNCHROMIMEMSGSFCN	SynchroMessagesFcn;
//		YAMN_APPENDQUEUEFCN	AppendQueueFcn;
//	} *pYAMNMailFcn;
//
//then you have to fill this structure with pointers...
//you have to use YAMN service to get pointer, like this (I wrote here all functions you may need,
//you can copy to your sources only those you need):
//
//	pYAMNMailFcn->SynchroMessagesFcn=(YAMN_SYNCHROMIMEMSGSFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SYNCHROMIMEMSGSID,0);
//	pYAMNMailFcn->TranslateHeaderFcn=(YAMN_TRANSLATEHEADERFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_TRANSLATEHEADERID,0);
//	pYAMNMailFcn->AppendQueueFcn=(YAMN_APPENDQUEUEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_APPENDQUEUEID,0);
//	pYAMNMailFcn->DeleteMessagesToEndFcn=(YAMN_DELETEMIMEQUEUEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_DELETEMIMEQUEUEID,0);
//	pYAMNMailFcn->DeleteMessageFromQueueFcn=(YAMN_DELETEMIMEMESSAGEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_DELETEMIMEMESSAGEID,0);
//	pYAMNMailFcn->FindMessageByIDFcn=(YAMN_FINDMIMEMESSAGEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_FINDMIMEMESSAGEID,0);
//	pYAMNMailFcn->CreateNewDeleteQueueFcn=(YAMN_CREATENEWDELETEQUEUEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_CREATENEWDELETEQUEUEID,0);
//	pYAMNMailFcn->SetRemoveQueueFlagsFcn=(YAMN_SETREMOVEQUEUEFLAGSFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SETREMOVEQUEUEFLAGSID,0);
//
//
//and in your plugin just simply use e.g.:
//
//	DeleteMIMEQueue(MyAccount,OldMessages);		//this command deletes all messages in the mail queue OldMessages
//

#define	YAMN_SYNCHROMIMEMSGSID		"YAMN/SynchroMessages"
#define	YAMN_TRANSLATEHEADERID		"YAMN/TranslateHeader"
#define	YAMN_APPENDQUEUEID		"YAMN/AppendQueue"
#define	YAMN_DELETEMIMEQUEUEID		"YAMN/DeleteMIMEQueue"
#define	YAMN_DELETEMIMEMESSAGEID	"YAMN/DeleteMIMEMessage"
#define	YAMN_FINDMIMEMESSAGEID		"YAMN/FindMIMEMessageByID"
#define	YAMN_CREATENEWDELETEQUEUEID	"YAMN/CreateNewDeleteQueue"
#define	YAMN_SETREMOVEQUEUEFLAGSID	"YAMN/SetRemoveQueueFlags"

#define YAMN_FLAG_REMOVE	0
#define YAMN_FLAG_SET		1


#define SynchroMessages(a,b,c,d,e)	pYAMNMailFcn->SynchroMessagesFcn(a,b,c,d,e)
#define TranslateHeader(a,b,c)		pYAMNMailFcn->TranslateHeaderFcn(a,b,c)
#define AppendQueue(x,y)		pYAMNMailFcn->AppendQueueFcn(x,y)
#define DeleteMIMEQueue(x,y)		pYAMNMailFcn->DeleteMessagesToEndFcn(x,y)
#define DeleteMIMEMessage(x,y)		pYAMNMailFcn->DeleteMessageFromQueueFcn(x,y,0)
#define DeleteMIMEMessageEx(x,y,z)	pYAMNMailFcn->DeleteMessageFromQueueFcn(x,y,z)
#define FindMIMEMessageByID(x,y)	pYAMNMailFcn->FindMessageByIDFcn(x,y)
#define CreateNewDeleteQueue(x)		pYAMNMailFcn->CreateNewDeleteQueueFcn(x)
#define SetQueueFlags(a,b,c,d)		pYAMNMailFcn->SetRemoveQueueFlagsFcn(a,b,c,d,1)
#define RemoveQueueFlags(a,b,c,d)	pYAMNMailFcn->SetRemoveQueueFlagsFcn(a,b,c,d,0)

#endif
