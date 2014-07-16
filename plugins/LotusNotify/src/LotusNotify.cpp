/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

#include "debug.h"
#include "resource.h"
#include "version.h"
#include "lotusnotes.h"
#include "LotusNotify.h"


#define MAX_FIELD 256
#define MAX_SETTING_STR 512
#define STATUS_COUNT 9

char PLUGINNAME[64] = {0}; //init at init_pluginname();
int hLangpack = 0;
HINSTANCE hInst;

HINSTANCE hLotusDll;
HEMREGISTRATION hLotusRegister = 0;

boolean volatile Plugin_Terminated = false;
CRITICAL_SECTION checkthreadCS;

HANDLE hMenuService = NULL;
HANDLE hMenuHandle = NULL;
HANDLE hCheckEvent = NULL;
HANDLE hCheckHook = NULL;
HANDLE hHookModulesLoaded = NULL;
HANDLE hHookPreShutdown = NULL;

HANDLE hOptInit = NULL;
static HWND hTimerWnd = (HWND)NULL;
static UINT TID = (UINT)2006;

char settingServer[MAX_SETTING_STR] = "";
char settingServerSec[MAX_SETTING_STR] = "";
char settingDatabase[MAX_SETTING_STR] = "";
char settingCommand[MAX_SETTING_STR] = "";
char settingParameters[MAX_SETTING_STR] = "";
TCHAR settingFilterSubject[MAX_SETTING_STR] = TEXT("");
TCHAR settingFilterSender[MAX_SETTING_STR] = TEXT("");
TCHAR settingFilterTo[MAX_SETTING_STR] = TEXT("");
char settingPassword[MAX_SETTING_STR] = "";

COLORREF settingBgColor;
COLORREF settingFgColor;
int settingInterval = 0;
int settingInterval1 = 0;
DWORD settingNewestID = 0;
BYTE settingSetColours = 0;
BYTE settingShowError = 1;
BYTE settingIniAnswer = -1;
BYTE settingIniCheck = 0;
BYTE settingOnceOnly = 0;
BYTE settingNonClickedOnly = 0;
BYTE settingNewest = 0;
BOOL settingStatus[STATUS_COUNT];
BOOL bMirandaCall=FALSE;

struct HISTORIA *first = NULL;
BOOL running = FALSE;
BOOL second = FALSE;
BOOL isPopupWaiting = FALSE;
int currentStatus = ID_STATUS_OFFLINE;
int diffstat = 0;
int startuperror = 0;
TCHAR *startuperrors[] = {
		LPGENT("Unable to load all required Lotus API functions"),
		LPGENT("Lotus Notes Client not detected. Check plugin configuration description on install.txt"),
		LPGENT("Unable to initialize Notes."),
		LPGENT("Lotus Notes Extension Manager was not registered. Authentication function will not work properly"),
		LPGENT("In notes.ini file there is no required entry EXTMGR_ADDINS=plugindllnamewithout\".dll\"")
	};


PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0x23eacc0d, 0xbab0, 0x49c0, { 0x8f, 0x37, 0x5e, 0x25, 0x9e, 0xce, 0x52, 0x7f } } // {23EACC0D-BAB0-49c0-8F37-5E259ECE527F}
};



// authentication callback futnction from extension manager  called by nnotes.dll
STATUS LNPUBLIC __stdcall EMCallBack (EMRECORD * pData)
{
	VARARG_PTR pArgs;
	DWORD maxPwdLen;
	DWORD * retLength;
	char * retPassword;
	char * fileName;
	char * ownerName;

	if (pData->EId != EM_GETPASSWORD) return (ERR_EM_CONTINUE); //asking for password?
	if (pData->Status != NOERROR) return (ERR_EM_CONTINUE);

	pArgs= pData->Ap;
	maxPwdLen = VARARG_GET (pArgs, DWORD);
	retLength = VARARG_GET (pArgs, DWORD *);
	retPassword = VARARG_GET (pArgs, char *);
	fileName = VARARG_GET (pArgs, char *);
	ownerName = VARARG_GET (pArgs, char *);
	strncpy(retPassword, settingPassword, strlen(settingPassword)); //set our password
	retPassword[strlen(settingPassword)]='\0';
	*retLength = (DWORD)strlen(retPassword);//and his length
	return ERR_BSAFE_EXTERNAL_PASSWORD;
}


//Main entry point for Ext Manager of Lotus API. called by nnotes.dll
//It don't work (don't know why), and Mirandas Load function is called with value 1 or 0 as parameter...
__declspec(dllexport)STATUS LNPUBLIC MainEntryPoint (void)
{
	STATUS rc;
	rc = EMRegister1 (EM_GETPASSWORD, EM_REG_BEFORE | EM_REG_AFTER, EMCallBack, 0, &hLotusRegister); //Extension Manager must know that we are here
	if(rc) {
		//Extension magager don't know who we are :(
		startuperror+=8;
		// Get the info from the .ini file
	}
	return rc;
}


//Clear Extension Manager when exiting
void ExtClear()
{
	STATUS status;
	if (0 != hLotusRegister) {
		status = (EMDeregister1) (&hLotusRegister); //we was registered, so let's unregister
	} else {
		status = NOERROR;
	}
	log_p(L"ExtClear() status=%d", status);
	return;
}


//check if msg was clicked and exists on msgs list
struct HISTORIA* getEl(DWORD id)
{
	struct HISTORIA *cur = first;
	while(cur != NULL)
	{
		if(cur->noteID == id)
			return cur;
		cur = cur->next;
	}
	return NULL;
}


//creates new entry on list of msgs
void addNewId(DWORD id)
{
	struct HISTORIA* nowy = (struct HISTORIA*)mir_alloc(sizeof(struct HISTORIA)) ;
	assert(nowy);
	nowy->noteID = id;
	nowy->next = first;
	nowy->pq = NULL;
	nowy->again = FALSE;
	first = nowy;
}


//add popup handle. This queue is used to close popups with same msg
void addPopup(DWORD id,HWND hWnd)
{
	struct POPUPSQUEUE* nowy = (struct POPUPSQUEUE*)mir_alloc(sizeof(struct POPUPSQUEUE)) ;
	struct HISTORIA *elem = getEl(id);
	assert(nowy);
	nowy->hWnd = hWnd;
	nowy->next = elem->pq;
	elem->pq = nowy;
}


//clear popups handles list
void deletePopupsHandles(struct POPUPSQUEUE **firstpq, BOOL closePopup)
{
	struct POPUPSQUEUE *curpq = *firstpq, *delpq;
	while(curpq != NULL) {
		delpq = curpq;
		curpq = curpq->next;
		if(closePopup)
			PUDeletePopup(delpq->hWnd);
		mir_free(delpq);
	}
	*firstpq = NULL;
}


//clear msgs list
void deleteElements()
{
	struct HISTORIA *cur = first, *del;
	while(cur != NULL)
	{
		del = cur;
		cur = cur->next;
		deletePopupsHandles(&(del->pq),FALSE);
		mir_free(del);
		first = cur;
	}
	first = NULL;
}


//set plugin name
void init_pluginname()
{
	char text[MAX_PATH], *p, *q;
    WIN32_FIND_DATAA ffd;
    HANDLE hFind;

    // Try to find name of the file having original letter sizes
	GetModuleFileNameA(hInst, text, sizeof(text));

    if((hFind = FindFirstFileA(text, &ffd)) != INVALID_HANDLE_VALUE) {
        strncpy_s(text, SIZEOF(text), ffd.cFileName, strlen(ffd.cFileName));
        FindClose(hFind);
    }
    // Check if we have relative or full path
	if(p = strrchr(text, '\\')) {
        p++;
	} else {
        p = text;
	}
	if(q = strrchr(p, '.')) {
		*q = '\0';
	}
	if((q = strstr(p, "debug")) && strlen(q) == 5) {
		*q = '\0';
	}

	// copy to static variable
	strncpy_s(PLUGINNAME, SIZEOF(PLUGINNAME), p, strlen(p));
	assert(strlen(PLUGINNAME)>0);

}


