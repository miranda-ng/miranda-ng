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

INT_PTR SetStatus(WPARAM wParam, LPARAM lParam);

#define MAX_FIELD 256
#define MAX_SETTING_STR 512
#define STATUS_COUNT 9

char MODULENAME[64] = {0}; //init at init_pluginname();
CMPlugin g_plugin;

HINSTANCE hLotusDll;
HEMREGISTRATION hLotusRegister = 0;

boolean volatile Plugin_Terminated = false;
mir_cs checkthreadCS;

HGENMENU hMenuHandle = nullptr;
HANDLE hCheckEvent = nullptr;

static HWND hTimerWnd = (HWND)nullptr;
static UINT TID = (UINT)2006;

char settingServer[MAX_SETTING_STR] = "", settingServerSec[MAX_SETTING_STR] = "", settingDatabase[MAX_SETTING_STR] = "",
	settingCommand[MAX_SETTING_STR] = "", settingParameters[MAX_SETTING_STR] = "", settingPassword[MAX_SETTING_STR] = "";
wchar_t settingFilterSubject[MAX_SETTING_STR] = TEXT(""), settingFilterSender[MAX_SETTING_STR] = TEXT(""), settingFilterTo[MAX_SETTING_STR] = TEXT("");

COLORREF settingBgColor, settingFgColor;
int settingInterval = 0, settingInterval1 = 0;
uint32_t settingNewestID = 0;
uint8_t settingSetColours = 0, settingShowError = 1, settingIniAnswer = -1, settingIniCheck = 0,
	settingOnceOnly = 0, settingNonClickedOnly = 0, settingNewest = 0, settingEvenNonClicked = 0, settingKeepConnection = 1;
BOOL settingStatus[STATUS_COUNT];
BOOL bMirandaCall=FALSE;

struct HISTORIA *first = nullptr;
BOOL running = FALSE;
BOOL second = FALSE;
BOOL isPopupWaiting = FALSE;
int currentStatus = ID_STATUS_OFFLINE;
int diffstat = 0;
int startuperror = 0;
wchar_t *startuperrors[] = {
		LPGENW("Unable to load all required Lotus API functions"),
		LPGENW("Lotus Notes Client not detected. Check plugin configuration description at https://miranda-ng.org/p/LotusNotify"),
		LPGENW("Unable to initialize Notes."),
		LPGENW("Lotus Notes Extension Manager was not registered. Authentication function will not work properly"),
		LPGENW("In notes.ini file there is no required entry EXTMGR_ADDINS=plugindllnamewithout\".dll\"")
	};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0x23eacc0d, 0xbab0, 0x49c0, { 0x8f, 0x37, 0x5e, 0x25, 0x9e, 0xce, 0x52, 0x7f } } // {23EACC0D-BAB0-49c0-8F37-5E259ECE527F}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
	RegisterProtocol(PROTOTYPE_PROTOCOL);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// authentication callback futnction from extension manager  called by nnotes.dll

STATUS LNPUBLIC __stdcall EMCallBack (EMRECORD * pData)
{
	VARARG_PTR pArgs;
	uint32_t maxPwdLen;
	uint32_t * retLength;
	char * retPassword;
	char * fileName;
	char * ownerName;

	if (pData->EId != EM_GETPASSWORD) return (ERR_EM_CONTINUE); //asking for password?
	if (pData->Status != NOERROR) return (ERR_EM_CONTINUE);

	pArgs= pData->Ap;
	maxPwdLen = VARARG_GET (pArgs, uint32_t);
	retLength = VARARG_GET (pArgs, uint32_t *);
	retPassword = VARARG_GET (pArgs, char *);
	fileName = VARARG_GET (pArgs, char *);
	ownerName = VARARG_GET (pArgs, char *);
	strncpy(retPassword, settingPassword, mir_strlen(settingPassword)); //set our password
	retPassword[mir_strlen(settingPassword)]='\0';
	*retLength = (uint32_t)mir_strlen(retPassword);//and his length
	return ERR_BSAFE_EXTERNAL_PASSWORD;
}


//Main entry point for Ext Manager of Lotus API. called by nnotes.dll
//It don't work (don't know why), and Mirandas Load function is called with value 1 or 0 as parameter...
__declspec(dllexport)STATUS LNPUBLIC MainEntryPoint (void)
{
	STATUS rc = EMRegister1 (EM_GETPASSWORD, EM_REG_BEFORE | EM_REG_AFTER, EMCallBack, 0, &hLotusRegister); //Extension Manager must know that we are here
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
		status = EMDeregister1(&hLotusRegister); //we was registered, so let's unregister
	} else {
		status = NOERROR;
	}
	log_p(L"ExtClear() status=%d", status);
	return;
}


//check if msg was clicked and exists on msgs list
struct HISTORIA* getEl(uint32_t id)
{
	for(struct HISTORIA *cur = first; cur != nullptr; cur = cur->next)
	{
		if(cur->noteID == id)
			return cur;
	}
	return nullptr;
}


//creates new entry on list of msgs
void addNewId(uint32_t id)
{
	struct HISTORIA* nowy = (struct HISTORIA*)mir_alloc(sizeof(struct HISTORIA)) ;
	assert(nowy);
	nowy->noteID = id;
	nowy->next = first;
	nowy->pq = nullptr;
	nowy->again = FALSE;
	first = nowy;
}


//add popup handle. This queue is used to close popups with same msg
void addPopup(uint32_t id,HWND hWnd)
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
	while(curpq != nullptr) {
		delpq = curpq;
		curpq = curpq->next;
		if(closePopup)
			PUDeletePopup(delpq->hWnd);
		mir_free(delpq);
	}
	*firstpq = nullptr;
}


//clear msgs list
void deleteElements()
{
	struct HISTORIA *cur = first, *del;
	while(cur != nullptr)
	{
		del = cur;
		cur = cur->next;
		deletePopupsHandles(&(del->pq),FALSE);
		mir_free(del);
		first = cur;
	}
	first = nullptr;
}


