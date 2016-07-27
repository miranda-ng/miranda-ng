#ifndef __debug_h__
#define __debug_h__

#define PlugName "SplashScreen"
extern wchar_t szLogFile[MAX_PATH];

/*
 * output a notification message.
 * may accept a hContact to include the contacts nickname in the notification message...
 * the actual message is using printf() rules for formatting and passing the arguments...
 *
 */

int inline _DebugPopup(MCONTACT hContact, wchar_t *fmt, ...)
{
	va_list va;
	wchar_t debug[1024];

	va_start(va, fmt);
	mir_snwprintf(debug, fmt, va);
    
	if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == 1) {
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		if(hContact != 0)
			mir_wstrncpy(ppd.lptzContactName, (wchar_t*)pcli->pfnGetContactDisplayName(hContact, 0), MAX_CONTACTNAME);
		else
			mir_wstrncpy(ppd.lptzContactName, _A2W(PlugName), MAX_CONTACTNAME);
		mir_wstrncpy(ppd.lptzText, debug, MAX_SECONDLINE - 20);
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
	fclose(_wfopen(szLogFile, L"w"));
	return 0;
}

/*
 * logging func
 */

void inline logMessage(wchar_t *func, wchar_t *msg)
{
	FILE *f = _wfopen(szLogFile, L"a");
	fwprintf(f, L"%s:\t\t%s\n", func, msg);
	fclose(f);
}

#endif // __debug_h__