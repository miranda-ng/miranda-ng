/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

part of this code based on:
Miranda IM Country Flags Plugin Copyright �2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "..\commonheaders.h"

#define M_ENABLE_SUBCTLS			(WM_APP+1)

FLAGSOPTIONS	gFlagsOpts;

/* Misc */
int		nCountriesCount;
struct	CountryListEntry *countries;
static	HANDLE hExtraIconSvc		= INVALID_HANDLE_VALUE;
/* hook */
static HANDLE hRebuildIconsHook		= NULL;
static HANDLE hApplyIconHook		= NULL;
static HANDLE hMsgWndEventHook		= NULL;
static HANDLE hIconsChangedHook		= NULL;
static HANDLE hSettingChangedHook	= NULL;

static HANDLE hOptInitHook			= NULL;

static int OnContactSettingChanged(WPARAM wParam,LPARAM lParam);

/***********************************************************************************************************
 * Buffered functions
 ***********************************************************************************************************/

struct BufferedCallData {
	DWORD startTick;
	UINT uElapse;
	BUFFEREDPROC pfnBuffProc;
	LPARAM lParam;
	#ifdef _DEBUG
	const char *pszProcName;
	#endif
};

static UINT_PTR idBufferedTimer;
static struct BufferedCallData *callList;
static int nCallListCount;

// always gets called in main message loop
static void CALLBACK BufferedProcTimer(HWND hwnd, UINT msg, UINT idTimer, DWORD currentTick) {
	int i;
	struct BufferedCallData *buf;
	UINT uElapsed, uElapseNext = USER_TIMER_MAXIMUM;
	BUFFEREDPROC pfnBuffProc;
	LPARAM lParam;
	#ifdef _DEBUG
	char szDbgLine[256];
	const char *pszProcName;
	#endif
	UNREFERENCED_PARAMETER(msg);

	for(i=0;i<nCallListCount;++i) {
		/* find elapsed procs */
		uElapsed=currentTick-callList[i].startTick; /* wraparound works */
		if ((uElapsed+USER_TIMER_MINIMUM)>=callList[i].uElapse) { 
			/* call elapsed proc */
			pfnBuffProc=callList[i].pfnBuffProc;
			lParam=callList[i].lParam;
			#ifdef _DEBUG
				pszProcName=callList[i].pszProcName;
			#endif
			/* resize storage array */
			if ((i+1)<nCallListCount)
				MoveMemory(&callList[i],&callList[i+1],((nCallListCount-i-1)*sizeof(struct BufferedCallData)));
			--nCallListCount;
			--i; /* reiterate current */
			if (nCallListCount) {
				buf=(struct BufferedCallData*)mir_realloc(callList,nCallListCount*sizeof(struct BufferedCallData));
				if (buf!=NULL) callList=buf;
			} else {
				mir_free(callList);
				callList=NULL;
			}
			#ifdef _DEBUG
				mir_snprintf(szDbgLine,sizeof(szDbgLine),"buffered call: %s(0x%X)\n",pszProcName,lParam); /* all ascii */
				OutputDebugStringA(szDbgLine);
			#endif
			CallFunctionAsync((void (CALLBACK *)(void*))pfnBuffProc,(void*)lParam); /* compatible */
		}
		/* find next timer delay */
		else if ((callList[i].uElapse-uElapsed)<uElapseNext)
			uElapseNext=callList[i].uElapse-uElapsed;
	}

	/* set next timer */
	if (nCallListCount) {
		#ifdef _DEBUG
			mir_snprintf(szDbgLine,sizeof(szDbgLine),"next buffered timeout: %ums\n",uElapseNext); /* all ascii */
			OutputDebugStringA(szDbgLine);
		#endif
		idBufferedTimer=SetTimer(hwnd, idBufferedTimer, uElapseNext, (TIMERPROC)BufferedProcTimer); /* will be reset */
	} else {
		KillTimer(hwnd,idTimer);
		idBufferedTimer=0;
		#ifdef _DEBUG
			OutputDebugStringA("empty buffered queue\n");
		#endif
	}
}

