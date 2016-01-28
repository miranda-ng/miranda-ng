/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (�) 2012-16 Miranda NG project,
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

bool g_bShutdown = false;

CGlobals PluginConfig;

static TContainerSettings _cnt_default = { false, CNT_FLAGS_DEFAULT, CNT_FLAGSEX_DEFAULT, 255, CInfoPanel::DEGRADE_THRESHOLD, 60, _T("%n (%s)"), 1, 0 };

TCHAR* CGlobals::m_default_container_name = _T("default");

extern HANDLE 	hHookButtonPressedEvt;
extern HANDLE 	hHookToolBarLoadedEvt;

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
	m_WinVerMajor = WinVerMajor();
	m_WinVerMinor = WinVerMinor();
	m_bIsVista = IsWinVerVistaPlus() != 0;
	m_bIsWin7 = IsWinVer7Plus() != 0;

	::RegisterTabCtrlClass();
	CTip::registerClass();

	dwThreadID = GetCurrentThreadId();

	PluginConfig.g_hMenuContext = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_TABCONTEXT));
	TranslateMenu(g_hMenuContext);

	SkinAddNewSoundEx("RecvMsgActive", LPGEN("Instant messages"), LPGEN("Incoming (focused window)"));
	SkinAddNewSoundEx("RecvMsgInactive", LPGEN("Instant messages"), LPGEN("Incoming (unfocused window)"));
	SkinAddNewSoundEx("AlertMsg", LPGEN("Instant messages"), LPGEN("Incoming (new session)"));
	SkinAddNewSoundEx("SendMsg", LPGEN("Instant messages"), LPGEN("Outgoing"));
	SkinAddNewSoundEx("SendError", LPGEN("Instant messages"), LPGEN("Message send error"));

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);
	hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
	if (hCurHyperlinkHand == NULL)
		hCurHyperlinkHand = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HYPERLINKHAND));

	HDC hScrnDC = GetDC(0);
	m_DPIscaleX = GetDeviceCaps(hScrnDC, LOGPIXELSX) / 96.0;
	m_DPIscaleY = GetDeviceCaps(hScrnDC, LOGPIXELSY) / 96.0;
	ReleaseDC(0, hScrnDC);

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

	// ieView
	BOOL bIEView = ServiceExists(MS_IEVIEW_WINDOW);
	if (bIEView) {
		BOOL bOldIEView = M.GetByte("ieview_installed", 0);
		if (bOldIEView != bIEView)
			db_set_b(0, SRMSGMOD_T, "default_ieview", 1);
		db_set_b(0, SRMSGMOD_T, "ieview_installed", 1);
		HookEvent(ME_IEVIEW_OPTIONSCHANGED, ::IEViewOptionsChanged);
	}
	else db_set_b(0, SRMSGMOD_T, "ieview_installed", 0);

	m_iButtonsBarGap = M.GetByte("ButtonsBarGap", 1);
	m_hwndClist = pcli->hwndContactList;

	g_bPopupAvail = ServiceExists(MS_POPUP_ADDPOPUPT) != 0;

	CMenuItem mi;
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

	m_bSendOnShiftEnter = M.GetBool("sendonshiftenter", false);
	m_bSendOnEnter = M.GetBool(SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER);
	m_bSendOnDblEnter = M.GetBool("SendOnDblEnter", false);
	m_bAutoLocaleSupport = M.GetBool("al", false);
	m_bAutoSwitchTabs = M.GetBool("autoswitchtabs", true);
	m_iTabNameLimit = db_get_w(NULL, SRMSGMOD_T, "cut_at", 15);
	m_bCutContactNameOnTabs = M.GetBool("cuttitle", false);
	m_bStatusOnTabs = M.GetBool("tabstatus", true);
	m_bLogStatusChanges = M.GetBool("logstatuschanges", false);
	m_bUseDividers = M.GetBool("usedividers", false);
	m_bDividersUsePopupConfig = M.GetBool("div_popupconfig", false);
	m_MsgTimeout = M.GetDword(SRMSGMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);

	if (m_MsgTimeout < SRMSGSET_MSGTIMEOUT_MIN)
		m_MsgTimeout = SRMSGSET_MSGTIMEOUT_MIN;

	m_EscapeCloses = M.GetByte("escmode", 0);

	m_bHideOnClose = M.GetBool("hideonclose", false);
	m_bAllowTab = M.GetBool("tabmode", false);

	m_bFlashOnClist = M.GetBool("flashcl", false);
	m_bAlwaysFullToolbarWidth = M.GetBool("alwaysfulltoolbar", true);
	m_LimitStaticAvatarHeight = M.GetDword("avatarheight", 96);
	m_SendFormat = M.GetByte("sendformat", 0);
	m_TabAppearance = M.GetDword("tabconfig", TCF_FLASHICON | TCF_SINGLEROWTABCONTROL);
	m_panelHeight = (DWORD)M.GetDword("panelheight", CInfoPanel::DEGRADE_THRESHOLD);
	m_MUCpanelHeight = M.GetDword(CHAT_MODULE, "panelheight", CInfoPanel::DEGRADE_THRESHOLD);
	m_bIdleDetect = M.GetBool("dimIconsForIdleContacts", true);
	m_smcxicon = m_smcyicon = 16;
	m_PasteAndSend = M.GetByte("pasteandsend", 1);
	m_LangPackCP = Langpack_GetDefaultCodePage();
	m_visualMessageSizeIndicator = M.GetByte("msgsizebar", 0);
	m_autoSplit = M.GetByte("autosplit", 0);
	m_FlashOnMTN = M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINFLASH, SRMSGDEFSET_SHOWTYPINGWINFLASH);
	if (m_MenuBar == 0) {
		m_MenuBar = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENUBAR));
		TranslateMenu(m_MenuBar);
	}

	m_ipBackgroundGradient = M.GetDword(FONTMODULE, "ipfieldsbg", 0x62caff);
	if (0 == m_ipBackgroundGradient)
		m_ipBackgroundGradient = 0x62caff;

	m_ipBackgroundGradientHigh = M.GetDword(FONTMODULE, "ipfieldsbgHigh", 0xf0f0f0);
	if (0 == m_ipBackgroundGradientHigh)
		m_ipBackgroundGradientHigh = 0xf0f0f0;

	m_tbBackgroundHigh = M.GetDword(FONTMODULE, "tbBgHigh", 0);
	m_tbBackgroundLow = M.GetDword(FONTMODULE, "tbBgLow", 0);
	m_fillColor = M.GetDword(FONTMODULE, "fillColor", 0);
	if (CSkin::m_BrushFill) {
		::DeleteObject(CSkin::m_BrushFill);
		CSkin::m_BrushFill = 0;
	}
	m_genericTxtColor = M.GetDword(FONTMODULE, "genericTxtClr", GetSysColor(COLOR_BTNTEXT));
	m_cRichBorders = M.GetDword(FONTMODULE, "cRichBorders", 0);

	::memcpy(&globalContainerSettings, &_cnt_default, sizeof(TContainerSettings));
	Utils::ReadContainerSettingsFromDB(0, &globalContainerSettings);
	globalContainerSettings.fPrivate = false;
	if (fReloadSkins)
		Skin->setupAeroSkins();
}