//set plugin name
void init_pluginname()
{
	char text[MAX_PATH], *p, *q;
    WIN32_FIND_DATAA ffd;

    // Try to find name of the file having original letter sizes
	GetModuleFileNameA(g_plugin.getInst(), text, sizeof(text));

    HANDLE hFind = FindFirstFileA(text, &ffd);
    if(hFind != INVALID_HANDLE_VALUE) {
        strncpy_s(text, _countof(text), ffd.cFileName, mir_strlen(ffd.cFileName));
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
	if((q = strstr(p, "debug")) && mir_strlen(q) == 5) {
		*q = '\0';
	}

	// copy to static variable
	strncpy_s(MODULENAME, _countof(MODULENAME), p, mir_strlen(p));
	assert(mir_strlen(MODULENAME)>0);

}


BOOL strrep(char *src, char *needle, char *newstring)
{
	char *found, begining[MAX_SETTING_STR], tail[MAX_SETTING_STR];

	//strset(begining,' ');
	//strset(tail,' ');
	if(!(found=strstr(src,needle)))
		return FALSE;

	size_t pos = (found-src);
	strncpy_s(begining, _countof(begining), src, pos);
	begining[pos]='\0';

	pos += mir_strlen(needle);
	strncpy_s(tail, _countof(tail), src+pos, _countof(tail));
	begining[pos]='\0';

	pos = sprintf(src, "%s%s%s", begining, newstring, tail); //!!!!!!!!!!!!!!!!
	return TRUE;
}


//check if given string contain filter string
//param field=	0-sender
//				1-subject
BOOL checkFilters(wchar_t* str, int field)
{
	wchar_t buff[512] = L"";
	wchar_t *strptr = nullptr;
	switch(field) {
	case 0:
		wcsncpy_s(buff, settingFilterSender, _TRUNCATE);
		break;
	case 1:
		wcsncpy_s(buff, settingFilterSubject, _TRUNCATE);
		break;
	case 2:
		wcsncpy_s(buff, settingFilterTo, _TRUNCATE);
		break;
	}


	while(strptr = wcschr(buff, ';'))
	{
		wchar_t tmp[512] = TEXT(""), *ptr;
		wcsncpy_s(tmp, buff, (strptr-buff));
		wcsncpy_s(buff, strptr + 1, _TRUNCATE);

		if(wcsstr(wcslwr(ptr=wcsdup(str)),wcslwr(tmp)))
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
	POPUPATT *pid = (POPUPATT*)PUGetPluginData(hWnd);
	if(settingOnceOnly&&settingNonClickedOnly)
		(getEl(pid->id))->clicked=TRUE;//add to msgs list

	deletePopupsHandles((&(getEl(pid->id))->pq),TRUE);

	if(settingNewest && (pid->id > settingNewestID) ){
		g_plugin.setDword("LNNewestID", settingNewestID=pid->id);
	}
	if(execute && settingCommand[0] ) {
		char tmpcommand[2*MAX_SETTING_STR];
		char tmpparameters[2*MAX_SETTING_STR];
		strncpy_s(tmpcommand, _countof(tmpcommand), settingCommand, _countof(tmpcommand));
		strncpy_s(tmpparameters, _countof(tmpparameters), settingParameters, _countof(tmpparameters));
		strrep(tmpcommand, "%OID%", pid->strNote);
		strrep(tmpparameters, "%OID%", pid->strNote);
		log_p(L"executing: %S %S", tmpcommand, tmpparameters);
		ShellExecuteA(nullptr, "Open", tmpcommand, tmpparameters, nullptr, SW_NORMAL);
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
			POPUPATT *pid = (POPUPATT*)PUGetPluginData(hWnd);
			addPopup(pid->id,hWnd);
			//PUDeletePopUp(hWnd);
			break;
		}

		case UM_FREEPLUGINDATA:
		{
			POPUPATT *mpd = (POPUPATT*)PUGetPluginData(hWnd);
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
	assert(mir_strlen(tmp1)>0);

	char* PLUGINNAME_lower = _strlwr(mir_strdup(MODULENAME));

	//is there our plugin as safe?
	if(strstr(tmp1,PLUGINNAME_lower) == nullptr)
	{
		if(!settingIniCheck && !bInfo)
			return FALSE;

		if(!settingIniAnswer || bInfo){
			switch(MessageBox(nullptr, TranslateT("This utility check your notes.ini file if it's set to authenticate this plugin as safe. Plugin is not added as Lotus Extension, so plugin built-in authentication will not work properly. Do you want to add plugin as Lotus Extension (modify notes.ini by adding \"EXTMGR_ADDINS=PLUGINNAME\")?"), TranslateT("LotusNotify plugin configuration"), MB_YESNO))
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
			if(mir_strlen(tmp) > 0) {
				strcat_s(tmp, _countof(tmp), ",");
				strcat_s(tmp, _countof(tmp), PLUGINNAME_lower); //add our plugin to extensions
			} else {
				strncpy_s(tmp, _countof(tmp), PLUGINNAME_lower, mir_strlen(PLUGINNAME_lower)); //set our plugin as extension
			}

			(OSSetEnvironmentVariable1) ("EXTMGR_ADDINS", tmp); //set notes.ini entry
			if(bInfo) {
				MessageBox(nullptr, TranslateT("notes.ini modified correctly. Miranda restart required."), TranslateT("LotusNotify plugin configuration"), MB_OK);
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
			MessageBox(nullptr, TranslateT("notes.ini seem to be set correctly."), TranslateT("LotusNotify plugin configuration"), MB_OK);
		return TRUE;
	}

	mir_free(PLUGINNAME_lower);
	return FALSE;
}


//popup plugin to show popup function
void showMsg(wchar_t* sender,wchar_t* text, uint32_t id, char *strUID)
{
	POPUPATT * mpd = (POPUPATT*)malloc(sizeof(POPUPATT));
	mpd->id = id;
	strncpy_s(mpd->strNote, _countof(mpd->strNote), strUID, mir_strlen(strUID));

	POPUPDATAW ppd;
	ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON1));
	wcscpy_s(ppd.lpwzContactName, _countof(ppd.lpwzContactName), sender);
	wcscpy_s(ppd.lpwzText, _countof(ppd.lpwzText), text);
	if(settingSetColours) {
		ppd.colorBack = settingBgColor;
		ppd.colorText = settingFgColor;
	}
	ppd.PluginWindowProc = PopupDlgProc;
	ppd.iSeconds=settingInterval1;
	ppd.PluginData = mpd;
	PUAddPopupW(&ppd);
}


//what to do with error msg
void ErMsgW(wchar_t* msg)
{
	wchar_t* msgT = mir_wstrdup(msg);
	ErMsgT(msgT);
	mir_free(msgT);
}
///TODO wchar_t->wchar_t and test
void ErMsgT(wchar_t* msg)
{
	log_p(L"Error: %S", msg);
	if(settingShowError && !isPopupWaiting) {
		wchar_t buffer[256+14];
		wcsncpy_s(buffer, L"LotusNotify: ", _TRUNCATE);
		wcscat_s(buffer, msg);
		isPopupWaiting = TRUE;
		PUShowMessageW(buffer, SM_WARNING);
		isPopupWaiting = FALSE;
	}
}


//Lotus error occured so translate it
void ErMsgByLotusCode(STATUS erno)
{
	char far error_text_LMBCS[200];
	char far error_text_UNICODEatCHAR[400];
	wchar_t far error_text_UNICODE[200];
    uint16_t text_len;

    text_len = OSLoadString1(NULLHANDLE, erno, error_text_LMBCS, sizeof(error_text_LMBCS)-1);
	OSTranslate1(OS_TRANSLATE_LMBCS_TO_UNICODE, error_text_LMBCS, (uint16_t)mir_strlen(error_text_LMBCS), error_text_UNICODEatCHAR, sizeof(error_text_UNICODEatCHAR)-1);
	memcpy(error_text_UNICODE, error_text_UNICODEatCHAR, sizeof(error_text_UNICODE));

	ErMsgW(error_text_UNICODE);
}

int check() {

	log_p(L"check: Entering check function. running=%d", running);

	if(startuperror) {
		int cnt;
		for(cnt = 0; cnt <= 4; cnt++)
			if(startuperror >> cnt & 1)
				ErMsgT(TranslateW(startuperrors[cnt]));
		return 1;
	}

	if (Plugin_Terminated || Miranda_IsTerminated()){
		log_p(L"check: Plugin_Terminated (=%d) OR Miranda_IsTerminated()", Plugin_Terminated);
		return 0;
	}

	if(running) {
		ErMsgT(TranslateT("Now checking Lotus, try again later"));
		return 1;
	}

	running = TRUE;
	Menu_EnableItem(hMenuHandle, !running);

	log(L"check: starting checkthread");
	mir_forkthread(checkthread);

	return 0;
}



//before pure lotus notes api functions call
void checkthread(void*)
{
	STATUS      error = NOERROR;
	char		fullpath[255];
	DBHANDLE    db_handle = NULLHANDLE;     /* database handle */
	char        UserName[MAXUSERNAME + 1];
	HANDLE      hTable;

	uint32_t		noteID = 0L;
	BOOL		fFirst = TRUE;

	NOTEHANDLE	note_handle;
	uint16_t        field_len;
	char        field_date[MAXALPHATIMEDATE + 1];

	char        field_lotus_LMBCS[MAX_FIELD];
	char        field_lotus_UNICODEatCHAR[MAX_FIELD * sizeof(wchar_t)];
	wchar_t field_from_UNICODE[MAX_FIELD], field_subject_UNICODE[MAX_FIELD], field_to_UNICODE[MAX_FIELD],field_copy_UNICODE[MAX_FIELD];

	mir_cslock lck(checkthreadCS);
	log(L"checkthread: inside new check thread");

	if (error = NotesInitThread1()) {
		goto errorblock;
	}
#ifdef _DEBUG
	log(L"checkthread: Started NotesInitThread");
#endif

	if (error = OSPathNetConstruct1(nullptr, settingServer, settingDatabase, fullpath)) {
		goto errorblock;
	}
#ifdef _DEBUG
	log_p(L"checkthread: OSPathNetConstruct: %S", fullpath);
#endif

	if (error = NSFDbOpen1(fullpath, &db_handle)) {
		if (mir_strcmp(settingServerSec, "") != 0) {
			if (error = OSPathNetConstruct1(nullptr, settingServerSec, settingDatabase, fullpath)) {
				goto errorblock;
			}
			else {
				if (error = NSFDbOpen1(fullpath, &db_handle)) {
					goto errorblock;
				}
			}
		}
		else {
			goto errorblock;
		}
	}
	assert(db_handle);
#ifdef _DEBUG
	log(L"checkthread: DBOpened");
#endif

	if (error = SECKFMGetUserName1(UserName)) {
		goto errorblock0;
	}
	assert(UserName);
#ifdef _DEBUG
	log_p(L"checkthread: Username: %S", UserName);
#endif

	/* Get the unread list */
	if (error = NSFDbGetUnreadNoteTable1(db_handle, UserName, (uint16_t)mir_strlen(UserName), TRUE, &hTable)) {
		goto errorblock0;
	}
#ifdef _DEBUG
	log(L"checkthread: Unread Table got");
#endif

	//error = IDTableCopy (hTable, &hOriginalTable);
	//IDDestroyTable (hTable);
	if (error = NSFDbUpdateUnread1(db_handle, hTable)) {
		goto errorblock;
	}
#ifdef _DEBUG
	log(L"checkthread: Unread Table updated");
#endif
	assert(hTable);


	while (IDScan1(hTable, fFirst, &noteID)) {

		uint16_t Att;
		BLOCKID bhAttachment;
		uint32_t cSize = 0;
		uint32_t attSize = 0;
		OID          retNoteOID;
		TIMEDATE     retModified;     /* modified timedate      */
		uint16_t         retNoteClass;    /* note class             */
		TIMEDATE     sendDate;
		char strLink[4 * 16];

		if (Plugin_Terminated || Miranda_IsTerminated()) {
			log_p(L"checkthread: Plugin_Terminated (=%d) OR Miranda_IsTerminated()", Plugin_Terminated);
			break;
		}

#ifdef _DEBUG
		log_p(L"checkthread: Getting info about: %d", noteID);
#endif

		fFirst = FALSE;
		assert(noteID);
		if (!getEl(noteID))
			addNewId(noteID);
		else
			(getEl(noteID))->again = TRUE;

		if (!settingOnceOnly && (getEl(noteID))->again == TRUE) {
			//don't show again and note was not showed (ID not on list)
			continue;
		}

#ifdef _DEBUG
		log(L"checkthread: skiped-don't show again and note was not showed (ID not on list)");
#endif

		if (settingOnceOnly && settingNonClickedOnly && (getEl(noteID))->clicked == TRUE) {
			//show again, but only not clicked (id added to list on Left Button click)
			continue;
		}

#ifdef _DEBUG
		log(L"checkthread: skiped-show again, but only not clicked (id added to list on Left Button click)");
#endif

		if (settingNewest && settingNewestID >= noteID) {
			//only newest option enabled, so if old id don't show it
			continue;
		}

#ifdef _DEBUG
		log(L"checkthread: skiped-only newest option enabled, so if old id don't show it");
#endif

		// remember newest id depending on options set
		if (settingNewest&&settingEvenNonClicked && (noteID > settingNewestID))
			g_plugin.setDword("LNNewestID", settingNewestID = noteID);

		//if(((!settingOnceOnly||(settingOnceOnly&&settingNonClickedOnly))&&existElem(noteID))||(settingNewest&&settingNewestID>=noteID))
		//continue;

		if (error = NSFNoteOpen1(db_handle, noteID, 0, &note_handle)) {
			continue;
		}

#ifdef _DEBUG
		log_p(L"checkthread: Opened Note: %d", noteID);
#endif

		NSFDbGetNoteInfo1(db_handle,      /* DBHANDLE */
			noteID,			/* NOTEID   */
			&retNoteOID,		/* out: OID */
			&retModified,		/* out:     */
			&retNoteClass);
		memset(strLink, 0, sizeof(strLink));
		mir_snprintf(strLink, "%.8lX%.8lX%.8lX%.8lX",
			retNoteOID.File.Innards[1],
			retNoteOID.File.Innards[0],
			retNoteOID.Note.Innards[1],
			retNoteOID.Note.Innards[0]
			);

		log_p(L"checkthread: got noteInfo, built link: %S", strLink);

		field_len = NSFItemGetText1(note_handle, MAIL_FROM_ITEM, field_lotus_LMBCS, (uint16_t)sizeof(field_lotus_LMBCS));
		OSTranslate1(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_from_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(wchar_t));
		field_from_UNICODE[field_len] = '\0';

		NSFItemGetTime1(note_handle, MAIL_POSTEDDATE_ITEM, &sendDate);
		error = ConvertTIMEDATEToText1(nullptr, nullptr, &sendDate, field_date, MAXALPHATIMEDATE, &field_len);
		field_date[field_len] = '\0';

		field_len = NSFItemGetText1(note_handle, MAIL_SUBJECT_ITEM, field_lotus_LMBCS, (uint16_t)sizeof(field_lotus_LMBCS));
		OSTranslate1(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_subject_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(wchar_t));
		field_subject_UNICODE[field_len] = '\0';

		field_len = NSFItemGetText1(note_handle, MAIL_SENDTO_ITEM, field_lotus_LMBCS, (uint16_t)sizeof(field_lotus_LMBCS));
		OSTranslate1(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_to_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(wchar_t));
		field_to_UNICODE[field_len] = '\0';

		field_len = NSFItemGetText1(note_handle, MAIL_COPYTO_ITEM, field_lotus_LMBCS, (uint16_t)sizeof(field_lotus_LMBCS));
		OSTranslate1(OS_TRANSLATE_LMBCS_TO_UNICODE, field_lotus_LMBCS, field_len, field_lotus_UNICODEatCHAR, sizeof(field_lotus_UNICODEatCHAR));
		memcpy(field_copy_UNICODE, field_lotus_UNICODEatCHAR, field_len * sizeof(wchar_t));
		field_copy_UNICODE[field_len] = '\0';


		wchar_t msgFrom[512], msgSubject[512];
		memset(msgFrom, 0, sizeof(msgFrom));
		memset(msgSubject, 0, sizeof(msgSubject));

		if (mir_wstrlen(field_from_UNICODE) < 512 && mir_wstrlen(field_from_UNICODE) > 3 && wcsstr(field_from_UNICODE, L"CN=") == field_from_UNICODE)
			wcsncpy_s(msgFrom, &(field_from_UNICODE[3]), wcscspn(field_from_UNICODE, L"/") - 3);
		else
			wcsncpy_s(msgFrom, field_from_UNICODE, _TRUNCATE);

		for (Att = 0; MailGetMessageAttachmentInfo1(note_handle, Att, &bhAttachment, nullptr, &cSize, nullptr, nullptr, nullptr, nullptr); Att++)
			attSize += cSize;

#ifdef _DEBUG
		log_p(L"checkthread: MAIL INFO: date=[%S], from=[%s], to=[%s], cc=[%s], sub=[%s], attSize=[%d]"
			, field_date
			, field_from_UNICODE
			, field_to_UNICODE
			, field_copy_UNICODE
			, field_subject_UNICODE
			, attSize
			);
#else
		//do not put private user data into log
		log_p(L"checkthread: MAIL INFO (sizes): date=[%S], from=[%d], to=[%d], cc=[%d], sub=[%d], attSize=[%d]"
			,field_date
			,mir_wstrlen(field_from_UNICODE)
			,mir_wstrlen(field_to_UNICODE)
			,mir_wstrlen(field_copy_UNICODE)
			,mir_wstrlen(field_subject_UNICODE)
			,attSize
			);
#endif


		if (attSize) {
			wchar_t field_attachments_UNICODE[MAX_FIELD];
			mir_snwprintf(field_attachments_UNICODE, TranslateT("Attachments: %d bytes"), attSize);
			mir_snwprintf(msgSubject, L"%S\n%s\n%s", field_date, field_subject_UNICODE, field_attachments_UNICODE);
		}
		else {
			mir_snwprintf(msgSubject, L"%S\n%s", field_date, field_subject_UNICODE);
		}

		//check if this is not filtered msg
		if (!checkFilters(field_from_UNICODE, 0)
			&& !checkFilters(field_subject_UNICODE, 1)
			&& !checkFilters(field_to_UNICODE, 2)
			&& !checkFilters(field_copy_UNICODE, 2)) {
			log(L"checkthread: filters checked - positive");
			///TODO eliminate popups with blank fields
			showMsg(msgFrom, msgSubject, noteID, strLink);
			Skin_PlaySound("LotusNotify");
		}
		else {
			log(L"checkthread: filters checked - negative");
		}

		if (error = NSFNoteClose1(note_handle)) {
			continue;
		}
#ifdef _DEBUG
		log_p(L"checkthread: Close note id: %d", noteID);
#endif

	}

	if (error = IDDestroyTable1(hTable)) {
		goto errorblock0;
	}
#ifdef _DEBUG
	log(L"checkthread: Table destroyed");
#endif

	if (error = NSFDbClose1(db_handle)) {
		goto errorblock;
	}
#ifdef _DEBUG
	log(L"checkthread: DB closed");
#endif

	//NotesTerm();
	NotesTermThread1();

#ifdef _DEBUG
	log(L"checkthread: Terminating Notes thread");
#endif
	running = FALSE;
	if (currentStatus != ID_STATUS_OFFLINE)
		Menu_EnableItem(hMenuHandle, !running);
	return;

errorblock0:
	log(L"checkthread: errorblock0");
	NSFDbClose1(db_handle);
errorblock:
	log_p(L"checkthread: errorblock. error=%d", error);
	ErMsgByLotusCode(error);
	//NotesTerm();

	// go offline if connection error occurs and let KeepStatus or other plugin managing reconnection
	if (!settingKeepConnection && currentStatus != ID_STATUS_OFFLINE) {
		Menu_EnableItem(hMenuHandle, !running);
		SetStatus(ID_STATUS_OFFLINE, 0);
	}

	running = FALSE;
	return;
}


//hooked notification from service that listning to check lotus
static int eventCheck(WPARAM, LPARAM)
{
	log(L"check event...");
	check();
	return 0;
}


//on click to menu callback function
static INT_PTR PluginMenuCommand(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hCheckEvent, wParam, lParam); //create event to check lotus
	return 0;
}


//window timer callback function, called on timer event
static void CALLBACK atTime(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	log(L"atTime: start");
	KillTimer(hTimerWnd, idEvent);
	if (currentStatus != ID_STATUS_OFFLINE) {
		//if status lets to check
		check();
		if (settingInterval != 0) {
			log_p(L"atTime: SetTimer settingInterval=%d", settingInterval * 60000);
			SetTimer(hTimerWnd, TID, settingInterval * 60000, atTime);
		}
	}
}


void decodeServer(char *tmp)
{
	if (strstr(tmp, "CN=") && strstr(tmp, "OU=") && strstr(tmp, "O=")) {
		//if lotus convention
		while (strrep(tmp, "CN=", ""));
		while (strrep(tmp, "OU=", ""));
		while (strrep(tmp, "O=", ""));
	}
}


//fill combo in options dlgbox with all known servers
void fillServersList(HWND hwndDlg)
{
	HANDLE    hServerList = NULLHANDLE;
	uint8_t far *pServerList;            /* Pointer to start of Server List */
	uint16_t      wServerCount;           /* Number of servers in list. */
	uint16_t far *pwServerLength;         /* Index to array of servername lens */
	uint8_t far *pServerName;
	STATUS    error = NOERROR;        /* Error return from API routines. */
	char      ServerString[MAXPATH];  /* String to hold server names.   */
	LPSTR     szServerString = ServerString;

	if (!hLotusDll) {
		return;
	}

	error = NSGetServerList1(nullptr, &hServerList);
	if (error == NOERROR) {

		pServerList = (uint8_t far *) OSLockObject1(hServerList);
		wServerCount = (uint16_t)*pServerList;

		pwServerLength = (uint16_t *)(pServerList + sizeof(uint16_t));

		pServerName = (uint8_t far *) pServerList + sizeof(wServerCount) + ((wServerCount)* sizeof(uint16_t));

		for (USHORT i = 0; i < wServerCount; pServerName += pwServerLength[i], i++) {
			memmove(szServerString, pServerName, pwServerLength[i]);
			szServerString[pwServerLength[i]] = '\0';
			decodeServer(ServerString);
			SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)szServerString);
		}
		OSUnlockObject1(hServerList);
		OSMemFree1(hServerList);

	}
	else {
		ErMsgByLotusCode(error);
	}
}


