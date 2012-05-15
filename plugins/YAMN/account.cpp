/*
 * This code implements manipulation with accounts
 * such as reading accounts from file, writing them to file,
 * finding account by name etc.
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"
#include "mails/m_mails.h"
#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES) || defined(DEBUG_SYNCHRO)
	#include <stdio.h>
#endif

//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

extern PSWMRG PluginBrowserSO;
extern TCHAR AccountFileName[];
extern TCHAR *CurDir;
extern PLUGININFO pluginInfo;
extern YAMN_VARIABLES YAMNVar;

extern BOOL WINAPI SWMRGInitialize(PSWMRG pSWMRG,TCHAR *Name);
extern void WINAPI SWMRGDelete(PSWMRG pSWMRG);
extern DWORD WINAPI SWMRGWaitToWrite(PSWMRG pSWMRG,DWORD dwTimeout);
extern void WINAPI SWMRGDoneWriting(PSWMRG pSWMRG);
extern DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout);
extern void WINAPI SWMRGDoneReading(PSWMRG pSWMRG);
extern DWORD WINAPI WaitToReadFcn(PSWMRG SObject);
extern void WINAPI ReadDoneFcn(PSWMRG SObject);
extern HYAMNPROTOPLUGIN FindPlugin(DWORD PluginID);

extern void WINAPI DeleteMessagesToEndFcn(HACCOUNT Account,HYAMNMAIL From);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

// SMALL INTRO
// Accounts are queued in a queue (chained list). Pointer to first account is called "FirstAccount"
// Account queue is ended with NULL- pointered account (NULL handle)
// FirstAccount has every plugin in its own YAMN_PLUGIN structure

//Account status CS
//When we check some account, thread should change status of account to idle, connecting etc.
//So if we want to read status, we have to successfully write and then read.
LPCRITICAL_SECTION AccountStatusCS;

//File Writing CS
//When 2 threads want to write to file...
LPCRITICAL_SECTION FileWritingCS;

// Creates new account, which has plugin specified structure
INT_PTR CreatePluginAccountSvc(WPARAM wParam,LPARAM lParam);

// Deletes account from memory
INT_PTR DeletePluginAccountSvc(WPARAM wParam,LPARAM);

// Initializes standard YAMN account parameters
int InitAccount(HACCOUNT Which);

// Deinitializes (deletes) standard YAMN account parameters
void DeInitAccount(HACCOUNT Which);

// Sends signal (sets event) in AccountAccessSO and MessagesAccessSO
// This event makes that all calls to ...WaitToRead or ...WaitToWrite are returned to WAIT_FAILED
void StopSignalFcn(HACCOUNT Which);

// Function used to encrypt password (codes / decodes string)
// There's a problem when a character is going to be encrypted to (TCHAR)0, (end of string)
// Dest- input and output string (input/output length of string is 1:1)
// Encrypt- TRUE for encryption, FALSE for decryption
void CodeDecodeString(TCHAR *Dest,BOOL Encrypt);

// Only do the main work for FileToMemoryX functions. FileToMemoryX only opens file.
static DWORD PostFileToMemory(HANDLE File,TCHAR **MemFile,TCHAR **End);

// Reads the file and stores the content to allocated memory
// FileName- name of file 
// MemFile- pointer to the pointer of TCHAR, new allocated memory by this function
// End- where new allocated memory filled with file contents ends
DWORD FileToMemoryA(char *FileName,TCHAR **MemFile,TCHAR **End);

// Same as FileToMemoryA, but Unicode filename
DWORD FileToMemoryW(char *FileName,TCHAR **MemFile,TCHAR **End);

// Reads string that ends with character 0 and copies it to new allocated memory
// Parser-pointer to memory with string
//		function sets this parameter to point at the next char after read string
// End-information how long this string can be (in pointer to last possible char)
// StoreTo-function allocates memory and copies the string, allocated memory returned in StoreTo
// DebugString-the debug message showed in debug version
#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemory(TCHAR **Parser,TCHAR *End,TCHAR **StoreTo,TCHAR *DebugString);
#endif
DWORD ReadStringFromMemory(TCHAR **Parser,TCHAR *End,TCHAR **StoreTo);
#ifndef UNICODE
	#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo,WCHAR *DebugString);
	#endif  //if defined(DEBUG...)
DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo);
#endif	//ifdef Unicode

// Reads notification parameters from memory
// Parser-pointer to memory with string
//		function sets this parameter to point at the next char after read string
// End-information how long this string can be (in pointer to last possible char)
// Which-pointer to notification structure to fill with read parameters
static DWORD ReadNotificationFromMemory(TCHAR **Parser,TCHAR *End,YAMN_NOTIFICATION *Which);

// Reads messages from memory
// Which- address of account that mails (messages) belong to
// Parser- pointer to data from memory, function changes it to the next char after messages read
// End- the last possible char of messages
DWORD ReadMessagesFromMemory(HACCOUNT Which,TCHAR **Parser,TCHAR *End);

// Does all needed operations to read account
static INT_PTR PerformAccountReading(HYAMNPROTOPLUGIN Plugin,TCHAR *MemFile,TCHAR *End);

// Read one account from memory
// Which- address of account
// Parser- pointer to data from memory, function changes it to the next char after account read
// End- the last possible char of account
DWORD ReadAccountFromMemory(HACCOUNT Which,TCHAR **Parser,TCHAR *End);

// Inserts accounts read from file to actual account queue
INT_PTR AddAccountsFromFileASvc(WPARAM wParam,LPARAM lParam);

// Same as AddAccountsFromFileA, but filename is (WCHAR *) type- Unicode string
INT_PTR AddAccountsFromFileWSvc(WPARAM,LPARAM);

// Writes simple string to file
// File- handle of open file for writing
// Source- string to write, ended with zero character
DWORD WriteStringToFile(HANDLE File,TCHAR *Source);
#ifndef UNICODE
DWORD WriteStringToFileW(HANDLE File,WCHAR *Source);
#else
#define ReadStringFromMemoryW	ReadStringFromMemory
#endif

// Writes mails to file
// File- handle of file
// Which- address of account that mails belong to
DWORD WriteMessagesToFile(HANDLE File,HACCOUNT Which);

// Does all needed operations to write account to file
static INT_PTR PerformAccountWriting(HYAMNPROTOPLUGIN Plugin,HANDLE File);

// Writes accounts to file
// Accounts are read from plugin's account queue
// Function writes account data and calls WriteMessagesToFile to store account mails
INT_PTR WriteAccountsToFileASvc(WPARAM wParam,LPARAM lParam);

// Same as WriteAccountsToFileA, but filename is (WCHAR *) type- Unicode string
INT_PTR WriteAccountsToFileWSvc(WPARAM wParam,LPARAM lParam);

// Finds account by name and returns a pointer to it, or NULL when not found
// we do not have to synchronize accounts for read access, because we never change name of account
// (so if we want to change name of account we have to delete and create the new one)
INT_PTR FindAccountByNameSvc(WPARAM wParam,LPARAM lParam);

// Allocates a new account, and returns pointer to it
// calling function should have write access (using AccountBrowserSO)
// because new account is queues do account queue
INT_PTR GetNextFreeAccountSvc(WPARAM wParam,LPARAM lParam);

// Finds account for plugin
//int FindPluginAccount(WPARAM wParam,LPARAM lParam);

// Removes requested account from queue
// and deletes it in memory calling new thread function, that does it in the background
// This is very easy and the most secure way for plugins to delete account
INT_PTR DeleteAccountSvc(WPARAM wParam,LPARAM);

// This function is used as a thread function, that waits to signal for deleting account from memory
// This signal is signaled in UsingAccount.Event (this signales that no thread will be use account in the future)
DWORD WINAPI DeleteAccountInBackground(LPVOID Which);

// Sends signal that all works with Plugin accounts should be stopped and accounts should not be used anymore.
int StopAccounts(HYAMNPROTOPLUGIN Plugin);

// Wait for accounts to finish its work
int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin,BOOL GetAccountBrowserAccess=FALSE);

// Deletes protocol accounts using protocol defined delete function.
int DeleteAccounts(HYAMNPROTOPLUGIN Plugin);

// If you want to get directly account status, call this function
// You don't have to be in read-access to account and you can call this to retrieve status
void WINAPI GetStatusFcn(HACCOUNT Which,char *Value);

// If you want to set directly account status, call this function
// You don't have to be in write-access to account and you can call this to retrieve status
void WINAPI SetStatusFcn(HACCOUNT Which,char *Value);

struct CExportedFunctions AccountExportedFcn[]=
{
	{YAMN_GETSTATUSID,(void *)GetStatusFcn},
	{YAMN_SETSTATUSID,(void *)SetStatusFcn},
};

struct CExportedServices AccountExportedSvc[]=
{
	{MS_YAMN_CREATEPLUGINACCOUNT,CreatePluginAccountSvc},
	{MS_YAMN_DELETEPLUGINACCOUNT,DeletePluginAccountSvc},
	{MS_YAMN_FINDACCOUNTBYNAME,FindAccountByNameSvc},
	{MS_YAMN_GETNEXTFREEACCOUNT,GetNextFreeAccountSvc},
	{MS_YAMN_DELETEACCOUNT,DeletePluginAccountSvc},
	{MS_YAMN_READACCOUNTSA,AddAccountsFromFileASvc},
	{MS_YAMN_READACCOUNTSW,AddAccountsFromFileWSvc},
	{MS_YAMN_WRITEACCOUNTSA,WriteAccountsToFileASvc},
	{MS_YAMN_WRITEACCOUNTSW,WriteAccountsToFileWSvc},
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR CreatePluginAccountSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	DWORD AccountVersion=(DWORD)lParam;
	HACCOUNT NewAccount;

//test if we are going to initialize members of suitable structure (structures of plugin and YAMN must match)
	if(AccountVersion!=YAMN_ACCOUNTVERSION)
		return NULL;

	if(Plugin!=NULL)
	{
		if(Plugin->Fcn->NewAccountFcnPtr!=NULL)
		{
//Let plugin create its own structure, which can be derived from CAccount structure
			NewAccount=Plugin->Fcn->NewAccountFcnPtr(Plugin,YAMN_ACCOUNTVERSION);
			NewAccount->Plugin=Plugin;
		}
		else
		{
//We suggest plugin uses standard CAccount structure, so we create it
			NewAccount=new struct CAccount;
			NewAccount->Plugin=Plugin;
		}
//If not created successfully
		if(NewAccount==NULL)
			return NULL;
//Init every members of structure, used by YAMN
		InitAccount(NewAccount);

		return (INT_PTR)NewAccount;
	}
	return NULL;
}

INT_PTR DeletePluginAccountSvc(WPARAM wParam,LPARAM)
{
	HACCOUNT OldAccount=(HACCOUNT)wParam;

	if(OldAccount->Plugin->Fcn!=NULL)
	{
//Deinit every members and allocated fields of structure used by YAMN
		DeInitAccount(OldAccount);
		if(OldAccount->Plugin->Fcn->DeleteAccountFcnPtr!=NULL)
		{
//Let plugin delete its own CAccount derived structure
			OldAccount->Plugin->Fcn->DeleteAccountFcnPtr(OldAccount);
		}
		else
		{
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"DeletePluginAccountSvc:delete OldAccount\n");
#endif
			delete OldAccount;	//consider account as standard YAMN HACCOUNT and use its own destructor
		}
		return 1;
	}
	delete OldAccount;			//consider account as standard YAMN HACCOUNT, not initialized before and use its own destructor
	return 1;
}

int InitAccount(HACCOUNT Which)
{
//initialize synchronizing objects
	Which->AccountAccessSO=new SWMRG;
	SWMRGInitialize(Which->AccountAccessSO,NULL);
	Which->MessagesAccessSO=new SWMRG;
	SWMRGInitialize(Which->MessagesAccessSO,NULL);
	Which->UsingThreads=new SCOUNTER;
	SWMRGInitialize(Which->MessagesAccessSO,NULL);

//zero memory, where timestamps are stored
	ZeroMemory(&Which->LastChecked,sizeof(Which->LastChecked));
	ZeroMemory(&Which->LastSChecked,sizeof(Which->LastSChecked));
	ZeroMemory(&Which->LastSynchronised,sizeof(Which->LastSynchronised));
	ZeroMemory(&Which->LastMail,sizeof(Which->LastMail));

	Which->Name=NULL;
	Which->Mails=NULL;
	Which->Interval=0;
	Which->Flags=0;
	Which->StatusFlags=0;
	Which->Next=NULL;

	Which->Server=new struct CServer;
	Which->AbleToWork=TRUE;

	return 1;
}

void DeInitAccount(HACCOUNT Which)
{
//delete YAMN allocated fields
	if(Which->Name!=NULL)
		delete[] Which->Name;
	if(Which->Server->Name!=NULL)
		delete[] Which->Server->Name;
	if(Which->Server->Login!=NULL)
		delete[] Which->Server->Login;
	if(Which->Server->Passwd!=NULL)
		delete[] Which->Server->Passwd;
	if(Which->Server!=NULL)
		delete[] Which->Server;

	SWMRGDelete(Which->AccountAccessSO);
	delete Which->AccountAccessSO;
	SWMRGDelete(Which->MessagesAccessSO);
	delete Which->MessagesAccessSO;
	delete Which->UsingThreads;
	DeleteMessagesToEndFcn(Which,(HYAMNMAIL)Which->Mails);
}

void StopSignalFcn(HACCOUNT Which)
//set event that we are going to delete account
{
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tStopSignalFcn:stop account: %x\n",Which);
#endif
	Which->AbleToWork=FALSE;
//do not use synchronizing objects anymore
//any access to these objects then ends with WAIT_FAILED
	SetEvent(Which->AccountAccessSO->hFinishEV);
	SetEvent(Which->MessagesAccessSO->hFinishEV);
}

void CodeDecodeString(TCHAR *Dest,BOOL Encrypt)
{
	TCHAR Code=STARTCODEPSW;

	if(Dest==NULL)
		return;

	for(;*Dest!=(TCHAR)0;Dest++)
	{
		if(Encrypt)
			*Dest=*Dest+Code;
		else
			*Dest=*Dest-Code;
		Code+=(TCHAR)ADDCODEPSW;
	}
}

static DWORD PostFileToMemory(HANDLE File,TCHAR **MemFile,TCHAR **End)
{
	DWORD FileSize,ReadBytes;

	if(!(FileSize=GetFileSize(File,NULL)))
	{
		CloseHandle(File);
		return EACC_FILESIZE;
	}
//allocate space in memory, where we copy the whole file
	if(NULL==(*MemFile=(char *)new char[FileSize]))
	{
		CloseHandle(File);
		return EACC_ALLOC;
	}
//copy file to memory
	if(!ReadFile(File,(LPVOID)*MemFile,FileSize,&ReadBytes,NULL))
	{
		CloseHandle(File);
		delete[] *MemFile;
		return EACC_SYSTEM;
	}
	CloseHandle(File);
	*End=*MemFile+FileSize/sizeof(TCHAR);
	return 0;
}

DWORD FileToMemoryA(char *FileName,TCHAR **MemFile,TCHAR **End)
{
	HANDLE File;

	if(INVALID_HANDLE_VALUE==(File=CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)))
		return EACC_SYSTEM;

	return PostFileToMemory(File,MemFile,End);
}

DWORD FileToMemoryW(WCHAR *FileName,TCHAR **MemFile,TCHAR **End)
{
	HANDLE File;

	if(INVALID_HANDLE_VALUE==(File=CreateFileW(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)))
		return EACC_SYSTEM;

	return PostFileToMemory(File,MemFile,End);
}

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemory(TCHAR **Parser,TCHAR *End,TCHAR **StoreTo,TCHAR *DebugString)
{
//This is the debug version of ReadStringFromMemory function. This version shows MessageBox where
//read string is displayed
	TCHAR *Dest,*Finder;
	DWORD Size;
	TCHAR Debug[65536];

	Finder=*Parser;
	while((*Finder!=(TCHAR)0) && (Finder<=End)) Finder++;
	_stprintf(Debug,_T("%s: %s,length is %d, remaining %d chars"),DebugString,*Parser,Finder-*Parser,End-Finder);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
	if(Finder>=End)
		return EACC_FILECOMPATIBILITY;
	if(Size=Finder-*Parser)
	{
		if(NULL==(Dest=*StoreTo=new TCHAR[Size+1]))
			return EACC_ALLOC;
		for(;*Parser<=Finder;(*Parser)++,Dest++)
			*Dest=**Parser;
	}
	else
	{
		*StoreTo=NULL;
		(*Parser)++;
	}
	return 0;
}
#endif

DWORD ReadStringFromMemory(TCHAR **Parser,TCHAR *End,TCHAR **StoreTo)
{
	TCHAR *Dest,*Finder;
	DWORD Size;

	Finder=*Parser;
	while((*Finder!=(TCHAR)0) && (Finder<=End)) Finder++;
	if(Finder>=End)
		return EACC_FILECOMPATIBILITY;
	if(Size=Finder-*Parser)
	{
		if(NULL==(Dest=*StoreTo=new TCHAR[Size+1]))
			return EACC_ALLOC;
		for(;*Parser<=Finder;(*Parser)++,Dest++)
			*Dest=**Parser;
	}
	else
	{
		*StoreTo=NULL;
		(*Parser)++;
	}
	return 0;
}

#ifndef UNICODE
	#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo,WCHAR *DebugString)
{
//This is the debug version of ReadStringFromMemoryW function. This version shows MessageBox where
//read string is displayed
	WCHAR *Dest,*Finder;
	DWORD Size;
	WCHAR Debug[65536];

	Finder=*Parser;
	while((*Finder!=(WCHAR)0) && (Finder<=(WCHAR *)End)) Finder++;
	swprintf(Debug,L"%s: %s,length is %d, remaining %d chars",DebugString,*Parser,Finder-*Parser,(WCHAR *)End-Finder);
	MessageBoxW(NULL,Debug,L"debug",MB_OK);
	if(Finder>=(WCHAR *)End)
		return EACC_FILECOMPATIBILITY;
	if(Size=Finder-*Parser)
	{
		if(NULL==(Dest=*StoreTo=new WCHAR[Size+1]))
			return EACC_ALLOC;
		for(;*Parser<=Finder;(*Parser)++,Dest++)
			*Dest=**Parser;
	}
	else
	{
		*StoreTo=NULL;
		(*Parser)++;
	}
	return 0;
}
	#endif  //if defined(DEBUG...)

DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo)
{
	WCHAR *Dest,*Finder;
	DWORD Size;

	Finder=*Parser;
	while((*Finder!=(WCHAR)0) && (Finder<=(WCHAR *)End)) Finder++;
	if(Finder>=(WCHAR *)End)
		return EACC_FILECOMPATIBILITY;
	if(Size=Finder-*Parser)
	{
		if(NULL==(Dest=*StoreTo=new WCHAR[Size+1]))
			return EACC_ALLOC;
		for(;*Parser<=Finder;(*Parser)++,Dest++)
			*Dest=**Parser;
	}
	else
	{
		*StoreTo=NULL;
		(*Parser)++;
	}
	return 0;
}
#endif	//ifdef unicode

static DWORD ReadNotificationFromMemory(TCHAR **Parser,TCHAR *End,YAMN_NOTIFICATION *Which)
{
	DWORD Stat;
#ifdef DEBUG_FILEREAD
	TCHAR Debug[65536];
#endif

	Which->Flags=*(DWORD *)(*Parser);
	(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("NFlags: %04x, remaining %d chars"),Which->Flags,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

	Which->PopUpB=*(COLORREF *)(*Parser);
	(*Parser)+=sizeof(COLORREF)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("PopUpB: %04x, remaining %d chars"),Which->PopUpB,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->PopUpT=*(COLORREF *)(*Parser);
	(*Parser)+=sizeof(COLORREF)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("PopUpT: %04x, remaining %d chars"),Which->PopUpT,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->PopUpTime=*(DWORD *)(*Parser);
	(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("PopUpTime: %04x, remaining %d chars"),Which->PopUpTime,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

#ifdef	DEBUG_FILEREAD
	if(Stat=ReadStringFromMemoryW((WCHAR **)Parser,End,&Which->App,L"App"))
#else
	if(Stat=ReadStringFromMemoryW((WCHAR **)Parser,End,&Which->App))
#endif
		return Stat;
#ifdef	DEBUG_FILEREAD
	if(Stat=ReadStringFromMemoryW((WCHAR **)Parser,End,&Which->AppParam,L"AppParam"))
#else
	if(Stat=ReadStringFromMemoryW((WCHAR **)Parser,End,&Which->AppParam))
#endif
		return Stat;
	return 0;
}

DWORD ReadMessagesFromMemory(HACCOUNT Which,TCHAR **Parser,TCHAR *End)
{
	TCHAR *Finder;
	DWORD Size,Stat;
	HYAMNMAIL ActualMail=NULL;
	struct CMimeItem *items;
	char *ReadString;

#ifdef DEBUG_FILEREAD
	MessageBox(NULL,_T("going to read messages, if any..."),_T("debug"),MB_OK);
#endif
	do
	{
		Finder=*Parser;
		while((*Finder!=(TCHAR)0) && (Finder<=End)) Finder++;
		if(Finder>=End)
			return EACC_FILECOMPATIBILITY;
		if(Size=Finder-*Parser)
		{
			if(Which->Mails==NULL)		//First message in queue
			{
				if(NULL==(Which->Mails=ActualMail=CreateAccountMail(Which)))
					return EACC_ALLOC;
			}
			else
			{
				if(NULL==(ActualMail->Next=CreateAccountMail(Which))){
					return EACC_ALLOC;
				}
				ActualMail=ActualMail->Next;
			}
			items=NULL;
#ifdef DEBUG_FILEREADMESSAGES
			if(Stat=ReadStringFromMemory(Parser,End,&ActualMail->ID,_T("ID")))
#else
			if(Stat=ReadStringFromMemory(Parser,End,&ActualMail->ID))
#endif
				return Stat;
//			ActualMail->MailData=new MAILDATA;		 !!! mem leake !!! this is alloc by CreateAccountMail, no need for doubble alloc !!!!

			ActualMail->MailData->Size=*(DWORD *)(*Parser);
			(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
			if(*Parser>=End)
				return EACC_FILECOMPATIBILITY;
			ActualMail->Flags=*(DWORD *)(*Parser);
			(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
			if(*Parser>=End)
				return EACC_FILECOMPATIBILITY;
			ActualMail->Number=*(DWORD *)(*Parser);
			(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
			if(*Parser>=End)
				return EACC_FILECOMPATIBILITY;

			if((NULL!=Which->Plugin->MailFcn) && (NULL!=Which->Plugin->MailFcn->ReadMailOptsFcnPtr))
				Which->Plugin->MailFcn->ReadMailOptsFcnPtr(ActualMail,Parser,End);	//read plugin mail settings from file

			do
			{
#ifdef DEBUG_FILEREADMESSAGES
				if(Stat=ReadStringFromMemory(Parser,End,&ReadString,_T("Name")))
#else
				if(Stat=ReadStringFromMemory(Parser,End,&ReadString))
#endif
					return Stat;
				if(ReadString==NULL)
					break;

#ifdef DEBUG_DECODE
				DebugLog(DecodeFile,"<read name>%s</read name>",ReadString);
#endif

				if(items==NULL)
					items=ActualMail->MailData->TranslatedHeader=new struct CMimeItem;
				else
				{
					items->Next=new struct CMimeItem;
					items=items->Next;
				}
				if(items==NULL)
					return EACC_ALLOC;
				items->name=ReadString;

#ifdef DEBUG_FILEREADMESSAGES
				if(Stat=ReadStringFromMemory(Parser,End,&ReadString,_T("Value")))
#else
				if(Stat=ReadStringFromMemory(Parser,End,&ReadString))
#endif
					return Stat;
				items->value=ReadString;
#ifdef DEBUG_DECODE
				DebugLog(DecodeFile,"<read value>%s</read value>\n",ReadString);
#endif
			}while(1);
		}
		else
			break;		//no next messages, new account!

	}while(1);
	(*Parser)++;
	return 0;
}

DWORD ReadAccountFromMemory(HACCOUNT Which,TCHAR **Parser,TCHAR *End)
{
	DWORD Stat;
#ifdef DEBUG_FILEREAD
	TCHAR Debug[65536];
#endif
//Read name of account	
#ifdef DEBUG_FILEREAD
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Name,_T("Name")))
#else
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Name))
#endif
		return Stat;
	if(Which->Name==NULL)
		return EACC_FILECOMPATIBILITY;

//Read server parameters
#ifdef	DEBUG_FILEREAD
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Server->Name,_T("Server")))
#else
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Server->Name))
#endif
		return Stat;
	Which->Server->Port=*(WORD *)(*Parser);
	(*Parser)+=sizeof(WORD)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef	DEBUG_FILEREAD
	_stprintf(Debug,_T("Port: %d, remaining %d chars"),Which->Server->Port,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
#ifdef	DEBUG_FILEREAD
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Server->Login,_T("Login")))
#else
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Server->Login))
#endif
		return Stat;
#ifdef	DEBUG_FILEREAD
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Server->Passwd,_T("Password")))
#else
	if(Stat=ReadStringFromMemory(Parser,End,&Which->Server->Passwd))
#endif
		return Stat;
	CodeDecodeString(Which->Server->Passwd,FALSE);

//Read account flags
	Which->Flags=*(DWORD *)(*Parser);
	(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("Flags: %04x, remaining %d chars"),Which->Flags,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->StatusFlags=*(DWORD *)(*Parser);
	(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("STFlags: %04x, remaining %d chars"),Which->StatusFlags,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->PluginFlags=*(DWORD *)(*Parser);
	(*Parser)+=sizeof(DWORD)/sizeof(TCHAR);
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("PFlags: %04x, remaining %d chars"),Which->PluginFlags,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

//Read account miscellaneous parameters
	Which->Interval=*(WORD *)(*Parser);
	Which->TimeLeft=Which->Interval;		//check on loading
	(*Parser)+=sizeof(WORD)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("Interval: %d, remaining %d chars"),Which->Interval,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

//Read notification parameters
	if(Stat=ReadNotificationFromMemory(Parser,End,&Which->NewMailN))
		return Stat;
	if(Stat=ReadNotificationFromMemory(Parser,End,&Which->NoNewMailN))
		return Stat;
	if(Stat=ReadNotificationFromMemory(Parser,End,&Which->BadConnectN))
		return Stat;

//Let plugin read its own data stored in file
	if(Which->Plugin->Fcn!=NULL && Which->Plugin->Fcn->ReadPluginOptsFcnPtr!=NULL)
		if(Stat=Which->Plugin->Fcn->ReadPluginOptsFcnPtr(Which,Parser,End))
			return Stat;
//Read mails
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ReadAccountFromMemory:ActualAccountMsgsSO-write wait\n");
#endif
	WaitToWriteFcn(Which->MessagesAccessSO);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ReadAccountFromMemory:ActualAccountMsgsSO-write enter\n");
#endif
	if(Stat=ReadMessagesFromMemory(Which,Parser,End))
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"ReadAccountFromMemory:ActualAccountMsgsSO-write done\n");
#endif
		WriteDoneFcn(Which->MessagesAccessSO);
		return Stat;
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ReadAccountFromMemory:ActualAccountMsgsSO-write done\n");
#endif
	WriteDoneFcn(Which->MessagesAccessSO);

//Read timestamps
	Which->LastChecked=*(SYSTEMTIME *)(*Parser);
	(*Parser)+=sizeof(SYSTEMTIME)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("LastChecked: %04x, remaining %d chars"),Which->LastChecked,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->LastSChecked=*(SYSTEMTIME *)(*Parser);
	(*Parser)+=sizeof(SYSTEMTIME)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("LastSChecked: %04x, remaining %d chars"),Which->LastSChecked,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->LastSynchronised=*(SYSTEMTIME *)(*Parser);
	(*Parser)+=sizeof(SYSTEMTIME)/sizeof(TCHAR);
	if(*Parser>=End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("LastSynchronised: %04x, remaining %d chars"),Which->LastSynchronised,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->LastMail=*(SYSTEMTIME *)(*Parser);
	(*Parser)+=sizeof(SYSTEMTIME)/sizeof(TCHAR);
	if(*Parser>End)		//WARNING! There's only > at the end of testing
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	_stprintf(Debug,_T("LastMail: %04x, remaining %d chars"),Which->LastMail,End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	if(*Parser==End)
		return EACC_ENDOFFILE;
	return 0;
		
}

static INT_PTR PerformAccountReading(HYAMNPROTOPLUGIN Plugin,TCHAR *MemFile,TCHAR *End)
{
//Retrieve info for account from memory
	TCHAR *Parser;
	DWORD Ver,Stat;

	HACCOUNT ActualAccount,FirstAllocatedAccount;

	Ver=*(DWORD *)MemFile;
	if(Ver>YAMN_ACCOUNTFILEVERSION)
	{
		delete[] MemFile;
		return EACC_FILEVERSION;
	}
	Parser=MemFile+sizeof(Ver)/sizeof(TCHAR);

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write wait\n");
#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO,INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write enter\n");
#endif
	if(NULL==(ActualAccount=(HACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT,(WPARAM)Plugin,(LPARAM)YAMN_ACCOUNTVERSION)))
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write done\n");
#endif
		SWMRGDoneWriting(Plugin->AccountBrowserSO);
		delete[] MemFile;
		return EACC_ALLOC;
	}
	FirstAllocatedAccount=ActualAccount;
	
	do
	{
		HACCOUNT Temp;

#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"AddAccountsFromFile:ActualAccountSO-write wait\n");
#endif
		WaitToWriteFcn(ActualAccount->AccountAccessSO);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"AddAccountsFromFile:ActualAccountSO-write enter\n");
#endif
		Stat=ReadAccountFromMemory(ActualAccount,&Parser,End);

		if(ActualAccount->StatusFlags & (YAMN_ACC_STARTA | YAMN_ACC_STARTS))
			ActualAccount->TimeLeft=1;		//check on loading

		if(Stat && (Stat!=EACC_ENDOFFILE))
		{
			for(ActualAccount=FirstAllocatedAccount;ActualAccount!=NULL;ActualAccount=Temp)
			{
				Temp=ActualAccount->Next;
				delete ActualAccount;
			}
			delete[] MemFile;
			if(Plugin->FirstAccount==FirstAllocatedAccount)
				Plugin->FirstAccount=NULL;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"AddAccountsFromFile:ActualAccountSO-write done\n");
#endif
			SWMRGDoneWriting(Plugin->AccountBrowserSO);
			return (INT_PTR)Stat;
		}
		
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"AddAccountsFromFile:ActualAccountSO-write done\n");
#endif
		WriteDoneFcn(ActualAccount->AccountAccessSO);

		if((Stat!=EACC_ENDOFFILE) && (NULL==(ActualAccount=(HACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT,(WPARAM)Plugin,(LPARAM)YAMN_ACCOUNTVERSION))))
		{
			for(ActualAccount=FirstAllocatedAccount;ActualAccount!=NULL;ActualAccount=Temp)
			{
				Temp=ActualAccount->Next;
				delete ActualAccount;
			}
			delete[] MemFile;
			if(Plugin->FirstAccount==FirstAllocatedAccount)
				Plugin->FirstAccount=NULL;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write done\n");
#endif
			SWMRGDoneWriting(Plugin->AccountBrowserSO);
			return EACC_ALLOC;
		}
	}while(Stat!=EACC_ENDOFFILE);

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write done\n");
#endif
	SWMRGDoneWriting(Plugin->AccountBrowserSO);
	delete[] MemFile;

	return 0;
}

INT_PTR AddAccountsFromFileASvc(WPARAM wParam,LPARAM lParam)
//Add accounts from file to memory
{
	DWORD Stat;
	TCHAR *MemFile,*End;

	if(Stat=FileToMemoryA((char *)lParam,&MemFile,&End))
		return (INT_PTR)Stat;

	return PerformAccountReading((HYAMNPROTOPLUGIN)wParam,MemFile,End);

}

INT_PTR AddAccountsFromFileWSvc(WPARAM wParam,LPARAM lParam)
//Add accounts from file to memory
{
	DWORD Stat;
	TCHAR *MemFile,*End;

	if(Stat=FileToMemoryW((WCHAR *)lParam,&MemFile,&End))
		return (INT_PTR)Stat;

	return PerformAccountReading((HYAMNPROTOPLUGIN)wParam,MemFile,End);

}

DWORD WriteStringToFile(HANDLE File,TCHAR *Source)
{
	DWORD Length,WrittenBytes;
	TCHAR null=(TCHAR)0;

	if((Source==NULL) || !(Length=(DWORD)_tcslen(Source)))
	{
		if(!WriteFile(File,&null,sizeof(TCHAR),&WrittenBytes,NULL))
		{
			CloseHandle(File);
			return EACC_SYSTEM;
		}
	}
	else if(!WriteFile(File,Source,(Length+1)*sizeof(TCHAR),&WrittenBytes,NULL))
	{
		CloseHandle(File);
		return EACC_SYSTEM;
	}
	return 0;
}

#ifndef UNICODE
DWORD WriteStringToFileW(HANDLE File,WCHAR *Source)
{
	DWORD Length,WrittenBytes;
	WCHAR null=(WCHAR)0;

	if((Source==NULL) || !(Length=(DWORD)wcslen(Source)))
	{
		if(!WriteFile(File,&null,sizeof(WCHAR),&WrittenBytes,NULL))
		{
			CloseHandle(File);
			return EACC_SYSTEM;
		}
	}
	else if(!WriteFile(File,Source,(Length+1)*sizeof(WCHAR),&WrittenBytes,NULL))
		return EACC_SYSTEM;
	return 0;
}
#endif

DWORD WriteMessagesToFile(HANDLE File,HACCOUNT Which)
{
	DWORD WrittenBytes,Stat;
	HYAMNMAIL ActualMail=(HYAMNMAIL)Which->Mails;
	struct CMimeItem *items;

	while(ActualMail!=NULL)
	{
		if(Stat=WriteStringToFile(File,ActualMail->ID))
			return Stat;
		if(!WriteFile(File,(char *)&ActualMail->MailData->Size,sizeof(ActualMail->MailData->Size),&WrittenBytes,NULL) ||
			!WriteFile(File,(char *)&ActualMail->Flags,sizeof(ActualMail->Flags),&WrittenBytes,NULL) ||
			!WriteFile(File,(char *)&ActualMail->Number,sizeof(ActualMail->Number),&WrittenBytes,NULL))
			return EACC_SYSTEM;
		if((NULL!=Which->Plugin->MailFcn) && (NULL!=Which->Plugin->MailFcn->WriteMailOptsFcnPtr))
			Which->Plugin->MailFcn->WriteMailOptsFcnPtr(File,ActualMail);	//write plugin mail options to file
		for(items=ActualMail->MailData->TranslatedHeader;items!=NULL;items=items->Next)
		{
			if(Stat=WriteStringToFile(File,items->name))
				return Stat;
			if(Stat=WriteStringToFile(File,items->value))
				return Stat;
		}		
		if(Stat=WriteStringToFile(File,_T("")))
			return Stat;
		ActualMail=ActualMail->Next;
	}
	if(Stat=WriteStringToFile(File,_T("")))
		return Stat;
	return 0;
}

static INT_PTR PerformAccountWriting(HYAMNPROTOPLUGIN Plugin,HANDLE File)
{
	DWORD WrittenBytes,Stat;
	HACCOUNT ActualAccount;
	DWORD Ver=YAMN_ACCOUNTFILEVERSION;
	BOOL Writed=FALSE;
	DWORD ReturnValue=0,EnterCode;
	
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"WriteAccountsToFile:AccountBrowserSO-read wait\n");
#endif
	SWMRGWaitToRead(Plugin->AccountBrowserSO,INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"WriteAccountsToFile:AccountBrowserSO-read enter\n");
#endif
	try
	{
		for(ActualAccount=Plugin->FirstAccount;ActualAccount!=NULL;ActualAccount=ActualAccount->Next)
		{
/*			TCHAR DEBUG[100];
			Beep(3000,100);Sleep(200);
			_stprintf(DEBUG,_T("Browsing account %s"),ActualAccount->Name==NULL ? _T("(null)") : ActualAccount->Name);
			MessageBox(NULL,DEBUG,_T("debug- WriteAccount..."),MB_OK);
*/
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read wait\n");
#endif
			EnterCode=WaitToReadFcn(ActualAccount->AccountAccessSO);
			if(EnterCode==WAIT_FINISH)		//account is about to delete
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read wait failed\n");
#endif
				ActualAccount=ActualAccount->Next;
				continue;
			}
			if(EnterCode==WAIT_FAILED)		//account is deleted
				break;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read enter\n");