BOOL strrep(char *src, char *needle, char *newstring)
{
	char *found, begining[MAX_SETTING_STR], tail[MAX_SETTING_STR];
	int pos=0;

	//strset(begining,' ');
	//strset(tail,' ');
	if(!(found=strstr(src,needle)))
		return FALSE;

	pos = (int)(found-src);
	strncpy_s(begining, _countof(begining), src, pos);
	begining[pos]='\0';

	pos = pos+(int)strlen(needle);
	strncpy_s(tail, _countof(tail), src+pos, SIZEOF(tail));
	begining[pos]='\0';

	pos = sprintf(src, "%s%s%s", begining, newstring, tail); //!!!!!!!!!!!!!!!!
	return TRUE;
}


//check if given string contain filter string
//param field=	0-sender
//				1-subject
BOOL checkFilters(TCHAR* str, int field)
{
	TCHAR buff[512] = _T("");
	TCHAR *strptr = NULL;
	switch(field) {
	case 0:
		_tcsncpy_s(buff, settingFilterSender, _TRUNCATE);
		break;
	case 1:
		_tcsncpy_s(buff, settingFilterSubject, _TRUNCATE);
		break;
	case 2:
		_tcsncpy_s(buff, settingFilterTo, _TRUNCATE);
		break;
	}


	while(strptr = _tcschr(buff, ';'))
	{
		TCHAR tmp[512] = TEXT(""), *ptr;
		_tcsncpy_s(tmp, buff, (strptr-buff));
		_tcsncpy_s(buff, strptr + 1, _TRUNCATE);

		if(_tcsstr(_tcslwr(ptr=_tcsdup(str)),_tcslwr(tmp)))
		{
			free(ptr);
			return TRUE;
		}
		free(ptr);
	}
	return FALSE;
}


//subfunction called from popup plugin callback function
void Click(HWND hWnd,BOOL execute)
{
	POPUPATT *pid=NULL;
	pid = (POPUPATT*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)pid);
	if(settingOnceOnly&&settingNonClickedOnly)
		(getEl(pid->id))->clicked=TRUE;//add to msgs list

	deletePopupsHandles((&(getEl(pid->id))->pq),TRUE);

	if(settingNewest && (pid->id > settingNewestID) ){
		db_set_dw(NULL, PLUGINNAME, "LNNewestID", settingNewestID=pid->id);
	}
	if(execute && settingCommand && strlen(settingCommand)>0 ) {
		char tmpcommand[2*MAX_SETTING_STR];
		char tmpparameters[2*MAX_SETTING_STR];
		strncpy_s(tmpcommand, SIZEOF(tmpcommand), settingCommand, SIZEOF(tmpcommand));
		strncpy_s(tmpparameters, SIZEOF(tmpparameters), settingParameters, SIZEOF(tmpparameters));
		strrep(tmpcommand, "%OID%", pid->strNote);
		strrep(tmpparameters, "%OID%", pid->strNote);
		log_p(L"executing: %S %S", tmpcommand, tmpparameters);
		ShellExecuteA(NULL, "Open", tmpcommand, tmpparameters, NULL, SW_NORMAL);
	}
}


//popup plugin callback function
static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
		{

			if (HIWORD(wParam) == STN_CLICKED)//client clicked on popup with left mouse button
			{
				Click(hWnd,TRUE);

				//system(settingCommand);
				//if(!settingOnceOnly)
					//addNewId(noteID);
				return TRUE;
			}

			break;
		}

		case WM_RBUTTONUP:
		{
			Click(hWnd,FALSE);
			break;
		}

		case UM_INITPOPUP:
		{
			POPUPATT *pid=NULL;
			pid = (POPUPATT*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)pid);
			addPopup(pid->id,hWnd);
			//PUDeletePopUp(hWnd);
			break;
		}

		case UM_FREEPLUGINDATA:
		{
			POPUPATT * mpd = NULL;
			mpd = (POPUPATT*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
			if (mpd > 0) free(mpd);
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}

		default:
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


//check notes.ini if it has entry about our plugin
//return TRUE if notes.ini is set correctly
//give bInfo=TRUE to show MsgBoxes
BOOL checkNotesIniFile(BOOL bInfo)
{
	char tmp[MAXENVVALUE+1], tmp1[MAXENVVALUE+1];
	(OSGetEnvironmentString1) ("EXTMGR_ADDINS", tmp, MAXENVVALUE);//get current setting
	strncpy_s(tmp1,_countof(tmp1),tmp,sizeof(tmp1));//copy temporary
	assert(strlen(tmp1)>0);

	char* PLUGINNAME_lower = _strlwr(mir_strdup(PLUGINNAME));

	//is there our plugin as safe?
	if(strstr(tmp1,PLUGINNAME_lower) == NULL)
	{
		if(!settingIniCheck && !bInfo)
			return FALSE;

		if(!settingIniAnswer || bInfo){
			switch(MessageBox(NULL, TranslateT("This utility check your notes.ini file if it's set to authenticate this plugin as safe. Plugin is not added as Lotus Extension, so plugin built-in authentication will not work properly. Do you want to add plugin as Lotus Extension (modify notes.ini by adding \"EXTMGR_ADDINS=PLUGINNAME\")?"), TranslateT("LotusNotify plugin configuration"), MB_YESNO))
			{
				case IDYES:
				{
					settingIniAnswer=1;
					break;
				}
				case IDNO:
				{
					settingIniAnswer=-1;
					break;
				}
			}
		}

		if(settingIniAnswer == 1)
		{
			if(strlen(tmp) > 0) {
				strcat_s(tmp, SIZEOF(tmp), ",");
				strcat_s(tmp, SIZEOF(tmp), PLUGINNAME_lower); //add our plugin to extensions
			} else {
				strncpy_s(tmp, SIZEOF(tmp), PLUGINNAME_lower, strlen(PLUGINNAME_lower)); //set our plugin as extension
			}

			(OSSetEnvironmentVariable1) ("EXTMGR_ADDINS", tmp); //set notes.ini entry
			if(bInfo) {
				MessageBox(NULL, TranslateT("notes.ini modified correctly. Miranda restart required."), TranslateT("LotusNotify plugin configuration"), MB_OK);
			} else{
				ErMsgT(TranslateT("notes.ini modified correctly. Miranda restart required."));
			}
			return TRUE;
		} else { if(settingIniAnswer == 0xFF)
			return FALSE;
		}
	} else {
		//setting set already
		if(bInfo)
			MessageBox(NULL, TranslateT("notes.ini seem to be set correctly."), TranslateT("LotusNotify plugin configuration"), MB_OK);
		return TRUE;
	}

	mir_free(PLUGINNAME_lower);
	return FALSE;
}


//popup plugin to show popup function
void showMsg(TCHAR* sender,TCHAR* text, DWORD id, char *strUID)
{

	POPUPDATAT ppd;
	//hContact = A_VALID_HANDLE_YOU_GOT_FROM_SOMEWHERE;
	//hIcon = A_VALID_HANDLE_YOU_GOT_SOMEWHERE;
	//char * lpzContactName = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)lhContact,0);
	//99% of the times you'll just copy this line.
	//1% of the times you may wish to change the contact's name. I don't know why you should, but you can.
	//char * lpzText;
	//The text for the second line. You could even make something like: char lpzText[128]; lstrcpy(lpzText, "Hello world!"); It's your choice.

	POPUPATT * mpd = (POPUPATT*)malloc(sizeof(POPUPATT));
	ZeroMemory(&ppd, sizeof(ppd)); //This is always a good thing to do.
	ppd.lchContact = NULL; //(HANDLE)hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	_tcscpy_s(ppd.lptzContactName, SIZEOF(ppd.lptzContactName), sender);
	_tcscpy_s(ppd.lptzText, SIZEOF(ppd.lptzText), text);
	if(settingSetColours)
	{
		ppd.colorBack = settingBgColor;
		ppd.colorText = settingFgColor;
	}
	ppd.PluginWindowProc = PopupDlgProc;

	ppd.iSeconds=settingInterval1;
	//Now the "additional" data.
	mpd->id = id;
	strncpy_s(mpd->strNote, SIZEOF(mpd->strNote), strUID, strlen(strUID));
	//mpd->newStatus = ID_STATUS_ONLINE;

	//Now that the plugin data has been filled, we add it to the PopUpData.
	ppd.PluginData = mpd;

	//Now that every field has been filled, we want to see the popup.
	PUAddPopupT(&ppd);
}


//what to do with error msg
void ErMsgW(WCHAR* msg)
{
	TCHAR* msgT = mir_u2t(msg);
	ErMsgT(msgT);
	mir_free(msgT);
}
///TODO TCHAR->WCHAR and test
void ErMsgT(TCHAR* msg)
{
	log_p(L"Error: %S", msg);
	if(settingShowError && !isPopupWaiting) {
		TCHAR buffer[256+14];
		_tcsncpy_s(buffer, _T("LotusNotify: "), _TRUNCATE);
		_tcscat_s(buffer, msg);
		isPopupWaiting = TRUE;
		PUShowMessageT(buffer, SM_WARNING);
		isPopupWaiting = FALSE;
	}
}


//Lotus error occured so translate it
void ErMsgByLotusCode(STATUS erno)
{
	char far error_text_LMBCS[200];
	char far error_text_UNICODEatCHAR[400];
	WCHAR far error_text_UNICODE[200];
    WORD text_len;

    text_len = (OSLoadString1)(NULLHANDLE, erno, error_text_LMBCS, sizeof(error_text_LMBCS)-1);
	(OSTranslate1)(OS_TRANSLATE_LMBCS_TO_UNICODE, error_text_LMBCS, (WORD)strlen(error_text_LMBCS), error_text_UNICODEatCHAR, sizeof(error_text_UNICODEatCHAR)-1);
	memcpy(error_text_UNICODE, error_text_UNICODEatCHAR, sizeof(error_text_UNICODE));

	ErMsgW(error_text_UNICODE);
}


//set menu avainability
static void LNEnableMenuItem(HANDLE hMenuItem, BOOL bEnable)
{
	log_p(L"LNEnableMenuItem: bEnable=%d", bEnable);
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS;
	if ( !bEnable )
		clmi.flags |= CMIF_GRAYED;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&clmi);
}


