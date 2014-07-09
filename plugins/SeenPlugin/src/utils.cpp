/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "seen.h"

void FileWrite(MCONTACT);
void HistoryWrite(MCONTACT hcontact);

extern HANDLE g_hShutdownEvent;
char * courProtoName = 0;

/*
Returns true if the protocols is to be monitored
*/
int IsWatchedProtocol(const char* szProto)
{
	if (szProto == NULL)
		return 0;
	
	PROTOACCOUNT *pd = ProtoGetAccount(szProto);
	if (pd == NULL || CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
		return 0;

	int iProtoLen = (int)strlen(szProto);
	char *szWatched;
	DBVARIANT dbv;
	if ( db_get_s(NULL, S_MOD, "WatchedProtocols", &dbv))
		szWatched = DEFAULT_WATCHEDPROTOCOLS;
	else {
		szWatched = NEWSTR_ALLOCA(dbv.pszVal);
		db_free(&dbv);
	}

	if (*szWatched == '\0') 
		return 1; //empty string: all protocols are watched

	char sTemp[MAXMODULELABELLENGTH+1];
	mir_snprintf(sTemp, SIZEOF(sTemp), "%s ", szProto);
	return strstr(szWatched, sTemp) != NULL;
}

BOOL isYahoo(char *protoname)
{
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting)
			return !strcmp(pszUniqueSetting, "yahoo_id");
	}
	return FALSE;
}

BOOL isJabber(char *protoname)
{
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting)
			return !strcmp(pszUniqueSetting, "jid");
	}
	return FALSE;
}

BOOL isICQ(char *protoname)
{
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting)
			return !strcmp(pszUniqueSetting, "UIN");
	}
	return FALSE;
}

BOOL isMSN(char *protoname)
{
	if (protoname) {
		char *pszUniqueSetting = (char*)CallProtoService(protoname, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (pszUniqueSetting)
			return !strcmp(pszUniqueSetting, "e-mail");
	}
	return FALSE;
}

DWORD isSeen(MCONTACT hcontact, SYSTEMTIME *st)
{
	FILETIME ft;
	ULONGLONG ll;
	DWORD res = db_get_dw(hcontact,S_MOD,"seenTS",0);
	if (res) {
		if (st) {
			ll = UInt32x32To64(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,res,0), 10000000) + NUM100NANOSEC;
			ft.dwLowDateTime = (DWORD)ll;
			ft.dwHighDateTime = (DWORD)(ll >> 32);
			FileTimeToSystemTime(&ft, st);
		}
		return res;
	}

	SYSTEMTIME lst;
	ZeroMemory(&lst,sizeof(lst));
	if (lst.wYear = db_get_w(hcontact,S_MOD,"Year",0)) {
		if (lst.wMonth = db_get_w(hcontact,S_MOD,"Month",0)) {
			if (lst.wDay = db_get_w(hcontact,S_MOD,"Day",0)) {
				lst.wDayOfWeek = db_get_w(hcontact,S_MOD,"WeekDay",0);
				lst.wHour = db_get_w(hcontact,S_MOD,"Hours",0);
				lst.wMinute = db_get_w(hcontact,S_MOD,"Minutes",0);
				lst.wSecond = db_get_w(hcontact,S_MOD,"Seconds",0);
				if (SystemTimeToFileTime(&lst,&ft)) {
					ll = ((LONGLONG)ft.dwHighDateTime<<32)|((LONGLONG)ft.dwLowDateTime);
					ll -= NUM100NANOSEC;
					ll /= 10000000;
					//perform LOCALTOTIMESTAMP
					res = (DWORD)ll - CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,0,0);
					//nevel look for Year/Month/Day/Hour/Minute/Second again
					db_set_dw(hcontact,S_MOD,"seenTS",res);
				}
	}	}	}

	if (st)
		CopyMemory (st, &lst, sizeof (SYSTEMTIME));

	return res;
}