#endif
			if((ActualAccount->Name==NULL) || (*ActualAccount->Name==(TCHAR)0))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read done\n");
#endif
				ReadDoneFcn(ActualAccount->AccountAccessSO);
				continue;
			}

			if(!Writed && !WriteFile(File,&Ver,sizeof(Ver),&WrittenBytes,NULL))
				throw (DWORD)EACC_SYSTEM;
			Writed=TRUE;

			if(Stat=WriteStringToFile(File,ActualAccount->Name))
				throw (DWORD)Stat;

			if(Stat=WriteStringToFile(File,ActualAccount->Server->Name))
				throw (DWORD)Stat;

			if(!WriteFile(File,(char *)&ActualAccount->Server->Port,2,&WrittenBytes,NULL))
				throw (DWORD)EACC_SYSTEM;

			if((Stat=WriteStringToFile(File,ActualAccount->Server->Login)))
				throw (DWORD)Stat;

			CodeDecodeString(ActualAccount->Server->Passwd,TRUE);

			if(Stat=WriteStringToFile(File,ActualAccount->Server->Passwd))
			{
				CodeDecodeString(ActualAccount->Server->Passwd,FALSE);
				throw (DWORD)Stat;
			}
			CodeDecodeString(ActualAccount->Server->Passwd,FALSE);

			if((!WriteFile(File,(char *)&ActualAccount->Flags,sizeof(DWORD),&WrittenBytes,NULL) ||
				(!WriteFile(File,(char *)&ActualAccount->StatusFlags,sizeof(DWORD),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->PluginFlags,sizeof(DWORD),&WrittenBytes,NULL))))
				throw (DWORD)EACC_SYSTEM;

			if(!WriteFile(File,(char *)&ActualAccount->Interval,sizeof(WORD),&WrittenBytes,NULL))
				throw (DWORD)EACC_SYSTEM;

			if((!WriteFile(File,(char *)&ActualAccount->NewMailN.Flags,sizeof(DWORD),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->NewMailN.PopUpB,sizeof(COLORREF),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->NewMailN.PopUpT,sizeof(COLORREF),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->NewMailN.PopUpTime,sizeof(DWORD),&WrittenBytes,NULL)))
				throw (DWORD)EACC_SYSTEM;

			if((Stat=WriteStringToFileW(File,ActualAccount->NewMailN.App)) ||
				(Stat=WriteStringToFileW(File,ActualAccount->NewMailN.AppParam)))
				throw (DWORD)Stat;

			if((!WriteFile(File,(char *)&ActualAccount->NoNewMailN.Flags,sizeof(DWORD),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->NoNewMailN.PopUpB,sizeof(COLORREF),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->NoNewMailN.PopUpT,sizeof(COLORREF),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->NoNewMailN.PopUpTime,sizeof(DWORD),&WrittenBytes,NULL)))
				throw (DWORD)EACC_SYSTEM;

			if((Stat=WriteStringToFileW(File,ActualAccount->NoNewMailN.App)) ||
				(Stat=WriteStringToFileW(File,ActualAccount->NoNewMailN.AppParam)))
				throw (DWORD)Stat;

			if((!WriteFile(File,(char *)&ActualAccount->BadConnectN.Flags,sizeof(DWORD),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->BadConnectN.PopUpB,sizeof(COLORREF),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->BadConnectN.PopUpT,sizeof(COLORREF),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->BadConnectN.PopUpTime,sizeof(DWORD),&WrittenBytes,NULL)))
				throw (DWORD)EACC_SYSTEM;

			if((Stat=WriteStringToFileW(File,ActualAccount->BadConnectN.App)) ||
				(Stat=WriteStringToFileW(File,ActualAccount->BadConnectN.AppParam)))
				throw (DWORD)Stat;

