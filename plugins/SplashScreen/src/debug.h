#ifndef __debug_h__
#define __debug_h__

#define PlugName "SplashScreen"
extern TCHAR szLogFile[MAX_PATH];

/*
 * output a notification message.
 * may accept a hContact to include the contacts nickname in the notification message...
 * the actual message is using printf() rules for formatting and passing the arguments...
 *
 */

int inline _DebugPopup(MCONTACT hContact, TCHAR *fmt, ...)
{
	va_list va;
	TCHAR debug[1024];

	va_start(va, fmt);
	mir_sntprintf(debug, SIZEOF(debug), fmt, va);
    
	if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == 1) {
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		if(hContact != 0)
			mir_tstrncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), MAX_CONTACTNAME);
		else
			mir_tstrncpy(ppd.lptzContactName, _T(PlugName), MAX_CONTACTNAME);
		mir_tstrncpy(ppd.lptzText, debug, MAX_SECONDLINE - 20);
		ppd.colorText = RGB(255,255,255);
		ppd.colorBack = RGB(255,0,0);
		PUAddPopupT(&ppd);
	}
	return 0;
}

/*
 * initialize logfile
 */

int inline initLog()
{
	fclose(_tfopen(szLogFile, _T("w")));
	return 0;
}

/*
 * logging func
 */

void inline logMessage(TCHAR *func, TCHAR *msg)
{
	FILE *f = _tfopen(szLogFile, _T("a"));
	_ftprintf(f, _T("%s:\t\t%s\n"), func, msg);
	fclose(f);
}

#endif // __debug_h__