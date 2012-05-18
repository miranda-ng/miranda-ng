/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: globals.cpp 13447 2011-03-14 19:55:07Z george.hazan $
 *
 * Plugin configuration variables and functions. Implemented as a class
 * though there will always be only a single instance.
 *
 */

#include "commonheaders.h"
extern PLUGININFOEX pluginInfo;

CGlobals 	PluginConfig;
CGlobals*	pConfig = &PluginConfig;

static TContainerSettings _cnt_default = {
		false,
		CNT_FLAGS_DEFAULT,
		CNT_FLAGSEX_DEFAULT,
		255,
		CInfoPanel::DEGRADE_THRESHOLD,
		60,
		_T("%n (%s)"),
		1,
		0
};

HANDLE		CGlobals::m_event_ModulesLoaded = 0, CGlobals::m_event_PrebuildMenu = 0, CGlobals::m_event_SettingChanged = 0;
HANDLE		CGlobals::m_event_ContactDeleted = 0, CGlobals::m_event_Dispatch = 0, CGlobals::m_event_EventAdded = 0;
HANDLE		CGlobals::m_event_IconsChanged = 0, CGlobals::m_event_TypingEvent = 0, CGlobals::m_event_ProtoAck = 0;
HANDLE		CGlobals::m_event_PreShutdown = 0, CGlobals::m_event_OkToExit = 0;
HANDLE		CGlobals::m_event_IcoLibChanged = 0, CGlobals::m_event_AvatarChanged = 0, CGlobals::m_event_MyAvatarChanged = 0, CGlobals::m_event_FontsChanged = 0;
HANDLE		CGlobals::m_event_SmileyAdd = 0, CGlobals::m_event_IEView = 0, CGlobals::m_event_FoldersChanged = 0;
HANDLE 		CGlobals::m_event_ME_MC_SUBCONTACTSCHANGED = 0, CGlobals::m_event_ME_MC_FORCESEND = 0, CGlobals::m_event_ME_MC_UNFORCESEND = 0;
TCHAR*		CGlobals::m_default_container_name = _T("default");

extern 		HANDLE 	hHookButtonPressedEvt;
extern		HANDLE 	hHookToolBarLoadedEvt;

EXCEPTION_RECORD CGlobals::m_exRecord = {0};
CONTEXT		 	 CGlobals::m_exCtx = {0};
LRESULT			 CGlobals::m_exLastResult = 0;
char			 CGlobals::m_exSzFile[MAX_PATH] = "\0";
wchar_t			 CGlobals::m_exReason[256] = L"\0";
int				 CGlobals::m_exLine = 0;
bool			 CGlobals::m_exAllowContinue = false;

#if defined(_WIN64)
	static char szCurrentVersion[30];
	static char *szVersionUrl = "http://download.miranda.or.at/tabsrmm/3/version.txt";
	static char *szUpdateUrl = "http://silvercircle.googlecode.com/files/tabsrmm-3_x64.zip";
	static char *szFLVersionUrl = "http://addons.miranda-im.org/details.php?action=viewfile&id=3699";
	static char *szFLUpdateurl = "http://addons.miranda-im.org/feed.php?dlfile=3699";
#else
	static char szCurrentVersion[30];
	static char *szVersionUrl = "http://download.miranda.or.at/tabsrmm/3/version.txt";
	static char *szUpdateUrl = "http://silvercircle.googlecode.com/files/tabsrmm-3_x86.zip";
	static char *szFLVersionUrl = ADDONS_UPDATE_URL;
	static char *szFLUpdateurl =  ADDONS_DL_URL;
#endif
	static char *szPrefix = "tabsrmm ";


CRTException::CRTException(const char *szMsg, const TCHAR *szParam) : std::runtime_error(std::string(szMsg))
{
	mir_sntprintf(m_szParam, MAX_PATH, szParam);
}

void CRTException::display() const
{
	TCHAR*	tszMsg = mir_a2t(what());
	TCHAR  	tszBoxMsg[500];

	mir_sntprintf(tszBoxMsg, 500, _T("%s\n\n(%s)"), tszMsg, m_szParam);
	::MessageBox(0, tszBoxMsg, _T("TabSRMM runtime error"), MB_OK | MB_ICONERROR);
	mir_free(tszMsg);
}

void CGlobals::RegisterWithUpdater()
{
	Update 		upd = {0};

 	if (!ServiceExists(MS_UPDATE_REGISTER))
 		return;

 	upd.cbSize 				=	sizeof(upd);
 	upd.szComponentName 	=	pluginInfo.shortName;
 	upd.pbVersion 			=	(BYTE *)CreateVersionString(pluginInfo.version, szCurrentVersion);
 	upd.cpbVersion 			=	(int)(strlen((char *)upd.pbVersion));
	upd.szVersionURL 		=	szFLVersionUrl;
	upd.szUpdateURL 		=	szFLUpdateurl;
	upd.pbVersionPrefix 	= 	(BYTE *)"<span class=\"fileNameHeader\">tabSRMM Unicode 2.0 ";
	upd.cpbVersionPrefix 	= 	(int)(strlen((char *)upd.pbVersionPrefix));

 	upd.szBetaUpdateURL 	=	szUpdateUrl;
 	upd.szBetaVersionURL 	=	szVersionUrl;
	upd.pbVersion 			=	(unsigned char *)szCurrentVersion;
	upd.cpbVersion 			= 	lstrlenA(szCurrentVersion);
 	upd.pbBetaVersionPrefix	= 	(BYTE *)szPrefix;
 	upd.cpbBetaVersionPrefix= 	(int)(strlen((char *)upd.pbBetaVersionPrefix));
 	upd.szBetaChangelogURL  =	"http://blog.miranda.or.at/tabsrmm-articles/tabsrmm-version-3-changelog";

 	CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
}
/**
 * reload system values. These are read ONCE and are not allowed to change
 * without a restart
 */
