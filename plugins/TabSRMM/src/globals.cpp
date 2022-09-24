/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// Plugin configuration variables and functions. Implemented as a class
// though there will always be only a single instance.

#include "stdafx.h"

void Chat_ModulesLoaded();
void CB_InitCustomButtons();

CGlobals PluginConfig;

static TContainerSettings _cnt_default = { 0, 0, 255, CInfoPanel::DEGRADE_THRESHOLD, 60, 60, L"%n (%s)", 1, 0 };

wchar_t* CGlobals::m_default_container_name = L"default";

extern HANDLE 	hHookButtonPressedEvt;

EXCEPTION_RECORD CGlobals::m_exRecord = { 0 };
CONTEXT          CGlobals::m_exCtx = { 0 };
LRESULT          CGlobals::m_exLastResult = 0;
char             CGlobals::m_exSzFile[MAX_PATH] = "\0";
wchar_t          CGlobals::m_exReason[256] = L"\0";
int              CGlobals::m_exLine = 0;
bool             CGlobals::m_exAllowContinue = false;

/////////////////////////////////////////////////////////////////////////////////////////
// reload system values. These are read ONCE and are not allowed to change
// without a restart

void CGlobals::reloadSystemStartup()
{
	::RegisterTabCtrlClass();
	CTip::registerClass();

	dwThreadID = GetCurrentThreadId();

	PluginConfig.g_hMenuContext = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_TABCONTEXT));
	TranslateMenu(g_hMenuContext);

	g_plugin.addSound("RecvMsgActive",   LPGENW("Instant messages"), LPGENW("Incoming (focused window)"));
	g_plugin.addSound("RecvMsgInactive", LPGENW("Instant messages"), LPGENW("Incoming (unfocused window)"));
	g_plugin.addSound("AlertMsg",        LPGENW("Instant messages"), LPGENW("Incoming (new session)"));
	g_plugin.addSound("SendMsg",         LPGENW("Instant messages"), LPGENW("Outgoing"));
	g_plugin.addSound("SendError",       LPGENW("Instant messages"), LPGENW("Message send error"));

	hCurSplitNS = LoadCursor(nullptr, IDC_SIZENS);
	hCurSplitWE = LoadCursor(nullptr, IDC_SIZEWE);
	hCurSplitSW = LoadCursor(nullptr, IDC_SIZENESW);
	hCurSplitWSE = LoadCursor(nullptr, IDC_SIZENWSE);

	HDC hScrnDC = GetDC(nullptr);
	m_DPIscaleX = GetDeviceCaps(hScrnDC, LOGPIXELSX) / 96.0;
	m_DPIscaleY = GetDeviceCaps(hScrnDC, LOGPIXELSY) / 96.0;
	ReleaseDC(nullptr, hScrnDC);

	reloadSettings(false);
	reloadAdv();
	hookSystemEvents();
}

/////////////////////////////////////////////////////////////////////////////////////////
// this runs ONCE at startup when the Modules Loaded event is fired
// by the core. all plugins are loaded and ready to use.
//
// any initialation for 3rd party plugins must go here.

