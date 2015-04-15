#include "StdAfx.h"
#include "Miranda.h"
#include "CConfig.h"

#include "CAppletManager.h"

#define FONTF_BOLD 1
#define FONTF_ITALIC 2

HANDLE CConfig::m_hOptionsInitHook = NULL;
bool CConfig::m_bInitializingDialog = false;
bool CConfig::m_abBoolSettings[BOOL_SETTINGS];
int CConfig::m_aiIntSettings[INT_SETTINGS];
LOGFONT CConfig::m_logfont[FONT_SETTINGS];
LOGFONT CConfig::m_templogfont[FONT_SETTINGS];
HANDLE CConfig::m_hSampleFont[FONT_SETTINGS];
TEXTMETRIC CConfig::m_tmFontInfo[FONT_SETTINGS];
vector<CProtoFilter*> CConfig::m_ProtoList;
HWND CConfig::hDeviceBox = NULL;

void CConfig::Initialize()
{
	CConfig::LoadSettings();
	m_hOptionsInitHook = HookEvent(ME_OPT_INITIALISE, CConfig::InitOptionsDialog);
	
}

void CConfig::Shutdown()
{
	DeleteObject(m_hSampleFont);
	UnhookEvent(m_hOptionsInitHook);
}

void CConfig::LoadSettings()
{
	m_abBoolSettings[SKIP_DRIVER_ERROR] =				db_get_dw(NULL, "MirandaG15", "SkipDriverError",0) != 0;

	m_abBoolSettings[SCREENSAVER_LOCK] =				db_get_dw(NULL, "MirandaG15", "ScreensaverLock",1) != 0;
	m_abBoolSettings[NOTIFY_SHOWPROTO] =				db_get_dw(NULL, "MirandaG15", "NotifyShowProto",0) != 0;
	m_abBoolSettings[NOTIFY_NICKCUTOFF] =				db_get_dw(NULL, "MirandaG15", "NotifyNickCutoff",0) != 0;
	m_abBoolSettings[NOTIFY_CHANNELCUTOFF] =				db_get_dw(NULL, "MirandaG15", "NotifyChannelCutoff",0) != 0;

	m_abBoolSettings[TRANSITIONS] =				db_get_dw(NULL, "MirandaG15", "Transitions",0) != 0;
	m_abBoolSettings[TIMESTAMP_SECONDS] =		db_get_dw(NULL, "MirandaG15", "TimestampSeconds",0) != 0;
	m_abBoolSettings[SESSION_TIMESTAMPS] =		db_get_dw(NULL, "MirandaG15", "SessionTimestamps",0) != 0;
	m_abBoolSettings[NOTIFY_TIMESTAMPS] =		db_get_dw(NULL, "MirandaG15", "NotifyTimestamps",0) != 0;

	m_abBoolSettings[CONTROL_BACKLIGHTS] =		db_get_dw(NULL, "MirandaG15", "ControlBacklights",0) != 0;
	m_abBoolSettings[HOOK_VOLUMEWHEEL] =		db_get_dw(NULL, "MirandaG15", "HookVolumeWheel",0) != 0;

	m_abBoolSettings[CLIST_SELECTION] =		db_get_dw(NULL, "MirandaG15", "CListSelection",1) != 0;
	m_abBoolSettings[CLIST_COLUMNS] =		db_get_dw(NULL, "MirandaG15", "CListColumns",0) != 0;
	m_abBoolSettings[CLIST_HIDEOFFLINE] =	db_get_dw(NULL, "MirandaG15", "CListHideOffline",1) != 0;
	m_abBoolSettings[CLIST_USEIGNORE] =		db_get_dw(NULL, "MirandaG15", "CListUseIgnore",1) != 0;
	m_abBoolSettings[CLIST_USEGROUPS] =		db_get_dw(NULL, "MirandaG15", "CListUseGroups",1) != 0;
	m_abBoolSettings[CLIST_SHOWPROTO] =		db_get_dw(NULL, "MirandaG15", "CListShowProto",1) != 0;
	m_abBoolSettings[CLIST_DRAWLINES] =		db_get_dw(NULL, "MirandaG15", "CListDrawLines",1) != 0;
	m_abBoolSettings[CLIST_COUNTERS] =		db_get_dw(NULL, "MirandaG15", "CListCounters",1) != 0;
	m_abBoolSettings[CLIST_POSITION] =		db_get_dw(NULL, "MirandaG15", "CListPosition",0) != 0;

	m_abBoolSettings[NOTIFY_IRC_MESSAGES] =		db_get_dw(NULL, "MirandaG15", "NotifyIRCMessages",	1) != 0;
	m_abBoolSettings[NOTIFY_IRC_USERS] =		db_get_dw(NULL, "MirandaG15", "NotifyIRCUsers",		0) != 0;
	m_abBoolSettings[NOTIFY_IRC_EMOTES] =		db_get_dw(NULL, "MirandaG15", "NotifyIRCEmotes",		0) != 0;
	m_abBoolSettings[NOTIFY_IRC_NOTICES] =		db_get_dw(NULL, "MirandaG15", "NotifyIRCNotices",	1) != 0;
	m_abBoolSettings[NOTIFY_IRC_CHANNEL] =		db_get_dw(NULL, "MirandaG15", "NotifyIRCChannel",	0) != 0;
	m_abBoolSettings[NOTIFY_IRC_STATUS] =		db_get_dw(NULL, "MirandaG15", "NotifyIRCStatus",		0) != 0;

	m_abBoolSettings[NOTIFY_PROTO_STATUS] =	db_get_dw(NULL, "MirandaG15", "NotifyProtoStatus",	0) != 0;
	m_abBoolSettings[NOTIFY_PROTO_SIGNON] =	db_get_dw(NULL, "MirandaG15", "NotifyProtoSignOn",	1) != 0;
	m_abBoolSettings[NOTIFY_PROTO_SIGNOFF] =db_get_dw(NULL, "MirandaG15", "NotifyProtoSignOff",	1) != 0;
	m_abBoolSettings[NOTIFY_MESSAGES] =		db_get_dw(NULL, "MirandaG15", "NotifyMessages",		1) != 0;
	m_abBoolSettings[NOTIFY_SIGNON] =		db_get_dw(NULL, "MirandaG15", "NotifySignOn",		1) != 0;
	m_abBoolSettings[NOTIFY_SIGNOFF] =		db_get_dw(NULL, "MirandaG15", "NotifySignOff",		1) != 0;
	m_abBoolSettings[NOTIFY_STATUS] =		db_get_dw(NULL, "MirandaG15", "NotifyStatus",		0) != 0;
	m_abBoolSettings[NOTIFY_SKIP_MESSAGES] =db_get_dw(NULL, "MirandaG15", "NotifySkipMessages",	1) != 0;
	m_abBoolSettings[NOTIFY_SKIP_SIGNON] =	db_get_dw(NULL, "MirandaG15", "NotifySkipSignOn",	0) != 0;
	m_abBoolSettings[NOTIFY_SKIP_SIGNOFF] =	db_get_dw(NULL, "MirandaG15", "NotifySkipSignOff",	0) != 0;
	m_abBoolSettings[NOTIFY_SKIP_STATUS] =	db_get_dw(NULL, "MirandaG15", "NotifySkipStatus",	1) != 0;
	m_abBoolSettings[NOTIFY_NO_SKIP_REPLY] =	db_get_dw(NULL, "MirandaG15", "NotifyNoSkipReply",1) != 0;
	m_abBoolSettings[NOTIFY_URL] =			db_get_dw(NULL, "MirandaG15", "NotifyURL",			1) != 0;
	m_abBoolSettings[NOTIFY_FILE] =			db_get_dw(NULL, "MirandaG15", "NotifyFile",			1) != 0;
	m_abBoolSettings[NOTIFY_CONTACTS] =		db_get_dw(NULL, "MirandaG15", "NotifyContacts",		1) != 0;
	
	m_abBoolSettings[SESSION_SCROLL_MAXIMIZED] = db_get_dw(NULL, "MirandaG15", "SessionScrollMaximized",	1) != 0;
	m_abBoolSettings[SESSION_REPLY_MAXIMIZED] = db_get_dw(NULL, "MirandaG15", "SessionReplyMaximized",	1) != 0;
	m_abBoolSettings[SESSION_LOADDB] =		db_get_dw(NULL, "MirandaG15", "SessionLoadDB",		0) != 0;
	m_abBoolSettings[SESSION_MARKREAD] =	db_get_dw(NULL, "MirandaG15", "SessionMarkRead",	1) != 0;
	m_abBoolSettings[SESSION_SENDRETURN] =	db_get_dw(NULL, "MirandaG15", "SessionSendReturn",0) != 0;
	m_abBoolSettings[SESSION_SHOWTYPING] =	db_get_dw(NULL, "MirandaG15", "SessionShowTyping",1) != 0;
	m_abBoolSettings[SESSION_SENDTYPING] =	db_get_dw(NULL, "MirandaG15", "SessionSendTyping",1) != 0;
	m_abBoolSettings[SESSION_SYMBOLS] =	db_get_dw(NULL, "MirandaG15", "SessionSymbols",0) != 0;
	m_abBoolSettings[SESSION_CLOSE] =	db_get_dw(NULL, "MirandaG15", "SessionClose",1) != 0;
	
	m_abBoolSettings[SHOW_LABELS] = db_get_dw(NULL, "MirandaG15", "ShowLabels",				1) != 0;
	m_abBoolSettings[MAXIMIZED_TITLE] = db_get_dw(NULL, "MirandaG15", "MaximizedTitle",		0) != 0;
	m_abBoolSettings[MAXIMIZED_LABELS] = db_get_dw(NULL, "MirandaG15", "MaximizedLabels",	1) != 0;

	m_aiIntSettings[NOTIFY_CHANNELCUTOFF_OFFSET] = db_get_dw(NULL, "MirandaG15", "NotifyChannelCutoffOffset", 10);
	m_aiIntSettings[NOTIFY_NICKCUTOFF_OFFSET] = db_get_dw(NULL, "MirandaG15", "NotifyNickCutoffOffset", 10);
	m_aiIntSettings[NOTIFY_DURATION] = db_get_dw(NULL, "MirandaG15", "NotifyDuration",		4);
	m_aiIntSettings[NOTIFY_LOGSIZE] = db_get_dw(NULL, "MirandaG15", "NotifyLogSize",			10);
	m_aiIntSettings[SESSION_LOGSIZE] = db_get_dw(NULL, "MirandaG15", "SessionLogSize",		10);
	m_aiIntSettings[SESSION_AUTOSCROLL] = db_get_dw(NULL, "MirandaG15", "SessionAutoScroll",	SESSION_AUTOSCROLL_FIRST);
	m_aiIntSettings[SESSION_CLOSETIMER] = db_get_dw(NULL, "MirandaG15", "SessionCloseTimer",	15);
	m_aiIntSettings[NOTIFY_TITLE] = db_get_dw(NULL, "MirandaG15", "NotifyTitle",			NOTIFY_TITLE_NAME);
	m_aiIntSettings[CLIST_GA] =	db_get_dw(NULL, "MirandaG15", "CListGA",CLIST_GA_NONE);
	m_aiIntSettings[DEVICE] =	db_get_dw(NULL, "MirandaG15", "Device",0);
	
	for(int i=0;i<FONT_SETTINGS;i++)
		LoadFontSettings(i);

	// Load protocol specific settings
	int iCount;
	int iProtoCount = 0;
	PROTOACCOUNT **ppAccounts;
	CProtoFilter *pProtoFilter;
	tstring strSetting;

	CallService(MS_PROTO_ENUMACCOUNTS,(WPARAM)&iCount,(LPARAM)&ppAccounts);
	for(int i=0;i<iCount;i++)
	{
		/*if(ppProtocolDescriptor[i]->type != PROTOTYPE_PROTOCOL)
			continue;*/
		if (ppAccounts[i]->bIsEnabled == 0)
			continue;

		iProtoCount++;
		pProtoFilter = new CProtoFilter();
		pProtoFilter->strName = toTstring(ppAccounts[i]->szModuleName);
		
		strSetting = _T("ProtoFilter_") + pProtoFilter->strName;
		pProtoFilter->bNotificationFilter = db_get_dw(NULL,"MirandaG15",toNarrowString(strSetting).c_str(),1) != 0;
		strSetting = _T("ProtoCListFilter_") + pProtoFilter->strName;
		pProtoFilter->bContactlistFilter = db_get_dw(NULL,"MirandaG15",toNarrowString(strSetting).c_str(),1) != 0;
		m_ProtoList.push_back(pProtoFilter);
	}
}

