#include "stdafx.h"

int nProtocol = 0;
static HANDLE hPopupClass;

HGENMENU g_hContactMenu;
OBJLIST<CNudgeElement> arNudges(5);
CNudgeElement DefaultNudge;
CShake shake;
CNudge GlobalNudge;

CMPlugin g_plugin;

static IconItem iconList[] =
{
	{ LPGEN("Nudge as Default"), "Nudge_Default", IDI_NUDGE }
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
	// {E47CC215-0D28-462D-A0F6-3AE4443D2926}
	{ 0xe47cc215, 0xd28, 0x462d, { 0xa0, 0xf6, 0x3a, 0xe4, 0x44, 0x3d, 0x29, 0x26 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR NudgeShowMenu(WPARAM wParam, LPARAM lParam)
{
	bool bEnabled = false;
	for (auto &p : arNudges)
		if (!mir_strcmp((char*)wParam, p->ProtocolName)) {
			bEnabled = (GlobalNudge.useByProtocol) ? p->enabled : DefaultNudge.enabled;
			break;
		}

	Menu_ShowItem(g_hContactMenu, bEnabled && lParam != 0);
	return 0;
}

static INT_PTR NudgeSend(WPARAM hContact, LPARAM lParam)
{
	char *protoName = Proto_GetBaseAccountName(hContact);
	int diff = time(0) - db_get_dw(hContact, "Nudge", "LastSent", time(0) - 30);
	if (diff < GlobalNudge.sendTimeSec) {
		wchar_t msg[500];
		mir_snwprintf(msg, TranslateT("You are not allowed to send too much nudge (only 1 each %d sec, %d sec left)"), GlobalNudge.sendTimeSec, 30 - diff);
		if (GlobalNudge.useByProtocol) {
			for (auto &p : arNudges)
				if (!mir_strcmp(protoName, p->ProtocolName))
					Nudge_ShowPopup(p, hContact, msg);
		}
		else Nudge_ShowPopup(&DefaultNudge, hContact, msg);

		return 0;
	}

	db_set_dw(hContact, "Nudge", "LastSent", time(0));

	if (GlobalNudge.useByProtocol) {
		for (auto &p : arNudges)
			if (!mir_strcmp(protoName, p->ProtocolName))
				if (p->showStatus)
					Nudge_SentStatus(p, hContact);
	}
	else if (DefaultNudge.showStatus)
		Nudge_SentStatus(&DefaultNudge, hContact);

	CallProtoService(protoName, PS_SEND_NUDGE, hContact, lParam);
	return 0;
}

void OpenContactList()
{
	HWND hWnd = g_clistApi.hwndContactList;
	ShowWindow(hWnd, SW_RESTORE);
	ShowWindow(hWnd, SW_SHOW);
}

static int NudgeReceived(WPARAM hContact, LPARAM lParam)
{
	char *protoName = Proto_GetBaseAccountName(hContact);

	uint32_t currentTimestamp = time(0);
	uint32_t nudgeSentTimestamp = lParam ? (uint32_t)lParam : currentTimestamp;

	int diff = currentTimestamp - db_get_dw(hContact, "Nudge", "LastReceived", currentTimestamp - 30);
	int diff2 = nudgeSentTimestamp - db_get_dw(hContact, "Nudge", "LastReceived2", nudgeSentTimestamp - 30);

	if (diff >= GlobalNudge.recvTimeSec)
		db_set_dw(hContact, "Nudge", "LastReceived", currentTimestamp);
	if (diff2 >= GlobalNudge.recvTimeSec)
		db_set_dw(hContact, "Nudge", "LastReceived2", nudgeSentTimestamp);

	if (GlobalNudge.useByProtocol) {
		for (auto &p : arNudges) {
			if (!mir_strcmp(protoName, p->ProtocolName)) {

				if (p->enabled) {
					if (p->useIgnoreSettings && Ignore_IsIgnored(hContact, IGNOREEVENT_USERONLINE))
						return 0;

					int Status = Proto_GetStatus(protoName);

					if (((p->statusFlags & NUDGE_ACC_ST0) && (Status <= ID_STATUS_OFFLINE)) ||
						((p->statusFlags & NUDGE_ACC_ST1) && (Status == ID_STATUS_ONLINE)) ||
						((p->statusFlags & NUDGE_ACC_ST2) && (Status == ID_STATUS_AWAY)) ||
						((p->statusFlags & NUDGE_ACC_ST3) && (Status == ID_STATUS_DND)) ||
						((p->statusFlags & NUDGE_ACC_ST4) && (Status == ID_STATUS_NA)) ||
						((p->statusFlags & NUDGE_ACC_ST5) && (Status == ID_STATUS_OCCUPIED)) ||
						((p->statusFlags & NUDGE_ACC_ST6) && (Status == ID_STATUS_FREECHAT)) ||
						((p->statusFlags & NUDGE_ACC_ST7) && (Status == ID_STATUS_INVISIBLE)))
					{
						if (diff >= GlobalNudge.recvTimeSec) {
							if (p->showPopup)
								Nudge_ShowPopup(p, hContact, p->recText);
							if (p->openContactList)
								OpenContactList();
							if (p->shakeClist)
								ShakeClist(hContact, lParam);
							if (p->openMessageWindow)
								CallService(MS_MSG_SENDMESSAGEW, hContact, 0);
							if (p->shakeChat)
								ShakeChat(hContact, lParam);
							if (p->autoResend)
								mir_forkthread(AutoResendNudge, (void*)hContact);

							Skin_PlaySound(p->NudgeSoundname);
						}
					}

					if (diff2 >= GlobalNudge.recvTimeSec)
						if (p->showStatus)
							Nudge_ShowStatus(p, hContact, nudgeSentTimestamp);
				}
				break;
			}
		}
	}
	else {
		if (DefaultNudge.enabled) {
			if (DefaultNudge.useIgnoreSettings && Ignore_IsIgnored(hContact, IGNOREEVENT_USERONLINE))
				return 0;

			uint32_t Status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
			if (((DefaultNudge.statusFlags & NUDGE_ACC_ST0) && (Status <= ID_STATUS_OFFLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST1) && (Status == ID_STATUS_ONLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST2) && (Status == ID_STATUS_AWAY)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST3) && (Status == ID_STATUS_DND)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST4) && (Status == ID_STATUS_NA)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST5) && (Status == ID_STATUS_OCCUPIED)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST6) && (Status == ID_STATUS_FREECHAT)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST6) && (Status == ID_STATUS_INVISIBLE)))
			{
				if (diff >= GlobalNudge.recvTimeSec) {
					if (DefaultNudge.showPopup)
						Nudge_ShowPopup(&DefaultNudge, hContact, DefaultNudge.recText);
					if (DefaultNudge.openContactList)
						OpenContactList();
					if (DefaultNudge.shakeClist)
						ShakeClist(hContact, lParam);
					if (DefaultNudge.openMessageWindow)
						CallService(MS_MSG_SENDMESSAGEW, hContact, 0);
					if (DefaultNudge.shakeChat)
						ShakeChat(hContact, lParam);
					if (DefaultNudge.autoResend)
						mir_forkthread(AutoResendNudge, (void*)hContact);

					Skin_PlaySound(DefaultNudge.NudgeSoundname);
				}
			}

			if (diff2 >= GlobalNudge.recvTimeSec)
				if (DefaultNudge.showStatus)
					Nudge_ShowStatus(&DefaultNudge, hContact, nudgeSentTimestamp);
		}
	}
	return 0;
}

