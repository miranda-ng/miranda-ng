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
	uint16_t  UserNameLength,
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
	uint32_t far *retID);
extern IDSCAN IDScan1;

typedef STATUS (CALLBACK LNPUBLIC *NSFNOTEOPEN)(
	DBHANDLE  db_handle,
	NOTEID  note_id,
	uint16_t  open_flags,
	NOTEHANDLE far *note_handle);
extern NSFNOTEOPEN NSFNoteOpen1;

typedef STATUS (CALLBACK LNPUBLIC *NSFDBGETNOTEINFO)(
	DBHANDLE  hDb,
	NOTEID  NoteID,
	OID far *retNoteOID,
	TIMEDATE far *retModified,
	uint16_t far *retNoteClass);
extern NSFDBGETNOTEINFO NSFDbGetNoteInfo1;

typedef uint16_t (CALLBACK LNPUBLIC *NSFITEMGETTEXT)(
	NOTEHANDLE  note_handle,
	const char far *item_name,
	char far *item_text,
	uint16_t  text_len);
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
	uint16_t  TextBufferLength,
	uint16_t far *retTextLength);
extern CONVERTTIMEDATETOTEXT ConvertTIMEDATEToText1;

typedef uint16_t (CALLBACK LNPUBLIC *OSTRANSLATE)(
	uint16_t  TranslateMode,
	const char far *In,
	uint16_t  InLength,
	char far *Out,
	uint16_t  OutLength);
extern OSTRANSLATE OSTranslate1;

typedef BOOL (CALLBACK LNPUBLIC *MAILGETMESSAGEATTACHMENTINFO)(
	HANDLE  hMessage, ///DHANDLE
	uint16_t  Num,
	BLOCKID far *bhItem,
	char far *FileName,
	uint32_t far *FileSize,
	uint16_t far *FileAttributes,
	uint16_t far *FileHostType,
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

typedef uint16_t (CALLBACK LNPUBLIC *OSLOADSTRING)(
	HMODULE  hModule,
	STATUS  StringCode,
	char far *retBuffer,
	uint16_t  BufferLength);
extern OSLOADSTRING OSLoadString1;

typedef void (CALLBACK LNPUBLIC *NOTESTERM)(void);
extern NOTESTERM NotesTerm1;

typedef BOOL (CALLBACK LNPUBLIC *OSGETENVIRONMENTSTRING)(
	const char far *VariableName,
	char far *retValueBuffer,
	uint16_t  BufferLength);
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
	uint32_t  Flags,
	EMHANDLER  Proc,
	uint16_t  RecursionID,
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
void GetLotusPath(wchar_t *sTemp, DWORD size);