void CConfig::SaveSettings()
{
	db_set_dw(NULL, "MirandaG15", "SkipDriverError",	m_abBoolSettings[SKIP_DRIVER_ERROR]);

	db_set_dw(NULL, "MirandaG15", "ScreensaverLock",	m_abBoolSettings[SCREENSAVER_LOCK]);
	db_set_dw(NULL, "MirandaG15", "NotifyShowProto",	m_abBoolSettings[NOTIFY_SHOWPROTO]);
	db_set_dw(NULL, "MirandaG15", "NotifyChannelCutoff",	m_abBoolSettings[NOTIFY_CHANNELCUTOFF]);
	db_set_dw(NULL, "MirandaG15", "NotifyNickCutoff",	m_abBoolSettings[NOTIFY_NICKCUTOFF]);
	db_set_dw(NULL, "MirandaG15", "Transitions",	m_abBoolSettings[TRANSITIONS]);
	db_set_dw(NULL, "MirandaG15", "TimestampSeconds",	m_abBoolSettings[TIMESTAMP_SECONDS]);
	db_set_dw(NULL, "MirandaG15", "SessionTimestamps",	m_abBoolSettings[SESSION_TIMESTAMPS]);
	db_set_dw(NULL, "MirandaG15", "NotifyTimestamps",	m_abBoolSettings[NOTIFY_TIMESTAMPS]);

	db_set_dw(NULL, "MirandaG15", "ControlBacklights",	m_abBoolSettings[CONTROL_BACKLIGHTS]);
	db_set_dw(NULL, "MirandaG15", "HookVolumeWheel",	m_abBoolSettings[HOOK_VOLUMEWHEEL]);
	
	db_set_dw(NULL, "MirandaG15", "CListSelection",	m_abBoolSettings[CLIST_SELECTION]);
	db_set_dw(NULL, "MirandaG15", "CListColumns",		m_abBoolSettings[CLIST_COLUMNS]);
	db_set_dw(NULL, "MirandaG15", "CListHideOffline",	m_abBoolSettings[CLIST_HIDEOFFLINE]);
	db_set_dw(NULL, "MirandaG15", "CListUseIgnore",	m_abBoolSettings[CLIST_USEIGNORE]);
	db_set_dw(NULL, "MirandaG15", "CListUseGroups",	m_abBoolSettings[CLIST_USEGROUPS]);
	db_set_dw(NULL, "MirandaG15", "CListShowProto",	m_abBoolSettings[CLIST_SHOWPROTO]);
	db_set_dw(NULL, "MirandaG15", "CListDrawLines",	m_abBoolSettings[CLIST_DRAWLINES]);
	db_set_dw(NULL, "MirandaG15", "CListCounters",		m_abBoolSettings[CLIST_COUNTERS]);
	db_set_dw(NULL, "MirandaG15", "CListPosition",		m_abBoolSettings[CLIST_POSITION]);
	
	db_set_dw(NULL, "MirandaG15", "NotifyIRCMessages",	m_abBoolSettings[NOTIFY_IRC_MESSAGES]);
	db_set_dw(NULL, "MirandaG15", "NotifyIRCUsers",	m_abBoolSettings[NOTIFY_IRC_USERS]);
	db_set_dw(NULL, "MirandaG15", "NotifyIRCEmotes",	m_abBoolSettings[NOTIFY_IRC_EMOTES]);
	db_set_dw(NULL, "MirandaG15", "NotifyIRCNotices",	m_abBoolSettings[NOTIFY_IRC_NOTICES]);
	db_set_dw(NULL, "MirandaG15", "NotifyIRCChannel",	m_abBoolSettings[NOTIFY_IRC_CHANNEL]);
	db_set_dw(NULL, "MirandaG15", "NotifyIRCStatus",	m_abBoolSettings[NOTIFY_IRC_STATUS]);

	db_set_dw(NULL, "MirandaG15", "NotifyProtoStatus",	m_abBoolSettings[NOTIFY_PROTO_STATUS]);
	db_set_dw(NULL, "MirandaG15", "NotifyProtoSignOn",	m_abBoolSettings[NOTIFY_PROTO_SIGNON]);
	db_set_dw(NULL, "MirandaG15", "NotifyProtoSignOff",m_abBoolSettings[NOTIFY_PROTO_SIGNOFF]);
	db_set_dw(NULL, "MirandaG15", "NotifyMessages",	m_abBoolSettings[NOTIFY_MESSAGES]);
	db_set_dw(NULL, "MirandaG15", "NotifySignOn",		m_abBoolSettings[NOTIFY_SIGNON]);
	db_set_dw(NULL, "MirandaG15", "NotifySignOff",		m_abBoolSettings[NOTIFY_SIGNOFF]);
	db_set_dw(NULL, "MirandaG15", "NotifyStatus",		m_abBoolSettings[NOTIFY_STATUS]);
	db_set_dw(NULL, "MirandaG15", "NotifySkipMessages",	m_abBoolSettings[NOTIFY_SKIP_MESSAGES]);
	db_set_dw(NULL, "MirandaG15", "NotifySkipSignOn",		m_abBoolSettings[NOTIFY_SKIP_SIGNON]);
	db_set_dw(NULL, "MirandaG15", "NotifySkipSignOff",		m_abBoolSettings[NOTIFY_SKIP_SIGNOFF]);
	db_set_dw(NULL, "MirandaG15", "NotifySkipStatus",		m_abBoolSettings[NOTIFY_SKIP_STATUS]);
	db_set_dw(NULL, "MirandaG15", "NotifyNoSkipReply",		m_abBoolSettings[NOTIFY_NO_SKIP_REPLY]);
	db_set_dw(NULL, "MirandaG15", "NotifyURL",			m_abBoolSettings[NOTIFY_URL]);
	db_set_dw(NULL, "MirandaG15", "NotifyFile",		m_abBoolSettings[NOTIFY_FILE]);
	db_set_dw(NULL, "MirandaG15", "NotifyContacts",	m_abBoolSettings[NOTIFY_CONTACTS]);
	
	db_set_dw(NULL, "MirandaG15", "SessionScrollMaximized",m_abBoolSettings[SESSION_SCROLL_MAXIMIZED]);
	db_set_dw(NULL, "MirandaG15", "SessionReplyMaximized",m_abBoolSettings[SESSION_REPLY_MAXIMIZED]);
	db_set_dw(NULL, "MirandaG15", "SessionShowTyping",	m_abBoolSettings[SESSION_SHOWTYPING]);
	db_set_dw(NULL, "MirandaG15", "SessionSendTyping",	m_abBoolSettings[SESSION_SENDTYPING]);
	db_set_dw(NULL, "MirandaG15", "SessionLoadDB",		m_abBoolSettings[SESSION_LOADDB]);
	db_set_dw(NULL, "MirandaG15", "SessionSendReturn",	m_abBoolSettings[SESSION_SENDRETURN]);
	db_set_dw(NULL, "MirandaG15", "SessionMarkRead",	m_abBoolSettings[SESSION_MARKREAD]);
	db_set_dw(NULL, "MirandaG15", "SessionSymbols",	m_abBoolSettings[SESSION_SYMBOLS]);
	db_set_dw(NULL, "MirandaG15", "SessionClose",		m_abBoolSettings[SESSION_CLOSE]);

	db_set_dw(NULL, "MirandaG15", "ShowLabels",		m_abBoolSettings[SHOW_LABELS]);
	db_set_dw(NULL, "MirandaG15", "MaximizedTitle",	m_abBoolSettings[MAXIMIZED_TITLE]);
	db_set_dw(NULL, "MirandaG15", "MaximizedLabels",	m_abBoolSettings[MAXIMIZED_LABELS]);

	db_set_dw(NULL, "MirandaG15", "NotifyChannelCutoffOffset",	m_aiIntSettings[NOTIFY_CHANNELCUTOFF_OFFSET]);
	db_set_dw(NULL, "MirandaG15", "NotifyNickCutoffOffset",	m_aiIntSettings[NOTIFY_NICKCUTOFF_OFFSET]);
	db_set_dw(NULL, "MirandaG15", "NotifyDuration",	m_aiIntSettings[NOTIFY_DURATION]);
	db_set_dw(NULL, "MirandaG15", "NotifyLogSize",		m_aiIntSettings[NOTIFY_LOGSIZE]);
	db_set_dw(NULL, "MirandaG15", "SessionLogSize",	m_aiIntSettings[SESSION_LOGSIZE]);
	db_set_dw(NULL, "MirandaG15", "NotifyTitle",		m_aiIntSettings[NOTIFY_TITLE]);
	db_set_dw(NULL, "MirandaG15", "SessionAutoScroll",	m_aiIntSettings[SESSION_AUTOSCROLL]);
	db_set_dw(NULL, "MirandaG15", "SessionCloseTimer",	m_aiIntSettings[SESSION_CLOSETIMER]);
	db_set_dw(NULL, "MirandaG15", "CListGA",	m_aiIntSettings[CLIST_GA]);
	db_set_dw(NULL, "MirandaG15", "Device",	m_aiIntSettings[DEVICE]);

	// Save font settings
	for(int i=0;i<FONT_SETTINGS;i++)
		SaveFontSettings(i);

	// save protocol specific settings
	vector<CProtoFilter*>::iterator iter = m_ProtoList.begin();
	tstring strSetting = _T("");
	while(iter != m_ProtoList.end())
	{
		strSetting = _T("ProtoFilter_") + (*iter)->strName;
		db_set_dw(NULL, "MirandaG15", toNarrowString(strSetting).c_str(),(*iter)->bNotificationFilter);
		strSetting = _T("ProtoCListFilter_") + (*iter)->strName;
		db_set_dw(NULL, "MirandaG15", toNarrowString(strSetting).c_str(),(*iter)->bContactlistFilter);
		iter++;
	}

	// Notify OutputManager of configuration change
	if(CAppletManager::GetInstance()->IsInitialized())
		CAppletManager::GetInstance()->OnConfigChanged();
}