//Let plugin write its own values into file
			if(ActualAccount->Plugin->Fcn!=NULL && ActualAccount->Plugin->Fcn->WritePluginOptsFcnPtr!=NULL)
				if(Stat=ActualAccount->Plugin->Fcn->WritePluginOptsFcnPtr(File,ActualAccount))
					throw (DWORD)Stat;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountMsgsSO-read wait\n");
#endif
			WaitToReadFcn(ActualAccount->MessagesAccessSO);
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountMsgsSO-read enter\n");
#endif
			if(Stat=WriteMessagesToFile(File,ActualAccount))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountMsgsSO-read done\n");
#endif
				ReadDoneFcn(ActualAccount->MessagesAccessSO);
				throw (DWORD)Stat;
			}
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountMsgsSO-read done\n");
#endif
			ReadDoneFcn(ActualAccount->MessagesAccessSO);

			if((!WriteFile(File,(char *)&ActualAccount->LastChecked,sizeof(SYSTEMTIME),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->LastSChecked,sizeof(SYSTEMTIME),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->LastSynchronised,sizeof(SYSTEMTIME),&WrittenBytes,NULL)) ||
				(!WriteFile(File,(char *)&ActualAccount->LastMail,sizeof(SYSTEMTIME),&WrittenBytes,NULL)))
				throw (DWORD)Stat;

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read done\n");
#endif
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
	}
	catch(DWORD ErrorCode)
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read done\n");
#endif
		ReadDoneFcn(ActualAccount->AccountAccessSO);
		ReturnValue=ErrorCode;
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"WriteAccountsToFile:AccountBrowserSO-read done\n");
#endif
	SWMRGDoneReading(Plugin->AccountBrowserSO);
	CloseHandle(File);
	return 0;
}

