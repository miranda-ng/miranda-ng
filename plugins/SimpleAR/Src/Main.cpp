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
PLUGINLINK *pluginLink;
HANDLE hPreBuildHook = NULL, hAddEventHook = NULL, hOptHook = NULL, hCheckDefHook = NULL, hOnPreShutdown = NULL, hToggleEnable = NULL, hToggleAutoanswer = NULL;
HANDLE hToggle = NULL, hEnableMenu = NULL;
CLISTMENUITEM mi;
BOOL fEnabled, gbVarsServiceExist = FALSE;
INT interval;
struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;
int hLangpack;

TCHAR* ptszDefaultMsg[]={
	_T("I am currently away. I will reply to you when I am back."),
	_T("I am currently very busy and can't spare any time to talk with you. Sorry..."),
	_T("I am not available right now."),
	_T("I am now doing something, I will talk to you later."),
	_T("I am on the phone right now. I will get back to you very soon."),
	_T("I am having meal right now. I will get back to you very soon.")
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
	0,
	// {46BF191F-8DFB-4656-88B2-4C20BE4CFA44}
	{0x46bf191f, 0x8dfb, 0x4656, { 0x88, 0xb2, 0x4c, 0x20, 0xbe, 0x4c, 0xfa, 0x44}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

static const MUUID interfaces[] = {{0x46bf191f, 0x8dfb, 0x4656, { 0x88, 0xb2, 0x4c, 0x20, 0xbe, 0x4c, 0xfa, 0x44}}, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hinstance=hinst;
	return TRUE;
}

INT_PTR ToggleEnable(WPARAM wParam, LPARAM lParam)
{
	mi.cbSize=sizeof(mi);
	mi.flags=CMIM_NAME|CMIM_ICON|CMIF_TCHAR;
	fEnabled=!fEnabled;
	if (fEnabled)
	{
		mi.ptszName = _T("Disable Auto&reply");
		mi.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ON));
	}
	else
	{
		mi.ptszName = _T("Enable Auto&reply");
		mi.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_OFF));
	}
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hEnableMenu,(LPARAM)&mi);
	return 0;
}

INT_PTR Toggle(WPARAM w, LPARAM l)
{
	HANDLE hContact = (HANDLE)w;
	BOOL on = 0;
	on = !DBGetContactSettingByte(hContact, protocolname, "TurnedOn", 0);
	DBWriteContactSettingByte(hContact, protocolname, "TurnedOn", on?1:0);
	on = on?0:1;
	mi.cbSize=sizeof(mi);
	mi.flags = CMIM_NAME |CMIM_ICON | CMIF_TCHAR;
	mi.ptszName=on?_T("Turn off Autoanswer"):_T("Turn on Autoanswer");
	mi.hIcon = on?LoadIcon(hinstance, MAKEINTRESOURCE(IDI_OFF)):LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ON));
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hToggle, (LPARAM)&mi);
	return 0;
}

INT OnPreBuildContactMenu(WPARAM w, LPARAM l)
{
	HANDLE hContact = (HANDLE)w;
	mi.cbSize=sizeof(mi);
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	BOOL  on = !DBGetContactSettingByte(hContact, protocolname, "TurnedOn", 0);
	mi.ptszName = on?_T("Turn off Autoanswer"):_T("Turn on Autoanswer");
	mi.hIcon = on?LoadIcon(hinstance, MAKEINTRESOURCE(IDI_OFF)):LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ON));
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hToggle, (LPARAM)&mi);
	return 0;
}

