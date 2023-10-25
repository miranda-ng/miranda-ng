/*
 * This code implements POP3 server checking for new mail and so on.
 * There's function SynchroPOP3 in this file- for checking and synchronising POP3 account
 * and DeleteMailsPOP3- for deleting mails from POP3 server
 *
 * Note this file acts as main file for internal plugin.
 *
 * (c) majvan 2002-2004
 * 18/08
 */


#include "../../stdafx.h"

#define ERRORSTR_MAXLEN	1024	// in wide-chars

HANDLE hNetLib = nullptr;
SCOUNTER CPOP3Account::AccountWriterSO;

// Creates new CPOP3Account structure
CAccount *MIR_CDECL CreatePOP3Account(YAMN_PROTOPLUGIN *Plugin);

// Deletes CPOP3Account structure
void MIR_CDECL DeletePOP3Account(CAccount *Which);

// Sets stop flag to account
void MIR_CDECL StopPOP3Account(CAccount *Which);

// Function registers standard functions for YAMN
int RegisterPOP3Plugin(WPARAM, LPARAM);

// Unloads all variables created on heap (delete[])
DWORD MIR_CDECL UnLoadPOP3(void *);

// Function stores plugin's data for account to file
DWORD MIR_CDECL WritePOP3Options(HANDLE, CAccount *);

// Function reads plugin's data for account from file
DWORD MIR_CDECL ReadPOP3Options(CAccount *, char **, char *);

// Creates new mail for an account
YAMNMAIL *MIR_CDECL CreatePOP3Mail(CAccount *Account);

// Function does all needed work when connection failed or any error occured
// Creates structure containing error code, closes internet session, runs "bad connect" function
static void PostErrorProc(CPOP3Account *ActualAccount, void *ParamToBadConnect, uint32_t POP3PluginParam, BOOL UseSSL);

// Checks POP3 account and stores all info to account. It deletes old mails=> synchro
// WhichTemp- pointer to strucure containing needed information
void MIR_CDECL SynchroPOP3(CheckParam *WhichTemp);

// Deletes mails from POP3 server
// WhichTemp- structure containing needed information (queued messages to delete)
// Function deletes from memory queue in WhichTemp structure
void __cdecl DeleteMailsPOP3(void *param);

// Function makes readable message about error. It sends it back to YAMN, so YAMN then
// can show it to the message window
wchar_t *MIR_CDECL GetErrorString(DWORD Code);

// Function deletes string allocated in GetErrorString
void MIR_CDECL DeleteErrorString(LPVOID String);

// Extracts info from result of POP3's STAT command
// stream- source string
// len- length of source string
// mboxsize- adreess to integer, that receives size of mailbox
// mails- adreess to integer, that receives number of mails
void ExtractStat(char *stream, int *mboxsize, int *mails);

// Extracts mail ID on mailbox
// stream- source string
// len- length of source string
// queue- address of first message, where first ID will be stored
void ExtractUIDL(char *stream, int len, YAMNMAIL *queue);

// Extracts mail size on mailbox
// stream- source string
// len- length of source string
// queue- address of first message, where size of message #1 will be stored
void ExtractList(char *stream, int len, YAMNMAIL *queue);

void ExtractMail(char *stream, int len, YAMNMAIL *queue);

YAMN_PROTOIMPORTFCN POP3ProtocolFunctions =
{
	CreatePOP3Account,
	DeletePOP3Account,
	StopPOP3Account,
	WritePOP3Options,
	ReadPOP3Options,
	SynchroPOP3,
	SynchroPOP3,
	SynchroPOP3,
	DeleteMailsPOP3,
	GetErrorString,
	nullptr,
	DeleteErrorString,
	WritePOP3Accounts,
	nullptr,
	UnLoadPOP3,
};

YAMN_MAILIMPORTFCN POP3MailFunctions =
{
	CreatePOP3Mail,
	nullptr,
	nullptr,
	nullptr,
};

YAMN_PROTOPLUGIN *POP3Plugin = nullptr;

YAMN_PROTOREGISTRATION POP3ProtocolRegistration =
{
	"POP3 protocol (internal)",
	__VERSION_STRING_DOTS,
	__COPYRIGHT,
	__DESCRIPTION,
	__AUTHORWEB,
};

static CMStringW wszFileName;

HANDLE RegisterNLClient(char *name);

// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------

CPOP3Account::CPOP3Account()
{
	// NOTE! This constructor constructs CAccount structure. If your plugin is not internal,
	// you will need these constructors. All you need is in Account.cpp. Just copy to your source code
	// constructor and destructor of CAccount.
	UseInternetFree = CreateEvent(nullptr, FALSE, TRUE, nullptr);
	AbilityFlags = YAMN_ACC_BROWSE | YAMN_ACC_POPUP;

	SetStatusFcn(this, TranslateT("Disconnected"));
}

CPOP3Account::~CPOP3Account()
{
	CloseHandle(UseInternetFree);
}

CAccount *MIR_CDECL CreatePOP3Account(YAMN_PROTOPLUGIN *)
{
	// First, we should check whether CAccountVersion matches.
	// But this is internal plugin, so YAMN's CAccount structure and our CAccount structure are
	// the same, so we do not need to test version. Otherwise, if CAccount version does not match
	// in your plugin, you should return NULL, like this:
	// 	if (CAccountVersion != YAMN_ACCOUNTVERSION) return NULL;

	// Now it is needed to construct our POP3 account and return its handle
	return new CPOP3Account();
}

void MIR_CDECL DeletePOP3Account(CAccount *Which)
{
	delete (CPOP3Account *)Which;
}

void MIR_CDECL StopPOP3Account(CAccount *Which)
{
	((CPOP3Account *)Which)->Client.Stopped = TRUE;
	if (((CPOP3Account *)Which)->Client.NetClient != nullptr)			// we should inform also network client. Usefull only when network client implements this feature
		((CPOP3Account *)Which)->Client.NetClient->Stopped = TRUE;
}

