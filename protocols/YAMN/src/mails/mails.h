#ifndef __MAILS_H
#define __MAILS_H

struct CAccount;

//
//================================== OTHER DEFINITIONS ========================================
//

struct CShortNames
{
	char *Value;
	char *ValueNick;
	CShortNames *Next;	
};

struct CMimeNames
{
	WCHAR *Value;
	WCHAR *ValueNick;
	CMimeNames *Next;
};

//this header is used in to get non-unicode data from mime header
struct CShortHeader
{
	char *From;
	char *FromNick;
	char *ReturnPath;
	char *ReturnPathNick;
	char *Subject;
	CShortNames *To;
	CShortNames *Cc;
	CShortNames *Bcc;
	char *Date;
	char Priority;
	char *Body;

	int CP;
};

//this header is used in miranda to store final results of mime reading in Unicode
struct CHeader
{
	~CHeader();

	CMStringW wszFrom;
	CMStringW wszFromNick;
	CMStringW wszReturnPath;
	CMStringW wszReturnPathNick;
	CMStringW wszSubject;
	CMimeNames *To = 0;
	CMimeNames *Cc = 0;
	CMimeNames *Bcc = 0;
	CMStringW wszDate;
	TCHAR Priority = 0;
	CMStringW wszBody;
};

struct CMimeItem
{
	char *name = nullptr;
	char *value = nullptr;
	CMimeItem *Next = nullptr;
};

// this is plugin-independent
struct CMailData
{
	DWORD Size = 0;
	int CP = -1;

	CMimeItem *TranslatedHeader = nullptr; // MIME items
	CMimeItem *Additional = nullptr;       // MIME items not read from server (custom, for filter plugins etc.)
	char *Body = nullptr;                  // Message body
};

struct YAMNMAIL : public MZeroedObject
{
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
	CMailData *MailData;

	HWND MsgWindow;

	//plugins can store here its own data
	void *PluginData;

	YAMNMAIL *Next;
};
#define	LoadedMailData(x)	(x->MailData!=nullptr)

//
//================================== FUNCTIONS DEFINITIONS ========================================
//

//typedef void (WINAPI *YAMN_SENDMESSAGEFCN)(UINT,WPARAM,LPARAM);
typedef void (WINAPI *YAMN_SYNCHROMIMEMSGSFCN)(CAccount *, YAMNMAIL **, YAMNMAIL **, YAMNMAIL **, YAMNMAIL **);
typedef void (WINAPI *YAMN_TRANSLATEHEADERFCN)(char *, int, struct CMimeItem **);
typedef void (WINAPI *YAMN_APPENDQUEUEFCN)(YAMNMAIL *, YAMNMAIL *);
typedef void (WINAPI *YAMN_DELETEMIMEQUEUEFCN)(CAccount *, YAMNMAIL *);
typedef void (WINAPI *YAMN_DELETEMIMEMESSAGEFCN)(YAMNMAIL **, YAMNMAIL *, int);
typedef YAMNMAIL *(WINAPI *YAMN_FINDMIMEMESSAGEFCN)(YAMNMAIL *, char *);
typedef YAMNMAIL *(WINAPI *YAMN_CREATENEWDELETEQUEUEFCN)(YAMNMAIL *);
typedef void (WINAPI *YAMN_SETREMOVEQUEUEFLAGSFCN)(YAMNMAIL *, DWORD, DWORD, DWORD, int);

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