/////////////////////////////////////////////////////////////////////////////////////////
// reload "advanced tweaks" that can be applied w / o a restart

void CGlobals::reloadAdv()
{
	m_bSoundOnTyping = M.GetBool("adv_soundontyping", false);
	m_bDontUseDefaultKbd = M.GetBool("adv_leaveKeyboardAlone", true);

	if (m_bSoundOnTyping && m_TypingSoundAdded == false) {
		SkinAddNewSoundEx("SoundOnTyping", LPGEN("Other"), LPGEN("TabSRMM: typing"));
		m_TypingSoundAdded = true;
	}
	m_bAllowOfflineMultisend = M.GetBool("AllowOfflineMultisend", false);
}

const HMENU CGlobals::getMenuBar()
{
	if (m_MenuBar == 0) {
		m_MenuBar = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENUBAR));
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
	HookEvent(ME_PROTO_CONTACTISTYPING, CMimAPI::TypingMessage);
	HookEvent(ME_PROTO_ACK, CMimAPI::ProtoAck);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreshutdownSendRecv);
	HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CMimAPI::PrebuildContactMenu);

	HookEvent(ME_SKIN2_ICONSCHANGED, ::IcoLibIconsChanged);
	HookEvent(ME_AV_MYAVATARCHANGED, ::MyAvatarChanged);
}

