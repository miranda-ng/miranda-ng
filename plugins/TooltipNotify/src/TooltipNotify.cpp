// TooltipNotify.cpp: implementation of the CTooltipNotify class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

enum
{
	ProtoIntBit = 0x01,
	ProtoUserBit = 0x02
};

static const char* SND_ONLINE = "ttntfOnline";
static const char* SND_OFFLINE = "ttntfOffline";
static const char* SND_OTHER = "ttntfOther";
static const char* SND_TYPING = "ttntfTyping";
static const char* CONTACT_IGNORE_TTNOTIFY = "Ignore";

static const int ID_TTNTF_STATUS_TYPING = ID_STATUS_INVISIBLE+10;
static const int ID_TTNTF_STATUS_IDLE = ID_STATUS_INVISIBLE+11;
static const int ID_TTNTF_STATUS_NOT_IDLE = ID_STATUS_INVISIBLE+12;

#define FONTSERV_GROUP   LPGENT("Tooltip Notify")
#define FONTSERV_ONLINE  LPGENT("Online")
#define FONTSERV_OFFLINE LPGENT("Offline")
#define FONTSERV_OTHER   LPGENT("Other status")
#define FONTSERV_TYPING  LPGENT("Typing")
#define FONTSERV_IDLE    LPGENT("Idle")

struct FontEntry
{
	int status;
	TCHAR* name;
	char* fontPrefix;
	char* clrPrefix;
};

static FontEntry s_fontTable[] = 
{
	ID_STATUS_ONLINE,         FONTSERV_ONLINE,  "OnlineFont",  "OnlineBgColor",
	ID_STATUS_OFFLINE,        FONTSERV_OFFLINE, "OfflineFont", "OfflineBgColor",
	ID_TTNTF_STATUS_TYPING,   FONTSERV_TYPING,  "TypingFont",  "TypingBgColor",
	ID_TTNTF_STATUS_IDLE,     FONTSERV_IDLE,    "IdleFont",    "IdleBgColor",
	ID_TTNTF_STATUS_NOT_IDLE, FONTSERV_IDLE,    "IdleFont",    "IdleBgColor",
	0,                        FONTSERV_OTHER,   "OtherFont",   "OtherBgColor",
};

CTooltipNotify *CTooltipNotify::s_pInstance = 0;
const char *CTooltipNotify::s_szModuleNameOld = "ttntfmod";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTooltipNotify::CTooltipNotify() :
	m_bNt50( IsNt50())
{
	if (s_pInstance!=0)
		throw EAlreadyExists();

	s_pInstance = this;

	CTooltip::Initialize();
}

CTooltipNotify::~CTooltipNotify()
{
	EndNotifyAll();
	CTooltip::Deinitialize();
	s_pInstance=0;
}

void CTooltipNotify::RegisterFonts()
{
	FontIDT fontId = { sizeof(fontId) };
	mir_tstrncpy(fontId.group, FONTSERV_GROUP, SIZEOF(fontId.group)-1);
	mir_strncpy(fontId.dbSettingsGroup, MODULENAME, SIZEOF(fontId.dbSettingsGroup)-1);
	fontId.flags = FIDF_DEFAULTVALID;
	fontId.deffontsettings.colour = DEF_SETTING_TXTCOLOR;
	fontId.deffontsettings.size = -MulDiv(DEF_SETTING_FONT_SIZE, DEF_LOGPIXELSY, 72);
	fontId.deffontsettings.style = DEF_SETTING_FONT_STYLE;
	fontId.deffontsettings.charset = DEF_SETTING_FONT_CHARSET;
	mir_tstrncpy(fontId.deffontsettings.szFace, DEF_SETTING_FONT_FACE, SIZEOF(fontId.deffontsettings.szFace)-1);
	fontId.order = 0;
	mir_tstrncpy(fontId.backgroundGroup, FONTSERV_GROUP, SIZEOF(fontId.backgroundGroup)-1);

	ColourIDT colorId = { sizeof(colorId) };
	mir_tstrncpy(colorId.group, FONTSERV_GROUP, SIZEOF(colorId.group)-1);
	mir_strncpy(colorId.dbSettingsGroup, MODULENAME, SIZEOF(colorId.dbSettingsGroup)-1);
	colorId.flags = 0;
	colorId.defcolour = DEF_SETTING_BGCOLOR;
	colorId.order = 0;

	for (int i=0; i<SIZEOF(s_fontTable); i++) {
		mir_tstrncpy(fontId.name, s_fontTable[i].name, SIZEOF(fontId.name)-1);
		mir_strncpy(fontId.prefix, s_fontTable[i].fontPrefix, SIZEOF(fontId.prefix)-1);
		mir_tstrncpy(fontId.backgroundName, s_fontTable[i].name, SIZEOF(fontId.backgroundName)-1);
		::FontRegisterT(&fontId);

		mir_tstrncpy(colorId.name, s_fontTable[i].name, SIZEOF(colorId.name)-1);
		mir_strncpy(colorId.setting, s_fontTable[i].clrPrefix, SIZEOF(colorId.setting)-1);
		::ColourRegisterT(&colorId);
	}
}