static LRESULT CALLBACK NudgePopupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		CallService(MS_MSG_SENDMESSAGEW, (WPARAM)PUGetContact(hWnd), 0);
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
	POPUPCLASS ppc = {};
	ppc.flags = PCF_UNICODE;
	ppc.pszName = "Nudge";
	ppc.pszDescription.w = TranslateT("Show Nudge");
	ppc.hIcon = IcoLib_GetIconByHandle(iconList[0].hIcolib);
	ppc.PluginWindowProc = NudgePopupProc;
	if (hPopupClass = Popup_RegisterClass(&ppc))
		HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
}

int Preview()
{
	MCONTACT hContact = db_find_first();
	if (GlobalNudge.useByProtocol) {
		for (auto &p : arNudges) {
			if (p->enabled) {
				Skin_PlaySound(p->NudgeSoundname);
				if (p->showPopup)
					Nudge_ShowPopup(p, hContact, p->recText);
				if (p->openContactList)
					OpenContactList();
				if (p->shakeClist)
					ShakeClist(0, 0);
				if (p->openMessageWindow)
					CallService(MS_MSG_SENDMESSAGEW, hContact, NULL);
				if (p->shakeChat)
					ShakeChat(hContact, (LPARAM)time(0));
			}
		}
	}
	else {
		if (DefaultNudge.enabled) {
			Skin_PlaySound(DefaultNudge.NudgeSoundname);
			if (DefaultNudge.showPopup)
				Nudge_ShowPopup(&DefaultNudge, hContact, DefaultNudge.recText);
			if (DefaultNudge.openContactList)
				OpenContactList();
			if (DefaultNudge.shakeClist)
				ShakeClist(0, 0);
			if (DefaultNudge.openMessageWindow)
				CallService(MS_MSG_SENDMESSAGEW, hContact, NULL);
			if (DefaultNudge.shakeChat)
				ShakeChat(hContact, (LPARAM)time(0));
		}
	}
	return 0;
}