TCHAR *weekdays[] = { LPGENT("Sunday"), LPGENT("Monday"), LPGENT("Tuesday"), LPGENT("Wednesday"), LPGENT("Thursday"), LPGENT("Friday"), LPGENT("Saturday") };
TCHAR *wdays_short[] = { LPGENT("Sun."), LPGENT("Mon."), LPGENT("Tue."), LPGENT("Wed."), LPGENT("Thu."), LPGENT("Fri."), LPGENT("Sat.") };
TCHAR *monthnames[] = { LPGENT("January"), LPGENT("February"), LPGENT("March"), LPGENT("April"), LPGENT("May"), LPGENT("June"), LPGENT("July"), LPGENT("August"), LPGENT("September"), LPGENT("October"), LPGENT("November"), LPGENT("December") };
TCHAR *mnames_short[] = { LPGENT("Jan."), LPGENT("Feb."), LPGENT("Mar."), LPGENT("Apr."), LPGENT("May"), LPGENT("Jun."), LPGENT("Jul."), LPGENT("Aug."), LPGENT("Sep."), LPGENT("Oct."), LPGENT("Nov."), LPGENT("Dec.") };

TCHAR *ParseString(TCHAR *szstring, MCONTACT hcontact, BYTE isfile)
{
#define MAXSIZE 1024
	static TCHAR sztemp[MAXSIZE+1];
	TCHAR szdbsetting[128];
	TCHAR *charPtr;
	int isetting=0;
	DWORD dwsetting=0;
	struct in_addr ia;
	DBVARIANT dbv;

	sztemp[0] = '\0';

	SYSTEMTIME st;
	if ( !isSeen(hcontact, &st)) {
		_tcscat(sztemp, TranslateT("<never seen>"));
		return sztemp;
	}

	CONTACTINFO ci = { sizeof(CONTACTINFO) };
	ci.hContact = hcontact;
	ci.szProto = hcontact ? GetContactProto(hcontact) : courProtoName;

	TCHAR *d = sztemp;
	for (TCHAR *p = szstring; *p; p++) {
		if (d >= sztemp + MAXSIZE)
			break;

		if (*p != '%' && *p !='#') {
			*d++ = *p;
			continue;
		}

		bool wantempty = *p =='#';
		switch(*++p) {
		case 'Y':
			if (!st.wYear) goto LBL_noData;
			d += _stprintf(d, _T("%04i"), st.wYear); //!!!!!!!!!!!!
			break;

		case 'y':
			if (!st.wYear) goto LBL_noData;
			d += _stprintf(d, _T("%02i"), st.wYear % 100); //!!!!!!!!!!!!
			break;

		case 'm':
			if (!(isetting=st.wMonth)) goto LBL_noData;
LBL_2DigNum:
			d += _stprintf(d, _T("%02i"), isetting); //!!!!!!!!!!!!
			break;

		case 'd':
			if (isetting = st.wDay) goto LBL_2DigNum;
			else goto LBL_noData;

		case 'W':
			isetting = st.wDayOfWeek;
			if (isetting == -1){
LBL_noData:
				charPtr = wantempty ? _T("") : TranslateT("<unknown>");
				goto LBL_charPtr;
			}
			charPtr = TranslateTS(weekdays[isetting]);
LBL_charPtr:
			d += mir_sntprintf(d, MAXSIZE-(d-sztemp), _T("%s"), charPtr);
			break;

		case 'w':
			isetting = st.wDayOfWeek;
			if (isetting == -1) goto LBL_noData;
			charPtr = TranslateTS( wdays_short[isetting] );
			goto LBL_charPtr;

		case 'E':
			if ( !(isetting = st.wMonth)) goto LBL_noData;
			charPtr = TranslateTS( monthnames[isetting-1] );
			goto LBL_charPtr;

		case 'e':
			if ( !(isetting = st.wMonth)) goto LBL_noData;
			charPtr = TranslateTS( mnames_short[isetting-1] );
			goto LBL_charPtr;

		case 'H':
			if ((isetting = st.wHour) == -1) goto LBL_noData;
			goto LBL_2DigNum;

		case 'h':
			if ((isetting = st.wHour) == -1) goto LBL_noData;
			if (!isetting) isetting = 12;
			isetting = isetting-((isetting>12)?12:0);
			goto LBL_2DigNum;

		case 'p':
			if ((isetting = st.wHour) == -1) goto LBL_noData;
			charPtr = (isetting >= 12) ? _T("PM") : _T("AM");
			goto LBL_charPtr;

		case 'M':
			if ((isetting = st.wMinute) == -1) goto LBL_noData;
			goto LBL_2DigNum;

		case 'S':
			if ((isetting = st.wHour) == -1) goto LBL_noData;
			goto LBL_2DigNum;

		case 'n':
			charPtr = hcontact ? (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hcontact, GCDNF_TCHAR) : (wantempty ? _T("") : _T("---"));
			goto LBL_charPtr;

		case 'N':
			ci.dwFlag = CNF_NICK | CNF_TCHAR;
			if ( !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
				charPtr = ci.pszVal;
				goto LBL_charPtr;
			}
			goto LBL_noData;

		case 'G':
			if ( !db_get_ts(hcontact, "CList", "Group", &dbv)) {
				_tcsncpy(szdbsetting, dbv.ptszVal, SIZEOF(szdbsetting));
				db_free(&dbv);
				charPtr = szdbsetting;
				goto LBL_charPtr;
			}
			break;

		case 'u':
			ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
			if ( !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
				switch(ci.type) {
				case CNFT_BYTE:
					_ltot(ci.bVal, szdbsetting, 10);
					break;
				case CNFT_WORD:
					_ltot(ci.wVal, szdbsetting, 10);
					break;
				case CNFT_DWORD:
					_ltot(ci.dVal, szdbsetting, 10);
					break;
				case CNFT_ASCIIZ:
					_tcsncpy(szdbsetting, ci.pszVal, SIZEOF(szdbsetting));
					break;
				}
			}
			else goto LBL_noData;
			charPtr = szdbsetting;
			goto LBL_charPtr;

		case 's':
			if (isetting = db_get_w(hcontact,S_MOD,hcontact ? "StatusTriger" : courProtoName, 0)) {
				_tcsncpy(szdbsetting, (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)(isetting|0x8000), GSMDF_TCHAR), SIZEOF(szdbsetting));
				if ( !(isetting & 0x8000)) {
					_tcsncat(szdbsetting, _T("/"), SIZEOF(szdbsetting));
					_tcsncat(szdbsetting, TranslateT("Idle"), SIZEOF(szdbsetting));
				}
				charPtr = szdbsetting;
				goto LBL_charPtr;
			}
			goto LBL_noData;

		case 'T':
			if ( db_get_ts(hcontact, "CList", "StatusMsg", &dbv))
				goto LBL_noData;

			d += mir_sntprintf(d, MAXSIZE-(d-sztemp), _T("%s"), dbv.ptszVal);
			db_free(&dbv);
			break;

		case 'o':
			if (isetting = db_get_w(hcontact, S_MOD, hcontact ? "OldStatus" : courProtoName, 0)) {
				_tcsncpy(szdbsetting, (TCHAR*) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)isetting, GSMDF_TCHAR), SIZEOF(szdbsetting));
				if (includeIdle && hcontact && db_get_b(hcontact, S_MOD, "OldIdle", 0)) {
					_tcsncat(szdbsetting, _T("/"), SIZEOF(szdbsetting));
					_tcsncat(szdbsetting, TranslateT("Idle"), SIZEOF(szdbsetting));
				}
				charPtr = szdbsetting;
				goto LBL_charPtr;
			}
			goto LBL_noData;

		case 'i':
		case 'r':
			if ( isJabber(ci.szProto)) {
				if ( db_get_ts(hcontact, ci.szProto, *p == 'i' ? "Resource" : "System", &dbv))
					goto LBL_noData;

				_tcsncpy(szdbsetting, dbv.ptszVal, SIZEOF(szdbsetting));
				db_free(&dbv);
				charPtr = szdbsetting;
			}
			else {
				dwsetting = db_get_dw(hcontact, ci.szProto, *p == 'i' ? "IP" : "RealIP", 0);
				if (!dwsetting)
					goto LBL_noData;

				ia.S_un.S_addr = htonl(dwsetting);
				_tcsncpy(szdbsetting, _A2T( inet_ntoa(ia)), SIZEOF(szdbsetting));
				charPtr = szdbsetting;
			}
			goto LBL_charPtr;

		case 'P':
			_tcsncpy(szdbsetting, ci.szProto ? _A2T(ci.szProto) : (wantempty ? _T("") : _T("ProtoUnknown")), SIZEOF(szdbsetting));
			charPtr = szdbsetting;
			goto LBL_charPtr;

		case 'b':
			charPtr = _T("\x0D\x0A");
			goto LBL_charPtr;

		case 'C': // Get Client Info
			if ( !db_get_ts(hcontact, ci.szProto, "MirVer", &dbv)) {
				_tcsncpy(szdbsetting, dbv.ptszVal, SIZEOF(szdbsetting));
				db_free(&dbv);
			}
			else goto LBL_noData;
			charPtr = szdbsetting;
			goto LBL_charPtr;

		case 't':
			charPtr = _T("\t");
			goto LBL_charPtr;

		case 'A':
			{
			PROTOACCOUNT *pa = ProtoGetAccount(ci.szProto);
			if(!pa) goto LBL_noData;
			_tcsncpy(szdbsetting, pa->tszAccountName, SIZEOF(szdbsetting));
			charPtr = szdbsetting;
			goto LBL_charPtr;
			}
			

		default:
			*d++ = p[-1];
			*d++ = *p;
		}
	}

	*d = 0;
	return sztemp;
}

