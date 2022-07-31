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

static const int ID_TTNTF_STATUS_TYPING = ID_STATUS_INVISIBLE + 10;
static const int ID_TTNTF_STATUS_IDLE = ID_STATUS_INVISIBLE + 11;
static const int ID_TTNTF_STATUS_NOT_IDLE = ID_STATUS_INVISIBLE + 12;

#define FONTSERV_GROUP   LPGENW("Tooltip Notify")
#define FONTSERV_ONLINE  LPGENW("Online")
#define FONTSERV_OFFLINE LPGENW("Offline")
#define FONTSERV_OTHER   LPGENW("Other status")
#define FONTSERV_TYPING  LPGENW("Typing")
#define FONTSERV_IDLE    LPGENW("Idle")

struct FontEntry
{
	int status;
	wchar_t* name;
	char* fontPrefix;
	char* clrPrefix;
};

static FontEntry s_fontTable[] =
{
	ID_STATUS_ONLINE, FONTSERV_ONLINE, "OnlineFont", "OnlineBgColor",
	ID_STATUS_OFFLINE, FONTSERV_OFFLINE, "OfflineFont", "OfflineBgColor",
	ID_TTNTF_STATUS_TYPING, FONTSERV_TYPING, "TypingFont", "TypingBgColor",
	ID_TTNTF_STATUS_IDLE, FONTSERV_IDLE, "IdleFont", "IdleBgColor",
	ID_TTNTF_STATUS_NOT_IDLE, FONTSERV_IDLE, "IdleFont", "IdleBgColor",
	0, FONTSERV_OTHER, "OtherFont", "OtherBgColor",
};

CTooltipNotify *CTooltipNotify::s_pInstance = nullptr;
const char *CTooltipNotify::s_szModuleNameOld = "ttntfmod";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTooltipNotify::CTooltipNotify()
{
	if (s_pInstance != nullptr)
		throw EAlreadyExists();

	s_pInstance = this;

	CTooltip::Initialize();
}

CTooltipNotify::~CTooltipNotify()
{
	EndNotifyAll();
	CTooltip::Deinitialize();
	s_pInstance = nullptr;
}

void CTooltipNotify::RegisterFonts()
{
	FontIDW fontId = {};
	wcsncpy(fontId.group, FONTSERV_GROUP, _countof(fontId.group) - 1);
	strncpy(fontId.dbSettingsGroup, MODULENAME, _countof(fontId.dbSettingsGroup) - 1);
	fontId.flags = FIDF_DEFAULTVALID;
	fontId.deffontsettings.colour = DEF_SETTING_TXTCOLOR;
	fontId.deffontsettings.size = -MulDiv(DEF_SETTING_FONT_SIZE, DEF_LOGPIXELSY, 72);
	fontId.deffontsettings.style = DEF_SETTING_FONT_STYLE;
	fontId.deffontsettings.charset = DEF_SETTING_FONT_CHARSET;
	wcsncpy(fontId.deffontsettings.szFace, DEF_SETTING_FONT_FACE, _countof(fontId.deffontsettings.szFace) - 1);
	fontId.order = 0;
	wcsncpy(fontId.backgroundGroup, FONTSERV_GROUP, _countof(fontId.backgroundGroup) - 1);

	ColourIDW colorId = {};
	wcsncpy(colorId.group, FONTSERV_GROUP, _countof(colorId.group) - 1);
	strncpy(colorId.dbSettingsGroup, MODULENAME, _countof(colorId.dbSettingsGroup) - 1);
	colorId.defcolour = DEF_SETTING_BGCOLOR;
	colorId.order = 0;

	for (int i = 0; i < _countof(s_fontTable); i++) {
		wcsncpy(fontId.name, s_fontTable[i].name, _countof(fontId.name) - 1);
		strncpy(fontId.setting, s_fontTable[i].fontPrefix, _countof(fontId.setting) - 1);
		wcsncpy(fontId.backgroundName, s_fontTable[i].name, _countof(fontId.backgroundName) - 1);
		::g_plugin.addFont(&fontId);

		wcsncpy(colorId.name, s_fontTable[i].name, _countof(colorId.name) - 1);
		strncpy(colorId.setting, s_fontTable[i].clrPrefix, _countof(colorId.setting) - 1);
		::g_plugin.addColor(&colorId);
	}
}