void CGlobals::reloadSystemStartup()
{
	HDC			hScrnDC;
	DBVARIANT 	dbv = {0};

	m_WinVerMajor = 					WinVerMajor();
	m_WinVerMinor = 					WinVerMinor();
	m_bIsXP = 							IsWinVerXPPlus();
	m_bIsVista = 						IsWinVerVistaPlus();
	m_bIsWin7 = 						IsWinVer7Plus();

	::LoadTSButtonModule();
	::RegisterTabCtrlClass();
	CTip::registerClass();

	dwThreadID = 						GetCurrentThreadId();

	PluginConfig.g_hMenuContext = 		LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_TABCONTEXT));
	CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)g_hMenuContext, 0);

	SkinAddNewSoundEx("RecvMsgActive", "Instant messages", "Incoming (Focused Window)");
	SkinAddNewSoundEx("RecvMsgInactive", "Instant messages", "Incoming (Unfocused Window)");
	SkinAddNewSoundEx("AlertMsg", "Instant messages", "Incoming (New Session)");
	SkinAddNewSoundEx("SendMsg", "Instant messages", "Outgoing");
	SkinAddNewSoundEx("SendError", "Instant messages", "Message send error");

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);
	hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
	if (hCurHyperlinkHand == NULL)
		hCurHyperlinkHand = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HYPERLINKHAND));

	hScrnDC = GetDC(0);
	g_DPIscaleX = 						GetDeviceCaps(hScrnDC, LOGPIXELSX) / 96.0;
	g_DPIscaleY = 						GetDeviceCaps(hScrnDC, LOGPIXELSY) / 96.0;
	ReleaseDC(0, hScrnDC);

	reloadSettings(false);
	reloadAdv();
	hookSystemEvents();
}

/**
 * this runs ONCE at startup when the Modules Loaded event is fired
 * by the core. all plugins are loaded and ready to use.
 * 
 * any initialation for 3rd party plugins must go here.
 */
void CGlobals::reloadSystemModulesChanged()
{
	BOOL				bIEView = FALSE;
	CLISTMENUITEM 		mi = { 0 };

	m_MathModAvail = 					ServiceExists(MATH_RTF_REPLACE_FORMULAE);

	/*
	 * smiley add
	 */
	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS)) {
		PluginConfig.g_SmileyAddAvail = 1;
		m_event_SmileyAdd = HookEvent(ME_SMILEYADD_OPTIONSCHANGED, ::SmileyAddOptionsChanged);
	}
	else
		m_event_SmileyAdd = 0;

	/*
	 * Flashavatars
	 */

	g_FlashAvatarAvail = (ServiceExists(MS_FAVATAR_GETINFO) ? 1 : 0);

	/*
	 * ieView
	 */

	bIEView = ServiceExists(MS_IEVIEW_WINDOW);
	if (bIEView) {
		BOOL bOldIEView = M->GetByte("ieview_installed", 0);
		if (bOldIEView != bIEView)
			M->WriteByte(SRMSGMOD_T, "default_ieview", 1);
		M->WriteByte(SRMSGMOD_T, "ieview_installed", 1);
		m_event_IEView = HookEvent(ME_IEVIEW_OPTIONSCHANGED, ::IEViewOptionsChanged);
	} else {
		M->WriteByte(SRMSGMOD_T, "ieview_installed", 0);
		m_event_IEView = 0;
	}

	g_iButtonsBarGap = 							M->GetByte("ButtonsBarGap", 1);
	m_hwndClist = 								(HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	m_MathModAvail = 							(ServiceExists(MATH_RTF_REPLACE_FORMULAE) ? 1 : 0);
	if (m_MathModAvail) {
		char *szDelim = (char *)CallService(MATH_GET_STARTDELIMITER, 0, 0);
		if (szDelim) {
			MultiByteToWideChar(CP_ACP, 0, szDelim, -1, PluginConfig.m_MathModStartDelimiter, safe_sizeof(PluginConfig.m_MathModStartDelimiter));
			CallService(MTH_FREE_MATH_BUFFER, 0, (LPARAM)szDelim);
		}
	}
	else
		PluginConfig.m_MathModStartDelimiter[0] = 0;

	g_MetaContactsAvail = (ServiceExists(MS_MC_GETDEFAULTCONTACT) ? 1 : 0);


	if(g_MetaContactsAvail) {
		mir_snprintf(szMetaName, 256, "%s", (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0));
		bMetaEnabled = abs(M->GetByte(0, szMetaName, "Enabled", -1));
	}
	else {
		szMetaName[0] = 0;
		bMetaEnabled = 0;
	}

	g_PopupAvail = (ServiceExists(MS_POPUP_ADDPOPUPEX) ? 1 : 0);
	g_PopupWAvail = (ServiceExists(MS_POPUP_ADDPOPUPW) ? 1 : 0);

	mi.cbSize = sizeof(mi);
	mi.position = -2000090000;
	if (ServiceExists(MS_SKIN2_GETICONBYHANDLE)) {
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_DEFAULT;
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_MESSAGE);
	} else {
		mi.flags = CMIF_DEFAULT;
		mi.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	}
	mi.pszName = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	PluginConfig.m_hMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

	m_useAeroPeek = M->GetByte("useAeroPeek", 1);
}

