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

File name      : $URL: http://svn.berlios.de/svnroot/repos/mgoodies/trunk/lastseen-mod/utils.c $
Revision       : $Rev: 1570 $
Last change on : $Date: 2007-12-30 01:30:07 +0300 (Вс, 30 дек 2007) $
Last change by : $Author: y_b $
*/
#include "seen.h"

void FileWrite(HANDLE);
void HistoryWrite(HANDLE hcontact);
//void SetOffline(void);
void ShowHistory(HANDLE hContact, BYTE isAlert);

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
	if ( DBGetContactSettingString(NULL, S_MOD, "WatchedProtocols", &dbv))
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

DWORD isSeen(HANDLE hcontact, SYSTEMTIME *st)
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
					DBWriteContactSettingDword(hcontact,S_MOD,"seenTS",res);
				}
	}	}	}

	if (st)
		CopyMemory (st, &lst, sizeof (SYSTEMTIME));

	return res;
}

TCHAR *weekdays[] = { _T("Sunday"), _T("Monday"), _T("Tuesday"), _T("Wednesday"), _T("Thursday"), _T("Friday"), _T("Saturday") };
TCHAR *wdays_short[] = { _T("Sun."), _T("Mon."), _T("Tue."), _T("Wed."), _T("Thu."), _T("Fri."), _T("Sat.") };
TCHAR *monthnames[] = { _T("January"), _T("February"), _T("March"), _T("April"), _T("May"), _T("June"), _T("July"), _T("August"), _T("September"), _T("October"), _T("November"), _T("December") };
TCHAR *mnames_short[] = { _T("Jan."), _T("Feb."), _T("Mar."), _T("Apr."), _T("May"), _T("Jun."), _T("Jul."), _T("Aug."), _T("Sep."), _T("Oct."), _T("Nov."), _T("Dec.") };

TCHAR *ParseString(TCHAR *szstring, HANDLE hcontact, BYTE isfile)
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
	ci.szProto = hcontact ? (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0) : courProtoName;

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
			d += _stprintf(d, _T("%04i"), st.wYear);
			break;

		case 'y':
			if (!st.wYear) goto LBL_noData;
			d += _stprintf(d, _T("%02i"), st.wYear % 100);
			break;

		case 'm':
			if (!(isetting=st.wMonth)) goto LBL_noData;