void CTooltipNotify::GetFont(int iStatus, LOGFONT* lf, COLORREF* text, COLORREF* bg)
{
	wchar_t* fontName = nullptr;
	for (int i = 0; i < _countof(s_fontTable); i++) {
		if (s_fontTable[i].status == iStatus) {
			fontName = s_fontTable[i].name;
		}
	}
	if (fontName == nullptr)
		fontName = s_fontTable[_countof(s_fontTable) - 1].name;

	// name and group only
	*text = ::Font_GetW(FONTSERV_GROUP, fontName, lf);
	*bg = ::Colour_GetW(FONTSERV_GROUP, fontName);
}

int CTooltipNotify::ModulesLoaded(WPARAM, LPARAM)
{
	MigrateSettings();
	LoadSettings();
	ValidateSettings();

	if (m_sOptions.bFirstRun) {
		db_set_b(0, "SkinSoundsOff", SND_ONLINE, 1);
		db_set_b(0, "SkinSoundsOff", SND_OFFLINE, 1);
		db_set_b(0, "SkinSoundsOff", SND_OTHER, 1);
		db_set_b(0, "SkinSoundsOff", SND_TYPING, 1);
		g_plugin.setByte("firstrun", 0);
	}

	g_plugin.addSound(SND_ONLINE,  LPGENW("Tooltip Notify"), LPGENW("Online"),  L"online.wav");
	g_plugin.addSound(SND_OFFLINE, LPGENW("Tooltip Notify"), LPGENW("Offline"), L"offline.wav");
	g_plugin.addSound(SND_OTHER,   LPGENW("Tooltip Notify"), LPGENW("Other"),   L"other.wav");
	g_plugin.addSound(SND_TYPING,  LPGENW("Tooltip Notify"), LPGENW("Typing"),  L"typing.wav");

	// register fonts
	RegisterFonts();
	return 0;
}


int CTooltipNotify::ProtoContactIsTyping(WPARAM hContact, LPARAM lParam)
{
	if (!m_sOptions.bTyping) return 0;

	if (lParam > 0) {
		STooltipData *pTooltipData = new STooltipData;
		pTooltipData->uiTimeout = lParam * 1000;
		pTooltipData->hContact = hContact;
		pTooltipData->iStatus = ID_TTNTF_STATUS_TYPING;

		EndNotifyAll();
		Skin_PlaySound(SND_TYPING);
		BeginNotify(pTooltipData);
	}
	else EndNotifyAll();

	return 0;
}