void _DBWriteTime(SYSTEMTIME *st, MCONTACT hcontact)
{
	db_set_w(hcontact,S_MOD,"Day",st->wDay);
	db_set_w(hcontact,S_MOD,"Month",st->wMonth);
	db_set_w(hcontact,S_MOD,"Year",st->wYear);
	db_set_w(hcontact,S_MOD,"Hours",st->wHour);
	db_set_w(hcontact,S_MOD,"Minutes",st->wMinute);
	db_set_w(hcontact,S_MOD,"Seconds",st->wSecond);
	db_set_w(hcontact,S_MOD,"WeekDay",st->wDayOfWeek);

}

void DBWriteTimeTS(DWORD t, MCONTACT hcontact){
	SYSTEMTIME st;
	FILETIME ft;
	ULONGLONG ll = UInt32x32To64(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,t,0), 10000000) + NUM100NANOSEC;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = (DWORD)(ll >> 32);
	FileTimeToSystemTime(&ft, &st);
	db_set_dw(hcontact,S_MOD,"seenTS",t);
	_DBWriteTime(&st, hcontact);
}
void GetColorsFromDWord(LPCOLORREF First, LPCOLORREF Second, DWORD colDword){
	WORD temp;
	COLORREF res=0;
	temp = (WORD)(colDword>>16);
	res |= ((temp & 0x1F) <<3);
	res |= ((temp & 0x3E0) <<6);
	res |= ((temp & 0x7C00) <<9);
	if (First) *First = res;
	res = 0;
	temp = (WORD)colDword;
	res |= ((temp & 0x1F) <<3);
	res |= ((temp & 0x3E0) <<6);
	res |= ((temp & 0x7C00) <<9);
	if (Second) *Second = res;
}