LBL_2DigNum:
			d += _stprintf(d, _T("%02i"), isetting);
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
			if ( !DBGetContactSettingTString(hcontact, "CList", "Group", &dbv)) {
				_tcscpy(szdbsetting, dbv.ptszVal);
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
					_tcscpy(szdbsetting, ci.pszVal);
					break;
				}
			}
			else if (ci.szProto != NULL) {
				if ( isYahoo(ci.szProto)) { // YAHOO support
					DBGetContactSettingTString(hcontact, ci.szProto, "id", &dbv);
					_tcscpy(szdbsetting, dbv.ptszVal);
					db_free(&dbv);
				}
				else if ( isJabber(ci.szProto)) { // JABBER support
					if ( DBGetContactSettingTString(hcontact, ci.szProto, "LoginName", &dbv))
						goto LBL_noData;

					_tcscpy(szdbsetting, dbv.ptszVal);
					db_free(&dbv);
					
					DBGetContactSettingTString(hcontact, ci.szProto, "LoginServer", &dbv);
					_tcscat(szdbsetting, _T("@"));
					_tcscat(szdbsetting, dbv.ptszVal);
					db_free(&dbv);
				}
				else goto LBL_noData;
			}
			else goto LBL_noData;
			charPtr = szdbsetting;
			goto LBL_charPtr;

		case 's':
			if (isetting = db_get_w(hcontact,S_MOD,hcontact ? "StatusTriger" : courProtoName, 0)) {
				_tcscpy(szdbsetting, TranslateTS((TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)(isetting|0x8000), GSMDF_TCHAR)));
				if ( !(isetting & 0x8000)) {
					_tcscat(szdbsetting, _T("/"));
					_tcscat(szdbsetting, TranslateT("Idle"));
				}
				charPtr = szdbsetting;
				goto LBL_charPtr;
			}
			goto LBL_noData;

		case 'T':
			if ( DBGetContactSettingTString(hcontact, "CList", "StatusMsg", &dbv))
				goto LBL_noData;

			d += mir_sntprintf(d, MAXSIZE-(d-sztemp), _T("%s"), dbv.ptszVal);
			db_free(&dbv);
			break;

		case 'o':
			if (isetting = db_get_w(hcontact, S_MOD, hcontact ? "OldStatus" : courProtoName, 0)) {
				_tcscpy(szdbsetting, TranslateTS((TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)isetting, GSMDF_TCHAR)));
				if (includeIdle) if (hcontact) if ( db_get_b(hcontact, S_MOD, "OldIdle", 0)) {
					_tcscat(szdbsetting, _T("/"));
					_tcscat(szdbsetting, TranslateT("Idle"));
				}
				charPtr = szdbsetting;
				goto LBL_charPtr;
			}
			goto LBL_noData;

		case 'i':
		case 'r':
			if ( isJabber(ci.szProto)) {
				if ( DBGetContactSettingTString(hcontact, ci.szProto, *p == 'i' ? "Resource" : "System", &dbv))
					goto LBL_noData;

				_tcscpy(szdbsetting, dbv.ptszVal);
				db_free(&dbv);
				charPtr = szdbsetting;
			}
			else {
				dwsetting = db_get_dw(hcontact, ci.szProto, *p == 'i' ? "IP" : "RealIP", 0);
				if (!dwsetting)
					goto LBL_noData;

				ia.S_un.S_addr = htonl(dwsetting);
				_tcscpy(szdbsetting, _A2T( inet_ntoa(ia)));
				charPtr = szdbsetting;
			}
			goto LBL_charPtr;

		case 'P':
			_tcscpy(szdbsetting, _A2T(ci.szProto ? ci.szProto : (wantempty ? "" : "ProtoUnknown")));
			goto LBL_charPtr;

		case 'b':
			charPtr = _T("x0D\x0A");
			goto LBL_charPtr;

		case 'C': // Get Client Info
			if (isMSN(ci.szProto)) {
				if (hcontact) {
					dwsetting = (int)db_get_dw(hcontact,ci.szProto,"FlagBits",0);
					wsprintf(szdbsetting, _T("MSNC%i"), (dwsetting&0x70000000)>>28);
					if (dwsetting & 0x00000001) _tcscat(szdbsetting, _T(" MobD")); //Mobile Device
					if (dwsetting & 0x00000004) _tcscat(szdbsetting, _T(" InkG")); //GIF Ink Send/Receive
					if (dwsetting & 0x00000008) _tcscat(szdbsetting, _T(" InkI")); //ISF Ink Send/Receive
					if (dwsetting & 0x00000010) _tcscat(szdbsetting, _T(" WCam")); //Webcam
					if (dwsetting & 0x00000020) _tcscat(szdbsetting, _T(" MPkt")); //Multi packet messages
					if (dwsetting & 0x00000040) _tcscat(szdbsetting, _T(" SMSr")); //Paging
					if (dwsetting & 0x00000080) _tcscat(szdbsetting, _T(" DSMS")); //Using MSN Direct
					if (dwsetting & 0x00000200) _tcscat(szdbsetting, _T(" WebM")); //WebMessenger
					if (dwsetting & 0x00001000) _tcscat(szdbsetting, _T(" MS7+")); //Unknown (Msgr 7 always[?] sets it)
					if (dwsetting & 0x00004000) _tcscat(szdbsetting, _T(" DirM")); //DirectIM
					if (dwsetting & 0x00008000) _tcscat(szdbsetting, _T(" Wink")); //Send/Receive Winks
					if (dwsetting & 0x00010000) _tcscat(szdbsetting, _T(" MSrc")); //MSN Search ??
					if (dwsetting & 0x00040000) _tcscat(szdbsetting, _T(" VoiC")); //Voice Clips
				}
				else _tcscpy(szdbsetting, _T("Miranda"));
			}
			else {
				if ( !DBGetContactSettingTString(hcontact, ci.szProto, "MirVer", &dbv)) {
					_tcscpy(szdbsetting, dbv.ptszVal);
					db_free(&dbv);
				}
				else goto LBL_noData;
			}
			charPtr = szdbsetting;
			goto LBL_charPtr;

		case 't':
			charPtr = _T("\t");
			goto LBL_charPtr;

		default:
			_tcsncpy(szdbsetting, p-1, 2);
			goto LBL_charPtr;
		}
	}

	*d = 0;
	return sztemp;
}