//gets default settings from notes.ini file
static void lookupLotusDefaultSettings(HWND hwndDlg)
{
	char tmp[MAXENVVALUE + 1];
	// Get the info from the .ini file
	if (hLotusDll) {
		if (OSGetEnvironmentString1("MailFile", tmp, MAXENVVALUE)) //path to mail file
			SetDlgItemTextA(hwndDlg, IDC_DATABASE, tmp); //and set fields in opt. dialog
		if (OSGetEnvironmentString1("MailServer", tmp, MAXENVVALUE)) //server name
		{
			decodeServer(tmp);
			SetDlgItemTextA(hwndDlg, IDC_SERVER, tmp);
		}
	}

}

// get variables values stored in db.
static void LoadSettings()
{
	settingInterval = (INT)g_plugin.getDword("LNInterval", 15);
	settingInterval1 = (INT)g_plugin.getDword("LNInterval1", 0);
	settingKeepConnection = g_plugin.getByte("LNKeepConnection", 1);

	DBVARIANT dbv;
	if (!g_plugin.getString("LNDatabase", &dbv)) {
		strncpy_s(settingDatabase, _countof(settingDatabase), dbv.pszVal, _countof(settingDatabase));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNServer", &dbv)) {
		strncpy_s(settingServer, _countof(settingServer), dbv.pszVal, _countof(settingServer));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNServerSec", &dbv)) {
		strncpy_s(settingServerSec, _countof(settingServerSec), dbv.pszVal, _countof(settingServerSec));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNPassword", &dbv)) {
		strncpy_s(settingPassword, _countof(settingPassword), dbv.pszVal, _countof(settingPassword));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNCommand", &dbv)) {
		strncpy_s(settingCommand, _countof(settingCommand), dbv.pszVal, _countof(settingCommand));
		db_free(&dbv);
	}
	if (!g_plugin.getString("LNParameters", &dbv)) {
		strncpy_s(settingParameters, _countof(settingParameters), dbv.pszVal, _countof(settingParameters));
		db_free(&dbv);
	}

	if (!g_plugin.getWString("LNFilterSender", &dbv)) {
		wcsncpy_s(settingFilterSender, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}
	if (!g_plugin.getWString("LNFilterSubject", &dbv)) {
		wcsncpy_s(settingFilterSubject, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}
	if (!g_plugin.getWString("LNFilterTo", &dbv)) {
		wcsncpy_s(settingFilterTo, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}

	settingOnceOnly = g_plugin.getByte("LNOnceOnly", 0);

	settingNonClickedOnly = g_plugin.getByte("LNNonClickedOnly", 1);
	settingShowError = g_plugin.getByte("LNShowError", 1);
	settingSetColours = g_plugin.getByte("LNSetColours", 0);
	settingBgColor = (COLORREF)g_plugin.getDword("LNBgColor", (uint32_t)0xFFFFFF);
	settingFgColor = (COLORREF)g_plugin.getDword("LNFgColor", (uint32_t)0x000000);
	settingNewest = g_plugin.getByte("LNNewest", 0);
	settingEvenNonClicked = g_plugin.getByte("LNEvenNonClicked", 0);
	settingNewestID = (uint32_t)g_plugin.getDword("LNNewestID", 0);
	settingIniAnswer = g_plugin.getByte("LNIniAnswer", 0);
	settingIniCheck = g_plugin.getByte("LNIniCheck", 0);

	for (int i = 0; i < STATUS_COUNT; i++) {
		char buff[128];
		mir_snprintf(buff, "LNStatus%d", i);
		settingStatus[i] = (g_plugin.getByte(buff, 0) == 1);
	}
	//lookupLotusDefaultSettings();
}

static void SaveSettings(HWND hwndDlg)
{
	char buff[128];
	GetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer, _countof(settingServer));
	g_plugin.setString("LNServer", settingServer);
	g_plugin.setString("LNServerSec", settingServerSec);
	g_plugin.setString("LNPassword", settingPassword);
	g_plugin.setString("LNDatabase", settingDatabase);
	g_plugin.setDword("LNInterval", settingInterval);
	g_plugin.setDword("LNInterval1", settingInterval1);
	g_plugin.setByte("LNKeepConnection", settingKeepConnection);
	g_plugin.setString("LNCommand", settingCommand);
	g_plugin.setString("LNParameters", settingParameters);
	g_plugin.setByte("LNOnceOnly", settingOnceOnly);
	g_plugin.setByte("LNNonClickedOnly", settingNonClickedOnly);
	g_plugin.setByte("LNShowError", settingShowError);
	g_plugin.setByte("LNSetColours", settingSetColours);
	g_plugin.setDword("LNBgColor", (uint32_t)settingBgColor);
	g_plugin.setDword("LNFgColor", (uint32_t)settingFgColor);
	g_plugin.setByte("LNNewest", settingNewest);
	g_plugin.setByte("LNEvenNonClicked", settingEvenNonClicked);
	g_plugin.setByte("LNIniCheck", settingIniCheck);
	g_plugin.setByte("LNIniAnswer", settingIniAnswer);

	for (int i = 0; i < STATUS_COUNT; i++) {
		mir_snprintf(buff, "LNStatus%d", i);
		settingStatus[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_STATUS), i) ? TRUE : FALSE);
		g_plugin.setByte(buff, settingStatus[i] ? 1 : 0);
	}

	settingFilterSender[0] = 0;
	for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETCOUNT, 0, 0); i++) {
		wchar_t text[512] = TEXT("");
		SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
		wcscat_s(settingFilterSender, _countof(settingFilterSender), text);
		wcscat_s(settingFilterSender, _countof(settingFilterSender), TEXT(";"));
	}
	g_plugin.setWString("LNFilterSender", settingFilterSender);

	settingFilterSubject[0] = 0;
	for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETCOUNT, 0, 0); i++) {
		wchar_t text[512] = TEXT("");
		SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
		wcscat_s(settingFilterSubject, _countof(settingFilterSubject), text);
		wcscat_s(settingFilterSubject, _countof(settingFilterSubject), TEXT(";"));
	}
	g_plugin.setWString("LNFilterSubject", settingFilterSubject);

	settingFilterTo[0] = 0;
	for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETCOUNT, 0, 0); i++) {
		wchar_t text[512] = TEXT("");
		SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
		wcscat_s(settingFilterTo, _countof(settingFilterTo), text);
		wcscat_s(settingFilterTo, _countof(settingFilterTo), TEXT(";"));
	}
	g_plugin.setWString("LNFilterTo", settingFilterTo);
}