DWORD StatusColors15bits[] = {
	0x63180000, // 0x00C0C0C0, 0x00000000, Offline - LightGray
	0x7B350000, // 0x00F0C8A8, 0x00000000, Online  - LightBlue
	0x33fe0000, // 0x0070E0E0, 0x00000000, Away -LightOrange
	0x295C0000, // 0x005050E0, 0x00000000, DND  -DarkRed
	0x5EFD0000, // 0x00B8B8E8, 0x00000000, NA   -LightRed
	0x295C0000, // 0x005050E0, 0x00000000, Occupied
	0x43900000, // 0x0080E080, 0x00000000, Free for chat - LightGreen
	0x76AF0000, // 0x00E8A878, 0x00000000, Invisible
	0x431C0000, // 0x0080C0E0, 0x00000000, On the phone
	0x5EFD0000, // 0x00B8B8E8, 0x00000000, Out to lunch
};

DWORD GetDWordFromColors(COLORREF First, COLORREF Second){
	DWORD res = 0;
	res |= (First&0xF8)>>3;
	res |= (First&0xF800)>>6;
	res |= (First&0xF80000)>>9;
	res <<= 16;
	res |= (Second&0xF8)>>3;
	res |= (Second&0xF800)>>6;
	res |= (Second&0xF80000)>>9;
	return res;
}

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch(message) {
		case WM_COMMAND: 
			if (HIWORD(wParam) == STN_CLICKED){
				MCONTACT hContact = PUGetContact(hwnd);
				if (hContact > 0) CallService(MS_MSG_SENDMESSAGE,hContact,0);
			}
		case WM_CONTEXTMENU:
			PUDeletePopup(hwnd);
			break;
		case UM_INITPOPUP: return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
};

