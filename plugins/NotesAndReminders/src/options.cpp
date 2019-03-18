#include "stdafx.h"

// min allowed alpha (don't want 0 because it's a waste of resources as well as might confuse user)
#define MIN_ALPHA 30

HICON g_hReminderIcon = nullptr;

LOGFONTA lfBody, lfCaption;
HFONT hBodyFont = nullptr, hCaptionFont = nullptr;
long BodyColor;
long CaptionFontColor, BodyFontColor;
int g_NoteTitleDate, g_NoteTitleTime;
int g_NoteWidth, g_NoteHeight;
int g_Transparency;
char *g_RemindSMS = nullptr;

char *g_lpszAltBrowser = nullptr;

int g_reminderListGeom[4] = {0};
int g_reminderListColGeom[2] = {150, 205};
int g_notesListGeom[4] = {0};
int g_notesListColGeom[4] = {150, 20, 20, 165};

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
	char *szDescr;
	COLORREF defColour;
	char *szDefFace;
	BYTE defStyle;
	int  defSize;
	char *szBkgName;
}
static fontOptionsList[] =
{
	{ LPGEN("Sticky Note Caption"), RGB(0, 0, 0), "Small Fonts", 0, 7, LPGEN("Sticky Note Background Color") },
	{ LPGEN("Sticky Note Body"), RGB(0, 0, 0), "System", DBFONTF_BOLD, 10, LPGEN("Sticky Note Background Color") },
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

	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, (COLORREF*)&CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, (COLORREF*)&BodyFontColor);

	if (hBodyFont)
		DeleteObject(hBodyFont);
	if (hCaptionFont)
		DeleteObject(hCaptionFont);

	hBodyFont = CreateFontIndirectA(&lfBody);
	hCaptionFont = CreateFontIndirectA(&lfCaption);
}


static int FS_FontsChanged(WPARAM, LPARAM)
{
	InitFonts();

	SaveNotes();
	LoadNotes(FALSE);

	return 0;
}

static int FS_ColorChanged(WPARAM, LPARAM)
{
	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, (COLORREF*)&CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, (COLORREF*)&BodyFontColor);

	BodyColor = g_plugin.getDword(colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

	SaveNotes();
	LoadNotes(FALSE);

	return 0;
}

void RegisterFontServiceFonts()
{
	char szTemp[100];

	FontID fontid = {};
	strncpy(fontid.group, SECTIONNAME, _countof(fontid.group));
	strncpy(fontid.backgroundGroup, SECTIONNAME, _countof(fontid.backgroundGroup));
	strncpy(fontid.dbSettingsGroup, MODULENAME, _countof(fontid.dbSettingsGroup));
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_SAVEPOINTSIZE;

	HDC hDC = GetDC(nullptr);
	int nFontScale = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(nullptr, hDC);

	for (int i = 0; i < _countof(fontOptionsList); i++) {
		fontid.order = i;
		mir_snprintf(szTemp, "Font%d", i);
		strncpy(fontid.setting, szTemp, _countof(fontid.setting));
		strncpy(fontid.name, fontOptionsList[i].szDescr, _countof(fontid.name));
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;

		fontid.deffontsettings.size = (char)-MulDiv(fontOptionsList[i].defSize, nFontScale, 72);
		//fontid.deffontsettings.size = fontOptionsList[i].defSize;

		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		fontid.deffontsettings.charset = DEFAULT_CHARSET;
		strncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _countof(fontid.deffontsettings.szFace));
		strncpy(fontid.backgroundName, fontOptionsList[i].szBkgName, _countof(fontid.backgroundName));

		g_plugin.addFont(&fontid);
	}

	ColourID colorid = {};
	strncpy(colorid.group, SECTIONNAME, _countof(colorid.group));
	strncpy(colorid.dbSettingsGroup, MODULENAME, _countof(fontid.dbSettingsGroup));
	colorid.flags = 0;

	for (int i = 0; i < _countof(colourOptionsList); i++) {
		colorid.order = i;
		strncpy(colorid.name, colourOptionsList[i].szName, _countof(colorid.name));
		colorid.defcolour = colourOptionsList[i].defColour;
		strncpy(colorid.setting, colourOptionsList[i].szSettingName, _countof(colorid.setting));

		g_plugin.addColor(&colorid);
	}

	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
	HookEvent(ME_COLOUR_RELOAD, FS_ColorChanged);
}

