#include "stdafx.h"

wchar_t *weekDays[7] = { LPGENW("Sunday"), LPGENW("Monday"), LPGENW("Tuesday"), LPGENW("Wednesday"), LPGENW("Thursday"), LPGENW("Friday"), LPGENW("Saturday") };

wchar_t *months[12] =
{
	LPGENW("January"), LPGENW("February"), LPGENW("March"), LPGENW("April"), LPGENW("May"), LPGENW("June"),
	LPGENW("July"), LPGENW("August"), LPGENW("September"), LPGENW("October"), LPGENW("November"), LPGENW("December")
};

///////////////////////////////////////////////////////////////////////////////
// Template formatting for options dialog

CMStringW TplFormatStringEx(int tpl, wchar_t *sztpl, ItemData *item)
{
	if (tpl < 0 || tpl >= TPL_COUNT || !sztpl)
		return mir_wstrdup(L"");

	TemplateVars vars;

	auto &T = templates[tpl];
	for (auto &it : T.vf)
		if (it)
			it(&vars, item->hContact, item);

	CMStringW buf;
	for (wchar_t *p = sztpl; *p; p++) {
		if (*p == '%') {
			wchar_t *var = vars.GetVar((p[1] & 0xff));
			if (var)
				buf.Append(var);
			p++;
		}
		else buf.AppendChar(*p);
	}

	return buf;
}

///////////////////////////////////////////////////////////////////////////////
// Template formatting for the control

static void AppendUnicodeToBuffer(CMStringA &buf, const wchar_t *p, TemplateVars *vars)
{
	for (; *p; p++) {
		if (*p == '\r' && p[1] == '\n') {
			buf.Append("\\p ");
			p++;
		}
		else if (*p == '\n') {
			buf.Append("\\p ");
		}
		else if (*p == '\t') {
			buf.Append("\\tab ");
		}
		else if (*p == '\\' || *p == '{' || *p == '}') {
			buf.AppendChar('\\');
			buf.AppendChar((char)*p);
		}
		else if (*p == '[') {
			if (p[1] == 'c' && p[3] == ']') {
				buf.AppendFormat("\\cf%c ", p[2]);
				p += 3;
				continue;
			}

			char *pEnd = "";
			if (p[1] == '/') {
				pEnd = "0";
				p++;
			}
			if (p[1] == 'b' && p[2] == ']') {
				buf.AppendFormat("\\b%s ", pEnd);
				p += 2;
			}
			else if (p[1] == 'i' && p[2] == ']') {
				buf.AppendFormat("\\i%s ", pEnd);
				p += 2;
			}
			else if (p[1] == 'u' && p[2] == ']') {
				buf.AppendFormat("\\ul%s ", pEnd);
				p += 2;
			}
			else if (p[1] == 's' && p[2] == ']') {
				buf.AppendFormat("\\strike%s ", pEnd);
				p += 2;
			}
			else if (p[1] == 'c' && p[2] == ']') {
				buf.Append("\\cf1 ");
				p += 2;
			}
			else buf.AppendChar('[');
		}
		else if (*p == '%' && vars) {
			wchar_t *var = vars->GetVar((p[1] & 0xff));
			if (var)
				AppendUnicodeToBuffer(buf, var, 0);
			p++;
		}
		else if (*p < 128) {
			buf.AppendChar((char)*p);
		}
		else {
			buf.AppendFormat("\\u%d ?", *p);
		}
	}
}

CMStringA TplFormatRtf(int tpl, MCONTACT hContact, ItemData *item)
{
	if (tpl < 0 || tpl >= TPL_COUNT)
		return CMStringA("");

	auto &T = templates[tpl];

	TemplateVars vars;

	for (auto &it : T.vf)
		if (it)
			it(&vars, hContact, item);

	CMStringA buf;
	buf.Append("{\\rtf1\\ansi\\deff0");

	COLORREF cr = GetSysColor(COLOR_WINDOWTEXT);
	buf.AppendFormat("{\\colortbl \\red%u\\green%u\\blue%u;", GetRValue(cr), GetGValue(cr), GetBValue(cr));
	cr = g_colorTable[(item->dbe.flags & DBEF_SENT) ? COLOR_OUTNICK : COLOR_INNICK].cl;
	buf.AppendFormat("\\colortbl \\red%u\\green%u\\blue%u;}", GetRValue(cr), GetGValue(cr), GetBValue(cr));

	wchar_t tmp[2] = { 0, 0 };

	buf.Append("\\par ");

	buf.Append("{\\uc1 ");
	AppendUnicodeToBuffer(buf, (T.value != nullptr) ? T.value : T.defvalue, &vars);
	buf.Append("}}");
	return buf;
}

