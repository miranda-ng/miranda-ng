#include "headers.h"

int nProtocol = 0;
static HANDLE hPopupClass;
HINSTANCE hInst;

NudgeElementList *NudgeList = NULL;
CNudgeElement DefaultNudge;
CShake shake;
CNudge GlobalNudge;

int hLangpack = 0;

//========================
//  MirandaPluginInfo
//========================
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E47CC215-0D28-462D-A0F6-3AE4443D2926}
	{ 0xe47cc215, 0xd28, 0x462d, { 0xa0, 0xf6, 0x3a, 0xe4, 0x44, 0x3d, 0x29, 0x26 } }
};

INT_PTR NudgeShowMenu(WPARAM wParam, LPARAM lParam)
{
	for (NudgeElementList *n = NudgeList; n != NULL; n = n->next) {
		if (!strcmp((char *)wParam, n->item.ProtocolName)) {
			bool bEnabled = GlobalNudge.useByProtocol ? n->item.enabled : DefaultNudge.enabled;
			Menu_ShowItem(n->item.hContactMenu, bEnabled && lParam != 0);
			break;
		}
	}

	return 0;
}

INT_PTR NudgeSend(WPARAM hContact, LPARAM lParam)
{
	char *protoName = GetContactProto(hContact);
	int diff = time(NULL) - db_get_dw(hContact, "Nudge", "LastSent", time(NULL) - 30);
	if (diff < GlobalNudge.sendTimeSec) {
		TCHAR msg[500];
		mir_sntprintf(msg, 500, TranslateT("You are not allowed to send too much nudge (only 1 each %d sec, %d sec left)"), GlobalNudge.sendTimeSec, 30 - diff);
		if (GlobalNudge.useByProtocol) {
			for (NudgeElementList *n = NudgeList; n != NULL; n = n->next)
				if (!strcmp(protoName, n->item.ProtocolName))
					Nudge_ShowPopup(n->item, hContact, msg);
		}
		else Nudge_ShowPopup(DefaultNudge, hContact, msg);

		return 0;
	}

	db_set_dw(hContact, "Nudge", "LastSent", time(NULL));

	if (GlobalNudge.useByProtocol) {
		for (NudgeElementList *n = NudgeList; n != NULL; n = n->next)
			if (!strcmp(protoName, n->item.ProtocolName))
				if (n->item.showStatus)
					Nudge_SentStatus(n->item, hContact);
	}
	else if (DefaultNudge.showStatus)
		Nudge_SentStatus(DefaultNudge, hContact);

	CallProtoService(protoName, PS_SEND_NUDGE, hContact, lParam);
	return 0;
}

void OpenContactList()
{
	HWND hWnd = (HWND) CallService(MS_CLUI_GETHWND,0,0);
	ShowWindow(hWnd, SW_RESTORE);
	ShowWindow(hWnd, SW_SHOW);
}