// assumes to be called in context of main thread
#ifdef _DEBUG
void _CallFunctionBuffered(BUFFEREDPROC pfnBuffProc,const char *pszProcName,LPARAM lParam,BOOL fAccumulateSameParam,UINT uElapse)
#else
void _CallFunctionBuffered(BUFFEREDPROC pfnBuffProc,LPARAM lParam,BOOL fAccumulateSameParam,UINT uElapse)
#endif
{
	struct BufferedCallData *data=NULL;
	int i;

	/* find existing */
	for(i=0;i<nCallListCount;++i)
		if (callList[i].pfnBuffProc==pfnBuffProc)
			if (!fAccumulateSameParam || callList[i].lParam==lParam) {
				data=&callList[i];
				break;
			}
	/* append new */
	if (data==NULL) {
		/* resize storage array */
		data=(struct BufferedCallData*)mir_realloc(callList,(nCallListCount+1)*sizeof(struct BufferedCallData));
		if (data==NULL) return;
		callList=data;
		data=&callList[nCallListCount];
		++nCallListCount;
	}
	/* set delay */
	data->startTick=GetTickCount();
	data->uElapse=uElapse;
	data->lParam=lParam;
	data->pfnBuffProc=pfnBuffProc;
	#ifdef _DEBUG
		{	char szDbgLine[256];
			data->pszProcName=pszProcName;
			mir_snprintf(szDbgLine,sizeof(szDbgLine),"buffered queue: %s(0x%X)\n",pszProcName,lParam); /* all ascii */
			OutputDebugStringA(szDbgLine);
			if (!idBufferedTimer) {
				mir_snprintf(szDbgLine,sizeof(szDbgLine),"next buffered timeout: %ums\n",uElapse); /* all ascii */
				OutputDebugStringA(szDbgLine);
			}
		}
	#endif
	/* set next timer */
	if (idBufferedTimer) uElapse=USER_TIMER_MINIMUM; /* will get recalculated */
	idBufferedTimer=SetTimer(NULL,idBufferedTimer,uElapse,(TIMERPROC)BufferedProcTimer);
}

// assumes to be called in context of main thread
void PrepareBufferedFunctions()
{
	idBufferedTimer=0;
	nCallListCount=0;
	callList=NULL;
}

// assumes to be called in context of main thread
void KillBufferedFunctions()
{
	if (idBufferedTimer) KillTimer(NULL,idBufferedTimer);
	nCallListCount=0;
	mir_free(callList); /* does NULL check */
}

/***********************************************************************************************************
 * service functions
 ***********************************************************************************************************/

static INT_PTR ServiceDetectContactOriginCountry(WPARAM wParam,LPARAM lParam)
{
	WORD countryNumber;
	char *pszProto = GetContactProto((HANDLE)wParam);
	/* UserinfoEx */
	if (countryNumber = db_get_w((HANDLE)wParam, USERINFO, SET_CONTACT_ORIGIN_COUNTRY, 0))
		return (INT_PTR)countryNumber;
	else if (countryNumber = db_get_w((HANDLE)wParam, USERINFO, SET_CONTACT_COUNTRY, 0))
		return (INT_PTR)countryNumber;
	else if (countryNumber = db_get_w((HANDLE)wParam, USERINFO, SET_CONTACT_COMPANY_COUNTRY, 0))
		return (INT_PTR)countryNumber;
	/* fallback proto settings */
	else if (countryNumber = db_get_w((HANDLE)wParam, pszProto, "Country", 0))
		return (INT_PTR)countryNumber;
	else if (countryNumber = db_get_w((HANDLE)wParam, pszProto, "CompanyCountry", 0))
		return (INT_PTR)countryNumber;
	/* fallback ip detect
	else if (countryNumber==0xFFFF && db_get_b(NULL,"Flags","UseIpToCountry",SETTING_USEIPTOCOUNTRY_DEFAULT)) {
		countryNumber=ServiceIpToCountry(db_get_dw((HANDLE)wParam,pszProto,"RealIP",0),0);
	}*/

	return (INT_PTR)0xFFFF;
}

/***********************************************************************************************************
 * Clist Extra Image functions
 ***********************************************************************************************************/

static void CALLBACK SetExtraImage(LPARAM lParam)
{
	/* get contact's country */
	int countryNumber = ServiceDetectContactOriginCountry((WPARAM)lParam,0);
	ExtraIcon_SetIcon(hExtraIconSvc, (HANDLE)lParam, (countryNumber != 0xFFFF || gFlagsOpts.bUseUnknownFlag) ? LoadFlagHandle(countryNumber) : NULL);
}