int CGlobals::TopToolbarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = { 0 };
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;
	ttb.pszService = MS_TABMSG_TRAYSUPPORT;
	ttb.name = "TabSRMM session list";
	ttb.pszTooltipUp = LPGEN("TabSRMM session list");
	ttb.hIconHandleUp = IcoLib_GetIcon("tabSRMM_sb_slist");
	TopToolbar_AddButton(&ttb);

	ttb.name = "TabSRMM Menu";
	ttb.pszTooltipUp = LPGEN("TabSRMM menu");
	ttb.lParamUp = ttb.lParamDown = 1;
	ttb.hIconHandleUp = IcoLib_GetIcon("tabSRMM_container");
	TopToolbar_AddButton(&ttb);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// second part of the startup initialisation.All plugins are now fully loaded

int CGlobals::ModulesLoaded(WPARAM, LPARAM)
{
	M.configureCustomFolders();

	Skin->Init(true);
	CSkin::initAeroEffect();

	for (int i = 0; i < NR_BUTTONBARICONS; i++)
		PluginConfig.g_buttonBarIcons[i] = 0;
	::LoadIconTheme();
	::CreateImageList(TRUE);

	MENUITEMINFOA mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = HBMMENU_CALLBACK;
	HMENU submenu = GetSubMenu(PluginConfig.g_hMenuContext, 7);
	for (int k = 0; k <= 8; k++)
		SetMenuItemInfoA(submenu, (UINT_PTR)k, TRUE, &mii);

	PluginConfig.reloadSystemModulesChanged();

	::Chat_ModulesLoaded();
	::BuildContainerMenu();

	::CB_InitDefaultButtons();
	::ModPlus_Init();
	::NotifyEventHooks(hHookToolBarLoadedEvt, 0, 0);

	if (M.GetByte("avatarmode", -1) == -1)
		db_set_b(0, SRMSGMOD_T, "avatarmode", 2);

	PluginConfig.g_hwndHotkeyHandler = CreateWindowEx(0, _T("TSHK"), _T(""), WS_POPUP,
		0, 0, 40, 40, 0, 0, g_hInst, NULL);

	::CreateTrayMenus(TRUE);
	if (nen_options.bTraySupport)
		::CreateSystrayIcon(TRUE);

	CMenuItem mi;

	SET_UID(mi, 0x9f68b822, 0xff97, 0x477d, 0xb7, 0x6d, 0xa5, 0x59, 0x33, 0x1c, 0x54, 0x40);
	mi.position = -500050005;
	mi.hIcolibItem = PluginConfig.g_iconContainer;
	mi.name.a = LPGEN("&Messaging settings...");
	mi.pszService = MS_TABMSG_SETUSERPREFS;
	PluginConfig.m_UserMenuItem = Menu_AddContactMenuItem(&mi);

	if (sendLater->isAvail()) {
		SET_UID(mi, 0x8f32b04e, 0x314e, 0x42eb, 0x89, 0xc6, 0x56, 0x21, 0xf5, 0x1a, 0x2f, 0x22);
		mi.position = -500050006;
		mi.hIcolibItem = 0;
		mi.name.a = LPGEN("&Send later job list...");
		mi.pszService = MS_TABMSG_SLQMGR;
		PluginConfig.m_UserMenuItem = Menu_AddMainMenuItem(&mi);
	}
	RestoreUnreadMessageAlerts();

	::RegisterFontServiceFonts();
	::CacheLogFonts();
	if (PluginConfig.g_bPopupAvail)
		TN_ModuleInit();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, DBContactDeleted);

	HookEvent(ME_DB_EVENT_ADDED, CMimAPI::DispatchNewEvent);
	HookEvent(ME_DB_EVENT_ADDED, CMimAPI::MessageEventAdded);
	HookEvent(ME_FONT_RELOAD, ::FontServiceFontsChanged);
	HookEvent(ME_TTB_MODULELOADED, TopToolbarLoaded);

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
	const char 	*szProto = NULL;
	const char  *setting = cws->szSetting;
	CContactCache* c = 0;
	bool fChanged = false, fNickChanged = false, fExtendedStatusChange = false;

	HWND hwnd = M.FindWindow(hContact);

	if (hwnd == 0 && hContact != 0) {     // we are not interested in this event if there is no open message window/tab
		if (!strcmp(setting, "Status") || !strcmp(setting, "MyHandle") || !strcmp(setting, "Nick") || !strcmp(cws->szModule, SRMSGMOD_T)) {
			c = CContactCache::getContactCache(hContact);
			if (c) {
				fChanged = c->updateStatus();
				if (strcmp(setting, "Status"))
					c->updateNick();
				if (!strcmp(setting, "isFavorite") || !strcmp(setting, "isRecent"))
					c->updateFavorite();
			}
		}
		return 0;
	}

	if (hContact == 0 && !strcmp("Nick", setting)) {
		M.BroadcastMessage(DM_OWNNICKCHANGED, 0, (LPARAM)cws->szModule);
		return 0;
	}

	if (hContact) {
		c = CContactCache::getContactCache(hContact);
		if (c) {
			szProto = c->getProto();
			if (!strcmp(cws->szModule, SRMSGMOD_T)) {					// catch own relevant settings
				if (!strcmp(setting, "isFavorite") || !strcmp(setting, "isRecent"))
					c->updateFavorite();
			}
		}
	}

	if (strcmp(cws->szModule, "CList") && (szProto == NULL || strcmp(cws->szModule, szProto)))
		return 0;

	if (!strcmp(cws->szModule, META_PROTO))
		if (hContact != 0 && !strcmp(setting, "Nick"))      // filter out this setting to avoid infinite loops while trying to obtain the most online contact
			return 0;

	if (hwnd) {
		if (c) {
			fChanged = c->updateStatus();
			fNickChanged = c->updateNick();
		}
		if (strlen(setting) > 6 && strlen(setting) < 9 && !strncmp(setting, "Status", 6)) {
			fChanged = true;
			if (c) {
				c->updateMeta();
				c->updateUIN();
			}
		}
		else if (!strcmp(setting, "MirVer"))
			PostMessage(hwnd, DM_CLIENTCHANGED, 0, 0);
		else if (!strcmp(setting, "display_uid")) {
			if (c)
				c->updateUIN();
			PostMessage(hwnd, DM_UPDATEUIN, 0, 0);
		}
		else if (strlen(setting) > 6 && strstr("StatusMsg,XStatusMsg,XStatusName,XStatusId,ListeningTo", setting)) {
			if (c) {
				c->updateStatusMsg(setting);
				fExtendedStatusChange = true;
			}
		}
		if (fChanged || fNickChanged || fExtendedStatusChange)
			PostMessage(hwnd, DM_UPDATETITLE, 0, 1);
		if (fExtendedStatusChange)
			PostMessage(hwnd, DM_UPDATESTATUSMSG, 0, 0);
		if (fChanged) {
			if (c && c->getStatus() == ID_STATUS_OFFLINE) {			// clear typing notification in the status bar when contact goes offline
				TWindowData *dat = c->getDat();
				if (dat) {
					dat->nTypeSecs = 0;
					dat->bShowTyping = 0;
					dat->szStatusBar[0] = 0;
					PostMessage(c->getHwnd(), DM_UPDATELASTMESSAGE, 0, 0);
				}
			}
			if (c)
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
		if (c)
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
		if (c) {
			c->updateMeta();
			if (c->getHwnd()) {
				::PostMessage(c->getHwnd(), DM_UPDATETITLE, 0, 1);
				::PostMessage(c->getHwnd(), DM_UPDATEPICLAYOUT, 0, 0);
				InvalidateRect(c->getHwnd(), 0, TRUE); // force redraw
			}
		}
	}
	return 0;
}

