#include "stdafx.h"

__forceinline wchar_t *TplGetVar(TemplateVars *vars, char id)
{
	return vars->val[id];
}

__forceinline void TplSetVar(TemplateVars *vars, char id, wchar_t *v, bool d)
{
	if (vars->val[id] && vars->del[id])
		mir_free(vars->val[id]);
	vars->val[id] = mir_wstrdup(v);
	vars->del[id] = d;
}

int TplMeasureVars(TemplateVars* vars, wchar_t* str);

wchar_t *TplFormatStringEx(int tpl, wchar_t *sztpl, MCONTACT hContact, HistoryArray::ItemData *item)
{
	if ((tpl < 0) || (tpl >= TPL_COUNT) || !sztpl)
		return mir_wstrdup(L"");

	int i;
	TemplateVars vars;
	for (i = 0; i < 256; i++) {
		vars.del[i] = false;
		vars.val[i] = 0;
	}

	for (i = 0; i < TemplateInfo::VF_COUNT; i++)
		if (templates[tpl].vf[i])
			templates[tpl].vf[i](VFM_VARS, &vars, hContact, item);

	wchar_t *buf = (wchar_t *)mir_alloc(sizeof(wchar_t) * (TplMeasureVars(&vars, sztpl) + 1));
	wchar_t *bufptr = buf;
	for (wchar_t *p = sztpl; *p; p++) {
		if (*p == '%') {
			wchar_t *var = TplGetVar(&vars, (char)(p[1] & 0xff));
			if (var) {
				mir_wstrcpy(bufptr, var);
				bufptr += mir_wstrlen(var);
			}
			p++;
		}
		else *bufptr++ = *p;
	}
	*bufptr = 0;
	return buf;
}