void ShowPopup(MCONTACT hcontact, const char * lpzProto, int newStatus)
{
	if ( CallService(MS_IGNORE_ISIGNORED, (WPARAM)hcontact, IGNOREEVENT_USERONLINE))
		return;

	if ( !ServiceExists(MS_POPUP_QUERY))
		return;

	if ( !db_get_b(NULL, S_MOD, "UsePopups", 0) || !db_get_b(hcontact, "CList", "Hidden", 0))
		return;

	DBVARIANT dbv;
	char szSetting[10];
	mir_snprintf(szSetting, SIZEOF(szSetting), "Col_%d", newStatus - ID_STATUS_OFFLINE);
	DWORD sett = db_get_dw(NULL, S_MOD, szSetting, StatusColors15bits[newStatus-ID_STATUS_OFFLINE]);

	POPUPDATAT ppd = {0};
	GetColorsFromDWord(&ppd.colorBack,&ppd.colorText,sett);

	ppd.lchContact = hcontact;
	ppd.lchIcon = LoadSkinnedProtoIcon(lpzProto, newStatus);

	if ( !db_get_ts(NULL, S_MOD, "PopupStamp", &dbv)) {
		_tcsncpy(ppd.lptzContactName, ParseString(dbv.ptszVal, hcontact, 0), MAX_CONTACTNAME);
		db_free(&dbv);
	}
	else _tcsncpy(ppd.lptzContactName, ParseString(DEFAULT_POPUPSTAMP, hcontact, 0), MAX_CONTACTNAME);

	if ( !db_get_ts(NULL, S_MOD, "PopupStampText", &dbv)) { 
		_tcsncpy(ppd.lptzText, ParseString(dbv.ptszVal, hcontact, 0), MAX_SECONDLINE);
		db_free(&dbv);
	}
	else _tcsncpy(ppd.lptzText, ParseString(DEFAULT_POPUPSTAMPTEXT, hcontact, 0), MAX_SECONDLINE);
	ppd.PluginWindowProc = PopupDlgProc;
	PUAddPopupT(&ppd);
}

void myPlaySound(MCONTACT hcontact, WORD newStatus, WORD oldStatus)
{
	if (CallService(MS_IGNORE_ISIGNORED,(WPARAM)hcontact,IGNOREEVENT_USERONLINE)) return;
	//oldStatus and hcontact are not used yet
	char *soundname = NULL;
	if ((newStatus == ID_STATUS_ONLINE) || (newStatus == ID_STATUS_FREECHAT)) soundname = "LastSeenTrackedStatusOnline";
	else if (newStatus == ID_STATUS_OFFLINE) soundname = "LastSeenTrackedStatusOffline";
	else if (oldStatus == ID_STATUS_OFFLINE) soundname = "LastSeenTrackedStatusFromOffline";
	else soundname = "LastSeenTrackedStatusChange";
	if (soundname != NULL)
		SkinPlaySound(soundname);
}

//will add hContact to queue and will return position;
static void waitThread(void *param)
{
	logthread_info* infoParam = (logthread_info*)param;

	WORD prevStatus = db_get_w(infoParam->hContact,S_MOD,"StatusTriger",ID_STATUS_OFFLINE);
	
	// I hope in 1.5 second all the needed info will be set
	if ( WaitForSingleObject(g_hShutdownEvent, 1500) == WAIT_TIMEOUT) {
		if (includeIdle)
			if (db_get_dw(infoParam->hContact,infoParam->sProtoName,"IdleTS",0))
				infoParam->currStatus &=0x7FFF;

		if (infoParam->currStatus != prevStatus){
			db_set_w(infoParam->hContact,S_MOD,"OldStatus",(WORD)(prevStatus|0x8000));
			if (includeIdle)
				db_set_b(infoParam->hContact,S_MOD,"OldIdle",(BYTE)((prevStatus&0x8000)==0));

			db_set_w(infoParam->hContact,S_MOD,"StatusTriger",infoParam->currStatus);
		}
	}
	{
		mir_cslock lck(csContacts);
		arContacts.remove(infoParam);
	}
	mir_free(infoParam);
}

