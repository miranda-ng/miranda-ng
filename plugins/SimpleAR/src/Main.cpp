/*
Plugin of miranda IM(ICQ) for Communicating with users of the baseProtocol.
Copyright (C) 2004 Daniel Savi (dss@brturbo.com)

-> Based on J. Lawler BaseProtocol <-

Added:
 - plugin option window
 - make.bat (vc++)
 - tinyclib http://msdn.microsoft.com/msdnmag/issues/01/01/hood/default.aspx
 - C++ version

Miranda ICQ: the free icq client for MS Windows
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede
*/

#include "stdafx.h"

CMPlugin g_plugin;

HGENMENU hToggle, hEnableMenu;
int interval;

wchar_t *ptszDefaultMsg[] =
{
	LPGENW("I am currently away. I will reply to you when I am back."),
	LPGENW("I am currently very busy and can't spare any time to talk with you. Sorry..."),
	LPGENW("I am not available right now."),
	LPGENW("I am now doing something, I will talk to you later.")
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {46BF191F-8DFB-4656-88B2-4C20BE4CFA44}
	{ 0x46bf191f, 0x8dfb, 0x4656, { 0x88, 0xb2, 0x4c, 0x20, 0xbe, 0x4c, 0xfa, 0x44 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR ToggleEnable(WPARAM, LPARAM)
{
	BOOL fEnabled = !g_plugin.getByte(KEY_ENABLED, 1);
	g_plugin.setByte(KEY_ENABLED, fEnabled);

	if (fEnabled)
		Menu_ModifyItem(hEnableMenu, LPGENW("Disable Auto&reply"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hEnableMenu, LPGENW("Enable Auto&reply"), iconList[1].hIcolib);
	return 0;
}

INT_PTR Toggle(WPARAM hContact, LPARAM)
{
	BOOL on = !g_plugin.getByte(hContact, "TurnedOn");
	g_plugin.setByte(hContact, "TurnedOn", on);
	on = !on;

	if (on)
		Menu_ModifyItem(hToggle, LPGENW("Turn off Autoanswer"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hToggle, LPGENW("Turn on Autoanswer"), iconList[1].hIcolib);
	return 0;
}

INT OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	BOOL on = !g_plugin.getByte(hContact, "TurnedOn");
	if (on)
		Menu_ModifyItem(hToggle, LPGENW("Turn off Autoanswer"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hToggle, LPGENW("Turn on Autoanswer"), iconList[1].hIcolib);
	return 0;
}

INT CheckDefaults(WPARAM, LPARAM)
{
	interval = g_plugin.getWord(KEY_REPEATINTERVAL, 300);

	wchar_t *ptszVal = g_plugin.getWStringA(KEY_HEADING);
	if (ptszVal == nullptr) // Heading not set
		g_plugin.setWString(KEY_HEADING, TranslateT("Dear %user%, the owner left the following message:"));
	else
		mir_free(ptszVal);

	for (int c = ID_STATUS_ONLINE; c < ID_STATUS_IDLE; c++) {
		if (c == ID_STATUS_ONLINE || c == ID_STATUS_FREECHAT || c == ID_STATUS_INVISIBLE)
			continue;
		else {
			char szStatus[6] = { 0 };
			mir_snprintf(szStatus, "%d", c);
			ptszVal = g_plugin.getWStringA(szStatus);
			if (ptszVal == nullptr) {
				wchar_t *ptszDefault;
				if (c < ID_STATUS_FREECHAT)
					// This mode does not have a preset message
					ptszDefault = ptszDefaultMsg[c - ID_STATUS_ONLINE - 1];
				else
					ptszDefault = nullptr;
				if (ptszDefault)
					g_plugin.setWString(szStatus, TranslateW(ptszDefault));
			}
			else mir_free(ptszVal);
		}
	}
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);

	BOOL fEnabled = g_plugin.getByte(KEY_ENABLED, 1);
	if (fEnabled)
		Menu_ModifyItem(hEnableMenu, LPGENW("Disable Auto&reply"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hEnableMenu, LPGENW("Enable Auto&reply"), iconList[1].hIcolib);
	return 0;
}

INT addEvent(WPARAM hContact, LPARAM hDBEvent)
{
	BOOL fEnabled = g_plugin.getByte(KEY_ENABLED, 1);
	if (!fEnabled || !hContact || !hDBEvent)
		return FALSE;	/// unspecified error

	char *pszProto = Proto_GetBaseAccountName(hContact);
	int status = Proto_GetStatus(pszProto);
	if (status == ID_STATUS_ONLINE || status == ID_STATUS_FREECHAT || status == ID_STATUS_INVISIBLE)
		return FALSE;

	// detect size of msg
	DBEVENTINFO dbei = {};
	if (db_event_get(hDBEvent, &dbei))
		return 0;

	if ((dbei.eventType != EVENTTYPE_MESSAGE) || (dbei.flags == DBEF_READ)) {
		// we need EVENTTYPE_MESSAGE event..
		return FALSE;
	}
	else {
		// needed event has occured..
		if (!dbei.cbBlob)	/// invalid size
			return FALSE;

		wchar_t *ptszVal = db_get_wsa(hContact, "Protocol", "p");
		if (ptszVal == nullptr) // Contact with no protocol ?!!
			return FALSE;
		mir_free(ptszVal);

		if (!Contact::OnList(hContact))
			return FALSE;

		if (g_plugin.getByte(hContact, "TurnedOn"))
			return FALSE;

		if (!(dbei.flags & DBEF_SENT)) {
			int timeBetween = time(0) - g_plugin.getDword(hContact, "LastReplyTS");
			if (timeBetween > interval || g_plugin.getWord(hContact, "LastStatus") != status) {
				size_t msgLen = 1;
				int isQun = db_get_b(hContact, pszProto, "IsQun", 0);
				if (isQun)
					return FALSE;

				char szStatus[6] = { 0 };
				mir_snprintf(szStatus, "%d", status);
				ptszVal = g_plugin.getWStringA(szStatus);
				if (ptszVal) {
					if (*ptszVal) {
						CMStringW ptszTemp;

						wchar_t *ptszNick = db_get_wsa(hContact, pszProto, "Nick");
						if (ptszNick == nullptr) {
							mir_free(ptszVal);
							return FALSE;
						}

						msgLen += mir_wstrlen(ptszVal);

						wchar_t *ptszHead = g_plugin.getWStringA(KEY_HEADING);
						if (ptszHead != nullptr) {
							ptszTemp = ptszHead;
							ptszTemp.Replace(L"%user%", ptszNick);
							msgLen += mir_wstrlen(ptszTemp);
							mir_free(ptszHead);
						}

						wchar_t *ptszTemp2 = (wchar_t *)mir_alloc(sizeof(wchar_t) * (msgLen + 5));
						mir_snwprintf(ptszTemp2, msgLen + 5, L"%s\r\n\r\n%s", ptszTemp.c_str(), ptszVal);
						if (ServiceExists(MS_VARS_FORMATSTRING)) {
							ptszTemp = variables_parse(ptszTemp2, nullptr, hContact);
						}
						else ptszTemp = Utils_ReplaceVarsW(ptszTemp2);

						T2Utf pszUtf(ptszTemp);
						ProtoChainSend(hContact, PSS_MESSAGE, 0, pszUtf);

						dbei.eventType = EVENTTYPE_MESSAGE;
						dbei.flags = DBEF_UTF | DBEF_SENT;
						dbei.szModule = pszProto;
						dbei.timestamp = time(0);
						dbei.cbBlob = (int)mir_strlen(pszUtf) + 1;
						dbei.pBlob = (uint8_t*)pszUtf;
						db_event_add(hContact, &dbei);

						mir_free(ptszTemp2);
						mir_free(ptszNick);
					}
					mir_free(ptszVal);
				}
			}
		}

		g_plugin.setDword(hContact, "LastReplyTS", time(0));
		g_plugin.setWord(hContact, "LastStatus", status);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

IconItem iconList[] =
{
	{ LPGEN("Disable Auto&reply"), "Disable Auto&reply", IDI_OFF },
	{ LPGEN("Enable Auto&reply"), "Enable Auto&reply", IDI_ON }
};

int CMPlugin::Load()
{
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0xac1c64a, 0x82ca, 0x4845, 0x86, 0x89, 0x59, 0x76, 0x12, 0x74, 0x72, 0x7b);
	mi.position = 500090000;
	mi.name.w = L"";
	mi.pszService = MODULENAME"/ToggleEnable";
	hEnableMenu = Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, ToggleEnable);

	SET_UID(mi, 0xb290cccd, 0x4ecc, 0x475e, 0x87, 0xcb, 0x51, 0xf4, 0x3b, 0xc3, 0x44, 0x9c);
	mi.position = -0x7FFFFFFF;
	mi.name.w = L"";
	mi.pszService = MODULENAME"/ToggleAutoanswer";
	hToggle = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, Toggle);

	// add hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, addEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, CheckDefaults);

	g_plugin.registerIcon("Simple Auto Replier", iconList);
	return 0;
}