int check() {

	log_p(L"check: Entering check function. running=%d", running);

	if(startuperror) {
		int cnt;
		for(cnt = 0; cnt <= 4; cnt++)
			if(startuperror >> cnt & 1)
				ErMsgT(TranslateTS(startuperrors[cnt]));
		return 1;
	}

	if (Plugin_Terminated || Miranda_Terminated()){
		log_p(L"check: Plugin_Terminated (=%d) OR Miranda_Terminated()", Plugin_Terminated);
		return 0;
	}

	if(running) {
		ErMsgT(TranslateT("Now checking Lotus, try again later"));
		return 1;
	} else {
		running = TRUE;
		LNEnableMenuItem(hMenuHandle, !running);
	}

	log(L"check: starting checkthread");
	mir_forkthread(checkthread, NULL);

	return 0;
}



//before pure lotus notes api functions call
void checkthread(void*)
{

	STATUS      error = NOERROR;
	char		fullpath[255];
	DBHANDLE    db_handle = NULLHANDLE;     /* database handle */
    TCHAR       buffer[NSF_INFO_SIZE] = TEXT(""); /* database info buffer */
    TCHAR       title[NSF_INFO_SIZE] = TEXT("");  /* database title */
	char        UserName [MAXUSERNAME + 1];
	HANDLE      hTable;

	DWORD		noteID = 0L;
	BOOL		fFirst = TRUE;

	NOTEHANDLE	note_handle;
	WORD        field_len;
	char        field_date[MAXALPHATIMEDATE + 1];

	char        field_lotus_LMBCS[MAX_FIELD];
	char        field_lotus_UNICODEatCHAR[MAX_FIELD * sizeof(TCHAR)];
	WCHAR       field_from_UNICODE[MAX_FIELD];
	WCHAR       field_subject_UNICODE[MAX_FIELD];
	WCHAR       field_to_UNICODE[MAX_FIELD];
	WCHAR       field_copy_UNICODE[MAX_FIELD];

	EnterCriticalSection(&checkthreadCS);
	log(L"checkthread: inside new check thread");

	if (error = (NotesInitThread1)()) {
		goto errorblock;
	}
	log(L"checkthread: Started NotesInitThread");

	if (error = (OSPathNetConstruct1)(NULL, settingServer, settingDatabase, fullpath)) {
		goto errorblock;
	}
	log_p(L"checkthread: OSPathNetConstruct: %S", fullpath);

	if (error = (NSFDbOpen1) (fullpath, &db_handle)) {
		if(strcmp(settingServerSec, "") != 0) {
			if (error = (OSPathNetConstruct1)(NULL, settingServerSec, settingDatabase, fullpath)) {
				goto errorblock;
			} else {
				if (error = (NSFDbOpen1) (fullpath, &db_handle)) {
					goto errorblock;
				}
			}
		} else {
			goto errorblock;
		}
	}
	assert(db_handle);
	log(L"checkthread: DBOpened");

	if(error = (SECKFMGetUserName1) (UserName)) {
		goto errorblock0;
	}
	assert(UserName);
	log_p(L"checkthread: Username: %S", UserName);

   /* Get the unread list */
	if(error = (NSFDbGetUnreadNoteTable1) (db_handle,UserName,(WORD) strlen(UserName),TRUE,&hTable)) {
		goto errorblock0;
	}
	log(L"checkthread: Unread Table got");

	//error = IDTableCopy (hTable, &hOriginalTable);
	//IDDestroyTable (hTable);
	if(error = (NSFDbUpdateUnread1) (db_handle, hTable)){
		goto errorblock;
	}
	log(L"checkthread: Unread Table updated");
	assert(hTable);


    while((IDScan1)(hTable, fFirst, &noteID)) {

		WORD Att;
		BLOCKID bhAttachment;
		DWORD cSize = 0;
		DWORD attSize = 0;
		OID          retNoteOID;
		TIMEDATE     retModified;     /* modified timedate      */
		WORD         retNoteClass;    /* note class             */
		TIMEDATE     sendDate;
		char strLink[4*16];

		if (Plugin_Terminated || Miranda_Terminated()){
			log_p(L"checkthread: Plugin_Terminated (=%d) OR Miranda_Terminated()", Plugin_Terminated);
			break;
		}

		log_p(L"checkthread: Getting info about: %d", noteID);

		fFirst = FALSE;
		assert(noteID);
		if(!getEl(noteID))
			addNewId(noteID);
		else
			(getEl(noteID))->again = TRUE;

		if(!settingOnceOnly && (getEl(noteID))->again == TRUE){
			//don't show again and note was not showed (ID not on list)
			continue;
		}

		log(L"checkthread: skiped-don't show again and note was not showed (ID not on list)");

		if(settingOnceOnly && settingNonClickedOnly && (getEl(noteID))->clicked == TRUE){
			//show again, but only not clicked (id added to list on Left Button click)
			continue;
		}

		log(L"checkthread: skiped-show again, but only not clicked (id added to list on Left Button click)");

		if(settingNewest && settingNewestID >= noteID){
			//only newest option enabled, so if old id don't show it
			continue;
		}

		log(L"checkthread: skiped-only newest option enabled, so if old id don't show it");

		//if(((!settingOnceOnly||(settingOnceOnly&&settingNonClickedOnly))&&existElem(noteID))||(settingNewest&&settingNewestID>=noteID))
			//continue;

	    if (error = (NSFNoteOpen1) (db_handle, noteID, 0, &note_handle)) {
			continue;
		}

		log_p(L"checkthread: Opened Note: %d", noteID);

		(NSFDbGetNoteInfo1)(db_handle,      /* DBHANDLE */
						  noteID,			/* NOTEID   */
						  &retNoteOID,		/* out: OID */
						  &retModified,		/* out:     */
						  &retNoteClass) ;
		ZeroMemory(strLink, SIZEOF(strLink));
		mir_snprintf(strLink, SIZEOF(strLink), "%.8lX%.8lX%.8lX%.8lX",
						retNoteOID.File.Innards[1],
						retNoteOID.File.Innards[0],
						retNoteOID.Note.Innards[1],
						retNoteOID.Note.Innards[0]
					);

		log_p(L"checkthread: got noteInfo, built link: %S", strLink);

		field_len = (NSFItemGetText1) (note_handle, MAIL_FROM_ITEM, field_lotus_LMBCS, (WORD)sizeof(field_lotus_LMBCS));
		(OSTranslate1)(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_from_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(TCHAR));
		field_from_UNICODE[field_len] = '\0';

		(NSFItemGetTime1) (note_handle,MAIL_POSTEDDATE_ITEM,&sendDate);
		error = (ConvertTIMEDATEToText1)(NULL, NULL, &sendDate, field_date, MAXALPHATIMEDATE, &field_len);
		field_date[field_len] = '\0';

		field_len = (NSFItemGetText1) (note_handle, MAIL_SUBJECT_ITEM, field_lotus_LMBCS, (WORD)sizeof(field_lotus_LMBCS));
		(OSTranslate1)(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_subject_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(TCHAR));
		field_subject_UNICODE[field_len] = '\0';

		field_len = (NSFItemGetText1) (note_handle, MAIL_SENDTO_ITEM, field_lotus_LMBCS, (WORD)sizeof (field_lotus_LMBCS));
		(OSTranslate1)(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_to_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(TCHAR));
		field_to_UNICODE[field_len] = '\0';

		field_len = (NSFItemGetText1) (note_handle, MAIL_COPYTO_ITEM, field_lotus_LMBCS, (WORD)sizeof (field_lotus_LMBCS));
		(OSTranslate1)(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_copy_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(TCHAR));
		field_copy_UNICODE[field_len] = '\0';


		WCHAR msgFrom[512];
		WCHAR msgSubject[512];
		ZeroMemory(msgFrom,512);
		ZeroMemory(msgSubject,512);

		if(wcslen(field_from_UNICODE) < 512 && wcslen(field_from_UNICODE) > 3 && wcsstr(field_from_UNICODE, L"CN=") == field_from_UNICODE)
			_tcsncpy_s(msgFrom, &(field_from_UNICODE[3]), wcscspn(field_from_UNICODE, L"/")-3 );
		else
			_tcsncpy_s(msgFrom, field_from_UNICODE, _TRUNCATE);

	    for (Att = 0; (MailGetMessageAttachmentInfo1)(note_handle, Att,&bhAttachment, NULL, &cSize, NULL, NULL, NULL, NULL); Att++)
			attSize += cSize;

		#ifdef _DEBUG
		log_p(L"checkthread: MAIL INFO: date=[%S], from=[%s], to=[%s], cc=[%s], sub=[%s], attSize=[%d]"
			,field_date
			,field_from_UNICODE
			,field_to_UNICODE
			,field_copy_UNICODE
			,field_subject_UNICODE
			,attSize
		);
		#else
		//do not put private user data into log
		log_p(L"checkthread: MAIL INFO (sizes): date=[%S], from=[%d], to=[%d], cc=[%d], sub=[%d], attSize=[%d]"
			,field_date
			,wcslen(field_from_UNICODE)
			,wcslen(field_to_UNICODE)
			,wcslen(field_copy_UNICODE)
			,wcslen(field_subject_UNICODE)
			,attSize
		);
		#endif


		if(attSize){
			WCHAR field_attachments_UNICODE[MAX_FIELD];
			mir_sntprintf(field_attachments_UNICODE, SIZEOF(field_attachments_UNICODE), TranslateW(L"Attachments: %d bytes"), attSize);
			mir_sntprintf(msgSubject, SIZEOF(msgSubject), L"%S\n%s\n%s", field_date, field_subject_UNICODE, field_attachments_UNICODE );
		} else {
			mir_sntprintf(msgSubject, SIZEOF(msgSubject), L"%S\n%s", field_date, field_subject_UNICODE );
		}

		//check if this is not filtered msg
		if(    ! checkFilters(field_from_UNICODE, 0)
			&& ! checkFilters(field_subject_UNICODE, 1)
			&& ! checkFilters(field_to_UNICODE, 2)
			&& ! checkFilters(field_copy_UNICODE, 2))
		{
			log(L"checkthread: filters checked - positive");
			///TODO eliminate popups with blank fields
			showMsg(msgFrom,msgSubject,noteID,strLink);
			SkinPlaySound("LotusNotify");
		} else {
			log(L"checkthread: filters checked - negative");
		}

		if (error = (NSFNoteClose1) (note_handle)) {
			continue;
		}
		log_p(L"checkthread: Close note id: %d", noteID);

    }

	if(error = (IDDestroyTable1) (hTable)){
		goto errorblock0;
	}
	log(L"checkthread: Table destroyed");

	if (error = (NSFDbClose1) (db_handle)){
		goto errorblock;
	}
	log(L"checkthread: DB closed");

	//NotesTerm();
	(NotesTermThread1)();

	log(L"checkthread: Terminating Notes thread");
	running = FALSE;
	if(currentStatus != ID_STATUS_OFFLINE){
		LNEnableMenuItem(hMenuHandle, !running);
	}
	LeaveCriticalSection(&checkthreadCS);
	return;

errorblock0:
	log(L"checkthread: errorblock0");
	(NSFDbClose1) (db_handle);
errorblock:
	log_p(L"checkthread: errorblock. error=%d", error);
	ErMsgByLotusCode(error);
	//NotesTerm();
	//if(currentStatus!=ID_STATUS_OFFLINE)
	//LNEnableMenuItem(hMenuHandle,!running);
	//SetStatus(ID_STATUS_OFFLINE,0);
	running = FALSE;
	LeaveCriticalSection(&checkthreadCS);
	return;
}


