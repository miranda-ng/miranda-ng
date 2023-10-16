/*
 * This code implements manipulation with accounts
 * such as reading accounts from file, writing them to file,
 * finding account by name etc.
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

 // Account status CS
 // When we check some account, thread should change status of account to idle, connecting etc.
 // So if we want to read status, we have to successfully write and then read.
static mir_cs csAccountStatusCS;

// File Writing CS
// When 2 threads want to write to file...
static mir_cs csFileWritingCS;

/////////////////////////////////////////////////////////////////////////////////////////
// CAccount class

void CAccount::CheckMail()
{
	// if we want to close miranda, we get event and do not run pop3 checking anymore
	if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
		return;

	mir_cslock lck(PluginRegCS);
	SReadGuard sra(AccountAccessSO, 0);
	if (sra.Succeeded()) {
		if ((Flags & YAMN_ACC_ENA) && Plugin->Fcn->SynchroFcnPtr) {
			TimeLeft = Interval;
			mir_forkThread<CheckParam>(Plugin->Fcn->SynchroFcnPtr, new CheckParam(this, g_plugin.CheckFlags()));
		}
	}
}

void CAccount::RefreshContact()
{
	if (hContact != 0) {
		Contact::Hide(hContact, !(Flags & YAMN_ACC_ENA) && (NewMailN.Flags & YAMN_ACC_CONT));
	}
	else if ((Flags & YAMN_ACC_ENA) && (NewMailN.Flags & YAMN_ACC_CONT)) {
		hContact = db_add_contact();
		Proto_AddToContact(hContact, YAMN_DBMODULE);
		g_plugin.setString(hContact, "Id", Name);
		g_plugin.setString(hContact, "Nick", Name);
		g_plugin.setWord(hContact, "Status", ID_STATUS_ONLINE);
		db_set_s(hContact, "CList", "StatusMsg", Translate("No new mail message"));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

CAccount* CreatePluginAccount(YAMN_PROTOPLUGIN *Plugin)
{
	if (Plugin == nullptr)
		return nullptr;

	CAccount *NewAccount;
	if (Plugin->Fcn->NewAccountFcnPtr != nullptr)
		// Let plugin create its own structure, which can be derived from CAccount structure
		NewAccount = Plugin->Fcn->NewAccountFcnPtr(Plugin);
	else
		// We suggest plugin uses standard CAccount structure, so we create it
		NewAccount = new CAccount();

	// If not created successfully
	if (NewAccount == nullptr)
		return NULL;

	NewAccount->Plugin = Plugin;

	// Init every members of structure, used by YAMN
	InitAccount(NewAccount);
	return NewAccount;
}

void DeletePluginAccount(CAccount *OldAccount)
{
	if (OldAccount->Plugin->Fcn != nullptr) {
		// Deinit every members and allocated fields of structure used by YAMN
		DeInitAccount(OldAccount);
		if (OldAccount->Plugin->Fcn->DeleteAccountFcnPtr != nullptr) {
			// Let plugin delete its own CAccount derived structure
			OldAccount->Plugin->Fcn->DeleteAccountFcnPtr(OldAccount);
			return;
		}
	}

	// consider account as standard YAMN CAccount *, not initialized before and use its own destructor
	delete OldAccount;
}

int InitAccount(CAccount *Which)
{
	// zero memory, where timestamps are stored
	memset(&Which->LastChecked, 0, sizeof(Which->LastChecked));
	memset(&Which->LastSChecked, 0, sizeof(Which->LastSChecked));
	memset(&Which->LastSynchronised, 0, sizeof(Which->LastSynchronised));
	memset(&Which->LastMail, 0, sizeof(Which->LastMail));

	Which->Name = nullptr;
	Which->Mails = nullptr;
	Which->Interval = 0;
	Which->Flags = 0;
	Which->StatusFlags = YAMN_ACC_ST1 + YAMN_ACC_ST7;
	Which->Next = nullptr;

	Which->Server = new CServer();
	Which->AbleToWork = TRUE;

	return 1;
}

void DeInitAccount(CAccount *Which)
{
	// delete YAMN allocated fields
	if (Which->Name != nullptr)
		delete[] Which->Name;
	if (Which->Server != nullptr) {
		if (Which->Server->Name != nullptr)
			delete[] Which->Server->Name;
		if (Which->Server->Login != nullptr)
			delete[] Which->Server->Login;
		if (Which->Server->Passwd != nullptr)
			delete[] Which->Server->Passwd;
		delete[] Which->Server;
	}

	DeleteMessagesToEndFcn(Which, (HYAMNMAIL)Which->Mails);
}

void StopSignalFcn(CAccount *Which)
// set event that we are going to delete account
{
	Which->AbleToWork = FALSE;
	
	// do not use synchronizing objects anymore
	// any access to these objects then ends with WAIT_FAILED
	Which->AccountAccessSO.Stop();
	Which->MessagesAccessSO.Stop();
}

void CodeDecodeString(char *Dest, BOOL Encrypt)
{
	wchar_t Code = STARTCODEPSW;

	if (Dest == nullptr)
		return;

	for (; *Dest != (wchar_t)0; Dest++) {
		if (Encrypt)
			*Dest = *Dest + Code;
		else
			*Dest = *Dest - Code;
		Code += (wchar_t)ADDCODEPSW;
	}
}

static uint32_t PostFileToMemory(HANDLE File, char **MemFile, char **End)
{
	DWORD FileSize, ReadBytes;
	if (!(FileSize = GetFileSize(File, nullptr))) {
		CloseHandle(File);
		return EACC_FILESIZE;
	}

	// allocate space in memory, where we copy the whole file
	if (nullptr == (*MemFile = new char[FileSize])) {
		CloseHandle(File);
		return EACC_ALLOC;
	}

	// copy file to memory
	if (!ReadFile(File, (LPVOID)*MemFile, FileSize, &ReadBytes, nullptr)) {
		CloseHandle(File);
		delete[] * MemFile;
		return EACC_SYSTEM;
	}
	CloseHandle(File);
	*End = *MemFile + FileSize;
	return 0;
}

uint32_t FileToMemory(const wchar_t *FileName, char **MemFile, char **End)
{
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return EACC_SYSTEM;

	return PostFileToMemory(hFile, MemFile, End);
}

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
uint32_t ReadStringFromMemory(char **Parser, wchar_t *End, char **StoreTo, wchar_t *DebugString)
{
	// This is the debug version of ReadStringFromMemory function. This version shows MessageBox where
	// read string is displayed
	wchar_t *Dest, *Finder;
	uint32_t Size;
	wchar_t Debug[65536];

	Finder = *Parser;
	while ((*Finder != (wchar_t)0) && (Finder <= End)) Finder++;
	mir_snwprintf(Debug, L"%s: %s,length is %d, remaining %d chars", DebugString, *Parser, Finder - *Parser, End - Finder);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	if (Finder >= End)
		return EACC_FILECOMPATIBILITY;
	if (Size = Finder - *Parser) {
		if (NULL == (Dest = *StoreTo = new wchar_t[Size + 1]))
			return EACC_ALLOC;
		for (; *Parser <= Finder; (*Parser)++, Dest++)
			*Dest = **Parser;
	}
	else {
		*StoreTo = NULL;
		(*Parser)++;
	}
	return 0;
}
#endif

uint32_t ReadStringFromMemory(char **Parser, char *End, char **StoreTo)
{
	char *Dest, *Finder;
	uint32_t Size;

	Finder = *Parser;
	while ((*Finder != (wchar_t)0) && (Finder <= End)) Finder++;
	if (Finder >= End)
		return EACC_FILECOMPATIBILITY;
	if (Size = Finder - *Parser) {
		if (nullptr == (Dest = *StoreTo = new char[Size + 1]))
			return EACC_ALLOC;
		for (; *Parser <= Finder; (*Parser)++, Dest++)
			*Dest = **Parser;
	}
	else {
		*StoreTo = nullptr;
		(*Parser)++;
	}
	return 0;
}

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
uint32_t ReadStringFromMemoryW(wchar_t **Parser, wchar_t *End, wchar_t **StoreTo, wchar_t *DebugString)
{
	// This is the debug version of ReadStringFromMemoryW function. This version shows MessageBox where
	// read string is displayed
	wchar_t *Dest, *Finder;
	uint32_t Size;
	wchar_t Debug[65536];

	Finder = *Parser;
	while ((*Finder != (wchar_t)0) && (Finder <= (wchar_t *)End)) Finder++;
	mir_snwprintf(Debug, L"%s: %s,length is %d, remaining %d chars", DebugString, *Parser, Finder - *Parser, (wchar_t *)End - Finder);
	MessageBoxW(NULL, Debug, L"debug", MB_OK);
	if (Finder >= (wchar_t *)End)
		return EACC_FILECOMPATIBILITY;
	if (Size = Finder - *Parser) {
		if (NULL == (Dest = *StoreTo = new wchar_t[Size + 1]))
			return EACC_ALLOC;
		for (; *Parser <= Finder; (*Parser)++, Dest++)
			*Dest = **Parser;
	}
	else {
		*StoreTo = NULL;
		(*Parser)++;
	}
	return 0;
}
#endif  // if defined(DEBUG...)

uint32_t ReadStringFromMemoryW(wchar_t **Parser, wchar_t *End, wchar_t **StoreTo)
{
	wchar_t *Dest, *Finder;
	uint32_t Size;

	Finder = *Parser;
	while ((*Finder != (wchar_t)0) && (Finder <= (wchar_t *)End)) Finder++;
	if (Finder >= (wchar_t *)End)
		return EACC_FILECOMPATIBILITY;
	if (Size = Finder - *Parser) {
		if (nullptr == (Dest = *StoreTo = new wchar_t[Size + 1]))
			return EACC_ALLOC;
		for (; *Parser <= Finder; (*Parser)++, Dest++)
			*Dest = **Parser;
	}
	else {
		*StoreTo = nullptr;
		(*Parser)++;
	}
	return 0;
}

static uint32_t ReadNotificationFromMemory(char **Parser, char *End, YAMN_NOTIFICATION *Which)
{
	uint32_t Stat;
	#ifdef DEBUG_FILEREAD
	wchar_t Debug[65536];
	#endif

	Which->Flags = *(uint32_t *)(*Parser);
	(*Parser) += sizeof(uint32_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"NFlags: %04x, remaining %d chars", Which->Flags, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif

	Which->PopupB = *(COLORREF *)(*Parser);
	(*Parser) += sizeof(COLORREF);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"PopupB: %04x, remaining %d chars", Which->PopupB, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif
	Which->PopupT = *(COLORREF *)(*Parser);
	(*Parser) += sizeof(COLORREF);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"PopupT: %04x, remaining %d chars", Which->PopupT, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif
	Which->PopupTime = *(uint32_t *)(*Parser);
	(*Parser) += sizeof(uint32_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"PopupTime: %04x, remaining %d chars", Which->PopupTime, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif

	#ifdef	DEBUG_FILEREAD
	if (Stat = ReadStringFromMemoryW((wchar_t **)Parser, (wchar_t *)End, &Which->App, L"App"))
		#else
	if (Stat = ReadStringFromMemoryW((wchar_t **)Parser, (wchar_t *)End, &Which->App))
		#endif
		return Stat;
	#ifdef	DEBUG_FILEREAD
	if (Stat = ReadStringFromMemoryW((wchar_t **)Parser, (wchar_t *)End, &Which->AppParam, L"AppParam"))
		#else
	if (Stat = ReadStringFromMemoryW((wchar_t **)Parser, (wchar_t *)End, &Which->AppParam))
		#endif
		return Stat;
	return 0;
}

uint32_t ReadMessagesFromMemory(CAccount *Which, char **Parser, char *End)
{
	char *Finder;
	uint32_t Size, Stat;
	HYAMNMAIL ActualMail = nullptr;
	struct CMimeItem *items;
	char *ReadString;

	#ifdef DEBUG_FILEREAD
	MessageBox(NULL, L"going to read messages, if any...", L"debug", MB_OK);
	#endif
	do {
		Finder = *Parser;
		while ((*Finder != (wchar_t)0) && (Finder <= End)) Finder++;
		if (Finder >= End)
			return EACC_FILECOMPATIBILITY;
		if (Size = Finder - *Parser) {
			if (Which->Mails == nullptr)		// First message in queue
			{
				if (nullptr == (Which->Mails = ActualMail = CreateAccountMail(Which)))
					return EACC_ALLOC;
			}
			else {
				if (nullptr == (ActualMail->Next = CreateAccountMail(Which))) {
					return EACC_ALLOC;
				}
				ActualMail = ActualMail->Next;
			}
			items = nullptr;
			#ifdef DEBUG_FILEREADMESSAGES
			if (Stat = ReadStringFromMemory(Parser, End, &ActualMail->ID, L"ID"))
				#else
			if (Stat = ReadStringFromMemory(Parser, End, &ActualMail->ID))
				#endif
				return Stat;
			// 			ActualMail->MailData=new MAILDATA;		 !!! mem leake !!! this is alloc by CreateAccountMail, no need for doubble alloc !!!!

			ActualMail->MailData->Size = *(uint32_t *)(*Parser);
			(*Parser) += sizeof(uint32_t);
			if (*Parser >= End)
				return EACC_FILECOMPATIBILITY;
			ActualMail->Flags = *(uint32_t *)(*Parser);
			(*Parser) += sizeof(uint32_t);
			if (*Parser >= End)
				return EACC_FILECOMPATIBILITY;
			ActualMail->Number = *(uint32_t *)(*Parser);
			(*Parser) += sizeof(uint32_t);
			if (*Parser >= End)
				return EACC_FILECOMPATIBILITY;

			if ((nullptr != Which->Plugin->MailFcn) && (nullptr != Which->Plugin->MailFcn->ReadMailOptsFcnPtr))
				Which->Plugin->MailFcn->ReadMailOptsFcnPtr(ActualMail, Parser, End);	// read plugin mail settings from file

			do {
				#if defined(DEBUG_FILEREADMESSAGES) || defined(DEBUG_FILEREAD)
				if (Stat = ReadStringFromMemory(Parser, End, &ReadString, L"Name"))
					#else
				if (Stat = ReadStringFromMemory(Parser, End, &ReadString))
					#endif
					return Stat;
				if (ReadString == nullptr)
					break;

				#ifdef DEBUG_DECODE
				DebugLog(DecodeFile, "<read name>%s</read name>", ReadString);
				#endif

				if (items == nullptr)
					items = ActualMail->MailData->TranslatedHeader = new CMimeItem();
				else {
					items->Next = new CMimeItem();
					items = items->Next;
				}
				if (items == nullptr)
					return EACC_ALLOC;
				items->name = ReadString;

				#ifdef DEBUG_FILEREADMESSAGES
				if (Stat = ReadStringFromMemory(Parser, End, &ReadString, L"Value"))
					#else
				if (Stat = ReadStringFromMemory(Parser, End, &ReadString))
					#endif
					return Stat;
				items->value = ReadString;
				#ifdef DEBUG_DECODE
				DebugLog(DecodeFile, "<read value>%s</read value>\n", ReadString);
				#endif
			} while (1);
		}
		else
			break;		// no next messages, new account!

	} while (1);
	(*Parser)++;
	return 0;
}

uint32_t ReadAccountFromMemory(CAccount *Which, char **Parser, char *End)
{
	uint32_t Stat;
	#ifdef DEBUG_FILEREAD
	wchar_t Debug[65536];
	#endif
	// Read name of account	

	#ifdef DEBUG_FILEREAD
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Name, L"Name"))
		#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Name))
		#endif
		return Stat;
	if (Which->Name == nullptr)
		return EACC_FILECOMPATIBILITY;

	// Read server parameters
	#ifdef	DEBUG_FILEREAD
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Name, L"Server"))
		#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Name))
		#endif
		return Stat;
	Which->Server->Port = *(uint16_t *)(*Parser);
	(*Parser) += sizeof(uint16_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef	DEBUG_FILEREAD
	mir_snwprintf(Debug, L"Port: %d, remaining %d chars", Which->Server->Port, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif
	#ifdef	DEBUG_FILEREAD
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Login, L"Login"))
		#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Login))
		#endif
		return Stat;
	#ifdef	DEBUG_FILEREAD
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Passwd, L"Password"))
		#else
	if (Stat = ReadStringFromMemory(Parser, End, &Which->Server->Passwd))
		#endif
		return Stat;
	CodeDecodeString(Which->Server->Passwd, FALSE);

	// Read account flags
	Which->Flags = *(uint32_t *)(*Parser);
	(*Parser) += sizeof(uint32_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"Flags: %04x, remaining %d chars", Which->Flags, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif
	Which->StatusFlags = *(uint32_t *)(*Parser);
	(*Parser) += sizeof(uint32_t);
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"STFlags: %04x, remaining %d chars", Which->StatusFlags, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif
	Which->PluginFlags = *(uint32_t *)(*Parser);
	(*Parser) += sizeof(uint32_t);
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"PFlags: %04x, remaining %d chars", Which->PluginFlags, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif

	// Read account miscellaneous parameters
	Which->Interval = *(uint16_t *)(*Parser);
	Which->TimeLeft = Which->Interval;		// check on loading
	(*Parser) += sizeof(uint16_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"Interval: %d, remaining %d chars", Which->Interval, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif

	// Read notification parameters
	if (Stat = ReadNotificationFromMemory(Parser, End, &Which->NewMailN))
		return Stat;
	if (Stat = ReadNotificationFromMemory(Parser, End, &Which->NoNewMailN))
		return Stat;
	if (Stat = ReadNotificationFromMemory(Parser, End, &Which->BadConnectN))
		return Stat;

	// Let plugin read its own data stored in file
	if (Which->Plugin->Fcn != nullptr && Which->Plugin->Fcn->ReadPluginOptsFcnPtr != nullptr)
		if (Stat = Which->Plugin->Fcn->ReadPluginOptsFcnPtr(Which, Parser, End))
			return Stat;

	// Read mails
	{
		SWriteGuard sw(Which->MessagesAccessSO);
		if (Stat = ReadMessagesFromMemory(Which, Parser, End))
			return Stat;
	}

	// Read timestamps
	Which->LastChecked = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;

	Which->LastSChecked = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;

	Which->LastSynchronised = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;

	Which->LastMail = *(SYSTEMTIME *)(*Parser);
	(*Parser) += sizeof(SYSTEMTIME);
	if (*Parser > End)		// WARNING! There's only > at the end of testing
		return EACC_FILECOMPATIBILITY;

	if (*Parser == End)
		return EACC_ENDOFFILE;
	return 0;
}

static uint32_t PerformAccountReading(YAMN_PROTOPLUGIN *Plugin, char *MemFile, char *End)
{
	// Retrieve info for account from memory
	char *Parser;
	uint32_t Ver, Stat;

	CAccount *ActualAccount, *FirstAllocatedAccount;

	Ver = *(uint32_t *)MemFile;
	if (Ver > YAMN_ACCOUNTFILEVERSION) {
		delete[] MemFile;
		return EACC_FILEVERSION;
	}
	Parser = MemFile + sizeof(Ver);
	{
		SWriteGuard swb(Plugin->AccountBrowserSO);
		if (nullptr == (ActualAccount = GetNextFreeAccount(Plugin))) {
			delete[] MemFile;
			return EACC_ALLOC;
		}
	}

	FirstAllocatedAccount = ActualAccount;

	do {
		CAccount *Temp;
		{
			SWriteGuard swb(Plugin->AccountBrowserSO);
			Stat = ReadAccountFromMemory(ActualAccount, &Parser, End);

			if (ActualAccount->StatusFlags & (YAMN_ACC_STARTA | YAMN_ACC_STARTS))
				ActualAccount->TimeLeft = 1;		// check on loading

			if (Stat && (Stat != EACC_ENDOFFILE)) {
				for (ActualAccount = FirstAllocatedAccount; ActualAccount != nullptr; ActualAccount = Temp) {
					Temp = ActualAccount->Next;
					delete ActualAccount;
				}
				delete[] MemFile;
				if (Plugin->FirstAccount == FirstAllocatedAccount)
					Plugin->FirstAccount = nullptr;

				return (INT_PTR)Stat;
			}
		}

		if ((Stat != EACC_ENDOFFILE) && (nullptr == (ActualAccount = GetNextFreeAccount(Plugin)))) {
			for (ActualAccount = FirstAllocatedAccount; ActualAccount != nullptr; ActualAccount = Temp) {
				Temp = ActualAccount->Next;
				delete ActualAccount;
			}
			delete[] MemFile;
			if (Plugin->FirstAccount == FirstAllocatedAccount)
				Plugin->FirstAccount = nullptr;

			return EACC_ALLOC;
		}
	} while (Stat != EACC_ENDOFFILE);

	delete[] MemFile;
	return 0;
}

// Add accounts from file to memory
uint32_t AddAccountsFromFile(YAMN_PROTOPLUGIN *Plugin, const wchar_t *pwszFilename)
{
	char *MemFile, *End;
	uint32_t Stat = FileToMemory(pwszFilename, &MemFile, &End);
	if (Stat != NO_ERROR)
		return Stat;

	return PerformAccountReading(Plugin, MemFile, End);
}

uint32_t WriteStringToFile(HANDLE File, char *Source)
{
	DWORD Length, WrittenBytes;
	char null = 0;

	if ((Source == nullptr) || !(Length = (uint32_t)mir_strlen(Source))) {
		if (!WriteFile(File, &null, 1, &WrittenBytes, nullptr)) {
			CloseHandle(File);
			return EACC_SYSTEM;
		}
	}
	else if (!WriteFile(File, Source, (Length + 1), &WrittenBytes, nullptr)) {
		CloseHandle(File);
		return EACC_SYSTEM;
	}
	return 0;
}

uint32_t WriteStringToFileW(HANDLE File, wchar_t *Source)
{
	DWORD Length, WrittenBytes;
	wchar_t null = (wchar_t)0;

	if ((Source == nullptr) || !(Length = (uint32_t)mir_wstrlen(Source))) {
		if (!WriteFile(File, &null, sizeof(wchar_t), &WrittenBytes, nullptr)) {
			CloseHandle(File);
			return EACC_SYSTEM;
		}
	}
	else if (!WriteFile(File, Source, (Length + 1) * sizeof(wchar_t), &WrittenBytes, nullptr))
		return EACC_SYSTEM;
	return 0;
}

DWORD WriteMessagesToFile(HANDLE File, CAccount *Which)
{
	DWORD WrittenBytes, Stat;
	HYAMNMAIL ActualMail = (HYAMNMAIL)Which->Mails;
	struct CMimeItem *items;

	while (ActualMail != nullptr) {
		if (Stat = WriteStringToFile(File, ActualMail->ID))
			return Stat;

		if (!WriteFile(File, (char *)&ActualMail->MailData->Size, sizeof(ActualMail->MailData->Size), &WrittenBytes, nullptr) ||
			!WriteFile(File, (char *)&ActualMail->Flags, sizeof(ActualMail->Flags), &WrittenBytes, nullptr) ||
			!WriteFile(File, (char *)&ActualMail->Number, sizeof(ActualMail->Number), &WrittenBytes, nullptr))
			return EACC_SYSTEM;
		if ((nullptr != Which->Plugin->MailFcn) && (nullptr != Which->Plugin->MailFcn->WriteMailOptsFcnPtr))
			Which->Plugin->MailFcn->WriteMailOptsFcnPtr(File, ActualMail);	// write plugin mail options to file
		for (items = ActualMail->MailData->TranslatedHeader; items != nullptr; items = items->Next) {
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

static INT_PTR PerformAccountWriting(YAMN_PROTOPLUGIN *Plugin, HANDLE File)
{
	DWORD WrittenBytes, Stat;
	CAccount *ActualAccount;
	uint32_t Ver = YAMN_ACCOUNTFILEVERSION;
	BOOL Writed = FALSE;
	uint32_t ReturnValue = 0;

	SReadGuard srb(Plugin->AccountBrowserSO);

	try {
		for (ActualAccount = Plugin->FirstAccount; ActualAccount != nullptr; ActualAccount = ActualAccount->Next) {
			SReadGuard sra(ActualAccount->AccountAccessSO);
			if (sra == WAIT_FINISH) { // account is about to delete
				ActualAccount = ActualAccount->Next;
				continue;
			}

			if (sra == WAIT_FAILED)		// account is deleted
				break;

			if ((ActualAccount->Name == nullptr) || (*ActualAccount->Name == (wchar_t)0))
				continue;

			if (!Writed && !WriteFile(File, &Ver, sizeof(Ver), &WrittenBytes, nullptr))
				throw (uint32_t)EACC_SYSTEM;

			Writed = TRUE;

			if (Stat = WriteStringToFile(File, ActualAccount->Name))
				throw (uint32_t)Stat;

			if (Stat = WriteStringToFile(File, ActualAccount->Server->Name))
				throw (uint32_t)Stat;

			if (!WriteFile(File, (char *)&ActualAccount->Server->Port, 2, &WrittenBytes, nullptr))
				throw (uint32_t)EACC_SYSTEM;

			if ((Stat = WriteStringToFile(File, ActualAccount->Server->Login)))
				throw (uint32_t)Stat;

			CodeDecodeString(ActualAccount->Server->Passwd, TRUE);

			if (Stat = WriteStringToFile(File, ActualAccount->Server->Passwd)) {
				CodeDecodeString(ActualAccount->Server->Passwd, FALSE);
				throw (uint32_t)Stat;
			}
			CodeDecodeString(ActualAccount->Server->Passwd, FALSE);

			if ((!WriteFile(File, (char *)&ActualAccount->Flags, sizeof(uint32_t), &WrittenBytes, nullptr) ||
				(!WriteFile(File, (char *)&ActualAccount->StatusFlags, sizeof(uint32_t), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->PluginFlags, sizeof(uint32_t), &WrittenBytes, nullptr))))
				throw (uint32_t)EACC_SYSTEM;

			if (!WriteFile(File, (char *)&ActualAccount->Interval, sizeof(uint16_t), &WrittenBytes, nullptr))
				throw (uint32_t)EACC_SYSTEM;

			if ((!WriteFile(File, (char *)&ActualAccount->NewMailN.Flags, sizeof(uint32_t), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->NewMailN.PopupB, sizeof(COLORREF), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->NewMailN.PopupT, sizeof(COLORREF), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->NewMailN.PopupTime, sizeof(uint32_t), &WrittenBytes, nullptr)))
				throw (uint32_t)EACC_SYSTEM;

			if ((Stat = WriteStringToFileW(File, ActualAccount->NewMailN.App)) ||
				(Stat = WriteStringToFileW(File, ActualAccount->NewMailN.AppParam)))
				throw (uint32_t)Stat;

			if ((!WriteFile(File, (char *)&ActualAccount->NoNewMailN.Flags, sizeof(uint32_t), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->NoNewMailN.PopupB, sizeof(COLORREF), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->NoNewMailN.PopupT, sizeof(COLORREF), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->NoNewMailN.PopupTime, sizeof(uint32_t), &WrittenBytes, nullptr)))
				throw (uint32_t)EACC_SYSTEM;

			if ((Stat = WriteStringToFileW(File, ActualAccount->NoNewMailN.App)) ||
				(Stat = WriteStringToFileW(File, ActualAccount->NoNewMailN.AppParam)))
				throw (uint32_t)Stat;

			if ((!WriteFile(File, (char *)&ActualAccount->BadConnectN.Flags, sizeof(uint32_t), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->BadConnectN.PopupB, sizeof(COLORREF), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->BadConnectN.PopupT, sizeof(COLORREF), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->BadConnectN.PopupTime, sizeof(uint32_t), &WrittenBytes, nullptr)))
				throw (uint32_t)EACC_SYSTEM;

			if ((Stat = WriteStringToFileW(File, ActualAccount->BadConnectN.App)) ||
				(Stat = WriteStringToFileW(File, ActualAccount->BadConnectN.AppParam)))
				throw (uint32_t)Stat;

			// Let plugin write its own values into file
			if (ActualAccount->Plugin->Fcn != nullptr && ActualAccount->Plugin->Fcn->WritePluginOptsFcnPtr != nullptr)
				if (Stat = ActualAccount->Plugin->Fcn->WritePluginOptsFcnPtr(File, ActualAccount))
					throw (uint32_t)Stat;

			if (Stat = WriteMessagesToFile(File, ActualAccount))
				throw (uint32_t)Stat;

			if ((!WriteFile(File, (char *)&ActualAccount->LastChecked, sizeof(SYSTEMTIME), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->LastSChecked, sizeof(SYSTEMTIME), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->LastSynchronised, sizeof(SYSTEMTIME), &WrittenBytes, nullptr)) ||
				(!WriteFile(File, (char *)&ActualAccount->LastMail, sizeof(SYSTEMTIME), &WrittenBytes, nullptr)))
				throw (uint32_t)Stat;
		}
	}
	catch (uint32_t ErrorCode) {
		ReturnValue = ErrorCode;
	}

	CloseHandle(File);
	return 0;
}

// Writes accounts to file
uint32_t WriteAccountsToFile(YAMN_PROTOPLUGIN *Plugin, const wchar_t *pwszFilename)
{
	mir_cslock lck(csFileWritingCS);
	HANDLE hFile = CreateFile(pwszFilename, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return EACC_SYSTEM;

	return PerformAccountWriting(Plugin, hFile);
}

CAccount* FindAccountByContact(YAMN_PROTOPLUGIN *Plugin, MCONTACT hContact)
{
	ptrA szId(g_plugin.getStringA(hContact, "Id"));
	if (!mir_strlen(szId))
		return nullptr;

	return FindAccountByName(Plugin, szId);
}

CAccount* FindAccountByName(YAMN_PROTOPLUGIN *Plugin, const char *SearchedAccount)
{
	SReadGuard srb(Plugin->AccountBrowserSO);

	for (auto *Finder = Plugin->FirstAccount; Finder != nullptr; Finder = Finder->Next)
		if ((Finder->Name != nullptr) && (0 == mir_strcmp(SearchedAccount, Finder->Name)))
			return Finder;

	return nullptr;
}

CAccount* GetNextFreeAccount(YAMN_PROTOPLUGIN *Plugin)
{
	if (Plugin->FirstAccount == nullptr) {
		Plugin->FirstAccount = CreatePluginAccount(Plugin);
		return Plugin->FirstAccount;
	}
	
	CAccount *Finder;
	for (Finder = Plugin->FirstAccount; Finder->Next != nullptr; Finder = Finder->Next);
	Finder->Next = CreatePluginAccount(Plugin);
	return Finder->Next;
}

// Deleting account works on these steps:
// 1. set signal that account should stop activity (set event)
// 	setting this event we achieve, that any access to account is failed,
// 	so threads do not start any work with accounts (better saying threads of plugins should not start)
// 2. wait to get write access to chained list of accounts
// 3. we can write to chained list, so we change chain not to show to actual account
// 	now, any thread browsing list of accounts does not browse through actual account
// 	actual account seems to be hidden (it exists, but it is not in accounts chained list (chained list=queue))
// Now, we should delete account from memory, BUT!!!
// 	Any thread can still be waked up and start asking account synchronizing object
// 	If account is deleted, asking about access to read account can throw memory exception (reading for
// 	a synchronizing object from memory, that was deleted)
// So, we cannot now delete account. We have to wait until we are sure no thread will be using account anymore
// 	(or to the end of Miranda, but problem is in allocated memory- it is allocated and Miranda is SMALLER, faster, easier, isn't it?)
// 	This deleting is achieved in 2 ways:
// 	We have event in UsingThreads synchronization objects. This event signals that no thread will use actual account
// 	1. Any thread using account first increment UsingThread, so we know that account is used
// 	2. If thread is about to close, it should decrement UsingThread
// 	3. If thread creates another thread, that will use account, caller has to wait until the new thread does not
// 		increment UsingThreads (imagine that caller ends before the new thread set it: if no other thread is using
// 		account, account is automaticaly (decreasing UsingThreads) signaled as "not used" and we delete it. But then
// 		new thread is going to read account...).
// 4. wait until UsingThread Event is signaled
// 5. delete account from memory

int DeleteAccount(YAMN_PROTOPLUGIN *Plugin, CAccount *Which)
{
	// 1. set stop signal 
	StopSignalFcn(Which);
	WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_STOPACCOUNT, (WPARAM)Which, 0);
	if (Plugin->Fcn->StopAccountFcnPtr != nullptr)
		Plugin->Fcn->StopAccountFcnPtr(Which);

	{	// 2. wait to get write access
		SWriteGuard swb(Plugin->AccountBrowserSO);

		// 3. remove from queue (chained list)
		if (Plugin->FirstAccount == nullptr)
			return 0;

		if (Plugin->FirstAccount == Which) {
			Plugin->FirstAccount = Plugin->FirstAccount->Next;
		}
		else {
			CAccount *Finder;
			for (Finder = Plugin->FirstAccount; Which != Finder->Next; Finder = Finder->Next);
			Finder->Next = Finder->Next->Next;
		}
	}

	// 4. wait while event "UsingThread" is not signaled
	// 	And what to do, if this event will be signaled in 1 hour? (Although it's paranoia, because we have sent "delete signal", so
	// 	other threads do not start any new work with actual account) We will wait in blocked state?
	// 	No, of course not. We will create new thread, that will wait and additionally remove our thread in background.
	// 5. So, the last point (deleting from memory) is performed in new DeleteAccountInBackground thread

	if ((Plugin->Fcn != nullptr) && (Plugin->Fcn->WriteAccountsFcnPtr != nullptr))
		Plugin->Fcn->WriteAccountsFcnPtr();
	mir_forkthread(DeleteAccountInBackground, Which);

	// Now, plugin can consider account as deleted, but plugin really can achieve deleting this account from memory when using
	// event UsingThreads.
	return 1;
}

void __cdecl DeleteAccountInBackground(void *Value)
{
	CAccount *Which = (CAccount *)Value;
	WaitForSingleObject(Which->UsingThreads.GetEvent(), INFINITE);
	DeletePluginAccount(Which);
}

int StopAccounts(YAMN_PROTOPLUGIN *Plugin)
{
	CAccount *Finder;

	// 1. wait to get write access
	SWriteGuard swb(Plugin->AccountBrowserSO);

	for (Finder = Plugin->FirstAccount; Finder != nullptr; Finder = Finder->Next) {
		// 2. set stop signal 
		StopSignalFcn(Finder);
		WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_STOPACCOUNT, (WPARAM)Finder, 0);
		if (Plugin->Fcn->StopAccountFcnPtr != nullptr)
			Plugin->Fcn->StopAccountFcnPtr(Finder);
	}

	// Now, account is stopped. It can be removed from memory...
	return 1;
}

int WaitForAllAccounts(YAMN_PROTOPLUGIN *Plugin, BOOL GetAccountBrowserAccess)
{
	if (GetAccountBrowserAccess) {
		// 1. wait to get write access
		Plugin->AccountBrowserSO.WaitToWrite();
	}
	for (CAccount *Finder = Plugin->FirstAccount; Finder != nullptr; Finder = Finder->Next) {
		// 2. wait for signal that account is not in use
		WaitForSingleObject(Finder->UsingThreads.GetEvent(), INFINITE);
		SetEvent(Finder->UsingThreads.GetEvent());
	}
	if (GetAccountBrowserAccess) {
		// leave write access
		Plugin->AccountBrowserSO.DoneWriting();
	}

	return 1;
}

int DeleteAccounts(YAMN_PROTOPLUGIN *Plugin)
{
	SWriteGuard swb(Plugin->AccountBrowserSO);

	WaitForAllAccounts(Plugin, FALSE);

	for (CAccount *Finder = Plugin->FirstAccount; Finder != nullptr;) {
		CAccount *Next = Finder->Next;
		DeletePluginAccount(Finder);
		Finder = Next;
	}
	return 1;
}

void GetStatusFcn(CAccount *Which, wchar_t *Value)
{
	if (Which == nullptr)
		return;

	mir_cslock lck(csAccountStatusCS);
	mir_wstrcpy(Value, Which->Status);
}

void SetStatusFcn(CAccount *Which, wchar_t *Value)
{
	if (Which != nullptr) {
		mir_cslock lck(csAccountStatusCS);
		mir_wstrcpy(Which->Status, Value);
	}

	WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_CHANGESTATUS, (WPARAM)Which, 0);
}