int CTooltipNotify::ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if ((ack == nullptr) || (ack->type != ACKTYPE_STATUS)) return 0;

	uint16_t wNewStatus = (uint16_t)ack->lParam;
	uint16_t wOldStatus = (UINT_PTR)ack->hProcess;
	if (wOldStatus == wNewStatus) return 0; //Useless message.

	char *szProtocol = (char *)ack->szModule;

	if (wNewStatus == ID_STATUS_OFFLINE) {
		uint8_t bProtoActive = g_plugin.getByte(szProtocol, ProtoUserBit | ProtoIntBit);
		bProtoActive &= ~ProtoIntBit;
		g_plugin.setByte(szProtocol, bProtoActive);
	}
	else {
		if (wOldStatus < ID_STATUS_ONLINE && wNewStatus > ID_STATUS_OFFLINE) {
			UINT_PTR idTimer = SetTimer(nullptr, 0, m_sOptions.wStartupDelay * 1000, ConnectionTimerProcWrapper);
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
	if (strcmp(cws->szSetting, "Status") == 0)
		idle = false;
	else if (strcmp(cws->szSetting, "IdleTS") == 0)
		idle = true;
	else return 0;

	if (Contact::IsHidden(hContact))
		return 0;

	const char *pszProto = cws->szModule;
	if (g_plugin.getByte(pszProto, ProtoUserBit | ProtoIntBit) != (ProtoUserBit | ProtoIntBit))
		return 0;

	if (!Contact::OnList(hContact) && m_sOptions.bIgnoreUnknown)
		return 0;

	if (g_plugin.getByte(hContact, CONTACT_IGNORE_TTNOTIFY, m_sOptions.bIgnoreNew))
		return 0;

	if (idle && !m_sOptions.bIdle)
		return 0;

	uint16_t wNewStatus = cws->value.wVal;
	switch (wNewStatus) {
	case ID_STATUS_OFFLINE:
		if (!m_sOptions.bOffline) return 0;
		Skin_PlaySound(SND_OFFLINE);
		break;

	case ID_STATUS_ONLINE:
		if (Ignore_IsIgnored(hContact, IGNOREEVENT_USERONLINE) && m_sOptions.bConjSOLN) return 0;
		if (!m_sOptions.bOnline) return 0;
		Skin_PlaySound(SND_ONLINE);
		break;

	default:
		if (!m_sOptions.bOther) return 0;
		Skin_PlaySound(SND_OTHER);
		break;
	}

	STooltipData *pTooltipData = new STooltipData;
	pTooltipData->uiTimeout = m_sOptions.wDuration * (wNewStatus == ID_STATUS_ONLINE ? (m_sOptions.bX2 + 1) : 1);
	pTooltipData->hContact = hContact;

	if (idle) wNewStatus = (wNewStatus != 0 ? ID_TTNTF_STATUS_IDLE : ID_TTNTF_STATUS_NOT_IDLE);
	pTooltipData->iStatus = wNewStatus;

	EndNotifyAll();
	BeginNotify(pTooltipData);

	return 0;

}

int CTooltipNotify::InitializeOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.a = LPGEN("Tooltip Notify");
	odp.szGroup.a = LPGEN("Popups");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = CTooltipNotify::OptionsDlgProcWrapper;
	::g_plugin.addOptions(wParam, &odp);
	return 0;
}

CTooltip *CTooltipNotify::BeginNotify(STooltipData *pTooltipData)
{
	wchar_t szTooltipText[64] = { 0 };
	MakeTooltipString(pTooltipData->hContact, pTooltipData->iStatus, szTooltipText, 64);

	LOGFONT lf = { 0 };
	COLORREF textColor = 0;
	COLORREF bgColor = 0;
	GetFont(pTooltipData->iStatus, &lf, &textColor, &bgColor);

	CTooltip *pTooltip = new CTooltip(this);
	pTooltip->set_Text(szTooltipText);
	pTooltip->set_Font(lf);
	pTooltip->set_BgColor(bgColor);
	pTooltip->set_TextColor(textColor);
	if (m_sOptions.bTransp)
		pTooltip->set_Translucency(m_sOptions.bAlpha);
	pTooltip->set_TransparentInput(m_sOptions.bTransp && m_sOptions.bTranspInput);
	pTooltip->Validate();

	RECT TooltipRect, WorkAreaRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkAreaRect, 0);
	pTooltip->get_Rect(&TooltipRect);

	if (m_sOptions.bAutoPos || Utils_RestoreWindowPosition(pTooltip->GetHandle(), 0, MODULENAME, "toolwindow", RWPF_NOSIZE | RWPF_NOACTIVATE))
		pTooltip->set_Position(WorkAreaRect.right - 10 - (TooltipRect.right - TooltipRect.left), WorkAreaRect.bottom - 2 - (TooltipRect.bottom - TooltipRect.top));

	UINT_PTR idTimer = SetTimer(nullptr, 0, pTooltipData->uiTimeout, TooltipTimerProcWrapper);
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
	while (mapRevIter != m_TooltipsList.rend()) {
		STooltipData* pTooltipData = *mapRevIter;
		if (EndNotify(pTooltipData)) {
			mapRevIter = m_TooltipsList.rbegin();
		}
		else {
			++mapRevIter;
		}
	}
}