void CConfig::UpdateFontSettings(int iFont)
{
	if(m_hSampleFont[iFont])
		DeleteObject(m_hSampleFont[iFont]);
	m_hSampleFont[iFont] =  CreateFontIndirect(&m_logfont[iFont]);
	HDC hDC = CreateCompatibleDC(NULL);
	SelectObject(hDC, m_hSampleFont[iFont]);   
	GetTextMetrics(hDC,&m_tmFontInfo[iFont]);
	DeleteObject(hDC);
}

void CConfig::OnConnectionChanged() {
	FillDeviceBox(CConfig::hDeviceBox);
}

int CConfig::InitOptionsDialog(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position    = 847000000;
	odp.hInstance   = hInstance;
	odp.pszGroup    = LPGEN("MirandaG15");
	odp.flags       = ODPF_BOLDGROUPS;

	// ---------------------
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_FONTS);
	odp.pszTitle    = LPGEN("Appearance");
	odp.pszGroup    = LPGEN("MirandaG15");
	odp.pfnDlgProc  = CConfig::AppearanceDlgProc;
	Options_AddPage(wParam, &odp);
	
	// ---------------------
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_NOTIFICATIONS);
	odp.pszTitle    = LPGEN("Notifications");
	odp.pfnDlgProc  = CConfig::NotificationsDlgProc;
	Options_AddPage(wParam, &odp);

	// ---------------------
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_CHAT);
	odp.pszTitle    = LPGEN("Chat sessions");
	odp.pfnDlgProc  = CConfig::ChatDlgProc;
	Options_AddPage(wParam, &odp);

	// ---------------------
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_CLIST);
	odp.pszTitle    = LPGEN("Contact list");
	odp.pfnDlgProc  = CConfig::ContactlistDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

