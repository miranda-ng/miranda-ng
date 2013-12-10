#include "globals.h"


//Proto_Status2Flag

 int OldIndexToStatus(int oldIndex){
	switch (oldIndex){
	case 0 : return ID_STATUS_AWAY; 
	case 1 : return ID_STATUS_DND; 
	case 2 : return ID_STATUS_NA; 
	case 3 : return ID_STATUS_OCCUPIED; 
	case 4 : return ID_STATUS_INVISIBLE; 
	case 5 : return ID_STATUS_ONTHEPHONE; 
	case 6 : return ID_STATUS_OUTTOLUNCH;
	case 7 : return ID_STATUS_ONLINE;
	case 8 : return ID_STATUS_FREECHAT; 
	case 9 : return ID_STATUS_OFFLINE;
	}
	return 0;
}
 int StatusToOldIndex(int status){
	switch (status){
	case ID_STATUS_AWAY : return  0; 
	case ID_STATUS_DND : return 1; 
	case ID_STATUS_NA : return 2; 
	case ID_STATUS_OCCUPIED : return 3; 
	case ID_STATUS_INVISIBLE : return 4; 
	case ID_STATUS_ONTHEPHONE : return 5; 
	case ID_STATUS_OUTTOLUNCH : return 6;
	case ID_STATUS_ONLINE : return 7;
	case ID_STATUS_FREECHAT : return 8; 
	case ID_STATUS_OFFLINE : return 9;
	}
	return 0;
}

 unsigned long SetBits (unsigned long target,unsigned long mask,unsigned long value)
{
	int shift = 0;
	unsigned long t;
	while (!((mask>>shift)&1)) {
		shift++;
	}
	t = mask ^ 0xFFFFFFFF;
	target = target & t;
	value = value<<shift;
	target = target | (value&mask);
	return target;
}

 unsigned long GetBits (unsigned long source,unsigned long mask)
{
	int shift = 0;
	unsigned long t;
	while (!((mask>>shift)&1)) {
		shift++;
	}
	t = source & mask;
	return t >> shift;
}

BOOL isInterestingProto(int j){
	///if ( accounts[j]->type != PROTOTYPE_PROTOCOL ) return 0;
	if (!accounts[j]->bIsEnabled) return 0;
	if (protoModes[j] > 1) return 1;
	else return 0;
 }

 BOOL IsStatusBitSet(int status, int bits){
	int i=0;
	int mask=1;
	for (i = numStatuses-1; i>=0; i--){
		if (aa_Status[i]==status) return (mask & bits);
		mask <<= 1;
	}
	return 0;
}

 int StatusByProtoIndex(int modes, int index){
	int i=0;
	int indexcount=0;
	int mask = (1<<(numStatuses-1));
	for (i = 0; i < numStatuses ; i++){
		BOOL isset = (modes & mask);
		if (isset) {
			indexcount++;
			if (indexcount==index) return aa_Status[i];
		}
		mask >>= 1;
	}
	return 0;
}

 int StatusToProtoIndex(int status, int modes){
	int i=0;
	int indexcount=0;
	int mask = (1<<(numStatuses-1));
	for (i = 0; i < numStatuses ; i++){
		BOOL isset = (modes & mask);
		if (isset) {
			indexcount++;
			if (status==aa_Status[i]) return indexcount;
		}
		mask >>= 1;
	}
	return 0;
	
}
 BOOL StrikeOut(HWND hWnd, int item, BOOL striked)
{
	LOGFONT lf;
	HFONT hFont;
	BOOL previous;

	hFont = (HFONT)SendDlgItemMessage(hWnd, item, WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(lf), &lf);
	previous = lf.lfItalic;
	lf.lfItalic = striked;
	SendDlgItemMessage(hWnd, item, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);
	return previous;
}

 int GetCourSelProtocol(int sel){
	int j;
	int protoindex=0;
	int selectedProto = 0;
	for (j = 0 ; j < protoCount ; j++) {
		if ( !isInterestingProto(j) ) continue;
		{
			if (protoindex==(sel)){
				return j;
			}
		}
		protoindex++;
	}
	return -1;
}

LRESULT SetDlgItems(HWND hwndDlg, int coursel)
{
	SendDlgItemMessage(hwndDlg, IDC_REQSTATUSLABEL, WM_SETTEXT, 0,(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)reqStatus[coursel], GSMDF_TCHAR)); 
	SendDlgItemMessage(hwndDlg, IDC_COURSTATUSLABEL, WM_SETTEXT, 0,(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)courStatus[coursel], GSMDF_TCHAR)); 
	return TRUE;
}