/**
 * reload plugin settings on startup and runtime. Most of these setttings can be
 * changed while plugin is running.
 */
void CGlobals::reloadSettings(bool fReloadSkins)
{
	m_ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &m_ncm, 0);

	DWORD dwFlags = 					M->GetDword("mwflags", MWF_LOG_DEFAULT);

	m_SendOnShiftEnter = 				(int)M->GetByte("sendonshiftenter", 0);
	m_SendOnEnter = 					(int)M->GetByte(SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER);
	m_SendOnDblEnter = 					(int)M->GetByte("SendOnDblEnter", 0);
	m_AutoLocaleSupport = 				(int)M->GetByte("al", 0);
	m_AutoSwitchTabs = 					(int)M->GetByte("autoswitchtabs", 1);
	m_CutContactNameTo = 				(int) DBGetContactSettingWord(NULL, SRMSGMOD_T, "cut_at", 15);
	m_CutContactNameOnTabs = 			(int)M->GetByte("cuttitle", 0);
	m_StatusOnTabs = 					(int)M->GetByte("tabstatus", 1);
	m_LogStatusChanges = 				(int)dwFlags & MWF_LOG_STATUSCHANGES;
	m_UseDividers = 					(int)M->GetByte("usedividers", 0);
	m_DividersUsePopupConfig = 			(int)M->GetByte("div_popupconfig", 0);
	m_MsgTimeout = 						(int)M->GetDword(SRMSGMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);

	if (m_MsgTimeout < SRMSGSET_MSGTIMEOUT_MIN)
		m_MsgTimeout = SRMSGSET_MSGTIMEOUT_MIN;

	m_EscapeCloses = 					(int)M->GetByte("escmode", 0);

	m_HideOnClose =						(int) M->GetByte("hideonclose", 0);
	m_AllowTab =						(int) M->GetByte("tabmode", 0);

	m_FlashOnClist = 					(int)M->GetByte("flashcl", 0);
	m_AlwaysFullToolbarWidth = 			(int)M->GetByte("alwaysfulltoolbar", 1);
	m_LimitStaticAvatarHeight = 		(int)M->GetDword("avatarheight", 96);
	m_SendFormat = 						(int)M->GetByte("sendformat", 0);
	m_FormatWholeWordsOnly = 1;
	m_RTLDefault = 						(int)M->GetByte("rtldefault", 0);
	m_TabAppearance = 					(int)M->GetDword("tabconfig", TCF_FLASHICON | TCF_SINGLEROWTABCONTROL);
	m_panelHeight = 					(DWORD)M->GetDword("panelheight", CInfoPanel::DEGRADE_THRESHOLD);
	m_MUCpanelHeight = 					M->GetDword("Chat", "panelheight", CInfoPanel::DEGRADE_THRESHOLD);
	m_IdleDetect = 						(int)M->GetByte("dimIconsForIdleContacts", 1);
	m_smcxicon = 16;
	m_smcyicon = 16;
	m_PasteAndSend = 					(int)M->GetByte("pasteandsend", 1);
	m_szNoStatus = 						const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_NO_STATUS));
	m_LangPackCP = 						ServiceExists(MS_LANGPACK_GETCODEPAGE) ? CallService(MS_LANGPACK_GETCODEPAGE, 0, 0) : CP_ACP;
	m_visualMessageSizeIndicator = 		M->GetByte("msgsizebar", 0);
	m_autoSplit = 						M->GetByte("autosplit", 0);
	m_FlashOnMTN = 						M->GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINFLASH, SRMSGDEFSET_SHOWTYPINGWINFLASH);
	if(m_MenuBar == 0) {
		m_MenuBar = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENUBAR));
		CallService(MS_LANGPACK_TRANSLATEMENU, WPARAM(m_MenuBar), 0);
	}

	m_ipBackgroundGradient = 			M->GetDword(FONTMODULE, "ipfieldsbg", 0x62caff);
	if(0 == m_ipBackgroundGradient)
		m_ipBackgroundGradient = 0x62caff;

	m_ipBackgroundGradientHigh = 		M->GetDword(FONTMODULE, "ipfieldsbgHigh", 0xf0f0f0);
	if(0 == m_ipBackgroundGradientHigh)
		m_ipBackgroundGradientHigh = 0xf0f0f0;

	m_tbBackgroundHigh = 				M->GetDword(FONTMODULE, "tbBgHigh", 0);
	m_tbBackgroundLow = 				M->GetDword(FONTMODULE, "tbBgLow", 0);
	m_fillColor =						M->GetDword(FONTMODULE, "fillColor", 0);
	if(CSkin::m_BrushFill) {
		::DeleteObject(CSkin::m_BrushFill);
		CSkin::m_BrushFill = 0;
	}
	m_genericTxtColor = 				M->GetDword(FONTMODULE, "genericTxtClr", GetSysColor(COLOR_BTNTEXT));
	m_cRichBorders =					M->GetDword(FONTMODULE, "cRichBorders", 0);

	::CopyMemory(&globalContainerSettings, &_cnt_default, sizeof(TContainerSettings));
	Utils::ReadContainerSettingsFromDB(0, &globalContainerSettings);
	globalContainerSettings.fPrivate = false;
	if(fReloadSkins)
		Skin->setupAeroSkins();
}

