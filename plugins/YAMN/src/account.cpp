/*
 * This code implements manipulation with accounts
 * such as reading accounts from file, writing them to file,
 * finding account by name etc.
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"

//Account status CS
//When we check some account, thread should change status of account to idle, connecting etc.
//So if we want to read status, we have to successfully write and then read.
CRITICAL_SECTION AccountStatusCS;

//File Writing CS
//When 2 threads want to write to file...
CRITICAL_SECTION FileWritingCS;

struct CExportedFunctions AccountExportedFcn[] =
{
	{ YAMN_GETSTATUSID, (void *)GetStatusFcn },
	{ YAMN_SETSTATUSID, (void *)SetStatusFcn },
};

struct CExportedServices AccountExportedSvc[] =
{
	{ MS_YAMN_CREATEPLUGINACCOUNT, CreatePluginAccountSvc },
	{ MS_YAMN_DELETEPLUGINACCOUNT, DeletePluginAccountSvc },
	{ MS_YAMN_FINDACCOUNTBYNAME, FindAccountByNameSvc },
	{ MS_YAMN_GETNEXTFREEACCOUNT, GetNextFreeAccountSvc },
	{ MS_YAMN_DELETEACCOUNT, DeletePluginAccountSvc },
	{ MS_YAMN_READACCOUNTS, AddAccountsFromFileSvc },
	{ MS_YAMN_WRITEACCOUNTS, WriteAccountsToFileSvc },
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR CreatePluginAccountSvc(WPARAM wParam, LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin = (HYAMNPROTOPLUGIN)wParam;
	DWORD AccountVersion = (DWORD)lParam;
	HACCOUNT NewAccount;

	//test if we are going to initialize members of suitable structure (structures of plugin and YAMN must match)
	if (AccountVersion != YAMN_ACCOUNTVERSION)
		return NULL;

	if (Plugin != NULL)
	{
		if (Plugin->Fcn->NewAccountFcnPtr != NULL)
		{
			//Let plugin create its own structure, which can be derived from CAccount structure
			NewAccount = Plugin->Fcn->NewAccountFcnPtr(Plugin, YAMN_ACCOUNTVERSION);
			NewAccount->Plugin = Plugin;
		}
		else
		{
			//We suggest plugin uses standard CAccount structure, so we create it
			NewAccount = new struct CAccount;
			NewAccount->Plugin = Plugin;
		}
		//If not created successfully
		if (NewAccount == NULL)
			return NULL;
		//Init every members of structure, used by YAMN
		InitAccount(NewAccount);

		return (INT_PTR)NewAccount;
	}
	return NULL;
}

INT_PTR DeletePluginAccountSvc(WPARAM wParam, LPARAM)
{
	HACCOUNT OldAccount = (HACCOUNT)wParam;

	if (OldAccount->Plugin->Fcn != NULL)
	{
		//Deinit every members and allocated fields of structure used by YAMN
		DeInitAccount(OldAccount);
		if (OldAccount->Plugin->Fcn->DeleteAccountFcnPtr != NULL)
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
	Which->AccountAccessSO = new SWMRG;
	SWMRGInitialize(Which->AccountAccessSO, NULL);
	Which->MessagesAccessSO = new SWMRG;
	SWMRGInitialize(Which->MessagesAccessSO, NULL);
	Which->UsingThreads = new SCOUNTER;
	SWMRGInitialize(Which->MessagesAccessSO, NULL);

	//zero memory, where timestamps are stored
	memset(&Which->LastChecked, 0, sizeof(Which->LastChecked));
	memset(&Which->LastSChecked, 0, sizeof(Which->LastSChecked));
	memset(&Which->LastSynchronised, 0, sizeof(Which->LastSynchronised));
	memset(&Which->LastMail, 0, sizeof(Which->LastMail));

	Which->Name = NULL;
	Which->Mails = NULL;
	Which->Interval = 0;
	Which->Flags = 0;
	Which->StatusFlags = 0;
	Which->Next = NULL;

	Which->Server = new struct CServer;
	Which->AbleToWork = TRUE;

	return 1;
}

void DeInitAccount(HACCOUNT Which)
{
	//delete YAMN allocated fields
	if (Which->Name != NULL)
		delete[] Which->Name;
	if (Which->Server != NULL) {
		if (Which->Server->Name != NULL)
			delete[] Which->Server->Name;
		if (Which->Server->Login != NULL)
			delete[] Which->Server->Login;
		if (Which->Server->Passwd != NULL)
			delete[] Which->Server->Passwd;
		delete[] Which->Server;
	}

	SWMRGDelete(Which->AccountAccessSO);
	delete Which->AccountAccessSO;
	SWMRGDelete(Which->MessagesAccessSO);
	delete Which->MessagesAccessSO;
	delete Which->UsingThreads;
	DeleteMessagesToEndFcn(Which, (HYAMNMAIL)Which->Mails);
}

void StopSignalFcn(HACCOUNT Which)
//set event that we are going to delete account
{
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tStopSignalFcn:stop account: %x\n",Which);
#endif
	Which->AbleToWork = FALSE;
	//do not use synchronizing objects anymore
	//any access to these objects then ends with WAIT_FAILED
	SetEvent(Which->AccountAccessSO->hFinishEV);
	SetEvent(Which->MessagesAccessSO->hFinishEV);
}

void CodeDecodeString(char *Dest, BOOL Encrypt)
{
	TCHAR Code = STARTCODEPSW;

	if (Dest == NULL)
		return;

	for (; *Dest != (TCHAR)0; Dest++)
	{
		if (Encrypt)
			*Dest = *Dest + Code;
		else
			*Dest = *Dest - Code;
		Code += (TCHAR)ADDCODEPSW;
	}
}

static DWORD PostFileToMemory(HANDLE File, char **MemFile, char **End)
{
	DWORD FileSize, ReadBytes;
	if (!(FileSize = GetFileSize(File, NULL))) {
		CloseHandle(File);
		return EACC_FILESIZE;
	}

	//allocate space in memory, where we copy the whole file
	if (NULL == (*MemFile = new char[FileSize]))
	{
		CloseHandle(File);
		return EACC_ALLOC;
	}

	//copy file to memory
	if (!ReadFile(File, (LPVOID)*MemFile, FileSize, &ReadBytes, NULL))
	{
		CloseHandle(File);
		delete[] * MemFile;
		return EACC_SYSTEM;
	}
	CloseHandle(File);
	*End = *MemFile + FileSize;
	return 0;
}

DWORD FileToMemory(TCHAR *FileName, char **MemFile, char **End)
{
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return EACC_SYSTEM;

	return PostFileToMemory(hFile, MemFile, End);
}

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemory(char **Parser,TCHAR *End,char **StoreTo,TCHAR *DebugString)
{
	//This is the debug version of ReadStringFromMemory function. This version shows MessageBox where
	//read string is displayed
	TCHAR *Dest,*Finder;
	DWORD Size;
	TCHAR Debug[65536];

	Finder=*Parser;
	while((*Finder != (TCHAR)0) && (Finder<=End)) Finder++;
	mir_sntprintf(Debug, SIZEOF(Debug), _T("%s: %s,length is %d, remaining %d chars"), DebugString, *Parser, Finder-*Parser, End-Finder);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
	if (Finder>=End)
		return EACC_FILECOMPATIBILITY;
	if (Size=Finder-*Parser)
	{
		if (NULL==(Dest=*StoreTo=new TCHAR[Size+1]))
			return EACC_ALLOC;
		for (;*Parser<=Finder;(*Parser)++,Dest++)
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

DWORD ReadStringFromMemory(char **Parser, char *End, char **StoreTo)
{
	char *Dest, *Finder;
	DWORD Size;

	Finder = *Parser;
	while ((*Finder != (TCHAR)0) && (Finder <= End)) Finder++;
	if (Finder >= End)
		return EACC_FILECOMPATIBILITY;
	if (Size = Finder - *Parser)
	{
		if (NULL == (Dest = *StoreTo = new char[Size + 1]))
			return EACC_ALLOC;
		for (; *Parser <= Finder; (*Parser)++, Dest++)
			*Dest = **Parser;
	}
	else
	{
		*StoreTo = NULL;
		(*Parser)++;
	}
	return 0;
}

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemoryW(WCHAR **Parser,TCHAR *End,WCHAR **StoreTo,WCHAR *DebugString)
{
	//This is the debug version of ReadStringFromMemoryW function. This version shows MessageBox where
	//read string is displayed
	WCHAR *Dest,*Finder;
	DWORD Size;
	WCHAR Debug[65536];

	Finder=*Parser;
	while((*Finder != (WCHAR)0) && (Finder<=(WCHAR *)End)) Finder++;
	mir_sntprintf(Debug, SIZEOF(Debug), L"%s: %s,length is %d, remaining %d chars", DebugString, *Parser, Finder-*Parser, (WCHAR *)End-Finder);
	MessageBoxW(NULL,Debug,L"debug",MB_OK);
	if (Finder>=(WCHAR *)End)
		return EACC_FILECOMPATIBILITY;
	if (Size=Finder-*Parser)
	{
		if (NULL==(Dest=*StoreTo=new WCHAR[Size+1]))
			return EACC_ALLOC;
		for (;*Parser<=Finder;(*Parser)++,Dest++)
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

DWORD ReadStringFromMemoryW(WCHAR **Parser, WCHAR *End, WCHAR **StoreTo)
{
	WCHAR *Dest, *Finder;
	DWORD Size;

	Finder = *Parser;
	while ((*Finder != (WCHAR)0) && (Finder <= (WCHAR *)End)) Finder++;
	if (Finder >= (WCHAR *)End)
		return EACC_FILECOMPATIBILITY;
	if (Size = Finder - *Parser)
	{
		if (NULL == (Dest = *StoreTo = new WCHAR[Size + 1]))
			return EACC_ALLOC;
		for (; *Parser <= Finder; (*Parser)++, Dest++)
			*Dest = **Parser;
	}
	else
	{
		*StoreTo = NULL;
		(*Parser)++;
	}
	return 0;
}

static DWORD ReadNotificationFromMemory(char **Parser, char *End, YAMN_NOTIFICATION *Which)
{
	DWORD Stat;
#ifdef DEBUG_FILEREAD
	TCHAR Debug[65536];
#endif

	Which->Flags = *(DWORD *)(*Parser);
	(*Parser) += sizeof(DWORD);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("NFlags: %04x, remaining %d chars"), Which->Flags, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

	Which->PopupB = *(COLORREF *)(*Parser);
	(*Parser) += sizeof(COLORREF);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("PopupB: %04x, remaining %d chars"), Which->PopupB, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->PopupT = *(COLORREF *)(*Parser);
	(*Parser) += sizeof(COLORREF);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("PopupT: %04x, remaining %d chars"), Which->PopupT, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->PopupTime = *(DWORD *)(*Parser);
	(*Parser) += sizeof(DWORD);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("PopupTime: %04x, remaining %d chars"), Which->PopupTime, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

#ifdef	DEBUG_FILEREAD
	if (Stat=ReadStringFromMemoryW((WCHAR **)Parser,(WCHAR*)End,&Which->App,L"App"))
#else
	if (Stat = ReadStringFromMemoryW((WCHAR **)Parser, (WCHAR*)End, &Which->App))
#endif
		return Stat;
#ifdef	DEBUG_FILEREAD
	if (Stat=ReadStringFromMemoryW((WCHAR **)Parser,(WCHAR*)End,&Which->AppParam,L"AppParam"))
#else
	if (Stat = ReadStringFromMemoryW((WCHAR **)Parser, (WCHAR*)End, &Which->AppParam))
#endif
		return Stat;
	return 0;
}

DWORD ReadMessagesFromMemory(HACCOUNT Which, char **Parser, char *End)
{
	char *Finder;
	DWORD Size, Stat;
	HYAMNMAIL ActualMail = NULL;
	struct CMimeItem *items;
	char *ReadString;

#ifdef DEBUG_FILEREAD
	MessageBox(NULL,_T("going to read messages, if any..."),_T("debug"),MB_OK);
#endif
	do
	{
		Finder = *Parser;
		while ((*Finder != (TCHAR)0) && (Finder <= End)) Finder++;
		if (Finder >= End)
			return EACC_FILECOMPATIBILITY;
		if (Size = Finder - *Parser)
		{
			if (Which->Mails == NULL)		//First message in queue
			{
				if (NULL == (Which->Mails = ActualMail = CreateAccountMail(Which)))
					return EACC_ALLOC;
			}
			else
			{
				if (NULL == (ActualMail->Next = CreateAccountMail(Which))) {
					return EACC_ALLOC;
				}
				ActualMail = ActualMail->Next;
			}
			items = NULL;
#ifdef DEBUG_FILEREADMESSAGES
			if (Stat=ReadStringFromMemory(Parser,End,&ActualMail->ID,_T("ID")))
#else
			if (Stat = ReadStringFromMemory(Parser, End, &ActualMail->ID))
#endif
				return Stat;
			//			ActualMail->MailData=new MAILDATA;		 !!! mem leake !!! this is alloc by CreateAccountMail, no need for doubble alloc !!!!

			ActualMail->MailData->Size = *(DWORD *)(*Parser);
			(*Parser) += sizeof(DWORD);
			if (*Parser >= End)
				return EACC_FILECOMPATIBILITY;
			ActualMail->Flags = *(DWORD *)(*Parser);
			(*Parser) += sizeof(DWORD);
			if (*Parser >= End)
				return EACC_FILECOMPATIBILITY;
			ActualMail->Number = *(DWORD *)(*Parser);
			(*Parser) += sizeof(DWORD);
			if (*Parser >= End)
				return EACC_FILECOMPATIBILITY;

			if ((NULL != Which->Plugin->MailFcn) && (NULL != Which->Plugin->MailFcn->ReadMailOptsFcnPtr))
				Which->Plugin->MailFcn->ReadMailOptsFcnPtr(ActualMail, Parser, End);	//read plugin mail settings from file

			do
			{
#if defined(DEBUG_FILEREADMESSAGES) || defined(DEBUG_FILEREAD)
				if (Stat=ReadStringFromMemory(Parser,End,&ReadString,_T("Name")))
#else
				if (Stat = ReadStringFromMemory(Parser, End, &ReadString))
#endif
					return Stat;
				if (ReadString == NULL)
					break;

#ifdef DEBUG_DECODE
				DebugLog(DecodeFile,"<read name>%s</read name>",ReadString);
#endif

				if (items == NULL)
					items = ActualMail->MailData->TranslatedHeader = new struct CMimeItem;
				else
				{
					items->Next = new struct CMimeItem;
					items = items->Next;
				}
				if (items == NULL)
					return EACC_ALLOC;
				items->name = ReadString;

#ifdef DEBUG_FILEREADMESSAGES
				if (Stat=ReadStringFromMemory(Parser,End,&ReadString,_T("Value")))
#else
				if (Stat = ReadStringFromMemory(Parser, End, &ReadString))
#endif
					return Stat;
				items->value = ReadString;
#ifdef DEBUG_DECODE
				DebugLog(DecodeFile,"<read value>%s</read value>\n",ReadString);
#endif
			} while (1);
		}
		else
			break;		//no next messages, new account!

	} while (1);
	(*Parser)++;
	return 0;
}

DWORD ReadAccountFromMemory(HACCOUNT Which, char **Parser, char *End)
{
	DWORD Stat;
#ifdef DEBUG_FILEREAD
	TCHAR Debug[65536];
#endif
	//Read name of account	

#ifdef DEBUG_FILEREAD
	if (Stat=ReadStringFromMemory(Parser,End,&Which->Name,_T("Name")))
#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Name))
#endif
		return Stat;
	if (Which->Name == NULL)
		return EACC_FILECOMPATIBILITY;

	//Read server parameters
#ifdef	DEBUG_FILEREAD
	if (Stat=ReadStringFromMemory(Parser,End,&Which->Server->Name,_T("Server")))
#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Name))
#endif
		return Stat;
	Which->Server->Port = *(WORD *)(*Parser);
	(*Parser) += sizeof(WORD);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef	DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("Port: %d, remaining %d chars"), Which->Server->Port, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
#ifdef	DEBUG_FILEREAD
	if (Stat=ReadStringFromMemory(Parser,End,&Which->Server->Login,_T("Login")))
#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Login))
#endif
		return Stat;
#ifdef	DEBUG_FILEREAD
	if (Stat=ReadStringFromMemory(Parser,End,&Which->Server->Passwd,_T("Password")))
#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Passwd))
#endif
		return Stat;
	CodeDecodeString(Which->Server->Passwd, FALSE);

	//Read account flags
	Which->Flags = *(DWORD *)(*Parser);
	(*Parser) += sizeof(DWORD);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("Flags: %04x, remaining %d chars"), Which->Flags, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->StatusFlags = *(DWORD *)(*Parser);
	(*Parser) += sizeof(DWORD);
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("STFlags: %04x, remaining %d chars"), Which->StatusFlags, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->PluginFlags = *(DWORD *)(*Parser);
	(*Parser) += sizeof(DWORD);
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("PFlags: %04x, remaining %d chars"), Which->PluginFlags, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

	//Read account miscellaneous parameters
	Which->Interval = *(WORD *)(*Parser);
	Which->TimeLeft = Which->Interval;		//check on loading
	(*Parser) += sizeof(WORD);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("Interval: %d, remaining %d chars"), Which->Interval, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif

	//Read notification parameters
	if (Stat = ReadNotificationFromMemory(Parser, End, &Which->NewMailN))
		return Stat;
	if (Stat = ReadNotificationFromMemory(Parser, End, &Which->NoNewMailN))
		return Stat;
	if (Stat = ReadNotificationFromMemory(Parser, End, &Which->BadConnectN))
		return Stat;

	//Let plugin read its own data stored in file
	if (Which->Plugin->Fcn != NULL && Which->Plugin->Fcn->ReadPluginOptsFcnPtr != NULL)
		if (Stat = Which->Plugin->Fcn->ReadPluginOptsFcnPtr(Which, Parser, End))
			return Stat;
	//Read mails
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ReadAccountFromMemory:ActualAccountMsgsSO-write wait\n");
#endif
	WaitToWriteFcn(Which->MessagesAccessSO);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ReadAccountFromMemory:ActualAccountMsgsSO-write enter\n");
#endif
	if (Stat = ReadMessagesFromMemory(Which, Parser, End))
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
	Which->LastChecked = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("LastChecked: %04x, remaining %d chars"), Which->LastChecked, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->LastSChecked = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("LastSChecked: %04x, remaining %d chars"), Which->LastSChecked, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->LastSynchronised = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("LastSynchronised: %04x, remaining %d chars"), Which->LastSynchronised, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	Which->LastMail = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser > End)		//WARNING! There's only > at the end of testing
		return EACC_FILECOMPATIBILITY;
#ifdef DEBUG_FILEREAD
	mir_sntprintf(Debug, SIZEOF(Debug), _T("LastMail: %04x, remaining %d chars"), Which->LastMail, End-*Parser);
	MessageBox(NULL,Debug,_T("debug"),MB_OK);
#endif
	if (*Parser == End)
		return EACC_ENDOFFILE;
	return 0;

}

static INT_PTR PerformAccountReading(HYAMNPROTOPLUGIN Plugin, char *MemFile, char *End)
{
	//Retrieve info for account from memory
	char *Parser;
	DWORD Ver, Stat;

	HACCOUNT ActualAccount, FirstAllocatedAccount;

	Ver = *(DWORD *)MemFile;
	if (Ver > YAMN_ACCOUNTFILEVERSION)
	{
		delete[] MemFile;
		return EACC_FILEVERSION;
	}
	Parser = MemFile + sizeof(Ver);

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write wait\n");
#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write enter\n");
#endif
	if (NULL == (ActualAccount = (HACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT, (WPARAM)Plugin, (LPARAM)YAMN_ACCOUNTVERSION)))
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write done\n");
#endif
		SWMRGDoneWriting(Plugin->AccountBrowserSO);
		delete[] MemFile;
		return EACC_ALLOC;
	}
	FirstAllocatedAccount = ActualAccount;

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
		Stat = ReadAccountFromMemory(ActualAccount, &Parser, End);

		if (ActualAccount->StatusFlags & (YAMN_ACC_STARTA | YAMN_ACC_STARTS))
			ActualAccount->TimeLeft = 1;		//check on loading

		if (Stat && (Stat != EACC_ENDOFFILE))
		{
			for (ActualAccount = FirstAllocatedAccount; ActualAccount != NULL; ActualAccount = Temp)
			{
				Temp = ActualAccount->Next;
				delete ActualAccount;
			}
			delete[] MemFile;
			if (Plugin->FirstAccount == FirstAllocatedAccount)
				Plugin->FirstAccount = NULL;
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

		if ((Stat != EACC_ENDOFFILE) && (NULL == (ActualAccount = (HACCOUNT)CallService(MS_YAMN_GETNEXTFREEACCOUNT, (WPARAM)Plugin, (LPARAM)YAMN_ACCOUNTVERSION))))
		{
			for (ActualAccount = FirstAllocatedAccount; ActualAccount != NULL; ActualAccount = Temp)
			{
				Temp = ActualAccount->Next;
				delete ActualAccount;
			}
			delete[] MemFile;
			if (Plugin->FirstAccount == FirstAllocatedAccount)
				Plugin->FirstAccount = NULL;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write done\n");
#endif
			SWMRGDoneWriting(Plugin->AccountBrowserSO);
			return EACC_ALLOC;
		}
	} while (Stat != EACC_ENDOFFILE);

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"AddAccountsFromFile:AccountBrowserSO-write done\n");
#endif
	SWMRGDoneWriting(Plugin->AccountBrowserSO);
	delete[] MemFile;

	return 0;
}

// Add accounts from file to memory
INT_PTR AddAccountsFromFileSvc(WPARAM wParam, LPARAM lParam)
{
	char *MemFile, *End;
	DWORD Stat = FileToMemory((TCHAR*)lParam, &MemFile, &End);
	if (Stat != NO_ERROR)
		return (INT_PTR)Stat;

	return PerformAccountReading((HYAMNPROTOPLUGIN)wParam, MemFile, End);
}

DWORD WriteStringToFile(HANDLE File, char *Source)
{
	DWORD Length, WrittenBytes;
	char null = 0;

	if ((Source == NULL) || !(Length = (DWORD)mir_strlen(Source))) {
		if (!WriteFile(File, &null, 1, &WrittenBytes, NULL)) {
			CloseHandle(File);
			return EACC_SYSTEM;
		}
	}
	else if (!WriteFile(File, Source, (Length + 1), &WrittenBytes, NULL)) {
		CloseHandle(File);
		return EACC_SYSTEM;
	}
	return 0;
}

DWORD WriteStringToFileW(HANDLE File, WCHAR *Source)
{
	DWORD Length, WrittenBytes;
	WCHAR null = (WCHAR)0;

	if ((Source == NULL) || !(Length = (DWORD)mir_wstrlen(Source)))
	{
		if (!WriteFile(File, &null, sizeof(WCHAR), &WrittenBytes, NULL))
		{
			CloseHandle(File);
			return EACC_SYSTEM;
		}
	}
	else if (!WriteFile(File, Source, (Length + 1)*sizeof(WCHAR), &WrittenBytes, NULL))
		return EACC_SYSTEM;
	return 0;
}

DWORD WriteMessagesToFile(HANDLE File, HACCOUNT Which)
{
	DWORD WrittenBytes, Stat;
	HYAMNMAIL ActualMail = (HYAMNMAIL)Which->Mails;
	struct CMimeItem *items;

	while (ActualMail != NULL)
	{
		if (Stat = WriteStringToFile(File, ActualMail->ID))
			return Stat;
		if (!WriteFile(File, (char *)&ActualMail->MailData->Size, sizeof(ActualMail->MailData->Size), &WrittenBytes, NULL) ||
			!WriteFile(File, (char *)&ActualMail->Flags, sizeof(ActualMail->Flags), &WrittenBytes, NULL) ||
			!WriteFile(File, (char *)&ActualMail->Number, sizeof(ActualMail->Number), &WrittenBytes, NULL))
			return EACC_SYSTEM;
		if ((NULL != Which->Plugin->MailFcn) && (NULL != Which->Plugin->MailFcn->WriteMailOptsFcnPtr))
			Which->Plugin->MailFcn->WriteMailOptsFcnPtr(File, ActualMail);	//write plugin mail options to file
		for (items = ActualMail->MailData->TranslatedHeader; items != NULL; items = items->Next)
		{
			if (Stat = WriteStringToFile(File, items->name))
				return Stat;
			if (Stat = WriteStringToFile(File, items->value))
				return Stat;
		}
		if (Stat = WriteStringToFile(File, ""))
			return Stat;
		ActualMail = ActualMail->Next;
	}
	if (Stat = WriteStringToFile(File, ""))
		return Stat;
	return 0;
}

static INT_PTR PerformAccountWriting(HYAMNPROTOPLUGIN Plugin, HANDLE File)
{
	DWORD WrittenBytes, Stat;
	HACCOUNT ActualAccount;
	DWORD Ver = YAMN_ACCOUNTFILEVERSION;
	BOOL Writed = FALSE;
	DWORD ReturnValue = 0, EnterCode;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"WriteAccountsToFile:AccountBrowserSO-read wait\n");
#endif
	SWMRGWaitToRead(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"WriteAccountsToFile:AccountBrowserSO-read enter\n");
#endif
	try
	{
		for (ActualAccount = Plugin->FirstAccount; ActualAccount != NULL; ActualAccount = ActualAccount->Next)
		{
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read wait\n");
#endif
			EnterCode = WaitToReadFcn(ActualAccount->AccountAccessSO);
			if (EnterCode == WAIT_FINISH)		//account is about to delete
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read wait failed\n");
#endif
				ActualAccount = ActualAccount->Next;
				continue;
			}
			if (EnterCode == WAIT_FAILED)		//account is deleted
				break;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read enter\n");
#endif
			if ((ActualAccount->Name == NULL) || (*ActualAccount->Name == (TCHAR)0))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read done\n");
#endif
				ReadDoneFcn(ActualAccount->AccountAccessSO);
				continue;
			}

			if (!Writed && !WriteFile(File, &Ver, sizeof(Ver), &WrittenBytes, NULL))
				throw (DWORD)EACC_SYSTEM;
			Writed = TRUE;

			if (Stat = WriteStringToFile(File, ActualAccount->Name))
				throw (DWORD)Stat;

			if (Stat = WriteStringToFile(File, ActualAccount->Server->Name))
				throw (DWORD)Stat;

			if (!WriteFile(File, (char *)&ActualAccount->Server->Port, 2, &WrittenBytes, NULL))
				throw (DWORD)EACC_SYSTEM;

			if ((Stat = WriteStringToFile(File, ActualAccount->Server->Login)))
				throw (DWORD)Stat;

			CodeDecodeString(ActualAccount->Server->Passwd, TRUE);

			if (Stat = WriteStringToFile(File, ActualAccount->Server->Passwd))
			{
				CodeDecodeString(ActualAccount->Server->Passwd, FALSE);
				throw (DWORD)Stat;
			}
			CodeDecodeString(ActualAccount->Server->Passwd, FALSE);

			if ((!WriteFile(File, (char *)&ActualAccount->Flags, sizeof(DWORD), &WrittenBytes, NULL) ||
				(!WriteFile(File, (char *)&ActualAccount->StatusFlags, sizeof(DWORD), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->PluginFlags, sizeof(DWORD), &WrittenBytes, NULL))))
				throw (DWORD)EACC_SYSTEM;

			if (!WriteFile(File, (char *)&ActualAccount->Interval, sizeof(WORD), &WrittenBytes, NULL))
				throw (DWORD)EACC_SYSTEM;

			if ((!WriteFile(File, (char *)&ActualAccount->NewMailN.Flags, sizeof(DWORD), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->NewMailN.PopupB, sizeof(COLORREF), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->NewMailN.PopupT, sizeof(COLORREF), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->NewMailN.PopupTime, sizeof(DWORD), &WrittenBytes, NULL)))
				throw (DWORD)EACC_SYSTEM;

			if ((Stat = WriteStringToFileW(File, ActualAccount->NewMailN.App)) ||
				(Stat = WriteStringToFileW(File, ActualAccount->NewMailN.AppParam)))
				throw (DWORD)Stat;

			if ((!WriteFile(File, (char *)&ActualAccount->NoNewMailN.Flags, sizeof(DWORD), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->NoNewMailN.PopupB, sizeof(COLORREF), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->NoNewMailN.PopupT, sizeof(COLORREF), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->NoNewMailN.PopupTime, sizeof(DWORD), &WrittenBytes, NULL)))
				throw (DWORD)EACC_SYSTEM;

			if ((Stat = WriteStringToFileW(File, ActualAccount->NoNewMailN.App)) ||
				(Stat = WriteStringToFileW(File, ActualAccount->NoNewMailN.AppParam)))
				throw (DWORD)Stat;

			if ((!WriteFile(File, (char *)&ActualAccount->BadConnectN.Flags, sizeof(DWORD), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->BadConnectN.PopupB, sizeof(COLORREF), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->BadConnectN.PopupT, sizeof(COLORREF), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->BadConnectN.PopupTime, sizeof(DWORD), &WrittenBytes, NULL)))
				throw (DWORD)EACC_SYSTEM;

			if ((Stat = WriteStringToFileW(File, ActualAccount->BadConnectN.App)) ||
				(Stat = WriteStringToFileW(File, ActualAccount->BadConnectN.AppParam)))
				throw (DWORD)Stat;

			//Let plugin write its own values into file
			if (ActualAccount->Plugin->Fcn != NULL && ActualAccount->Plugin->Fcn->WritePluginOptsFcnPtr != NULL)
				if (Stat = ActualAccount->Plugin->Fcn->WritePluginOptsFcnPtr(File, ActualAccount))
					throw (DWORD)Stat;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountMsgsSO-read wait\n");
#endif
			WaitToReadFcn(ActualAccount->MessagesAccessSO);
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountMsgsSO-read enter\n");
#endif
			if (Stat = WriteMessagesToFile(File, ActualAccount))
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

			if ((!WriteFile(File, (char *)&ActualAccount->LastChecked, sizeof(SYSTEMTIME), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->LastSChecked, sizeof(SYSTEMTIME), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->LastSynchronised, sizeof(SYSTEMTIME), &WrittenBytes, NULL)) ||
				(!WriteFile(File, (char *)&ActualAccount->LastMail, sizeof(SYSTEMTIME), &WrittenBytes, NULL)))
				throw (DWORD)Stat;

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read done\n");
#endif
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
	}
	catch (DWORD ErrorCode)
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WriteAccountsToFile:ActualAccountSO-read done\n");
#endif
		ReadDoneFcn(ActualAccount->AccountAccessSO);
		ReturnValue = ErrorCode;
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"WriteAccountsToFile:AccountBrowserSO-read done\n");
#endif
	SWMRGDoneReading(Plugin->AccountBrowserSO);
	CloseHandle(File);
	return 0;
}

//Writes accounts to file
INT_PTR WriteAccountsToFileSvc(WPARAM wParam, LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin = (HYAMNPROTOPLUGIN)wParam;
	TCHAR* tszFileName = (TCHAR*)lParam;

	EnterCriticalSection(&FileWritingCS);
	HANDLE hFile = CreateFile(tszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		LeaveCriticalSection(&FileWritingCS);
		return EACC_SYSTEM;
	}

	INT_PTR rv = PerformAccountWriting(Plugin, hFile);
	LeaveCriticalSection(&FileWritingCS);

	return rv;
}

INT_PTR FindAccountByNameSvc(WPARAM wParam, LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin = (HYAMNPROTOPLUGIN)wParam;
	char *SearchedAccount = (char *)lParam;
	HACCOUNT Finder;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"FindAccountByName:AccountBrowserSO-read wait\n");
#endif
	SWMRGWaitToRead(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"FindAccountByName:AccountBrowserSO-read enter\n");
#endif
	for (Finder = Plugin->FirstAccount; Finder != NULL; Finder = Finder->Next)
		if ((Finder->Name != NULL) && (0 == mir_strcmp(SearchedAccount, Finder->Name)))
			break;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"FindAccountByName:AccountBrowserSO-read done\n");
#endif
	SWMRGDoneReading(Plugin->AccountBrowserSO);
	return (INT_PTR)Finder;
}

INT_PTR GetNextFreeAccountSvc(WPARAM wParam, LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin = (HYAMNPROTOPLUGIN)wParam;
	HACCOUNT Finder;

	if (Plugin->FirstAccount == NULL)
	{
		Plugin->FirstAccount = (HACCOUNT)CallService(MS_YAMN_CREATEPLUGINACCOUNT, wParam, lParam);
		return (INT_PTR)Plugin->FirstAccount;
	}
	for (Finder = Plugin->FirstAccount; Finder->Next != NULL; Finder = Finder->Next);
	Finder->Next = (HACCOUNT)CallService(MS_YAMN_CREATEPLUGINACCOUNT, wParam, lParam);
	return (INT_PTR)Finder->Next;
}

/*
int FindPluginAccount(WPARAM wParam,LPARAM lParam)
{
HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
HACCOUNT Finder=(HACCOUNT)lParam;

if (Finder=NULL)	Finder=Plugin->FirstAccount;

//	for (;Finder != NULL && Finder->PluginID != Plugin->PluginInfo->PluginID;Finder=(HACCOUNT)Finder->Next);
return (int)Finder;
}
*/
INT_PTR DeleteAccountSvc(WPARAM wParam, LPARAM lParam)
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

	HYAMNPROTOPLUGIN Plugin = (HYAMNPROTOPLUGIN)wParam;
	HACCOUNT Which = (HACCOUNT)lParam;
	HACCOUNT Finder;

	//1. set stop signal 
	StopSignalFcn(Which);
	WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_STOPACCOUNT, (WPARAM)Which, 0);
	if (Plugin->Fcn->StopAccountFcnPtr != NULL)
		Plugin->Fcn->StopAccountFcnPtr(Which);

	//2. wait to get write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write wait\n");