INT_PTR WriteAccountsToFileASvc(WPARAM wParam,LPARAM lParam)
//Writes accounts to file
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	char *FileName=(char *)lParam;

	HANDLE File;

	EnterCriticalSection(FileWritingCS);
	if(INVALID_HANDLE_VALUE==(File=CreateFileA(FileName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)))
	{
		LeaveCriticalSection(FileWritingCS);
		return EACC_SYSTEM;
	}
	LeaveCriticalSection(FileWritingCS);

	return PerformAccountWriting(Plugin,File);
}

INT_PTR WriteAccountsToFileWSvc(WPARAM wParam,LPARAM lParam)
//Writes accounts to file
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	WCHAR *FileName=(WCHAR *)lParam;
	INT_PTR rv;

	HANDLE File;

	EnterCriticalSection(FileWritingCS);
	if(INVALID_HANDLE_VALUE==(File=CreateFileW(FileName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)))
	{
		LeaveCriticalSection(FileWritingCS);
		return EACC_SYSTEM;
	}

	rv=PerformAccountWriting(Plugin,File);

	LeaveCriticalSection(FileWritingCS);

	return rv;
}

INT_PTR FindAccountByNameSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	TCHAR *SearchedAccount=(TCHAR *)lParam;
	HACCOUNT Finder;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"FindAccountByName:AccountBrowserSO-read wait\n");