void CConfig::SaveFontSettings(int iFont)
{
	if(iFont > FONT_SETTINGS -1 || iFont < 0)
		return;

	char szSetting[128];

	// Height
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dHeight",iFont);
	db_set_b(NULL, "MirandaG15", szSetting, m_logfont[iFont].lfHeight);
	// Style
	int style = 0;
	if(m_logfont[iFont].lfWeight == FW_BOLD)
		style |= FONTF_BOLD;
	if(m_logfont[iFont].lfItalic)
		style |= FONTF_ITALIC;
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dStyle",iFont);
	db_set_b(NULL, "MirandaG15", szSetting, style);
	// Charset
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dCharset",iFont);
	db_set_b(NULL, "MirandaG15", szSetting, m_logfont[iFont].lfCharSet);
	// Name
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dName",iFont);
	db_set_ts(NULL, "MirandaG15", szSetting, m_logfont[iFont].lfFaceName);
	
	UpdateFontSettings(iFont);
}

void CConfig::LoadFontSettings(int iFont)
{
	if(iFont > FONT_SETTINGS -1 || iFont < 0)
		return;

	char szSetting[128];
	
	// Fixed Values
	m_logfont[iFont].lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_logfont[iFont].lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_logfont[iFont].lfQuality = DEFAULT_QUALITY;
	m_logfont[iFont].lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	// Height
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dHeight",iFont);
	m_logfont[iFont].lfHeight = (char) db_get_b(NULL, "MirandaG15", szSetting, -MulDiv(6, 96, 72));
	// Style
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dStyle",iFont);
	int style = db_get_b(NULL, "MirandaG15", szSetting, 0);
	m_logfont[iFont].lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
    m_logfont[iFont].lfItalic = style & FONTF_ITALIC ? 1 : 0;
	// Charset
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dCharset",iFont);
	m_logfont[iFont].lfCharSet = db_get_b(NULL, "MirandaG15", szSetting, DEFAULT_CHARSET);
	// Name
	mir_snprintf(szSetting,SIZEOF(szSetting),"Font%dName",iFont);
	DBVARIANT dbv;
	if (db_get_ts(NULL, "MirandaG15", szSetting, &dbv))
        mir_tstrcpy(m_logfont[iFont].lfFaceName,_T("Small Fonts"));
    else {
		mir_tstrcpy(m_logfont[iFont].lfFaceName, dbv.ptszVal);
       db_free(&dbv);
    }

	UpdateFontSettings(iFont);
}

LOGFONT &CConfig::GetFont(int iFont)
{
	return m_logfont[iFont];
}

int CConfig::GetFontHeight(int iFont)
{
	return m_tmFontInfo[iFont].tmHeight;
}

int CConfig::GetSampleField(int iFont)
{
	switch(iFont)
	{
	case 0: return IDC_SAMPLE1;
	case 1: return IDC_SAMPLE2;
	case 2: return IDC_SAMPLE3;
	case 3: return IDC_SAMPLE4;
	case 4: return IDC_SAMPLE5;
	};
	return 0;
}