int NudgeReceived(WPARAM hContact, LPARAM lParam)
{
	char *protoName = GetContactProto(hContact);

	DWORD currentTimestamp = time(NULL);
	DWORD nudgeSentTimestamp = lParam ? (DWORD)lParam : currentTimestamp;

	int diff = currentTimestamp - db_get_dw(hContact, "Nudge", "LastReceived", currentTimestamp - 30);
	int diff2 = nudgeSentTimestamp - db_get_dw(hContact, "Nudge", "LastReceived2", nudgeSentTimestamp - 30);

	if (diff >= GlobalNudge.recvTimeSec)
		db_set_dw(hContact, "Nudge", "LastReceived", currentTimestamp);
	if (diff2 >= GlobalNudge.recvTimeSec)
		db_set_dw(hContact, "Nudge", "LastReceived2", nudgeSentTimestamp);

	if (GlobalNudge.useByProtocol) {
		for (NudgeElementList *n = NudgeList; n != NULL; n = n->next) {
			if (!strcmp(protoName, n->item.ProtocolName)) {

				if (n->item.enabled) {
					if (n->item.useIgnoreSettings && CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_USERONLINE))
						return 0;

					DWORD Status = CallProtoService(protoName, PS_GETSTATUS, 0, 0);

					if (((n->item.statusFlags & NUDGE_ACC_ST0) && (Status <= ID_STATUS_OFFLINE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST1) && (Status == ID_STATUS_ONLINE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST2) && (Status == ID_STATUS_AWAY)) ||
						((n->item.statusFlags & NUDGE_ACC_ST3) && (Status == ID_STATUS_DND)) ||
						((n->item.statusFlags & NUDGE_ACC_ST4) && (Status == ID_STATUS_NA)) ||
						((n->item.statusFlags & NUDGE_ACC_ST5) && (Status == ID_STATUS_OCCUPIED)) ||
						((n->item.statusFlags & NUDGE_ACC_ST6) && (Status == ID_STATUS_FREECHAT)) ||
						((n->item.statusFlags & NUDGE_ACC_ST7) && (Status == ID_STATUS_INVISIBLE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST8) && (Status == ID_STATUS_ONTHEPHONE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST9) && (Status == ID_STATUS_OUTTOLUNCH)))
					{
						if (diff >= GlobalNudge.recvTimeSec) {
							if (n->item.showPopup)
								Nudge_ShowPopup(n->item, hContact, n->item.recText);
							if (n->item.openContactList)
								OpenContactList();
							if (n->item.shakeClist)
								ShakeClist(hContact, lParam);
							if (n->item.openMessageWindow)
								CallService(MS_MSG_SENDMESSAGET, hContact, 0);
							if (n->item.shakeChat)
								ShakeChat(hContact, lParam);
							if (n->item.autoResend)
								mir_forkthread(AutoResendNudge, (void*)hContact);

							SkinPlaySound(n->item.NudgeSoundname);
						}
					}

					if (diff2 >= GlobalNudge.recvTimeSec)
						if (n->item.showStatus)
							Nudge_ShowStatus(n->item, hContact, nudgeSentTimestamp);
				}
				break;
			}
		}
	}
	else {
		if (DefaultNudge.enabled) {
			if (DefaultNudge.useIgnoreSettings && CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_USERONLINE))
				return 0;

			DWORD Status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
			if (((DefaultNudge.statusFlags & NUDGE_ACC_ST0) && (Status <= ID_STATUS_OFFLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST1) && (Status == ID_STATUS_ONLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST2) && (Status == ID_STATUS_AWAY)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST3) && (Status == ID_STATUS_DND)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST4) && (Status == ID_STATUS_NA)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST5) && (Status == ID_STATUS_OCCUPIED)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST6) && (Status == ID_STATUS_FREECHAT)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST7) && (Status == ID_STATUS_INVISIBLE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST8) && (Status == ID_STATUS_ONTHEPHONE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST9) && (Status == ID_STATUS_OUTTOLUNCH)))
			{
				if (diff >= GlobalNudge.recvTimeSec) {
					if (DefaultNudge.showPopup)
						Nudge_ShowPopup(DefaultNudge, hContact, DefaultNudge.recText);
					if (DefaultNudge.openContactList)
						OpenContactList();
					if (DefaultNudge.shakeClist)
						ShakeClist(hContact, lParam);
					if (DefaultNudge.openMessageWindow)
						CallService(MS_MSG_SENDMESSAGET, hContact, 0);
					if (DefaultNudge.shakeChat)
						ShakeChat(hContact, lParam);
					if (DefaultNudge.autoResend)
						mir_forkthread(AutoResendNudge, (void*)hContact);

					SkinPlaySound(DefaultNudge.NudgeSoundname);
				}
			}

			if (diff2 >= GlobalNudge.recvTimeSec)
				if (DefaultNudge.showStatus)
					Nudge_ShowStatus(DefaultNudge, hContact, nudgeSentTimestamp);
		}
	}
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

