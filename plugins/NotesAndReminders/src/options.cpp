#include "stdafx.h"

// min allowed alpha (don't want 0 because it's a waste of resources as well as might confuse user)
#define MIN_ALPHA 30

HICON g_hReminderIcon = nullptr;

LOGFONT lfBody, lfCaption;
HFONT hBodyFont = nullptr, hCaptionFont = nullptr;
long BodyColor;
COLORREF CaptionFontColor, BodyFontColor;
int g_NoteTitleDate, g_NoteTitleTime;
int g_NoteWidth, g_NoteHeight;
int g_Transparency;
char *g_RemindSMS = nullptr;

char *g_lpszAltBrowser = nullptr;

#define NRCDEFAULT_BODYCLR		RGB(255,255,0)

struct DateFormat
{
	wchar_t *lpszUI;
	wchar_t *lpszFmt;
}
static dateFormats[] =
{
	{ L"1981-12-31",  L"yyyy'-'MM'-'dd"  },
	{ L"31-12-1981",  L"dd'-'MM'-'yyyy"  },
	{ L"12-31-1981",  L"MM'-'dd'-'yyyy"  },
	{ L"1981-dec-31", L"yyyy'-'MMM'-'dd" },
	{ L"31-dec-1981", L"dd'-'MMM'-'yyyy" },
	{ L"dec-31-1981", L"MMM'-'dd'-'yyyy" },
	{ L"1981/12/31",  L"yyyy'/'MM'/'dd"  },
	{ L"31/12/1981",  L"dd'/'MM'/'yyyy"  },
	{ L"12/31/1981",  L"MM'/'dd'/'yyyy"  },
	{ L"1981/dec/31", L"yyyy'/'MMM'/'dd" },
	{ L"31/dec/1981", L"dd'/'MMM'/'yyyy" },
	{ L"dec/31/1981", L"MMM'/'dd'/'yyyy" },
	{ L"1981 dec 31", L"yyyy MMM dd"     },
	{ L"31 dec 1981", L"dd MMM yyyy"     },
	{ L"dec 31 1981", L"MMM dd yyyy"     }
};

struct TimeFormat
{
	wchar_t *lpszUI;
	wchar_t *lpszFmt;
}
static timeFormats[] =
{
	{ L"19:30:00",   L"HH':'mm':'ss"    },
	{ L"19:30",      L"HH':'mm'"        },
	{ L"7:30:00 PM", L"hh':'mm':'ss tt" },
	{ L"7:30 PM",    L"hh':'mm tt"      },
	{ L"7:30:00P",   L"hh':'mm':'sst"   },
	{ L"7:30P",      L"hh':'mmt"        }
};

struct FontOptionsList
{
	wchar_t *szDescr;
	COLORREF defColour;
	wchar_t *szDefFace;
	uint8_t defStyle;
	int  defSize;
	wchar_t *szBkgName;
}
static fontOptionsList[] =
{
	{ LPGENW("Sticky Note Caption"), RGB(0, 0, 0), L"Small Fonts", 0, 7, LPGENW("Sticky Note Background Color") },
	{ LPGENW("Sticky Note Body"), RGB(0, 0, 0), L"System", DBFONTF_BOLD, 10, LPGENW("Sticky Note Background Color") },
};


struct ColourOptionsList
{
	char *szName;
	char *szSettingName;
	COLORREF defColour;
}
static colourOptionsList[] =
{
	{ LPGEN("Sticky Note Background Color"), "BodyColor", NRCDEFAULT_BODYCLR }
};


wchar_t* GetDateFormatStr()
{
	return dateFormats[g_NoteTitleDate ? g_NoteTitleDate - 1 : 0].lpszFmt;
}

wchar_t* GetTimeFormatStr()
{
	return timeFormats[g_NoteTitleTime ? g_NoteTitleTime - 1 : 0].lpszFmt;
}

static void InitFonts()
{
	memset(&lfBody, 0, sizeof(lfBody));
	memset(&lfCaption, 0, sizeof(lfCaption));

	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, &CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, &BodyFontColor);

	if (hBodyFont)
		DeleteObject(hBodyFont);
	if (hCaptionFont)
		DeleteObject(hCaptionFont);

	hBodyFont = CreateFontIndirectW(&lfBody);
	hCaptionFont = CreateFontIndirectW(&lfCaption);
}

static int FS_FontsChanged(WPARAM, LPARAM)
{
	InitFonts();

	SaveNotes();
	LoadNotes(false);
	return 0;
}

static int FS_ColorChanged(WPARAM, LPARAM)
{
	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, (COLORREF*)&CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, (COLORREF*)&BodyFontColor);

	BodyColor = g_plugin.getDword(colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

	SaveNotes();
	LoadNotes(false);
	return 0;
}