int GetReconnectDelayFromSlider(int slide)
{
	     if (slide <  4) return    6+slide;
	else if (slide < 29) return    2+slide* 2;
	else if (slide < 53) return  -85+slide* 5;
	else if (slide < 65) return -350+slide*10;
	else if (slide < 85) return-1650+slide*30;
	else return -4200+slide*60;
}

int GetSliderFromReconnectDelay(int delay)
{
	     if (delay<   6) return 0;
	else if (delay<  10) return -6+delay;
	else if (delay<  60) return -1+delay/ 2;
	else if (delay< 180) return 17+delay/ 5;
	else if (delay< 300) return 35+delay/10;
	else if (delay< 900) return 55+delay/30;
	else if (delay<1800) return 70+delay/60;
	else return 100;
}
BOOL GetStringFromDelay(TCHAR * str, int delay)
{
	int min,sec;
	min = (int)delay/60;
	sec = delay-min*60;
	int pos = _stprintf(str,_T("%s "), TranslateT("Reconnect after"));
	if (min) pos += _stprintf(&str[pos],_T("%d %s "),min,TranslateT("min"));
	if (sec) pos += _stprintf(&str[pos],_T("%d %s"),sec,TranslateT("sec"));
//	if (!min) sprintf(str,"%s (Too fast!)",str);
	return (delay<60);
}


void GetLastActiveLocalTime(SYSTEMTIME * at, unsigned long Seconds){
	FILETIME ft;
	ULARGE_INTEGER t;

	GetSystemTimeAsFileTime(&ft);
	t.u.LowPart = ft.dwLowDateTime;
	t.u.HighPart = ft.dwHighDateTime;
	t.QuadPart -= (__int64)Seconds*10000000;
	ft.dwLowDateTime = t.u.LowPart;
	ft.dwHighDateTime = t.u.HighPart;
	FileTimeToSystemTime(&ft,at);
	SystemTimeToTzSpecificLocalTime(NULL,at,at);
}

int GetTimeZone(TCHAR * name){
	TIME_ZONE_INFORMATION tzi={0};
	DWORD dwSta;

    dwSta= GetTimeZoneInformation( &tzi );
	switch (dwSta) {
		case 0:
		case TIME_ZONE_ID_STANDARD: 
#ifdef UNICODE
			wcscpy(name,tzi.StandardName);
#else
			_stprintf(name,_T("%S"),tzi.StandardName);
#endif
			return -(tzi.Bias+tzi.StandardBias);
			break;
		case TIME_ZONE_ID_DAYLIGHT: 
#ifdef UNICODE
			wcscpy(name,tzi.DaylightName);
#else
			_stprintf(name,_T("%S"),tzi.DaylightName);
#endif
			return -(tzi.Bias+tzi.DaylightBias);
			break;
		case TIME_ZONE_ID_INVALID:
		default:
			name[0] = 0;
			return 0;
			break;
	}		
}
void my_u2a(const TCHAR* src, char * target, unsigned int max){
#ifdef UNICODE 
	int r = WideCharToMultiByte(codePage, 0, src, -1, target, max, NULL, NULL );
	/*if (!r){
		int err=GetLastError();
		WCHAR errBuf[1024];
		int p = wsprintf(errBuf,L"Error %u: \n",err);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
			NULL, 
			err, 
			0,
			&errBuf[p],
			1024-p-1,
			NULL);
		MessageBoxW(NULL, errBuf, L"Error u2a",  MB_OK); 
	}*/
#else
	strncpy(target,src,max);
#endif
	target[ max ] = 0;
}

void my_a2u(const char* src, TCHAR * target, unsigned int max){
#ifdef UNICODE 
	int r = MultiByteToWideChar(codePage, 0, src, -1, target, max);
#else
	strncpy(target,src,max);
#endif
	target[ max ] = 0;
}