void CGlobals::reloadSystemModulesChanged()
{
	// smiley add
	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS)) {
		PluginConfig.g_SmileyAddAvail = 1;
		HookEvent(ME_SMILEYADD_OPTIONSCHANGED, ::SmileyAddOptionsChanged);
	}

	m_hwndClist = g_clistApi.hwndContactList;

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x58d8dc1, 0x1c25, 0x49c0, 0xb8, 0x7c, 0xa3, 0x22, 0x2b, 0x3d, 0xf1, 0xd8);
	mi.position = -2000090000;
	mi.flags = CMIF_DEFAULT;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_MESSAGE);
	mi.name.a = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	PluginConfig.m_hMenuItem = Menu_AddContactMenuItem(&mi);

	m_useAeroPeek = M.GetByte("useAeroPeek", 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// reload plugin settings on startup and runtime.Most of these setttings can be
// changed while plugin is running.

void CGlobals::reloadSettings(bool fReloadSkins)
{
	m_ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &m_ncm, 0);

	m_iTabNameLimit = db_get_w(0, SRMSGMOD_T, "cut_at", 15);
	m_bCutContactNameOnTabs = M.GetBool("cuttitle", false);
	m_MsgTimeout = g_plugin.getDword(SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);

	if (m_MsgTimeout < SRMSGSET_MSGTIMEOUT_MIN)
		m_MsgTimeout = SRMSGSET_MSGTIMEOUT_MIN;

	m_EscapeCloses = M.GetByte("escmode", 0);

	m_bAlwaysFullToolbarWidth = M.GetBool("alwaysfulltoolbar", true);
	m_LimitStaticAvatarHeight = M.GetDword("avatarheight", 96);
	m_panelHeight = (uint32_t)M.GetDword("panelheight", CInfoPanel::DEGRADE_THRESHOLD);
	m_MUCpanelHeight = db_get_dw(0, CHAT_MODULE, "panelheight", CInfoPanel::DEGRADE_THRESHOLD);
	m_bIdleDetect = M.GetBool("dimIconsForIdleContacts", true);
	m_smcxicon = m_smcyicon = 16;
	m_LangPackCP = Langpack_GetDefaultCodePage();
	m_visualMessageSizeIndicator = M.GetByte("msgsizebar", 0);
	m_FlashOnMTN = g_plugin.getByte(SRMSGSET_SHOWTYPINGWINFLASH, SRMSGDEFSET_SHOWTYPINGWINFLASH);
	if (m_MenuBar == nullptr) {
		m_MenuBar = ::LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENUBAR));
		TranslateMenu(m_MenuBar);
	}

	m_ipBackgroundGradient = db_get_dw(0, FONTMODULE, "ipfieldsbg", 0x62caff);
	if (0 == m_ipBackgroundGradient)
		m_ipBackgroundGradient = 0x62caff;

	m_ipBackgroundGradientHigh = db_get_dw(0, FONTMODULE, "ipfieldsbgHigh", 0xf0f0f0);
	if (0 == m_ipBackgroundGradientHigh)
		m_ipBackgroundGradientHigh = 0xf0f0f0;

	m_tbBackgroundHigh = db_get_dw(0, FONTMODULE, "tbBgHigh", 0);
	m_tbBackgroundLow = db_get_dw(0, FONTMODULE, "tbBgLow", 0);
	m_fillColor = db_get_dw(0, FONTMODULE, "fillColor", 0);
	if (CSkin::m_BrushFill) {
		::DeleteObject(CSkin::m_BrushFill);
		CSkin::m_BrushFill = nullptr;
	}
	m_genericTxtColor = db_get_dw(0, FONTMODULE, "genericTxtClr", GetSysColor(COLOR_BTNTEXT));
	m_cRichBorders = db_get_dw(0, FONTMODULE, "cRichBorders", 0);

	TContainerFlags f; f.dw = 0;
	f.m_bDontReport = f.m_bDontReportUnfocused = f.m_bAlwaysReportInactive = f.m_bHideTabs = f.m_bNewContainerFlags = f.m_bNoMenuBar = f.m_bInfoPanel = true;

	::memcpy(&globalContainerSettings, &_cnt_default, sizeof(TContainerSettings));
	globalContainerSettings.flags = f;
	Utils::ReadContainerSettingsFromDB(0, &globalContainerSettings);
	globalContainerSettings.fPrivate = false;
	if (fReloadSkins)
		Skin->setupAeroSkins();
}

/////////////////////////////////////////////////////////////////////////////////////////
// reload "advanced tweaks" that can be applied w / o a restart

void CGlobals::reloadAdv()
{
	if (m_TypingSoundAdded == false) {
		g_plugin.addSound("SoundOnTyping", LPGENW("Other"), LPGENW("TabSRMM: typing"));
		m_TypingSoundAdded = true;
	}
}

const HMENU CGlobals::getMenuBar()
{
	if (m_MenuBar == nullptr) {
		m_MenuBar = ::LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENUBAR));
		TranslateMenu(m_MenuBar);
	}
	return(m_MenuBar);
}

