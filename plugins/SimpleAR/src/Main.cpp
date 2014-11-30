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

#include "Common.h"

HINSTANCE hinstance;

HGENMENU hToggle, hEnableMenu;
BOOL gbVarsServiceExist = FALSE;
INT interval;
int hLangpack;

TCHAR* ptszDefaultMsg[]={
	TranslateT("I am currently away. I will reply to you when I am back."),
	TranslateT("I am currently very busy and can't spare any time to talk with you. Sorry..."),
	TranslateT("I am not available right now."),
	TranslateT("I am now doing something, I will talk to you later."),
	TranslateT("I am on the phone right now. I will get back to you very soon."),
	TranslateT("I am having meal right now. I will get back to you very soon.")
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
	{0x46bf191f, 0x8dfb, 0x4656, {0x88, 0xb2, 0x4c, 0x20, 0xbe, 0x4c, 0xfa, 0x44}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	hinstance = hinst;
	return TRUE;
}

INT_PTR ToggleEnable(WPARAM wParam, LPARAM lParam)
{
	BOOL fEnabled = !db_get_b(NULL, protocolname, KEY_ENABLED, 1);
	db_set_b(NULL, protocolname, KEY_ENABLED, fEnabled);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	mi.ptszName = fEnabled ? LPGENT("Disable Auto&reply") : LPGENT("Enable Auto&reply");
	mi.icolibItem = fEnabled ? iconList[0].hIcolib : iconList[1].hIcolib;
	Menu_ModifyItem(hEnableMenu, &mi);
	return 0;
}

INT_PTR Toggle(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	BOOL on = !db_get_b(hContact, protocolname, "TurnedOn", 0);
	db_set_b(hContact, protocolname, "TurnedOn", on);
	on = !on;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	mi.ptszName = on ? LPGENT("Turn off Autoanswer") : LPGENT("Turn on Autoanswer");
	mi.icolibItem = on ? iconList[0].hIcolib : iconList[1].hIcolib;
	Menu_ModifyItem(hToggle, &mi);
	return 0;
}

INT OnPreBuildContactMenu(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	BOOL  on = !db_get_b(hContact, protocolname, "TurnedOn", 0);
	mi.ptszName = on ? LPGENT("Turn off Autoanswer") : LPGENT("Turn on Autoanswer");
	mi.icolibItem = on ? iconList[0].hIcolib : iconList[1].hIcolib;
	Menu_ModifyItem(hToggle, &mi);
	return 0;
}

INT CheckDefaults(WPARAM, LPARAM)
{
	DBVARIANT dbv;
	TCHAR *ptszDefault;
	char szStatus[6]={0};

	interval=db_get_w(NULL,protocolname,KEY_REPEATINTERVAL,300);

	if (db_get_ts(NULL,protocolname,KEY_HEADING,&dbv))
		// Heading not set
		db_set_ts(NULL,protocolname,KEY_HEADING,TranslateT("Dear %user%, the owner left the following message:"));
	else
		db_free(&dbv);

	for (int c=ID_STATUS_ONLINE; c<ID_STATUS_IDLE; c++)
	{
		mir_snprintf(szStatus,SIZEOF(szStatus),"%d",c);
		if (c == ID_STATUS_ONLINE || c == ID_STATUS_FREECHAT || c == ID_STATUS_INVISIBLE)
			continue;
		else
		{
			if (db_get_ts(NULL,protocolname,szStatus,&dbv))
			{
				if (c < ID_STATUS_FREECHAT)
					// This mode does not have a preset message
					ptszDefault=ptszDefaultMsg[c-ID_STATUS_ONLINE-1];
				else if(c > ID_STATUS_INVISIBLE)
					ptszDefault=ptszDefaultMsg[c-ID_STATUS_ONLINE-3];
				if (ptszDefault)
					db_set_ts(NULL,protocolname,szStatus,ptszDefault);
			}
			else
				db_free(&dbv);
		}
	}
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;

	BOOL fEnabled = db_get_b(NULL, protocolname, KEY_ENABLED, 1);
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	mi.ptszName = fEnabled ? LPGENT("Disable Auto&reply") : LPGENT("Enable Auto&reply");
	mi.icolibItem = fEnabled ? iconList[0].hIcolib : iconList[1].hIcolib;
	Menu_ModifyItem(hEnableMenu, &mi);

	return 0;
}

TCHAR* StrReplace (TCHAR* Search, TCHAR* Replace, TCHAR* Resource)
{
	int i = 0;
	int SearchLen = (int)_tcslen(Search);
	TCHAR* Work = mir_tstrdup(Replace);
	int ReplaceLen = (int)_tcslen(Work);

	TCHAR* Pointer = _tcsstr(Resource, Search);

	while (Pointer != NULL)
	{
		int PointerLen = (int)_tcslen(Pointer);
		int ResourceLen = (int)_tcslen(Resource);

		TCHAR* NewText = (TCHAR*)mir_calloc((ResourceLen - SearchLen + ReplaceLen + 1)*sizeof(TCHAR));

		_tcsncpy(NewText, Resource, ResourceLen - PointerLen);
		_tcscat(NewText, Work);
		_tcscat(NewText, Pointer + SearchLen);

		Resource = (TCHAR*)mir_realloc(Resource, (ResourceLen - SearchLen + ReplaceLen + 1)*sizeof(TCHAR));

		for (i = 0; i < (ResourceLen - SearchLen + ReplaceLen); i++)
			Resource[i] = NewText[i];
		Resource[i] = 0;
		mir_free(NewText);

		Pointer = _tcsstr(Resource + (ResourceLen - PointerLen + ReplaceLen), Search);
	}
	mir_free(Work);

	return Resource;
}

INT addEvent(WPARAM hContact, LPARAM lParam)
{
	HANDLE hDBEvent = (HANDLE)lParam;
	BOOL fEnabled = db_get_b(NULL, protocolname, KEY_ENABLED, 1);

	if (!fEnabled || !hContact || !hDBEvent)
		return FALSE;	/// unspecifyed error

	char* pszProto = GetContactProto(hContact);
	int status = CallProtoService(pszProto, PS_GETSTATUS, 0, 0);
	if (status == ID_STATUS_ONLINE || status == ID_STATUS_FREECHAT || status == ID_STATUS_INVISIBLE)
		return FALSE;

	DBEVENTINFO dbei = {sizeof(dbei)};
	db_event_get(hDBEvent, &dbei); /// detect size of msg

	if ((dbei.eventType != EVENTTYPE_MESSAGE) || (dbei.flags == DBEF_READ))
		return FALSE; /// we need EVENTTYPE_MESSAGE event..
	else
	{	/// needed event has occured..
		DBVARIANT dbv;

		if (!dbei.cbBlob)	/// invalid size
			return FALSE;

		if (db_get_ts(hContact,"Protocol","p",&dbv))
			// Contact with no protocol ?!!
			return FALSE;
		else
			db_free(&dbv);

		if(db_get_b(hContact, "CList", "NotOnList", 0))
			return FALSE;

		if(db_get_b(hContact, protocolname, "TurnedOn", 0))
			return FALSE;

		if (!( dbei.flags & DBEF_SENT))
		{
			int timeBetween=time(NULL)-db_get_dw(hContact,protocolname,"LastReplyTS",0);
			if (timeBetween>interval || db_get_w(hContact,protocolname,"LastStatus",0)!=status)
			{
				char szStatus[6]={0};
				int msgLen=1;
				int isQun=db_get_b(hContact,pszProto,"IsQun",0);
				if (isQun)
					return FALSE;

				mir_snprintf(szStatus,SIZEOF(szStatus),"%d",status);
				if (!db_get_ts(NULL,protocolname,szStatus,&dbv))
				{
					if (*dbv.ptszVal)
					{
						DBVARIANT dbvHead={0}, dbvNick={0};
						TCHAR *ptszTemp, *ptszTemp2;

						db_get_ts(hContact,pszProto,"Nick",&dbvNick);
						if (mir_tstrcmp(dbvNick.ptszVal, NULL) == 0)
						{
							db_free(&dbvNick);
							return FALSE;
						}

						msgLen += (int)_tcslen(dbv.ptszVal);
						if (!db_get_ts(NULL,protocolname,KEY_HEADING,&dbvHead))
						{
							ptszTemp = (TCHAR*)mir_alloc(sizeof(TCHAR) * (_tcslen(dbvHead.ptszVal)+1));
							_tcscpy(ptszTemp, dbvHead.ptszVal);
							ptszTemp = StrReplace(_T("%user%"), dbvNick.ptszVal, ptszTemp);
							msgLen += (int)(_tcslen(ptszTemp));
						}
						ptszTemp2 = (TCHAR*)mir_alloc(sizeof(TCHAR) * (msgLen+5));
						mir_sntprintf(ptszTemp2, msgLen+5, _T("%s\r\n\r\n%s"), ptszTemp, dbv.ptszVal);
						if (ServiceExists(MS_VARS_FORMATSTRING))
						{
							FORMATINFO fi;

							ZeroMemory(&fi, sizeof(fi));
							fi.cbSize = sizeof(fi);
							fi.flags = FIF_TCHAR;
							fi.tszFormat = ptszTemp2;
							ptszTemp = (TCHAR*)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
						}else
							ptszTemp = Utils_ReplaceVarsT(ptszTemp2);
						char* pszUtf = mir_utf8encodeT(ptszTemp);
						CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)pszUtf);

						dbei.cbSize = sizeof(dbei);
						dbei.eventType = EVENTTYPE_MESSAGE;
						dbei.flags = DBEF_UTF | DBEF_SENT; //DBEF_READ;
						dbei.szModule = pszProto;
						dbei.timestamp = time(NULL);
						dbei.cbBlob = mir_strlen(pszUtf) + 1;
						dbei.pBlob = (PBYTE)pszUtf;
						db_event_add(hContact, &dbei);

						mir_free(ptszTemp);
						mir_free(ptszTemp2);
						mir_free(pszUtf);
						if (dbvNick.ptszVal)
							db_free(&dbvNick);
						if (dbvHead.ptszVal)
							db_free(&dbvHead);
					}
					db_free(&dbv);
				}
			}
		}

		db_set_dw(hContact,protocolname,"LastReplyTS",time(NULL));
		db_set_w(hContact,protocolname,"LastStatus",status);
	}
	return 0;
}

IconItemT iconList[] =
{
	{ LPGENT("Disable Auto&reply"), "Disable Auto&reply", IDI_OFF },
	{ LPGENT("Enable Auto&reply"), "Enable Auto&reply", IDI_ON }
};

extern "C" int __declspec(dllexport)Load(void)
{
	mir_getLP(&pluginInfoEx);

	CreateServiceFunction(protocolname"/ToggleEnable", ToggleEnable);
	CreateServiceFunction(protocolname"/ToggleAutoanswer",Toggle);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 500090000;
	mi.ptszName = _T("");
	mi.pszService = protocolname"/ToggleEnable";
	hEnableMenu = Menu_AddMainMenuItem(&mi);

	mi.position = -0x7FFFFFFF;
	mi.ptszName = _T("");
	mi.pszService = protocolname"/ToggleAutoanswer";
	hToggle = Menu_AddContactMenuItem(&mi);

	//add hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, addEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, CheckDefaults);

	Icon_RegisterT(hinstance, _T("Simple Auto Replier"), iconList, SIZEOF(iconList));

	return 0;
}

extern "C" __declspec(dllexport)int Unload(void)
{
	return 0;
}