#include "stdafx.h"

int TplMeasureVars(TemplateVars* vars, wchar_t* str);

wchar_t *weekDays[7] = { LPGENW("Sunday"), LPGENW("Monday"), LPGENW("Tuesday"), LPGENW("Wednesday"), LPGENW("Thursday"), LPGENW("Friday"), LPGENW("Saturday") };

wchar_t *months[12] =
{
	LPGENW("January"), LPGENW("February"), LPGENW("March"), LPGENW("April"), LPGENW("May"), LPGENW("June"),
	LPGENW("July"), LPGENW("August"), LPGENW("September"), LPGENW("October"), LPGENW("November"), LPGENW("December")
};

wchar_t *TplFormatStringEx(int tpl, wchar_t *sztpl, MCONTACT hContact, ItemData *item)
{
	if (tpl < 0 || tpl >= TPL_COUNT || !sztpl)
		return mir_wstrdup(L"");

	TemplateVars vars;
	memset(&vars, 0, sizeof(vars));

	auto &T = templates[tpl];
	for (int i = 0; i < TemplateInfo::VF_COUNT; i++)
		if (T.vf[i])
			T.vf[i](VFM_VARS, &vars, hContact, item);

	wchar_t *buf = (wchar_t *)mir_alloc(sizeof(wchar_t) * (TplMeasureVars(&vars, sztpl) + 1));
	wchar_t *bufptr = buf;
	for (wchar_t *p = sztpl; *p; p++) {
		if (*p == '%') {
			wchar_t *var = vars.GetVar((p[1] & 0xff));
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

wchar_t *TplFormatString(int tpl, MCONTACT hContact, ItemData *item)
{
	if ((tpl < 0) || (tpl >= TPL_COUNT))
		return mir_wstrdup(L"");

	auto &T = templates[tpl];
	if (T.value == nullptr)
		T.value = mir_wstrdup(T.defvalue);

	TemplateVars vars;
	memset(&vars, 0, sizeof(vars));

	for (int i = 0; i < TemplateInfo::VF_COUNT; i++)
		if (T.vf[i])
			T.vf[i](VFM_VARS, &vars, hContact, item);

	wchar_t *buf = (wchar_t *)mir_alloc(sizeof(wchar_t) * (TplMeasureVars(&vars, T.value) + 1));
	wchar_t *bufptr = buf;
	for (wchar_t *p = T.value; *p; p++) {
		if (*p == '%') {
			wchar_t *var = vars.GetVar((p[1] & 0xff));
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
	memset(&vars, 0, sizeof(vars));
}

void TplCleanVars(TemplateVars *vars)
{
	for (auto &V : vars->vars)
		if (V.val && V.del)
			mir_free(V.val);

	memset(&vars, 0, sizeof(vars));
}

int TplMeasureVars(TemplateVars *vars, wchar_t *str)
{
	int res = 0;
	for (wchar_t *p = str; *p; p++) {
		if (*p == '%') {
			wchar_t *var = vars->GetVar(p[1] & 0xff);
			if (var)
				res += (int)mir_wstrlen(var);
			p++;
		}
		else res++;
	}
	return res;
}

// Loading variables
void vfGlobal(int, TemplateVars *vars, MCONTACT hContact, ItemData *)
{
	//  %%: simply % character
	vars->SetVar('%', L"%", false);

	//  %n: line break
	vars->SetVar('n', L"\x0d\x0a", false);

	// %S: my nick (not for messages)
	char* proto = Proto_GetBaseAccountName(hContact);
	ptrW nick(Contact::GetInfo(CNF_DISPLAY, 0, proto));
	vars->SetVar('S', nick, false);
}

void vfContact(int, TemplateVars *vars, MCONTACT hContact, ItemData *)
{
	// %N: buddy's nick (not for messages)
	wchar_t *nick = (hContact == 0) ? TranslateT("System history") : Clist_GetContactDisplayName(hContact, 0);
	vars->SetVar('N', nick, false);

	wchar_t buf[20];
	// %c: event count
	mir_snwprintf(buf, L"%d", db_event_count(hContact));
	vars->SetVar('c', buf, false);
}

void vfSystem(int, TemplateVars *vars, MCONTACT hContact, ItemData *)
{
	// %N: buddy's nick (not for messages)
	vars->SetVar('N', TranslateT("System event"), false);

	// %c: event count
	wchar_t  buf[20];
	mir_snwprintf(buf, L"%d", db_event_count(hContact));
	vars->SetVar('c', buf, false);
}

void vfEvent(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	HICON hIcon;
	wchar_t buf[100];

	//  %N: Nickname
	if (item->dbe.flags & DBEF_SENT) {
		char *proto = Proto_GetBaseAccountName(item->hContact);
		ptrW nick(Contact::GetInfo(CNF_DISPLAY, 0, proto));
		vars->SetVar('N', nick, false);
	}
	else {
		wchar_t *nick = (item->wszNick) ? item->wszNick : Clist_GetContactDisplayName(item->hContact, 0);
		vars->SetVar('N', nick, false);
	}

	//  %I: Icon
	switch (item->dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		hIcon = g_plugin.getIcon(ICO_SENDMSG);
		break;
	case EVENTTYPE_FILE:
		hIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
		break;
	case EVENTTYPE_STATUSCHANGE:
		hIcon = g_plugin.getIcon(ICO_SIGNIN);
		break;
	default:
		hIcon = g_plugin.getIcon(ICO_UNKNOWN);
		break;
	}
	mir_snwprintf(buf, L"[$hicon=%d$]", hIcon);
	vars->SetVar('I', buf, true);

	//  %i: Direction icon
	if (item->dbe.flags & DBEF_SENT)
		hIcon = g_plugin.getIcon(ICO_MSGOUT);
	else
		hIcon = g_plugin.getIcon(ICO_MSGIN);

	mir_snwprintf(buf, L"[$hicon=%d$]", hIcon);
	vars->SetVar('i', buf, true);

	// %D: direction symbol
	if (item->dbe.flags & DBEF_SENT)
		vars->SetVar('D', L"<<", false);
	else
		vars->SetVar('D', L">>", false);

	//  %t: timestamp
	SYSTEMTIME st;
	if (!TimeZone_GetSystemTime(nullptr, item->dbe.timestamp, &st, 0)) {
		int iLocale = Langpack_GetDefaultLocale();

		CMStringW tmp;
		GetDateFormatW(iLocale, 0, &st, L"dd.MM.yyyy, ", buf, _countof(buf)); tmp += buf;
		GetTimeFormatW(iLocale, 0, &st, L"HH:mm", buf, _countof(buf)); tmp += buf;
		vars->SetVar('t', tmp, true);

		//  %h: hour (24 hour format, 0-23)
		GetTimeFormatW(iLocale, 0, &st, L"HH", buf, _countof(buf));
		vars->SetVar('h', buf, true);

		//  %a: hour (12 hour format)
		GetTimeFormatW(iLocale, 0, &st, L"hh", buf, _countof(buf));
		vars->SetVar('a', buf, true);

		//  %m: minute
		GetTimeFormatW(iLocale, 0, &st, L"mm", buf, _countof(buf));
		vars->SetVar('m', buf, true);

		//  %s: second
		GetTimeFormatW(iLocale, 0, &st, L"ss", buf, _countof(buf));
		vars->SetVar('s', buf, true);

		//  %o: month
		GetDateFormatW(iLocale, 0, &st, L"MM", buf, _countof(buf));
		vars->SetVar('o', buf, true);

		//  %d: day of month
		GetDateFormatW(iLocale, 0, &st, L"dd", buf, _countof(buf));
		vars->SetVar('d', buf, true);

		//  %y: year
		GetDateFormatW(iLocale, 0, &st, L"yyyy", buf, _countof(buf));
		vars->SetVar('y', buf, true);

		//  %w: day of week (Sunday, Monday... translatable)
		vars->SetVar('w', TranslateW(weekDays[st.wDayOfWeek]), false);

		//  %p: AM/PM symbol
		vars->SetVar('p', (st.wHour > 11) ? L"PM" : L"AM", false);

		//  %O: Name of month, translatable
		vars->SetVar('O', TranslateW(months[st.wMonth-1]), false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// %M: the message string itself

void vfMessage(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfFile(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	CMStringW wszFileName(item->getWBuf());
	wszFileName.Replace('\\', '/');
	wszFileName = L"[url]file://" + wszFileName + L"[/url]";

	vars->SetVar('M', wszFileName, false);
}

void vfUrl(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfSign(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfAuth(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfAdded(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfPresence(int, TemplateVars* vars, MCONTACT, ItemData* item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfDeleted(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfOther(int, TemplateVars *vars, MCONTACT, ItemData *item)
{
	auto *pText = item->getWBuf();
	vars->SetVar('M', mir_wstrlen(pText) == 0 ? TranslateT("Unknown event") : pText, false);
}

/////////////////////////////////////////////////////////////////////////////////////////

HICON TemplateInfo::getIcon() const
{
	return (iIcon < 0) ? Skin_LoadIcon(-iIcon) : g_plugin.getIcon(iIcon);
}

TemplateInfo templates[TPL_COUNT] =
{
	{ "tpl/interface/title", LPGENW("Interface"), ICO_NEWSTORY, LPGENW("Window title"),
		L"%N - history [%c messages total]", 0, 0,
		{ vfGlobal, vfContact, 0, 0, 0 } },

	{ "tpl/msglog/msg", LPGENW("Message log"), ICO_SENDMSG, LPGENW("Messages"),
	L"%I%i[b][color=red]%N[/color], %t:[/b]\x0d\x0a%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/msg_grp", LPGENW("Message log"), ICO_SENDMSG, LPGENW("Grouped messages"),
		L"%I%i[b]%t:[/b] %M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/file", LPGENW("Message log"), -SKINICON_EVENT_FILE, LPGENW("Files"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/msglog/status", LPGENW("Message log"), ICO_SIGNIN, LPGENW("Status changes"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/msglog/presense", LPGENW("Message log"), ICO_UNKNOWN, LPGENW("Presence requests"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfPresence, 0 } },
	{ "tpl/msglog/other", LPGENW("Message log"), ICO_UNKNOWN, LPGENW("Other events"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/msglog/authrq", LPGENW("Message log"), ICO_UNKNOWN, LPGENW("Authorization requests"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/msglog/added", LPGENW("Message log"), ICO_UNKNOWN, LPGENW("'You were added' events"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/msglog/deleted", LPGENW("Message log"), ICO_UNKNOWN, LPGENW("'You were deleted' events"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } },

	{ "tpl/copy/msg", LPGENW("Clipboard"), ICO_SENDMSG, LPGENW("Messages"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/copy/file", LPGENW("Clipboard"), -SKINICON_EVENT_FILE, LPGENW("Files"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/copy/url", LPGENW("Clipboard"), -SKINICON_EVENT_URL, LPGENW("URLs"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfUrl, 0 } },
	{ "tpl/copy/status", LPGENW("Clipboard"), ICO_SIGNIN, LPGENW("Status changes"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/copy/presence", LPGENW("Clipboard"), ICO_UNKNOWN, LPGENW("Presence requests"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfPresence, 0 } },
	{ "tpl/copy/other", LPGENW("Clipboard"), ICO_UNKNOWN, LPGENW("Other events"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/copy/authrq", LPGENW("Clipboard"), ICO_UNKNOWN, LPGENW("Authorization requests"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/copy/added", LPGENW("Clipboard"), ICO_UNKNOWN, LPGENW("'You were added' events"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/copy/deleted", LPGENW("Clipboard"), ICO_UNKNOWN, LPGENW("'You were deleted' events"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } }
};

void LoadTemplates()
{
	for (auto &it : templates)
		replaceStrW(it.value, g_plugin.getWStringA(it.setting));
}

void SaveTemplates()
{
	for (auto &it : templates) {
		if (it.value) {
			if (mir_wstrcmp(it.value, it.defvalue))
				g_plugin.setWString(it.setting, it.value);
			else
				g_plugin.delSetting(it.setting);
		}
		else g_plugin.delSetting(it.setting);
	}
}