/////////////////////////////////////////////////////////////////////////////////////////
// hook core events.This runs in LoadModule()
// only core events and services are guaranteed to exist at this time

void CGlobals::hookSystemEvents()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED, ::IconsChanged);
	HookEvent(ME_SKIN_ICONSCHANGED, ::IcoLibIconsChanged);
	HookEvent(ME_PROTO_CONTACTISTYPING, CMimAPI::TypingMessage);
	HookEvent(ME_PROTO_ACK, CMimAPI::ProtoAck);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreshutdownSendRecv);
	HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CMimAPI::PrebuildContactMenu);
	
	HookEvent(ME_AV_AVATARCHANGED, ::AvatarChanged);
	HookEvent(ME_AV_MYAVATARCHANGED, ::MyAvatarChanged);
}

/////////////////////////////////////////////////////////////////////////////////////////
// second part of the startup initialisation.All plugins are now fully loaded

int CGlobals::ModulesLoaded(WPARAM, LPARAM)
{
	M.configureCustomFolders();

	Skin->Init(true);
	CSkin::initAeroEffect();

	for (int i = 0; i < NR_BUTTONBARICONS; i++)
		PluginConfig.g_buttonBarIcons[i] = nullptr;
	::LoadIconTheme();
	::CreateImageList(true);
	::CB_InitCustomButtons();

	PluginConfig.reloadSystemModulesChanged();

	::Chat_ModulesLoaded();
	::BuildContainerMenu();

	::ModPlus_Init();

	if (M.GetByte("avatarmode", -1) == -1)
		db_set_b(0, SRMSGMOD_T, "avatarmode", 2);

	PluginConfig.g_hwndHotkeyHandler = CreateWindowEx(0, L"TSHK", L"", WS_POPUP, 0, 0, 40, 40, nullptr, nullptr, g_plugin.getInst(), nullptr);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x9f68b822, 0xff97, 0x477d, 0xb7, 0x6d, 0xa5, 0x59, 0x33, 0x1c, 0x54, 0x40);
	mi.position = -500050005;
	mi.hIcolibItem = PluginConfig.g_iconContainer;
	mi.name.a = LPGEN("&Messaging settings...");
	mi.pszService = MS_TABMSG_SETUSERPREFS;
	PluginConfig.m_UserMenuItem = Menu_AddContactMenuItem(&mi);

	if (SendLater::Avail) {
		SET_UID(mi, 0x8f32b04e, 0x314e, 0x42eb, 0x89, 0xc6, 0x56, 0x21, 0xf5, 0x1a, 0x2f, 0x22);
		mi.position = -500050006;
		mi.hIcolibItem = nullptr;
		mi.name.a = LPGEN("&Send later job list...");
		mi.pszService = MS_TABMSG_SLQMGR;
		PluginConfig.m_UserMenuItem = Menu_AddMainMenuItem(&mi);
	}
	RestoreUnreadMessageAlerts();

	::RegisterFontServiceFonts();
	::CacheLogFonts();
	TN_ModuleInit();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, DBContactDeleted);

	HookEvent(ME_DB_EVENT_ADDED, CMimAPI::DispatchNewEvent);
	HookEvent(ME_DB_EVENT_ADDED, CMimAPI::MessageEventAdded);

	HookEvent(ME_DB_EVENT_EDITED, CMimAPI::DispatchNewEvent);
	HookEvent(ME_DB_EVENT_EDITED, CMimAPI::MessageEventAdded);

	HookEvent(ME_FONT_RELOAD, ::FontServiceFontsChanged);

	HookEvent(ME_MC_ENABLED, &CContactCache::cacheUpdateMetaChanged);
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaContactEvent);
	HookEvent(ME_MC_SUBCONTACTSCHANGED, MetaContactEvent);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// watches various important database settings and reacts accordingly
// needed to catch status, nickname and other changes in order to update open message
// sessions.