#endif
	SWMRGWaitToRead(Plugin->AccountBrowserSO,INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"FindAccountByName:AccountBrowserSO-read enter\n");
#endif
	for(Finder=Plugin->FirstAccount;Finder!=NULL;Finder=Finder->Next)
		if((Finder->Name!=NULL) && (0==_tcscmp(SearchedAccount,Finder->Name)))
			break;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"FindAccountByName:AccountBrowserSO-read done\n");
#endif
	SWMRGDoneReading(Plugin->AccountBrowserSO);
	return (INT_PTR)Finder; 
}

INT_PTR GetNextFreeAccountSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	HACCOUNT Finder;

	if(Plugin->FirstAccount==NULL)
	{
		Plugin->FirstAccount=(HACCOUNT)CallService(MS_YAMN_CREATEPLUGINACCOUNT,wParam,lParam);
		return (INT_PTR)Plugin->FirstAccount;
	}
	for(Finder=Plugin->FirstAccount;Finder->Next!=NULL;Finder=Finder->Next);
	Finder->Next=(HACCOUNT)CallService(MS_YAMN_CREATEPLUGINACCOUNT,wParam,lParam);
	return (INT_PTR)Finder->Next;
}

/*
int FindPluginAccount(WPARAM wParam,LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	HACCOUNT Finder=(HACCOUNT)lParam;

	if(Finder=NULL)	Finder=Plugin->FirstAccount;

//	for(;Finder!=NULL && Finder->PluginID!=Plugin->PluginInfo->PluginID;Finder=(HACCOUNT)Finder->Next);
	return (int)Finder;
}
*/
INT_PTR DeleteAccountSvc(WPARAM wParam,LPARAM lParam)
{
//Deleting account works on these steps:
//1. set signal that account should stop activity (set event)
//	setting this event we achieve, that any access to account is failed,
//	so threads do not start any work with accounts (better saying threads of plugins should not start)
//2. wait to get write access to chained list of accounts
//3. we can write to chained list, so we change chain not to show to actual account
//	now, any thread browsing list of accounts does not browse through actual account
//	actual account seems to be hidden (it exists, but it is not in accounts chained list (chained list=queue))
//Now, we should delete account from memory, BUT!!!
//	Any thread can still be waked up and start asking account synchronizing object
//	If account is deleted, asking about access to read account can throw memory exception (reading for
//	a synchronizing object from memory, that was deleted)
//So, we cannot now delete account. We have to wait until we are sure no thread will be using account anymore
//	(or to the end of Miranda, but problem is in allocated memory- it is allocated and Miranda is SMALLER, faster, easier, isn't it?)
//	This deleting is achieved in 2 ways:
//	We have event in UsingThreads synchronization objects. This event signals that no thread will use actual account
//	1. Any thread using account first increment UsingThread, so we know that account is used
//	2. If thread is about to close, it should decrement UsingThread
//	3. If thread creates another thread, that will use account, caller has to wait until the new thread does not
//		increment UsingThreads (imagine that caller ends before the new thread set it: if no other thread is using
//		account, account is automaticaly (decreasing UsingThreads) signaled as "not used" and we delete it. But then
//		new thread is going to read account...).
//4. wait until UsingThread Event is signaled
//5. delete account from memory

	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	HACCOUNT Which=(HACCOUNT)lParam;
	HACCOUNT Finder;
	DWORD tid;

//1. set stop signal 
	StopSignalFcn(Which);
	WindowList_BroadcastAsync(YAMNVar.MessageWnds,WM_YAMN_STOPACCOUNT,(WPARAM)Which,(LPARAM)0);
	if(Plugin->Fcn->StopAccountFcnPtr!=NULL)
		Plugin->Fcn->StopAccountFcnPtr(Which);

//2. wait to get write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write wait\n");
#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO,INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write enter\n");
#endif

//3. remove from queue (chained list)
	if(Plugin->FirstAccount==NULL)
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write done\n");
#endif
		SWMRGDoneWriting(Plugin->AccountBrowserSO);
		return 0;
	}
	if(Plugin->FirstAccount==Which)
	{
		Finder=Plugin->FirstAccount->Next;
		Plugin->FirstAccount=Finder;
	}
	else
	{
		for(Finder=Plugin->FirstAccount;Which!=Finder->Next;Finder=Finder->Next);
		Finder->Next=Finder->Next->Next;
	}