#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write enter\n");
#endif

	//3. remove from queue (chained list)
	if (Plugin->FirstAccount == NULL)
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"DeleteAccount:AccountBrowserSO-write done\n");
#endif
		SWMRGDoneWriting(Plugin->AccountBrowserSO);
		return 0;
	}
	if (Plugin->FirstAccount == Which)
	{
		Finder = Plugin->FirstAccount->Next;
		Plugin->FirstAccount = Finder;
	}
	else
	{
		for (Finder = Plugin->FirstAccount; Which != Finder->Next; Finder = Finder->Next);
		Finder->Next = Finder->Next->Next;
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

	if ((Plugin->Fcn != NULL) && (Plugin->Fcn->WriteAccountsFcnPtr != NULL))
		Plugin->Fcn->WriteAccountsFcnPtr();
	CloseHandle(mir_forkthread(DeleteAccountInBackground, (void*)Which));

	//Now, plugin can consider account as deleted, but plugin really can achieve deleting this account from memory when using
	//event UsingThreads.
	return 1;
}

void __cdecl DeleteAccountInBackground(void *Value)
{
	HACCOUNT Which = (HACCOUNT)Value;
	WaitForSingleObject(Which->UsingThreads->Event, INFINITE);
	CallService(MS_YAMN_DELETEPLUGINACCOUNT, (WPARAM)Which, 0);
}