//hooked notification from service that listning to check lotus
static int eventCheck(WPARAM wParam,LPARAM lParam)
{
	log(L"check event...");
	check();
	return 0;
}


//on click to menu callback function
INT_PTR PluginMenuCommand(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hCheckEvent, wParam, lParam); //create event to check lotus
	return 0;
}


//window timer callback function, called on timer event
VOID CALLBACK atTime(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	log(L"atTime: start");
	BOOL b = KillTimer(hTimerWnd, idEvent);
	if(currentStatus != ID_STATUS_OFFLINE) {
		//if status lets to check
		check();
		if(settingInterval != 0){
			log_p(L"atTime: SetTimer settingInterval=%d", settingInterval * 60000);
			SetTimer(hTimerWnd, TID, settingInterval * 60000, (TIMERPROC)atTime);
		}
	}
}


void decodeServer(char *tmp)
{
	if (strstr(tmp,"CN=") && strstr(tmp,"OU=") && strstr(tmp,"O=")) {
		//if lotus convention
		while(strrep(tmp,"CN=",""));
		while(strrep(tmp,"OU=",""));
		while(strrep(tmp,"O=",""));
	}
}


//fill combo in options dlgbox with all known servers
void fillServersList(HWND hwndDlg)
{
	HANDLE    hServerList=NULLHANDLE;
	BYTE far *pServerList;            /* Pointer to start of Server List */
	WORD      wServerCount;           /* Number of servers in list. */
	WORD far *pwServerLength;         /* Index to array of servername lens */
	BYTE far *pServerName;
	STATUS    error = NOERROR;        /* Error return from API routines. */
	char      ServerString[MAXPATH];  /* String to hold server names.   */
	LPSTR     szServerString = ServerString;
	USHORT i;

	if(!hLotusDll) {
		return;
	}

	error = (NSGetServerList1)(NULL, &hServerList);
	if (error == NOERROR) {

		pServerList  = (BYTE far *)(OSLockObject1)(hServerList);
		wServerCount = (WORD) *pServerList;

		pwServerLength = (WORD *)(pServerList + sizeof(WORD));

		pServerName = (BYTE far *) pServerList + sizeof(wServerCount) +((wServerCount) * sizeof(WORD));

		for (i=0; i<wServerCount; pServerName+=pwServerLength[i], i++)
		{
			memmove (szServerString, pServerName, pwServerLength[i]);
			szServerString[pwServerLength[i]] = '\0';
			decodeServer(ServerString);
			SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)szServerString);
		}
		(OSUnlockObject1) (hServerList);
		(OSMemFree1) (hServerList);

	} else {
		ErMsgByLotusCode(error);
	}

	return;
}