void CTooltipNotify::GetFont(int iStatus, LOGFONT* lf, COLORREF* text, COLORREF* bg)
{
	TCHAR* fontName = 0;
	for(int i=0; i<SIZEOF(s_fontTable); i++)
	{
		if (s_fontTable[i].status == iStatus)
		{
			fontName = s_fontTable[i].name;
		}
	}
	if (fontName == 0)
	{
		fontName = s_fontTable[SIZEOF(s_fontTable)-1].name;
	}

	// name and group only
	FontIDT fontId = { sizeof(fontId), FONTSERV_GROUP, 0 };
	mir_tstrncpy(fontId.name, fontName,SIZEOF(fontId.name)-1);
	*text = (COLORREF)::CallService(MS_FONT_GETT, (WPARAM)&fontId, (LPARAM)lf);
	ColourIDT colorId = { sizeof(colorId), FONTSERV_GROUP, 0 };
	mir_tstrncpy(colorId.name, fontName,SIZEOF(colorId.name)-1);
	*bg = (COLORREF)::CallService(MS_COLOUR_GETT, (WPARAM)&colorId, 0);
}

int CTooltipNotify::ModulesLoaded(WPARAM, LPARAM)
{
	MigrateSettings();
	LoadSettings();
	ValidateSettings();

	if (m_sOptions.bFirstRun)
	{
		db_set_b(NULL, "SkinSoundsOff", SND_ONLINE, 1);
		db_set_b(NULL, "SkinSoundsOff", SND_OFFLINE, 1);
		db_set_b(NULL, "SkinSoundsOff", SND_OTHER, 1);
		db_set_b(NULL, "SkinSoundsOff", SND_TYPING, 1);
		db_set_b(NULL, MODULENAME, "firstrun", 0);
	}
	
	// register fonts
	SkinAddNewSound(SND_ONLINE,  LPGEN("Tooltip Notify: Online"),  "online.wav");
	SkinAddNewSound(SND_OFFLINE, LPGEN("Tooltip Notify: Offline"), "offline.wav");
	SkinAddNewSound(SND_OTHER,   LPGEN("Tooltip Notify: Other"),   "other.wav");
	SkinAddNewSound(SND_TYPING,  LPGEN("Tooltip Notify: Typing"),  "typing.wav");

	RegisterFonts();

	return 0;
}


int CTooltipNotify::ProtoContactIsTyping(WPARAM hContact, LPARAM lParam)
{
	if (!m_sOptions.bTyping) return 0;

	if (lParam > 0) {
		STooltipData *pTooltipData = new STooltipData;
		pTooltipData->uiTimeout = lParam*1000;
		pTooltipData->hContact = hContact;
		pTooltipData->iStatus = ID_TTNTF_STATUS_TYPING;

		EndNotifyAll();
		SkinPlaySound(SND_TYPING);
		BeginNotify(pTooltipData);
	}
	else EndNotifyAll();

	return 0;
}


int CTooltipNotify::ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack=(ACKDATA*)lParam;
	if((ack == NULL) || (ack->type != ACKTYPE_STATUS)) return 0;

	WORD wNewStatus = (WORD)ack->lParam;
	WORD wOldStatus = (WORD)ack->hProcess;
	if (wOldStatus == wNewStatus) return 0; //Useless message.

	char *szProtocol = (char *)ack->szModule;
	
	if (wNewStatus == ID_STATUS_OFFLINE)
	{
		BYTE bProtoActive = db_get_b(NULL, MODULENAME, szProtocol, ProtoUserBit|ProtoIntBit);
		bProtoActive &= ~ProtoIntBit;
		db_set_b(NULL, MODULENAME, szProtocol, bProtoActive);
	}
	else
	{			
		if (wOldStatus < ID_STATUS_ONLINE && wNewStatus > ID_STATUS_OFFLINE)
		{
			UINT_PTR idTimer = SetTimer(0, 0, m_sOptions.wStartupDelay*1000, ConnectionTimerProcWrapper);
			ProtoData protoData = { _strdup(szProtocol), idTimer };
			m_mapTimerIdProto.push_back(protoData);
		} 
	}

	return 0;
}

int CTooltipNotify::ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!hContact)
		return 0;
	
	bool idle = false;
	if (mir_strcmp(cws->szSetting,"Status")==0)
		idle = false;
	else if (mir_strcmp(cws->szSetting,"IdleTS")==0)
		idle = true;
	else return 0;
	
	if(db_get_b(hContact, "CList", "Hidden", 0))
		return 0;
	
	const char *pszProto = cws->szModule;
	if (db_get_b(NULL, MODULENAME, pszProto, ProtoUserBit|ProtoIntBit) != (ProtoUserBit|ProtoIntBit))
		return 0;

	if (db_get_b(hContact, "CList", "NotOnList", 0) && m_sOptions.bIgnoreUnknown)
		return 0;

	if (db_get_b(hContact, MODULENAME, CONTACT_IGNORE_TTNOTIFY, m_sOptions.bIgnoreNew))
		return 0;

	if (idle && !m_sOptions.bIdle)
		return 0;

	WORD wNewStatus = cws->value.wVal; 
	switch(wNewStatus)
	{
		case ID_STATUS_OFFLINE:
			if (!m_sOptions.bOffline) return 0;
			SkinPlaySound(SND_OFFLINE);
			break;

		case ID_STATUS_ONLINE:
			if(CallService(MS_IGNORE_ISIGNORED,hContact,IGNOREEVENT_USERONLINE) && m_sOptions.bConjSOLN) return 0;
			if (!m_sOptions.bOnline) return 0;
			SkinPlaySound(SND_ONLINE);
			break;

		default:
			if (!m_sOptions.bOther) return 0;
			SkinPlaySound(SND_OTHER);
			break;
	}

	STooltipData *pTooltipData = new STooltipData;
	pTooltipData->uiTimeout = m_sOptions.wDuration * (wNewStatus==ID_STATUS_ONLINE ? (m_sOptions.bX2+1):1);
	pTooltipData->hContact = hContact;
	
	if (idle) wNewStatus = (wNewStatus!=0 ? ID_TTNTF_STATUS_IDLE : ID_TTNTF_STATUS_NOT_IDLE);
	pTooltipData->iStatus = wNewStatus;

	EndNotifyAll();
	BeginNotify(pTooltipData);

	return 0; 

}