//callback function to speak with user interactions in options page
static INT_PTR CALLBACK DlgProcLotusNotifyConnectionOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = false;

	switch (msg) {
	case WM_INITDIALOG://initialize dialog, so set properties from db.
		bInit = true;
		TranslateDialogDefault(hwndDlg);//translate miranda function
		LoadSettings();
		CheckDlgButton(hwndDlg, IDC_BUTTON_CHECK, settingIniCheck ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer);
		SetDlgItemTextA(hwndDlg, IDC_SERVERSEC, settingServerSec);
		SetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase);
		SetDlgItemTextA(hwndDlg, IDC_PASSWORD, settingPassword);
		SetDlgItemInt(hwndDlg, IDC_INTERVAL, settingInterval, FALSE);
		CheckDlgButton(hwndDlg, IDC_KEEP_CONNEXION_ON_ERROR, settingKeepConnection ? BST_CHECKED : BST_UNCHECKED);
		bInit = false;
		break;

	case WM_COMMAND://user changed something, so get changes to variables
		if (!bInit) {
			switch (HIWORD(wParam)) {
			case EN_CHANGE:     // text is modified in an edit ctrl
			case BN_CLICKED:    // a checkbox is modified
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_DETECT:
			lookupLotusDefaultSettings(hwndDlg);
			GetDlgItemTextA(hwndDlg, IDC_SERVER, settingServer, _countof(settingServer));
			GetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase, _countof(settingDatabase));
			break;
		case IDC_BUTTON_CHECK:
			settingIniCheck = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BUTTON_CHECK);
			checkNotesIniFile(TRUE);
			break;
		case IDC_DATABASE:
			GetDlgItemTextA(hwndDlg, IDC_DATABASE, settingDatabase, _countof(settingDatabase));
			break;
		case IDC_SERVER:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				int i = SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_GETCURSEL, 0, 0);
				char text[MAXENVVALUE];
				SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_GETLBTEXT, (WPARAM)i, (LPARAM)text);
				SetDlgItemTextA(hwndDlg, IDC_SERVER, text);
				if (!bInit) {
					PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			}

			case CBN_DROPDOWN:
				SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_RESETCONTENT, 0, 0);
				fillServersList(hwndDlg);
				SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)settingServer);
				SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_SELECTSTRING, -1, (LPARAM)settingServer);
				break;
			}
			break;
		case IDC_SERVERSEC:
			GetDlgItemTextA(hwndDlg, IDC_SERVERSEC, settingServerSec, _countof(settingServerSec));
			break;
		case IDC_PASSWORD:
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, settingPassword, _countof(settingPassword));
			break;
		case IDC_INTERVAL:
			settingInterval = GetDlgItemInt(hwndDlg, IDC_INTERVAL, nullptr, FALSE);
			break;
		case IDC_KEEP_CONNEXION_ON_ERROR:
			settingKeepConnection = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_KEEP_CONNEXION_ON_ERROR);
			break;
		}
		break;

	case WM_NOTIFY://apply changes so write it to db
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadSettings();
				return TRUE;

			case PSN_APPLY:
				SaveSettings(hwndDlg);
				return TRUE;
			}
			break;
		} //id from

		break; //switch(msg)

	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcLotusNotifyPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = false;

	switch (msg) {
	case WM_INITDIALOG://initialize dialog, so set properties from db.
		bInit = true;
		TranslateDialogDefault(hwndDlg);//translate miranda function
		LoadSettings();

		CheckDlgButton(hwndDlg, IDC_SETCOLOURS, settingSetColours ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingBgColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), settingSetColours != 0);
		SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingFgColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FGCOLOR), settingSetColours != 0);

		SetDlgItemInt(hwndDlg, IDC_INTERVAL1, settingInterval1, TRUE);
		CheckDlgButton(hwndDlg, IDC_ONCEONLY, settingOnceOnly ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NONCLICKEDONLY, settingNonClickedOnly ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_NONCLICKEDONLY), settingOnceOnly != 0);
		CheckDlgButton(hwndDlg, IDC_SHOWERROR, settingShowError ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NEWEST, settingNewest ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_REMEMBEREVENNONCLICKED, settingEvenNonClicked ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_REMEMBEREVENNONCLICKED), settingNewest != 0);
		SetDlgItemTextA(hwndDlg, IDC_COMMAND, settingCommand);
		SetDlgItemTextA(hwndDlg, IDC_PARAMETERS, settingParameters);

		bInit = FALSE;
		break;

	case WM_COMMAND://user changed something, so get changes to variables
		if (!bInit) {
			switch (HIWORD(wParam)) {
			case EN_CHANGE:         // text is modified in an edit ctrl
			case BN_CLICKED:        // a checkbox is modified
			case CPN_COLOURCHANGED: // a color has changed
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		switch (LOWORD(wParam)) {
		case IDC_SETCOLOURS:
			settingSetColours = IsDlgButtonChecked(hwndDlg, IDC_SETCOLOURS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), settingSetColours);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FGCOLOR), settingSetColours);
			break;
		case IDC_BGCOLOR:
			settingBgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
			break;
		case IDC_FGCOLOR:
			settingFgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_GETCOLOUR, 0, 0);
			break;
		case IDC_INTERVAL1:
			settingInterval1 = GetDlgItemInt(hwndDlg, IDC_INTERVAL1, nullptr, TRUE);
			break;
		case IDC_ONCEONLY:
			settingOnceOnly = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONCEONLY);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NONCLICKEDONLY), settingOnceOnly);
			break;
		case IDC_NONCLICKEDONLY:
			settingNonClickedOnly = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NONCLICKEDONLY);
			break;
		case IDC_SHOWERROR:
			settingShowError = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWERROR);
			break;
		case IDC_NEWEST:
			settingNewest = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NEWEST);
			EnableWindow(GetDlgItem(hwndDlg, IDC_REMEMBEREVENNONCLICKED), settingNewest);
			break;
		case IDC_REMEMBEREVENNONCLICKED:
			settingEvenNonClicked = (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_REMEMBEREVENNONCLICKED);
			break;
		case IDC_COMMAND:
			GetDlgItemTextA(hwndDlg, IDC_COMMAND, settingCommand, _countof(settingCommand));
			break;
		case IDC_PARAMETERS:
			GetDlgItemTextA(hwndDlg, IDC_PARAMETERS, settingParameters, _countof(settingParameters));
			break;
		case IDC_BUTTON_CLEAR:
			deleteElements();
			break;
		}
		break;

	case WM_NOTIFY://apply changes so write it to db
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			{
				switch (((LPNMHDR)lParam)->code) {
				case PSN_RESET:
					LoadSettings();
					return TRUE;
				case PSN_APPLY:
					SaveSettings(hwndDlg);

					return TRUE;
					break;
				}
				//KillTimer(hTimerWnd,TID);
				//if(settingInterval!=0)
				//	SetTimer(hTimerWnd, TID, settingInterval*60000, (TIMERPROC)atTime);

				break;
			} //case 0
		} //id from

		break; //switch(msg)

	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcLotusNotifyMiscOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = false;

	wchar_t* strptr;
	LVITEM lvI = { 0 };
	LVCOLUMN lvc = { 0 };
	switch (msg) {
	case WM_INITDIALOG://initialize dialog, so set properties from db.
	{
		wchar_t buff[512];
		bInit = true;
		TranslateDialogDefault(hwndDlg);//translate miranda function
		LoadSettings();

		//fill filter combos

		wcsncpy_s(buff, settingFilterSender, _TRUNCATE);
		while (strptr = wcschr(buff, TEXT(';'))) {
			wchar_t tmp[512] = TEXT("");
			wcsncpy_s(tmp, buff, (strptr - buff));
			SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_ADDSTRING, 0, (LPARAM)tmp);
			wcsncpy_s(buff, strptr + 1, _TRUNCATE);
		}

		wcsncpy_s(buff, settingFilterSubject, _TRUNCATE);
		while (strptr = wcschr(buff, TEXT(';'))) {
			wchar_t tmp[512] = TEXT("");
			wcsncpy_s(tmp, buff, (strptr - buff));
			SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_ADDSTRING, 0, (LPARAM)tmp);
			wcsncpy_s(buff, strptr + 1, _TRUNCATE);
		}

		wcsncpy_s(buff, settingFilterTo, _TRUNCATE);
		while (strptr = wcschr(buff, TEXT(';'))) {
			wchar_t tmp[512] = TEXT("");
			wcsncpy_s(tmp, buff, (strptr - buff));
			SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_ADDSTRING, 0, (LPARAM)tmp);
			wcsncpy_s(buff, strptr + 1, _TRUNCATE);
		}

		// initialise and fill listbox
		HWND hwndList = GetDlgItem(hwndDlg, IDC_STATUS);
		ListView_DeleteAllItems(hwndList);

		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		// Initialize the LVCOLUMN structure.
		// The mask specifies that the format, width, text, and
		// subitem members of the structure are valid.
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		lvc.iSubItem = 0;
		lvc.pszText = TranslateT("Status");
		lvc.cx = 120;     // width of column in pixels
		ListView_InsertColumn(hwndList, 0, &lvc);

		// Some code to create the list-view control.
		// Initialize LVITEM members that are common to all items.
		lvI.mask = LVIF_TEXT;
		for (int i = 0; i < STATUS_COUNT; i++) {
			lvI.pszText = Clist_GetStatusModeDescription(ID_STATUS_ONLINE + i, 0);
			lvI.iItem = i;
			ListView_InsertItem(hwndList, &lvI);
			ListView_SetCheckState(hwndList, i, settingStatus[i]);
		}

		bInit = false;
		break;
	}
	case WM_COMMAND://user changed something, so get changes to variables
	{
		if (!bInit && (HIWORD(wParam) == EN_CHANGE)) {
			PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		char tmp[255];
		int index, size;
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_ADD_SENDER_FILTER:
			GetDlgItemTextA(hwndDlg, IDC_FILTER_SENDER, tmp, _countof(tmp));
			if (strlen(tmp) > 0) {
				SendDlgItemMessageA(hwndDlg, IDC_FILTER_SENDER, CB_ADDSTRING, 0, (LPARAM)tmp);
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		case IDC_BUTTON_REMOVE_SENDER_FILTER:
			index = SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_GETCURSEL, 0, 0);
			size = SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_DELETESTRING, index, 0);
			SendDlgItemMessage(hwndDlg, IDC_FILTER_SENDER, CB_SETCURSEL, min(index, size - 1), 0);
			PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_BUTTON_ADD_SUBJECT_FILTER:
			GetDlgItemTextA(hwndDlg, IDC_FILTER_SUBJECT, tmp, _countof(tmp));
			if (strlen(tmp) > 0) {
				SendDlgItemMessageA(hwndDlg, IDC_FILTER_SUBJECT, CB_ADDSTRING, 0, (LPARAM)tmp);
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		case IDC_BUTTON_REMOVE_SUBJECT_FILTER:
			index = SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_GETCURSEL, 0, 0);
			size = SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_DELETESTRING, index, 0);
			SendDlgItemMessage(hwndDlg, IDC_FILTER_SUBJECT, CB_SETCURSEL, min(index, size - 1), 0);
			PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_BUTTON_ADD_TO_FILTER:
			GetDlgItemTextA(hwndDlg, IDC_FILTER_TO, tmp, _countof(tmp));
			if (strlen(tmp) > 0) {
				SendDlgItemMessageA(hwndDlg, IDC_FILTER_TO, CB_ADDSTRING, 0, (LPARAM)tmp);
				PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		case IDC_BUTTON_REMOVE_TO_FILTER:
			index = SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_GETCURSEL, 0, 0);
			size = SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_DELETESTRING, index, 0);
			SendDlgItemMessage(hwndDlg, IDC_FILTER_TO, CB_SETCURSEL, min(index, size - 1), 0);
			PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;
	}
	case WM_NOTIFY://apply changes so write it to db
		if (bInit) {
			break;
		}
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadSettings();
				return TRUE;

			case PSN_APPLY:
				SaveSettings(hwndDlg);
				return TRUE;
			}

			break;
		} //id from

		if (GetDlgItem(hwndDlg, IDC_STATUS) == ((LPNMHDR)lParam)->hwndFrom) {
			switch (((LPNMHDR)lParam)->code) {
			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
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
int LotusNotifyOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Plugins");
	odp.szTitle.w = _A2W(__PLUGIN_NAME);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	odp.szTab.w = LPGENW("Connection");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOTUS_CONECTION);
	odp.pfnDlgProc = DlgProcLotusNotifyConnectionOpts;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Popup");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOTUS_POPUP);
	odp.pfnDlgProc = DlgProcLotusNotifyPopupOpts;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Miscellaneous");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LOTUS_MISC);
	odp.pfnDlgProc = DlgProcLotusNotifyMiscOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}