//gets default settings from notes.ini file
void lookupLotusDefaultSettings(HWND hwndDlg)
{
	char tmp[MAXENVVALUE+1];
	// Get the info from the .ini file
	if(hLotusDll) {
		if ((OSGetEnvironmentString1) ("MailFile", tmp, MAXENVVALUE)) //path to mail file
			SetDlgItemTextA(hwndDlg, IDC_DATABASE, tmp); //and set fields in opt. dialog
		if ((OSGetEnvironmentString1) ("MailServer", tmp, MAXENVVALUE)) //server name
		{
			decodeServer(tmp);
			SetDlgItemTextA(hwndDlg, IDC_SERVER, tmp);
		}
	}

}

//get variables values stored in db.
void LoadSettings()
{
	DBVARIANT dbv;
	char buff[128];
	int i=0;

	settingInterval = (INT)db_get_dw(NULL, PLUGINNAME, "LNInterval", 15);
	settingInterval1 = (INT)db_get_dw(NULL, PLUGINNAME, "LNInterval1", 0);
	//if(!db_get(NULL, PLUGINNAME, "LNInterval1",&dbv))
	//	settingInterval1 = int(dbv.lVal);
	//db_free(&dbv);
	//settingInterval=dbv.lVal;
	if(!db_get_s(NULL, PLUGINNAME, "LNDatabase", &dbv))
		strncpy_s(settingDatabase, _countof(settingDatabase), dbv.pszVal, SIZEOF(settingDatabase));
	db_free(&dbv);
	if(!db_get_s(NULL, PLUGINNAME, "LNServer", &dbv))
		strncpy_s(settingServer, _countof(settingServer), dbv.pszVal, SIZEOF(settingServer));
	db_free(&dbv);
	if(!db_get_s(NULL, PLUGINNAME, "LNServerSec", &dbv))
		strncpy_s(settingServerSec, _countof(settingServerSec), dbv.pszVal, SIZEOF(settingServerSec));
	db_free(&dbv);
	if(!db_get(NULL, PLUGINNAME, "LNPassword", &dbv))
		strncpy_s(settingPassword, _countof(settingPassword), dbv.pszVal, SIZEOF(settingPassword));
	db_free(&dbv);
	if(!db_get_s(NULL, PLUGINNAME, "LNCommand", &dbv, DBVT_ASCIIZ))
		strncpy_s(settingCommand, _countof(settingCommand), dbv.pszVal, SIZEOF(settingCommand));
	db_free(&dbv);
	if(!db_get_s(NULL, PLUGINNAME, "LNParameters", &dbv, DBVT_ASCIIZ))
		strncpy_s(settingParameters, _countof(settingParameters), dbv.pszVal, SIZEOF(settingParameters));
	db_free(&dbv);

	if(!db_get_ts(NULL, PLUGINNAME, "LNFilterSender",&dbv))
		_tcsncpy_s(settingFilterSender, dbv.ptszVal, _TRUNCATE);
	db_free(&dbv);
	if(!db_get_ts(NULL, PLUGINNAME, "LNFilterSubject",&dbv))
		_tcsncpy_s(settingFilterSubject, dbv.ptszVal, _TRUNCATE);
	db_free(&dbv);
	if(!db_get_ts(NULL, PLUGINNAME, "LNFilterTo",&dbv))
		_tcsncpy_s(settingFilterTo, dbv.ptszVal, _TRUNCATE);
	db_free(&dbv);

	settingOnceOnly = db_get_b  (NULL, PLUGINNAME, "LNOnceOnly",0);

	settingNonClickedOnly = db_get_b(NULL, PLUGINNAME, "LNNonClickedOnly", 1);
	settingShowError = db_get_b(NULL, PLUGINNAME, "LNShowError", 1);
	settingSetColours = db_get_b(NULL, PLUGINNAME, "LNSetColours", 0);
	settingBgColor = (COLORREF)db_get_dw(NULL, PLUGINNAME, "LNBgColor", (DWORD)0xFFFFFF);
	settingFgColor = (COLORREF)db_get_dw(NULL, PLUGINNAME, "LNFgColor", (DWORD)0x000000);
	settingNewest = db_get_b(NULL, PLUGINNAME, "LNNewest", 0);
	settingNewestID = (DWORD)db_get_dw(NULL, PLUGINNAME, "LNNewestID", 0);
	settingIniAnswer = db_get_b(NULL, PLUGINNAME, "LNIniAnswer", 0);
	settingIniCheck = db_get_b(NULL, PLUGINNAME, "LNIniCheck", 0);

	for(i = 0; i < STATUS_COUNT; i++) {
		mir_snprintf(buff, SIZEOF(buff), "LNStatus%d", i);
		settingStatus[i] = (db_get_b(0, PLUGINNAME, buff, 0) == 1);
	}
	//lookupLotusDefaultSettings();
}