void _DBWriteTime(SYSTEMTIME *st,HANDLE hcontact)
{
	db_set_w((HANDLE)hcontact,S_MOD,"Day",st->wDay);
	db_set_w((HANDLE)hcontact,S_MOD,"Month",st->wMonth);
	db_set_w((HANDLE)hcontact,S_MOD,"Year",st->wYear);
	db_set_w((HANDLE)hcontact,S_MOD,"Hours",st->wHour);
	db_set_w((HANDLE)hcontact,S_MOD,"Minutes",st->wMinute);
	db_set_w((HANDLE)hcontact,S_MOD,"Seconds",st->wSecond);
	db_set_w((HANDLE)hcontact,S_MOD,"WeekDay",st->wDayOfWeek);

}

void DBWriteTimeTS(DWORD t, HANDLE hcontact){
	SYSTEMTIME st;
	FILETIME ft;
	ULONGLONG ll = UInt32x32To64(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,t,0), 10000000) + NUM100NANOSEC;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = (DWORD)(ll >> 32);
	FileTimeToSystemTime(&ft, &st);
	DBWriteContactSettingDword(hcontact,S_MOD,"seenTS",t);
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
				HANDLE hContact = PUGetContact(hwnd);
				if (hContact > 0) CallService(MS_MSG_SENDMESSAGE,(WPARAM)hContact,0);
			}
		case WM_CONTEXTMENU:
			PUDeletePopUp(hwnd);
			break;
		case UM_INITPOPUP: return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
};

void ShowPopup(HANDLE hcontact, const char * lpzProto, int newStatus)
{
	if ( CallService(MS_IGNORE_ISIGNORED, (WPARAM)hcontact, IGNOREEVENT_USERONLINE))
		return;

	if ( !ServiceExists(MS_POPUP_QUERY))
		return;

	if ( !db_get_b(NULL, S_MOD, "UsePopups", 0) || !db_get_b(hcontact, "CList", "Hidden", 0))
		return;

	DBVARIANT dbv;
	char szSetting[10];
	sprintf(szSetting, "Col_%d",newStatus-ID_STATUS_OFFLINE);
	DWORD sett = db_get_dw(NULL, S_MOD, szSetting, StatusColors15bits[newStatus-ID_STATUS_OFFLINE]);

	POPUPDATAT ppd = {0};
	GetColorsFromDWord(&ppd.colorBack,&ppd.colorText,sett);

	ppd.lchContact = hcontact;
	ppd.lchIcon = LoadSkinnedProtoIcon(lpzProto, newStatus);

	if ( !DBGetContactSettingTString(NULL, S_MOD, "PopupStamp", &dbv)) {
		_tcsncpy(ppd.lptzContactName, ParseString(dbv.ptszVal, hcontact, 0), MAX_CONTACTNAME);
		db_free(&dbv);
	}
	else _tcsncpy(ppd.lptzContactName, ParseString(DEFAULT_POPUPSTAMP, hcontact, 0), MAX_CONTACTNAME);

	if ( !DBGetContactSettingTString(NULL, S_MOD, "PopupStampText", &dbv)) { 
		_tcsncpy(ppd.lptzText, ParseString(dbv.ptszVal, hcontact, 0), MAX_SECONDLINE);
		db_free(&dbv);
	}
	else _tcsncpy(ppd.lptzText, ParseString(DEFAULT_POPUPSTAMPTEXT, hcontact, 0), MAX_SECONDLINE);
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
}