wchar_t *TplFormatString(int tpl, MCONTACT hContact, HistoryArray::ItemData *item)
{
	if ((tpl < 0) || (tpl >= TPL_COUNT))
		return mir_wstrdup(L"");

	if (!templates[tpl].value)
		templates[tpl].value = mir_wstrdup(templates[tpl].defvalue);

	int i;
	TemplateVars vars;
	for (i = 0; i < 256; i++) {
		vars.del[i] = false;
		vars.val[i] = 0;
	}

	for (i = 0; i < TemplateInfo::VF_COUNT; i++)
		if (templates[tpl].vf[i])
			templates[tpl].vf[i](VFM_VARS, &vars, hContact, item);

	wchar_t *buf = (wchar_t *)mir_alloc(sizeof(wchar_t) * (TplMeasureVars(&vars, templates[tpl].value) + 1));
	wchar_t *bufptr = buf;
	for (wchar_t *p = templates[tpl].value; *p; p++) {
		if (*p == '%') {
			wchar_t *var = TplGetVar(&vars, (char)(p[1] & 0xff));
			if (var) {
				mir_wstrcpy(bufptr, var);
				bufptr += mir_wstrlen(var);
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
			mir_free(vars->val[i]);
			vars->val[i] = 0;
			vars->del[i] = false;
		}
}

int TplMeasureVars(TemplateVars *vars, wchar_t *str)
{
	int res = 0;
	for (wchar_t *p = str; *p; p++) {
		if (*p == '%') {
			wchar_t *var = TplGetVar(vars, (char)(p[1] & 0xff));
			if (var) res += mir_wstrlen(var);
			p++;
		}
		else res++;
	}
	return res;
}

// Loading variables
void vfGlobal(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *)
{
	//  %%: simply % character
	TplSetVar(vars, '%', L"%", false);

	//  %n: line break
	TplSetVar(vars, 'n', L"\x0d\x0a", false);

	// %M: my nick (not for messages)
	//todo: not working now
	wchar_t *buf = Clist_GetContactDisplayName(0, 0);
	TplSetVar(vars, 'M', buf, false);
}

void vfContact(int, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *)
{
	// %N: buddy's nick (not for messages)
	wchar_t *nick = Clist_GetContactDisplayName(hContact, 0);
	TplSetVar(vars, 'N', nick, false);

	wchar_t buf[20];
	// %c: event count
	mir_snwprintf(buf, L"%d", db_event_count(hContact));
	TplSetVar(vars, 'c', buf, false);
}

void vfSystem(int, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *)
{
	// %N: buddy's nick (not for messages)
	TplSetVar(vars, 'N', L"System Event", false);

	// %c: event count
	wchar_t  buf[20];
	mir_snwprintf(buf, L"%d", db_event_count(hContact));
	TplSetVar(vars, 'c', buf, false);
}

void vfEvent(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *item)
{
	HICON hIcon;
	wchar_t buf[100];

	//  %N: Nickname
	if (item->dbe.flags & DBEF_SENT) {
		char *proto = Proto_GetBaseAccountName(item->hContact);
		ptrW nick(Contact_GetInfo(CNF_DISPLAY, 0, proto));
		TplSetVar(vars, 'N', nick, false);
	}
	else {
		wchar_t *nick = Clist_GetContactDisplayName(item->hContact, 0);
		TplSetVar(vars, 'N', nick, false);
	}

	//  %I: Icon
	switch (item->dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		hIcon = g_plugin.getIcon(ICO_SENDMSG);
		break;
	case EVENTTYPE_FILE:
		hIcon = g_plugin.getIcon(ICO_FILE);
		break;
	case EVENTTYPE_STATUSCHANGE:
		hIcon = g_plugin.getIcon(ICO_SIGNIN);
		break;
	default:
		hIcon = g_plugin.getIcon(ICO_UNKNOWN);
		break;
	}
	mir_snwprintf(buf, L"[$hicon=%d$]", hIcon);
	TplSetVar(vars, 'I', buf, true);

	//  %i: Direction icon
	if (item->dbe.flags & DBEF_SENT)
		hIcon = g_plugin.getIcon(ICO_MSGOUT);
	else
		hIcon = g_plugin.getIcon(ICO_MSGIN);

	mir_snwprintf(buf, L"[$hicon=%d$]", hIcon);
	TplSetVar(vars, 'i', buf, true);

	// %D: direction symbol
	if (item->dbe.flags & DBEF_SENT)
		TplSetVar(vars, 'D', L"<<", false);
	else
		TplSetVar(vars, 'D', L">>", false);

	//  %t: timestamp
	_tcsftime(buf, _countof(buf), L"%d.%m.%Y, %H:%M", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 't', buf, true);

	//  %h: hour (24 hour format, 0-23)
	_tcsftime(buf, _countof(buf), L"%H", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'h', buf, true);

	//  %a: hour (12 hour format)
	_tcsftime(buf, _countof(buf), L"%h", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'a', buf, true);

	//  %m: minute
	_tcsftime(buf, _countof(buf), L"%M", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'm', buf, true);

	//  %s: second
	_tcsftime(buf, _countof(buf), L"%S", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 's', buf, true);

	//  %o: month
	_tcsftime(buf, _countof(buf), L"%m", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'o', buf, true);

	//  %d: day of month
	_tcsftime(buf, _countof(buf), L"%d", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'd', buf, true);

	//  %y: year
	_tcsftime(buf, _countof(buf), L"%Y", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'y', buf, true);

	//  %w: day of week (Sunday, Monday.. translateable)
	_tcsftime(buf, _countof(buf), L"%A", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'w', TranslateW(buf), false);

	//  %p: AM/PM symbol
	_tcsftime(buf, _countof(buf), L"%p", _localtime32((__time32_t *)&item->dbe.timestamp));
	TplSetVar(vars, 'p', buf, true);

	//  %O: Name of month, translateable
	_tcsftime(buf, _countof(buf), L"%B", _localtime32((__time32_t *)&item->dbe.timestamp));
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

void vfOther(int, TemplateVars *vars, MCONTACT, HistoryArray::ItemData *)
{
	//  %M: the message string itself
	TplSetVar(vars, 'M', L"Unknown Event", false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TemplateInfo templates[TPL_COUNT] =
{
	{ "tpl/interface/title", L"Interface", ICO_NEWSTORY, L"Window Title",
		L"%N's Newstory [%c messages total]", 0, 0,
		{ vfGlobal, vfContact, 0, 0, 0 } },

	{ "tpl/msglog/msg", L"Message Log", ICO_SENDMSG, L"Messages",
		L"%I%i[b]%N, %t:[/b]\x0d\x0a%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/file", L"Message Log", ICO_FILE, L"Files",
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/msglog/status", L"Message Log", ICO_SIGNIN, L"Status Changes",
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/msglog/other", L"Message Log", ICO_UNKNOWN, L"Other Events",
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/msglog/authrq", L"Message Log", ICO_UNKNOWN, L"Authorization Requests",
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/msglog/added", L"Message Log", ICO_UNKNOWN, L"'You were added' events",
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/msglog/deleted", L"Message Log", ICO_UNKNOWN, L"'You were deleted' events",
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } },

	{ "tpl/copy/msg", L"Clipboard", ICO_SENDMSG, L"Messages",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/copy/file", L"Clipboard", ICO_FILE, L"Files",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/copy/url", L"Clipboard", ICO_URL, L"URLs",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfUrl, 0 } },
	{ "tpl/copy/status", L"Clipboard", ICO_SIGNIN, L"Status Changes",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/copy/other", L"Clipboard", ICO_UNKNOWN, L"Other Events",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/copy/authrq", L"Clipboard", ICO_UNKNOWN, L"Authorization Requests",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/copy/added", L"Clipboard", ICO_UNKNOWN, L"'You were added' events",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/copy/deleted", L"Clipboard", ICO_UNKNOWN, L"'You were deleted' events",
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } }
};

void LoadTemplates()
{
	for (int i = 0; i < TPL_COUNT; i++) {
		DBVARIANT dbv = { 0 };
		db_get_ws(0, MODULENAME, templates[i].setting, &dbv);
		if (templates[i].value)
			mir_free(templates[i].value);
		if (dbv.pwszVal) {
			templates[i].value = mir_wstrdup(dbv.pwszVal);
		}
		else {
			templates[i].value = 0;
		}
		db_free(&dbv);
	}
}

void SaveTemplates()
{
	for (auto &it : templates)
		if (it.value)
			db_set_ws(0, MODULENAME, it.setting, it.value);
}