int UpdateValues(WPARAM hContact,LPARAM lparam)
{
	// to make this code faster
	if (!hContact)
		return 0;

	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING *)lparam;
	//if (CallService(MS_IGNORE_ISIGNORED,hContact,IGNOREEVENT_USERONLINE)) return 0;
	BOOL isIdleEvent = includeIdle?(strcmp(cws->szSetting,"IdleTS")==0):0;
	if (strcmp(cws->szSetting,"Status") && strcmp(cws->szSetting,"StatusTriger") && (isIdleEvent==0)) return 0;
	if (!strcmp(cws->szModule,S_MOD)) {
		//here we will come when Settings/SeenModule/StatusTriger is changed
		WORD prevStatus=db_get_w(hContact, S_MOD, "OldStatus", ID_STATUS_OFFLINE);
		if (includeIdle){
			if ( db_get_b(hContact, S_MOD, "OldIdle", 0)) prevStatus &= 0x7FFF;
			else prevStatus |= 0x8000;
		}
		if ((cws->value.wVal|0x8000)<=ID_STATUS_OFFLINE)
		{
			// avoid repeating the offline status
			if ((prevStatus|0x8000)<=ID_STATUS_OFFLINE)
				return 0;

			char *proto = GetContactProto(hContact);
			db_set_b(hContact, S_MOD, "Offline", 1);
			{
				DWORD t;
				char *str = (char *)malloc(MAXMODULELABELLENGTH+9);
				mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",proto);
				t = db_get_dw(NULL,S_MOD,str,0);
				if (!t) t = time(NULL);
				free(str);
				DBWriteTimeTS(t, hContact);
			}

			if (!db_get_b(NULL,S_MOD,"IgnoreOffline",1))
			{
				if ( db_get_b(NULL,S_MOD,"FileOutput",0))
					FileWrite(hContact);

				char *sProto = GetContactProto(hContact);
				if (CallProtoService(sProto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE)	{
					myPlaySound(hContact, ID_STATUS_OFFLINE, prevStatus);
					if ( db_get_b(NULL, S_MOD, "UsePopups", 0))
						ShowPopup(hContact, sProto, ID_STATUS_OFFLINE);
				}

				if ( db_get_b(NULL, S_MOD, "KeepHistory", 0))
					HistoryWrite(hContact);

				if ( db_get_b(hContact, S_MOD, "OnlineAlert", 0)) 
					ShowHistory(hContact, 1);
			}

		} else {

			if (cws->value.wVal==prevStatus && !db_get_b(hContact, S_MOD, "Offline", 0)) 
				return 0;

			DBWriteTimeTS(time(NULL), hContact);

			//db_set_w(hContact,S_MOD,"StatusTriger",(WORD)cws->value.wVal);

			if (db_get_b(NULL, S_MOD, "FileOutput", 0)) FileWrite(hContact);
			if (prevStatus != cws->value.wVal) myPlaySound(hContact, cws->value.wVal, prevStatus);
			if (db_get_b(NULL, S_MOD, "UsePopups", 0))
				if (prevStatus != cws->value.wVal)
					ShowPopup(hContact, GetContactProto(hContact), cws->value.wVal|0x8000);

			if (db_get_b(NULL, S_MOD, "KeepHistory", 0)) HistoryWrite(hContact);
			if (db_get_b(hContact, S_MOD, "OnlineAlert", 0)) ShowHistory(hContact, 1);
			db_set_b(hContact, S_MOD, "Offline", 0);
		}
	}
	else if (hContact && IsWatchedProtocol(cws->szModule) && !db_get_b(hContact, cws->szModule, "ChatRoom", false)) {
		//here we will come when <User>/<module>/Status is changed or it is idle event and if <module> is watched
		if ( CallProtoService(cws->szModule,PS_GETSTATUS,0,0) > ID_STATUS_OFFLINE){
			mir_cslock lck(csContacts);
			logthread_info *p = arContacts.find((logthread_info*)&hContact);
			if (p == NULL) {
				p = (logthread_info*)mir_calloc(sizeof(logthread_info));
				p->hContact = hContact;
				strncpy(p->sProtoName, cws->szModule, MAXMODULELABELLENGTH);
				arContacts.insert(p);
				mir_forkthread(waitThread, p);
			}
			p->currStatus = isIdleEvent ? db_get_w(hContact, cws->szModule, "Status", ID_STATUS_OFFLINE) : cws->value.wVal;
	}	}	

	return 0;
}