int CGlobals::DBSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	const char *setting = cws->szSetting;

	if (hContact == 0) {
		if (!strcmp("Nick", setting))
			Srmm_Broadcast(DM_OWNNICKCHANGED, 0, (LPARAM)cws->szModule);
		return 0;
	}

	CContactCache *c = CContactCache::getContactCache(hContact);
	const char *szProto = c->getProto();
	if (szProto == nullptr)
		return 0;

	if (!c->isValid())
		c->resetMeta(); // restart constructor

	// neither clist nor contact's settings -> skip
	if (strcmp(cws->szModule, "CList") && strcmp(cws->szModule, szProto))
		return 0;

	if (!strcmp(cws->szModule, META_PROTO))
		if (!strcmp(setting, "Nick"))      // filter out this setting to avoid infinite loops while trying to obtain the most online contact
			return 0;

	HWND hwnd = Srmm_FindWindow(hContact);
	bool fChanged = false, fExtendedStatusChange = false;
	if (!strcmp(cws->szSetting, "Status"))
		fChanged = c->updateStatus(cws->value.wVal);

	fChanged |= c->updateNick();

	if (strlen(setting) > 6 && strlen(setting) < 9 && !strncmp(setting, "Status", 6)) {
		fChanged = true;
		c->updateMeta();
		c->updateUIN();
	}
	if (strlen(setting) > 6 && strstr("StatusMsg,XStatusMsg,XStatusName,XStatusId,ListeningTo", setting)) {
		c->updateStatusMsg(setting);
		fExtendedStatusChange = true;
	}
	if (!strcmp(setting, "display_uid")) {
		c->updateUIN();
		if (hwnd)
			PostMessage(hwnd, DM_UPDATEUIN, 0, 0);
	}

	if (hwnd != nullptr) {
		CMsgDialog *dat = c->getDat();
		if (!strcmp(setting, "MirVer"))
			PostMessage(hwnd, DM_CLIENTCHANGED, 0, 0);

		if (dat && !strcmp(setting, "NotOnList") && (cws->value.type == DBVT_DELETED || cws->value.bVal == 0))
			((CMsgDialog*)dat)->onClick_CancelAdd(0);
		
		if (dat && (fChanged || fExtendedStatusChange))
			dat->UpdateTitle();

		if (fExtendedStatusChange)
			PostMessage(hwnd, DM_UPDATESTATUSMSG, 0, 0);

		if (fChanged) {
			if (dat && c->getStatus() == ID_STATUS_OFFLINE) // clear typing notification in the status bar when contact goes offline
				dat->ClearTyping();

			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_LOGSTATUSCHANGE, MAKELONG(c->getStatus(), c->getOldStatus()), (LPARAM)c);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// event fired when a contact has been deleted. Make sure to close its message session

int CGlobals::DBContactDeleted(WPARAM hContact, LPARAM)
{
	if (hContact) {
		CContactCache *c = CContactCache::getContactCache(hContact);
		c->deletedHandler();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Handle events from metacontacts protocol.Basically, just update
// our contact cache and, if a message window exists, tell it to update
// relevant information.

int CGlobals::MetaContactEvent(WPARAM hContact, LPARAM)
{
	if (hContact) {
		CContactCache *c = CContactCache::getContactCache(hContact);
		c->updateMeta();
		CMsgDialog *pDlg = c->getDat();
		if (pDlg) {
			pDlg->UpdateTitle();
			pDlg->GetClientIcon();
			::PostMessage(pDlg->GetHwnd(), DM_UPDATEPICLAYOUT, 0, 0);
			InvalidateRect(pDlg->GetHwnd(), nullptr, TRUE); // force redraw
		}
	}
	return 0;
}

int CGlobals::PreshutdownSendRecv(WPARAM, LPARAM)
{
	::TN_ModuleDeInit();
	::CloseAllContainers();

	for (auto &hContact : Contacts())
		db_set_dw(hContact, SRMSGMOD_T, "messagecount", 0);

	::DestroyWindow(PluginConfig.g_hwndHotkeyHandler);

	::UnregisterClass(L"TSStatusBarClass", g_plugin.getInst());
	::UnregisterClass(L"SideBarClass", g_plugin.getInst());
	::UnregisterClass(L"TSTabCtrlClass", g_plugin.getInst());
	::UnregisterClass(L"RichEditTipClass", g_plugin.getInst());
	::UnregisterClass(L"TSHK", g_plugin.getInst());
	return 0;
}

int CGlobals::OkToExit(WPARAM, LPARAM)
{
	CWarning::destroyAll();

	CMimAPI::m_shutDown = true;

	PluginConfig.globalContainerSettings.fPrivate = false;
	Utils::WriteContainerSettingsToDB(0, &PluginConfig.globalContainerSettings);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// used on startup to restore flashing tray icon if one or more messages are still "unread"

struct MSavedEvent
{
	MSavedEvent(MCONTACT _hContact, MEVENT _hEvent) :
		hContact(_hContact),
		hEvent(_hEvent)
	{}

	MEVENT   hEvent;
	MCONTACT hContact;
};

void CGlobals::RestoreUnreadMessageAlerts(void)
{
	OBJLIST<MSavedEvent> arEvents(10, NumericKeySortT);

	for (auto &hContact : Contacts()) {
		if (db_get_dw(hContact, "SendLater", "count", 0))
			SendLater::addContact(hContact);

		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = {};
			if (db_event_get(hDbEvent, &dbei))
				continue;
			if (Proto_GetBaseAccountName(hContact) == nullptr)
				continue;

			if (!dbei.markedRead() && dbei.eventType == EVENTTYPE_MESSAGE) {
				if (Srmm_FindWindow(hContact) != nullptr)
					continue;

				arEvents.insert(new MSavedEvent(hContact, hDbEvent));
			}
		}
	}

	wchar_t toolTip[256];

	CLISTEVENT cle = {};
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = MS_MSG_READMESSAGE;
	cle.flags = CLEF_UNICODE;
	cle.szTooltip.w = toolTip;

	for (auto &e : arEvents) {
		mir_snwprintf(toolTip, TranslateT("Message from %s"), Clist_GetContactDisplayName(e->hContact));
		cle.hContact = e->hContact;
		cle.hDbEvent = e->hEvent;
		g_clistApi.pfnAddEvent(&cle);
	}
}

void CGlobals::logStatusChange(WPARAM wParam, const CContactCache *c)
{
	if (c == nullptr)
		return;

	CMsgDialog *dat = c->getDat();
	if (dat == nullptr || !c->isValid() || dat->m_iLogMode != WANT_BUILTIN_LOG)
		return;

	MCONTACT hContact = c->getContact();
	if (!g_plugin.bLogStatusChanges && !M.GetByte(hContact, "logstatuschanges", 0))
		return;

	// don't log them if WE are logging off
	if (Proto_GetStatus(c->getProto()) == ID_STATUS_OFFLINE || Contact::IsGroupChat(hContact, c->getProto()))
		return;

	uint16_t wStatus = LOWORD(wParam);
	uint16_t wOldStatus = HIWORD(wParam);
	if (wStatus == wOldStatus)
		return;

	wchar_t *szOldStatus = Clist_GetStatusModeDescription(wOldStatus, 0);
	wchar_t *szNewStatus = Clist_GetStatusModeDescription(wStatus, 0);
	if (szOldStatus == nullptr || szNewStatus == nullptr)
		return;

	CMStringW text;
	if (wStatus == ID_STATUS_OFFLINE)
		text = TranslateT("signed off.");
	else if (wOldStatus == ID_STATUS_OFFLINE)
		text.Format(TranslateT("signed on and is now %s."), szNewStatus);
	else
		text.Format(TranslateT("changed status from %s to %s."), szOldStatus, szNewStatus);

	T2Utf szMsg(text);
	DBEVENTINFO dbei = {};
	dbei.pBlob = (uint8_t*)(char*)szMsg;
	dbei.cbBlob = (int)mir_strlen(szMsg) + 1;
	dbei.flags = DBEF_UTF | DBEF_READ;
	dbei.eventType = EVENTTYPE_STATUSCHANGE;
	dbei.timestamp = time(0);
	dbei.szModule = (char*)c->getProto();
	dat->LogEvent(dbei);
}
