#include "stdafx.h"

DWORD toggleBit(DWORD dw, DWORD bit)
{
	if (dw & bit)
		return dw & ~bit;
	return dw | bit;
}

bool CheckFilter(wchar_t *buf, wchar_t *filter)
{
	//	MessageBox(0, buf, filter, MB_OK);
	int l1 = (int)mir_wstrlen(buf);
	int l2 = (int)mir_wstrlen(filter);
	for (int i = 0; i < l1 - l2 + 1; i++)
		if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, buf + i, l2, filter, l2) == CSTR_EQUAL)
			return true;
	return false;
}

void CopyText(HWND hwnd, const wchar_t *text)
{
	OpenClipboard(hwnd);
	EmptyClipboard();
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * (int)(mir_wstrlen(text) + 1));
	wchar_t *s = (wchar_t *)GlobalLock(hMem);
	mir_wstrcpy(s, text);
	GlobalUnlock(hMem);
	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();
	//	GlobalFree(hMem);
}

/*void ExportHistory(HANDLE hContact, char *fnTemplate, char *fn, HWND hwndHistory)
{
/*
	char *szHeader    = "---header---";
	char *szMsgIn     = "---msgin---";
	char *szMsgOut    = "---msgout---";
	char *szFile      = "---file---";
	char *szURL       = "---url---";
	char *szStatus    = "---status---";
	char *szOther     = "---other---";
	char *szFooter    = "---footer---";
	char *szEnd       = "---end---";

	char *tHeader = 0;
	char *tMsgIn = 0;
	char *tMsgOut = 0;
	char *tFile = 0;
	char *tURL = 0;
	char *tStatus = 0;
	char *tOther = 0;
	char *tFooter = 0;
	char *tEnd = 0;

	char **tActive = 0;
	int size;

	char line[1024];

	FILE *f;

	f = fopen(fnTemplate, "rt");
	while (!feof(f))
	{
		fgets(line, 1024, f);
		int linesize = lstrlen(line)-1;
		if (*line) line[linesize] = 0;

		if (!lstrcmp(line, szHeader))
		{
			tActive = &tHeader;
			size = 0;
		} else
		if (!lstrcmp(line, szMsgIn))
		{
			tActive = &tMsgIn;
			size = 0;
		} else
		if (!lstrcmp(line, szMsgOut))
		{
			tActive = &tMsgOut;
			size = 0;
		} else
		if (!lstrcmp(line, szFile))
		{
			tActive = &tFile;
			size = 0;
		} else
		if (!lstrcmp(line, szURL))
		{
			tActive = &tURL;
			size = 0;
		} else
		if (!lstrcmp(line, szStatus))
		{
			tActive = &tStatus;
			size = 0;
		} else
		if (!lstrcmp(line, szOther))
		{
			tActive = &tOther;
			size = 0;
		} else
		if (!lstrcmp(line, szFooter))
		{
			tActive = &tFooter;
			size = 0;
		} else
		if (!lstrcmp(line, szEnd))
		{
			break;
		} else
		if (tActive)
		{
			line[linesize+0]  = '\n';
			line[linesize+1]  = '\0';
			*tActive = appendString(*tActive, line);
		}
	}
	fclose(f);

	f = fopen(fn, "wt");
	if (!f)
	{
		MessageBox(hwndHistory, Translate("Can't open optput file."), Translate("Newstory Export"), MB_ICONSTOP|MB_OK);
		free(tHeader);
		free(tMsgIn);
		free(tMsgOut);
		free(tFile);
		free(tURL);
		free(tStatus);
		free(tOther);
		free(tFooter);
		free(tEnd);
		return;
	}

	char *s;

	VFArgs args;
	args.hContact = hContact;
	args.event = 0;

	s = TplFormatStringEx(TPL_TITLE, tHeader, &args);
	fputs(s,f);
	free(s);

	int eventCount = SendMessage(hwndHistory, UM_GETEVENTCOUNT, 0, 0);
	for (int i = 0; i < eventCount; i++)
	{
		if (!SendMessage(hwndHistory, UM_SELECTED, i, 0))
			continue;

		args.event = (DBEVENTINFO *)SendMessage(hwndHistory, UM_GETEVENT, i, TRUE);
		switch (args.event->eventType)
		{
			case EVENTTYPE_MESSAGE:
				if (args.event->flags & DBEF_SENT)
				{
					s = TplFormatStringEx(TPL_MESSAGE, tMsgOut?tMsgOut:tOther, &args);
				} else
				{
					s = TplFormatStringEx(TPL_MESSAGE, tMsgIn?tMsgOut:tOther, &args);
				}
				break;

			case EVENTTYPE_FILE:
				s = TplFormatStringEx(TPL_FILE, tFile?tFile:tOther, &args);
				break;

			case EVENTTYPE_STATUSCHANGE:
				s = TplFormatStringEx(TPL_SIGN, tStatus?tStatus:tOther, &args);
				break;

			default:
				s = TplFormatStringEx(TPL_OTHER, tOther, &args);
				break;
		}

		fputs(s,f);
		free(s);
	}

	s = TplFormatStringEx(TPL_TITLE, tHeader, &args);
	fputs(s,f);
	free(s);

	fclose(f);

	free(tHeader);
	free(tMsgIn);
	free(tMsgOut);
	free(tFile);
	free(tURL);
	free(tStatus);
	free(tOther);
	free(tFooter);
	free(tEnd);

}*/