//gives protocol avainable statuses
INT_PTR GetCaps(WPARAM wParam, LPARAM)
{
	if (wParam == PFLAGNUM_1)
		return 0;
	if (wParam == PFLAGNUM_2)
		return PF2_ONLINE; // add the possible statuses here.
	if (wParam == PFLAGNUM_3)
		return 0;
	return 0;
}


//gives  name to protocol module
INT_PTR GetName(WPARAM wParam, LPARAM lParam)
{
	strncpy((char*)lParam, MODULENAME, wParam);
	return 0;
}


//gives icon for proto module
INT_PTR TMLoadIcon(WPARAM wParam, LPARAM)
{
	UINT id;

	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL:
		id = IDI_ICON1;
		break; // IDI_TM is the main icon for the protocol
	default:
		return 0;
	}
	return (INT_PTR)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON), GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}


INT_PTR SetStatus(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_STATUS_OFFLINE) {
		// the status has been changed to online (maybe run some more code)
		Menu_EnableItem(hMenuHandle, FALSE);
		diffstat = 0;

	}
	else if (wParam == ID_STATUS_ONLINE) {
		diffstat = 0;
		//Menu_EnableItem(hMenuHandle ,TRUE);
		//NotifyEventHooks(hCheckEvent,wParam,lParam);
		// the status has been changed to offline (maybe run some more code)
		if (currentStatus != ID_STATUS_ONLINE) {
			if (startuperror) {
				int cnt;
				for (cnt = 0; cnt <= 4; cnt++)
					if (startuperror >> cnt & 1)
						ErMsgT(TranslateW(startuperrors[cnt]));
				return 1;
			}

			if (check() == 0) {
				if (settingInterval != 0)
					SetTimer(hTimerWnd, TID, settingInterval * 60000, atTime);
				Menu_EnableItem(hMenuHandle, TRUE);
			}
			else {
				ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)currentStatus, wParam);
				return -1;
			}
		}
	}
	else {
		int retv;
		if (settingStatus[wParam - ID_STATUS_ONLINE])
			retv = SetStatus(ID_STATUS_OFFLINE, lParam);
		else
			retv = SetStatus(ID_STATUS_ONLINE, lParam);
		//Menu_EnableItem(hMenuHandle ,TRUE);
		diffstat = wParam;
		return retv;
		// the status has been changed to unknown  (maybe run some more code)
	}
	//broadcast the message
	if (currentStatus != (int)wParam)
		ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)currentStatus, wParam);
	currentStatus = wParam;

	return 0;
}