void LoadNRFont(int i, LOGFONTA *lf, COLORREF *colour)
{
	COLORREF col = Font_Get(SECTIONNAME, fontOptionsList[i].szDescr, lf);
	if (colour)
		*colour = col;
}


static void TrimString(char *s)
{
	if (!s || !*s)
		return;

	char *start = s;
	UINT n = UINT(mir_strlen(s) - 1);

	char *end = s + n;

	if (!iswspace(*start) && !iswspace(*end)) {
		// nothing to trim
		return;
	}

	// scan past leading spaces
	while (*start && iswspace(*start)) start++;

	if (!*start) {
		// empty string
		*s = 0;
		return;
	}

	// trim trailing spaces
	while (iswspace(*end)) end--;
	end[1] = 0;

	if (start > s) {
		// remove leading spaces
		memmove(s, start, ((UINT)(end - start) + 2) * sizeof(wchar_t));
	}
}

static void FillValues(HWND hdlg)
{
	CheckDlgButton(hdlg, IDC_CHECK_HIDENOTES, g_plugin.bShowNotesAtStart ? BST_UNCHECKED : BST_CHECKED); // reversed
	CheckDlgButton(hdlg, IDC_CHECK_MENUS, g_plugin.bAddContListMI ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hdlg, IDC_CHECK_BUTTONS, g_plugin.bShowNoteButtons ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hdlg, IDC_CHECK_SCROLLBARS, g_plugin.bShowScrollbar ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hdlg, IDC_CHECK_CLOSE, g_plugin.bCloseAfterAddReminder ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hdlg, IDC_CHECK_MSI, g_plugin.bUseMSI ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hdlg, IDC_EDIT_WIDTH, g_NoteWidth, FALSE);
	SetDlgItemInt(hdlg, IDC_EDIT_HEIGHT, g_NoteHeight, FALSE);

	SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_SETCURSEL, (WPARAM)(g_NoteTitleDate ? g_NoteTitleDate - 1 : SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_GETCOUNT, 0, 0) - 1), 0);
	SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_SETCURSEL, (WPARAM)(g_NoteTitleTime ? g_NoteTitleTime - 1 : SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_GETCOUNT, 0, 0) - 1), 0);

	SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_SETPOS, TRUE, 255 - g_Transparency);
}