void RegisterFontServiceFonts()
{
	char szTemp[100];

	FontIDW fontid = {};
	wcsncpy_s(fontid.group, _A2W(SECTIONNAME), _TRUNCATE);
	wcsncpy_s(fontid.backgroundGroup, _A2W(SECTIONNAME), _TRUNCATE);
	strncpy_s(fontid.dbSettingsGroup, MODULENAME, _TRUNCATE);
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_SAVEPOINTSIZE;

	HDC hDC = GetDC(nullptr);
	int nFontScale = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(nullptr, hDC);

	for (int i = 0; i < _countof(fontOptionsList); i++) {
		fontid.order = i;
		mir_snprintf(szTemp, "Font%d", i);
		strncpy_s(fontid.setting, szTemp, _countof(fontid.setting));
		wcsncpy_s(fontid.name, fontOptionsList[i].szDescr, _TRUNCATE);

		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = (char)-MulDiv(fontOptionsList[i].defSize, nFontScale, 72);
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		fontid.deffontsettings.charset = DEFAULT_CHARSET;

		wcsncpy_s(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _TRUNCATE);
		wcsncpy_s(fontid.backgroundName, fontOptionsList[i].szBkgName, _TRUNCATE);
		g_plugin.addFont(&fontid);
	}

	ColourID colorid = {};
	strncpy_s(colorid.group, SECTIONNAME, _TRUNCATE);
	strncpy_s(colorid.dbSettingsGroup, MODULENAME, _TRUNCATE);

	for (int i = 0; i < _countof(colourOptionsList); i++) {
		colorid.order = i;
		strncpy_s(colorid.name, colourOptionsList[i].szName, _TRUNCATE);
		colorid.defcolour = colourOptionsList[i].defColour;
		strncpy_s(colorid.setting, colourOptionsList[i].szSettingName, _TRUNCATE);
		g_plugin.addColor(&colorid);
	}

	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
	HookEvent(ME_COLOUR_RELOAD, FS_ColorChanged);
}