static void CALLBACK RemoveExtraImages(LPARAM lParam)
{
	/* enum all contacts */
	for (HANDLE hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
		ExtraIcon_Clear(hExtraIconSvc, hContact);
}

// always call in context of main thread
void EnsureExtraImages()
{
	for (HANDLE hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
		CallFunctionBuffered(SetExtraImage,(LPARAM)hContact,TRUE,EXTRAIMAGE_REFRESHDELAY);
}

static void CALLBACK UpdateExtraImages(LPARAM lParam) {
	if (!lParam)
		RemoveExtraImages(0);
	else
		EnsureExtraImages();
}

static int OnCListApplyIcons(WPARAM wParam,LPARAM lParam)
{
	SetExtraImage((LPARAM)wParam); /* unbuffered */
	return 0;
}

//hookProc (ME_DB_CONTACT_SETTINGCHANGED) - workaround for missing event from ExtraIconSvc
static int OnExtraIconSvcChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
	if ((HANDLE)wParam != NULL)
		return 0;

	if (!lstrcmpA(dbcws->szModule, "ExtraIcons") && !lstrcmpA(dbcws->szSetting,"Slot_Flags")) {
		BOOL bEnable;
		switch (dbcws->value.type) {
		case DBVT_BYTE:
			bEnable = dbcws->value.bVal != (BYTE)-1;
			break;
		case DBVT_WORD:
			bEnable = dbcws->value.wVal != (WORD)-1;
			break;
		case DBVT_DWORD:
			bEnable = dbcws->value.dVal != (DWORD)-1;
			break;
		default:
			bEnable = -1;
			break;
		}
		if (bEnable == -1)
			return 0;

		if (bEnable && !hApplyIconHook)
			hApplyIconHook = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnCListApplyIcons);
		else if (!bEnable && hApplyIconHook)
			UnhookEvent(hApplyIconHook); hApplyIconHook = NULL;

		CallFunctionBuffered(UpdateExtraImages,(LPARAM)bEnable,FALSE,EXTRAIMAGE_REFRESHDELAY);
	}
	return 0;
}

void SvcFlagsEnableExtraIcons(BYTE bColumn, BYTE bUpdateDB)
{
	gFlagsOpts.bShowExtraImgFlag = (bColumn!=((BYTE)-1));
	if (bUpdateDB)
		db_set_b(NULL, MODNAMEFLAGS, "ShowExtraImgFlag", bColumn!=(BYTE)-1);

	// Flags is on
	if (gFlagsOpts.bShowExtraImgFlag) {
		if (hExtraIconSvc == INVALID_HANDLE_VALUE) {
			//get local langID for descIcon (try to use user local Flag as icon)
			DWORD langid = 0;
			int r = GetLocaleInfo(
				LOCALE_USER_DEFAULT,
				LOCALE_ICOUNTRY | LOCALE_RETURN_NUMBER ,
				(LPTSTR)&langid, sizeof(langid)/sizeof(TCHAR));
			if (!CallService(MS_UTILS_GETCOUNTRYBYNUMBER,langid,0)) langid = 1;

			char szId[20];
			mir_snprintf(szId, SIZEOF(szId), (langid==0xFFFF)?"%s_0x%X":"%s_%i","flags",langid); /* buffer safe */
			hExtraIconSvc = ExtraIcon_Register("Flags", LPGEN("Flags (uinfoex)"), szId);
			if (hExtraIconSvc)
				HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnExtraIconSvcChanged);
		}

		//init hooks
		if (!hApplyIconHook)
			hApplyIconHook = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnCListApplyIcons);

		if (!hSettingChangedHook)
			hSettingChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	}
}

/***********************************************************************************************************
 * message winsow status icon functions
 ***********************************************************************************************************/

MsgWndData::MsgWndData(HWND hwnd, HANDLE hContact) {
	m_hwnd		= hwnd;
	m_hContact	= hContact;
	m_countryID	= (int)ServiceDetectContactOriginCountry((WPARAM)m_hContact,0);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODNAMEFLAGS;
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	FlagsIconUpdate();
}

MsgWndData::~MsgWndData() {
	FlagsIconUnset();			//check if realy need
}

void MsgWndData::FlagsIconSet()
{
	/* ensure status icon is registered */
	if (m_countryID != 0xFFFF || gFlagsOpts.bUseUnknownFlag) {
		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODNAMEFLAGS;
		sid.hIconDisabled	= sid.hIcon = LoadFlagIcon(m_countryID);
		sid.szTooltip = Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,m_countryID,0));
		Srmm_ModifyIcon(m_hContact, &sid);
	}	
}