int CTooltipNotify::InitializeOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = g_hInstDLL;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = LPGEN("Tooltip Notify");
	odp.pszGroup = LPGEN("Popups");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = CTooltipNotify::OptionsDlgProcWrapper;
	::Options_AddPage(wParam, &odp);
	return 0;
}

CTooltip *CTooltipNotify::BeginNotify(STooltipData *pTooltipData)
{
	TCHAR szTooltipText[64] = {0};
	MakeTooltipString(pTooltipData->hContact, pTooltipData->iStatus, szTooltipText, 64);

	LOGFONT lf = {0};
	COLORREF textColor = 0;
	COLORREF bgColor = 0;
	GetFont(pTooltipData->iStatus, &lf, &textColor, &bgColor);

	CTooltip *pTooltip = new CTooltip(this);
	pTooltip->set_Text(szTooltipText);
	pTooltip->set_Font(lf);
	pTooltip->set_BgColor(bgColor);
	pTooltip->set_TextColor(textColor);
	if (m_bNt50 && m_sOptions.bTransp)
		pTooltip->set_Translucency(m_sOptions.bAlpha);
	pTooltip->set_TransparentInput(m_bNt50 && m_sOptions.bTransp && m_sOptions.bTranspInput);
	pTooltip->Validate();

	RECT TooltipRect, WorkAreaRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkAreaRect, 0);
	pTooltip->get_Rect(&TooltipRect);

	if (m_sOptions.bAutoPos || Utils_RestoreWindowPositionEx(pTooltip->GetHandle(), RWPF_NOSIZE | RWPF_NOACTIVATE, 0, MODULENAME, "toolwindow"))
		pTooltip->set_Position(
			WorkAreaRect.right - 10 - (TooltipRect.right-TooltipRect.left), 
			WorkAreaRect.bottom - 2 - (TooltipRect.bottom-TooltipRect.top));

	UINT_PTR idTimer = SetTimer(0, 0, pTooltipData->uiTimeout, TooltipTimerProcWrapper);
	pTooltipData->idTimer = idTimer;
	pTooltipData->pTooltip = pTooltip;

	m_TooltipsList.push_back(pTooltipData);

	pTooltip->Show();

	return pTooltip;
}

BOOL CTooltipNotify::EndNotify(STooltipData* pTooltipData)
{
	CTooltip* pTooltip = pTooltipData->pTooltip;

	// return if the tooltip timer suspended
	if (pTooltipData->idTimer == 0) return FALSE;
	SuspendTimer(pTooltip);

	pTooltip->Hide();
	delete pTooltip;
	delete pTooltipData;

	m_TooltipsList.erase(
		std::remove(m_TooltipsList.begin(), m_TooltipsList.end(), pTooltipData),
		m_TooltipsList.end());

	return TRUE;
}

void CTooltipNotify::EndNotifyAll()
{
	// iterate through active tooltips and
	// remove one which do not have its timer suspended
	TooltipsList::reverse_iterator mapRevIter = m_TooltipsList.rbegin();
	while (mapRevIter != m_TooltipsList.rend())
	{
		STooltipData* pTooltipData = *mapRevIter;
		if (EndNotify(pTooltipData))
		{
			mapRevIter = m_TooltipsList.rbegin();
		}
		else
		{
			++mapRevIter;
		}
	}
}

CTooltipNotify::MapTimerIdProtoIter CTooltipNotify::FindProtoByTimer(UINT idTimer)
{
	for (
		MapTimerIdProtoIter iter = m_mapTimerIdProto.begin(); 
		iter != m_mapTimerIdProto.end(); 
		++iter)
	{
		if (iter->timerId == idTimer)
		{
			return iter;
		}
	}

	return m_mapTimerIdProto.end();
}

void CTooltipNotify::OnConnectionTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	BOOL bSuccess = KillTimer(0, idEvent);
	assert(bSuccess);

	MapTimerIdProtoIter iter = FindProtoByTimer(idEvent);
	assert(iter!=m_mapTimerIdProto.end());
	
	BYTE bProtoActive = db_get_b(NULL, MODULENAME, iter->proto, ProtoUserBit|ProtoIntBit);
	bProtoActive |= ProtoIntBit;
	db_set_b(NULL, MODULENAME, iter->proto, bProtoActive);

	free((char*)iter->proto);
	m_mapTimerIdProto.erase(iter);
}