INT CheckDefaults(WPARAM, LPARAM)
{
	DBVARIANT dbv;
	TCHAR* ptszDefault;
	char szStatus[6]={0};

	UnhookEvent(hCheckDefHook);

	fEnabled=!DBGetContactSettingByte(NULL,protocolname,KEY_ENABLED,1)==1;
	interval=DBGetContactSettingWord(NULL,protocolname,KEY_REPEATINTERVAL,300);

	if (DBGetContactSettingTString(NULL,protocolname,KEY_HEADING,&dbv))
		// Heading not set
		DBWriteContactSettingTString(NULL,protocolname,KEY_HEADING,TranslateT("Dear %user%, the owner left the following message:"));
	else
		DBFreeVariant(&dbv);

	for (int c=ID_STATUS_ONLINE; c<ID_STATUS_IDLE; c++)
	{
		mir_snprintf(szStatus,SIZEOF(szStatus),"%d",c);
		if (c == 40072 || c == 40077 || c == 40078)
			continue;
		else
		{
			if (DBGetContactSettingTString(NULL,protocolname,szStatus,&dbv))
			{
				if (c < 40077)
					// This mode does not have a preset message
					ptszDefault=TranslateTS(ptszDefaultMsg[c-ID_STATUS_ONLINE-1]);
				else if(c > 40078)
					ptszDefault=TranslateTS(ptszDefaultMsg[c-ID_STATUS_ONLINE-3]);
				if (ptszDefault)
					DBWriteContactSettingTString(NULL,protocolname,szStatus,ptszDefault);
			}
			else
				DBFreeVariant(&dbv);
		}
	}
	hPreBuildHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;

	ToggleEnable(0,0);
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

INT addEvent(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	HANDLE hDBEvent = (HANDLE)lParam;
	DBEVENTINFO dbei = {sizeof(dbei)};

	if (!fEnabled || !hContact || !hDBEvent)
		return FALSE;	/// unspecifyed error 

	char* pszProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	int status = CallProtoService(pszProto, PS_GETSTATUS, 0, 0);
	if (status == 40072 || status == 40077 || status == 40078)
		return FALSE;

	CallService(MS_DB_EVENT_GET, (WPARAM)hDBEvent, (LPARAM)&dbei); /// detect size of msg

	if ((dbei.eventType != EVENTTYPE_MESSAGE) || (dbei.flags == DBEF_READ)) 
		return FALSE; /// we need EVENTTYPE_MESSAGE event..
	else
	{	/// needed event has occured..
		DBVARIANT dbv;

		if (!dbei.cbBlob)	/// invalid size
			return FALSE;

		if (DBGetContactSettingTString(hContact,"Protocol","p",&dbv))
			// Contact with no protocol ?!!
			return FALSE;
		else 
			DBFreeVariant(&dbv);
		
		if(DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
			return FALSE;
		
		if(DBGetContactSettingByte(hContact, protocolname, "TurnedOn", 0))
			return FALSE;
		
		if (!( dbei.flags & DBEF_SENT))
		{
			int timeBetween=time(NULL)-DBGetContactSettingDword(hContact,protocolname,"LastReplyTS",0);
			if (timeBetween>interval || DBGetContactSettingWord(hContact,protocolname,"LastStatus",0)!=status)
			{
				char szStatus[6]={0};
				int msgLen=1;
				int isQun=DBGetContactSettingByte(hContact,pszProto,"IsQun",0);
				if (isQun)
					return FALSE;

				mir_snprintf(szStatus,SIZEOF(szStatus),"%d",status);
				if (!DBGetContactSettingTString(NULL,protocolname,szStatus,&dbv))
				{
					if (*dbv.ptszVal)
					{
						DBVARIANT dbvHead={0}, dbvNick={0};
						TCHAR *ptszTemp, *ptszTemp2;

						DBGetContactSettingTString(hContact,pszProto,"Nick",&dbvNick);
						if (lstrcmp(dbvNick.ptszVal, NULL) == 0)
						{
							DBFreeVariant(&dbvNick);
							return FALSE;
						}

						msgLen += (int)_tcslen(dbv.ptszVal);
						if (!DBGetContactSettingTString(NULL,protocolname,KEY_HEADING,&dbvHead))
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
						dbei.cbBlob = lstrlenA(pszUtf) + 1;
						dbei.pBlob = (PBYTE)pszUtf;
						CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
						
						if (ServiceExists(MS_VARS_FORMATSTRING))
							CallService(MS_VARS_FREEMEMORY, (WPARAM)ptszTemp, 0);
						else
							mir_free(ptszTemp);
						mir_free(ptszTemp2);
						mir_free(pszUtf);
						if (dbvNick.ptszVal)
							DBFreeVariant(&dbvNick);
						if (dbvHead.ptszVal)
							DBFreeVariant(&dbvHead);
					}
					DBFreeVariant(&dbv);
				}
			}
		}

		DBWriteContactSettingDword(hContact,protocolname,"LastReplyTS",time(NULL));
		DBWriteContactSettingWord(hContact,protocolname,"LastStatus",status);
	}
	return 0;
}

INT OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(hAddEventHook);
	UnhookEvent(hPreBuildHook);
	UnhookEvent(hOptHook);
	UnhookEvent(hOnPreShutdown);
	return 0;
}

extern "C" int __declspec(dllexport)Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLP(&pluginInfoEx);

	hToggleEnable = CreateServiceFunction(protocolname"/ToggleEnable", ToggleEnable);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 500090000;
	mi.ptszName = _T("");
	mi.pszService = protocolname"/ToggleEnable";
	hEnableMenu = Menu_AddMainMenuItem(&mi);

	hToggleAutoanswer = CreateServiceFunction(protocolname"/ToggleAutoanswer",Toggle);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.ptszName=_T("");
	mi.pszService=protocolname"/ToggleAutoanswer";
	hToggle = Menu_AddContactMenuItem(&mi);

	//add hook
	hOptHook = HookEvent(ME_OPT_INITIALISE, OptInit);
	hAddEventHook = HookEvent(ME_DB_EVENT_ADDED, addEvent);
	hCheckDefHook = HookEvent(ME_SYSTEM_MODULESLOADED, CheckDefaults);
	hOnPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);

	return 0;
}

extern "C" __declspec(dllexport)int Unload(void)
{
	DestroyServiceFunction(hToggleEnable);
	DestroyServiceFunction(hToggleAutoanswer);
	return 0;
}