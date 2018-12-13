#include "stdafx.h"

void TplInitVars(TemplateVars *vars);
void TplCleanVars(TemplateVars *vars);
__forceinline TCHAR *TplGetVar(TemplateVars *vars, char id);
__forceinline void TplSetVar(TemplateVars *vars, char id, TCHAR *v, bool d);
int TplMeasureVars(TemplateVars *vars, TCHAR *str);

void vfGlobal(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfContact(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfSystem(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfEvent(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfMessage(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfFile(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfUrl(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfSign(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfAuth(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfAdded(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfDeleted(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);
void vfOther(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);

TemplateInfo templates[TPL_COUNT] =
{
	{ "tpl/interface/title", _T("Interface"), ICO_NEWSTORY, _T("Window Title"),
		_T("%N's Newstory [%c messages total]"), 0, 0,
		{ vfGlobal, vfContact, 0, 0, 0 } },
	{ "tpl/interface/passwordq", _T("Interface"), ICO_NOPASSWORD, _T("Password Query"),
		_T("Ooops, %N's history seems to be password protected, so type your password here:"), 0, 0,
		{ vfGlobal, vfContact, 0, 0, 0 } },

	{ "tpl/msglog/msg", _T("Message Log"), ICO_SENDMSG, _T("Messages"),
		_T("%I%i[b]%N, %t:[/b]\x0d\x0a%M"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/file", _T("Message Log"), ICO_FILE, _T("Files"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/msglog/url", _T("Message Log"), ICO_URL, _T("URLs"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfUrl, 0 } },
	{ "tpl/msglog/status", _T("Message Log"), ICO_SIGNIN, _T("Status Changes"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/msglog/other", _T("Message Log"), ICO_UNKNOWN, _T("Other Events"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/msglog/authrq", _T("Message Log"), ICO_UNKNOWN, _T("Authorization Requests"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/msglog/added", _T("Message Log"), ICO_UNKNOWN, _T("'You were added' events"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/msglog/deleted", _T("Message Log"), ICO_UNKNOWN, _T("'You were deleted' events"),
		_T("%I%i[b]%N, %t:[/b]%n%M"), 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } },

	{ "tpl/copy/msg", _T("Clipboard"), ICO_SENDMSG, _T("Messages"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/copy/file", _T("Clipboard"), ICO_FILE, _T("Files"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/copy/url", _T("Clipboard"), ICO_URL, _T("URLs"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfUrl, 0 } },
	{ "tpl/copy/status", _T("Clipboard"), ICO_SIGNIN, _T("Status Changes"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/copy/other", _T("Clipboard"), ICO_UNKNOWN, _T("Other Events"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/copy/authrq", _T("Clipboard"), ICO_UNKNOWN, _T("Authorization Requests"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/copy/added", _T("Clipboard"), ICO_UNKNOWN, _T("'You were added' events"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/copy/deleted", _T("Clipboard"), ICO_UNKNOWN, _T("'You were deleted' events"),
		_T("%N, %t:\x0d\x0a%M%n"), 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } }
};

void LoadTemplates()
{
	for (int i = 0; i < TPL_COUNT; i++) {
		DBVARIANT dbv = { 0 };
		db_get_ws(0, MODULENAME, templates[i].setting, &dbv);
		if (templates[i].value)
			free(templates[i].value);
		if (dbv.pwszVal) {
			templates[i].value = _tcsdup(dbv.pwszVal);
		}
		else {
			templates[i].value = 0;
		}
		db_free(&dbv);
	}
}

void SaveTemplates()
{
	for (int i = 0; i < TPL_COUNT; i++)
		if (templates[i].value)
			db_set_ws(0, MODULENAME, templates[i].setting, templates[i].value);
}

TCHAR *TplFormatStringEx(int tpl, TCHAR *sztpl, MCONTACT hContact, HistoryArray::ItemData *item)
{
	if ((tpl < 0) || (tpl >= TPL_COUNT) || !sztpl)
		return _tcsdup(_T(""));

	int i;
	TemplateVars vars;
	for (i = 0; i < 256; i++) {
		vars.del[i] = false;
		vars.val[i] = 0;
	}
	
	for (i = 0; i < TemplateInfo::VF_COUNT; i++)
		if (templates[tpl].vf[i])
			templates[tpl].vf[i](VFM_VARS, &vars, hContact, item);

	TCHAR *buf = (TCHAR *)malloc(sizeof(TCHAR)*(TplMeasureVars(&vars, sztpl) + 1));
	TCHAR *bufptr = buf;
	for (TCHAR *p = sztpl; *p; p++) {
		if (*p == '%') {
			TCHAR *var = TplGetVar(&vars, (char)(p[1] & 0xff));
			if (var) {
				lstrcpy(bufptr, var);
				bufptr += lstrlen(var);
			}
			p++;
		}
		else *bufptr++ = *p;
	}
	*bufptr = 0;
	return buf;
}

TCHAR *TplFormatString(int tpl, MCONTACT hContact, HistoryArray::ItemData *item)
{
	if ((tpl < 0) || (tpl >= TPL_COUNT))
		return _tcsdup(_T(""));

	if (!templates[tpl].value)
		templates[tpl].value = _tcsdup(templates[tpl].defvalue);

	int i;
	TemplateVars vars;
	for (i = 0; i < 256; i++) {
		vars.del[i] = false;
		vars.val[i] = 0;
	}

	for (i = 0; i < TemplateInfo::VF_COUNT; i++)
		if (templates[tpl].vf[i])
			templates[tpl].vf[i](VFM_VARS, &vars, hContact, item);

	TCHAR *buf = (TCHAR *)malloc(sizeof(TCHAR)*(TplMeasureVars(&vars, templates[tpl].value) + 1));
	TCHAR *bufptr = buf;
	for (TCHAR *p = templates[tpl].value; *p; p++) {
		if (*p == '%') {
			TCHAR *var = TplGetVar(&vars, (char)(p[1] & 0xff));
			if (var) {
				lstrcpy(bufptr, var);
				bufptr += lstrlen(var);
			}
			p++;
		}
		else *bufptr++ = *p;
	}
	*bufptr = 0;
	return buf;
}

// Variable management
void TplInitVars(TemplateVars *vars)
{
	for (int i = 0; i < 256; i++) {
		vars->val[i] = 0;
		vars->del[i] = false;
	}
}

void TplCleanVars(TemplateVars *vars)
{
	for (int i = 0; i < 256; i++)
		if (vars->val[i] && vars->del[i]) {
			free(vars->val[i]);
			vars->val[i] = 0;
			vars->del[i] = false;
		}
}

__forceinline TCHAR *TplGetVar(TemplateVars *vars, char id)
{
	return vars->val[id];
}

__forceinline void TplSetVar(TemplateVars *vars, char id, TCHAR *v, bool d)
{
	if (vars->val[id] && vars->del[id])
		free(vars->val[id]);
	vars->val[id] = v;
	vars->del[id] = d;
}

int TplMeasureVars(TemplateVars *vars, TCHAR *str)
{
	int res = 0;
	for (TCHAR *p = str; *p; p++) {
		if (*p == '%') {
			TCHAR *var = TplGetVar(vars, (char)(p[1] & 0xff));
			if (var) res += lstrlen(var);
			p++;
		}
		else res++;
	}
	return res;
}

// Loading variables
void vfGlobal(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData*)
{
	//  %%: simply % character
	TplSetVar(vars, '%', _T("%"), false);

	//  %n: line break
	TplSetVar(vars, 'n', _T("\x0d\x0a"), false);

	// %M: my nick (not for messages)
	wchar_t *buf = Clist_GetContactDisplayName(0, 0);
	TplSetVar(vars, 'M', buf, false);
}

void vfContact(int, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData*)
{
	// %N: buddy's nick (not for messages)
	wchar_t *buff = Clist_GetContactDisplayName(hContact, 0);
	TplSetVar(vars, 'N', buff, false);

	// %c: event count
	TCHAR *buf = new TCHAR[20];
	wsprintf(buf, _T("%d"), db_event_count(hContact));
	TplSetVar(vars, 'c', buf, false);
}

void vfSystem(int, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData*)
{
	// %N: buddy's nick (not for messages)
	TplSetVar(vars, 'N', /*TranslateTS*/_T("System Event"), false);

	// %c: event count
	TCHAR *buf = new TCHAR[20];
	wsprintf(buf, _T("%d"), db_event_count(hContact));
	TplSetVar(vars, 'c', buf, false);
}

void vfEvent(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	HICON hIcon;
	TCHAR *s;

	//  %U: UIN (contextual, own uin for sent, buddys UIN for received messages)

	//  %N: Nickname
	wchar_t *buff = Clist_GetContactDisplayName(item->dbe.flags&DBEF_SENT ? 0 : (WPARAM)item->hContact, 0);
	TplSetVar(vars, 'N', buff, false);

	//  %I: Icon
	switch (item->dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		hIcon = GetIcon(ICO_SENDMSG);
		break;
	case EVENTTYPE_FILE:
		hIcon = GetIcon(ICO_FILE);
		break;
	case EVENTTYPE_URL:
		hIcon = GetIcon(ICO_URL);
		break;
	case EVENTTYPE_STATUSCHANGE:
		hIcon = GetIcon(ICO_SIGNIN);
		break;
	default:
		hIcon = GetIcon(ICO_UNKNOWN);
		break;
	}
	s = (TCHAR *)calloc(64, sizeof(TCHAR));
	wsprintf(s, _T("[$hicon=%d$]"), hIcon);
	TplSetVar(vars, 'I', s, true);

	//  %i: Direction icon
	if (item->dbe.flags & DBEF_SENT)
		hIcon = GetIcon(ICO_MSGOUT);
	else
		hIcon = GetIcon(ICO_MSGIN);

	s = (TCHAR *)calloc(64, sizeof(TCHAR));
	wsprintf(s, _T("[$hicon=%d$]"), hIcon);
	TplSetVar(vars, 'i', s, true);

	// %D: direction symbol
	if (item->dbe.flags & DBEF_SENT)
		TplSetVar(vars, 'D', _T("<<"), false);
	else
		TplSetVar(vars, 'D', _T(">>"), false);

	//  %t: timestamp
	TCHAR *buf = (TCHAR *)calloc(100, sizeof(TCHAR));
	_tcsftime(buf, 100, _T("%d.%m.%Y, %H:%M"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 't', buf, true);

	//	DBTIMETOSTRING tts;
	//	tts.cbDest = 100;
	//	tts.szFormat = "d, t";
	//	tts.szDest = (char *)calloc(100, 1);
	//	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, item->dbe.timestamp, (LPARAM)&tts);
	//	TplSetVar(vars, 't', tts.szDest, true);

	//	tts.szFormat = "";
	//	tts.szDest = (char *)calloc(100, 1);
	//	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, item->dbe.timestamp, (LPARAM)&tts);
	//	TplSetVar(vars, 't', tts.szDest, true);

	//  %h: hour (24 hour format, 0-23)
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%H"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'h', buf, true);

	//  %a: hour (12 hour format)
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%h"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'a', buf, true);

	//  %m: minute
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%M"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'm', buf, true);

	//  %s: second
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%S"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 's', buf, true);

	//  %o: month
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%m"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'o', buf, true);

	//  %d: day of month
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%d"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'd', buf, true);

	//  %y: year
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%Y"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'y', buf, true);

	//  %w: day of week (Sunday, Monday.. translateable)
	buf = (TCHAR *)calloc(25, sizeof(TCHAR));
	_tcsftime(buf, 25, _T("%A"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'w', TranslateW(buf), false);

	//  %p: AM/PM symbol
	buf = (TCHAR *)calloc(5, sizeof(TCHAR));
	_tcsftime(buf, 5, _T("%p"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'p', buf, true);

	//  %O: Name of month, translateable
	buf = (TCHAR *)calloc(25, sizeof(TCHAR));
	_tcsftime(buf, 25, _T("%B"), localtime((time_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'O', TranslateW(buf), false);
}

void vfMessage(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfFile(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfUrl(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfSign(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfAuth(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfAdded(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfDeleted(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', item->getTBuf(), false);
}

void vfOther(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData*)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', _T("Unknown Event"), false);
}

// Option dialog

INT_PTR CALLBACK OptTemplatesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int CurrentTemplate;
	switch (msg) {
	case WM_INITDIALOG:
		CurrentTemplate = -1;
		{
			HTREEITEM hGroup = 0;
			HTREEITEM hFirst = 0;

			HIMAGELIST himgTree = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
			TreeView_SetImageList(GetDlgItem(hwnd, IDC_TEMPLATES), himgTree, TVSIL_NORMAL);

			ImageList_AddIcon(himgTree, GetIcon(ICO_TPLGROUP));

			for (int i = 0; i < TPL_COUNT; i++) {
				if (!i || lstrcmp(templates[i].group, templates[i - 1].group)) {
					if (hGroup)
						TreeView_Expand(GetDlgItem(hwnd, IDC_TEMPLATES), hGroup, TVE_EXPAND);

					TVINSERTSTRUCT tvis;
					tvis.hParent = 0;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvis.item.state = tvis.item.stateMask = TVIS_BOLD;
					tvis.item.iSelectedImage = tvis.item.iImage = 0;
					tvis.item.pszText = templates[i].group;
					tvis.item.lParam = -1;
					hGroup = TreeView_InsertItem(GetDlgItem(hwnd, IDC_TEMPLATES), &tvis);

					if (!hFirst) hFirst = hGroup;
				}

				TVINSERTSTRUCT tvis;
				tvis.hParent = hGroup;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvis.item.pszText = templates[i].title;
				tvis.item.iSelectedImage = tvis.item.iImage =
					ImageList_AddIcon(himgTree, GetIcon(templates[i].icon));
				tvis.item.lParam = i;
				TreeView_InsertItem(GetDlgItem(hwnd, IDC_TEMPLATES), &tvis);
			}

			if (hGroup)
				TreeView_Expand(GetDlgItem(hwnd, IDC_TEMPLATES), hGroup, TVE_EXPAND);

			TreeView_SelectItem(GetDlgItem(hwnd, IDC_TEMPLATES), hFirst);
			TreeView_EnsureVisible(GetDlgItem(hwnd, IDC_TEMPLATES), hFirst);
		}
		SendMessage(GetDlgItem(hwnd, IDC_DISCARD), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_VARHELP), BUTTONSETASFLATBTN, 0, 0);

		SendMessage(GetDlgItem(hwnd, IDC_DISCARD), BUTTONADDTOOLTIP, (WPARAM)Translate("Cancel Edit"), 0);
		SendMessage(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), BUTTONADDTOOLTIP, (WPARAM)Translate("Update Preview"), 0);
		SendMessage(GetDlgItem(hwnd, IDC_VARHELP), BUTTONADDTOOLTIP, (WPARAM)Translate("Help on Variables"), 0);

		SendMessage(GetDlgItem(hwnd, IDC_DISCARD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_RESET));
		SendMessage(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_PREVIEW));
		SendMessage(GetDlgItem(hwnd, IDC_VARHELP), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_VARHELP));

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDITTEMPLATE:
			SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);

		case IDC_UPDATEPREVIEW:
			if (templates[CurrentTemplate].tmpValue)
				free(templates[CurrentTemplate].tmpValue);
			{
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDITTEMPLATE)) + 1;
				templates[CurrentTemplate].tmpValue = (TCHAR *)malloc(length * sizeof(TCHAR));
				GetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].tmpValue, length);

				HistoryArray::ItemData item;
				item.hContact = db_find_first();
				while (item.hContact && !item.hEvent) {
					item.hEvent = db_event_first(item.hContact);
					if (!item.hEvent)
						item.hContact = db_find_next(item.hContact);
				}

				if (item.hContact && item.hEvent) {
					item.load(ELM_DATA);
					TCHAR *preview = TplFormatStringEx(CurrentTemplate, templates[CurrentTemplate].tmpValue, item.hContact, &item);
					SetWindowText(GetDlgItem(hwnd, IDC_PREVIEW), preview);
					//						SetWindowText(GetDlgItem(hwnd, IDC_GPREVIEW), preview);
					SetWindowText(GetDlgItem(hwnd, IDC_GPREVIEW), _T("$hit :)"));
					free(preview);
				}
				else {
					SetWindowText(GetDlgItem(hwnd, IDC_PREVIEW), _T(""));
					SetWindowText(GetDlgItem(hwnd, IDC_GPREVIEW), _T(""));
				}
			}
			break;

		case IDC_DISCARD:
			if (templates[CurrentTemplate].tmpValue)
				free(templates[CurrentTemplate].tmpValue);
			templates[CurrentTemplate].tmpValue = 0;
			if (templates[CurrentTemplate].value)
				SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].value);
			else
				SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].defvalue);

			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_UPDATEPREVIEW, 0), 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				for (int i = 0; i < TPL_COUNT; i++) {
					if (templates[i].tmpValue) {
						if (templates[i].tmpValue)
							free(templates[i].tmpValue);
					}
				}
				return TRUE;

			case PSN_APPLY:
				for (int i = 0; i < TPL_COUNT; i++) {
					if (templates[i].tmpValue) {
						if (templates[i].value)
							free(templates[i].value);
						templates[i].value = templates[i].tmpValue;
						templates[i].tmpValue = 0;
					}
				}
				SaveTemplates();
				return TRUE;
			}
			break;

		case IDC_TEMPLATES:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_SELCHANGED:
			case TVN_SELCHANGING:
				{
					LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

					TVITEM tvi;
					tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd, IDC_TEMPLATES));
					tvi.mask = TVIF_PARAM;
					TreeView_GetItem(GetDlgItem(hwnd, IDC_TEMPLATES), &tvi);

					if ((tvi.lParam < 0) || (tvi.lParam >= TPL_COUNT)) {
						EnableWindow(GetDlgItem(hwnd, IDC_EDITTEMPLATE), FALSE);
						EnableWindow(GetDlgItem(hwnd, IDC_GPREVIEW), FALSE);
						EnableWindow(GetDlgItem(hwnd, IDC_PREVIEW), FALSE);
						EnableWindow(GetDlgItem(hwnd, IDC_DISCARD), FALSE);
						EnableWindow(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), FALSE);
						EnableWindow(GetDlgItem(hwnd, IDC_VARHELP), FALSE);
						/*								HTREEITEM hItem = TreeView_GetChild(GetDlgItem(hwnd, IDC_TEMPLATES), tvi.hItem);
														if (hItem)
														{
															TreeView_Expand(GetDlgItem(hwnd, IDC_TEMPLATES), tvi.hItem, TVE_EXPAND);
															TreeView_SelectItem(GetDlgItem(hwnd, IDC_TEMPLATES), hItem);
														}*/
						break;
					}
					else {
						EnableWindow(GetDlgItem(hwnd, IDC_EDITTEMPLATE), TRUE);
						EnableWindow(GetDlgItem(hwnd, IDC_GPREVIEW), TRUE);
						EnableWindow(GetDlgItem(hwnd, IDC_PREVIEW), TRUE);
						EnableWindow(GetDlgItem(hwnd, IDC_DISCARD), TRUE);
						EnableWindow(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), TRUE);
						EnableWindow(GetDlgItem(hwnd, IDC_VARHELP), TRUE);
					}

					if ((lpnmtv->itemOld.mask&TVIF_HANDLE) && lpnmtv->itemOld.hItem && (lpnmtv->itemOld.hItem != lpnmtv->itemNew.hItem) && (lpnmtv->itemOld.lParam >= 0) && (lpnmtv->itemOld.lParam < TPL_COUNT)) {
						if (templates[lpnmtv->itemOld.lParam].tmpValue)
							free(templates[lpnmtv->itemOld.lParam].tmpValue);
						int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDITTEMPLATE)) + 1;
						templates[lpnmtv->itemOld.lParam].tmpValue = (TCHAR *)malloc(length * sizeof(TCHAR));
						GetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[lpnmtv->itemOld.lParam].tmpValue, length);
					}

					CurrentTemplate = tvi.lParam;

					if (templates[CurrentTemplate].tmpValue)
						SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].tmpValue);
					else if (templates[CurrentTemplate].value)
						SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].value);
					else
						SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].defvalue);

					PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_UPDATEPREVIEW, 0), 0);
				}
				break;

			case TVN_KEYDOWN:
			case NM_CLICK:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

				/*
										{
											TVHITTESTINFO hti;
											hti.pt.x = (short)LOWORD(GetMessagePos());
											hti.pt.y = (short)HIWORD(GetMessagePos());
											ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
											if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
												if(hti.flags&TVHT_ONITEMSTATEICON)
													SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
										}
				*/

			}
			break;
		}
		break;
	}
	return FALSE;
}