void CTooltipNotify::OnTooltipTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	TooltipsList::iterator iter = FindBy(&STooltipData::idTimer, idEvent);
	assert(iter!=m_TooltipsList.end());

	STooltipData* pTooltipData = *iter;
	EndNotify(pTooltipData);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	const char *szOldSetting, *szNewSetting;
}
static arSettings[] = 
{
	{ "xpos", "toolwindowx" },
	{ "ypos", "toolwindowy" }
};

void CTooltipNotify::MigrateSettings()
{
	for (int i=0; i < SIZEOF(arSettings); i++) {
		int val = db_get_w(NULL, MODULENAME, arSettings[i].szOldSetting, -1);
		if (val != -1) {
			db_set_w(NULL, MODULENAME, arSettings[i].szNewSetting, val);
			db_unset(NULL, MODULENAME, arSettings[i].szOldSetting);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTooltipNotify::LoadSettings()
{ 
	m_sOptions.bFirstRun = db_get_b(NULL, MODULENAME, "firstrun", DEF_SETTING_FIRSTRUN);
	m_sOptions.bOffline = db_get_b(NULL, MODULENAME, "offline", DEF_SETTING_OFFLINE);
	m_sOptions.bOnline = db_get_b(NULL, MODULENAME, "online", DEF_SETTING_ONLINE);
	m_sOptions.bOther = db_get_b(NULL, MODULENAME, "other", DEF_SETTING_OTHER);
	m_sOptions.bTyping = db_get_b(NULL, MODULENAME, "typing", DEF_SETTING_TYPING);
	m_sOptions.bIdle = db_get_b(NULL, MODULENAME, "idle", DEF_SETTING_TYPING);
	m_sOptions.bX2 = db_get_b(NULL, MODULENAME, "x2", DEF_SETTING_X2);
	m_sOptions.bConjSOLN = db_get_b(NULL, MODULENAME, "conjsoln", DEF_SETTING_CONJSOLN);
	m_sOptions.bAutoPos = db_get_b(NULL, MODULENAME, "autopos", DEF_SETTING_DEF_POS);
	m_sOptions.bBallonTip = db_get_b(NULL, MODULENAME, "balloontip", DEF_SETTING_BALLONTIP);
	m_sOptions.bTransp = db_get_b(NULL, MODULENAME, "transp", DEF_SETTING_TRANSP);
	m_sOptions.bAlpha = db_get_b(NULL, MODULENAME, "alpha", DEF_SETTING_ALPHA);
	m_sOptions.bTranspInput = db_get_b(NULL, MODULENAME, "transpinput", DEF_SETTING_TRANSP_INPUT);
	m_sOptions.bPrefixProto = db_get_b(NULL, MODULENAME, "prfxproto", DEF_SETTING_PREFIX_PROTO);
	m_sOptions.bLDblClick = db_get_b(NULL, MODULENAME, "ldblclick", DEF_SETTING_LDBLCLICK);
	m_sOptions.wDuration = db_get_w(NULL, MODULENAME, "duration", DEF_SETTING_DURATION);
	m_sOptions.wStartupDelay = db_get_w(NULL, MODULENAME, "suprconndelay", DEF_SETTING_STARTUP_DELAY);	
	m_sOptions.bIgnoreUnknown = db_get_b(NULL, MODULENAME, "ignoreunknown", DEF_SETTING_IGNORE_UNKNOWN);	
	m_sOptions.bIgnoreNew = db_get_b(NULL, MODULENAME, "ignorenew", DEF_SETTING_IGNORE_NEW);	
}

void CTooltipNotify::SaveSettings()
{
	db_set_w(NULL, MODULENAME, "duration", m_sOptions.wDuration);
	db_set_w(NULL, MODULENAME, "suprconndelay", m_sOptions.wStartupDelay);
	db_set_b(NULL, MODULENAME, "offline", m_sOptions.bOffline);
	db_set_b(NULL, MODULENAME, "online", m_sOptions.bOnline);
	db_set_b(NULL, MODULENAME, "other", m_sOptions.bOther);
	db_set_b(NULL, MODULENAME, "typing", m_sOptions.bTyping);
	db_set_b(NULL, MODULENAME, "idle", m_sOptions.bIdle);
	db_set_b(NULL, MODULENAME, "prfxproto", m_sOptions.bPrefixProto);
	db_set_b(NULL, MODULENAME, "x2", m_sOptions.bX2);
	db_set_b(NULL, MODULENAME, "conjsoln", m_sOptions.bConjSOLN);
	db_set_b(NULL, MODULENAME, "autopos", m_sOptions.bAutoPos);
	db_set_b(NULL, MODULENAME, "balloontip", m_sOptions.bBallonTip);
	db_set_b(NULL, MODULENAME, "transp", m_sOptions.bTransp);
	db_set_b(NULL, MODULENAME, "alpha", m_sOptions.bAlpha);
	db_set_b(NULL, MODULENAME, "transpinput", m_sOptions.bTranspInput);
	db_set_b(NULL, MODULENAME, "ldblclick", m_sOptions.bLDblClick);
	db_set_b(NULL, MODULENAME, "ignoreunknown", m_sOptions.bIgnoreUnknown);
	db_set_b(NULL, MODULENAME, "ignorenew", m_sOptions.bIgnoreNew);
}


void CTooltipNotify::ReadSettingsFromDlg(HWND hDlg)
{
	m_sOptions.bOffline = (BYTE)(IsDlgButtonChecked(hDlg, IDC_OFFLINE) == BST_CHECKED ? 1:0);
	m_sOptions.bOnline = (BYTE)(IsDlgButtonChecked(hDlg, IDC_ONLINE) == BST_CHECKED ? 1:0);
	m_sOptions.bOther = (BYTE)(IsDlgButtonChecked(hDlg, IDC_OTHER) == BST_CHECKED ? 1:0);
	m_sOptions.bTyping = (BYTE)(IsDlgButtonChecked(hDlg, IDC_TYPING) == BST_CHECKED ? 1:0);
	m_sOptions.bIdle = (BYTE)(IsDlgButtonChecked(hDlg, IDC_IDLE) == BST_CHECKED ? 1:0);
	m_sOptions.bX2 = (BYTE)(IsDlgButtonChecked(hDlg, IDC_X2) == BST_CHECKED ? 1:0);
	m_sOptions.bConjSOLN = (BYTE)(IsDlgButtonChecked(hDlg, IDC_CONJSOLN) == BST_CHECKED ? 1:0);
	m_sOptions.bAutoPos = (BYTE)(IsDlgButtonChecked(hDlg, IDC_AUTOPOS) == BST_CHECKED ? 1:0);
	m_sOptions.bBallonTip = (BYTE)(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED ? 1:0);
	m_sOptions.bTransp = (BYTE)(IsDlgButtonChecked(hDlg, IDC_TRANSPARENCY) == BST_CHECKED ? 1:0);
	m_sOptions.bAlpha = (BYTE)SendDlgItemMessage(hDlg,IDC_TRANSPARENCY_SLIDER,TBM_GETPOS,0,0);
	m_sOptions.bTranspInput = (BYTE)(IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT) == BST_CHECKED ? 1:0);
	m_sOptions.bPrefixProto = (BYTE)(IsDlgButtonChecked(hDlg, IDC_PREFIX_PROTO) == BST_CHECKED ? 1:0);
	m_sOptions.bLDblClick = (BYTE)(IsDlgButtonChecked(hDlg, IDC_RB_CLIST) == BST_CHECKED) ? SHOW_HIDE_CLIST:OPEN_MSGDLG;
	m_sOptions.wDuration = LOWORD(SendDlgItemMessage(hDlg, IDC_DURATIONSPIN, UDM_GETPOS, 0, 0));
	m_sOptions.wStartupDelay = LOWORD(SendDlgItemMessage(hDlg, IDC_DELAYONCONNSPIN, UDM_GETPOS, 0, 0));	
}

void CTooltipNotify::WriteSettingsToDlg(HWND hDlg)
{
	SendDlgItemMessage(hDlg, IDC_DURATIONSPIN, UDM_SETRANGE, 0, MAKELONG(550*36, 550));
	SendDlgItemMessage(hDlg, IDC_DURATIONSPIN, UDM_SETPOS, 0, MAKELONG(m_sOptions.wDuration, 0));
	SendDlgItemMessage(hDlg, IDC_DELAYONCONNSPIN, UDM_SETRANGE, 0, MAKELONG(30, 0));
	SendDlgItemMessage(hDlg, IDC_DELAYONCONNSPIN, UDM_SETPOS, 0, MAKELONG(m_sOptions.wStartupDelay, 0));

	CheckDlgButton(hDlg, IDC_OFFLINE, m_sOptions.bOffline ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_ONLINE, m_sOptions.bOnline ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_OTHER, m_sOptions.bOther ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_TYPING, m_sOptions.bTyping ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_IDLE, m_sOptions.bIdle ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_PREFIX_PROTO, m_sOptions.bPrefixProto ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_X2, m_sOptions.bX2 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CONJSOLN, m_sOptions.bConjSOLN ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_AUTOPOS, m_sOptions.bAutoPos ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_BALLONTIP, m_sOptions.bBallonTip ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_TRANSPARENCY, m_sOptions.bTransp ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_TRANSP_INPUT, m_sOptions.bTranspInput ? BST_CHECKED : BST_UNCHECKED);

	switch(m_sOptions.bLDblClick)
	{
		case SHOW_HIDE_CLIST:	CheckDlgButton(hDlg, IDC_RB_CLIST, BST_CHECKED);	break;
		case OPEN_MSGDLG:	CheckDlgButton(hDlg, IDC_RB_MSGDLG, BST_CHECKED);	break;
		default:		CheckDlgButton(hDlg, IDC_RB_CLIST, BST_CHECKED);	break;
	}

	EnableWindow(GetDlgItem(hDlg, IDC_GB_TRANSP), m_bNt50);
	EnableWindow(GetDlgItem(hDlg, IDC_TRANSPARENCY), m_bNt50);
	EnableWindow(GetDlgItem(hDlg, IDC_TRANSPARENCY_SLIDER), m_sOptions.bTransp && m_bNt50);
	EnableWindow(GetDlgItem(hDlg, IDC_TRANSPERC), m_sOptions.bTransp && m_bNt50);
	EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_INPUT), m_sOptions.bTransp && m_bNt50);
	//EnableWindow(GetDlgItem(hDlg, IDC_GB_DBLCLICK), m_sOptions.bTranspInput);
	EnableWindow(GetDlgItem(hDlg, IDC_RB_CLIST), !m_sOptions.bTranspInput);
	EnableWindow(GetDlgItem(hDlg, IDC_RB_MSGDLG), !m_sOptions.bTranspInput);

	SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(1,255));
	SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_SETPOS, TRUE, m_sOptions.bAlpha);
}