//callback function to speak with user interactions in options page
INT_PTR CALLBACK DlgProcLotusNotifyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList;
	switch(msg)
		{
			case WM_INITDIALOG://initialize dialog, so set properties from db.
				{
				TranslateDialogDefault(hwndDlg);//translate miranda function
				TCHAR buffa[256];
				mir_sntprintf(buffa, SIZEOF(buffa), _T("%d.%d.%d.%d"), HIBYTE(HIWORD(pluginInfo.version)), LOBYTE(HIWORD(pluginInfo.version)), HIBYTE(LOWORD(pluginInfo.version)), LOBYTE(LOWORD(pluginInfo.version)));
				SetDlgItemText(hwndDlg, IDC_VERSION, buffa);
				LoadSettings();
				SetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase);
				SetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer);
				SetDlgItemTextA(hwndDlg, IDC_SERVERSEC, settingServerSec);
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, settingPassword);
				SetDlgItemInt(hwndDlg, IDC_INTERVAL, settingInterval,FALSE);
				SetDlgItemInt(hwndDlg, IDC_INTERVAL1, settingInterval1,TRUE);
				SetDlgItemTextA(hwndDlg, IDC_COMMAND, settingCommand);
				SetDlgItemTextA(hwndDlg, IDC_PARAMETERS, settingParameters);
				CheckDlgButton(hwndDlg, IDC_ONCEONLY, settingOnceOnly);
				CheckDlgButton(hwndDlg, IDC_SHOWERROR, settingShowError);
				CheckDlgButton(hwndDlg, IDC_NEWEST, settingNewest);
				CheckDlgButton(hwndDlg, IDC_NONCLICKEDONLY, settingNonClickedOnly);
				CheckDlgButton(hwndDlg, IDC_BUTTON_CHECK, settingIniCheck );

				if(!settingOnceOnly) {
					HWND hwnd = GetDlgItem(hwndDlg, IDC_NONCLICKEDONLY);
					EnableWindow(hwnd, FALSE);
				}
				CheckDlgButton(hwndDlg, IDC_SETCOLOURS, settingSetColours ? TRUE : FALSE);
				SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingBgColor);
				SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingFgColor);
				if(!settingSetColours) {
					HWND hwnd = GetDlgItem(hwndDlg, IDC_BGCOLOR);
					CheckDlgButton(hwndDlg, IDC_SETCOLOURS, FALSE);
					EnableWindow(hwnd, FALSE);
					hwnd = GetDlgItem(hwndDlg, IDC_FGCOLOR);
					EnableWindow(hwnd, FALSE);
				}


				//SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_SELECTSTRING ,-1,(LPARAM)(LPCSTR)settingServer);

			// initialise and fill listbox
				hwndList = GetDlgItem(hwndDlg, IDC_STATUS);
				ListView_DeleteAllItems(hwndList);

				SendMessage(hwndList,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

				// Initialize the LVCOLUMN structure.
				// The mask specifies that the format, width, text, and
				// subitem members of the structure are valid.
				LVCOLUMN lvc={0};
				lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
				lvc.fmt = LVCFMT_LEFT;

				lvc.iSubItem = 0;
				lvc.pszText = TranslateT("Status");
				lvc.cx = 120;     // width of column in pixels
				ListView_InsertColumn(hwndList, 0, &lvc);

				// Some code to create the list-view control.
				// Initialize LVITEM members that are common to all items.
				LVITEM lvI={0};
				lvI.mask = LVIF_TEXT;
				for(int i = 0; i < STATUS_COUNT; i++) {
					lvI.pszText =  (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, ID_STATUS_ONLINE + i, GSMDF_TCHAR);
					lvI.iItem = i;
					ListView_InsertItem(hwndList, &lvI);
					ListView_SetCheckState(hwndList, i, settingStatus[i]);
				}

				//fill filter combos
				TCHAR buff[512];
				TCHAR* strptr;

				_tcsncpy_s(buff, settingFilterSender, _TRUNCATE);
				while(strptr = _tcschr(buff, TEXT(';'))) {
					TCHAR tmp[512] = TEXT("");
					_tcsncpy_s(tmp, buff, (strptr-buff));
					SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER , CB_ADDSTRING, 0, (LPARAM)tmp);
					_tcsncpy_s(buff, strptr + 1, _TRUNCATE);
				}

				_tcsncpy_s(buff, settingFilterSubject, _TRUNCATE);
				while(strptr = _tcschr(buff, TEXT(';'))) {
					TCHAR tmp[512] = TEXT("");
					_tcsncpy_s(tmp, buff, (strptr-buff));
					SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT , CB_ADDSTRING, 0, (LPARAM)tmp);
					_tcsncpy_s(buff, strptr + 1, _TRUNCATE);
				}

				_tcsncpy_s(buff, settingFilterTo, _TRUNCATE);
				while(strptr = _tcschr(buff, TEXT(';'))) {
					TCHAR tmp[512] = TEXT("");
					_tcsncpy_s(tmp, buff, (strptr-buff));
					SendDlgItemMessage(hwndDlg, IDC_FILTER_TO , CB_ADDSTRING, 0, (LPARAM)tmp);
					_tcsncpy_s(buff, strptr + 1, _TRUNCATE);
				}

				break;
				}
			case WM_COMMAND://user changed something, so get changes to variables
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				switch(LOWORD(wParam))
					{
						case IDC_DATABASE: GetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase, sizeof(settingDatabase)); break;
						case IDC_SERVER:
							{
								switch(HIWORD(wParam))
								{
									case CBN_SELCHANGE:
										{
											int i;
											char text[MAXENVVALUE];
											i=SendDlgItemMessage(hwndDlg,IDC_SERVER,CB_GETCURSEL,0,0);
											SendDlgItemMessageA(hwndDlg,IDC_SERVER,CB_GETLBTEXT,(WPARAM)i,(LONG)/* (LPSTR) */text);
											SetDlgItemTextA(hwndDlg,IDC_SERVER,text);
											break;
										}

									//case CBN_EDITCHANGE:

									case CBN_DROPDOWN:
										{
											SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_RESETCONTENT ,0, 0);
											fillServersList(hwndDlg);
											SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)settingServer);
											SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_SELECTSTRING , -1, (LPARAM)(LPCSTR)settingServer);
											break;
										}
								}
							}
						case IDC_SERVERSEC: GetDlgItemTextA(hwndDlg, IDC_SERVERSEC, settingServerSec, sizeof(settingServerSec)); break;
						case IDC_PASSWORD: GetDlgItemTextA(hwndDlg, IDC_PASSWORD, settingPassword, sizeof(settingPassword)); break;

						case IDC_INTERVAL: settingInterval =GetDlgItemInt(hwndDlg, IDC_INTERVAL, NULL, FALSE); break;
						case IDC_INTERVAL1: settingInterval1 =GetDlgItemInt(hwndDlg, IDC_INTERVAL1, NULL, TRUE); break;
						case IDC_COMMAND: GetDlgItemTextA(hwndDlg, IDC_COMMAND, settingCommand, sizeof(settingCommand)); break;
						case IDC_PARAMETERS: GetDlgItemTextA(hwndDlg, IDC_PARAMETERS, settingParameters, sizeof(settingParameters)); break;
						case IDC_ONCEONLY:
							{
								HWND hwnd = GetDlgItem(hwndDlg, IDC_NONCLICKEDONLY);
								settingOnceOnly=(BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONCEONLY);
								EnableWindow(hwnd, settingOnceOnly);
								break;
							}

						case IDC_NONCLICKEDONLY: settingNonClickedOnly=(BYTE) IsDlgButtonChecked(hwndDlg, IDC_NONCLICKEDONLY); break;
						case IDC_NEWEST: settingNewest =(BYTE) IsDlgButtonChecked(hwndDlg, IDC_NEWEST); break;
						case IDC_SHOWERROR: settingShowError=(BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWERROR); break;

						case IDC_SETCOLOURS:
						{
							HWND hwnd = GetDlgItem(hwndDlg, IDC_BGCOLOR);
							settingSetColours=IsDlgButtonChecked(hwndDlg, IDC_SETCOLOURS);
							EnableWindow(hwnd,settingSetColours );
							hwnd = GetDlgItem(hwndDlg, IDC_FGCOLOR);
							EnableWindow(hwnd, settingSetColours);
							break;
						}
						case IDC_BGCOLOR: settingBgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);break;
						case IDC_FGCOLOR: settingFgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_GETCOLOUR, 0, 0);break;
						case IDC_BUTTON_DETECT:
							{
								lookupLotusDefaultSettings(hwndDlg);
								GetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer, sizeof(settingServer));
								GetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase, sizeof(settingDatabase));
								break;
							}
						case IDC_BUTTON_CLEAR:
							{
								deleteElements();
								break;
							}
						case IDC_BUTTON_CHECK:
							{
								settingIniCheck = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BUTTON_CHECK);
								checkNotesIniFile(TRUE);
								break;
							}

						case IDC_BUTTON_ADD_SENDER_FILTER:
							{
								char tmp[255];
								GetDlgItemTextA(hwndDlg, IDC_FILTER_SENDER, tmp, sizeof(tmp));
								SendDlgItemMessageA(hwndDlg, IDC_FILTER_SENDER, CB_ADDSTRING, 0, (LPARAM)tmp);
								break;
							}
						case IDC_BUTTON_REMOVE_SENDER_FILTER:
							{
								SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_DELETESTRING, 0, (LPARAM)SendDlgItemMessage(hwndDlg,IDC_FILTER_SENDER ,CB_GETCURSEL,0,0));
								break;
							}
						case IDC_BUTTON_ADD_SUBJECT_FILTER:
							{
								char tmp[255];
								GetDlgItemTextA(hwndDlg, IDC_FILTER_SUBJECT, tmp, sizeof(tmp));
								SendDlgItemMessageA(hwndDlg, IDC_FILTER_SUBJECT, CB_ADDSTRING, 0, (LPARAM)tmp);
								break;
							}
						case IDC_BUTTON_REMOVE_SUBJECT_FILTER:
							{
								SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_DELETESTRING, 0, (LPARAM)SendDlgItemMessage(hwndDlg,IDC_FILTER_SUBJECT ,CB_GETCURSEL,0,0));
								break;
							}
						case IDC_BUTTON_ADD_TO_FILTER:
							{
								char tmp[255];
								GetDlgItemTextA(hwndDlg, IDC_FILTER_TO, tmp, sizeof(tmp));
								SendDlgItemMessageA(hwndDlg, IDC_FILTER_TO, CB_ADDSTRING, 0, (LPARAM)tmp);
								break;
							}
						case IDC_BUTTON_REMOVE_TO_FILTER:
							{
								SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_DELETESTRING, 0, (LPARAM)SendDlgItemMessage(hwndDlg,IDC_FILTER_SUBJECT ,CB_GETCURSEL,0,0));
								break;
							}


					}
				break;

			case WM_NOTIFY://apply changes so write it to db
				switch(((LPNMHDR)lParam)->idFrom)
				{
					case 0:
					{
						switch (((LPNMHDR)lParam)->code)
						{
							case PSN_RESET:
								LoadSettings();
								return TRUE;
							case PSN_APPLY:

							{
								char buff[128];
								int i=0;
								GetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer, sizeof(settingServer));
								db_set_s(NULL, PLUGINNAME, "LNServer", settingServer );
								db_set_s(NULL, PLUGINNAME, "LNServerSec", settingServerSec);
								db_set_s(NULL, PLUGINNAME, "LNPassword", settingPassword);
								db_set_s(NULL, PLUGINNAME, "LNDatabase", settingDatabase);
								db_set_dw (NULL, PLUGINNAME, "LNInterval", settingInterval);
								db_set_dw (NULL, PLUGINNAME, "LNInterval1", settingInterval1);
								db_set_s(NULL, PLUGINNAME, "LNCommand", settingCommand );
								db_set_s(NULL, PLUGINNAME, "LNParameters", settingParameters);
								db_set_b(NULL, PLUGINNAME, "LNOnceOnly", settingOnceOnly);
								db_set_b(NULL, PLUGINNAME, "LNNonClickedOnly", settingNonClickedOnly);
								db_set_b(NULL, PLUGINNAME, "LNShowError", settingShowError);
								db_set_b(NULL, PLUGINNAME, "LNSetColours", settingSetColours);
								db_set_dw(NULL, PLUGINNAME, "LNBgColor", (DWORD)settingBgColor);
								db_set_dw(NULL, PLUGINNAME, "LNFgColor", (DWORD)settingFgColor);
								db_set_b(NULL, PLUGINNAME, "LNNewest", settingNewest);
								db_set_b(NULL, PLUGINNAME, "LNIniCheck", settingIniCheck);
								db_set_b(NULL, PLUGINNAME, "LNIniAnswer", settingIniAnswer);

								for(i = 0; i < STATUS_COUNT ; i++){
									mir_snprintf(buff, SIZEOF(buff), "LNStatus%d", i);
									settingStatus[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_STATUS), i) ? TRUE : FALSE);
									db_set_b(0, PLUGINNAME, buff, settingStatus[i] ? 1 : 0);
								}

								settingFilterSender[0] = 0;
								for(i=0; i<SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETCOUNT, 0, (LONG)0); i++){
									TCHAR text[512] = TEXT("");
									SendDlgItemMessage(hwndDlg,IDC_FILTER_SENDER ,CB_GETLBTEXT,(WPARAM)i,(LONG) (LPSTR)text);
									_tcscat_s(settingFilterSender, SIZEOF(settingFilterSender), text);
									_tcscat_s(settingFilterSender, SIZEOF(settingFilterSender), TEXT(";"));
								}
								db_set_ts(NULL, PLUGINNAME, "LNFilterSender", settingFilterSender);

								settingFilterSubject[0] = 0;
								for(i=0; i<SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETCOUNT, 0, (LONG)0); i++){
									TCHAR text[512] = TEXT("");
									SendDlgItemMessage(hwndDlg,IDC_FILTER_SUBJECT ,CB_GETLBTEXT,(WPARAM)i,(LONG) (LPSTR)text);
									_tcscat_s(settingFilterSubject, SIZEOF(settingFilterSubject), text);
									_tcscat_s(settingFilterSubject, SIZEOF(settingFilterSubject), TEXT(";"));
								}
								db_set_ts(NULL, PLUGINNAME, "LNFilterSubject", settingFilterSubject);

								settingFilterTo[0] = 0;
								for(i=0; i<SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETCOUNT, 0, (LONG)0); i++){
									TCHAR text[512] = TEXT("");
									SendDlgItemMessage(hwndDlg,IDC_FILTER_TO ,CB_GETLBTEXT,(WPARAM)i,(LONG) (LPSTR)text);
									_tcscat_s(settingFilterTo, SIZEOF(settingFilterTo), text);
									_tcscat_s(settingFilterTo, SIZEOF(settingFilterTo), TEXT(";"));
								}
								db_set_ts(NULL, PLUGINNAME, "LNFilterTo", settingFilterTo);

							}


							return TRUE;
							break;
						}
								//KillTimer(hTimerWnd,TID);
								//if(settingInterval!=0)
								//	SetTimer(hTimerWnd, TID, settingInterval*60000, (TIMERPROC)atTime);

						break;
					} //case 0
				} //id from

				if (GetDlgItem(hwndDlg, IDC_STATUS) == ((LPNMHDR) lParam)->hwndFrom){
					switch (((LPNMHDR) lParam)->code)
					{
						case LVN_ITEMCHANGED:
						{
							NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
							if((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK){
								SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0 );
							}
							break;

						}
						break;
					}
				}
				break; //switch(msg)


		}
	return FALSE;
}