void checkEnvPath(wchar_t *path)
{
	log_p(L"checkEnvPath: [%s]", path);

	wcslwr(path);
	wchar_t *cur = _wgetenv(L"PATH");
	wcslwr(cur);
	wchar_t *found = wcsstr(cur, path);
	size_t len = mir_wstrlen(path);
	if (found != nullptr && (found[len] == ';' || found[len] == 0 || (found[len] == '\\' && (found[len + 1] == ';' || found[len + 1] == 0))))
		return;

	_wputenv(CMStringW(FORMAT, L"PATH=%s;%s;", cur, path));
}

//GetStatus
static INT_PTR GetStatus(WPARAM, LPARAM)
{
	return currentStatus;
}


//called after all plugins loaded.
//all lotus staff will be called, that will not hang miranda on startup
static int modulesloaded(WPARAM, LPARAM)
{
	int cnt;
	wchar_t path[255] = { 0 };

	log(L"Modules loaded, lets start LN...");

	GetLotusPath(path, sizeof(path));
	checkEnvPath(path);
	wcscat_s(path, _countof(path), L"nnotes.dll");
	assert(mir_wstrlen(path) > 0);

	log_p(L"Loading dll: %s", path);

	hLotusDll = LoadLibrary(path);
	assert(hLotusDll);
	if (hLotusDll != nullptr) {

		log(L"Loading LN Functions");

		if (!HookLotusFunctions()) {
			FreeLibrary(hLotusDll);
			startuperror += 1;
		}
		else {

			log(L"Initializing Lotus");

			if (NotesInitExtended1(0, nullptr)) {

				//initialize lotus    //TODO: Lotus can terminate miranda process here with msgbox "Shared Memory from a previous Notes/Domino run has been detected, this process will exit now"
				startuperror += 4;
				running = TRUE;
				Menu_EnableItem(hMenuHandle, !running);//disable menu cause lotus is not initialized

			}
			else {
				log(L"Checking Notes Ini File");
				if (!checkNotesIniFile(FALSE)) {
					startuperror += 16;
				}
			}
		}

	}
	else {
		startuperror += 2;
	}

	assert(startuperror == 0);
	for (cnt = 0; cnt <= 4; cnt++) {
		if (startuperror >> cnt & 1)
			ErMsgT(TranslateW(startuperrors[cnt]));
	}

	return 0;
}