void CTooltipNotify::ValidateSettings()
{
	if (m_sOptions.wStartupDelay>30) m_sOptions.wStartupDelay=30;
	if (m_sOptions.wDuration>550*36) m_sOptions.wDuration=550*36;
	if (m_sOptions.wDuration<550*1) m_sOptions.wDuration=550*1;
	if (!m_sOptions.bTransp) m_sOptions.bTranspInput=0;
}


// main options dialog
BOOL CTooltipNotify::OptionsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hDlg);
			WriteSettingsToDlg(hDlg);
			SendMessage(hDlg, WM_HSCROLL, 0x12345678, 0);

			return TRUE;
		}

		case WM_VSCROLL:
		case WM_HSCROLL:
		{	
			TCHAR str[10];
			mir_sntprintf(str, SIZEOF(str), _T("%d%%"), 100 * SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemText(hDlg, IDC_TRANSPERC, str);
			if(wParam!=0x12345678) SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}		

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_TRANSPARENCY:				
					EnableWindow(GetDlgItem(hDlg, IDC_TRANSPERC), IsDlgButtonChecked(hDlg, IDC_TRANSPARENCY) == BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg, IDC_TRANSPARENCY_SLIDER), IsDlgButtonChecked(hDlg, IDC_TRANSPARENCY) == BST_CHECKED);
					EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_INPUT), IsDlgButtonChecked(hDlg, IDC_TRANSPARENCY) == BST_CHECKED);
					CheckDlgButton(hDlg, IDC_TRANSP_INPUT, IsDlgButtonChecked(hDlg, IDC_TRANSPARENCY) == BST_CHECKED ? (m_sOptions.bTranspInput ? BST_CHECKED : BST_UNCHECKED) : BST_UNCHECKED);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_TRANSP_INPUT, 0), 0);
					SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);					
					break;

				case IDC_TRANSP_INPUT:
					//EnableWindow(GetDlgItem(hDlg,IDC_GB_DBLCLICK), IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT));
					EnableWindow(GetDlgItem(hDlg,IDC_RB_CLIST), !(IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT) == BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_RB_MSGDLG), !(IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT) == BST_CHECKED));
					SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_BALLONTIP:
					EnableWindow(GetDlgItem(hDlg,IDC_DURATION),!(IsDlgButtonChecked(hDlg,IDC_BALLONTIP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_X2),!(IsDlgButtonChecked(hDlg,IDC_BALLONTIP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_RB_CLIST),!(IsDlgButtonChecked(hDlg,IDC_BALLONTIP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_RB_MSGDLG),!(IsDlgButtonChecked(hDlg,IDC_BALLONTIP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_TRANSP_INPUT),!(IsDlgButtonChecked(hDlg,IDC_BALLONTIP)==BST_CHECKED));
					EnableWindow(GetDlgItem(hDlg,IDC_AUTOPOS),!(IsDlgButtonChecked(hDlg,IDC_BALLONTIP)==BST_CHECKED));
					SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_DURATION:
					if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()) return FALSE;
					SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_DELAYONCONN:
					if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()) return FALSE;
					SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_PREVIEW:
				{
					STooltipData *pTooltipData = new STooltipData;
					pTooltipData->uiTimeout = m_sOptions.wDuration * (m_sOptions.bX2 ? (m_sOptions.bX2+1):1);
					pTooltipData->hContact = 0;
					pTooltipData->iStatus = ID_STATUS_ONLINE;

					EndNotifyAll();
					BeginNotify(pTooltipData);
					break;
				}

				case IDC_SEL_PROTO:
					DialogBox(g_hInstDLL, MAKEINTRESOURCE(IDD_PROTOS), hDlg, CTooltipNotify::ProtosDlgProcWrapper);
					break;

				case IDC_IGNORE:
					DialogBox(g_hInstDLL, MAKEINTRESOURCE(IDD_CONTACTS), hDlg, CTooltipNotify::ContactsDlgProcWrapper);
					break;

				default:
					// activate 'apply' button
					SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
					return TRUE;
			}
			
			return TRUE;
		}
			
		case WM_NOTIFY:
		{
			//Here we have pressed either the OK or the APPLY button.
			switch(((LPNMHDR)lParam)->idFrom)
			{
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
				case PSN_APPLY:
					ReadSettingsFromDlg(hDlg);
					SaveSettings();
					return TRUE;

				} // switch code
				break;

			} //switch idFrom

			return TRUE;
		}
		
		default:
			break;

	}

	return FALSE;

}


