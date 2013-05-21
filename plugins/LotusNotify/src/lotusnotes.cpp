#include "stdafx.h"
#include "lotusnotes.h"

OSPATHNETCONSTRUCT OSPathNetConstruct1;
NOTESINITEXTENDED NotesInitExtended1;
NSFDBOPEN NSFDbOpen1;
SECKFMGETUSERNAME SECKFMGetUserName1;
NSFDBGETUNREADNOTETABLE NSFDbGetUnreadNoteTable1;
NSFDBUPDATEUNREAD NSFDbUpdateUnread1;
IDSCAN IDScan1;
NSFNOTEOPEN NSFNoteOpen1;
NSFDBGETNOTEINFO NSFDbGetNoteInfo1;
NSFITEMGETTEXT NSFItemGetText1;
NSFITEMGETTIME NSFItemGetTime1;
CONVERTTIMEDATETOTEXT ConvertTIMEDATEToText1;
OSTRANSLATE OSTranslate1;
MAILGETMESSAGEATTACHMENTINFO MailGetMessageAttachmentInfo1;
NSFNOTECLOSE NSFNoteClose1;
IDDESTROYTABLE IDDestroyTable1;
NSFDBCLOSE NSFDbClose1;
OSLOADSTRING OSLoadString1;
NOTESTERM NotesTerm1;
OSGETENVIRONMENTSTRING OSGetEnvironmentString1;
OSSETENVIRONMENTVARIABLE OSSetEnvironmentVariable1;
NSGETSERVERLIST NSGetServerList1;
OSLOCKOBJECT OSLockObject1;
OSUNLOCKOBJECT OSUnlockObject1;
OSMEMFREE OSMemFree1;
EMREGISTER EMRegister1;
EMDEREGISTER EMDeregister1;
NOTESINITTHREAD NotesInitThread1;
NOTESTERMTHREAD NotesTermThread1;

BOOL HookLotusFunctions()
{
	return (
		   (OSPathNetConstruct1 = (OSPATHNETCONSTRUCT)GetProcAddress(hLotusDll, "OSPathNetConstruct"))
		&& (NotesInitExtended1 = (NOTESINITEXTENDED)GetProcAddress(hLotusDll, "NotesInitExtended"))
		&& (NSFDbOpen1 = (NSFDBOPEN)GetProcAddress(hLotusDll, "NSFDbOpen"))
		&& (SECKFMGetUserName1 = (SECKFMGETUSERNAME)GetProcAddress(hLotusDll, "SECKFMGetUserName"))
		&& (NSFDbGetUnreadNoteTable1 = (NSFDBGETUNREADNOTETABLE)GetProcAddress(hLotusDll, "NSFDbGetUnreadNoteTable"))
		&& (NSFDbUpdateUnread1 = (NSFDBUPDATEUNREAD)GetProcAddress(hLotusDll, "NSFDbUpdateUnread"))
		&& (IDScan1 = (IDSCAN)GetProcAddress(hLotusDll, "IDScan"))
		&& (NSFNoteOpen1 = (NSFNOTEOPEN)GetProcAddress(hLotusDll, "NSFNoteOpen"))
		&& (NSFDbGetNoteInfo1 = (NSFDBGETNOTEINFO)GetProcAddress(hLotusDll, "NSFDbGetNoteInfo"))
		&& (NSFItemGetText1 = (NSFITEMGETTEXT)GetProcAddress(hLotusDll, "NSFItemGetText"))
		&& (NSFItemGetTime1 = (NSFITEMGETTIME)GetProcAddress(hLotusDll, "NSFItemGetTime"))
		&& (ConvertTIMEDATEToText1 = (CONVERTTIMEDATETOTEXT)GetProcAddress(hLotusDll, "ConvertTIMEDATEToText"))
		&& (OSTranslate1 = (OSTRANSLATE)GetProcAddress(hLotusDll, "OSTranslate"))
		&& (MailGetMessageAttachmentInfo1 = (MAILGETMESSAGEATTACHMENTINFO)GetProcAddress(hLotusDll, "MailGetMessageAttachmentInfo"))
		&& (NSFNoteClose1 = (NSFNOTECLOSE)GetProcAddress(hLotusDll, "NSFNoteClose"))
		&& (IDDestroyTable1 = (IDDESTROYTABLE)GetProcAddress(hLotusDll, "IDDestroyTable"))
		&& (NSFDbClose1 = (NSFDBCLOSE)GetProcAddress(hLotusDll, "NSFDbClose"))
		&& (OSLoadString1 = (OSLOADSTRING)GetProcAddress(hLotusDll, "OSLoadString"))
		&& (NotesTerm1 = (NOTESTERM)GetProcAddress(hLotusDll, "NotesTerm"))
		&& (OSGetEnvironmentString1 = (OSGETENVIRONMENTSTRING)GetProcAddress(hLotusDll, "OSGetEnvironmentString"))
		&& (OSSetEnvironmentVariable1 = (OSSETENVIRONMENTVARIABLE)GetProcAddress(hLotusDll, "OSSetEnvironmentVariable"))
		&& (NSGetServerList1 = (NSGETSERVERLIST)GetProcAddress(hLotusDll, "NSGetServerList"))
		&& (OSLockObject1 = (OSLOCKOBJECT)GetProcAddress(hLotusDll, "OSLockObject"))
		&& (OSUnlockObject1 = (OSUNLOCKOBJECT)GetProcAddress(hLotusDll, "OSUnlockObject"))
		&& (OSMemFree1 = (OSMEMFREE)GetProcAddress(hLotusDll, "OSMemFree"))
		&& (EMRegister1 = (EMREGISTER)GetProcAddress(hLotusDll, "EMRegister"))
		&& (EMDeregister1 = (EMDEREGISTER)GetProcAddress(hLotusDll, "EMDeregister"))
		&& (NotesInitThread1 = (NOTESINITTHREAD)GetProcAddress(hLotusDll, "NotesInitThread"))
		&& (NotesTermThread1 = (NOTESTERMTHREAD)GetProcAddress(hLotusDll, "NotesTermThread"))
	) ? TRUE : FALSE;
}

void GetLotusPath(TCHAR *sTemp, DWORD size)
{
	DWORD rc; 
	HKEY dmKey;

	rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE , TEXT("Software\\Lotus\\Notes") ,0, KEY_QUERY_VALUE, &dmKey ); 

	if (rc != ERROR_SUCCESS) { 
		return; 
	} 

	RegQueryValueEx( dmKey, TEXT("Path"), NULL, NULL, (BYTE*)sTemp, &size );
	RegCloseKey(dmKey);
	return;	
}