//leave write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write done\n");
#endif
	SWMRGDoneWriting(Plugin->AccountBrowserSO);

//4. wait while event "UsingThread" is not signaled
//	And what to do, if this event will be signaled in 1 hour? (Although it's paranoia, because we have sent "delete signal", so
//	other threads do not start any new work with actual account) We will wait in blocked state?
//	No, of course not. We will create new thread, that will wait and additionally remove our thread in background.
//5. So, the last point (deleting from memory) is performed in new DeleteAccountInBackground thread

	if((Plugin->Fcn!=NULL) && (Plugin->Fcn->WriteAccountsFcnPtr!=NULL))
			Plugin->Fcn->WriteAccountsFcnPtr();
	CloseHandle(CreateThread(NULL,0,DeleteAccountInBackground,(LPVOID)Which,0,&tid));

//Now, plugin can consider account as deleted, but plugin really can achieve deleting this account from memory when using
//event UsingThreads.
	return 1;
}

DWORD WINAPI DeleteAccountInBackground(LPVOID Value)
{
	HACCOUNT Which=(HACCOUNT)Value;
	WaitForSingleObject(Which->UsingThreads->Event,INFINITE);
	CallService(MS_YAMN_DELETEPLUGINACCOUNT,(WPARAM)Which,(LPARAM)0);
	return 0;
}