CTooltipNotify::MapTimerIdProtoIter CTooltipNotify::FindProtoByTimer(UINT idTimer)
{
	for (MapTimerIdProtoIter iter = m_mapTimerIdProto.begin(); iter != m_mapTimerIdProto.end(); ++iter)
		if (iter->timerId == idTimer)
			return iter;

	return m_mapTimerIdProto.end();
}

void CTooltipNotify::OnConnectionTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	BOOL bSuccess = KillTimer(nullptr, idEvent);
	assert(bSuccess);

	MapTimerIdProtoIter iter = FindProtoByTimer(idEvent);
	assert(iter != m_mapTimerIdProto.end());

	uint8_t bProtoActive = g_plugin.getByte(iter->proto, ProtoUserBit | ProtoIntBit);
	bProtoActive |= ProtoIntBit;
	g_plugin.setByte(iter->proto, bProtoActive);

	free((char*)iter->proto);
	m_mapTimerIdProto.erase(iter);
}


void CTooltipNotify::OnTooltipTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	TooltipsList::iterator iter = FindBy(&STooltipData::idTimer, idEvent);
	assert(iter != m_TooltipsList.end());

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
	for (int i = 0; i < _countof(arSettings); i++) {
		int val = g_plugin.getWord(arSettings[i].szOldSetting, -1);
		if (val != -1) {
			g_plugin.setWord(arSettings[i].szNewSetting, val);
			g_plugin.delSetting(arSettings[i].szOldSetting);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTooltipNotify::LoadSettings()
{
	m_sOptions.bFirstRun = g_plugin.getByte("firstrun", DEF_SETTING_FIRSTRUN);
	m_sOptions.bOffline = g_plugin.getByte("offline", DEF_SETTING_OFFLINE);
	m_sOptions.bOnline = g_plugin.getByte("online", DEF_SETTING_ONLINE);
	m_sOptions.bOther = g_plugin.getByte("other", DEF_SETTING_OTHER);
	m_sOptions.bTyping = g_plugin.getByte("typing", DEF_SETTING_TYPING);
	m_sOptions.bIdle = g_plugin.getByte("idle", DEF_SETTING_TYPING);
	m_sOptions.bX2 = g_plugin.getByte("x2", DEF_SETTING_X2);
	m_sOptions.bConjSOLN = g_plugin.getByte("conjsoln", DEF_SETTING_CONJSOLN);
	m_sOptions.bAutoPos = g_plugin.getByte("autopos", DEF_SETTING_DEF_POS);
	m_sOptions.bBallonTip = g_plugin.getByte("balloontip", DEF_SETTING_BALLONTIP);
	m_sOptions.bTransp = g_plugin.getByte("transp", DEF_SETTING_TRANSP);
	m_sOptions.bAlpha = g_plugin.getByte("alpha", DEF_SETTING_ALPHA);
	m_sOptions.bTranspInput = g_plugin.getByte("transpinput", DEF_SETTING_TRANSP_INPUT);
	m_sOptions.bPrefixProto = g_plugin.getByte("prfxproto", DEF_SETTING_PREFIX_PROTO);
	m_sOptions.bLDblClick = g_plugin.getByte("ldblclick", DEF_SETTING_LDBLCLICK);
	m_sOptions.wDuration = g_plugin.getWord("duration", DEF_SETTING_DURATION);
	m_sOptions.wStartupDelay = g_plugin.getWord("suprconndelay", DEF_SETTING_STARTUP_DELAY);
	m_sOptions.bIgnoreUnknown = g_plugin.getByte("ignoreunknown", DEF_SETTING_IGNORE_UNKNOWN);
	m_sOptions.bIgnoreNew = g_plugin.getByte("ignorenew", DEF_SETTING_IGNORE_NEW);
}

void CTooltipNotify::SaveSettings()
{
	g_plugin.setWord("duration", m_sOptions.wDuration);
	g_plugin.setWord("suprconndelay", m_sOptions.wStartupDelay);
	g_plugin.setByte("offline", m_sOptions.bOffline);
	g_plugin.setByte("online", m_sOptions.bOnline);
	g_plugin.setByte("other", m_sOptions.bOther);
	g_plugin.setByte("typing", m_sOptions.bTyping);
	g_plugin.setByte("idle", m_sOptions.bIdle);
	g_plugin.setByte("prfxproto", m_sOptions.bPrefixProto);
	g_plugin.setByte("x2", m_sOptions.bX2);
	g_plugin.setByte("conjsoln", m_sOptions.bConjSOLN);
	g_plugin.setByte("autopos", m_sOptions.bAutoPos);
	g_plugin.setByte("balloontip", m_sOptions.bBallonTip);
	g_plugin.setByte("transp", m_sOptions.bTransp);
	g_plugin.setByte("alpha", m_sOptions.bAlpha);
	g_plugin.setByte("transpinput", m_sOptions.bTranspInput);
	g_plugin.setByte("ldblclick", m_sOptions.bLDblClick);
	g_plugin.setByte("ignoreunknown", m_sOptions.bIgnoreUnknown);
	g_plugin.setByte("ignorenew", m_sOptions.bIgnoreNew);
}


void CTooltipNotify::ReadSettingsFromDlg(HWND hDlg)
{
	m_sOptions.bOffline = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_OFFLINE) == BST_CHECKED ? 1 : 0);
	m_sOptions.bOnline = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_ONLINE) == BST_CHECKED ? 1 : 0);
	m_sOptions.bOther = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_OTHER) == BST_CHECKED ? 1 : 0);
	m_sOptions.bTyping = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_TYPING) == BST_CHECKED ? 1 : 0);
	m_sOptions.bIdle = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_IDLE) == BST_CHECKED ? 1 : 0);
	m_sOptions.bX2 = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_X2) == BST_CHECKED ? 1 : 0);
	m_sOptions.bConjSOLN = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_CONJSOLN) == BST_CHECKED ? 1 : 0);
	m_sOptions.bAutoPos = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_AUTOPOS) == BST_CHECKED ? 1 : 0);
	m_sOptions.bBallonTip = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED ? 1 : 0);
	m_sOptions.bTransp = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_TRANSPARENCY) == BST_CHECKED ? 1 : 0);
	m_sOptions.bAlpha = (uint8_t)SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_GETPOS, 0, 0);
	m_sOptions.bTranspInput = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT) == BST_CHECKED ? 1 : 0);
	m_sOptions.bPrefixProto = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_PREFIX_PROTO) == BST_CHECKED ? 1 : 0);
	m_sOptions.bLDblClick = (uint8_t)(IsDlgButtonChecked(hDlg, IDC_RB_CLIST) == BST_CHECKED) ? SHOW_HIDE_CLIST : OPEN_MSGDLG;
	m_sOptions.wDuration = LOWORD(SendDlgItemMessage(hDlg, IDC_DURATIONSPIN, UDM_GETPOS, 0, 0));
	m_sOptions.wStartupDelay = LOWORD(SendDlgItemMessage(hDlg, IDC_DELAYONCONNSPIN, UDM_GETPOS, 0, 0));
}