void myPlaySound(HANDLE hcontact, WORD newStatus, WORD oldStatus){
	if (CallService(MS_IGNORE_ISIGNORED,(WPARAM)hcontact,IGNOREEVENT_USERONLINE)) return;
	//oldStatus and hcontact are not used yet
	if ( db_get_b(NULL,"Skin","UseSound",1)) {
		char * soundname=0;
		if ((newStatus==ID_STATUS_ONLINE) || (newStatus==ID_STATUS_FREECHAT)) soundname = "LastSeenTrackedStatusOnline";
		else if (newStatus==ID_STATUS_OFFLINE) soundname = "LastSeenTrackedStatusOffline";
		else if (oldStatus==ID_STATUS_OFFLINE) soundname = "LastSeenTrackedStatusFromOffline";
		else soundname = "LastSeenTrackedStatusChange";
		if (!db_get_b(NULL,"SkinSoundsOff",soundname,0)) {
			DBVARIANT dbv;
			if ( !DBGetContactSettingTString(NULL, "SkinSounds", soundname, &dbv)) {
				PlaySound(dbv.ptszVal, NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
				db_free(&dbv);
}	}	}	}

//will give hContact position or zero
int isContactQueueActive(HANDLE hContact){
	int i = 0;
	if (!hContact) {
//		MessageBox(0,"Is myself in the queue: never","LastSeen-Mod",0);
		return 0;
	}
	for (i=1;i<contactQueueSize;i++) {
		if (contactQueue[i])
			if (contactQueue[i]->hContact==hContact) return i;
	}
	return 0;
}

//will add hContact to queue and will return position;
int addContactToQueue(HANDLE hContact){
	int i = 0;
	if (!hContact) {
//		MessageBox(0,"Adding myself to queue","LastSeen-Mod",0);
		return 0;
	}
	for (i=1;i<contactQueueSize;i++) {
		if (!contactQueue[i]) {
			contactQueue[i] = (logthread_info *)malloc(sizeof(logthread_info));
			contactQueue[i]->queueIndex = i;
			contactQueue[i]->hContact = hContact;
			return i;
		}
	}
	//no free space. Create some
	//MessageBox(0,"Creating more space","LastSeen-Mod",0);
	contactQueue = (logthread_info **)realloc(contactQueue,(contactQueueSize+16)*sizeof(logthread_info *));
	memset(&contactQueue[contactQueueSize],0, 16*sizeof(logthread_info *));
	i = contactQueueSize;
	contactQueue[i] = (logthread_info *)malloc(sizeof(logthread_info));
	contactQueue[i]->queueIndex = i;
	contactQueue[i]->hContact = hContact;
	contactQueueSize += 16;
	return i;
}

static DWORD __stdcall waitThread(logthread_info* infoParam)
{
	WORD prevStatus = db_get_w(infoParam->hContact,S_MOD,"StatusTriger",ID_STATUS_OFFLINE);
	Sleep(1500); // I hope in 1.5 second all the needed info will be set
	if (includeIdle)
		if (db_get_dw(infoParam->hContact,infoParam->sProtoName,"IdleTS",0))
			infoParam->courStatus &=0x7FFF;

	if (infoParam->courStatus != prevStatus){
		db_set_w(infoParam->hContact,S_MOD,"OldStatus",(WORD)(prevStatus|0x8000));
		if (includeIdle)
			db_set_b(infoParam->hContact,S_MOD,"OldIdle",(BYTE)((prevStatus&0x8000)==0));

		db_set_w(infoParam->hContact,S_MOD,"StatusTriger",infoParam->courStatus);
	}

	contactQueue[infoParam->queueIndex] = 0;
	free(infoParam);
	return 0;
}



int UpdateValues(WPARAM wparam,LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws;
	BOOL isIdleEvent;
	// to make this code faster
	if (!wparam) return 0;
	cws=(DBCONTACTWRITESETTING *)lparam;
	//if (CallService(MS_IGNORE_ISIGNORED,(WPARAM)hContact,IGNOREEVENT_USERONLINE)) return 0;
	isIdleEvent = includeIdle?(strcmp(cws->szSetting,"IdleTS")==0):0;
	if (strcmp(cws->szSetting,"Status") && strcmp(cws->szSetting,"StatusTriger") && (isIdleEvent==0)) return 0;
	if (!strcmp(cws->szModule,S_MOD)) {
		//here we will come when Settings/SeenModule/StatusTriger is changed
		WORD prevStatus=db_get_w((HANDLE)wparam, S_MOD, "OldStatus", ID_STATUS_OFFLINE);
		if (includeIdle){
			if ( db_get_b((HANDLE)wparam, S_MOD, "OldIdle", 0)) prevStatus &= 0x7FFF;
			else prevStatus |= 0x8000;
		}
		if ((cws->value.wVal|0x8000)<=ID_STATUS_OFFLINE)
		{
			char * proto;
			// avoid repeating the offline status
			if ((prevStatus|0x8000)<=ID_STATUS_OFFLINE)
				return 0;
			proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0);
			db_set_b((HANDLE)wparam, S_MOD, "Offline", 1);
			{
				DWORD t;
				char *str = (char *)malloc(MAXMODULELABELLENGTH+9);
				mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",proto);
				t = db_get_dw(NULL,S_MOD,str,0);
				if (!t) t = time(NULL);
				free(str);
				DBWriteTimeTS(t, (HANDLE)wparam);
			}

			if (!db_get_b(NULL,S_MOD,"IgnoreOffline",1))
			{
				char * sProto;
				if ( db_get_b(NULL,S_MOD,"FileOutput",0))
					FileWrite((HANDLE)wparam);

				if (CallProtoService(sProto = 
					(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0),
					PS_GETSTATUS,0,0
					)>ID_STATUS_OFFLINE)	{
					myPlaySound((HANDLE)wparam, ID_STATUS_OFFLINE, prevStatus);
					if ( db_get_b(NULL, S_MOD, "UsePopups", 0)) {
						ShowPopup((HANDLE)wparam, sProto, ID_STATUS_OFFLINE);
				}	}

				if ( db_get_b(NULL, S_MOD, "KeepHistory", 0))
					HistoryWrite((HANDLE)wparam);

				if ( db_get_b((HANDLE)wparam, S_MOD, "OnlineAlert", 0)) 
					ShowHistory((HANDLE)wparam, 1);
			}

		} else {

			if (cws->value.wVal==prevStatus && !db_get_b((HANDLE)wparam, S_MOD, "Offline", 0)) 
				return 0;

			DBWriteTimeTS(time(NULL), (HANDLE)wparam);

			//db_set_w(hContact,S_MOD,"StatusTriger",(WORD)cws->value.wVal);

			if ( db_get_b(NULL, S_MOD, "FileOutput", 0)) FileWrite((HANDLE)wparam);
			if (prevStatus != cws->value.wVal) myPlaySound((HANDLE)wparam, cws->value.wVal, prevStatus);
			if ( db_get_b(NULL, S_MOD, "UsePopups", 0))
				if (prevStatus != cws->value.wVal) ShowPopup((HANDLE)wparam, (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0), cws->value.wVal|0x8000);

			if ( db_get_b(NULL, S_MOD, "KeepHistory", 0)) HistoryWrite((HANDLE)wparam);
			if ( db_get_b((HANDLE)wparam, S_MOD, "OnlineAlert", 0)) ShowHistory((HANDLE)wparam, 1);
			db_set_b((HANDLE)wparam, S_MOD, "Offline", 0);
		}
	} else if (IsWatchedProtocol(cws->szModule)) {
		//here we will come when <User>/<module>/Status is changed or it is idle event and if <module> is watched
		if (CallProtoService(cws->szModule,PS_GETSTATUS,0,0)>ID_STATUS_OFFLINE){
			int index;
			if (!(index = isContactQueueActive((HANDLE)wparam))) {
				index = addContactToQueue((HANDLE)wparam);
				strncpy(contactQueue[index]->sProtoName,cws->szModule,MAXMODULELABELLENGTH);
	
				unsigned int dwThreadId;
				mir_forkthreadex((pThreadFuncEx)waitThread, contactQueue[index], &dwThreadId);
			}
			contactQueue[index]->courStatus = isIdleEvent ? db_get_w((HANDLE)wparam, cws->szModule, "Status", ID_STATUS_OFFLINE) : cws->value.wVal;
	}	}	

	return 0;
}