void MsgWndData::FlagsIconUnset()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODNAMEFLAGS;
	sid.flags = MBF_HIDDEN;
	Srmm_ModifyIcon(m_hContact, &sid);
}

static int CompareMsgWndData(const MsgWndData* p1, const MsgWndData* p2)
{
	return (int)((INT_PTR)p1->m_hContact - (INT_PTR)p2->m_hContact);
}
static LIST<MsgWndData> gMsgWndList(10, CompareMsgWndData);

static int CompareIconListData(const IconList* p1, const IconList* p2)
{
	return (int)((INT_PTR)p1->m_ID - (INT_PTR)p2->m_ID);
}
static OBJLIST<IconList> gIListMW(10, CompareIconListData);

IconList::IconList(StatusIconData *sid)
{
	m_StatusIconData.cbSize			= sid->cbSize;
	m_StatusIconData.szModule		= mir_strdup(sid->szModule);
	m_StatusIconData.dwId			= sid->dwId;
	m_StatusIconData.hIcon			= CopyIcon(sid->hIcon);
	m_StatusIconData.hIconDisabled	= sid->hIconDisabled;
	m_StatusIconData.flags			= sid->flags;
	m_StatusIconData.szTooltip		= mir_strdup(sid->szTooltip);

	m_ID = sid->dwId;
	Srmm_AddIcon(sid);

}
IconList::~IconList()
{
	mir_free(m_StatusIconData.szModule);
	mir_free(m_StatusIconData.szTooltip);
}

// const char *pszName;			// [Optional] Name of an icon registered with icolib to be used in GUI.
static __inline int MessageAPI_AddIcon(const char* pszName, const char* szModul/*StatusIconData *sid*/,int ID, int flags, const char* szTooltip)
{
	HICON hIcon = Skin_GetIcon(pszName);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = (char*)szModul;
	sid.dwId = (DWORD)ID;
	sid.hIcon = (hIcon!=NULL)?CopyIcon(hIcon):NULL;
	sid.szTooltip = Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,ID,0));
	Skin_ReleaseIcon(hIcon); /* does NULL check */

	int res = -1;
	IconList* p = new IconList(&sid);
	if (!p->m_ID)delete p;
	else res = gIListMW.insert(p);
	if (res == -1)delete p;
	return res;
}

void CALLBACK UpdateStatusIcons(LPARAM lParam) {
	if (!lParam) {
		/* enum all opened message windows */
		for (int i = 0; i < gMsgWndList.getCount(); i++)
			gMsgWndList[i]->FlagsIconUpdate();
	}
	else {
		int i = gMsgWndList.getIndex((MsgWndData*)&lParam);
		if (i!= -1) gMsgWndList[i]->FlagsIconUpdate();
	}
}

//hookProc ME_MSG_WINDOWEVENT
static int OnMsgWndEvent(WPARAM wParam,LPARAM lParam) {
	MessageWindowEventData *msgwe=(MessageWindowEventData*)lParam;
	/* sanity check */
	if (msgwe->hContact==NULL)
		return 0;

	switch(msgwe->uType) {
		case MSG_WINDOW_EVT_OPENING:
			{
				MsgWndData* msgwnd = gMsgWndList.find((MsgWndData*)&msgwe->hContact);
				if (msgwnd == NULL) {
					msgwnd = new MsgWndData(msgwe->hwndWindow, msgwe->hContact);
					gMsgWndList.insert(msgwnd);
				}
			}
			break;

		case MSG_WINDOW_EVT_CLOSE:
			{
				int i = gMsgWndList.getIndex((MsgWndData*)&msgwe->hContact);
				if (i != -1) {
					delete gMsgWndList[i];
					gMsgWndList.remove(i);
				}
			}
			break;
	}
	return 0;
}

//hookProc ME_SKIN2_ICONSCHANGED
static int OnStatusIconsChanged(WPARAM wParam,LPARAM lParam)
{
	if (gFlagsOpts.bShowStatusIconFlag)
		CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
	return 0;
}

/***********************************************************************************************************
 * option page (not used yet)
 ***********************************************************************************************************/
static INT_PTR CALLBACK ExtraImgOptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	return FALSE;
}