void CTooltipNotify::WriteSettingsToDlg(HWND hDlg)
{
	SendDlgItemMessage(hDlg, IDC_DURATIONSPIN, UDM_SETRANGE, 0, MAKELONG(550 * 36, 550));
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

	switch (m_sOptions.bLDblClick) {
	case SHOW_HIDE_CLIST:	CheckDlgButton(hDlg, IDC_RB_CLIST, BST_CHECKED);	break;
	case OPEN_MSGDLG:	CheckDlgButton(hDlg, IDC_RB_MSGDLG, BST_CHECKED);	break;
	default:		CheckDlgButton(hDlg, IDC_RB_CLIST, BST_CHECKED);	break;
	}

	EnableWindow(GetDlgItem(hDlg, IDC_TRANSPARENCY_SLIDER), m_sOptions.bTransp);
	EnableWindow(GetDlgItem(hDlg, IDC_TRANSPERC), m_sOptions.bTransp);
	EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_INPUT), m_sOptions.bTransp);
	EnableWindow(GetDlgItem(hDlg, IDC_RB_CLIST), !m_sOptions.bTranspInput);
	EnableWindow(GetDlgItem(hDlg, IDC_RB_MSGDLG), !m_sOptions.bTranspInput);

	SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
	SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_SETPOS, TRUE, m_sOptions.bAlpha);
}