static INT_PTR CALLBACK DlgProcOptions(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_SETRANGE, TRUE, MAKELONG(0, 255 - MIN_ALPHA));

		FillValues(hdlg);

		SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_RESETCONTENT, 0, 0);
		for (auto &it : dateFormats)
			SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_ADDSTRING, 0, (LPARAM)it.lpszUI);
		for (auto &it : timeFormats)
			SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_ADDSTRING, 0, (LPARAM)it.lpszUI);
		SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));
		SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));

		if (g_RemindSMS)
			SetDlgItemTextA(hdlg, IDC_EDIT_EMAILSMS, g_RemindSMS);
		else
			SetDlgItemTextA(hdlg, IDC_EDIT_EMAILSMS, "");

		SetDlgItemTextA(hdlg, IDC_EDIT_ALTBROWSER, g_lpszAltBrowser ? g_lpszAltBrowser : "");
		return TRUE;

	case WM_HSCROLL:
		SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			g_plugin.bShowNotesAtStart = IsDlgButtonChecked(hdlg, IDC_CHECK_HIDENOTES) == 0; // reversed
			g_plugin.bShowNoteButtons = IsDlgButtonChecked(hdlg, IDC_CHECK_BUTTONS) != 0;
			g_plugin.bShowScrollbar = IsDlgButtonChecked(hdlg, IDC_CHECK_SCROLLBARS) != 0;
			g_plugin.bAddContListMI = IsDlgButtonChecked(hdlg, IDC_CHECK_MENUS) != 0;

			BOOL LB;
			g_NoteWidth = GetDlgItemInt(hdlg, IDC_EDIT_WIDTH, &LB, FALSE);
			g_NoteHeight = GetDlgItemInt(hdlg, IDC_EDIT_HEIGHT, &LB, FALSE);
			g_Transparency = 255 - SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_GETPOS, 0, 0);
			
			g_plugin.bCloseAfterAddReminder = IsDlgButtonChecked(hdlg, IDC_CHECK_CLOSE) != 0;
			g_plugin.bUseMSI = IsDlgButtonChecked(hdlg, IDC_CHECK_MSI) != 0;
			
			g_NoteTitleDate = (SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_GETCURSEL, 0, 0) + 1) % SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_GETCOUNT, 0, 0);
			g_NoteTitleTime = (SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_GETCURSEL, 0, 0) + 1) % SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_GETCOUNT, 0, 0);
			if (g_NoteWidth < 179) {
				g_NoteWidth = 179;
				SetDlgItemInt(hdlg, IDC_EDIT_WIDTH, g_NoteWidth, FALSE);
			}
			if (g_NoteHeight < 35) {
				g_NoteHeight = 35;
				SetDlgItemInt(hdlg, IDC_EDIT_HEIGHT, g_NoteHeight, FALSE);
			}
			WORD SzT = (WORD)SendDlgItemMessage(hdlg, IDC_EDIT_EMAILSMS, WM_GETTEXTLENGTH, 0, 0);
			if (SzT != 0) {
				g_RemindSMS = (char*)realloc(g_RemindSMS, SzT + 1);
				GetDlgItemTextA(hdlg, IDC_EDIT_EMAILSMS, g_RemindSMS, SzT + 1);
			}
			char *P = g_RemindSMS;
			db_set_blob(0, MODULENAME, "RemindEmail", P, SzT);

			SzT = (WORD)SendDlgItemMessage(hdlg, IDC_EDIT_ALTBROWSER, WM_GETTEXTLENGTH, 0, 0);
			if (SzT != 0) {
				g_lpszAltBrowser = (char*)mir_realloc(g_lpszAltBrowser, SzT + 1);
				GetDlgItemTextA(hdlg, IDC_EDIT_ALTBROWSER, g_lpszAltBrowser, SzT + 1);
				TrimString(g_lpszAltBrowser);
				if (!*g_lpszAltBrowser) {
					mir_free(g_lpszAltBrowser);
					g_lpszAltBrowser = nullptr;
				}
			}
			else if (g_lpszAltBrowser) {
				mir_free(g_lpszAltBrowser);
				g_lpszAltBrowser = nullptr;
			}
			SetDlgItemTextA(hdlg, IDC_EDIT_ALTBROWSER, g_lpszAltBrowser ? g_lpszAltBrowser : "");
			if (g_lpszAltBrowser)
				g_plugin.setString("AltBrowser", g_lpszAltBrowser);
			else
				g_plugin.delSetting("AltBrowser");

			g_plugin.setDword("NoteWidth", g_NoteWidth);
			g_plugin.setDword("NoteHeight", g_NoteHeight);
			g_plugin.setDword("Transparency", g_Transparency);
			g_plugin.setDword("NoteTitleDate", g_NoteTitleDate);
			g_plugin.setDword("NoteTitleTime", g_NoteTitleTime);

			SaveNotes();
			LoadNotes(FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BTN_BROWSEBROWSER:
			{
				wchar_t s[MAX_PATH];
				GetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, s, _countof(s));

				OPENFILENAME ofn = {0};
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hdlg;
				ofn.lpstrFilter = TranslateT("Executable Files\0*.exe\0All Files\0*.*\0\0");
				ofn.lpstrFile = s;
				ofn.nMaxFile = _countof(s);
				ofn.lpstrTitle = TranslateT("Select Executable");
				ofn.lpstrInitialDir = L".";
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_DONTADDTORECENT;

				if (GetOpenFileName(&ofn)) {
					SetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, s);
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				}
			}
			break;

		case IDC_BUTTON_RESET:
			SAFE_FREE((void**)&g_RemindSMS);
			SetDlgItemTextA(hdlg, IDC_EDIT_EMAILSMS, "");
			SetDlgItemTextA(hdlg, IDC_EDIT_ALTBROWSER, "");

			replaceStr(g_lpszAltBrowser, nullptr);

			g_plugin.bShowNotesAtStart = g_plugin.bAddContListMI = g_plugin.bShowScrollbar = g_plugin.bShowNoteButtons = true;
			g_plugin.bCloseAfterAddReminder = g_plugin.bUseMSI = true;

			g_NoteTitleDate = 1;
			g_NoteTitleTime = 1;
			g_NoteWidth = 179;
			g_NoteHeight = 35;
			g_Transparency = 255;
			FillValues(hdlg);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0); // JK optim
			return TRUE;

		case IDC_EDIT_ALTBROWSER:
		case IDC_EDIT_EMAILSMS:
		case IDC_EDIT_WIDTH:
		case IDC_EDIT_HEIGHT:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_COMBODATE:
		case IDC_COMBOTIME:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_CHECK_SCROLLBARS:
		case IDC_CHECK_BUTTONS:
		case IDC_CHECK_HIDENOTES:
		case IDC_CHECK_MENUS:
		case IDC_CHECK_CLOSE:
		case IDC_CHECK_MSI:
			if (HIWORD(wParam) == BN_CLICKED)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;
		}
	}
	return FALSE;
}