UINT ParseString(TCHAR *szstring,TCHAR *sztemp, unsigned int MaxOutLen)
{
	UINT loop=0;
	TCHAR *weekdays[]={LPGENT("Sunday"),LPGENT("Monday"),LPGENT("Tuesday"),LPGENT("Wednesday"),LPGENT("Thursday"),LPGENT("Friday"),LPGENT("Saturday")};
	TCHAR *wdays_short[]={LPGENT("Sun"),LPGENT("Mon"),LPGENT("Tue"),LPGENT("Wed"),LPGENT("Thu"),LPGENT("Fri"),LPGENT("Sat")};
	TCHAR *monthnames[]={LPGENT("January"),LPGENT("February"),LPGENT("March"),LPGENT("April"),LPGENT("May"),LPGENT("June"),LPGENT("July"),LPGENT("August"),LPGENT("September"),LPGENT("October"),LPGENT("November"),LPGENT("December")};
	TCHAR *mnames_short[]={LPGENT("Jan"),LPGENT("Feb"),LPGENT("Mar"),LPGENT("Apr"),LPGENT("May"),LPGENT("Jun"),LPGENT("Jul"),LPGENT("Aug"),LPGENT("Sep"),LPGENT("Oct"),LPGENT("Nov"),LPGENT("Dec")};
	SYSTEMTIME st = {0};
	BOOL stFilled = FALSE;
	BOOL tzFilled = FALSE;
	TCHAR tzName[128];
	int tzDrift;
	*sztemp = '\0';
	UINT inpLen=_tcsclen(szstring);
	UINT pos=0;
	for(loop=0;loop<inpLen && pos<=MaxOutLen;loop++)
	{
		int res=0;
		if(szstring[loop]!='%')
		{
			res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%c"),szstring[loop]);
		}

		else
		{
			switch(szstring[++loop]){
				case 'Y':
				case 'y':
				case 'm':
				case 'd':
				case 'W':
				case 'w':
				case 'E':
				case 'e':
				case 'H':
				case 'h':
				case 'p':
				case 'M':
				case 'S':
				case 'L':
				case 'l':
				case 'K':
				case 'k':
					if (!stFilled) {
						GetLastActiveLocalTime(&st,secondsAfterLastActivity);
						stFilled = TRUE;
					}
					switch (szstring[loop]){
						case 'Y':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%04i"),st.wYear);
							break;
						case 'y':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02i"),div(st.wYear,100).rem);
							break;
						case 'm':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02i"),st.wMonth);
							break;

						case 'd':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02i"),st.wDay);
							break;

						case 'W':case 'w':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%s"),TranslateTS((szstring[loop]=='W')?weekdays[st.wDayOfWeek]:wdays_short[st.wDayOfWeek]));
							break;

						case 'E':case 'e':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%s"),TranslateTS((szstring[loop]=='E')?monthnames[st.wMonth-1]:mnames_short[st.wMonth-1]));
							break;

						case 'H':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02i"),st.wHour);
							break;

						case 'h':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%i"),(st.wHour)?(st.wHour-((st.wHour>12)?12:0)):12);
							break;

						case 'p':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%s"),TranslateTS((st.wHour>12)?_T("PM"):_T("AM")));
							break;

						case 'M':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02i"),st.wMinute);
							break;

						case 'S':
							res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02i"),st.wSecond);
							break;

						case 'L':
						case 'l':
						case 'K':
						case 'k':
							{
								switch (szstring[loop]){
									case 'L':
									case 'l':
										res = GetDateFormat(localeID, szstring[loop]=='L'?DATE_LONGDATE:DATE_SHORTDATE, &st, NULL, &sztemp[pos], MaxOutLen-pos);
										break;
									case 'K':
									case 'k':
										res = GetTimeFormat(localeID, szstring[loop]=='K'?0:TIME_NOSECONDS, &st, NULL, &sztemp[pos], MaxOutLen-pos);
									default:
										break;
								}
								if (res>0) res--;
								else res=0;
							} break;
						default: 
							break;
					}
					break;
				case 'z': case 'Z':
					if (!tzFilled) {
						tzDrift = GetTimeZone(tzName);
						tzFilled = TRUE;
					}
					if (szstring[loop]=='Z') res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%s"),tzName);
					else {
						res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%02d:%02d"),div(tzDrift,60).quot,tzDrift<0?-div(tzDrift,60).rem:div(tzDrift,60).rem);
					}
					break;
				case 'b':
					res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("\x0D\x0A"));
					break;
				case 't':
					res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%c"),_T('\t'));
					break;
				default:	
					res = _sntprintf(&sztemp[pos],MaxOutLen-pos,_T("%%%c"),szstring[loop]);
					break;
			}
		}
		if (res<0) break;
		else pos += res;
	}
	//sztemp[MaxOutLen]=sztemp[pos]=0; // Just in case
	return pos;
}