///////////////////////////////////////////////////////////////////////////////
// Template formatting for copying text

CMStringW TplFormatString(int tpl, MCONTACT hContact, ItemData *item)
{
	if (tpl < 0 || tpl >= TPL_COUNT)
		return CMStringW();

	auto &T = templates[tpl];
	if (T.value == nullptr)
		T.value = mir_wstrdup(T.defvalue);

	TemplateVars vars;

	for (auto &it : T.vf)
		if (it)
			it(&vars, hContact, item);

	CMStringW buf;

	for (wchar_t *p = T.value; *p; p++) {
		if (*p == '%') {
			wchar_t *var = vars.GetVar((p[1] & 0xff));
			if (var)
				buf.Append(var);

			p++;
		}
		else buf.AppendChar(*p);
	}

	return buf;
}

///////////////////////////////////////////////////////////////////////////////
// TemplateVars members

TemplateVars::TemplateVars()
{
	memset(&vars, 0, sizeof(vars));
}

TemplateVars::~TemplateVars()
{
	for (auto &V : vars)
		if (V.val && V.del)
			mir_free(V.val);
}

// Loading variables
void vfGlobal(TemplateVars *vars, MCONTACT hContact, ItemData *)
{
	//  %%: simply % character
	vars->SetVar('%', L"%", false);

	//  %n: line break
	vars->SetVar('n', L"\x0d\x0a", false);

	// %S: my nick (not for messages)
	char* proto = Proto_GetBaseAccountName(hContact);
	ptrW nick(Contact::GetInfo(CNF_DISPLAY, 0, proto));
	vars->SetVar('S', nick, true);
}

void vfContact(TemplateVars *vars, MCONTACT hContact, ItemData *)
{
	// %N: buddy's nick (not for messages)
	wchar_t *nick = (hContact == 0) ? TranslateT("System history") : Clist_GetContactDisplayName(hContact, 0);
	vars->SetVar('N', nick, false);

	wchar_t buf[20];
	// %c: event count
	mir_snwprintf(buf, L"%d", db_event_count(hContact));
	vars->SetVar('c', buf, true);
}

void vfSystem(TemplateVars *vars, MCONTACT hContact, ItemData *)
{
	// %N: buddy's nick (not for messages)
	vars->SetVar('N', TranslateT("System event"), false);

	// %c: event count
	wchar_t  buf[20];
	mir_snwprintf(buf, L"%d", db_event_count(hContact));
	vars->SetVar('c', buf, true);
}