static DWORD __stdcall cleanThread(logthread_info* infoParam)
{
	Sleep(10000); // I hope in 10 secons all logged-in contacts will be listed

	HANDLE hcontact = db_find_first();
	while(hcontact != NULL) {
		char *contactProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0);
		if (contactProto) {
			if ( !strncmp(infoParam->sProtoName, contactProto, MAXMODULELABELLENGTH)) {
				WORD oldStatus = db_get_w(hcontact,S_MOD,"StatusTriger",ID_STATUS_OFFLINE) | 0x8000;
				if (oldStatus > ID_STATUS_OFFLINE) {
					if (db_get_w(hcontact,contactProto,"Status",ID_STATUS_OFFLINE)==ID_STATUS_OFFLINE){
						db_set_w(hcontact,S_MOD,"OldStatus",(WORD)(oldStatus|0x8000));
						if (includeIdle)db_set_b(hcontact,S_MOD,"OldIdle",(BYTE)((oldStatus&0x8000)?0:1));
						db_set_w(hcontact,S_MOD,"StatusTriger",ID_STATUS_OFFLINE);
					}
				}
			}
		}
		hcontact = db_find_next(hcontact);
	}

	char *str = (char *)malloc(MAXMODULELABELLENGTH+9);
	mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",infoParam->sProtoName);
	DBDeleteContactSetting(NULL,S_MOD,str);
	free(str);

	free(infoParam);
	return 0;
}