INT_PTR CALLBACK CConfig::AppearanceDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			m_bInitializingDialog = true;
			
			// Translate the dialog
			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_SKIP_DRIVER_ERROR,		m_abBoolSettings[SKIP_DRIVER_ERROR]	? BST_CHECKED	: BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_SCREENSAVER_LOCK,		m_abBoolSettings[SCREENSAVER_LOCK]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANSITIONS,		m_abBoolSettings[TRANSITIONS]		? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONTROL_BACKLIGHTS,		m_abBoolSettings[CONTROL_BACKLIGHTS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOW_LABELS,			m_abBoolSettings[SHOW_LABELS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_HOOK_VOLUMEWHEEL,		m_abBoolSettings[HOOK_VOLUMEWHEEL]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TIMESTAMP_SECONDS,		m_abBoolSettings[TIMESTAMP_SECONDS]	? BST_CHECKED	: BST_UNCHECKED);
			
			for(int i=0;i<FONT_SETTINGS;i++)
			{
				m_templogfont[i] = m_logfont[i];
				SendDlgItemMessage(hwndDlg,CConfig::GetSampleField(i),WM_SETFONT,(WPARAM)m_hSampleFont[i],(LPARAM)true);
			}
			
			// fill device box
			CConfig::hDeviceBox = GetDlgItem(hwndDlg,IDC_DEVICE);
			FillDeviceBox(CConfig::hDeviceBox);
			
			m_bInitializingDialog = false;
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					m_abBoolSettings[SKIP_DRIVER_ERROR] =  IsDlgButtonChecked(hwndDlg,IDC_SKIP_DRIVER_ERROR) == BST_CHECKED ? true : false;

					m_abBoolSettings[SCREENSAVER_LOCK] =  IsDlgButtonChecked(hwndDlg,IDC_SCREENSAVER_LOCK) == BST_CHECKED ? true : false;
					m_abBoolSettings[TRANSITIONS] =  IsDlgButtonChecked(hwndDlg,IDC_TRANSITIONS) == BST_CHECKED ? true : false;
					m_abBoolSettings[SHOW_LABELS] =  IsDlgButtonChecked(hwndDlg,IDC_SHOW_LABELS) == BST_CHECKED ? true : false;
					m_abBoolSettings[CONTROL_BACKLIGHTS] =  IsDlgButtonChecked(hwndDlg,IDC_CONTROL_BACKLIGHTS) == BST_CHECKED ? true : false;
					m_abBoolSettings[HOOK_VOLUMEWHEEL] =  IsDlgButtonChecked(hwndDlg,IDC_HOOK_VOLUMEWHEEL) == BST_CHECKED ? true : false;
					m_abBoolSettings[TIMESTAMP_SECONDS] =  IsDlgButtonChecked(hwndDlg,IDC_TIMESTAMP_SECONDS) == BST_CHECKED ? true : false;
					m_aiIntSettings[DEVICE] = SendDlgItemMessage(hwndDlg,IDC_DEVICE,CB_GETCURSEL ,0,0);

					for(int i=0;i<FONT_SETTINGS;i++)
						m_logfont[i] = m_templogfont[i];

					CConfig::SaveSettings();
					SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
					break;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDC_CREDITS)
			{
				CAppletManager::GetInstance()->ActivateCreditsScreen();
				break;
			}

			int iFont = LOWORD(wParam);
			switch(iFont)
			{
			case IDC_CHOOSEFONT1: iFont = 0; break;
			case IDC_CHOOSEFONT2: iFont = 1; break;
			case IDC_CHOOSEFONT3: iFont = 2; break;
			case IDC_CHOOSEFONT4: iFont = 3; break;
			case IDC_CHOOSEFONT5: iFont = 4; break;
			default: iFont = -1;
			};

			if(iFont >= 0)
			{
				CHOOSEFONT cf = { 0 };
				cf.lStructSize = sizeof(cf);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &m_templogfont[iFont];
				cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
				if (ChooseFont(&cf))
				{
					if(m_hSampleFont[iFont])
						DeleteObject(m_hSampleFont[iFont]);
					m_hSampleFont[iFont] =  CreateFontIndirect(&m_templogfont[iFont]);
					SendDlgItemMessage(hwndDlg,CConfig::GetSampleField(iFont),WM_SETFONT,(WPARAM)m_hSampleFont[iFont],(LPARAM)true);
					TRACE(_T("Font selected!"));
				}
			}
			if(LOWORD(wParam) == IDC_DEVICE && SendDlgItemMessage(hwndDlg,IDC_DEVICE,CB_GETCURSEL ,0,0) != m_aiIntSettings[DEVICE]) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}

			// Activate the apply button
			if (!m_bInitializingDialog && ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED))) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		}
	}
	return 0;
}