void Nudge_ShowPopup(CNudgeElement*, MCONTACT hContact, wchar_t * Message)
{
	hContact = db_mc_tryMeta(hContact);
	wchar_t *lpzContactName = Clist_GetContactDisplayName(hContact);

	POPUPDATACLASS NudgePopup = {};
	NudgePopup.hContact = hContact;
	NudgePopup.szText.w = Message;
	NudgePopup.szTitle.w = lpzContactName;
	NudgePopup.pszClassName = "nudge";
	Popup_AddClass(&NudgePopup);
}

void Nudge_SentStatus(CNudgeElement *n, MCONTACT hContact)
{
	T2Utf buff(n->senText);

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.flags = DBEF_SENT | DBEF_UTF;
	dbei.timestamp = (uint32_t)time(0);
	dbei.eventType = 1;
	dbei.cbBlob = (uint32_t)mir_strlen(buff) + 1;
	dbei.pBlob = (uint8_t*)buff;
	db_event_add(hContact, &dbei);
}

void Nudge_ShowStatus(CNudgeElement *n, MCONTACT hContact, uint32_t timestamp)
{
	T2Utf buff(n->recText);

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.eventType = 1;
	dbei.flags = DBEF_UTF;
	dbei.timestamp = timestamp;
	dbei.cbBlob = (uint32_t)mir_strlen(buff) + 1;
	dbei.pBlob = (uint8_t*)buff;
	db_event_add(hContact, &dbei);
}

void Nudge_AddAccount(PROTOACCOUNT *proto)
{
	char str[MAXMODULELABELLENGTH + 10];
	mir_snprintf(str, "%s/Nudge", proto->szModuleName);
	HANDLE hevent = HookEvent(str, NudgeReceived);
	if (hevent == nullptr)
		return;

	nProtocol++;

	// Add a specific sound per protocol
	CNudgeElement *p = new CNudgeElement();
	mir_snprintf(p->NudgeSoundname, "%s: Nudge", proto->szModuleName);

	strcpy_s(p->ProtocolName, proto->szModuleName);
	wcscpy_s(p->AccountName, proto->tszAccountName);

	p->Load();
	p->hEvent = hevent;

	wchar_t soundDesc[MAXMODULELABELLENGTH + 10];
	mir_snwprintf(soundDesc, TranslateT("Nudge for %s"), proto->tszAccountName);
	g_plugin.addSound(p->NudgeSoundname, LPGENW("Nudge"), soundDesc);

	arNudges.insert(p);
}

void AutoResendNudge(void *wParam)
{
	Sleep(GlobalNudge.resendDelaySec * 1000);
	NudgeSend((WPARAM)wParam, NULL);
}