//hookProc ME_OPT_INITIALISE
static int ExtraImgOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	return 0;
}

/***********************************************************************************************************
 * misc functions
 ***********************************************************************************************************/
//hookProc ME_DB_CONTACT_SETTINGCHANGED
static int OnContactSettingChanged(WPARAM wParam,LPARAM lParam) {
	if ((HANDLE)wParam==NULL) return 0;
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;

	/* user details update */
	if (/*!lstrcmpA(dbcws->szSetting,"RealIP") || */
	    !lstrcmpA(dbcws->szSetting,SET_CONTACT_COUNTRY) ||
	    !lstrcmpA(dbcws->szSetting,SET_CONTACT_ORIGIN_COUNTRY) ||
	    !lstrcmpA(dbcws->szSetting,SET_CONTACT_COMPANY_COUNTRY))
		{
		/* Extra Image */
		CallFunctionBuffered(SetExtraImage,(LPARAM)wParam,TRUE,EXTRAIMAGE_REFRESHDELAY);
		/* Status Icon */
		if (hMsgWndEventHook) {
			int i = gMsgWndList.getIndex((MsgWndData*)&wParam);
			if (i != -1) {
				gMsgWndList[i]->ContryIDchange((int)ServiceDetectContactOriginCountry(wParam,0));
				gMsgWndList[i]->FlagsIconUpdate();
			}
		}
	}
	return 0;
}


/***********************************************************************************************************
 * module loading & unloading
 ***********************************************************************************************************/

/**
 * This function initially loads all required stuff for Flags.
 *
 * @param	none
 *
 * @return	nothing
 **/
void SvcFlagsLoadModule()
{
	PrepareBufferedFunctions();
	if (CallService(MS_UTILS_GETCOUNTRYLIST,(WPARAM)&nCountriesCount,(LPARAM)&countries))
		nCountriesCount=0;
	InitIcons();			/* load in iconlib */
	//InitIpToCountry();	/* not implementet */
	CreateServiceFunction(MS_FLAGS_DETECTCONTACTORIGINCOUNTRY,ServiceDetectContactOriginCountry);
	//init settings
	gFlagsOpts.bShowExtraImgFlag   = db_get_b(NULL, MODNAMEFLAGS, "ShowExtraImgFlag", SETTING_SHOWEXTRAIMGFLAG_DEFAULT);
	gFlagsOpts.bUseUnknownFlag     = db_get_b(NULL, MODNAMEFLAGS, "UseUnknownFlag", SETTING_USEUNKNOWNFLAG_DEFAULT);
	gFlagsOpts.bShowStatusIconFlag = db_get_b(NULL, MODNAMEFLAGS, "ShowStatusIconFlag", SETTING_SHOWSTATUSICONFLAG_DEFAULT);

	hOptInitHook		= HookEvent(ME_OPT_INITIALISE,ExtraImgOptInit);
	hIconsChangedHook	= HookEvent(ME_SKIN2_ICONSCHANGED,OnStatusIconsChanged);
}

/**
 * This function is called by Miranda just after loading all system modules.
 *
 * @param	none
 *
 * @return	nothing
 **/
void SvcFlagsOnModulesLoaded() 
{
	SvcFlagsEnableExtraIcons(db_get_b(NULL, MODNAME, SET_CLIST_EXTRAICON_FLAGS2, 0), FALSE);

	/* Status Icon */
	hMsgWndEventHook = HookEvent(ME_MSG_WINDOWEVENT, OnMsgWndEvent);
}

/**
 * This function unloads the module.
 *
 * @param	none
 *
 * @return	nothing
 **/
void SvcFlagsUnloadModule() {
	KillBufferedFunctions();
	//Uninit ExtraImg
	UnhookEvent(hRebuildIconsHook);
	UnhookEvent(hApplyIconHook);
	UnhookEvent(hIconsChangedHook);
	//Uninit message winsow
	UnhookEvent(hMsgWndEventHook);
	for(int i = 0; i < gMsgWndList.getCount(); i++) {
		//this should not happen
		delete gMsgWndList[i];
		gMsgWndList.remove(i);
	}
	gMsgWndList.destroy();
	gIListMW.destroy();
	//Uninit misc
	UnhookEvent(hSettingChangedHook);
	UnhookEvent(hOptInitHook);
	UninitIcons();

	//UninitIpToCountry();			/* not implementet */
}