int OnOptInitialise(WPARAM w, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 900002000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_STNOTEOPTIONS);
	odp.szTitle.a = SECTIONNAME;
	odp.szGroup.a = LPGEN("Plugins");
	odp.pfnDlgProc = DlgProcOptions;
	g_plugin.addOptions(w, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitSettings(void)
{
	void *P = nullptr;
	short Sz1 = MAX_PATH;

	ReadSettingBlob(0, MODULENAME, "RemindEmail", (WORD*)&Sz1, &P);
	if (!(Sz1 && P))
		g_RemindSMS = nullptr;
	else {
		g_RemindSMS = (char*)malloc(Sz1 + 1);
		memcpy(g_RemindSMS, P, Sz1);
		g_RemindSMS[Sz1] = 0;
		FreeSettingBlob(Sz1, P);
	}

	g_lpszAltBrowser = g_plugin.getStringA("AltBrowser");

	g_NoteWidth = g_plugin.getDword("NoteWidth", 179);
	g_NoteHeight = g_plugin.getDword("NoteHeight", 50);
	g_Transparency = g_plugin.getDword("Transparency", 255);
	g_NoteTitleDate = g_plugin.getDword("NoteTitleDate", 1);
	g_NoteTitleTime = g_plugin.getDword("NoteTitleTime", 1);

	ReadSettingIntArray(0, MODULENAME, "ReminderListGeom", g_reminderListGeom, _countof(g_reminderListGeom));
	ReadSettingIntArray(0, MODULENAME, "ReminderListColGeom", g_reminderListColGeom, _countof(g_reminderListColGeom));
	ReadSettingIntArray(0, MODULENAME, "NotesListGeom", g_notesListGeom, _countof(g_notesListGeom));
	ReadSettingIntArray(0, MODULENAME, "NotesListColGeom", g_notesListColGeom, _countof(g_notesListColGeom));

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
	if (g_reminderListGeom[2] > 0 && g_reminderListGeom[3] > 0) {
		WriteSettingIntArray(0, MODULENAME, "ReminderListGeom", g_reminderListGeom, _countof(g_reminderListGeom));
		WriteSettingIntArray(0, MODULENAME, "ReminderListColGeom", g_reminderListColGeom, _countof(g_reminderListColGeom));
	}
	if (g_notesListGeom[2] > 0 && g_notesListGeom[3] > 0) {
		WriteSettingIntArray(0, MODULENAME, "NotesListGeom", g_notesListGeom, _countof(g_notesListGeom));
		WriteSettingIntArray(0, MODULENAME, "NotesListColGeom", g_notesListColGeom, _countof(g_notesListColGeom));
	}

	if (g_lpszAltBrowser) {
		mir_free(g_lpszAltBrowser);
		g_lpszAltBrowser = nullptr;
	}
}