static void cleanThread(void *param)
{
	logthread_info* infoParam = (logthread_info*)param;
	char *szProto = infoParam->sProtoName;

	// I hope in 10 secons all logged-in contacts will be listed
	if ( WaitForSingleObject(g_hShutdownEvent, 10000) == WAIT_TIMEOUT) {
		for (MCONTACT hContact = db_find_first(szProto); hContact; hContact = db_find_next(hContact, szProto)) {
			WORD oldStatus = db_get_w(hContact,S_MOD,"StatusTriger",ID_STATUS_OFFLINE) | 0x8000;
			if (oldStatus > ID_STATUS_OFFLINE) {
				if (db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE)==ID_STATUS_OFFLINE){
					db_set_w(hContact,S_MOD,"OldStatus",(WORD)(oldStatus|0x8000));
					if (includeIdle)db_set_b(hContact,S_MOD,"OldIdle",(BYTE)((oldStatus&0x8000)?0:1));
					db_set_w(hContact,S_MOD,"StatusTriger",ID_STATUS_OFFLINE);
				}
			}
		}

		char *str = (char *)malloc(MAXMODULELABELLENGTH+9);
		mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",infoParam->sProtoName);
		db_unset(NULL,S_MOD,str);
		free(str);
	}
	free(infoParam);
}

int ModeChange(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack=(ACKDATA *)lparam;

	if (ack->type!=ACKTYPE_STATUS || ack->result!=ACKRESULT_SUCCESS || ack->hContact!=NULL) return 0;
	courProtoName = (char *)ack->szModule;
	if (!IsWatchedProtocol(courProtoName) && strncmp(courProtoName,"MetaContacts",12)) 
		return 0;

	DBWriteTimeTS(time(NULL),NULL);

//	isetting=CallProtoService(ack->szModule,PS_GETSTATUS,0,0);
	WORD isetting=(WORD)ack->lParam;
	if (isetting<ID_STATUS_OFFLINE) isetting = ID_STATUS_OFFLINE;
	if ((isetting>ID_STATUS_OFFLINE)&&((WORD)ack->hProcess<=ID_STATUS_OFFLINE)) {
		//we have just loged-in
		db_set_dw(NULL, "UserOnline", ack->szModule, GetTickCount());
		if (!Miranda_Terminated() && IsWatchedProtocol(ack->szModule)) {
			logthread_info *info = (logthread_info *)malloc(sizeof(logthread_info));
			strncpy(info->sProtoName,courProtoName,MAXMODULELABELLENGTH);
			info->hContact = 0;
			info->currStatus = 0;

			mir_forkthread(cleanThread, info);
		}
	}
	else if ((isetting==ID_STATUS_OFFLINE)&&((WORD)ack->hProcess>ID_STATUS_OFFLINE)) {
		//we have just loged-off
		if (IsWatchedProtocol(ack->szModule)) {
			char *str = (char *)malloc(MAXMODULELABELLENGTH+9);
			time_t t;
			time(&t);
			mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",ack->szModule);
			db_set_dw(NULL,S_MOD,str,t);
			free(str);
		}
	}
	
	if (isetting==db_get_w(NULL,S_MOD,courProtoName,ID_STATUS_OFFLINE))
		return 0;

	db_set_w(NULL,S_MOD,courProtoName,isetting);

	if ( db_get_b(NULL,S_MOD,"FileOutput",0))
		FileWrite(NULL);

	courProtoName = NULL;
	return 0;
}

short int isDbZero(MCONTACT hContact, const char *module_name, const char *setting_name)
{
	DBVARIANT dbv;
	if ( !db_get(hContact, module_name, setting_name, &dbv)) {
		short int res = 0;
		switch (dbv.type) {
			case DBVT_BYTE: res=dbv.bVal==0; break;
			case DBVT_WORD: res=dbv.wVal==0; break;
			case DBVT_DWORD: res=dbv.dVal==0; break;
			case DBVT_BLOB: res=dbv.cpbVal==0; break;
			default: res=dbv.pszVal[0]==0; break;
		}
		db_free(&dbv); 
		return res;
	}
	return -1;
}

TCHAR *any_to_IdleNotidleUnknown(MCONTACT hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	short int r = isDbZero(hContact, module_name, setting_name);
	if (r==-1){
		_tcsncpy(buff, TranslateT("Unknown"), bufflen);
	} else {
		_tcsncpy(buff, r ? TranslateT("Not Idle") : TranslateT("Idle"), bufflen);
	};
	buff[bufflen - 1] = 0;
	return buff;
}

TCHAR *any_to_Idle(MCONTACT hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	if (isDbZero(hContact, module_name, setting_name)==0) { //DB setting is NOT zero and exists
		buff[0] = L'/';
		_tcsncpy(&buff[1], TranslateT("Idle"), bufflen-1);
	} else buff[0] = 0;
	buff[bufflen - 1] = 0;
	return buff;
}