//function hooks before unload
static int preshutdown(WPARAM, LPARAM)
{
	Plugin_Terminated = true;
	deleteElements();
	if (hLotusDll) {
		NotesTerm1();
		FreeLibrary(hLotusDll);
	}
	return 0;
}


int CMPlugin::Load()
{
	Plugin_Terminated = false;

	//if(pluginLink)//strange, but this function is called by Lotus API Extension Manager (instead of MainEntryPoint) probably always with parameter poiter =1
	if (bMirandaCall) {
		STATUS rc = EMRegister1(EM_GETPASSWORD, EM_REG_BEFORE | EM_REG_AFTER, EMCallBack, 0, &hLotusRegister); //Extension Manager must know that we are here
		if (rc) {
			//Extension magager don't know who we are :(
			startuperror += 8;
			// Get the info from the .ini file
		}
		//log_p(L"Load: Registered Ext. Mngr. res=%d", rc);
		return rc;
	}
	bMirandaCall = TRUE;

	init_pluginname();
	logRegister();
	log_p(L"Load: Entering LotusNotify.dll Load() bMirandaCall=%d MODULENAME=[%S]", bMirandaCall, MODULENAME);

	if (!(hCheckEvent = CreateHookableEvent("LotusNotify/Check"))) //check if there is another copy of plugin running
		second = TRUE;

	HookEvent("LotusNotify/Check", eventCheck); //hook function to menu click event

	if (!second) //if its first plugin instance
	{
		//function that will be called on menu click
		CreateServiceFunction("LotusNotify/MenuCommand", PluginMenuCommand);

		CMenuItem mi(&g_plugin);
		SET_UID(mi, 0x4519458, 0xb55a, 0x4e22, 0xac, 0x95, 0x5e, 0xa4, 0x4d, 0x92, 0x65, 0x65);
		mi.position = -0x7FFFFFFF; //on top menu position
		mi.flags = CMIF_UNICODE;
		mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON1));
		mi.name.w = LPGENW("&Check Lotus");
		mi.pszService = "LotusNotify/MenuCommand"; //service name thet listning for menu call
		hMenuHandle = Menu_AddMainMenuItem(&mi); //create menu pos.

		Menu_EnableItem(hMenuHandle, FALSE);
	}

	// set all contacts to offline
	for (auto &hContact : Contacts(MODULENAME))
		g_plugin.setWord(hContact, "status", ID_STATUS_OFFLINE);

	CreateProtoServiceFunction(MODULENAME, PS_GETCAPS, GetCaps);
	CreateProtoServiceFunction(MODULENAME, PS_GETNAME, GetName);
	CreateProtoServiceFunction(MODULENAME, PS_LOADICON, TMLoadIcon);
	CreateProtoServiceFunction(MODULENAME, PS_SETSTATUS, SetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_GETSTATUS, GetStatus);

	LoadSettings(); //read from db to variables

	g_plugin.addSound("LotusNotify", LPGENW("Lotus Notify"), LPGENW("New Lotus document detected"));

	HookEvent(ME_OPT_INITIALISE, LotusNotifyOptInit); //register service to hook option call
	HookEvent(ME_SYSTEM_MODULESLOADED, modulesloaded); //hook event that all plugins are loaded
	HookEvent(ME_SYSTEM_PRESHUTDOWN, preshutdown);

	log(L"Load: ok");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	log(L"Unload: start");
	Plugin_Terminated = true;
	mir_cslock lck(checkthreadCS);

	DestroyHookableEvent(hCheckEvent);

	log(L"Unload: ok");
	logUnregister();

	ExtClear();
	return 0;
}