// dialog for protocols selecting
BOOL CTooltipNotify::ProtosDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg)
	{

		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hDlg);

				ListView_SetExtendedListViewStyle(GetDlgItem(hDlg, IDC_PROTOS), LVS_EX_CHECKBOXES);
			
				// enum protocols currently running						
				int iProtoCount = 0;
				PROTOACCOUNT **ppProtos = 0;
				ProtoEnumAccounts(&iProtoCount, &ppProtos);

				// and fill in the list
				for (int i=0; i < iProtoCount; i++)
				{
					LV_ITEM lvi;

					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = i; 
					lvi.lParam = i;

					WCHAR wszProto[128];
					long lLen = MultiByteToWideChar(CP_ACP, 0, ppProtos[i]->szModuleName,
						(int)mir_strlen(ppProtos[i]->szModuleName), wszProto, SIZEOF(wszProto));
					wszProto[lLen] = L'\0';

					lvi.pszText = wszProto;

					int new_item = ListView_InsertItem(GetDlgItem(hDlg,IDC_PROTOS),&lvi);

					BYTE bProtoState = db_get_b(NULL, MODULENAME, ppProtos[i]->szModuleName, ProtoUserBit|ProtoIntBit);
					BOOL bProtoEnabled = (bProtoState & ProtoUserBit) != 0;
					ListView_SetCheckState(GetDlgItem(hDlg,IDC_PROTOS), i, bProtoEnabled);
				}

				return TRUE;
			}

	
		case WM_COMMAND:
			{
				if (LOWORD(wParam) == IDOK) 
				{
					int proto_count = ListView_GetItemCount(GetDlgItem(hDlg,IDC_PROTOS));
					
					for (int i=0; i < proto_count; i++)
					{
						TCHAR szProto[64];

						ListView_GetItemText(GetDlgItem(hDlg,IDC_PROTOS), i, 0, szProto, SIZEOF(szProto));

						char szMultiByteProto[128];
						long lLen = WideCharToMultiByte(CP_ACP, 0, szProto, (int)mir_tstrlen(szProto), 
							szMultiByteProto, sizeof(szMultiByteProto), NULL, NULL);
						szMultiByteProto[lLen] = '\0';

						BYTE bProtoState = db_get_b(NULL, MODULENAME, szMultiByteProto, ProtoUserBit|ProtoIntBit);


						BOOL bProtoEnabled = ListView_GetCheckState(GetDlgItem(hDlg,IDC_PROTOS), i);
						bProtoState = bProtoEnabled ? bProtoState|ProtoUserBit : bProtoState&~ProtoUserBit;

						db_set_b(NULL, MODULENAME, szMultiByteProto, bProtoState);		
 
					}

					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}

				if (LOWORD(wParam) == IDCANCEL) 
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}

				return TRUE;
			}

		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
					
		default:
			break;

	} // switch (msg)

	return FALSE;

}