// This function is like main function for POP3 internal protocol
int RegisterPOP3Plugin(WPARAM, LPARAM)
{
	// Register new pop3 user in netlib
	if (nullptr == (hNetLib = RegisterNLClient("YAMN (POP3)"))) {
		UnLoadPOP3(nullptr);
		return 0;
	}

	// First, we register this plugin
	// it is quite impossible this function returns zero (failure) as YAMN and internal plugin structre versions are the same
	POP3ProtocolRegistration.Name = Translate("POP3 protocol (internal)");
	POP3ProtocolRegistration.Description = Translate(__DESCRIPTION);
	if (nullptr == (POP3Plugin = RegisterProtocolPlugin(&POP3ProtocolRegistration)))
		return 0;

	// Next we set our imported functions for YAMN
	if (!SetProtocolPluginFcnImportFcn(POP3Plugin, &POP3ProtocolFunctions, &POP3MailFunctions))
		return 0;

	// Then, we read all mails for accounts.
	// You must first register account, before using this function as YAMN must use CreatePOP3Account function to add new accounts
	// But if CreatePOP3Account is not implemented (equals to NULL), YAMN creates account as YAMN's standard CAccount *
	wszFileName = GetFileName(L"pop3");

	switch (AddAccountsFromFile(POP3Plugin, wszFileName)) {
	case EACC_FILEVERSION:
		MessageBox(nullptr, TranslateT("Found new version of account book, not compatible with this version of YAMN."), TranslateT("YAMN (internal POP3) read error"), MB_OK);
		wszFileName.Empty();
		return 0;
	case EACC_FILECOMPATIBILITY:
		MessageBox(nullptr, TranslateT("Error reading account file. Account file corrupted."), TranslateT("YAMN (internal POP3) read error"), MB_OK);
		wszFileName.Empty();
		return 0;
	case EACC_ALLOC:
		MessageBox(nullptr, TranslateT("Memory allocation error while data reading"), TranslateT("YAMN (internal POP3) read error"), MB_OK);
		wszFileName.Empty();
		return 0;
	case EACC_SYSTEM:
		if (ERROR_FILE_NOT_FOUND != GetLastError()) {
			wchar_t temp[1024] = { 0 };
			mir_snwprintf(temp, L"%s\n%s", TranslateT("Reading file error. File already in use?"), wszFileName.c_str());
			MessageBox(nullptr, temp, TranslateT("YAMN (internal POP3) read error"), MB_OK);
			wszFileName.Empty();
			return 0;
		}
		break;
	}

	for (CAccount *pAcc = POP3Plugin->FirstAccount; pAcc; pAcc = pAcc->Next) {
		pAcc->hContact = 0;
		for (auto &hContact : Contacts(YAMN_DBMODULE)) {
			if (g_plugin.getMStringA(hContact, "Id") == pAcc->Name) {
				pAcc->hContact = hContact;
				break;
			}
		}

		pAcc->RefreshContact();
	}

	return 0;
}

DWORD MIR_CDECL UnLoadPOP3(void *)
{
	Netlib_CloseHandle(hNetLib); hNetLib = nullptr;
	return 1;
}

// Function writes POP3 accounts using YAMN exported functions
DWORD MIR_CDECL WritePOP3Accounts()
{
	uint32_t ReturnValue = WriteAccountsToFile(POP3Plugin, wszFileName);
	if (ReturnValue == EACC_SYSTEM) {
		wchar_t temp[1024] = { 0 };
		mir_snwprintf(temp, L"%s\n%s", TranslateT("Error while copying data to disk occurred. Is file in use?"), wszFileName.c_str());
		MessageBox(nullptr, temp, TranslateT("POP3 plugin - write file error"), MB_OK);
	}

	return ReturnValue;
}

DWORD MIR_CDECL WritePOP3Options(HANDLE File, CAccount *Which)
{
	DWORD WrittenBytes;
	uint32_t Ver = POP3_FILEVERSION;

	if ((!WriteFile(File, (char *)&Ver, sizeof(uint32_t), &WrittenBytes, nullptr)) ||
		(!WriteFile(File, (char *)&((CPOP3Account *)Which)->CP, sizeof(uint16_t), &WrittenBytes, nullptr)))
		return EACC_SYSTEM;
	return 0;
}