/**
 * reload "advanced tweaks" that can be applied w/o a restart
 */
void CGlobals::reloadAdv()
{
	g_bDisableAniAvatars=				M->GetByte("adv_DisableAniAvatars", 0);
	g_bSoundOnTyping = 					M->GetByte("adv_soundontyping", 0);
	m_dontUseDefaultKbd=				M->GetByte("adv_leaveKeyboardAlone", 1);
	g_bClientInStatusBar = 				M->GetByte("adv_ClientIconInStatusBar", 0);

	if(g_bSoundOnTyping && m_TypingSoundAdded == false) {
		SkinAddNewSoundEx("SoundOnTyping", "Other", "TABSRMM: Typing");
		m_TypingSoundAdded = true;
	}
	m_AllowOfflineMultisend =			M->GetByte("AllowOfflineMultisend", 0);
}

const HMENU CGlobals::getMenuBar()
{
	if(m_MenuBar == 0) {
		m_MenuBar = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENUBAR));
		CallService(MS_LANGPACK_TRANSLATEMENU, WPARAM(m_MenuBar), 0);
	}
	return(m_MenuBar);
}

/**
 * hook core events. This runs in LoadModule()
 * only core events and services are guaranteed to exist at this time
 */
void CGlobals::hookSystemEvents()
{
	m_event_ModulesLoaded 	= 	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	m_event_IconsChanged 	= 	HookEvent(ME_SKIN_ICONSCHANGED, ::IconsChanged);
	m_event_TypingEvent 	= 	HookEvent(ME_PROTO_CONTACTISTYPING, CMimAPI::TypingMessage);
	m_event_ProtoAck 		= 	HookEvent(ME_PROTO_ACK, CMimAPI::ProtoAck);
	m_event_PreShutdown 	= 	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreshutdownSendRecv);
	m_event_OkToExit 		= 	HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);

	m_event_PrebuildMenu 	= 	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CMimAPI::PrebuildContactMenu);

	m_event_IcoLibChanged 	= 	HookEvent(ME_SKIN2_ICONSCHANGED, ::IcoLibIconsChanged);
	m_event_AvatarChanged 	= 	HookEvent(ME_AV_AVATARCHANGED, ::AvatarChanged);
	m_event_MyAvatarChanged = 	HookEvent(ME_AV_MYAVATARCHANGED, ::MyAvatarChanged);
}

/**
 * second part of the startup initialisation. All plugins are now fully loaded
 */

int CGlobals::ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int 				i;
	MENUITEMINFOA 		mii = {0};
	HMENU 				submenu;
	CLISTMENUITEM 		mi = { 0 };

	::UnhookEvent(m_event_ModulesLoaded);

	M->configureCustomFolders();

	Skin->Init(true);
	CSkin::initAeroEffect();

	for (i = 0; i < NR_BUTTONBARICONS; i++)
		PluginConfig.g_buttonBarIcons[i] = 0;
	::LoadIconTheme();
	::CreateImageList(TRUE);

	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = HBMMENU_CALLBACK;
	submenu = GetSubMenu(PluginConfig.g_hMenuContext, 7);
	for (i = 0; i <= 8; i++)
		SetMenuItemInfoA(submenu, (UINT_PTR)i, TRUE, &mii);

	PluginConfig.reloadSystemModulesChanged();

	::BuildContainerMenu();

	::CB_InitDefaultButtons();
	::ModPlus_Init(wParam, lParam);
	::NotifyEventHooks(hHookToolBarLoadedEvt, (WPARAM)0, (LPARAM)0);
	//

	if (M->GetByte("avatarmode", -1) == -1)
		M->WriteByte(SRMSGMOD_T, "avatarmode", 2);

	PluginConfig.g_hwndHotkeyHandler = CreateWindowEx(0, _T("TSHK"), _T(""), WS_POPUP,
								  0, 0, 40, 40, 0, 0, g_hInst, NULL);

	::CreateTrayMenus(TRUE);
	if (nen_options.bTraySupport)
		::CreateSystrayIcon(TRUE);

	mi.cbSize = sizeof(mi);
	mi.position = -500050005;
	mi.hIcon = PluginConfig.g_iconContainer;
	mi.pszContactOwner = NULL;
	mi.pszName = LPGEN("&Messaging settings...");
	mi.pszService = MS_TABMSG_SETUSERPREFS;
	PluginConfig.m_UserMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

	if(sendLater->isAvail()) {
		mi.cbSize = sizeof(mi);
		mi.position = -500050006;
		mi.hIcon = 0;
		mi.pszContactOwner = NULL;
		mi.pszName = LPGEN("&Send later job list...");
		mi.pszService = MS_TABMSG_SLQMGR;
		PluginConfig.m_UserMenuItem = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) & mi);
	}
	RestoreUnreadMessageAlerts();

	RegisterWithUpdater();

	::RegisterFontServiceFonts();
	::CacheLogFonts();
	::Chat_ModulesLoaded(wParam, lParam);
	if(PluginConfig.g_PopupWAvail||PluginConfig.g_PopupAvail)
		TN_ModuleInit();

	m_event_SettingChanged 	= HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	m_event_ContactDeleted 	= HookEvent(ME_DB_CONTACT_DELETED, DBContactDeleted);

	m_event_Dispatch 		= HookEvent(ME_DB_EVENT_ADDED, CMimAPI::DispatchNewEvent);
	m_event_EventAdded 		= HookEvent(ME_DB_EVENT_ADDED, CMimAPI::MessageEventAdded);
	if(PluginConfig.g_MetaContactsAvail) {
		m_event_ME_MC_SUBCONTACTSCHANGED = HookEvent(ME_MC_SUBCONTACTSCHANGED, MetaContactEvent);
		m_event_ME_MC_FORCESEND			 = HookEvent(ME_MC_FORCESEND, MetaContactEvent);
		m_event_ME_MC_UNFORCESEND		 = HookEvent(ME_MC_UNFORCESEND, MetaContactEvent);
	}
	m_event_FontsChanged 	= 	HookEvent(ME_FONT_RELOAD, ::FontServiceFontsChanged);
	return 0;
}