void LoadProtocols(void)
{
	//Load the default nudge
	mir_snprintf(DefaultNudge.ProtocolName, SIZEOF(DefaultNudge.ProtocolName), "Default");
	mir_snprintf(DefaultNudge.NudgeSoundname, SIZEOF(DefaultNudge.NudgeSoundname), "Nudge : Default");
	SkinAddNewSoundEx(DefaultNudge.NudgeSoundname, LPGEN("Nudge"), LPGEN("Default Nudge"));
	DefaultNudge.Load();

	GlobalNudge.Load();

	int numberOfProtocols = 0;
	PROTOACCOUNT **ppProtocolDescriptors;
	INT_PTR ret = ProtoEnumAccounts(&numberOfProtocols, &ppProtocolDescriptors);
	if (ret == 0)
	for (int i = 0; i < numberOfProtocols; i++)
		Nudge_AddAccount(ppProtocolDescriptors[i]);

	shake.Load();
}

static IconItem iconList[] =
{
	{ LPGEN("Nudge as Default"), "Nudge_Default", IDI_NUDGE }
};

void LoadIcons(void)
{
	//Load icons
	Icon_Register(hInst, LPGEN("Nudge"), iconList, SIZEOF(iconList));
}

// Nudge support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	if (!strcmp(cbcd->pszModule, "Nudge"))
		NudgeSend(wParam, 0);

	return 0;
}

static int TabsrmmButtonInit(WPARAM wParam, LPARAM lParam)
{
	BBButton bbd = { sizeof(bbd) };
	bbd.pszModuleName = "Nudge";
	bbd.ptszTooltip = LPGENT("Send Nudge");
	bbd.dwDefPos = 300;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_CANBEHIDDEN;
	bbd.hIcon = iconList[0].hIcolib;
	bbd.dwButtonID = 6000;
	bbd.iButtonWidth = 0;
	CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

	return 0;
}

void HideNudgeButton(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (!ProtoServiceExists(szProto, PS_SEND_NUDGE)) {
		BBButton bbd = { sizeof(bbd) };
		bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		bbd.pszModuleName = "Nudge";
		bbd.dwButtonID = 6000;
		CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);
	}
}

static int ContactWindowOpen(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;
	if (MWeventdata->uType == MSG_WINDOW_EVT_OPENING && MWeventdata->hContact)
		HideNudgeButton(MWeventdata->hContact);

	return 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	char *szProto = GetContactProto(wParam);
	if (szProto != NULL) {
		bool isChat = db_get_b(wParam, szProto, "ChatRoom", false) != 0;
		NudgeShowMenu((WPARAM)szProto, (LPARAM)!isChat);
	}

	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	LoadProtocols();
	LoadPopupClass();

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	if (HookEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonInit)) {
		HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
		HookEvent(ME_MSG_WINDOWEVENT, ContactWindowOpen);
	}
	return 0;
}

int AccListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *proto = (PROTOACCOUNT*)wParam;
	if (proto == NULL)
		return 0;

	if (lParam == PRAC_ADDED)
		Nudge_AddAccount(proto);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	LoadIcons();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged);
	HookEvent(ME_OPT_INITIALISE, NudgeOptInit);

	// Create function for plugins
	CreateServiceFunction(MS_SHAKE_CLIST, ShakeClist);
	CreateServiceFunction(MS_SHAKE_CHAT, ShakeChat);
	CreateServiceFunction(MS_NUDGE_SEND, NudgeSend);
	CreateServiceFunction(MS_NUDGE_SHOWMENU, NudgeShowMenu);

	// register special type of event
	// there's no need to declare the special service for getting text
	// because a blob contains only text
	DBEVENTTYPEDESCR evtype = { sizeof(evtype) };
	evtype.module = MODULENAME;
	evtype.eventType = 1;
	evtype.descr = LPGEN("Nudge");
	evtype.eventIcon = iconList[0].hIcolib;
	evtype.flags = DETF_HISTORY | DETF_MSGWINDOW;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&evtype);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	NudgeElementList *p = NudgeList;
	while (p != NULL) {
		if (p->item.hEvent) UnhookEvent(p->item.hEvent);
		NudgeElementList* p1 = p->next;
		delete p;
		p = p1;
	}
	return 0;
}