DWORD MIR_CDECL ReadPOP3Options(CAccount *Which, char **Parser, char *End)
{
	uint32_t Ver;
	#ifdef DEBUG_FILEREAD
	wchar_t Debug[256];
	#endif
	Ver = *(uint32_t *)(*Parser);
	(*Parser) += sizeof(uint32_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	if (Ver != POP3_FILEVERSION)
		return EACC_FILECOMPATIBILITY;

	((CPOP3Account *)Which)->CP = *(uint16_t *)(*Parser);
	(*Parser) += sizeof(uint16_t);
	if (*Parser >= End)
		return EACC_FILECOMPATIBILITY;
	#ifdef DEBUG_FILEREAD
	mir_snwprintf(Debug, L"CodePage: %d, remaining %d chars", ((CPOP3Account *)Which)->CP, End - *Parser);
	MessageBox(NULL, Debug, L"debug", MB_OK);
	#endif
	return 0;
}

YAMNMAIL *MIR_CDECL CreatePOP3Mail(CAccount *Account)
{
	YAMNMAIL *NewMail;
	// First, we should check whether MAILDATA matches.
	// But this is internal plugin, so YAMN's MAILDATA structure and our MAILDATA structure are
	// the same, so we do not need to test version. Otherwise, if MAILDATA version does not match
	// in your plugin, you should return NULL, like this:
	// 	if (MailDataVersion != YAMN_MAILDATAVERSION) return NULL;

	// Now it is needed to construct our POP3 account and return its handle
	if (nullptr == (NewMail = new YAMNMAIL))
		return nullptr;

	if (nullptr == (NewMail->MailData = new CMailData())) {
		delete NewMail;
		return nullptr;
	}
	NewMail->MailData->CP = ((CPOP3Account *)Account)->CP;
	return (YAMNMAIL *)NewMail;
}

static void SetContactStatus(CAccount *account, int status)
{
	if (account->NewMailN.Flags & YAMN_ACC_CONT)
		g_plugin.setWord(account->hContact, "Status", status);
}

static void PostErrorProc(CPOP3Account *ActualAccount, void *ParamToBadConnection, uint32_t POP3PluginParam, BOOL UseSSL)
{
	// We create new structure, that we pass to bad connection dialog procedure. This procedure next calls YAMN imported fuction
	// from POP3 protocol to determine the description of error. We can describe error from our error code structure, because later,
	// when YAMN calls our function, it passes us our error code. This is pointer to structure for POP3 protocol in fact. 
	POP3_ERRORCODE *ErrorCode = new POP3_ERRORCODE();
	ErrorCode->SSL = UseSSL;
	ErrorCode->AppError = ActualAccount->SystemError;
	ErrorCode->POP3Error = ActualAccount->Client.POP3Error;
	ErrorCode->NetError = ActualAccount->Client.NetClient->NetworkError;
	ErrorCode->SystemError = ActualAccount->Client.NetClient->SystemError;

	// if it was normal YAMN call (force check or so on)
	if (POP3PluginParam == 0) {
		try {
			char *DataRX = ActualAccount->Client.Quit();
			if (DataRX != nullptr)
				free(DataRX);
		}
		catch (...) {
		}
		// We always close connection if error occured
		try {
			ActualAccount->Client.NetClient->Disconnect();
		}
		catch (...) {
		}

		SetStatusFcn(ActualAccount, TranslateT("Disconnected"));
	}

	if ((ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG) || (ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO) || (ActualAccount->BadConnectN.Flags & YAMN_ACC_POP))
		RunBadConnection(ActualAccount, (UINT_PTR)ErrorCode, ParamToBadConnection);

	// if it was normal YAMN call
	if (POP3PluginParam == 0)
		SetEvent(ActualAccount->UseInternetFree);
}

// Checks POP3 account and synchronizes it
void MIR_CDECL SynchroPOP3(CheckParam *WhichTemp)
{
	CPop3Client *MyClient;
	YAMNMAIL *NewMails = nullptr, *MsgQueuePtr = nullptr;
	char *DataRX = nullptr;
	int mboxsize, msgs, i;
	SYSTEMTIME now;
	BOOL UsingInternet = FALSE;

	ptrA ServerName, ServerLogin, ServerPasswd;
	uint32_t ServerPort, Flags, NFlags, NNFlags;

	auto *ActualAccount = (CPOP3Account *)WhichTemp->AccountParam;
	bool bForceCheck = WhichTemp->bParam;
	delete WhichTemp;

	SCGuard sc(ActualAccount->UsingThreads);
	{
		SReadGuard sra(ActualAccount->AccountAccessSO);
		if (!sra.Succeeded())
			return;

		MyClient = &ActualAccount->Client;
		// Now, copy all needed information about account to local variables, so ActualAccount is not blocked in read mode during all connection process, which can last for several minutes.
		ServerName = mir_strdup(ActualAccount->Server->Name);
		ServerPort = ActualAccount->Server->Port;
		Flags = ActualAccount->Flags;
		ServerLogin = mir_strdup(ActualAccount->Server->Login);
		ServerPasswd = mir_strdup(ActualAccount->Server->Passwd);
		NFlags = ActualAccount->NewMailN.Flags;
		NNFlags = ActualAccount->NoNewMailN.Flags;
	}
	{
		SCGuard scq(ActualAccount->InternetQueries);
		WaitForSingleObject(ActualAccount->UseInternetFree, INFINITE);
	}

	// OK, we enter the "use internet" section. But after we start communication, we can test if we did not enter the "use internet" section only for the reason,
	// that previous thread release the internet section because this account has stop signal (we stop account and there are 2 threads: one communicating,
	// the second one waiting for network access- the first one ends because we want to stop account, this one is released, but should be stopped as well).
	if (!ActualAccount->AbleToWork) {
		SetEvent(ActualAccount->UseInternetFree);
		return;
	}
	UsingInternet = TRUE;

	GetLocalTime(&now);
	ActualAccount->SystemError = 0; // now we can use internet for this socket. First, clear errorcode.
	try {
		SetContactStatus(ActualAccount, ID_STATUS_OCCUPIED);

		// if we are already connected, we have open session (another thread left us open session), so we don't need to login
		// note that connected state without logging cannot occur, because if we close session, we always close socket too 
		// (we must close socket is the right word :))
		if ((MyClient->NetClient == nullptr) || !MyClient->NetClient->Connected()) {
			SetStatusFcn(ActualAccount, TranslateT("Connecting to server"));

			DataRX = MyClient->Connect(ServerName, ServerPort, Flags & YAMN_ACC_SSL23, Flags & YAMN_ACC_NOTLS);
			char *timestamp = nullptr;

			if (DataRX != nullptr) {
				if (Flags & YAMN_ACC_APOP) {
					char *lpos = strchr(DataRX, '<');
					char *rpos = strchr(DataRX, '>');
					if (lpos && rpos && rpos > lpos) {
						int sz = (int)(rpos - lpos + 2);
						timestamp = new char[sz];
						memcpy(timestamp, lpos, sz - 1);
						timestamp[sz - 1] = '\0';
					}
				}
				free(DataRX);
				DataRX = nullptr;
			}

			SetStatusFcn(ActualAccount, TranslateT("Entering POP3 account"));

			if (Flags & YAMN_ACC_APOP) {
				DataRX = MyClient->APOP(ServerLogin, ServerPasswd, timestamp);
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				delete[] timestamp;
			}
			else {
				DataRX = MyClient->User(ServerLogin);
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				DataRX = MyClient->Pass(ServerPasswd);
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
			}
		}
		SetStatusFcn(ActualAccount, TranslateT("Searching for new mail message"));

		DataRX = MyClient->Stat();

		ExtractStat(DataRX, &mboxsize, &msgs);

		if (DataRX != nullptr)
			free(DataRX);
		DataRX = nullptr;
		for (i = 0; i < msgs; i++) {
			if (!i)
				MsgQueuePtr = NewMails = ActualAccount->CreateMail();
			else {
				MsgQueuePtr->Next = ActualAccount->CreateMail();
				MsgQueuePtr = MsgQueuePtr->Next;
			}
			if (MsgQueuePtr == nullptr) {
				ActualAccount->SystemError = EPOP3_QUEUEALLOC;
				throw (uint32_t)ActualAccount->SystemError;
			}
		}

		if (msgs) {
			DataRX = MyClient->List();
			ExtractList(DataRX, MyClient->NetClient->Rcv, NewMails);
			if (DataRX != nullptr)
				free(DataRX);

			DataRX = MyClient->Uidl();
			ExtractUIDL(DataRX, MyClient->NetClient->Rcv, NewMails);

			if (DataRX != nullptr)
				free(DataRX);
			DataRX = nullptr;
		}
		{
			SWriteGuard swm(ActualAccount->MessagesAccessSO);
			if (!swm.Succeeded())
				throw (uint32_t)(ActualAccount->SystemError = EACC_STOPPED);

			ActualAccount->LastChecked = now;
			for (MsgQueuePtr = (YAMNMAIL *)ActualAccount->Mails; MsgQueuePtr != nullptr; MsgQueuePtr = MsgQueuePtr->Next) {
				if (MsgQueuePtr->Flags & YAMN_MSG_BODYREQUESTED) {
					YAMNMAIL *NewMsgsPtr = nullptr;
					for (NewMsgsPtr = (YAMNMAIL *)NewMails; NewMsgsPtr != nullptr; NewMsgsPtr = NewMsgsPtr->Next) {
						if (!mir_strcmp(MsgQueuePtr->ID, NewMsgsPtr->ID)) {
							wchar_t accstatus[512];
							mir_snwprintf(accstatus, TranslateT("Reading body %s"), NewMsgsPtr->ID);
							SetStatusFcn(ActualAccount, accstatus);
							DataRX = MyClient->Top(MsgQueuePtr->Number, 100);
							if (DataRX == nullptr)
								continue;

							char *Temp = DataRX;
							while ((Temp < DataRX + MyClient->NetClient->Rcv) && (WS(Temp) || ENDLINE(Temp)))
								Temp++;

							if (OKLINE(DataRX))
								for (Temp = DataRX; (Temp < DataRX + MyClient->NetClient->Rcv) && (!ENDLINE(Temp)); Temp++);
							while ((Temp < DataRX + MyClient->NetClient->Rcv) && ENDLINE(Temp))
								Temp++;

							// delete all the headers of the old mail MsgQueuePtr->MailData->TranslatedHeader
							struct CMimeItem *TH = MsgQueuePtr->MailData->TranslatedHeader;
							if (TH) for (; MsgQueuePtr->MailData->TranslatedHeader != nullptr;) {
								TH = TH->Next;
								if (MsgQueuePtr->MailData->TranslatedHeader->name != nullptr)
									delete[] MsgQueuePtr->MailData->TranslatedHeader->name;
								if (MsgQueuePtr->MailData->TranslatedHeader->value != nullptr)
									delete[] MsgQueuePtr->MailData->TranslatedHeader->value;
								delete MsgQueuePtr->MailData->TranslatedHeader;
								MsgQueuePtr->MailData->TranslatedHeader = TH;
							}

							TranslateHeaderFcn(Temp, MyClient->NetClient->Rcv - (Temp - DataRX), &MsgQueuePtr->MailData->TranslatedHeader);

							MsgQueuePtr->Flags |= YAMN_MSG_BODYRECEIVED;

							if (DataRX != nullptr)
								free(DataRX);
							DataRX = nullptr;
							break;
						}
					}
				}
			}

			SynchroMessagesFcn(ActualAccount, (YAMNMAIL **)&ActualAccount->Mails, nullptr, (YAMNMAIL **)&NewMails, nullptr);		// we get only new mails on server!
		}

		for (MsgQueuePtr = (YAMNMAIL *)ActualAccount->Mails; MsgQueuePtr != nullptr; MsgQueuePtr = MsgQueuePtr->Next) {
			if ((MsgQueuePtr->Flags & YAMN_MSG_BODYREQUESTED) && (MsgQueuePtr->Flags & YAMN_MSG_BODYRECEIVED)) {
				MsgQueuePtr->Flags &= ~YAMN_MSG_BODYREQUESTED;
				if (MsgQueuePtr->MsgWindow)
					SendMessage(MsgQueuePtr->MsgWindow, WM_YAMN_CHANGECONTENT, 0, 0);
			}
		}

		for (msgs = 0, MsgQueuePtr = NewMails; MsgQueuePtr != nullptr; MsgQueuePtr = MsgQueuePtr->Next, msgs++);			// get number of new mails

		try {
			wchar_t accstatus[512];

			for (i = 0, MsgQueuePtr = NewMails; MsgQueuePtr != nullptr; i++) {
				BOOL autoretr = (ActualAccount->Flags & YAMN_ACC_BODY) != 0;
				DataRX = MyClient->Top(MsgQueuePtr->Number, autoretr ? 100 : 0);
				mir_snwprintf(accstatus, TranslateT("Reading new mail messages (%d%% done)"), 100 * i / msgs);
				SetStatusFcn(ActualAccount, accstatus);

				if (DataRX == nullptr)
					continue;

				char *Temp = DataRX;
				while ((Temp < DataRX + MyClient->NetClient->Rcv) && (WS(Temp) || ENDLINE(Temp)))
					Temp++;

				if (OKLINE(DataRX))
					for (Temp = DataRX; (Temp < DataRX + MyClient->NetClient->Rcv) && (!ENDLINE(Temp)); Temp++);
				while ((Temp < DataRX + MyClient->NetClient->Rcv) && ENDLINE(Temp))
					Temp++;

				TranslateHeaderFcn(Temp, MyClient->NetClient->Rcv - (Temp - DataRX), &MsgQueuePtr->MailData->TranslatedHeader);

				#ifdef DEBUG_DECODE
				mir_writeLogA(DecodeFile, "</New mail>\n");
				#endif
				MsgQueuePtr->Flags |= YAMN_MSG_NORMALNEW;
				if (autoretr)
					MsgQueuePtr->Flags |= YAMN_MSG_BODYRECEIVED;

				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;

				// MsgQueuePtr->MailData->Body=MyClient->Retr(MsgQueuePtr->Number);

				MsgQueuePtr = MsgQueuePtr->Next;

			}

			{
				SWriteGuard swm(ActualAccount->MessagesAccessSO);
				if (!swm.Succeeded())
					throw (uint32_t)ActualAccount->SystemError == EACC_STOPPED;

				if (ActualAccount->Mails == nullptr)
					ActualAccount->Mails = NewMails;
				else {
					ActualAccount->LastMail = ActualAccount->LastChecked;
					AppendQueueFcn((YAMNMAIL *)ActualAccount->Mails, NewMails);
				}
			}

			// 	we are going to delete mails having SPAM flag level3 and 4 (see m_mails.h) set
			// 	Delete mails from server. Here we should not be in write access for account's mails
			DeleteMailsPOP3(new DeleteParam(ActualAccount, true));

			// 	if there is no waiting thread for internet connection close it
			// 	else leave connection open
			if (0 == ActualAccount->InternetQueries.GetNumber()) {
				DataRX = MyClient->Quit();
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				MyClient->NetClient->Disconnect();

				SetStatusFcn(ActualAccount, TranslateT("Disconnected"));
			}

			UsingInternet = FALSE;
			SetEvent(ActualAccount->UseInternetFree);

			ActualAccount->LastSChecked = ActualAccount->LastChecked;
			ActualAccount->LastSynchronised = ActualAccount->LastChecked;
		}
		catch (...) {
			throw;			// go to the main exception handling
		}

		YAMN_MAILBROWSERPARAM Param = { ActualAccount, NFlags, NNFlags, 0 };
		if (bForceCheck)
			Param.nnflags |= YAMN_ACC_FORCEPOP;  // if force check, show popup anyway and if mailbrowser was opened, do not close
		Param.nnflags |= YAMN_ACC_MSGP;    // do not close browser if already open
		RunMailBrowser(&Param);

		SetContactStatus(ActualAccount, ActualAccount->isCounting ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
	}
	#ifdef DEBUG_COMM
	catch (uint32_t ErrorCode)
		#else
	catch (uint32_t)
		#endif
	{
		if (ActualAccount->Client.POP3Error == EPOP3_STOPPED)
			ActualAccount->SystemError = EACC_STOPPED;
		#ifdef DEBUG_COMM
		mir_writeLogA(CommFile, "ERROR: %x\n", ErrorCode);
		#endif
		{
			SWriteGuard swm(ActualAccount->MessagesAccessSO);
			if (swm.Succeeded())
				ActualAccount->LastChecked = now;
		}

		DeleteMessagesToEndFcn(ActualAccount, NewMails);

		if (DataRX != nullptr)
			free(DataRX);
		DataRX = nullptr;
		switch (ActualAccount->SystemError) {
		case EACC_QUEUEALLOC:
		case EACC_STOPPED:
			ActualAccount->Client.NetClient->Disconnect();
			break;
		default:
			PostErrorProc(ActualAccount, 0, 0, MyClient->SSL);	// it closes internet connection too
		}

		if (UsingInternet)	// if our thread still uses internet
			SetEvent(ActualAccount->UseInternetFree);

		SetContactStatus(ActualAccount, ID_STATUS_NA);
	}

	#ifdef DEBUG_COMM
	mir_writeLogA(CommFile, "</--------Communication-------->\n");
	#endif
}

void __cdecl DeleteMailsPOP3(void *param)
{
	DeleteParam *WhichTemp = (DeleteParam *)param;

	YAMNMAIL *DeleteMails, *NewMails = nullptr, *MsgQueuePtr = nullptr;
	int mboxsize = 0, msgs = 0, i;

	ptrA ServerName, ServerLogin, ServerPasswd;
	uint32_t ServerPort, Flags, NFlags, NNFlags;

	// copy address of structure from calling thread to stack of this thread
	CPOP3Account *ActualAccount = (CPOP3Account *)WhichTemp->AccountParam;
	bool bDelete = WhichTemp->bParam;
	delete WhichTemp;

	SCGuard sc(ActualAccount->UsingThreads);

	CPop3Client *MyClient;
	{
		SReadGuard sra(ActualAccount->AccountAccessSO);
		if (!sra.Succeeded())
			return;

		// if there's no mail for deleting, return
		if (nullptr == (DeleteMails = CreateNewDeleteQueueFcn((YAMNMAIL *)ActualAccount->Mails))) {
			// We do not wait for free internet when calling from SynchroPOP3. It is because UseInternetFree is blocked
			if (!bDelete) {
				YAMN_MAILBROWSERPARAM Param = { ActualAccount, YAMN_ACC_MSGP, YAMN_ACC_MSGP, 0 };		// Just update the window
				RunMailBrowser(&Param);
			}
			return;
		}

		MyClient = &ActualAccount->Client;

		// Now, copy all needed information about account to local variables, so ActualAccount is not blocked in read mode during all connection process, which can last for several minutes.
		ServerName = mir_strdup(ActualAccount->Server->Name);
		ServerPort = ActualAccount->Server->Port;
		Flags = ActualAccount->Flags;
		ServerLogin = mir_strdup(ActualAccount->Server->Login);
		ServerPasswd = mir_strdup(ActualAccount->Server->Passwd);
		NFlags = ActualAccount->NewMailN.Flags;
		NNFlags = ActualAccount->NoNewMailN.Flags;
	}
	{
		SCGuard scq(ActualAccount->InternetQueries);	// This is POP3-internal SCOUNTER, we set another thread wait for this account to be connected to inet
		if (!bDelete) // We do not wait for free internet when calling from SynchroPOP3. It is because UseInternetFree is blocked
			WaitForSingleObject(ActualAccount->UseInternetFree, INFINITE);
	}

	BOOL UsingInternet = TRUE;

	try {
		SetContactStatus(ActualAccount, ID_STATUS_OCCUPIED);
		#ifdef DEBUG_COMM
		mir_writeLogA(CommFile, "<--------Communication-------->\n");
		#endif
		if ((MyClient->NetClient == nullptr) || !MyClient->NetClient->Connected()) {
			SetStatusFcn(ActualAccount, TranslateT("Connecting to server"));

			char *DataRX = MyClient->Connect(ServerName, ServerPort, Flags & YAMN_ACC_SSL23, Flags & YAMN_ACC_NOTLS);

			char *timestamp = nullptr;
			if (DataRX != nullptr) {
				if (ActualAccount->Flags & YAMN_ACC_APOP) {
					char *lpos = strchr(DataRX, '<');
					char *rpos = strchr(DataRX, '>');
					if (lpos && rpos && rpos > lpos) {
						int sz = (int)(rpos - lpos + 2);
						timestamp = new char[sz];
						memcpy(timestamp, lpos, sz - 1);
						timestamp[sz - 1] = '\0';
					}
				}
				free(DataRX);
				DataRX = nullptr;
			}
			SetStatusFcn(ActualAccount, TranslateT("Entering POP3 account"));

			if (ActualAccount->Flags & YAMN_ACC_APOP) {
				DataRX = MyClient->APOP(ServerLogin, ServerPasswd, timestamp);
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				delete[] timestamp;
			}
			else {
				DataRX = MyClient->User(ServerLogin);
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				DataRX = MyClient->Pass(ServerPasswd);
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
			}
		}

		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<--------Deleting requested mails-------->\n");
		#endif
		if (!bDelete) { // We do not need to get mails on server as we have already it from check function
			SetStatusFcn(ActualAccount, TranslateT("Deleting requested mails"));

			char *DataRX = MyClient->Stat();

			#ifdef DEBUG_DECODE
			mir_writeLogA(DecodeFile, "<Extracting stat>\n");
			#endif
			ExtractStat(DataRX, &mboxsize, &msgs);
			#ifdef DEBUG_DECODE
			mir_writeLogA(DecodeFile, "<MailBoxSize>%d</MailBoxSize>\n", mboxsize);
			mir_writeLogA(DecodeFile, "<Msgs>%d</Msgs>\n", msgs);
			mir_writeLogA(DecodeFile, "</Extracting stat>\n");
			#endif
			if (DataRX != nullptr)
				free(DataRX);
			DataRX = nullptr;
			for (i = 0; i < msgs; i++) {
				if (!i)
					MsgQueuePtr = NewMails = ActualAccount->CreateMail();
				else {
					MsgQueuePtr->Next = ActualAccount->CreateMail();
					MsgQueuePtr = MsgQueuePtr->Next;
				}
				if (MsgQueuePtr == nullptr) {
					ActualAccount->SystemError = EPOP3_QUEUEALLOC;
					throw (uint32_t)ActualAccount->SystemError;
				}
			}

			if (msgs) {
				#ifdef DEBUG_DECODE
				mir_writeLogA(DecodeFile, "<Extracting UIDL>\n");
				#endif
				DataRX = MyClient->Uidl();
				ExtractUIDL(DataRX, MyClient->NetClient->Rcv, NewMails);
				#ifdef DEBUG_DECODE
				mir_writeLogA(DecodeFile, "</Extracting UIDL>\n");
				#endif
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				// we get "new mails" on server (NewMails will contain all mails on server not found in DeleteMails)
				// but also in DeleteMails we get only those, which are still on server with their responsable numbers
				SynchroMessagesFcn(ActualAccount, (YAMNMAIL **)&DeleteMails, nullptr, (YAMNMAIL **)&NewMails, nullptr);
			}
		}
		else SetStatusFcn(ActualAccount, TranslateT("Deleting spam"));

		{
			SWriteGuard swm(ActualAccount->MessagesAccessSO);
			if (!swm.Succeeded())
				throw (uint32_t)EACC_STOPPED;

			if (msgs || bDelete) {
				for (i = 0, MsgQueuePtr = DeleteMails; MsgQueuePtr != nullptr; i++) {
					if (!(MsgQueuePtr->Flags & YAMN_MSG_VIRTUAL)) {	// of course we can only delete real mails, not virtual
						char *DataRX = MyClient->Dele(MsgQueuePtr->Number);
						YAMNMAIL *Temp = MsgQueuePtr->Next;
						if (POP3_FOK == MyClient->AckFlag) { // if server answers that mail was deleted
							DeleteMessageFromQueueFcn((YAMNMAIL **)&DeleteMails, MsgQueuePtr);
							YAMNMAIL *DeletedMail = FindMessageByIDFcn((YAMNMAIL *)ActualAccount->Mails, MsgQueuePtr->ID);
							if ((MsgQueuePtr->Flags & YAMN_MSG_MEMDELETE)) { // if mail should be deleted from memory (or disk)
								DeleteMessageFromQueueFcn((YAMNMAIL **)&ActualAccount->Mails, DeletedMail);	// remove from queue
								DeleteAccountMail(POP3Plugin, DeletedMail);
							}
							else { // else mark it only as "deleted mail"
								DeletedMail->Flags |= (YAMN_MSG_VIRTUAL | YAMN_MSG_DELETED);
								DeletedMail->Flags &= ~(YAMN_MSG_NEW | YAMN_MSG_USERDELETE | YAMN_MSG_AUTODELETE);	// clear "new mail"
							}
							delete   MsgQueuePtr->MailData;
							delete[] MsgQueuePtr->ID;
							delete   MsgQueuePtr;
						}
						MsgQueuePtr = Temp;

						if (DataRX != nullptr)
							free(DataRX);
						DataRX = nullptr;
					}
					else MsgQueuePtr = MsgQueuePtr->Next;
				}

				if (NewMails != nullptr)
					// 	in ActualAccount->Mails we have all mails stored before calling this function
					// 	in NewMails we have all mails not found in DeleteMails (in other words: we performed new ID checking and we
					// 	stored all mails found on server, then we deleted the ones we wanted to delete in this function
					// 	and NewMails queue now contains actual state of mails on server). But we will not use NewMails as actual state, because NewMails does not contain header data (subject, from...)
					// 	We perform deleting from ActualAccount->Mails: we remove from original queue (ActualAccount->Mails) all deleted mails
					SynchroMessagesFcn(ActualAccount, (YAMNMAIL **)&ActualAccount->Mails, nullptr, (YAMNMAIL **)&NewMails, nullptr);
				// 	Now ActualAccount->Mails contains all mails when calling this function except the ones, we wanted to delete (these are in DeleteMails)
				// 	And in NewMails we have new mails (if any)
				else if (!bDelete) {
					DeleteMessagesToEndFcn(ActualAccount, (YAMNMAIL *)ActualAccount->Mails);
					ActualAccount->Mails = nullptr;
				}
			}
			else {
				DeleteMessagesToEndFcn(ActualAccount, (YAMNMAIL *)ActualAccount->Mails);
				ActualAccount->Mails = nullptr;
			}
		}

		#ifdef DEBUG_DECODE     	
		mir_writeLogA(DecodeFile, "</--------Deleting requested mails-------->\n");
		#endif

		// 	TODO: now, we have in NewMails new mails. If NewMails is not NULL, we found some new mails, so Checking for new mail should be performed
		// 	now, we do not call CheckPOP3

		// 	if there is no waiting thread for internet connection close it
		// 	else leave connection open
		// 	if this functin was called from SynchroPOP3, then do not try to disconnect 
		if (!bDelete) {
			YAMN_MAILBROWSERPARAM Param = { ActualAccount, NFlags, YAMN_ACC_MSGP, 0 };
			RunMailBrowser(&Param);

			if (0 == ActualAccount->InternetQueries.GetNumber()) {
				char *DataRX = MyClient->Quit();
				if (DataRX != nullptr)
					free(DataRX);
				DataRX = nullptr;
				MyClient->NetClient->Disconnect();

				SetStatusFcn(ActualAccount, TranslateT("Disconnected"));
			}

			UsingInternet = FALSE;
			SetEvent(ActualAccount->UseInternetFree);
		}
		SetContactStatus(ActualAccount, ActualAccount->isCounting ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
	}
	#ifdef DEBUG_COMM
	catch (uint32_t ErrorCode)
		#else
	catch (uint32_t)
		#endif
	{
		if (ActualAccount->Client.POP3Error == EPOP3_STOPPED)
			ActualAccount->SystemError = EACC_STOPPED;
		#ifdef DEBUG_COMM
		mir_writeLogA(CommFile, "ERROR %x\n", ErrorCode);
		#endif

		switch (ActualAccount->SystemError) {
		case EACC_QUEUEALLOC:
		case EACC_STOPPED:
			ActualAccount->Client.NetClient->Disconnect();
			break;
		default:
			PostErrorProc(ActualAccount, 0, bDelete, MyClient->SSL);	// it closes internet connection too
		}

		if (UsingInternet && !bDelete)	// if our thread still uses internet and it is needed to release internet
			SetEvent(ActualAccount->UseInternetFree);
	}

	DeleteMessagesToEndFcn(ActualAccount, NewMails);
	DeleteMessagesToEndFcn(ActualAccount, DeleteMails);

	#ifdef DEBUG_COMM
	mir_writeLogA(CommFile, "</--------Communication-------->\n");
	#endif

	// 	WriteAccounts();
	return;
}

void ExtractStat(char *stream, int *mboxsize, int *mails)
{
	char *finder = stream;
	while (WS(finder) || ENDLINE(finder)) finder++;
	if (ACKLINE(finder)) {
		SkipNonSpaces(finder);
		SkipSpaces(finder);
	}
	if (1 != sscanf(finder, "%d", mails))
		throw (uint32_t)EPOP3_STAT;

	SkipNonSpaces(finder);
	SkipSpaces(finder);
	if (1 != sscanf(finder, "%d", mboxsize))
		throw (uint32_t)EPOP3_STAT;
}
void ExtractMail(char *stream, int len, YAMNMAIL *queue)
{
	char *finder = stream;
	char *finderend;
	int msgnr, i;
	YAMNMAIL *queueptr = queue;

	while (WS(finder) || ENDLINE(finder)) finder++;
	while (!ACKLINE(finder)) finder++;
	while (!ENDLINE(finder)) finder++;			// now we at the end of first ack line
	while (finder <= (stream + len)) {
		while (ENDLINE(finder)) finder++;		// go to the new line
		if (DOTLINE(finder + 1))					// at the end of stream
			break;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Message>\n");
		#endif
		SkipSpaces(finder);			// jump whitespace
		if (1 != sscanf(finder, "%d", &msgnr))
			throw (uint32_t)EPOP3_UIDL;

		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Nr>%d</Nr>\n", msgnr);
		#endif

		SkipNonSpaces(finder);
		SkipSpaces(finder);
		finderend = finder + 1;
		while (!WS(finderend) && !ENDLINE(finderend)) finderend++;
		queueptr->ID = new char[finderend - finder + 1];
		for (i = 0; finder != finderend; finder++, i++)
			queueptr->MailData->Body[i] = *finder;
		queueptr->MailData->Body[i] = 0;				// ends string
		queueptr->Number = msgnr;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<ID>%s</ID>\n", queueptr->MailData->Body);
		mir_writeLogA(DecodeFile, "</Message>\n");
		#endif
		queueptr = queueptr->Next;
		while (!ENDLINE(finder)) finder++;
	}
}

void ExtractUIDL(char *stream, int len, YAMNMAIL *queue)
{
	char *finder = stream;
	char *finderend;
	int msgnr, i;
	YAMNMAIL *queueptr = queue;

	while (WS(finder) || ENDLINE(finder)) finder++;
	while (!ACKLINE(finder)) finder++;
	while (!ENDLINE(finder)) finder++;			// now we at the end of first ack line
	while (finder <= (stream + len)) {
		while (ENDLINE(finder)) finder++;		// go to the new line
		if (DOTLINE(finder + 1))					// at the end of stream
			break;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Message>\n");
		#endif
		SkipSpaces(finder);
		if (1 != sscanf(finder, "%d", &msgnr))
			throw (uint32_t)EPOP3_UIDL;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Nr>%d</Nr>\n", msgnr);
		#endif
		// 		for (i=1,queueptr=queue;(queueptr->Next != NULL) && (i<msgnr);queueptr=queueptr->Next,i++);
		// 		if (i != msgnr)
		// 			throw (uint32_t)EPOP3_UIDL;
		SkipNonSpaces(finder);
		SkipSpaces(finder);
		finderend = finder + 1;
		while (!WS(finderend) && !ENDLINE(finderend)) finderend++;
		queueptr->ID = new char[finderend - finder + 1];
		for (i = 0; finder != finderend; finder++, i++)
			queueptr->ID[i] = *finder;
		queueptr->ID[i] = 0;				// ends string
		queueptr->Number = msgnr;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<ID>%s</ID>\n", queueptr->ID);
		mir_writeLogA(DecodeFile, "</Message>\n");
		#endif
		queueptr = queueptr->Next;
		while (!ENDLINE(finder)) finder++;
	}
}

void ExtractList(char *stream, int len, YAMNMAIL *queue)
{
	char *finder = stream;
	char *finderend;
	int msgnr, i;
	YAMNMAIL *queueptr;

	while (WS(finder) || ENDLINE(finder)) finder++;
	while (!ACKLINE(finder)) finder++;
	while (!ENDLINE(finder)) finder++;			// now we at the end of first ack line
	while (finder <= (stream + len)) {
		while (ENDLINE(finder)) finder++;		// go to the new line
		if (DOTLINE(finder + 1))				// at the end of stream
			break;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Message>\n", NULL, 0);
		#endif
		SkipSpaces(finder);
		if (1 != sscanf(finder, "%d", &msgnr))		// message nr.
			throw (uint32_t)EPOP3_LIST;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Nr>%d</Nr>\n", msgnr);
		#endif

		for (i = 1, queueptr = queue; (queueptr->Next != nullptr) && (i < msgnr); queueptr = queueptr->Next, i++);
		if (i != msgnr)
			throw (uint32_t)EPOP3_LIST;

		SkipNonSpaces(finder);
		SkipSpaces(finder);			// jump whitespace
		finderend = finder + 1;
		if (1 != sscanf(finder, "%u", &queueptr->MailData->Size))
			throw (uint32_t)EPOP3_LIST;
		#ifdef DEBUG_DECODE
		mir_writeLogA(DecodeFile, "<Nr>%d</Nr>\n", queueptr->MailData->Size);
		#endif
		while (!ENDLINE(finder)) finder++;
	}
}

wchar_t *MIR_CDECL GetErrorString(DWORD Code)
{
	static wchar_t *POP3Errors[] =
	{
		LPGENW("Memory allocation error."),		// memory allocation
		LPGENW("Account is about to be stopped."),	// stop account
		LPGENW("Cannot connect to POP3 server."),
		LPGENW("Cannot allocate memory for received data."),
		LPGENW("Cannot login to POP3 server."),
		LPGENW("Bad user or password."),
		LPGENW("Server does not support APOP authorization."),
		LPGENW("Error while executing POP3 command."),
		LPGENW("Error while executing POP3 command."),
		LPGENW("Error while executing POP3 command."),
	};

	static wchar_t *NetlibErrors[] =
	{
		LPGENW("Cannot connect to server with NetLib."),
		LPGENW("Cannot send data."),
		LPGENW("Cannot receive data."),
		LPGENW("Cannot allocate memory for received data."),
	};

	static wchar_t *SSLErrors[] =
	{
		LPGENW("OpenSSL not loaded."),
		LPGENW("Windows socket 2.0 init failed."),
		LPGENW("DNS lookup error."),
		LPGENW("Error while creating base socket."),
		LPGENW("Error connecting to server with socket."),
		LPGENW("Error while creating SSL structure."),
		LPGENW("Error connecting socket with SSL."),
		LPGENW("Server rejected connection with SSL."),
		LPGENW("Cannot write SSL data."),
		LPGENW("Cannot read SSL data."),
		LPGENW("Cannot allocate memory for received data."),
	};

	wchar_t *ErrorString = new wchar_t[ERRORSTR_MAXLEN];
	POP3_ERRORCODE *ErrorCode = (POP3_ERRORCODE *)(UINT_PTR)Code;

	mir_snwprintf(ErrorString, ERRORSTR_MAXLEN, TranslateT("Error %d-%d-%d-%d:"), ErrorCode->AppError, ErrorCode->POP3Error, ErrorCode->NetError, ErrorCode->SystemError);
	if (ErrorCode->POP3Error)
		mir_snwprintf(ErrorString, ERRORSTR_MAXLEN, L"%s\n%s", ErrorString, TranslateW(POP3Errors[ErrorCode->POP3Error - 1]));
	if (ErrorCode->NetError) {
		if (ErrorCode->SSL)
			mir_snwprintf(ErrorString, ERRORSTR_MAXLEN, L"%s\n%s", ErrorString, TranslateW(SSLErrors[ErrorCode->NetError - 1]));
		else
			mir_snwprintf(ErrorString, ERRORSTR_MAXLEN, L"%s\n%s", ErrorString, TranslateW(NetlibErrors[ErrorCode->NetError - 4]));
	}

	return ErrorString;
}

void MIR_CDECL DeleteErrorString(LPVOID String)
{
	delete (char *)String;
}