/**
 * watches various important database settings and reacts accordingly
 * needed to catch status, nickname and other changes in order to update open message
 * sessions.
 */

int CGlobals::DBSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	const char 	*szProto = NULL;
	const char  *setting = cws->szSetting;
	HWND		hwnd = 0;
	CContactCache* c = 0;
	bool		fChanged = false, fNickChanged = false, fExtendedStatusChange = false;

	hwnd = M->FindWindow((HANDLE)wParam);

	if (hwnd == 0 && wParam != 0) {     // we are not interested in this event if there is no open message window/tab
		if(!strcmp(setting, "Status") || !strcmp(setting, "MyHandle") || !strcmp(setting, "Nick") || !strcmp(cws->szModule, SRMSGMOD_T)) {
			c = CContactCache::getContactCache((HANDLE)wParam);
			if(c) {
				fChanged = c->updateStatus();
				if(strcmp(setting, "Status"))
					c->updateNick();
				if(!strcmp(setting, "isFavorite") || !strcmp(setting, "isRecent"))
					c->updateFavorite();
			}
		}
		return(0);
	}

	if (wParam == 0 && !strcmp("Nick", setting)) {
		M->BroadcastMessage(DM_OWNNICKCHANGED, 0, (LPARAM)cws->szModule);
		return(0);
	}

	if(wParam) {
		c = CContactCache::getContactCache((HANDLE)wParam);
		if(c) {
			szProto = c->getProto();
			if(!strcmp(cws->szModule, SRMSGMOD_T)) {					// catch own relevant settings
				if(!strcmp(setting, "isFavorite") || !strcmp(setting, "isRecent"))
					c->updateFavorite();
			}
		}
	}

	if(wParam == 0 && !lstrcmpA(setting, "Enabled")) {
		if(PluginConfig.g_MetaContactsAvail && !lstrcmpA(cws->szModule, PluginConfig.szMetaName)) { 		// catch the disabled meta contacts
			PluginConfig.bMetaEnabled = abs(M->GetByte(0, PluginConfig.szMetaName, "Enabled", -1));
			cacheUpdateMetaChanged();
		}
	}

	if (lstrcmpA(cws->szModule, "CList") && (szProto == NULL || lstrcmpA(cws->szModule, szProto)))
		return(0);

	if (PluginConfig.g_MetaContactsAvail && !lstrcmpA(cws->szModule, PluginConfig.szMetaName)) {
		if(wParam != 0 && !lstrcmpA(setting, "Nick"))      // filter out this setting to avoid infinite loops while trying to obtain the most online contact
			return(0);
	}

	if (hwnd) {
		if(c) {
			fChanged = c->updateStatus();
			fNickChanged = c->updateNick();
		}
		if (lstrlenA(setting) > 6 && lstrlenA(setting) < 9 && !strncmp(setting, "Status", 6)) {
			fChanged = true;
			if(c) {
				c->updateMeta(true);
				c->updateUIN();
			}
		}
		else if (!strcmp(setting, "MirVer"))
			PostMessage(hwnd, DM_CLIENTCHANGED, 0, 0);
		else if (!strcmp(setting, "display_uid")) {
			if(c)
				c->updateUIN();
			PostMessage(hwnd, DM_UPDATEUIN, 0, 0);
		}
		else if(lstrlenA(setting) > 6 && strstr("StatusMsg,XStatusMsg,XStatusName,XStatusId,ListeningTo", setting)) {
			if(c) {
				c->updateStatusMsg(setting);
				fExtendedStatusChange = true;
			}
		}
		if(fChanged || fNickChanged || fExtendedStatusChange)
			PostMessage(hwnd, DM_UPDATETITLE, 0, 1);
		if(fExtendedStatusChange)
			PostMessage(hwnd, DM_UPDATESTATUSMSG, 0, 0);
		if(fChanged) {
			if(c && c->getStatus() == ID_STATUS_OFFLINE) {			// clear typing notification in the status bar when contact goes offline
				TWindowData* dat = c->getDat();
				if(dat) {
					dat->nTypeSecs = 0;
					dat->showTyping = 0;
					dat->szStatusBar[0] = 0;
					PostMessage(c->getHwnd(), DM_UPDATELASTMESSAGE, 0, 0);
				}
			}
			if(c)
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_LOGSTATUSCHANGE, MAKELONG(c->getStatus(), c->getOldStatus()), (LPARAM)c);
		}
	}
	return(0);
}