void LoadNRFont(int i, LOGFONT *lf, COLORREF *colour)
{
	COLORREF col = Font_GetW(_A2W(SECTIONNAME), fontOptionsList[i].szDescr, lf);
	if (colour)
		*colour = col;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CNROptionsDlg : public CDlgBase
{
	void FillValues()
	{
		chkHide.SetState(!g_plugin.bShowNotesAtStart); // reversed

		SetDlgItemInt(m_hwnd, IDC_EDIT_WIDTH, g_NoteWidth, FALSE);
		SetDlgItemInt(m_hwnd, IDC_EDIT_HEIGHT, g_NoteHeight, FALSE);

		cmbDate.SetCurSel(g_NoteTitleDate ? g_NoteTitleDate - 1 : cmbDate.GetCount()-1);
		cmbTime.SetCurSel(g_NoteTitleTime ? g_NoteTitleTime - 1 : cmbTime.GetCount()-1);

		slider.SetPosition(255 - g_Transparency);
	}

	CCtrlEdit edtBrowser, edtEmail;
	CCtrlCombo cmbDate, cmbTime;
	CCtrlCheck chkHide, chkButtons, chkScroll, chkClose, chkMSI;
	CCtrlButton btnBrowse, btnReset;
	CCtrlSlider slider;

public:
	CNROptionsDlg() :
		CDlgBase(g_plugin, IDD_STNOTEOPTIONS),
		slider(this, IDC_SLIDER_TRANSPARENCY, 255 - MIN_ALPHA),
		edtEmail(this, IDC_EDIT_EMAILSMS),
		edtBrowser(this, IDC_EDIT_ALTBROWSER),
		cmbDate(this, IDC_COMBODATE),
		cmbTime(this, IDC_COMBOTIME),
		chkMSI(this, IDC_CHECK_MSI),
		chkHide(this, IDC_CHECK_HIDENOTES),
		chkClose(this, IDC_CHECK_CLOSE),
		chkScroll(this, IDC_CHECK_SCROLLBARS),
		chkButtons(this, IDC_CHECK_BUTTONS),
		btnReset(this, IDC_BUTTON_RESET),
		btnBrowse(this, IDC_BTN_BROWSEBROWSER)
	{
		btnReset.OnClick = Callback(this, &CNROptionsDlg::onClick_Reset);
		btnBrowse.OnClick = Callback(this, &CNROptionsDlg::onClick_Browse);

		CreateLink(chkMSI, g_plugin.bUseMSI);
		CreateLink(chkClose, g_plugin.bCloseAfterAddReminder);
		CreateLink(chkScroll, g_plugin.bShowScrollbar);
		CreateLink(chkButtons, g_plugin.bShowNoteButtons);
	}

	bool OnInitDialog() override
	{
		cmbDate.ResetContent();
		for (auto &it : dateFormats)
			cmbDate.AddString(it.lpszUI);
		cmbDate.AddString(TranslateT("None"));

		cmbTime.ResetContent();
		for (auto &it : timeFormats)
			cmbTime.AddString(it.lpszUI);
		cmbTime.AddString(TranslateT("None"));

		FillValues();

		if (g_RemindSMS)
			edtEmail.SetTextA(g_RemindSMS);
		else
			edtEmail.SetTextA("");

		edtBrowser.SetTextA(g_lpszAltBrowser ? g_lpszAltBrowser : "");
		return true;
	}

	bool OnApply() override
	{
		g_plugin.bShowNotesAtStart = !chkHide.GetState(); // reversed

		BOOL LB;
		g_NoteWidth = GetDlgItemInt(m_hwnd, IDC_EDIT_WIDTH, &LB, FALSE);
		g_NoteHeight = GetDlgItemInt(m_hwnd, IDC_EDIT_HEIGHT, &LB, FALSE);
		g_Transparency = 255 - slider.GetPosition();
		
		g_NoteTitleDate = (cmbDate.GetCurSel()+1) % cmbDate.GetCount();
		g_NoteTitleTime = (cmbTime.GetCurSel()+1) % cmbTime.GetCount();
		if (g_NoteWidth < 179) {
			g_NoteWidth = 179;
			SetDlgItemInt(m_hwnd, IDC_EDIT_WIDTH, g_NoteWidth, FALSE);
		}
		
		if (g_NoteHeight < 35) {
			g_NoteHeight = 35;
			SetDlgItemInt(m_hwnd, IDC_EDIT_HEIGHT, g_NoteHeight, FALSE);
		}

		mir_free(g_RemindSMS);
		g_RemindSMS = edtEmail.GetTextA();
		g_plugin.setString("RemindEmail", g_RemindSMS);

		mir_free(g_lpszAltBrowser);
		g_lpszAltBrowser = edtBrowser.GetTextA();
		g_plugin.setString("AltBrowser", g_lpszAltBrowser);

		g_plugin.setDword("NoteWidth", g_NoteWidth);
		g_plugin.setDword("NoteHeight", g_NoteHeight);
		g_plugin.setDword("Transparency", g_Transparency);
		g_plugin.setDword("NoteTitleDate", g_NoteTitleDate);
		g_plugin.setDword("NoteTitleTime", g_NoteTitleTime);

		SaveNotes();
		LoadNotes(false);
		return true;
	}

	void onClick_Browse(CCtrlButton*)
	{
		wchar_t s[MAX_PATH];
		GetDlgItemText(m_hwnd, IDC_EDIT_ALTBROWSER, s, _countof(s));

		OPENFILENAME ofn = {0};
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFilter = TranslateT("Executable Files\0*.exe\0All Files\0*.*\0\0");
		ofn.lpstrFile = s;
		ofn.nMaxFile = _countof(s);
		ofn.lpstrTitle = TranslateT("Select Executable");
		ofn.lpstrInitialDir = L".";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_DONTADDTORECENT;

		if (GetOpenFileNameW(&ofn)) {
			SetDlgItemText(m_hwnd, IDC_EDIT_ALTBROWSER, s);
			NotifyChange();
		}
	}

	void onClick_Reset(CCtrlButton*)
	{
		edtEmail.SetTextA("");
		edtBrowser.SetTextA("");

		replaceStr(g_RemindSMS, nullptr);
		replaceStr(g_lpszAltBrowser, nullptr);

		g_plugin.bShowNotesAtStart = g_plugin.bShowScrollbar = g_plugin.bShowNoteButtons = true;
		g_plugin.bCloseAfterAddReminder = g_plugin.bUseMSI = true;

		g_NoteTitleDate = 1;
		g_NoteTitleTime = 1;
		g_NoteWidth = 179;
		g_NoteHeight = 35;
		g_Transparency = 255;
		FillValues();
		NotifyChange();
	}
};

int OnOptInitialise(WPARAM w, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 900002000;
	odp.szTitle.a = SECTIONNAME;
	odp.szGroup.a = LPGEN("Plugins");
	odp.pDialog = new CNROptionsDlg();
	g_plugin.addOptions(w, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitSettings(void)
{
	g_RemindSMS = g_plugin.getStringA("RemindEmail");
	g_lpszAltBrowser = g_plugin.getStringA("AltBrowser");

	g_NoteWidth = g_plugin.getDword("NoteWidth", 179);
	g_NoteHeight = g_plugin.getDword("NoteHeight", 50);
	g_Transparency = g_plugin.getDword("Transparency", 255);
	g_NoteTitleDate = g_plugin.getDword("NoteTitleDate", 1);
	g_NoteTitleTime = g_plugin.getDword("NoteTitleTime", 1);

	BodyColor = g_plugin.getDword(colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

	InitFonts();

	g_hReminderIcon = IcoLib_GetIconByHandle(iconList[10].hIcolib);

	if (g_Transparency < MIN_ALPHA)
		g_Transparency = MIN_ALPHA;
	else if (g_Transparency > 255)
		g_Transparency = 255;
}

void TermSettings(void)
{
	if (g_lpszAltBrowser) {
		mir_free(g_lpszAltBrowser);
		g_lpszAltBrowser = nullptr;
	}
}