static void LoadProtocols(void)
{
	// Load the default nudge
	mir_snprintf(DefaultNudge.ProtocolName, "Default");
	mir_snprintf(DefaultNudge.NudgeSoundname, "Nudge : Default");
	g_plugin.addSound(DefaultNudge.NudgeSoundname, LPGENW("Nudge"), LPGENW("Default Nudge"));
	
	DefaultNudge.Load();
	GlobalNudge.Load();

	for (auto &pa : Accounts())
		Nudge_AddAccount(pa);

	shake.Load();
}

// Load icons
void LoadIcons(void)
{
	g_plugin.registerIcon(LPGEN("Nudge"), iconList);
}

// Nudge support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	if (!mir_strcmp(cbcd->pszModule, "Nudge"))
		NudgeSend(wParam, 0);

	return 0;
}

static int TabsrmmButtonInit(WPARAM, LPARAM)
{
	HOTKEYDESC hkd = { "srmm_nudge", LPGEN("Send nudge"), BB_HK_SECTION, nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'N'), 0, LPARAM(g_plugin.getInst()) };
	g_plugin.addHotkey(&hkd);

	BBButton bbd = {};
	bbd.pszModuleName = "Nudge";
	bbd.pwszTooltip = LPGENW("Send nudge");
	bbd.dwDefPos = 300;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_CANBEHIDDEN;
	bbd.hIcon = iconList[0].hIcolib;
	bbd.dwButtonID = 6000;
	bbd.pszHotkey = hkd.pszName;
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

void HideNudgeButton(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (!ProtoServiceExists(szProto, PS_SEND_NUDGE)) {
		BBButton bbd = {};
		bbd.pszModuleName = "Nudge";
		bbd.dwButtonID = 6000;
		bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		Srmm_SetButtonState(hContact, &bbd);
	}
}

static int ContactWindowOpen(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;
	if (MWeventdata->uType == MSG_WINDOW_EVT_OPENING && MWeventdata->hContact)
		HideNudgeButton(MWeventdata->hContact);

	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto != nullptr) {
		bool isChat = db_get_b(hContact, szProto, "ChatRoom", false) != 0;
		NudgeShowMenu((WPARAM)szProto, !isChat);
	}

	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	LoadProtocols();
	LoadPopupClass();

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonInit);
	HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
	HookEvent(ME_MSG_WINDOWEVENT, ContactWindowOpen);
	return 0;
}

static int AccListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *proto = (PROTOACCOUNT*)wParam;
	if (proto == nullptr)
		return 0;

	if (lParam == PRAC_ADDED)
		Nudge_AddAccount(proto);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	LoadIcons();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged);
	HookEvent(ME_OPT_INITIALISE, NudgeOptInit);

	// Create function for plugins
	CreateServiceFunction(MS_SHAKE_CLIST, ShakeClist);
	CreateServiceFunction(MS_SHAKE_CHAT, ShakeChat);
	CreateServiceFunction(MS_NUDGE_SEND, NudgeSend);
	CreateServiceFunction(MS_NUDGE_SHOWMENU, NudgeShowMenu);

	// Add contact menu entry
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xd617db26, 0x22ba, 0x4205, 0x9c, 0x3e, 0x53, 0x10, 0xbc, 0xcf, 0xce, 0x19);
	mi.flags = CMIF_NOTOFFLINE | CMIF_UNICODE;
	mi.position = -500050004;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = LPGENW("Send &nudge");
	mi.pszService = MS_NUDGE_SEND;
	g_hContactMenu = Menu_AddContactMenuItem(&mi);

	// register special type of event
	// there's no need to declare the special service for getting text
	// because a blob contains only text
	DBEVENTTYPEDESCR evtype = {};
	evtype.module = MODULENAME;
	evtype.eventType = 1;
	evtype.descr = LPGEN("Nudge");
	evtype.eventIcon = iconList[0].hIcolib;
	evtype.flags = DETF_HISTORY | DETF_MSGWINDOW;
	DbEvent_RegisterType(&evtype);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	arNudges.destroy();
	return 0;
}