void CTooltipNotify::ValidateSettings()
{
	if (m_sOptions.wStartupDelay > 30) m_sOptions.wStartupDelay = 30;
	if (m_sOptions.wDuration > 550 * 36) m_sOptions.wDuration = 550 * 36;
	if (m_sOptions.wDuration < 550 * 1) m_sOptions.wDuration = 550 * 1;
	if (!m_sOptions.bTransp) m_sOptions.bTranspInput = 0;
}

// main options dialog
BOOL CTooltipNotify::OptionsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		WriteSettingsToDlg(hDlg);
		SendMessage(hDlg, WM_HSCROLL, 0x12345678, 0);
		return TRUE;

	case WM_VSCROLL:
	case WM_HSCROLL:
		wchar_t str[10];
		mir_snwprintf(str, L"%d%%", 100 * SendDlgItemMessage(hDlg, IDC_TRANSPARENCY_SLIDER, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemText(hDlg, IDC_TRANSPERC, str);
		if (wParam != 0x12345678)
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
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
			EnableWindow(GetDlgItem(hDlg, IDC_RB_CLIST), !(IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT) == BST_CHECKED));
			EnableWindow(GetDlgItem(hDlg, IDC_RB_MSGDLG), !(IsDlgButtonChecked(hDlg, IDC_TRANSP_INPUT) == BST_CHECKED));
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BALLONTIP:
			EnableWindow(GetDlgItem(hDlg, IDC_DURATION), !(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED));
			EnableWindow(GetDlgItem(hDlg, IDC_X2), !(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED));
			EnableWindow(GetDlgItem(hDlg, IDC_RB_CLIST), !(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED));
			EnableWindow(GetDlgItem(hDlg, IDC_RB_MSGDLG), !(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED));
			EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_INPUT), !(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED));
			EnableWindow(GetDlgItem(hDlg, IDC_AUTOPOS), !(IsDlgButtonChecked(hDlg, IDC_BALLONTIP) == BST_CHECKED));
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
				pTooltipData->uiTimeout = m_sOptions.wDuration * (m_sOptions.bX2 ? (m_sOptions.bX2 + 1) : 1);
				pTooltipData->hContact = 0;
				pTooltipData->iStatus = ID_STATUS_ONLINE;

				EndNotifyAll();
				BeginNotify(pTooltipData);
				break;
			}

		case IDC_SEL_PROTO:
			DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_PROTOS), hDlg, CTooltipNotify::ProtosDlgProcWrapper);
			break;

		case IDC_IGNORE:
			DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONTACTS), hDlg, CTooltipNotify::ContactsDlgProcWrapper);
			break;

		default:
			// activate 'apply' button
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		// Here we have pressed either the OK or the APPLY button.
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				ReadSettingsFromDlg(hDlg);
				SaveSettings();
				return TRUE;

			} // switch code
			break;

		} //switch idFrom

		return TRUE;
	}

	return FALSE;
}

