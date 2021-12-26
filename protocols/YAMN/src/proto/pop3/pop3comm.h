#ifndef __POP3COMM_H
#define __POP3COMM_H

#define	POP3_FILEVERSION	1	//Version of aditional information stored in book file

typedef struct CPOP3Account: public CAccount
{
// We can use SCOUNTER structure, because this is internal plugin.
// This SO is used to determine if any POP3 account is in "write access" mode
	static PSCOUNTER AccountWriterSO;

// It is usefull to have client structure in account. With this structure we have access to account's socket.
// This is related to InternetQueries and UseInternetFree
// This member should be synchronized with UseInternetFree
	class CPop3Client Client;

// This member is usefull for MIME headers. It is default codepage, if no other codepage found
	uint16_t CP;				//access only through AccountAccessSO

// In this memeber last error code is stored
	uint32_t SystemError;			//access through UseInternetFree

// We use only counter from this object and it is # of threads waiting to work on internet.
// We use event UseInternet to access critical sections.
// It is usefull in 2 ways: we have mutual exclusion that only one thread works with account on internet.
// Thread, which has done its work with account on internet can close socket, but it is not needed, when any other
// thread wants to work (e.g. we have deleted mails, but when deleting, another thread wants to check new mail, so
// we delete all needed mails and check if there's thread that wants to work. If yes, we do not need to quit session,
// we leave socket open, and leave internet. Another thread then start checking and does not connect, does not send
// user and password... because socket is open- it continues)
	PSCOUNTER InternetQueries;
	HANDLE UseInternetFree;

	CPOP3Account();
	~CPOP3Account();

} POP3ACCOUNT,*HPOP3ACCOUNT;

typedef struct POP3LayeredError
{
	BOOL SSL;
	uint32_t AppError;
	uint32_t POP3Error;
	uint32_t NetError;
	uint32_t SystemError;
} POP3_ERRORCODE,*PPOP3_ERRORCODE;

struct YAMNExportedFcns
{
	YAMN_SETPROTOCOLPLUGINFCNIMPORTFCN	SetProtocolPluginFcnImportFcn;
	YAMN_WAITTOWRITEFCN	WaitToWriteFcn;
	YAMN_WRITEDONEFCN	WriteDoneFcn;
	YAMN_WAITTOREADFCN	WaitToReadFcn;
	YAMN_READDONEFCN	ReadDoneFcn;
	YAMN_SCMANAGEFCN	SCGetNumberFcn;
	YAMN_SCMANAGEFCN	SCIncFcn;
	YAMN_SCMANAGEFCN	SCDecFcn;
	YAMN_SETSTATUSFCN	SetStatusFcn;
	YAMN_GETSTATUSFCN	GetStatusFcn;
};

struct MailExportedFcns
{
	YAMN_SYNCHROMIMEMSGSFCN	SynchroMessagesFcn;
	YAMN_TRANSLATEHEADERFCN	TranslateHeaderFcn;
	YAMN_APPENDQUEUEFCN	AppendQueueFcn;
	YAMN_DELETEMIMEQUEUEFCN	DeleteMessagesToEndFcn;
	YAMN_DELETEMIMEMESSAGEFCN	DeleteMessageFromQueueFcn;
	YAMN_FINDMIMEMESSAGEFCN	FindMessageByIDFcn;
	YAMN_CREATENEWDELETEQUEUEFCN	CreateNewDeleteQueueFcn;
};

enum
{
	EACC_QUEUEALLOC=1,	//memory allocation
	EACC_STOPPED,		//stop account
};

#define	NO_MAIL_FOR_DELETE	1

#define	POP3_DELETEFROMCHECK	1

#endif