LRESULT CALLBACK NudgePopupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		CallService(MS_MSG_SENDMESSAGET, (WPARAM)PUGetContact(hWnd), 0);
		PUDeletePopup(hWnd);
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;
	case UM_FREEPLUGINDATA:
		//Here we'd free our own data, if we had it.
		return FALSE;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

static int OnShutdown(WPARAM, LPARAM)
{
	Popup_UnregisterClass(hPopupClass);
	return 0;
}

void LoadPopupClass()
{
	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;
	ppc.pszName = "Nudge";
	ppc.ptszDescription = LPGENT("Show Nudge");
	ppc.hIcon = Skin_GetIconByHandle(iconList[0].hIcolib);
	ppc.colorBack = NULL;
	ppc.colorText = NULL;
	ppc.iSeconds = 0;
	ppc.PluginWindowProc = NudgePopupProc;
	if (hPopupClass = Popup_RegisterClass(&ppc))
		HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
}

int Preview()
{
	MCONTACT hContact = db_find_first();
	if (GlobalNudge.useByProtocol) {
		for (NudgeElementList *n = NudgeList; n != NULL; n = n->next) {
			if (n->item.enabled) {
				SkinPlaySound(n->item.NudgeSoundname);
				if (n->item.showPopup)
					Nudge_ShowPopup(n->item, hContact, n->item.recText);
				if (n->item.openContactList)
					OpenContactList();
				if (n->item.shakeClist)
					ShakeClist(0, 0);
				if (n->item.openMessageWindow)
					CallService(MS_MSG_SENDMESSAGET, hContact, NULL);
				if (n->item.shakeChat)
					ShakeChat(hContact, (LPARAM)time(NULL));
			}
		}
	}
	else {
		if (DefaultNudge.enabled) {
			SkinPlaySound(DefaultNudge.NudgeSoundname);
			if (DefaultNudge.showPopup)
				Nudge_ShowPopup(DefaultNudge, hContact, DefaultNudge.recText);
			if (DefaultNudge.openContactList)
				OpenContactList();
			if (DefaultNudge.shakeClist)
				ShakeClist(0, 0);
			if (DefaultNudge.openMessageWindow)
				CallService(MS_MSG_SENDMESSAGET, hContact, NULL);
			if (DefaultNudge.shakeChat)
				ShakeChat(hContact, (LPARAM)time(NULL));
		}
	}
	return 0;
}

void Nudge_ShowPopup(CNudgeElement n, MCONTACT hContact, TCHAR * Message)
{
	hContact = db_mc_tryMeta(hContact);
	TCHAR * lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		POPUPDATACLASS NudgePopup = { 0 };
		NudgePopup.cbSize = sizeof(NudgePopup);
		NudgePopup.hContact = hContact;
		NudgePopup.ptszText = Message;
		NudgePopup.ptszTitle = lpzContactName;
		NudgePopup.pszClassName = "nudge";
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&NudgePopup);
	}
	else if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		POPUPDATAT NudgePopup = { 0 };
		NudgePopup.lchContact = hContact;
		NudgePopup.lchIcon = Skin_GetIconByHandle(n.hIcoLibItem);
		NudgePopup.colorBack = 0;
		NudgePopup.colorText = 0;
		NudgePopup.iSeconds = 0;
		NudgePopup.PluginWindowProc = NudgePopupProc;
		NudgePopup.PluginData = (void *)1;

		//mir_tstrcpy(NudgePopup.lpzText, Translate(Message));
		mir_tstrcpy(NudgePopup.lptzText, Message);

		mir_tstrcpy(NudgePopup.lptzContactName, lpzContactName);

		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&NudgePopup, 0);
	}
	else MessageBox(NULL, Message, lpzContactName, 0);
}