INT_PTR CALLBACK CConfig::ChatDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[128];

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			m_bInitializingDialog = true;
			
			// Translate the dialog
			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_MAXIMIZED_TITLE,		m_abBoolSettings[MAXIMIZED_TITLE]	? BST_UNCHECKED	: BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_MAXIMIZED_LABELS,		m_abBoolSettings[MAXIMIZED_LABELS]	? BST_UNCHECKED	: BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_REPLY_MAXIMIZED,	m_abBoolSettings[SESSION_REPLY_MAXIMIZED]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_SCROLL_MAXIMIZED,	m_abBoolSettings[SESSION_SCROLL_MAXIMIZED]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_SHOWTYPING,		m_abBoolSettings[SESSION_SHOWTYPING]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_SENDTYPING,		m_abBoolSettings[SESSION_SENDTYPING]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_LOADDB,		m_abBoolSettings[SESSION_LOADDB]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_MARKREAD,		m_abBoolSettings[SESSION_MARKREAD]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_SYMBOLS,		m_abBoolSettings[SESSION_SYMBOLS]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_TIMESTAMPS,		m_abBoolSettings[SESSION_TIMESTAMPS]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_CLOSE,		m_abBoolSettings[SESSION_CLOSE]? BST_CHECKED	: BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_SESSION_SENDRETURN,		m_abBoolSettings[SESSION_SENDRETURN]? BST_CHECKED	: BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_SESSION_SCROLLNONE,		m_aiIntSettings[SESSION_AUTOSCROLL] == SESSION_AUTOSCROLL_NONE? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_SCROLLFIRST,	m_aiIntSettings[SESSION_AUTOSCROLL] == SESSION_AUTOSCROLL_FIRST? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SESSION_SCROLLLAST,		m_aiIntSettings[SESSION_AUTOSCROLL] == SESSION_AUTOSCROLL_LAST? BST_CHECKED	: BST_UNCHECKED);
			
			_itoa_s(m_aiIntSettings[SESSION_LOGSIZE], buf, SIZEOF(buf), 10);
			SetDlgItemTextA(hwndDlg, IDC_SESSION_LOGSIZE, buf);
			
			_itoa_s(m_aiIntSettings[SESSION_CLOSETIMER], buf, SIZEOF(buf), 10);
			SetDlgItemTextA(hwndDlg, IDC_SESSION_CLOSETIMER, buf);

			m_bInitializingDialog = false;
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					m_abBoolSettings[SESSION_SCROLL_MAXIMIZED] =	IsDlgButtonChecked(hwndDlg,IDC_SESSION_SCROLL_MAXIMIZED) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_REPLY_MAXIMIZED] =		IsDlgButtonChecked(hwndDlg,IDC_SESSION_REPLY_MAXIMIZED) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_SHOWTYPING] =			IsDlgButtonChecked(hwndDlg,IDC_SESSION_SHOWTYPING) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_SENDTYPING] =			IsDlgButtonChecked(hwndDlg,IDC_SESSION_SENDTYPING) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_LOADDB] =				IsDlgButtonChecked(hwndDlg,IDC_SESSION_LOADDB) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_SENDRETURN] =			IsDlgButtonChecked(hwndDlg,IDC_SESSION_SENDRETURN) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_MARKREAD] =			IsDlgButtonChecked(hwndDlg,IDC_SESSION_MARKREAD) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_SYMBOLS] =				IsDlgButtonChecked(hwndDlg,IDC_SESSION_SYMBOLS) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_TIMESTAMPS] =			IsDlgButtonChecked(hwndDlg,IDC_SESSION_TIMESTAMPS) == BST_CHECKED ? true : false;
					m_abBoolSettings[SESSION_CLOSE] =				IsDlgButtonChecked(hwndDlg,IDC_SESSION_CLOSE) == BST_CHECKED ? true : false;

					if(IsDlgButtonChecked(hwndDlg,IDC_SESSION_SCROLLNONE) == BST_CHECKED)
						m_aiIntSettings[SESSION_AUTOSCROLL] = SESSION_AUTOSCROLL_NONE;
					else if(IsDlgButtonChecked(hwndDlg,IDC_SESSION_SCROLLFIRST) == BST_CHECKED)
						m_aiIntSettings[SESSION_AUTOSCROLL] = SESSION_AUTOSCROLL_FIRST;
					else
						m_aiIntSettings[SESSION_AUTOSCROLL] = SESSION_AUTOSCROLL_LAST;

					m_abBoolSettings[MAXIMIZED_TITLE] =  IsDlgButtonChecked(hwndDlg,IDC_MAXIMIZED_TITLE) == BST_UNCHECKED ? true : false;
					m_abBoolSettings[MAXIMIZED_LABELS] =  IsDlgButtonChecked(hwndDlg,IDC_MAXIMIZED_LABELS) == BST_UNCHECKED ? true : false;

					GetDlgItemTextA(hwndDlg,IDC_SESSION_LOGSIZE,buf,SIZEOF(buf));
					m_aiIntSettings[SESSION_LOGSIZE] = atoi(buf) > 0 ? atoi(buf):1;
					
					GetDlgItemTextA(hwndDlg,IDC_SESSION_CLOSETIMER,buf,SIZEOF(buf));
					m_aiIntSettings[SESSION_CLOSETIMER] = atoi(buf) >= 0 ? atoi(buf):1;

					CConfig::SaveSettings();
					SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
					break;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			// Activate the apply button
			if (!m_bInitializingDialog && ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
	}
	return 0;
}