//options page on miranda called
int LotusNotifyOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DIALOG);
	odp.ptszTitle = _A2T(PLUGINNAME);
	odp.ptszGroup = LPGENT("Plugins");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcLotusNotifyOpts; //callback function name
	Options_AddPage(wParam, &odp); //add page to options menu pages
	return 0;
}


//gives protocol avainable statuses
INT_PTR GetCaps(WPARAM wParam, LPARAM lParam)
{
	if(wParam == PFLAGNUM_1)
		return 0;
	if(wParam == PFLAGNUM_2)
		return PF2_ONLINE; // add the possible statuses here.
	if(wParam == PFLAGNUM_3)
		return 0;
	return 0;
}


//gives  name to protocol module
INT_PTR GetName(WPARAM wParam, LPARAM lParam)
{
	strncpy((char*)lParam, PLUGINNAME, wParam);
	return 0;
}


//gives icon for proto module
INT_PTR TMLoadIcon(WPARAM wParam, LPARAM lParam)
{
	UINT id;

	switch(wParam & 0xFFFF) {
		case PLI_ONLINE:
		case PLI_PROTOCOL: id=IDI_ICON1; break; // IDI_TM is the main icon for the protocol
		case PLI_OFFLINE: id=IDI_ICON2; break;
		default: return 0;
	}
	return (int)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON), GetSystemMetrics(wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON), 0);
}


INT_PTR SetStatus(WPARAM wParam, LPARAM lParam)
{

	if (wParam == ID_STATUS_OFFLINE){

		// the status has been changed to online (maybe run some more code)
		LNEnableMenuItem(hMenuHandle, FALSE);
		diffstat = 0;

	} else if (wParam == ID_STATUS_ONLINE){

		diffstat = 0;
		//LNEnableMenuItem(hMenuHandle ,TRUE);
		//NotifyEventHooks(hCheckEvent,wParam,lParam);
		// the status has been changed to offline (maybe run some more code)
		if (currentStatus != ID_STATUS_ONLINE){
			if(startuperror){
				int cnt;
				for(cnt=0; cnt<=4; cnt++)
					if(startuperror >> cnt & 1)
						ErMsgT(TranslateTS(startuperrors[cnt]));
				return 1;
			}

			if(check() == 0){
				if(settingInterval != 0)
					SetTimer(hTimerWnd, TID, settingInterval * 60000, (TIMERPROC)atTime);
				LNEnableMenuItem(hMenuHandle, TRUE);
			} else {
				ProtoBroadcastAck(PLUGINNAME, NULL, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)currentStatus, wParam);
				return -1;
			}
		}
	} else {

		int retv = 0;
		if(settingStatus[wParam - ID_STATUS_ONLINE])
			retv = SetStatus(ID_STATUS_OFFLINE, lParam);
		else
			retv = SetStatus(ID_STATUS_ONLINE, lParam);
		//LNEnableMenuItem(hMenuHandle ,TRUE);
		diffstat = wParam;
		return retv;
		// the status has been changed to unknown  (maybe run some more code)

	}

	//broadcast the message
	if(currentStatus != wParam)
		ProtoBroadcastAck(PLUGINNAME,NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)currentStatus, wParam);
	currentStatus = wParam;

	return 0;
}