int StopAccounts(HYAMNPROTOPLUGIN Plugin)
{
	HACCOUNT Finder;

//1. wait to get write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"StopAccounts:AccountBrowserSO-write wait\n");
#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO,INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"StopAccounts:AccountBrowserSO-write enter\n");
#endif
	for(Finder=Plugin->FirstAccount;Finder!=NULL;Finder=Finder->Next)
	{
//2. set stop signal 
		StopSignalFcn(Finder);
		WindowList_BroadcastAsync(YAMNVar.MessageWnds,WM_YAMN_STOPACCOUNT,(WPARAM)Finder,(LPARAM)0);
		if(Plugin->Fcn->StopAccountFcnPtr!=NULL)
			Plugin->Fcn->StopAccountFcnPtr(Finder);
	}

//leave write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"StopAccounts:AccountBrowserSO-write done\n");
#endif
	SWMRGDoneWriting(Plugin->AccountBrowserSO);

//Now, account is stopped. It can be removed from memory...
	return 1;
}

int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin,BOOL GetAccountBrowserAccess)
{
	HACCOUNT Finder;

	if(GetAccountBrowserAccess)
	{
//1. wait to get write access
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:AccountBrowserSO-write wait\n");
#endif
		SWMRGWaitToWrite(Plugin->AccountBrowserSO,INFINITE);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:AccountBrowserSO-write enter\n");
#endif
	}
	for(Finder=Plugin->FirstAccount;Finder!=NULL;Finder=Finder->Next)
	{
//2. wait for signal that account is not in use
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:waiting for UsingThreadEV %x (account %x)\n",Finder->UsingThreads,Finder);
#endif
		WaitForSingleObject(Finder->UsingThreads->Event,INFINITE);
		SetEvent(Finder->UsingThreads->Event);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:UsingThreadEV signaled\n");
#endif
	}
	if(GetAccountBrowserAccess)
	{
//leave write access
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:AccountBrowserSO-write done\n");
#endif
		SWMRGDoneWriting(Plugin->AccountBrowserSO);
	}

	return 1;
}