// dialog for protocols selecting
BOOL CTooltipNotify::ProtosDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {

	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{
			ListView_SetExtendedListViewStyle(GetDlgItem(hDlg, IDC_PROTOS), LVS_EX_CHECKBOXES);

			// enum protocols currently running						
			auto &accs = Accounts();
			for (auto &pa : accs) {
				LV_ITEM lvi;

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 0;
				lvi.iItem = lvi.lParam = accs.indexOf(&pa);

				wchar_t wszProto[128];
				long lLen = MultiByteToWideChar(CP_ACP, 0, pa->szModuleName, (int)mir_strlen(pa->szModuleName), wszProto, _countof(wszProto));
				wszProto[lLen] = L'\0';

				lvi.pszText = wszProto;

				ListView_InsertItem(GetDlgItem(hDlg, IDC_PROTOS), &lvi);

				uint8_t bProtoState = g_plugin.getByte(pa->szModuleName, ProtoUserBit | ProtoIntBit);
				BOOL bProtoEnabled = (bProtoState & ProtoUserBit) != 0; 
				ListView_SetCheckState(GetDlgItem(hDlg, IDC_PROTOS), lvi.iItem, bProtoEnabled);
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			int proto_count = ListView_GetItemCount(GetDlgItem(hDlg, IDC_PROTOS));

			for (int i = 0; i < proto_count; i++) {
				wchar_t szProto[64];

				ListView_GetItemText(GetDlgItem(hDlg, IDC_PROTOS), i, 0, szProto, _countof(szProto));

				char szMultiByteProto[128];
				long lLen = WideCharToMultiByte(CP_ACP, 0, szProto, (int)mir_wstrlen(szProto),
					szMultiByteProto, sizeof(szMultiByteProto), nullptr, nullptr);
				szMultiByteProto[lLen] = '\0';

				uint8_t bProtoState = g_plugin.getByte(szMultiByteProto, ProtoUserBit | ProtoIntBit);


				BOOL bProtoEnabled = ListView_GetCheckState(GetDlgItem(hDlg, IDC_PROTOS), i);
				bProtoState = bProtoEnabled ? bProtoState | ProtoUserBit : bProtoState&~ProtoUserBit;

				g_plugin.setByte(szMultiByteProto, bProtoState);

			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	}

	return FALSE;
}

void CTooltipNotify::ResetCList(HWND hwndDlg)
{
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, Clist::UseGroups, 0);
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

void CTooltipNotify::LoadList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew && !m_sOptions.bIgnoreNew)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

	if (hItemUnknown && !m_sOptions.bIgnoreUnknown)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem && !g_plugin.getByte(hContact, CONTACT_IGNORE_TTNOTIFY, m_sOptions.bIgnoreNew))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
	}
}

void CTooltipNotify::SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew)
		m_sOptions.bIgnoreNew = (uint8_t)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 0 : 1);

	if (hItemUnknown)
		m_sOptions.bIgnoreUnknown = (uint8_t)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 0 : 1);

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem) {
			uint8_t bChecked = (uint8_t)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0));
			g_plugin.setByte(hContact, CONTACT_IGNORE_TTNOTIFY, bChecked ? 0 : 1);
		}
	}
}


// dialog for ignore tooltip notifications
BOOL CTooltipNotify::ContactsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM)
{
	static HANDLE hItemNew, hItemUnknown;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{
			CLCINFOITEM cii = { 0 };
			cii.cbSize = sizeof(cii);
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
			cii.pszText = TranslateT("** New contacts **");
			hItemNew = (HANDLE)SendDlgItemMessage(hDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)& cii);
			cii.pszText = TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)& cii);

			ResetCList(hDlg);
			LoadList(hDlg, hItemNew, hItemUnknown);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			SaveList(hDlg, hItemNew, hItemUnknown);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	}

	return FALSE;

}