INT_PTR CALLBACK CConfig::NotificationsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[128];

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			m_bInitializingDialog = true;

			// Translate the dialog
			TranslateDialogDefault(hwndDlg);

			// Initialize the protocol filter list
			SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_PROTOCOLS),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_PROTOCOLS),GWL_STYLE)|TVS_NOHSCROLL);
			int iRes = 0;
			
			HIMAGELIST himlCheckBoxes;
			himlCheckBoxes=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,2,2);
			iRes = ImageList_AddIcon(himlCheckBoxes,LoadIcon(hInstance,MAKEINTRESOURCE(IDI_NOTICK)));
			iRes = ImageList_AddIcon(himlCheckBoxes,LoadIcon(hInstance,MAKEINTRESOURCE(IDI_TICK)));
			TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_PROTOCOLS),himlCheckBoxes,TVSIL_NORMAL);

			FillTree(GetDlgItem(hwndDlg,IDC_PROTOCOLS));
		
			CheckDlgButton(hwndDlg, IDC_NOTIFY_TIMESTAMPS,		m_abBoolSettings[NOTIFY_TIMESTAMPS]			? BST_CHECKED	: BST_UNCHECKED);
		
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SHOWPROTO,		m_abBoolSettings[NOTIFY_SHOWPROTO]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_CHANNELCUTOFF,	m_abBoolSettings[NOTIFY_CHANNELCUTOFF]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_NICKCUTOFF,		m_abBoolSettings[NOTIFY_NICKCUTOFF]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_IRC_MESSAGES,	m_abBoolSettings[NOTIFY_IRC_MESSAGES]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_IRC_USERS,		m_abBoolSettings[NOTIFY_IRC_USERS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_IRC_EMOTES,		m_abBoolSettings[NOTIFY_IRC_EMOTES]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_IRC_NOTICES,		m_abBoolSettings[NOTIFY_IRC_NOTICES]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_IRC_CHANNEL,		m_abBoolSettings[NOTIFY_IRC_CHANNEL]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_IRC_STATUS,		m_abBoolSettings[NOTIFY_IRC_STATUS]	? BST_CHECKED	: BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_NOTIFY_PROTO_STATUS,	m_abBoolSettings[NOTIFY_PROTO_STATUS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_PROTO_SIGNON,	m_abBoolSettings[NOTIFY_PROTO_SIGNON]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_PROTO_SIGNOFF,	m_abBoolSettings[NOTIFY_PROTO_SIGNOFF]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SIGNON,		m_abBoolSettings[NOTIFY_SIGNON]		? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SIGNOFF,		m_abBoolSettings[NOTIFY_SIGNOFF]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_STATUS,		m_abBoolSettings[NOTIFY_STATUS]		? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_MESSAGES,		m_abBoolSettings[NOTIFY_MESSAGES]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_FILE,		m_abBoolSettings[NOTIFY_FILE]		? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_CONTACTS,		m_abBoolSettings[NOTIFY_CONTACTS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_URL,			m_abBoolSettings[NOTIFY_URL]		? BST_CHECKED	: BST_UNCHECKED);
		
			CheckDlgButton(hwndDlg, IDC_NOTIFY_NO_SKIP_REPLY,	m_abBoolSettings[NOTIFY_NO_SKIP_REPLY]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SKIP_SIGNON,		m_abBoolSettings[NOTIFY_SKIP_SIGNON]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SKIP_SIGNOFF,	m_abBoolSettings[NOTIFY_SKIP_SIGNOFF]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SKIP_STATUS,		m_abBoolSettings[NOTIFY_SKIP_STATUS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_SKIP_MESSAGES,	m_abBoolSettings[NOTIFY_SKIP_MESSAGES]	? BST_CHECKED	: BST_UNCHECKED);
			
			_itoa_s(m_aiIntSettings[NOTIFY_CHANNELCUTOFF_OFFSET], buf, SIZEOF(buf), 10);
			SetDlgItemTextA(hwndDlg, IDC_NOTIFY_CHANNELCUTOFF_OFFSET, buf);

			_itoa_s(m_aiIntSettings[NOTIFY_NICKCUTOFF_OFFSET], buf, SIZEOF(buf), 10);
			SetDlgItemTextA(hwndDlg, IDC_NOTIFY_NICKCUTOFF_OFFSET, buf);

			_itoa_s(m_aiIntSettings[NOTIFY_LOGSIZE], buf, SIZEOF(buf), 10);
			SetDlgItemTextA(hwndDlg, IDC_NOTIFY_LOGSIZE, buf);
			
			_itoa_s(m_aiIntSettings[NOTIFY_DURATION], buf, SIZEOF(buf), 10);
			SetDlgItemTextA(hwndDlg, IDC_NOTIFY_DURATION, buf);

			CheckDlgButton(hwndDlg, IDC_NOTIFY_TITLEHIDE, m_aiIntSettings[NOTIFY_TITLE] == NOTIFY_TITLE_HIDE? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_TITLENAME, m_aiIntSettings[NOTIFY_TITLE] == NOTIFY_TITLE_NAME? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTIFY_TITLEINFO, m_aiIntSettings[NOTIFY_TITLE] == NOTIFY_TITLE_INFO? BST_CHECKED : BST_UNCHECKED);
			
			m_bInitializingDialog = false;
			break;
		}
		case WM_NOTIFY:
		{
			if(((LPNMHDR)lParam)->idFrom == IDC_PROTOCOLS)
			{
				if(((LPNMHDR)lParam)->code != NM_CLICK)
					return 0;

				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
				if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
					if(hti.flags&TVHT_ONITEMICON) {
						TVITEM tvi;
						tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
						tvi.hItem=hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						tvi.iImage=tvi.iSelectedImage=!tvi.iImage;
						((CProtoFilter *)tvi.lParam)->bTempNotificationFilter=tvi.iImage != 0;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						
					}
				break;
			}
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					m_abBoolSettings[NOTIFY_TIMESTAMPS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_TIMESTAMPS) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_CHANNELCUTOFF] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_CHANNELCUTOFF) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_NICKCUTOFF] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_NICKCUTOFF) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SHOWPROTO] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SHOWPROTO) == BST_CHECKED ? true : false;
							
					m_abBoolSettings[NOTIFY_IRC_MESSAGES] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_IRC_MESSAGES) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_IRC_USERS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_IRC_USERS) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_IRC_EMOTES] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_IRC_EMOTES) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_IRC_NOTICES] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_IRC_NOTICES) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_IRC_CHANNEL] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_IRC_CHANNEL) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_IRC_STATUS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_IRC_STATUS) == BST_CHECKED ? true : false;

					m_abBoolSettings[NOTIFY_PROTO_STATUS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_PROTO_STATUS) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_PROTO_SIGNON] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_PROTO_SIGNON) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_PROTO_SIGNOFF] =	IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_PROTO_SIGNOFF) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_MESSAGES] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_MESSAGES) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SIGNON] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SIGNON) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SIGNOFF] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SIGNOFF) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_STATUS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_STATUS) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_URL] =			IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_URL) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_FILE] =			IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_FILE) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_CONTACTS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_CONTACTS) == BST_CHECKED ? true : false;
			
					m_abBoolSettings[NOTIFY_NO_SKIP_REPLY] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_NO_SKIP_REPLY) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SKIP_MESSAGES] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SKIP_MESSAGES) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SKIP_SIGNON] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SKIP_SIGNON) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SKIP_SIGNOFF] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SKIP_SIGNOFF) == BST_CHECKED ? true : false;
					m_abBoolSettings[NOTIFY_SKIP_STATUS] =		IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_SKIP_STATUS) == BST_CHECKED ? true : false;
					
					GetDlgItemTextA(hwndDlg,IDC_NOTIFY_CHANNELCUTOFF_OFFSET,buf,SIZEOF(buf));
					m_aiIntSettings[NOTIFY_CHANNELCUTOFF_OFFSET] = atoi(buf) > 0 ? atoi(buf):1;

					GetDlgItemTextA(hwndDlg,IDC_NOTIFY_NICKCUTOFF_OFFSET,buf,SIZEOF(buf));
					m_aiIntSettings[NOTIFY_NICKCUTOFF_OFFSET] = atoi(buf) > 0 ? atoi(buf):1;

					GetDlgItemTextA(hwndDlg,IDC_NOTIFY_DURATION,buf,SIZEOF(buf));
					m_aiIntSettings[NOTIFY_DURATION] = atoi(buf) > 0 ? atoi(buf):1;

					GetDlgItemTextA(hwndDlg,IDC_NOTIFY_LOGSIZE,buf,SIZEOF(buf));
					m_aiIntSettings[NOTIFY_LOGSIZE] = atoi(buf) > 0 ? atoi(buf):1;

					if(IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_TITLEHIDE) == BST_CHECKED)
						m_aiIntSettings[NOTIFY_TITLE] = NOTIFY_TITLE_HIDE;
					else if(IsDlgButtonChecked(hwndDlg,IDC_NOTIFY_TITLENAME) == BST_CHECKED)
						m_aiIntSettings[NOTIFY_TITLE] = NOTIFY_TITLE_NAME;
					else
						m_aiIntSettings[NOTIFY_TITLE] = NOTIFY_TITLE_INFO;

					// apply all contactlist protocol filters
					vector<CProtoFilter*>::iterator iter = m_ProtoList.begin();
					while(iter != m_ProtoList.end())
					{
						(*iter)->bNotificationFilter = (*iter)->bTempNotificationFilter;
						iter++;
					}

					CConfig::SaveSettings();
					SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
					break;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			// Activate the apply button
			if (!m_bInitializingDialog && ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
	}
	return 0;
}

