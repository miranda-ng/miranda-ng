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

#include "stdafx.h"

void FileWrite(MCONTACT);
void HistoryWrite(MCONTACT hcontact);

extern HANDLE g_hShutdownEvent;
char *courProtoName = nullptr;

void LoadWatchedProtos()
{
	// Upgrade from old settings (separated by " ")
	ptrA szProtosOld(g_plugin.getStringA("WatchedProtocols"));
	if (szProtosOld != NULL) {
		CMStringA tmp(szProtosOld);
		tmp.Replace(" ", "\n");
		g_plugin.setString("WatchedAccounts", tmp.c_str());
		g_plugin.delSetting("WatchedProtocols");
	}

	ptrA szProtos(g_plugin.getStringA("WatchedAccounts"));
	if (szProtos == NULL)
		return;

	for (char *p = strtok(szProtos, "\n"); p != nullptr; p = strtok(nullptr, "\n"))
		arWatchedProtos.insert(mir_strdup(p));
}

void UnloadWatchedProtos()
{
	for (auto &it : arWatchedProtos)
		mir_free(it);
	arWatchedProtos.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Returns true if the protocols is to be monitored

int IsWatchedProtocol(const char* szProto)
{
	if (szProto == nullptr)
		return 0;

	PROTOACCOUNT *pd = Proto_GetAccount(szProto);
	if (pd == nullptr || CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
		return 0;

	return arWatchedProtos.find((char*)szProto) != nullptr;
}

bool isJabber(const char *protoname)
{
	if (protoname)
		return !mir_strcmp(Proto_GetUniqueId(protoname), "jid");

	return false;
}

uint32_t isSeen(MCONTACT hcontact, SYSTEMTIME *st)
{
	FILETIME ft;
	ULONGLONG ll;
	uint32_t res = g_plugin.getDword(hcontact, "seenTS", 0);
	if (res) {
		if (st) {
			ll = UInt32x32To64(TimeZone_ToLocal(res), 10000000) + NUM100NANOSEC;
			ft.dwLowDateTime = (uint32_t)ll;
			ft.dwHighDateTime = (uint32_t)(ll >> 32);
			FileTimeToSystemTime(&ft, st);
		}
		return res;
	}

	SYSTEMTIME lst;
	memset(&lst, 0, sizeof(lst));
	if (lst.wYear = g_plugin.getWord(hcontact, "Year", 0)) {
		if (lst.wMonth = g_plugin.getWord(hcontact, "Month", 0)) {
			if (lst.wDay = g_plugin.getWord(hcontact, "Day", 0)) {
				lst.wDayOfWeek = g_plugin.getWord(hcontact, "WeekDay", 0);
				lst.wHour = g_plugin.getWord(hcontact, "Hours", 0);
				lst.wMinute = g_plugin.getWord(hcontact, "Minutes", 0);
				lst.wSecond = g_plugin.getWord(hcontact, "Seconds", 0);
				if (SystemTimeToFileTime(&lst, &ft)) {
					ll = ((LONGLONG)ft.dwHighDateTime << 32) | ((LONGLONG)ft.dwLowDateTime);
					ll -= NUM100NANOSEC;
					ll /= 10000000;
					//perform LOCALTOTIMESTAMP
					res = (uint32_t)ll - TimeZone_ToLocal(0);
					//nevel look for Year/Month/Day/Hour/Minute/Second again
					g_plugin.setDword(hcontact, "seenTS", res);
				}
			}
		}
	}

	if (st)
		memcpy(st, &lst, sizeof(SYSTEMTIME));

	return res;
}

wchar_t *weekdays[] = { LPGENW("Sunday"), LPGENW("Monday"), LPGENW("Tuesday"), LPGENW("Wednesday"), LPGENW("Thursday"), LPGENW("Friday"), LPGENW("Saturday") };
wchar_t *wdays_short[] = { LPGENW("Sun."), LPGENW("Mon."), LPGENW("Tue."), LPGENW("Wed."), LPGENW("Thu."), LPGENW("Fri."), LPGENW("Sat.") };
wchar_t *monthnames[] = { LPGENW("January"), LPGENW("February"), LPGENW("March"), LPGENW("April"), LPGENW("May"), LPGENW("June"), LPGENW("July"), LPGENW("August"), LPGENW("September"), LPGENW("October"), LPGENW("November"), LPGENW("December") };
wchar_t *mnames_short[] = { LPGENW("Jan."), LPGENW("Feb."), LPGENW("Mar."), LPGENW("Apr."), LPGENW("May"), LPGENW("Jun."), LPGENW("Jul."), LPGENW("Aug."), LPGENW("Sep."), LPGENW("Oct."), LPGENW("Nov."), LPGENW("Dec.") };

CMStringW ParseString(const wchar_t *pwszFormat, MCONTACT hcontact)
{
	SYSTEMTIME st;
	if (!isSeen(hcontact, &st))
		return TranslateT("<never seen>");

	char *szProto = hcontact ? Proto_GetBaseAccountName(hcontact) : courProtoName;
	ptrW info;

	CMStringW res;
	for (const wchar_t *p = pwszFormat; *p; p++) {
		if (*p != '%' && *p != '#') {
			res.AppendChar(*p);
			continue;
		}

		bool wantempty = *p == '#';
		switch (*++p) {
		case 'Y':
			if (!st.wYear) goto LBL_noData;
			res.AppendFormat(L"%04i", st.wYear);
			break;

		case 'y':
			if (!st.wYear) goto LBL_noData;
			res.AppendFormat(L"%02i", st.wYear % 100);
			break;

		case 'm':
			if (!st.wMonth) goto LBL_noData;
			res.AppendFormat(L"%02i", st.wMonth);
			break;

		case 'd':
			if (!st.wDay) goto LBL_noData;
			res.AppendFormat(L"%02i", st.wDay);
			break;

		case 'W':
			if (st.wDayOfWeek != -1) {
				res.Append(TranslateW(weekdays[st.wDayOfWeek]));
				break;
			}

LBL_noData:
			res.Append(wantempty ? L"" : TranslateT("<unknown>"));
			break;
			
		case 'w':
			if (st.wDayOfWeek == -1) goto LBL_noData;
			res.Append(TranslateW(wdays_short[st.wDayOfWeek]));
			break;

		case 'E':
			if (!st.wMonth) goto LBL_noData;
			res.Append(TranslateW(monthnames[st.wMonth - 1]));
			break;

		case 'e':
			if (!st.wMonth) goto LBL_noData;
			res.Append(TranslateW(mnames_short[st.wMonth - 1]));
			break;

		case 'H':
			if (st.wHour == -1) goto LBL_noData;
			res.AppendFormat(L"%02i", st.wHour);
			break;

		case 'h':
			if (st.wHour == -1) goto LBL_noData;
			if (!st.wHour) st.wHour = 12;
			st.wHour = st.wHour - ((st.wHour > 12) ? 12 : 0);
			res.AppendFormat(L"%02i", st.wHour);
			break;

		case 'p':
			if (st.wHour == -1) goto LBL_noData;
			res.Append((st.wHour >= 12) ? L"PM" : L"AM");
			break;

		case 'M':
			if (st.wMinute == -1) goto LBL_noData;
			res.AppendFormat(L"%02i", st.wMinute);
			break;

		case 'S':
			if (st.wSecond == -1) goto LBL_noData;
			res.AppendFormat(L"%02i", st.wSecond);
			break;

		case 'n':
			res.Append(hcontact ? Clist_GetContactDisplayName(hcontact) : (wantempty ? L"" : L"---"));
			break;

		case 'N':
			if (info = Contact::GetInfo(CNF_NICK, hcontact, szProto)) {
				res.Append(info);
				break;
			}
			goto LBL_noData;

		case 'G':
			{
				ptrW wszGroup(Clist_GetGroup(hcontact));
				if (wszGroup)
					res.Append(wszGroup);
			}
			break;

		case 'u':
			if (info = Contact::GetInfo(CNF_UNIQUEID, hcontact, szProto)) {
				res.Append(info);
				break;
			}
			goto LBL_noData;

		case 's':
			if (int iStatus = g_plugin.getWord(hcontact, hcontact ? "StatusTriger" : courProtoName, 0)) {
				res.Append(Clist_GetStatusModeDescription(iStatus | 0x8000, 0));
				if (!(iStatus & 0x8000)) {
					res.AppendChar('/');
					res.Append(TranslateT("Idle"));
				}
				break;
			}
			goto LBL_noData;

		case 'T':
			if (info = db_get_wsa(hcontact, "CList", "StatusMsg")) {
				res.Append(info);
				break;
			}

			goto LBL_noData;

		case 'o':
			if (int iStatus = g_plugin.getWord(hcontact, hcontact ? "OldStatus" : courProtoName, 0)) {
				res.Append(Clist_GetStatusModeDescription(iStatus, 0));
				if (includeIdle && hcontact && g_plugin.getByte(hcontact, "OldIdle")) {
					res.Append(L"/");
					res.Append(TranslateT("Idle"));
				}
				break;
			}
			goto LBL_noData;

		case 'i':
		case 'r':
			if (isJabber(szProto)) {
				if (info = db_get_wsa(hcontact, szProto, *p == 'i' ? "Resource" : "System")) {
					res.Append(info);
					break;
				}
				goto LBL_noData;
			}
			else {
				int dwsetting = db_get_dw(hcontact, szProto, *p == 'i' ? "IP" : "RealIP", 0);
				if (!dwsetting)
					goto LBL_noData;

				struct in_addr ia;
				ia.S_un.S_addr = htonl(dwsetting);
				res.Append(_A2T(inet_ntoa(ia)));
			}
			break;

		case 'P':
			res.Append(szProto ? _A2T(szProto) : (wantempty ? L"" : L"ProtoUnknown"));
			break;

		case 'b':
			res.Append(L"\x0D\x0A");
			break;

		case 'C': // Get Client Info
			if (info = db_get_wsa(hcontact, szProto, "MirVer")) {
				res.Append(info);
				break;
			}
			goto LBL_noData;

		case 't':
			res.Append(L"\t");
			break;

		case 'A':
			if (PROTOACCOUNT *pa = Proto_GetAccount(szProto)) {
				res.Append(pa->tszAccountName);
				break;
			}
			goto LBL_noData;

		default:
			res.AppendChar(p[-1]);
			res.AppendChar(*p);
		}
	}

	return res;
}

void DBWriteTimeTS(uint32_t t, MCONTACT hcontact)
{
	ULONGLONG ll = UInt32x32To64(TimeZone_ToLocal(t), 10000000) + NUM100NANOSEC;

	FILETIME ft;
	ft.dwLowDateTime = (uint32_t)ll;
	ft.dwHighDateTime = (uint32_t)(ll >> 32);

	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	g_plugin.setDword(hcontact, "seenTS", t);
	g_plugin.setWord(hcontact, "Day", st.wDay);
	g_plugin.setWord(hcontact, "Month", st.wMonth);
	g_plugin.setWord(hcontact, "Year", st.wYear);
	g_plugin.setWord(hcontact, "Hours", st.wHour);
	g_plugin.setWord(hcontact, "Minutes", st.wMinute);
	g_plugin.setWord(hcontact, "Seconds", st.wSecond);
	g_plugin.setWord(hcontact, "WeekDay", st.wDayOfWeek);
}

void GetColorsFromDWord(LPCOLORREF First, LPCOLORREF Second, uint32_t colDword)
{
	uint16_t temp;
	COLORREF res = 0;
	temp = (uint16_t)(colDword >> 16);
	res |= ((temp & 0x1F) << 3);
	res |= ((temp & 0x3E0) << 6);
	res |= ((temp & 0x7C00) << 9);
	if (First) *First = res;
	res = 0;
	temp = (uint16_t)colDword;
	res |= ((temp & 0x1F) << 3);
	res |= ((temp & 0x3E0) << 6);
	res |= ((temp & 0x7C00) << 9);
	if (Second) *Second = res;
}

uint32_t StatusColors15bits[] = {
	0x63180000, // 0x00C0C0C0, 0x00000000, Offline - LightGray
	0x7B350000, // 0x00F0C8A8, 0x00000000, Online  - LightBlue
	0x33fe0000, // 0x0070E0E0, 0x00000000, Away - LightOrange
	0x295C0000, // 0x005050E0, 0x00000000, DND - DarkRed
	0x5EFD0000, // 0x00B8B8E8, 0x00000000, Not available - LightRed
	0x295C0000, // 0x005050E0, 0x00000000, Occupied
	0x43900000, // 0x0080E080, 0x00000000, Free for chat - LightGreen
	0x76AF0000, // 0x00E8A878, 0x00000000, Invisible
};

uint32_t GetDWordFromColors(COLORREF First, COLORREF Second)
{
	uint32_t res = 0;
	res |= (First & 0xF8) >> 3;
	res |= (First & 0xF800) >> 6;
	res |= (First & 0xF80000) >> 9;
	res <<= 16;
	res |= (Second & 0xF8) >> 3;
	res |= (Second & 0xF800) >> 6;
	res |= (Second & 0xF80000) >> 9;
	return res;
}

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			MCONTACT hContact = PUGetContact(hwnd);
			if (hContact > 0) CallService(MS_MSG_SENDMESSAGE, hContact, 0);
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
	if (Ignore_IsIgnored(hcontact, IGNOREEVENT_USERONLINE))
		return;

	if (!g_plugin.bUsePopups || Contact::IsHidden(hcontact))
		return;

	char szSetting[10];
	mir_snprintf(szSetting, "Col_%d", newStatus - ID_STATUS_OFFLINE);
	uint32_t sett = g_plugin.getDword(szSetting, StatusColors15bits[newStatus - ID_STATUS_OFFLINE]);

	POPUPDATAW ppd;
	GetColorsFromDWord(&ppd.colorBack, &ppd.colorText, sett);

	wcsncpy_s(ppd.lpwzContactName, ParseString(g_plugin.getMStringW("PopupStamp", DEFAULT_POPUPSTAMP), hcontact), _TRUNCATE);
	wcsncpy_s(ppd.lpwzText, ParseString(g_plugin.getMStringW("PopupStampText", DEFAULT_POPUPSTAMPTEXT), hcontact), _TRUNCATE);

	ppd.lchContact = hcontact;
	ppd.lchIcon = Skin_LoadProtoIcon(lpzProto, newStatus);
	ppd.PluginWindowProc = PopupDlgProc;
	PUAddPopupW(&ppd);
}

void myPlaySound(MCONTACT hcontact, uint16_t newStatus, uint16_t oldStatus)
{
	if (Ignore_IsIgnored(hcontact, IGNOREEVENT_USERONLINE))
		return;
	
	// oldStatus and hcontact are not used yet
	char *soundname = nullptr;
	if ((newStatus == ID_STATUS_ONLINE) || (newStatus == ID_STATUS_FREECHAT)) soundname = "LastSeenTrackedStatusOnline";
	else if (newStatus == ID_STATUS_OFFLINE) soundname = "LastSeenTrackedStatusOffline";
	else if (oldStatus == ID_STATUS_OFFLINE) soundname = "LastSeenTrackedStatusFromOffline";
	else soundname = "LastSeenTrackedStatusChange";
	if (soundname != nullptr)
		Skin_PlaySound(soundname);
}

// will add hContact to queue and will return position;
static void waitThread(logthread_info* infoParam)
{
	Thread_SetName("SeenPlugin: waitThread");

	uint16_t prevStatus = g_plugin.getWord(infoParam->hContact, "StatusTriger", ID_STATUS_OFFLINE);

	// I hope in 1.5 second all the needed info will be set
	if (WaitForSingleObject(g_hShutdownEvent, 1500) == WAIT_TIMEOUT) {
		if (includeIdle)
			if (db_get_dw(infoParam->hContact, infoParam->sProtoName, "IdleTS", 0))
				infoParam->currStatus &= 0x7FFF;

		if (infoParam->currStatus != prevStatus) {
			g_plugin.setWord(infoParam->hContact, "OldStatus", (uint16_t)(prevStatus | 0x8000));
			if (includeIdle)
				g_plugin.setByte(infoParam->hContact, "OldIdle", (uint8_t)((prevStatus & 0x8000) == 0));

			g_plugin.setWord(infoParam->hContact, "StatusTriger", infoParam->currStatus);
		}
	}
	{
		mir_cslock lck(csContacts);
		arContacts.remove(infoParam);
	}
	mir_free(infoParam);
}

int UpdateValues(WPARAM hContact, LPARAM lparam)
{
	// to make this code faster
	if (!hContact)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lparam;
	char *szProto = Proto_GetBaseAccountName(hContact);

	if (cws->value.type == DBVT_DWORD && !strcmp(cws->szSetting, "LastSeen") && !mir_strcmp(cws->szModule, szProto)) {
		DBWriteTimeTS(cws->value.dVal, hContact);
		
		HWND hwnd = WindowList_Find(g_pUserInfo, hContact);
		if (hwnd != nullptr)
			SendMessage(hwnd, WM_REFRESH_UI, hContact, 0);
		return 0;
	}

	BOOL isIdleEvent = includeIdle ? (strcmp(cws->szSetting, "IdleTS") == 0) : 0;
	if (strcmp(cws->szSetting, "Status") && strcmp(cws->szSetting, "StatusTriger") && (isIdleEvent == 0))
		return 0;
	
	if (!strcmp(cws->szModule, MODULENAME)) {
		// here we will come when Settings/SeenModule/StatusTriger is changed
		uint16_t prevStatus = g_plugin.getWord(hContact, "OldStatus", ID_STATUS_OFFLINE);
		if (includeIdle) {
			if (g_plugin.getByte(hContact, "OldIdle", 0))
				prevStatus &= 0x7FFF;
			else
				prevStatus |= 0x8000;
		}
		
		if ((cws->value.wVal | 0x8000) <= ID_STATUS_OFFLINE) {
			// avoid repeating the offline status
			if ((prevStatus | 0x8000) <= ID_STATUS_OFFLINE)
				return 0;

			g_plugin.setByte(hContact, "Offline", 1);
			{
				char str[MAXMODULELABELLENGTH + 9];

				mir_snprintf(str, "OffTime-%s", szProto);
				uint32_t t = g_plugin.getDword(str, 0);
				if (!t)
					t = time(0);
				DBWriteTimeTS(t, hContact);
			}

			if (!g_plugin.getByte("IgnoreOffline", 1)) {
				if (g_bFileActive)
					FileWrite(hContact);

				char *sProto = Proto_GetBaseAccountName(hContact);
				if (Proto_GetStatus(sProto) > ID_STATUS_OFFLINE) {
					myPlaySound(hContact, ID_STATUS_OFFLINE, prevStatus);
					if (g_plugin.bUsePopups)
						ShowPopup(hContact, sProto, ID_STATUS_OFFLINE);
				}

				if (g_plugin.getByte("KeepHistory", 0))
					HistoryWrite(hContact);

				if (g_plugin.getByte(hContact, "OnlineAlert", 0))
					ShowHistory(hContact, 1);
			}

		}
		else {
			if (cws->value.wVal == prevStatus && !g_plugin.getByte(hContact, "Offline", 0))
				return 0;

			DBWriteTimeTS(time(0), hContact);

			if (g_bFileActive) FileWrite(hContact);
			if (prevStatus != cws->value.wVal) myPlaySound(hContact, cws->value.wVal, prevStatus);
			if (g_plugin.bUsePopups)
				if (prevStatus != cws->value.wVal)
					ShowPopup(hContact, Proto_GetBaseAccountName(hContact), cws->value.wVal | 0x8000);

			if (g_plugin.getByte("KeepHistory", 0))
				HistoryWrite(hContact);
			if (g_plugin.getByte(hContact, "OnlineAlert", 0))
				ShowHistory(hContact, 1);
			g_plugin.setByte(hContact, "Offline", 0);
		}
	}
	else if (hContact && IsWatchedProtocol(cws->szModule) && !Contact::IsGroupChat(hContact, cws->szModule)) {
		// here we will come when <User>/<module>/Status is changed or it is idle event and if <module> is watched
		if (Proto_GetStatus(cws->szModule) > ID_STATUS_OFFLINE) {
			mir_cslock lck(csContacts);
			logthread_info *p = arContacts.find((logthread_info*)&hContact);
			if (p == nullptr) {
				p = (logthread_info*)mir_calloc(sizeof(logthread_info));
				p->hContact = hContact;
				mir_strncpy(p->sProtoName, cws->szModule, _countof(p->sProtoName));
				arContacts.insert(p);
				mir_forkThread<logthread_info>(waitThread, p);
			}
			p->currStatus = isIdleEvent ? db_get_w(hContact, cws->szModule, "Status", ID_STATUS_OFFLINE) : cws->value.wVal;
		}
	}

	return 0;
}

static void cleanThread(logthread_info* infoParam)
{
	Thread_SetName("SeenPlugin: cleanThread");

	char *szProto = infoParam->sProtoName;

	// I hope in 10 secons all logged-in contacts will be listed
	if (WaitForSingleObject(g_hShutdownEvent, 10000) == WAIT_TIMEOUT) {
		for (auto &hContact : Contacts(szProto)) {
			uint16_t oldStatus = g_plugin.getWord(hContact, "StatusTriger", ID_STATUS_OFFLINE) | 0x8000;
			if (oldStatus > ID_STATUS_OFFLINE) {
				if (db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
					g_plugin.setWord(hContact, "OldStatus", (uint16_t)(oldStatus | 0x8000));
					if (includeIdle)
						g_plugin.setByte(hContact, "OldIdle", (uint8_t)((oldStatus & 0x8000) ? 0 : 1));
					g_plugin.setWord(hContact, "StatusTriger", ID_STATUS_OFFLINE);
				}
			}
		}

		char str[MAXMODULELABELLENGTH + 9];
		mir_snprintf(str, "OffTime-%s", infoParam->sProtoName);
		g_plugin.delSetting(str);
	}
	mir_free(infoParam);
}

int ModeChange(WPARAM, LPARAM lparam)
{
	ACKDATA *ack = (ACKDATA *)lparam;

	if (ack->type != ACKTYPE_STATUS || ack->result != ACKRESULT_SUCCESS || ack->hContact != NULL) return 0;
	courProtoName = (char *)ack->szModule;
	if (!IsWatchedProtocol(courProtoName) && strncmp(courProtoName, "MetaContacts", 12))
		return 0;

	DBWriteTimeTS(time(0), NULL);

	uint16_t isetting = (uint16_t)ack->lParam;
	if (isetting < ID_STATUS_OFFLINE)
		isetting = ID_STATUS_OFFLINE;
	if ((isetting > ID_STATUS_OFFLINE) && ((UINT_PTR)ack->hProcess <= ID_STATUS_OFFLINE)) {
		//we have just loged-in
		db_set_dw(0, "UserOnline", ack->szModule, GetTickCount());
		if (!Miranda_IsTerminated() && IsWatchedProtocol(ack->szModule)) {
			logthread_info *info = (logthread_info *)mir_alloc(sizeof(logthread_info));
			mir_strncpy(info->sProtoName, courProtoName, _countof(info->sProtoName));
			info->hContact = 0;
			info->currStatus = 0;

			mir_forkThread<logthread_info>(cleanThread, info);
		}
	}
	else if ((isetting == ID_STATUS_OFFLINE) && ((UINT_PTR)ack->hProcess > ID_STATUS_OFFLINE)) {
		//we have just loged-off
		if (IsWatchedProtocol(ack->szModule)) {
			char str[MAXMODULELABELLENGTH + 9];
			time_t t;

			time(&t);
			mir_snprintf(str, "OffTime-%s", ack->szModule);
			g_plugin.setDword(str, t);
		}
	}

	if (isetting == g_plugin.getWord(courProtoName, ID_STATUS_OFFLINE))
		return 0;

	g_plugin.setWord(courProtoName, isetting);

	if (g_bFileActive)
		FileWrite(NULL);

	courProtoName = nullptr;
	return 0;
}

short int isDbZero(MCONTACT hContact, const char *module_name, const char *setting_name)
{
	DBVARIANT dbv;
	if (!db_get(hContact, module_name, setting_name, &dbv)) {
		short int res = 0;
		switch (dbv.type) {
			case DBVT_BYTE: res = dbv.bVal == 0; break;
			case DBVT_WORD: res = dbv.wVal == 0; break;
			case DBVT_DWORD: res = dbv.dVal == 0; break;
			case DBVT_BLOB: res = dbv.cpbVal == 0; break;
			default: res = dbv.pszVal[0] == 0; break;
		}
		db_free(&dbv);
		return res;
	}
	return -1;
}

wchar_t* any_to_IdleNotidleUnknown(MCONTACT hContact, const char *module_name, const char *setting_name, wchar_t *buff, int bufflen)
{
	short int r = isDbZero(hContact, module_name, setting_name);
	if (r == -1) {
		wcsncpy(buff, TranslateT("Unknown"), bufflen);
	}
	else {
		wcsncpy(buff, r ? TranslateT("Not Idle") : TranslateT("Idle"), bufflen);
	};
	buff[bufflen - 1] = 0;
	return buff;
}

wchar_t* any_to_Idle(MCONTACT hContact, const char *module_name, const char *setting_name, wchar_t *buff, int bufflen)
{
	if (isDbZero(hContact, module_name, setting_name) == 0) { //DB setting is NOT zero and exists
		buff[0] = L'/';
		wcsncpy(&buff[1], TranslateT("Idle"), bufflen - 1);
	}
	else buff[0] = 0;
	buff[bufflen - 1] = 0;
	return buff;
}
