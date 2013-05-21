#pragma once


extern HINSTANCE hLotusDll;

typedef STATUS (CALLBACK *OSPATHNETCONSTRUCT)(
	const char far *PortName,
	const char far *ServerName,
	const char far *FileName,
	char far *retPathName);
extern OSPATHNETCONSTRUCT OSPathNetConstruct1;

typedef STATUS (CALLBACK LNPUBLIC *NOTESINITEXTENDED)(
	int  argc,
	char far * far *argv);
extern NOTESINITEXTENDED NotesInitExtended1;

typedef STATUS (CALLBACK LNPUBLIC *NSFDBOPEN)(
	const char far *PathName,
	DBHANDLE far *rethDB);
extern NSFDBOPEN NSFDbOpen1;

typedef STATUS  (CALLBACK LNPUBLIC *SECKFMGETUSERNAME)(
	char far *retUserName);
extern SECKFMGETUSERNAME SECKFMGetUserName1;

typedef STATUS (CALLBACK LNPUBLIC *NSFDBGETUNREADNOTETABLE)(
	DBHANDLE  hDB,
	char far *UserName,
	WORD  UserNameLength,
	BOOL  fCreateIfNotAvailable,
	HANDLE far *rethUnreadList);
extern NSFDBGETUNREADNOTETABLE NSFDbGetUnreadNoteTable1;

typedef STATUS (CALLBACK LNPUBLIC *NSFDBUPDATEUNREAD)(
	DBHANDLE  hDataDB,
	HANDLE  hUnreadList);
extern NSFDBUPDATEUNREAD NSFDbUpdateUnread1;

typedef BOOL (CALLBACK LNPUBLIC *IDSCAN)(
	HANDLE  hTable,///DHANDLE
	BOOL  fFirst,
	DWORD far *retID);
extern IDSCAN IDScan1;

typedef STATUS (CALLBACK LNPUBLIC *NSFNOTEOPEN)(
	DBHANDLE  db_handle,
	NOTEID  note_id,
	WORD  open_flags,
	NOTEHANDLE far *note_handle);
extern NSFNOTEOPEN NSFNoteOpen1;

typedef STATUS (CALLBACK LNPUBLIC *NSFDBGETNOTEINFO)(
	DBHANDLE  hDb,
	NOTEID  NoteID,
	OID far *retNoteOID,
	TIMEDATE far *retModified,
	WORD far *retNoteClass);
extern NSFDBGETNOTEINFO NSFDbGetNoteInfo1;

typedef WORD (CALLBACK LNPUBLIC *NSFITEMGETTEXT)(
	NOTEHANDLE  note_handle,
	const char far *item_name,
	char far *item_text,
	WORD  text_len);
extern NSFITEMGETTEXT NSFItemGetText1;

typedef BOOL (CALLBACK LNPUBLIC *NSFITEMGETTIME)(
	NOTEHANDLE  note_handle,
	const char far *td_item_name,
	TIMEDATE far *td_item_value);
extern NSFITEMGETTIME NSFItemGetTime1;

typedef STATUS (CALLBACK LNPUBLIC *CONVERTTIMEDATETOTEXT)(
	const void far *IntlFormat,
	const TFMT far *TextFormat,
	const TIMEDATE far *InputTime,
	char far *retTextBuffer,
	WORD  TextBufferLength,
	WORD far *retTextLength);
extern CONVERTTIMEDATETOTEXT ConvertTIMEDATEToText1;

typedef WORD (CALLBACK LNPUBLIC *OSTRANSLATE)(
	WORD  TranslateMode,
	const char far *In,
	WORD  InLength,
	char far *Out,
	WORD  OutLength);
extern OSTRANSLATE OSTranslate1;

typedef BOOL (CALLBACK LNPUBLIC *MAILGETMESSAGEATTACHMENTINFO)(
	HANDLE  hMessage, ///DHANDLE
	WORD  Num,
	BLOCKID far *bhItem,
	char far *FileName,
	DWORD far *FileSize,
	WORD far *FileAttributes,
	WORD far *FileHostType,
	TIMEDATE *FileCreated,
	TIMEDATE far *FileModified);
extern MAILGETMESSAGEATTACHMENTINFO MailGetMessageAttachmentInfo1;

typedef STATUS (CALLBACK LNPUBLIC *NSFNOTECLOSE)(
	NOTEHANDLE note_handle);
extern NSFNOTECLOSE NSFNoteClose1;

typedef STATUS (CALLBACK LNPUBLIC *IDDESTROYTABLE)(
	HANDLE hTable); ///DHANDLE
extern IDDESTROYTABLE IDDestroyTable1;

typedef STATUS (CALLBACK LNPUBLIC *NSFDBCLOSE)(
	DBHANDLE hDB);
extern NSFDBCLOSE NSFDbClose1;

typedef WORD (CALLBACK LNPUBLIC *OSLOADSTRING)(
	HMODULE  hModule,
	STATUS  StringCode,
	char far *retBuffer,
	WORD  BufferLength);
extern OSLOADSTRING OSLoadString1;

typedef void (CALLBACK LNPUBLIC *NOTESTERM)(void);
extern NOTESTERM NotesTerm1;

typedef BOOL (CALLBACK LNPUBLIC *OSGETENVIRONMENTSTRING)(
	const char far *VariableName,
	char far *retValueBuffer,
	WORD  BufferLength);
extern OSGETENVIRONMENTSTRING OSGetEnvironmentString1;

typedef void (CALLBACK LNPUBLIC *OSSETENVIRONMENTVARIABLE)(
	const char far *VariableName,
	const char far *Value);
extern OSSETENVIRONMENTVARIABLE OSSetEnvironmentVariable1;

typedef STATUS  (CALLBACK LNPUBLIC *NSGETSERVERLIST)(
	char far *pPortName,
	HANDLE far *retServerTextList); ///DHANDLE
extern NSGETSERVERLIST NSGetServerList1;

typedef void far* (CALLBACK LNPUBLIC *OSLOCKOBJECT)(
	HANDLE  Handle); ///DHANDLE
extern OSLOCKOBJECT OSLockObject1;

typedef BOOL (CALLBACK LNPUBLIC *OSUNLOCKOBJECT)(
	HANDLE  Handle); ///DHANDLE
extern OSUNLOCKOBJECT OSUnlockObject1;

typedef STATUS  (CALLBACK LNPUBLIC *OSMEMFREE)(
	HANDLE Handle); ///DHANDLE
extern OSMEMFREE OSMemFree1;

typedef STATUS  (CALLBACK LNPUBLIC *EMREGISTER)(
	EID  EmID,
	DWORD  Flags,
	EMHANDLER  Proc,
	WORD  RecursionID,
	HEMREGISTRATION far *rethRegistration);
extern EMREGISTER EMRegister1;

typedef STATUS (CALLBACK LNPUBLIC *EMDEREGISTER)(
	HANDLE  Handle); ///HEMREGISTRATION
extern EMDEREGISTER EMDeregister1;

typedef STATUS  (CALLBACK LNPUBLIC *NOTESINITTHREAD)(void);
extern NOTESINITTHREAD NotesInitThread1;

typedef void  (CALLBACK LNPUBLIC *NOTESTERMTHREAD)(void);
extern NOTESTERMTHREAD NotesTermThread1;




BOOL HookLotusFunctions();
void GetLotusPath(TCHAR *sTemp, DWORD size);