INT_PTR CALLBACK CConfig::ContactlistDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			m_bInitializingDialog = true;
			
			// Translate the dialog
			TranslateDialogDefault(hwndDlg);

			// Initialize the protocol filter list
			SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CLIST_PROTOFILTER),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CLIST_PROTOFILTER),GWL_STYLE)|TVS_NOHSCROLL);
			int iRes = 0;
			
			HIMAGELIST himlCheckBoxes;
			himlCheckBoxes=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,2,2);
			iRes = ImageList_AddIcon(himlCheckBoxes,LoadIcon(hInstance,MAKEINTRESOURCE(IDI_NOTICK)));
			iRes = ImageList_AddIcon(himlCheckBoxes,LoadIcon(hInstance,MAKEINTRESOURCE(IDI_TICK)));
			TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_CLIST_PROTOFILTER),himlCheckBoxes,TVSIL_NORMAL);

			FillTree(GetDlgItem(hwndDlg,IDC_CLIST_PROTOFILTER),true);
			
			CheckDlgButton(hwndDlg, IDC_CLIST_SELECTION,	m_abBoolSettings[CLIST_SELECTION]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_COLUMNS,	m_abBoolSettings[CLIST_COLUMNS]		? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_HIDEOFFLINE,	m_abBoolSettings[CLIST_HIDEOFFLINE]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_USEIGNORE,	m_abBoolSettings[CLIST_USEIGNORE]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_USEGROUPS,	m_abBoolSettings[CLIST_USEGROUPS]	? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_SHOWPROTO,	m_abBoolSettings[CLIST_SHOWPROTO]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_DRAWLINES,	m_abBoolSettings[CLIST_DRAWLINES]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_POSITION,	m_abBoolSettings[CLIST_POSITION]? BST_CHECKED	: BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIST_COUNTERS,	m_abBoolSettings[CLIST_COUNTERS]? BST_CHECKED	: BST_UNCHECKED);

			if(m_aiIntSettings[CLIST_GA] == CLIST_GA_NONE)
				CheckDlgButton(hwndDlg, IDC_CLIST_GA_NONE, BST_CHECKED);
			else if(m_aiIntSettings[CLIST_GA] == CLIST_GA_COLLAPSE)
				CheckDlgButton(hwndDlg, IDC_CLIST_GA_COLLAPSE, BST_CHECKED);
			else
				CheckDlgButton(hwndDlg, IDC_CLIST_GA_EXPAND, BST_CHECKED);

			m_bInitializingDialog = false;
			break;
		}
		case WM_NOTIFY:
		{
			if(((LPNMHDR)lParam)->idFrom == IDC_CLIST_PROTOFILTER)
			{
				if(((LPNMHDR)lParam)->code != NM_CLICK)
					return 0;

				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
				if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
					if(hti.flags&TVHT_ONITEMICON) {
						TVITEM tvi;
						tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
						tvi.hItem=hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						tvi.iImage=tvi.iSelectedImage=!tvi.iImage;
						((CProtoFilter *)tvi.lParam)->bTempContactlistFilter=tvi.iImage != 0;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				break;
			}
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					m_abBoolSettings[CLIST_SELECTION] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_SELECTION) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_COLUMNS] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_COLUMNS) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_HIDEOFFLINE] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_HIDEOFFLINE) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_USEGROUPS] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_USEGROUPS) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_USEIGNORE] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_USEIGNORE) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_SHOWPROTO] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_SHOWPROTO) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_DRAWLINES] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_DRAWLINES) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_POSITION] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_POSITION) == BST_CHECKED ? true : false;
					m_abBoolSettings[CLIST_COUNTERS] =	IsDlgButtonChecked(hwndDlg,IDC_CLIST_COUNTERS) == BST_CHECKED ? true : false;
					
					if(IsDlgButtonChecked(hwndDlg,IDC_CLIST_GA_NONE) == BST_CHECKED)
						m_aiIntSettings[CLIST_GA] = CLIST_GA_NONE;
					else if(IsDlgButtonChecked(hwndDlg,IDC_CLIST_GA_COLLAPSE) == BST_CHECKED)
						m_aiIntSettings[CLIST_GA] = CLIST_GA_COLLAPSE;
					else
						m_aiIntSettings[CLIST_GA] = CLIST_GA_EXPAND;

					// apply all contactlist protocol filters
					vector<CProtoFilter*>::iterator iter = m_ProtoList.begin();
					while(iter != m_ProtoList.end())
					{
						(*iter)->bContactlistFilter = (*iter)->bTempContactlistFilter;
						iter++;
					}
					CConfig::SaveSettings();
					SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
					break;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			// Activate the apply button
			if (!m_bInitializingDialog && ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
	}
	return 0;
}


bool CConfig::GetProtocolContactlistFilter(tstring strProtocol)
{
	vector<CProtoFilter*>::iterator iter = m_ProtoList.begin();
	while(iter != m_ProtoList.end())
	{
		if((*iter)->strName == strProtocol)
			return (*iter)->bContactlistFilter;
		iter++;
	}
	return false;
}

bool CConfig::GetProtocolNotificationFilter(tstring strProtocol)
{
	vector<CProtoFilter*>::iterator iter = m_ProtoList.begin();
	while(iter != m_ProtoList.end())
	{
		if((*iter)->strName == strProtocol)
			return (*iter)->bNotificationFilter;
		iter++;
	}
	return false;
}

bool CConfig::GetBoolSetting(int iSetting)
{
	if(iSetting >= 0 && iSetting < BOOL_SETTINGS)
		return m_abBoolSettings[iSetting];
	return false;
}

int CConfig::GetIntSetting(int iSetting)
{
	if(iSetting >= 0 && iSetting < INT_SETTINGS)
		return m_aiIntSettings[iSetting];
	return false;
}

void CConfig::ClearTree(HWND hTree)
{
	TreeView_DeleteAllItems(hTree);
}

void CConfig::FillDeviceBox(HWND hBox) {
	CLCDConnection *connection = CAppletManager::GetInstance()->GetLCDConnection();
	SendMessage(hBox,CB_RESETCONTENT,0,0);

	int count = 0;
	for (int i = 0; i < 10; i++) {
		CLCDDevice *device = connection->GetAttachedDevice(i);
		if (device != NULL) {
			SendMessage(hBox,CB_ADDSTRING,0,(LPARAM)device->GetDisplayName().c_str());
			count++;
		}
	}

	if (!count) {
		SendMessage(hBox,CB_ADDSTRING,0,(LPARAM)TranslateT("No device attached"));
	}

	int iDevice = CConfig::GetIntSetting(DEVICE);
	if (iDevice >= count)
		iDevice = 0;

	SendMessage(hBox, CB_SETCURSEL, iDevice, 0);
}

void CConfig::FillTree(HWND hTree,bool bCList)
{
	TreeView_DeleteAllItems(hTree);

	TVINSERTSTRUCT tvis;
	tvis.hParent=NULL;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;	
	
	vector<CProtoFilter*>::iterator iter = m_ProtoList.begin();
	while(iter != m_ProtoList.end())
	{
		tvis.item.lParam=(LPARAM)(*iter);
		tvis.item.pszText= (LPTSTR)(*iter)->strName.c_str();
		if(!bCList)
		{
			tvis.item.iImage= (*iter)->bNotificationFilter;
			(*iter)->bTempNotificationFilter = (*iter)->bNotificationFilter;
		}
		else
		{
			tvis.item.iImage= (*iter)->bContactlistFilter;
			(*iter)->bTempContactlistFilter = (*iter)->bContactlistFilter;
		}
		tvis.item.iSelectedImage= tvis.item.iImage;
		
		TreeView_InsertItem(hTree,&tvis);
		iter++;
	}
}