int StopAccounts(HYAMNPROTOPLUGIN Plugin)
{
	HACCOUNT Finder;

	//1. wait to get write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"StopAccounts:AccountBrowserSO-write wait\n");
#endif
	SWMRGWaitToWrite(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"StopAccounts:AccountBrowserSO-write enter\n");
#endif
	for (Finder = Plugin->FirstAccount; Finder != NULL; Finder = Finder->Next)
	{
		//2. set stop signal 
		StopSignalFcn(Finder);
		WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_STOPACCOUNT, (WPARAM)Finder, 0);
		if (Plugin->Fcn->StopAccountFcnPtr != NULL)
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

int WaitForAllAccounts(HYAMNPROTOPLUGIN Plugin, BOOL GetAccountBrowserAccess)
{
	HACCOUNT Finder;

	if (GetAccountBrowserAccess)
	{
		//1. wait to get write access
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:AccountBrowserSO-write wait\n");
#endif
		SWMRGWaitToWrite(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:AccountBrowserSO-write enter\n");
#endif
	}
	for (Finder = Plugin->FirstAccount; Finder != NULL; Finder = Finder->Next)
	{
		//2. wait for signal that account is not in use
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:waiting for UsingThreadEV %x (account %x)\n",Finder->UsingThreads,Finder);
#endif
		WaitForSingleObject(Finder->UsingThreads->Event, INFINITE);
		SetEvent(Finder->UsingThreads->Event);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"WaitForAllAccounts:UsingThreadEV signaled\n");
#endif
	}
	if (GetAccountBrowserAccess)
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
	SWMRGWaitToWrite(Plugin->AccountBrowserSO, INFINITE);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccounts:AccountBrowserSO-write enter\n");
#endif

	WaitForAllAccounts(Plugin, FALSE);

	for (Finder = Plugin->FirstAccount; Finder != NULL;)
	{
		HACCOUNT Next = Finder->Next;
		DeletePluginAccountSvc((WPARAM)Finder, 0);
		Finder = Next;
	}

	//leave write access
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"DeleteAccounts:AccountBrowserSO-write done\n");
#endif
	SWMRGDoneWriting(Plugin->AccountBrowserSO);

	return 1;
}