int ModeChange(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	WORD isetting=0;

	ack=(ACKDATA *)lparam;

	if (ack->type!=ACKTYPE_STATUS || ack->result!=ACKRESULT_SUCCESS || ack->hContact!=NULL) return 0;
	courProtoName = (char *)ack->szModule;
	if (!IsWatchedProtocol(courProtoName) && strncmp(courProtoName,"MetaContacts",12)) 
	{
		//MessageBox(NULL,"Protocol not watched",courProtoName,0);
		return 0;
	}

	DBWriteTimeTS(time(NULL),NULL);

//	isetting=CallProtoService(ack->szModule,PS_GETSTATUS,0,0);
	isetting=(WORD)ack->lParam;
	if (isetting<ID_STATUS_OFFLINE) isetting = ID_STATUS_OFFLINE;
	if ((isetting>ID_STATUS_OFFLINE)&&((WORD)ack->hProcess<=ID_STATUS_OFFLINE)) {
		//we have just loged-in
		db_set_dw(NULL, "UserOnline", ack->szModule, GetTickCount());
		if (IsWatchedProtocol(ack->szModule)) {
			logthread_info *info;
			info = (logthread_info *)malloc(sizeof(logthread_info));
			strncpy(info->sProtoName,courProtoName,MAXMODULELABELLENGTH);
			info->hContact = 0;
			info->courStatus = 0;

			unsigned int dwThreadId;
			CloseHandle( mir_forkthreadex((pThreadFuncEx)cleanThread, info, &dwThreadId));
		}
	} else if ((isetting==ID_STATUS_OFFLINE)&&((WORD)ack->hProcess>ID_STATUS_OFFLINE)) {
		//we have just loged-off
		if (IsWatchedProtocol(ack->szModule)) {
			char *str = (char *)malloc(MAXMODULELABELLENGTH+9);
			time_t t;
			time(&t);
			mir_snprintf(str,MAXMODULELABELLENGTH+8,"OffTime-%s",ack->szModule);
			DBWriteContactSettingDword(NULL,S_MOD,str,t);
			free(str);
	}	}
	if (isetting==db_get_w(NULL,S_MOD,courProtoName,ID_STATUS_OFFLINE)) return 0;
	db_set_w(NULL,S_MOD,courProtoName,isetting);

	// log "myself"
	if ( db_get_b(NULL,S_MOD,"FileOutput",0))
		FileWrite(NULL);

//	if (isetting==ID_STATUS_OFFLINE) //this is removed 'cause I want other contacts to be logged only if the status changed while I was offline
//		SetOffline();

	courProtoName = NULL;

	return 0;
}

short int isDbZero(HANDLE hContact, const char *module_name, const char *setting_name)
{
	DBVARIANT dbv;
	if ( !DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
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

WCHAR *any_to_IdleNotidleUnknown(HANDLE hContact, const char *module_name, const char *setting_name, WCHAR *buff, int bufflen) {
	short int r = isDbZero(hContact, module_name, setting_name);
	if (r==-1){
		wcsncpy(buff, TranslateW(L"Unknown"), bufflen);
	} else {
		wcsncpy(buff, TranslateW(r?L"Not Idle":L"Idle"), bufflen);
	};
	buff[bufflen - 1] = 0;
	return buff;
}

WCHAR *any_to_Idle(HANDLE hContact, const char *module_name, const char *setting_name, WCHAR *buff, int bufflen) {
	if (isDbZero(hContact, module_name, setting_name)==0) { //DB setting is NOT zero and exists
		buff[0] = L'/';
		wcsncpy((WCHAR *)&buff[1], TranslateW(L"Idle"), bufflen-1);
	} else buff[0] = 0;
	buff[bufflen - 1] = 0;
	return buff;
}