void vfEvent(TemplateVars *vars, MCONTACT, ItemData *item)
{
	HICON hIcon;
	wchar_t buf[100];

	//  %N: Nickname
	if (item->dbe.flags & DBEF_SENT) {
		if (!item->wszNick) {
			char *proto = Proto_GetBaseAccountName(item->hContact);
			ptrW nick(Contact::GetInfo(CNF_DISPLAY, 0, proto));
			vars->SetNick(nick, false);
		}
		else vars->SetNick(item->wszNick, false);
	}
	else {
		wchar_t *nick = (item->wszNick) ? item->wszNick : Clist_GetContactDisplayName(item->hContact, 0);
		vars->SetNick(nick, true);
	}

	//  %I: Icon
	switch (item->dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		hIcon = g_plugin.getIcon(IDI_SENDMSG);
		break;
	case EVENTTYPE_FILE:
		hIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
		break;
	case EVENTTYPE_STATUSCHANGE:
		hIcon = g_plugin.getIcon(IDI_SIGNIN);
		break;
	default:
		hIcon = g_plugin.getIcon(IDI_UNKNOWN);
		break;
	}
	mir_snwprintf(buf, L"[$hicon=%p$]", hIcon);
	vars->SetVar('I', buf, true);

	if (item->dbe.flags & DBEF_BOOKMARK) {
		mir_snwprintf(buf, L"[$hicon=%p$]", g_plugin.getIcon(IDI_BOOKMARK));
		vars->SetVar('B', buf, true);
	}

	//  %i: Direction icon
	if (item->dbe.flags & DBEF_SENT)
		hIcon = g_plugin.getIcon(IDI_MSGOUT);
	else
		hIcon = g_plugin.getIcon(IDI_MSGIN);

	mir_snwprintf(buf, L"[$hicon=%p$]", hIcon);
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

		GetDateFormatW(iLocale, 0, &st, L"dd.MM.yyyy, ", buf, _countof(buf));
		GetTimeFormatW(iLocale, 0, &st, L"HH:mm", buf + 12, _countof(buf));
		vars->SetVar('t', buf, true);

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

void vfMessage(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfFile(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfUrl(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfSign(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfAuth(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfAdded(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfPresence(TemplateVars* vars, MCONTACT, ItemData* item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfDeleted(TemplateVars *vars, MCONTACT, ItemData *item)
{
	vars->SetVar('M', item->getWBuf(), false);
}

void vfOther(TemplateVars *vars, MCONTACT, ItemData *item)
{
	auto *pText = item->getWBuf();
	vars->SetVar('M', mir_wstrlen(pText) == 0 ? TranslateT("Unknown event") : pText, false);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TemplateVars::SetNick(wchar_t *v, bool bIncoming)
{
	CMStringW wszNick(FORMAT, L"[color=%d]%s[/color]", g_colorTable[(bIncoming) ? COLOR_INNICK : COLOR_OUTNICK].cl, v);

	auto &V = vars['N'];
	if (V.del)
		mir_free(V.val);
	V.val = wszNick.Detach();
	V.del = true;
}

void TemplateVars::SetVar(uint8_t id, wchar_t *v, bool d)
{
	auto &V = vars[id];
	if (V.del)
		mir_free(V.val);

	V.val = (d) ? mir_wstrdup(v) : v;
	V.del = d;
}

/////////////////////////////////////////////////////////////////////////////////////////

HICON TemplateInfo::getIcon() const
{
	return (iIcon < 0) ? Skin_LoadIcon(-iIcon) : g_plugin.getIcon(iIcon);
}

TemplateInfo templates[TPL_COUNT] =
{
	{ "tpl/interface/title", LPGENW("Interface"), IDI_NEWSTORY, LPGENW("Window title"),
		LPGENW("%N - history [%c messages total]"), 0, 0,
		{ vfGlobal, vfContact, 0, 0, 0 } },

	{ "tpl/msglog/msg", LPGENW("Message log"), IDI_SENDMSG, LPGENW("Messages"),
		L"%I%i%B[b]%N, %t:[/b]\r\n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/msg_head", LPGENW("Message log"), IDI_SENDMSG, LPGENW("Group head"),
		L"%I%i%B[b]%N, %t:[/b] %M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/msg_grp", LPGENW("Message log"), IDI_SENDMSG, LPGENW("Grouped messages"),
		L"%I%i%B[b]%h:%m:%s:[/b] %M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/msglog/file", LPGENW("Message log"), -SKINICON_EVENT_FILE, LPGENW("Files"),
		L"%I%i%B[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/msglog/status", LPGENW("Message log"), IDI_SIGNIN, LPGENW("Status changes"),
		L"%I%i%B[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/msglog/presense", LPGENW("Message log"), IDI_UNKNOWN, LPGENW("Presence requests"),
		L"%I%i%B[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfPresence, 0 } },
	{ "tpl/msglog/other", LPGENW("Message log"), IDI_UNKNOWN, LPGENW("Other events"),
		L"%I%i%B[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/msglog/authrq", LPGENW("Message log"), IDI_UNKNOWN, LPGENW("Authorization requests"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/msglog/added", LPGENW("Message log"), IDI_UNKNOWN, LPGENW("'You were added' events"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/msglog/deleted", LPGENW("Message log"), IDI_UNKNOWN, LPGENW("'You were deleted' events"),
		L"%I%i[b]%N, %t:[/b]%n%M", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfDeleted, 0 } },

	{ "tpl/copy/msg", LPGENW("Clipboard"), IDI_SENDMSG, LPGENW("Messages"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfMessage, 0 } },
	{ "tpl/copy/file", LPGENW("Clipboard"), -SKINICON_EVENT_FILE, LPGENW("Files"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfFile, 0 } },
	{ "tpl/copy/url", LPGENW("Clipboard"), -SKINICON_EVENT_URL, LPGENW("URLs"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfUrl, 0 } },
	{ "tpl/copy/status", LPGENW("Clipboard"), IDI_SIGNIN, LPGENW("Status changes"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfSign, 0 } },
	{ "tpl/copy/presence", LPGENW("Clipboard"), IDI_UNKNOWN, LPGENW("Presence requests"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfPresence, 0 } },
	{ "tpl/copy/other", LPGENW("Clipboard"), IDI_UNKNOWN, LPGENW("Other events"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfContact, vfEvent, vfOther, 0 } },

	{ "tpl/copy/authrq", LPGENW("Clipboard"), IDI_UNKNOWN, LPGENW("Authorization requests"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAuth, 0 } },
	{ "tpl/copy/added", LPGENW("Clipboard"), IDI_UNKNOWN, LPGENW("'You were added' events"),
		L"%N, %t:\x0d\x0a%M%n", 0, 0,
		{ vfGlobal, vfEvent, vfSystem, vfAdded, 0 } },
	{ "tpl/copy/deleted", LPGENW("Clipboard"), IDI_UNKNOWN, LPGENW("'You were deleted' events"),
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