wchar_t* CTooltipNotify::StatusToString(int iStatus, wchar_t *szStatus, int iBufSize)
{
	if (iStatus >= ID_STATUS_OFFLINE && iStatus <= ID_STATUS_MAX)
		mir_wstrncpy(szStatus, Clist_GetStatusModeDescription(iStatus, 0), iBufSize);
	else {
		switch (iStatus) {
		case ID_TTNTF_STATUS_TYPING:
			mir_wstrncpy(szStatus, TranslateT("Typing"), iBufSize);
			break;

		case ID_TTNTF_STATUS_IDLE:
			mir_wstrncpy(szStatus, TranslateT("Idle"), iBufSize);
			break;

		case ID_TTNTF_STATUS_NOT_IDLE:
			mir_wstrncpy(szStatus, TranslateT("Not Idle"), iBufSize);
			break;

		default:
			mir_wstrncpy(szStatus, TranslateT("Unknown"), iBufSize);
			break;
		}
	}
	return szStatus;
}

wchar_t* CTooltipNotify::MakeTooltipString(MCONTACT hContact, int iStatus, wchar_t *szString, int iBufSize)
{
	wchar_t szStatus[32];
	StatusToString(iStatus, szStatus, _countof(szStatus));

	// "proro: user is online"
	const wchar_t *szFormatString = m_sOptions.bPrefixProto ? L"%s%s%s" : L"%.0s%.0s%s";
	const wchar_t* szIs = TranslateT("is");

	const char* szProto = hContact == 0 ? "Proto" : ::Proto_GetBaseAccountName(hContact);
	const wchar_t* szContactName = ::Clist_GetContactDisplayName(hContact);

	memset(szString, 0, iBufSize*sizeof(wchar_t));


	wchar_t wszProto[32];
	long lLen = MultiByteToWideChar(CP_ACP, 0, szProto, (int)mir_strlen(szProto), wszProto, _countof(wszProto));
	wszProto[lLen] = '\0';

	mir_snwprintf(szString, iBufSize - 1, szFormatString, wszProto, L": ", szContactName);


	TruncateWithDots(szString, iBufSize - 1 - mir_wstrlen(szStatus) - mir_wstrlen(szIs) - 2); // 2 spaces around szIs
	mir_snwprintf(szString + mir_wstrlen(szString), iBufSize - 1 - mir_wstrlen(szString), L" %s %s", szIs, szStatus);

	return szString;
}


void CTooltipNotify::OnTooltipDblClicked(CTooltip *pTooltip)
{
	switch (m_sOptions.bLDblClick) {
	case SHOW_HIDE_CLIST:
		g_clistApi.pfnShowHide();
		break;

	case OPEN_MSGDLG:
		{
			TooltipsList::iterator iter = FindBy(&STooltipData::pTooltip, pTooltip);
			STooltipData* pTooltipData = *iter;
			WPARAM wParam = (WPARAM)pTooltipData->hContact;
			if (wParam)
				Clist_ContactDoubleClicked(wParam);
			break;
		}

	default:
		g_clistApi.pfnShowHide();
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
	assert(iter != m_TooltipsList.end());

	STooltipData* pTooltipData = *iter;

	BOOL bSuccess = KillTimer(nullptr, pTooltipData->idTimer);
	assert(bSuccess);
	pTooltipData->idTimer = 0;	// denote that the timer is inactive
}


void CTooltipNotify::ResumeTimer(CTooltip *pTooltip)
{
	TooltipsList::iterator iter = FindBy(&STooltipData::pTooltip, pTooltip);
	assert(iter != m_TooltipsList.end());

	STooltipData* pTooltipData = *iter;

	UINT_PTR idTimer = SetTimer(nullptr, 0, pTooltipData->uiTimeout, TooltipTimerProcWrapper);
	pTooltipData->idTimer = idTimer;
}

template<typename T>
CTooltipNotify::TooltipsList::iterator CTooltipNotify::FindBy(T STooltipData::* field, const T& value)
{
	for (TooltipsList::iterator iter = m_TooltipsList.begin(); iter != m_TooltipsList.end(); ++iter) {
		STooltipData *pTooltipData = *iter;
		if (pTooltipData->*field == value)
			return iter;
	}

	return m_TooltipsList.end();
}