void Nudge_SentStatus(CNudgeElement n, MCONTACT hContact)
{
	char *buff = mir_utf8encodeT(n.senText);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULENAME;
	dbei.flags = DBEF_SENT | DBEF_UTF;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.eventType = 1;
	dbei.cbBlob = (DWORD)strlen(buff) + 1;
	dbei.pBlob = (PBYTE)buff;
	db_event_add(hContact, &dbei);
	mir_free(buff);
}

void Nudge_ShowStatus(CNudgeElement n, MCONTACT hContact, DWORD timestamp)
{
	char *buff = mir_utf8encodeT(n.recText);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULENAME;
	dbei.eventType = 1;
	dbei.flags = DBEF_UTF;
	dbei.timestamp = timestamp;
	dbei.cbBlob = (DWORD)strlen(buff) + 1;
	dbei.pBlob = (PBYTE)buff;
	db_event_add(hContact, &dbei);
	mir_free(buff);
}

void Nudge_AddAccount(PROTOACCOUNT *proto)
{
	char str[MAXMODULELABELLENGTH + 10];
	mir_snprintf(str, SIZEOF(str), "%s/Nudge", proto->szModuleName);
	HANDLE hevent = HookEvent(str, NudgeReceived);
	if (hevent == NULL)
		return;

	nProtocol++;

	//Add a specific sound per protocol
	NudgeElementList *newNudge = new NudgeElementList;
	//newNudge = (NudgeElementList*) malloc(sizeof(NudgeElementList));
	mir_snprintf(newNudge->item.NudgeSoundname, SIZEOF(newNudge->item.NudgeSoundname), "%s: Nudge", proto->szModuleName);

	strcpy(newNudge->item.ProtocolName, proto->szProtoName);
	_tcscpy(newNudge->item.AccountName, proto->tszAccountName);

	newNudge->item.Load();

	newNudge->item.hEvent = hevent;

	TCHAR soundDesc[MAXMODULELABELLENGTH + 10];
	mir_sntprintf(soundDesc, SIZEOF(soundDesc), LPGENT("Nudge for %s"), proto->tszAccountName);
	SkinAddNewSoundExT(newNudge->item.NudgeSoundname, LPGENT("Nudge"), soundDesc);

	newNudge->next = NudgeList;
	NudgeList = newNudge;

	char iconName[MAXMODULELABELLENGTH + 10];
	mir_snprintf(iconName, SIZEOF(iconName), "Nudge_%s", proto->szModuleName);

	TCHAR szFilename[MAX_PATH], iconDesc[MAXMODULELABELLENGTH + 10];
	GetModuleFileName(hInst, szFilename, MAX_PATH);
	mir_sntprintf(iconDesc, SIZEOF(iconDesc), TranslateT("Nudge for %s"), proto->tszAccountName);

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszSection = LPGENT("Nudge");
	sid.ptszDefaultFile = szFilename;
	sid.pszName = iconName;
	sid.ptszDescription = iconDesc;
	sid.iDefaultIndex = -IDI_NUDGE;
	newNudge->item.hIcoLibItem = Skin_AddIcon(&sid);

	//Add contact menu entry
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.popupPosition = 500085000;
	mi.pszContactOwner = proto->szModuleName;
	mi.pszPopupName = proto->szModuleName;
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.position = -500050004;
	mi.icolibItem = newNudge->item.hIcoLibItem;
	mi.ptszName = LPGENT("Send &Nudge");
	mi.pszService = MS_NUDGE_SEND;
	newNudge->item.hContactMenu = Menu_AddContactMenuItem(&mi);
}

void AutoResendNudge(void *wParam)
{
	Sleep(GlobalNudge.resendDelaySec * 1000);
	NudgeSend((WPARAM)wParam, NULL);
}