/**
 * event fired when a contact has been deleted. Make sure to close its message session
 */

int CGlobals::DBContactDeleted(WPARAM wParam, LPARAM lParam)
{
	if(wParam) {
		CContactCache *c = CContactCache::getContactCache((HANDLE)wParam);
		if(c)
			c->deletedHandler();
	}
	return 0;
}

/**
 * Handle events from metacontacts protocol. Basically, just update
 * our contact cache and, if a message window exists, tell it to update
 * relevant information.
 */
int CGlobals::MetaContactEvent(WPARAM wParam, LPARAM lParam)
{
	if(wParam) {
		CContactCache *c = CContactCache::getContactCache((HANDLE)wParam);
		if(c) {
			c->updateMeta(true);
			if(c->getHwnd()) {
				c->updateUIN();								// only do this for open windows, not needed normally
				::PostMessage(c->getHwnd(), DM_UPDATETITLE, 0, 0);
			}
		}
	}
	return(0);
}

int CGlobals::PreshutdownSendRecv(WPARAM wParam, LPARAM lParam)
{
	HANDLE 	hContact;
	int		i;

#if defined(__USE_EX_HANDLERS)
	__try {
#endif
		if (PluginConfig.m_chat_enabled)
			::Chat_PreShutdown();

		::TN_ModuleDeInit();

		while(pFirstContainer){
			if (PluginConfig.m_HideOnClose)
				PluginConfig.m_HideOnClose = FALSE;
			::SendMessage(pFirstContainer->hwnd, WM_CLOSE, 0, 1);
		}

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact) {
			M->WriteDword(hContact, SRMSGMOD_T, "messagecount", 0);
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}

		for(i = 0; i < SERVICE_LAST; i++) {
			if(PluginConfig.hSvc[i])
				DestroyServiceFunction(PluginConfig.hSvc[i]);
		}

		::SI_DeinitStatusIcons();
		::CB_DeInitCustomButtons();
		/*
		 * the event API
		 */

		DestroyHookableEvent(PluginConfig.m_event_MsgWin);
		DestroyHookableEvent(PluginConfig.m_event_MsgPopup);

		::NEN_WriteOptions(&nen_options);
		::DestroyWindow(PluginConfig.g_hwndHotkeyHandler);

		::UnregisterClass(_T("TSStatusBarClass"), g_hInst);
		::UnregisterClass(_T("SideBarClass"), g_hInst);
		::UnregisterClassA("TSTabCtrlClass", g_hInst);
		::UnregisterClass(_T("RichEditTipClass"), g_hInst);
		::UnregisterClass(_T("TSHK"), g_hInst);
#if defined(__USE_EX_HANDLERS)
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"SHUTDOWN_STAGE2", false)) {
		return(0);
	}
#endif
	return 0;
}

int CGlobals::OkToExit(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(m_event_OkToExit);
#if defined(__USE_EX_HANDLERS)
	__try {
#endif
		::CreateSystrayIcon(0);
		::CreateTrayMenus(0);

		CWarning::destroyAll();

		CMimAPI::m_shutDown = true;
		UnhookEvent(m_event_EventAdded);
		UnhookEvent(m_event_Dispatch);
		UnhookEvent(m_event_PrebuildMenu);
		UnhookEvent(m_event_SettingChanged);
		UnhookEvent(m_event_ContactDeleted);
		UnhookEvent(m_event_AvatarChanged);
		UnhookEvent(m_event_MyAvatarChanged);
		UnhookEvent(m_event_ProtoAck);
		UnhookEvent(m_event_TypingEvent);
		UnhookEvent(m_event_FontsChanged);
		UnhookEvent(m_event_IcoLibChanged);
		UnhookEvent(m_event_IconsChanged);

		if(m_event_SmileyAdd)
			UnhookEvent(m_event_SmileyAdd);

		if(m_event_IEView)
			UnhookEvent(m_event_IEView);

		if(m_event_FoldersChanged)
			UnhookEvent(m_event_FoldersChanged);

		if(m_event_ME_MC_FORCESEND) {
			UnhookEvent(m_event_ME_MC_FORCESEND);
			UnhookEvent(m_event_ME_MC_SUBCONTACTSCHANGED);
			UnhookEvent(m_event_ME_MC_UNFORCESEND);
		}
		::ModPlus_PreShutdown(wParam, lParam);
		PluginConfig.globalContainerSettings.fPrivate = false;
		::DBWriteContactSettingBlob(0, SRMSGMOD_T, CNT_KEYNAME, &PluginConfig.globalContainerSettings, sizeof(TContainerSettings));
#if defined(__USE_EX_HANDLERS)
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"SHUTDOWN_STAGE1", false)) {
		return(0);
	}