void WINAPI GetStatusFcn(HACCOUNT Which, TCHAR *Value)
{
	if (Which == NULL)
		return;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetStatus:AccountStatusCS-cs wait\n");
#endif
	EnterCriticalSection(&AccountStatusCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetStatus:AccountStatusCS-cs enter\n");
#endif
	mir_tstrcpy(Value, Which->Status);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetStatus:AccountStatusCS-cs done\n");
#endif
	LeaveCriticalSection(&AccountStatusCS);
	return;
}

void WINAPI SetStatusFcn(HACCOUNT Which, TCHAR *Value)
{
	if (Which == NULL)
		return;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSetStatus:AccountStatusCS-cs wait\n");
#endif
	EnterCriticalSection(&AccountStatusCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSetStatus:AccountStatusCS-cs enter\n");
#endif
	mir_tstrcpy(Which->Status, Value);
	WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_CHANGESTATUS, (WPARAM)Which, 0);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSetStatus:AccountStatusCS-cs done\n");
#endif
	LeaveCriticalSection(&AccountStatusCS);
}

/*
#ifdef DEBUG_ACCOUNTS
int GetAccounts()
{
HACCOUNT Finder;
int cnt=0;

for (Finder=Account;Finder != NULL;Finder=Finder->Next)
cnt++;
return cnt;
}

void WriteAccounts()
{
HACCOUNT Finder;

for (Finder=Account;Finder != NULL;Finder=Finder->Next)
MessageBoxA(NULL,Finder->Name,"Browsing account",MB_OK);
}
#endif
*/