int CGlobals::PreshutdownSendRecv(WPARAM, LPARAM)
{
	g_bShutdown = true;

	::TN_ModuleDeInit();
	::CloseAllContainers();

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		db_set_dw(hContact, SRMSGMOD_T, "messagecount", 0);

	::SI_DeinitStatusIcons();
	::CB_DeInitCustomButtons();

	// the event API
	DestroyHookableEvent(PluginConfig.m_event_MsgWin);
	DestroyHookableEvent(PluginConfig.m_event_MsgPopup);
	DestroyHookableEvent(PluginConfig.m_event_WriteEvent);

	::NEN_WriteOptions(&nen_options);
	::DestroyWindow(PluginConfig.g_hwndHotkeyHandler);

	::UnregisterClass(_T("TSStatusBarClass"), g_hInst);
	::UnregisterClass(_T("SideBarClass"), g_hInst);
	::UnregisterClass(_T("TSTabCtrlClass"), g_hInst);
	::UnregisterClass(_T("RichEditTipClass"), g_hInst);
	::UnregisterClass(_T("TSHK"), g_hInst);
	return 0;
}

int CGlobals::OkToExit(WPARAM, LPARAM)
{
	::CreateSystrayIcon(0);
	::CreateTrayMenus(0);

	CWarning::destroyAll();

	CMimAPI::m_shutDown = true;

	PluginConfig.globalContainerSettings.fPrivate = false;
	::db_set_blob(0, SRMSGMOD_T, CNT_KEYNAME, &PluginConfig.globalContainerSettings, sizeof(TContainerSettings));
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

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (db_get_dw(hContact, "SendLater", "count", 0))
			sendLater->addContact(hContact);

		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			if (!dbei.markedRead() && dbei.eventType == EVENTTYPE_MESSAGE) {
				if (M.FindWindow(hContact) != NULL)
					continue;

				arEvents.insert(new MSavedEvent(hContact, hDbEvent));
			}
		}
	}

	TCHAR toolTip[256];

	CLISTEVENT cle = {};
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_TCHAR;
	cle.ptszTooltip = toolTip;

	for (int i = 0; i < arEvents.getCount(); i++) {
		MSavedEvent &e = arEvents[i];
		mir_sntprintf(toolTip, TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(e.hContact, 0));
		cle.hContact = e.hContact;
		cle.hDbEvent = e.hEvent;
		pcli->pfnAddEvent(&cle);
	}
}