void CTooltipNotify::ResetCList(HWND hwndDlg)
{
	BOOL b = (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && 
			db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT));
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, (WPARAM) b, 0);

	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

void CTooltipNotify::LoadList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew && !m_sOptions.bIgnoreNew)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM) hItemNew, 1);

	if (hItemUnknown && !m_sOptions.bIgnoreUnknown)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM) hItemUnknown, 1);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE) SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem && !db_get_b(hContact, MODULENAME, CONTACT_IGNORE_TTNOTIFY, m_sOptions.bIgnoreNew))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM) hItem, 1);
	}
}

void CTooltipNotify::SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew)
		m_sOptions.bIgnoreNew = (BYTE) (SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM) hItemNew, 0) ? 0 : 1);

	if (hItemUnknown)
		m_sOptions.bIgnoreUnknown = (BYTE) (SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM) hItemUnknown, 0) ? 0 : 1);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE) SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem) {
			BYTE bChecked = (BYTE) (SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM) hItem, 0));
			db_set_b(hContact, MODULENAME, CONTACT_IGNORE_TTNOTIFY, bChecked ? 0 : 1);
		}
	}
}


// dialog for ignore tooltip notifications
BOOL CTooltipNotify::ContactsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	static HANDLE hItemNew, hItemUnknown;

	switch (msg)
	{
		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hDlg);

				CLCINFOITEM cii = { 0 };
				cii.cbSize = sizeof(cii);
				cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
				cii.pszText = TranslateT("** New contacts **");
				hItemNew = (HANDLE) SendDlgItemMessage(hDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM) & cii);
				cii.pszText = TranslateT("** Unknown contacts **");
				hItemUnknown = (HANDLE) SendDlgItemMessage(hDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM) & cii);

				ResetCList(hDlg);
				LoadList(hDlg, hItemNew, hItemUnknown); 
				return TRUE;
			}

	
		case WM_COMMAND:
			{
				if (LOWORD(wParam) == IDOK) 
				{
					SaveList(hDlg, hItemNew, hItemUnknown); 
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}

				if (LOWORD(wParam) == IDCANCEL) 
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}

				return TRUE;
			}

		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
					
		default:
			break;

	} // switch (msg)

	return FALSE;

}