int DeleteAccounts(HYAMNPROTOPLUGIN Plugin)
{
	HACCOUNT Finder;

	//1. wait to get write access
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccounts:AccountBrowserSO-write wait\n");
	#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO,INFINITE);
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccounts:AccountBrowserSO-write enter\n");
	#endif

	WaitForAllAccounts(Plugin,FALSE);

	for(Finder=Plugin->FirstAccount;Finder!=NULL;)
	{
		HACCOUNT Next = Finder->Next;
		DeletePluginAccountSvc((WPARAM)Finder,(LPARAM)0);
		Finder = Next;
	}

	//leave write access
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccounts:AccountBrowserSO-write done\n");
	#endif
	SWMRGDoneWriting(Plugin->AccountBrowserSO);

	return 1;
}

void WINAPI GetStatusFcn(HACCOUNT Which,char *Value)
{
	if(Which==NULL)
		return;
	
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetStatus:AccountStatusCS-cs wait\n");
#endif
	EnterCriticalSection(AccountStatusCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetStatus:AccountStatusCS-cs enter\n");
#endif
	lstrcpy(Value,Which->Status);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetStatus:AccountStatusCS-cs done\n");
#endif
	LeaveCriticalSection(AccountStatusCS);
	return;
}

void WINAPI SetStatusFcn(HACCOUNT Which,char *Value)
{
	if(Which==NULL)
		return;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSetStatus:AccountStatusCS-cs wait\n");
#endif
	EnterCriticalSection(AccountStatusCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSetStatus:AccountStatusCS-cs enter\n");
#endif
	lstrcpy(Which->Status,Value);
	WindowList_BroadcastAsync(YAMNVar.MessageWnds,WM_YAMN_CHANGESTATUS,(WPARAM)Which,(LPARAM)0);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSetStatus:AccountStatusCS-cs done\n");
#endif
	LeaveCriticalSection(AccountStatusCS);
}

/*
#ifdef DEBUG_ACCOUNTS
int GetAccounts()
{
	HACCOUNT Finder;
	int cnt=0;

	for(Finder=Account;Finder!=NULL;Finder=Finder->Next)
		cnt++;
	return cnt;
}

void WriteAccounts()
{
	HACCOUNT Finder;

	for(Finder=Account;Finder!=NULL;Finder=Finder->Next)
		MessageBoxA(NULL,Finder->Name,"Browsing account",MB_OK);
}
#endif
*/