void CGlobals::logStatusChange(WPARAM wParam, const CContactCache *c)
{
	if (c == 0)
		return;

	TWindowData *dat = c->getDat();
	if (dat == NULL || !c->isValid())
		return;

	MCONTACT hContact = c->getContact();
	if (!PluginConfig.m_bLogStatusChanges && !M.GetByte(hContact, "logstatuschanges", 0))
		return;

	// don't log them if WE are logging off
	if (CallProtoService(c->getProto(), PS_GETSTATUS, 0, 0) == ID_STATUS_OFFLINE)
		return;

	WORD wStatus = LOWORD(wParam);
	WORD wOldStatus = HIWORD(wParam);
	if (wStatus == wOldStatus)
		return;

	TCHAR *szOldStatus = pcli->pfnGetStatusModeDescription(wOldStatus, 0);
	TCHAR *szNewStatus = pcli->pfnGetStatusModeDescription(wStatus, 0);
	if (szOldStatus == 0 || szNewStatus == 0)
		return;

	CMString text;
	if (wStatus == ID_STATUS_OFFLINE)
		text = TranslateT("signed off.");
	else if (wOldStatus == ID_STATUS_OFFLINE)
		text.Format(TranslateT("signed on and is now %s."), szNewStatus);
	else
		text.Format(TranslateT("changed status from %s to %s."), szOldStatus, szNewStatus);

	T2Utf szMsg(text);
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.pBlob = (PBYTE)(char*)szMsg;
	dbei.cbBlob = (int)mir_strlen(szMsg) + 1;
	dbei.flags = DBEF_UTF | DBEF_READ;
	dbei.eventType = EVENTTYPE_STATUSCHANGE;
	dbei.timestamp = time(NULL);
	dbei.szModule = (char*)c->getProto();
	StreamInEvents(dat->hwnd, NULL, 1, 1, &dbei);
}

/////////////////////////////////////////////////////////////////////////////////////////
// on Windows 7, when using new task bar features (grouping mode and per tab
// previews), autoswitching does not work relieably, so it is disabled.
//
// @return: true if configuration dictates autoswitch

bool CGlobals::haveAutoSwitch()
{
	if (m_bIsWin7 && m_useAeroPeek && !CSkin::m_skinEnabled)
		return false;

	return m_bAutoSwitchTabs;
}