TCHAR *CTooltipNotify::StatusToString(int iStatus, TCHAR *szStatus, int iBufSize)
{
	if((iStatus>=ID_STATUS_OFFLINE) && (iStatus<=ID_STATUS_OUTTOLUNCH))
	{
		mir_tstrncpy(szStatus, (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,iStatus,GSMDF_TCHAR), iBufSize);
	}
	else
	{
		switch(iStatus)
		{
			case ID_TTNTF_STATUS_TYPING: 
				mir_tstrncpy(szStatus, TranslateT("Typing"), iBufSize);
				break;

			case ID_TTNTF_STATUS_IDLE: 
				mir_tstrncpy(szStatus, TranslateT("Idle"), iBufSize);
				break;

			case ID_TTNTF_STATUS_NOT_IDLE: 
				mir_tstrncpy(szStatus, TranslateT("Not Idle"), iBufSize);
				break;

			default:
				mir_tstrncpy(szStatus, TranslateT("Unknown"), iBufSize);
				break;
		}
	}
	return szStatus;

}

TCHAR *CTooltipNotify::MakeTooltipString(MCONTACT hContact, int iStatus, TCHAR *szString, int iBufSize)
{
	TCHAR szStatus[32];
	StatusToString(iStatus, szStatus, SIZEOF(szStatus));

	// "proro: user is online"
	const TCHAR *szFormatString = m_sOptions.bPrefixProto ? _T("%s%s%s") : _T("%.0s%.0s%s");
	const TCHAR* szIs = TranslateT("is");

	const char* szProto = 
		hContact==0 ? "Proto" : (char*)::GetContactProto(hContact);
	const TCHAR* szContactName = 
		(TCHAR *)::CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

	memset(szString, 0, iBufSize*sizeof(TCHAR));


	WCHAR wszProto[32];
	long lLen = MultiByteToWideChar(CP_ACP, 0, szProto, (int)mir_strlen(szProto), wszProto, SIZEOF(wszProto));
	wszProto[lLen] = _T('\0');

	mir_sntprintf(szString, iBufSize - 1, szFormatString, wszProto, _T(": "), szContactName);


	TruncateWithDots(szString, iBufSize-1-mir_tstrlen(szStatus)-mir_tstrlen(szIs)-2); // 2 spaces around szIs
	mir_sntprintf(szString + mir_tstrlen(szString), iBufSize - 1 - mir_tstrlen(szString), _T(" %s %s"), szIs, szStatus);

	return szString;
}


void CTooltipNotify::OnTooltipDblClicked(CTooltip *pTooltip)
{
	switch(m_sOptions.bLDblClick)
	{
		case SHOW_HIDE_CLIST:
			::CallService(MS_CLIST_SHOWHIDE,0,0);
			break;

		case OPEN_MSGDLG:
		{
			TooltipsList::iterator iter = FindBy(&STooltipData::pTooltip, pTooltip);
			STooltipData* pTooltipData = *iter;
			WPARAM wParam = (WPARAM)pTooltipData->hContact;
			if (wParam)	::CallService(MS_CLIST_CONTACTDOUBLECLICKED, wParam, 0);
			break;
		}

		default:
			::CallService(MS_CLIST_SHOWHIDE,0,0);
			break;
	}		
}


BOOL CTooltipNotify::OnTooltipBeginMove(CTooltip *pTooltip)
{
	if (m_sOptions.bAutoPos) return FALSE;

	SuspendTimer(pTooltip);
	return TRUE;
}


void CTooltipNotify::OnTooltipEndMove(CTooltip *pTooltip)
{
	Utils_SaveWindowPosition(pTooltip->GetHandle(), NULL, MODULENAME, "toolwindow");
	ResumeTimer(pTooltip);
}


void CTooltipNotify::SuspendTimer(CTooltip *pTooltip)
{
	TooltipsList::iterator iter = FindBy(&STooltipData::pTooltip, pTooltip);
	assert(iter!=m_TooltipsList.end());

	STooltipData* pTooltipData = *iter;

	BOOL bSuccess = KillTimer(0, pTooltipData->idTimer);
	assert(bSuccess);
	pTooltipData->idTimer = 0;	// denote that the timer is inactive
}


void CTooltipNotify::ResumeTimer(CTooltip *pTooltip)
{
	TooltipsList::iterator iter = FindBy(&STooltipData::pTooltip, pTooltip);
	assert(iter!=m_TooltipsList.end());

	STooltipData* pTooltipData = *iter;

	UINT_PTR idTimer = SetTimer(0, 0, pTooltipData->uiTimeout, TooltipTimerProcWrapper); 
	pTooltipData->idTimer = idTimer;
}

template<typename T>
CTooltipNotify::TooltipsList::iterator CTooltipNotify::FindBy(T STooltipData::* field, const T& value)
{
	for (
		TooltipsList::iterator iter = m_TooltipsList.begin(); 
		iter != m_TooltipsList.end(); 
		++iter)
	{
		STooltipData *pTooltipData = *iter;
		if (pTooltipData->*field == value)
		{
			return iter;
		}
	}

	return m_TooltipsList.end();
}
