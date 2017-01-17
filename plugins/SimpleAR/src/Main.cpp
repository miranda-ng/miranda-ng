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

CLIST_INTERFACE *pcli;
HINSTANCE hinstance;

HGENMENU hToggle, hEnableMenu;
BOOL gbVarsServiceExist = FALSE;
INT interval;
int hLangpack;

wchar_t* ptszDefaultMsg[] = {
	LPGENW("I am currently away. I will reply to you when I am back."),
	LPGENW("I am currently very busy and can't spare any time to talk with you. Sorry..."),
	LPGENW("I am not available right now."),
	LPGENW("I am now doing something, I will talk to you later."),
	LPGENW("I am on the phone right now. I will get back to you very soon."),
	LPGENW("I am having meal right now. I will get back to you very soon.")
};

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {46BF191F-8DFB-4656-88B2-4C20BE4CFA44}
	{ 0x46bf191f, 0x8dfb, 0x4656, { 0x88, 0xb2, 0x4c, 0x20, 0xbe, 0x4c, 0xfa, 0x44 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD, LPVOID)
{
	hinstance = hinst;
	return TRUE;
}

INT_PTR ToggleEnable(WPARAM, LPARAM)
{
	BOOL fEnabled = !db_get_b(NULL, protocolname, KEY_ENABLED, 1);
	db_set_b(NULL, protocolname, KEY_ENABLED, fEnabled);

	if (fEnabled)
		Menu_ModifyItem(hEnableMenu, LPGENW("Disable Auto&reply"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hEnableMenu, LPGENW("Enable Auto&reply"), iconList[1].hIcolib);
	return 0;
}

INT_PTR Toggle(WPARAM hContact, LPARAM)
{
	BOOL on = !db_get_b(hContact, protocolname, "TurnedOn", 0);
	db_set_b(hContact, protocolname, "TurnedOn", on);
	on = !on;

	if (on)
		Menu_ModifyItem(hToggle, LPGENW("Turn off Autoanswer"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hToggle, LPGENW("Turn on Autoanswer"), iconList[1].hIcolib);
	return 0;
}

INT OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	BOOL on = !db_get_b(hContact, protocolname, "TurnedOn", 0);
	if (on)
		Menu_ModifyItem(hToggle, LPGENW("Turn off Autoanswer"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hToggle, LPGENW("Turn on Autoanswer"), iconList[1].hIcolib);
	return 0;
}

INT CheckDefaults(WPARAM, LPARAM)
{
	interval = db_get_w(NULL, protocolname, KEY_REPEATINTERVAL, 300);

	wchar_t *ptszVal = db_get_wsa(NULL, protocolname, KEY_HEADING);
	if (ptszVal == 0)
		// Heading not set
		db_set_ws(NULL, protocolname, KEY_HEADING, TranslateT("Dear %user%, the owner left the following message:"));
	else
		mir_free(ptszVal);

	for (int c = ID_STATUS_ONLINE; c < ID_STATUS_IDLE; c++) {
		if (c == ID_STATUS_ONLINE || c == ID_STATUS_FREECHAT || c == ID_STATUS_INVISIBLE)
			continue;
		else {
			char szStatus[6] = { 0 };
			mir_snprintf(szStatus, "%d", c);
			ptszVal = db_get_wsa(NULL, protocolname, szStatus);
			if (ptszVal == 0) {
				wchar_t *ptszDefault;
				if (c < ID_STATUS_FREECHAT)
					// This mode does not have a preset message
					ptszDefault = ptszDefaultMsg[c - ID_STATUS_ONLINE - 1];
				else if (c > ID_STATUS_INVISIBLE)
					ptszDefault = ptszDefaultMsg[c - ID_STATUS_ONLINE - 3];
				else
					ptszDefault = 0;
				if (ptszDefault)
					db_set_ws(NULL, protocolname, szStatus, TranslateW(ptszDefault));
			}
			else
				mir_free(ptszVal);
		}
	}
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;

	BOOL fEnabled = db_get_b(NULL, protocolname, KEY_ENABLED, 1);
	if (fEnabled)
		Menu_ModifyItem(hEnableMenu, LPGENW("Disable Auto&reply"), iconList[0].hIcolib);
	else
		Menu_ModifyItem(hEnableMenu, LPGENW("Enable Auto&reply"), iconList[1].hIcolib);
	return 0;
}

INT addEvent(WPARAM hContact, LPARAM hDBEvent)
{
	BOOL fEnabled = db_get_b(NULL, protocolname, KEY_ENABLED, 1);
	if (!fEnabled || !hContact || !hDBEvent)
		return FALSE;	/// unspecifyed error

	char* pszProto = GetContactProto(hContact);
	int status = CallProtoService(pszProto, PS_GETSTATUS, 0, 0);
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
		if (ptszVal == NULL) // Contact with no protocol ?!!
			return FALSE;
		mir_free(ptszVal);

		if (db_get_b(hContact, "CList", "NotOnList", 0))
			return FALSE;

		if (db_get_b(hContact, protocolname, "TurnedOn", 0))
			return FALSE;

		if (!(dbei.flags & DBEF_SENT)) {
			int timeBetween = time(NULL) - db_get_dw(hContact, protocolname, "LastReplyTS", 0);
			if (timeBetween > interval || db_get_w(hContact, protocolname, "LastStatus", 0) != status) {
				size_t msgLen = 1;
				int isQun = db_get_b(hContact, pszProto, "IsQun", 0);
				if (isQun)
					return FALSE;

				char szStatus[6] = { 0 };
				mir_snprintf(szStatus, "%d", status);
				ptszVal = db_get_wsa(NULL, protocolname, szStatus);
				if (ptszVal) {
					if (*ptszVal) {
						CMStringW ptszTemp;

						wchar_t *ptszNick = db_get_wsa(hContact, pszProto, "Nick");
						if (ptszNick == 0) {
							mir_free(ptszVal);
							return FALSE;
						}

						msgLen += mir_wstrlen(ptszVal);

						wchar_t *ptszHead = db_get_wsa(NULL, protocolname, KEY_HEADING);
						if (ptszHead != NULL) {
							ptszTemp = ptszHead;
							ptszTemp.Replace(L"%user%", ptszNick);
							msgLen += mir_wstrlen(ptszTemp);
							mir_free(ptszHead);
						}

						wchar_t *ptszTemp2 = (wchar_t*)mir_alloc(sizeof(wchar_t) * (msgLen + 5));
						mir_snwprintf(ptszTemp2, msgLen + 5, L"%s\r\n\r\n%s", ptszTemp.c_str(), ptszVal);
						if (ServiceExists(MS_VARS_FORMATSTRING)) {
							ptszTemp = variables_parse(ptszTemp2, 0, hContact);
						}
						else ptszTemp = Utils_ReplaceVarsW(ptszTemp2);

						T2Utf pszUtf(ptszTemp);
						ProtoChainSend(hContact, PSS_MESSAGE, 0, pszUtf);

						dbei.eventType = EVENTTYPE_MESSAGE;
						dbei.flags = DBEF_UTF | DBEF_SENT; //DBEF_READ;
						dbei.szModule = pszProto;
						dbei.timestamp = time(NULL);
						dbei.cbBlob = (int)mir_strlen(pszUtf) + 1;
						dbei.pBlob = (PBYTE)pszUtf;
						db_event_add(hContact, &dbei);

						mir_free(ptszTemp2);
						mir_free(ptszNick);
					}
					mir_free(ptszVal);
				}
			}
		}

		db_set_dw(hContact, protocolname, "LastReplyTS", time(NULL));
		db_set_w(hContact, protocolname, "LastStatus", status);
	}
	return 0;
}

IconItemT iconList[] =
{
	{ LPGENW("Disable Auto&reply"), "Disable Auto&reply", IDI_OFF },
	{ LPGENW("Enable Auto&reply"), "Enable Auto&reply", IDI_ON }
};

extern "C" int __declspec(dllexport)Load(void)
{
	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	CreateServiceFunction(protocolname"/ToggleEnable", ToggleEnable);
	CreateServiceFunction(protocolname"/ToggleAutoanswer", Toggle);

	CMenuItem mi;

	SET_UID(mi, 0xac1c64a, 0x82ca, 0x4845, 0x86, 0x89, 0x59, 0x76, 0x12, 0x74, 0x72, 0x7b);
	mi.position = 500090000;
	mi.name.w = L"";
	mi.pszService = protocolname"/ToggleEnable";
	hEnableMenu = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xb290cccd, 0x4ecc, 0x475e, 0x87, 0xcb, 0x51, 0xf4, 0x3b, 0xc3, 0x44, 0x9c);
	mi.position = -0x7FFFFFFF;
	mi.name.w = L"";
	mi.pszService = protocolname"/ToggleAutoanswer";
	hToggle = Menu_AddContactMenuItem(&mi);

	//add hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, addEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, CheckDefaults);

	Icon_RegisterT(hinstance, L"Simple Auto Replier", iconList, _countof(iconList));

	return 0;
}

extern "C" __declspec(dllexport)int Unload(void)
{
	return 0;
}