#endif
	return 0;
}

/**
 * used on startup to restore flashing tray icon if one or more messages are
 * still "unread"
 */

void CGlobals::RestoreUnreadMessageAlerts(void)
{
	CLISTEVENT 	cle = { 0 };
	DBEVENTINFO dbei = { 0 };
	TCHAR		toolTip[256];
	int 		windowAlreadyExists;
	int 		usingReadNext = 0;

	int autoPopup = M->GetByte(SRMSGMOD, SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP);
	HANDLE hDbEvent, hContact;

	dbei.cbSize = sizeof(dbei);
	cle.cbSize = sizeof(cle);
	cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_TCHAR;

	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {

		if(M->GetDword(hContact, "SendLater", "count", 0))
		   sendLater->addContact(hContact);

		hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM) hContact, 0);
		while (hDbEvent) {
			dbei.cbBlob = 0;
			CallService(MS_DB_EVENT_GET, (WPARAM) hDbEvent, (LPARAM) & dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && dbei.eventType == EVENTTYPE_MESSAGE) {
				windowAlreadyExists = M->FindWindow(hContact) != NULL;
				if (!usingReadNext && windowAlreadyExists)
					continue;

				cle.hContact = hContact;
				cle.hDbEvent = hDbEvent;
				mir_sntprintf(toolTip, safe_sizeof(toolTip), CTranslator::get(CTranslator::GEN_STRING_MESSAGEFROM),
							  (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_TCHAR));
				cle.ptszTooltip = toolTip;
				CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
			}
			hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
}

void CGlobals::logStatusChange(WPARAM wParam, const CContactCache *c)
{
	if(c == 0)
		return;

	HANDLE	hContact = c->getContact();

	bool	fGlobal = PluginConfig.m_LogStatusChanges ? true : false;
	DWORD	dwMask = M->GetDword(hContact, SRMSGMOD_T, "mwmask", 0);
	DWORD	dwFlags = M->GetDword(hContact, SRMSGMOD_T, "mwflags", 0);

	bool	fLocal = ((dwMask & MWF_LOG_STATUSCHANGES) ? (dwFlags & MWF_LOG_STATUSCHANGES ? true : false) : false);

	if(fGlobal || fLocal) {
		/*
		 * don't log them if WE are logging off
		 */
		if(CallProtoService(c->getProto(), PS_GETSTATUS, 0, 0) == ID_STATUS_OFFLINE)
			return;

		WORD	wStatus, wOldStatus;

		wStatus = LOWORD(wParam);
		wOldStatus = HIWORD(wParam);

		if(wStatus == wOldStatus)
			return;

		DBEVENTINFO 	dbei;
		TCHAR 			buffer[450];
		HANDLE 			hNewEvent;

		TCHAR*	szOldStatus = (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)wOldStatus, GSMDF_TCHAR);
		TCHAR*	szNewStatus = (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)wStatus, GSMDF_TCHAR);

		if(szOldStatus == 0 || szNewStatus == 0)
			return;

		if (c->isValid()) {
			if (wStatus == ID_STATUS_OFFLINE)
				mir_sntprintf(buffer, safe_sizeof(buffer), CTranslator::get(CTranslator::GEN_MSG_SIGNEDOFF));
			else if (wOldStatus == ID_STATUS_OFFLINE)
				mir_sntprintf(buffer, safe_sizeof(buffer), CTranslator::get(CTranslator::GEN_MSG_SIGNEDON), szNewStatus);
			else
				mir_sntprintf(buffer, safe_sizeof(buffer), CTranslator::get(CTranslator::GEN_MSG_CHANGEDSTATUS), szOldStatus, szNewStatus);
		}

		char *szMsg = M->utf8_encodeT(buffer);

		dbei.pBlob = (PBYTE)szMsg;
		dbei.cbBlob = lstrlenA(szMsg) + 1;
		dbei.flags = DBEF_UTF | DBEF_READ;
		dbei.cbSize = sizeof(dbei);
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.timestamp = time(NULL);
		dbei.szModule = const_cast<char *>(c->getProto());
		hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) hContact, (LPARAM) & dbei);

		mir_free(szMsg);
	}
}

/**
 * when the state of the meta contacts protocol changes from enabled to disabled
 * (or vice versa), this updates the contact cache
 *
 * it is ONLY called from the DBSettingChanged() event handler when the relevant
 * database value is touched.
 */
void CGlobals::cacheUpdateMetaChanged()
{
	CContactCache* 	c = CContactCache::m_cCache;
	bool			fMetaActive = (PluginConfig.g_MetaContactsAvail && PluginConfig.bMetaEnabled) ? true : false;

	while(c) {
		if(c->isMeta() && PluginConfig.bMetaEnabled == false) {
			c->closeWindow();
			c->resetMeta();
		}

		// meta contacts are enabled, but current contact is a subcontact - > close window

		if(fMetaActive && c->isSubContact())
			c->closeWindow();

		// reset meta contact information, if metacontacts protocol became avail

		if(fMetaActive && !strcmp(c->getProto(), PluginConfig.szMetaName))
			c->resetMeta();

		c = c->m_next;
	}
}