void checkEnvPath(TCHAR *path)
{

	TCHAR *cur;
	TCHAR nowy[2048]={0};
	TCHAR *found;
	int len;

	log_p(L"checkEnvPath: [%s]", path);

	_tcslwr(path);
	cur = _tgetenv(_T("PATH"));
	_tcslwr(cur);
	found = _tcsstr(cur,path);
	len = (int)_tcslen(path);
	if(found != NULL && (found[len] == ';' || found[len] == 0 || (found[len] == '\\' && (found[len+1] == ';' || found[len+1] == 0)))) {
		return;
	}

	assert(_tcslen(path) + _tcslen(cur) + 1 < SIZEOF(nowy));
	_tcsncpy_s(nowy, _T("PATH="), _TRUNCATE);
	_tcscat_s(nowy, cur);
	if(cur[_tcslen(cur)-1]!=';')
		_tcscat_s(nowy, _T(";"));
	_tcscat_s(nowy, path);
	_tcscat_s(nowy, _T(";"));

	_tputenv(nowy);

}


//GetStatus
INT_PTR GetStatus(WPARAM wParam, LPARAM lParam)
{
	return currentStatus;
	if (diffstat)
		return diffstat;
	else
		return currentStatus ;
}


//called after all plugins loaded.
//all lotus staff will be called, that will not hang miranda on startup
static int modulesloaded(WPARAM wParam, LPARAM lParam)
{
	int cnt;
	TCHAR path[255] = {0};

	log(L"Modules loaded, lets start LN...");

	GetLotusPath(path, sizeof(path));
	checkEnvPath(path);
	_tcscat_s(path, _countof(path), _T("nnotes.dll"));
	assert(_tcslen(path)>0);

	log_p(L"Loading dll: %s", path);

	hLotusDll = LoadLibrary(path);
	assert(hLotusDll);
	if(hLotusDll != NULL) {

		log(L"Loading LN Functions");

		if(!HookLotusFunctions()) {
			FreeLibrary(hLotusDll);
			startuperror += 1;
		} else {

			log(L"Initializing Lotus");

			if((NotesInitExtended1) (0, NULL)){

				//initialize lotus    //TODO: Lotus can terminate miranda process here with msgbox "Shared Memory from a previous Notes/Domino run has been detected, this process will exit now"
				startuperror += 4;
				running = TRUE;
				LNEnableMenuItem(hMenuHandle, !running);//disable menu cause lotus is not initialized

			} else {

				log(L"Checking Notes Ini File");
				if (!checkNotesIniFile(FALSE)) {
					startuperror += 16;
				}
			}
		}

	} else {
		startuperror += 2;
	}

	assert(startuperror==0);
	for(cnt=0; cnt<=4; cnt++)
	{
		if(startuperror >> cnt & 1)
			ErMsgT(TranslateTS(startuperrors[cnt]));
	}

	return 0;
}


//function hooks before unload
static int preshutdown(WPARAM wParam,LPARAM lParam)
{
	Plugin_Terminated = true;
	deleteElements();
	if(hLotusDll){
		(NotesTerm1)();
		FreeLibrary(hLotusDll);
	}
	return 0;
}


extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	Plugin_Terminated = false;
	InitializeCriticalSection(&checkthreadCS);

	//if(pluginLink)//strange, but this function is called by Lotus API Extension Manager (instead of MainEntryPoint) probably always with parameter poiter =1
	if(bMirandaCall){
		STATUS rc = EMRegister1 (EM_GETPASSWORD, EM_REG_BEFORE | EM_REG_AFTER, EMCallBack, 0, &hLotusRegister); //Extension Manager must know that we are here
		if(rc){
			//Extension magager don't know who we are :(
			startuperror+=8;
			// Get the info from the .ini file
		}
		//log_p(L"Load: Registered Ext. Mngr. res=%d", rc);
		return rc;
	}
	bMirandaCall=TRUE;


	init_pluginname();
	logRegister();
	log_p(L"Load: Entering LotusNotify.dll Load() bMirandaCall=%d PLUGINNAME=[%S]", bMirandaCall, PLUGINNAME);


	if(!(hCheckEvent = CreateHookableEvent("LotusNotify/Check"))) //check if there is another copy of plugin running
	{
		second = TRUE;
	}

	hCheckHook = HookEvent("LotusNotify/Check", eventCheck); //hook function to menu click event

	if(!second) //if its first plugin instance
	{
		//function that will be called on menu click
		hMenuService = CreateServiceFunction("LotusNotify/MenuCommand", PluginMenuCommand);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position = -0x7FFFFFFF; //on top menu position
		mi.flags = CMIF_TCHAR;
		mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		mi.ptszName = LPGENT("&Check Lotus");
		mi.pszService = "LotusNotify/MenuCommand"; //service name thet listning for menu call
		hMenuHandle = Menu_AddMainMenuItem(&mi); //create menu pos.

		LNEnableMenuItem(hMenuHandle ,FALSE);
	}

	//create protocol
	PROTOCOLDESCRIPTOR pd = {PROTOCOLDESCRIPTOR_V3_SIZE};
	pd.szName = PLUGINNAME;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	//set all contacts to offline  ///TODO: are that contacts exists ?
	for(MCONTACT hContact = db_find_first(PLUGINNAME); hContact; hContact = db_find_next(hContact, PLUGINNAME)){
		db_set_w(hContact, PLUGINNAME, "status", ID_STATUS_OFFLINE);
	}

	char service[100] = {""};
	mir_snprintf(service, SIZEOF(service), "%s%s", PLUGINNAME, PS_GETCAPS);
	CreateServiceFunction(service, GetCaps);
	mir_snprintf(service, SIZEOF(service), "%s%s", PLUGINNAME, PS_GETNAME);
	CreateServiceFunction(service, GetName);
	mir_snprintf(service, SIZEOF(service), "%s%s", PLUGINNAME, PS_LOADICON);
	CreateServiceFunction(service, TMLoadIcon);
	mir_snprintf(service, SIZEOF(service), "%s%s", PLUGINNAME, PS_SETSTATUS);
	CreateServiceFunction(service, SetStatus);
	mir_snprintf(service, SIZEOF(service), "%s%s", PLUGINNAME, PS_GETSTATUS);
	CreateServiceFunction(service, GetStatus);

	LoadSettings(); //read from db to variables

	SkinAddNewSoundExT("LotusNotify", LPGENT("Lotus Notify") , LPGENT("New Lotus document detected"));

	hOptInit = HookEvent(ME_OPT_INITIALISE, LotusNotifyOptInit); //register service to hook option call
	assert(hOptInit);
	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, modulesloaded); //hook event that all plugins are loaded
	assert(hHookModulesLoaded);
	hHookPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, preshutdown);
	assert(hHookPreShutdown);

	log(L"Load: ok");
	return 0;
}


extern "C" int __declspec(dllexport) Unload()
{

	log(L"Unload: start");
	Plugin_Terminated = true;
	EnterCriticalSection(&checkthreadCS);
	LeaveCriticalSection(&checkthreadCS);
	DeleteCriticalSection(&checkthreadCS);

	if (hMenuService) DestroyServiceFunction(hMenuService);
	if (hCheckEvent) DestroyHookableEvent(hCheckEvent);
	if (hOptInit) UnhookEvent(hOptInit);
	if (hCheckHook) UnhookEvent(hCheckHook);
	if (hHookModulesLoaded) UnhookEvent(hHookModulesLoaded);
	if (hHookPreShutdown) UnhookEvent(hHookPreShutdown);

	log(L"Unload: ok");
	logUnregister();

	return 0;
}


extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
#ifdef _WIN64
	MessageBox(NULL
		, TranslateT("LotusNotify.dll cannot work with 64bit Miranda. (Lotus client is 32bit only)")
		, TranslateT("LotusNotify")
		, MB_OK | MB_ICONWARNING );
	return NULL;
#else
	return &pluginInfo;
#endif
}


extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_LOTUSNOTIFY, MIID_PROTOCOL, MIID_LAST};


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		/* Save the instance handle */
		Plugin_Terminated = false;
		hInst = hinstDLL;
		break;
	case DLL_PROCESS_DETACH:
		/* Deregister extension manager callbacks */
		Plugin_Terminated = true;
		ExtClear();
		break;
	}
	return TRUE;
}