/**
 * on Windows 7, when using new task bar features (grouping mode and per tab
 * previews), autoswitching does not work relieably, so it is disabled.
 *
 * @return: true if configuration dictates autoswitch
 */
bool CGlobals::haveAutoSwitch()
{
	if(m_bIsWin7) {
		if(m_useAeroPeek && !CSkin::m_skinEnabled)
			return(false);
	}
	return(m_AutoSwitchTabs ? true : false);
}
/**
 * exception handling - copy error message to clip board
 * @param hWnd: 	window handle of the edit control containing the error message
 */
void CGlobals::Ex_CopyEditToClipboard(HWND hWnd)
{
	SendMessage(hWnd, EM_SETSEL, 0, 65535L);
	SendMessage(hWnd, WM_COPY, 0 , 0);
	SendMessage(hWnd, EM_SETSEL, 0, 0);
}

INT_PTR CALLBACK CGlobals::Ex_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode, wID;

	switch(uMsg) {
		case WM_INITDIALOG: {
				char szBuffer[2048];
#ifdef _WIN64
				sprintf(szBuffer,
						"Exception %16.16X at address %16.16X occured in %s at line %d.\r\n\r\nEAX=%16.16X EBX=%16.16X ECX=%16.16X\r\nEDX=%16.16X ESI=%16.16X EDI=%16.16X\r\nEBP=%16.16X ESP=%16.16X EIP=%16.16X",
						m_exRecord.ExceptionCode, m_exRecord.ExceptionAddress, m_exSzFile, m_exLine,
						m_exCtx.Rax,m_exCtx.Rbx, m_exCtx.Rcx, m_exCtx.Rdx,
						m_exCtx.Rsi, m_exCtx.Rdi, m_exCtx.Rbp, m_exCtx.Rsp, m_exCtx.Rip);
#else
				sprintf(szBuffer,
						"Exception %8.8X at address %8.8X occured in %s at line %d.\r\n\r\nEAX=%8.8X EBX=%8.8X ECX=%8.8X\r\nEDX=%8.8X ESI=%8.8X EDI=%8.8X\r\nEBP=%8.8X ESP=%8.8X EIP=%8.8X",
						m_exRecord.ExceptionCode, m_exRecord.ExceptionAddress, m_exSzFile, m_exLine,
						m_exCtx.Eax,m_exCtx.Ebx, m_exCtx.Ecx, m_exCtx.Edx,
						m_exCtx.Esi, m_exCtx.Edi, m_exCtx.Ebp, m_exCtx.Esp, m_exCtx.Eip);
#endif
				SetDlgItemTextA(hwndDlg, IDC_EXCEPTION_DETAILS, szBuffer);
				SetFocus(GetDlgItem(hwndDlg, IDC_EXCEPTION_DETAILS));
				SendDlgItemMessage(hwndDlg, IDC_EXCEPTION_DETAILS, WM_SETFONT, (WPARAM)GetStockObject(OEM_FIXED_FONT), 0);
				SetDlgItemTextW(hwndDlg, IDC_EX_REASON, m_exReason);
				Utils::enableDlgControl(hwndDlg, IDOK, m_exAllowContinue ? TRUE : FALSE);
			}
			break;

		case WM_COMMAND:
			wNotifyCode = HIWORD(wParam);
			wID = LOWORD(wParam);
			if (wNotifyCode == BN_CLICKED)
			{
				if (wID == IDOK || wID == IDCANCEL)
					EndDialog(hwndDlg, wID);

				if (wID == IDC_COPY_EXCEPTION)
					Ex_CopyEditToClipboard(GetDlgItem(hwndDlg, IDC_EXCEPTION_DETAILS));
			}

			break;
	}
	return FALSE;
}

void CGlobals::Ex_Handler()
{
	if (m_exLastResult == IDCANCEL)
		ExitProcess(1);
}

int CGlobals::Ex_ShowDialog(EXCEPTION_POINTERS *ep, const char *szFile, int line, wchar_t* szReason, bool fAllowContinue)
{
	char	szDrive[MAX_PATH], szDir[MAX_PATH], szName[MAX_PATH], szExt[MAX_PATH];

	_splitpath(szFile, szDrive, szDir, szName, szExt);
	memcpy(&m_exRecord, ep->ExceptionRecord, sizeof(EXCEPTION_RECORD));
	memcpy(&m_exCtx, ep->ContextRecord, sizeof(CONTEXT));

	_snprintf(m_exSzFile, MAX_PATH, "%s%s", szName, szExt);
	mir_sntprintf(m_exReason, 256, L"An application error has occured: %s", szReason);
	m_exLine = line;
	m_exLastResult = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_EXCEPTION), 0, CGlobals::Ex_DlgProc, 0);
	m_exAllowContinue = fAllowContinue;
	if(IDCANCEL == m_exLastResult)
		ExitProcess(1);
	return 1;
}
