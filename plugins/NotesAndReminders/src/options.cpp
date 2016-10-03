#include "globals.h"

// min allowed alpha (don't want 0 because it's a waste of resources as well as might confuse user)
#define MIN_ALPHA 30

extern HANDLE hkFontChange;
extern HANDLE hkColorChange;

BOOL g_CloseAfterAddReminder, g_UseDefaultPlaySound;
HICON g_hReminderIcon = NULL;

LOGFONT lfBody,lfCaption;
HFONT hBodyFont = NULL, hCaptionFont = NULL;
long BodyColor;
long CaptionFontColor,BodyFontColor;
BOOL g_ShowNotesAtStart,g_ShowScrollbar,g_AddContListMI,g_ShowNoteButtons;
int g_NoteTitleDate, g_NoteTitleTime;
int g_NoteWidth,g_NoteHeight;
int g_Transparency;
char *g_RemindSMS = NULL;

char *g_lpszAltBrowser = NULL;

int g_reminderListGeom[4] = {0};
int g_reminderListColGeom[2] = { 150, 205 };
int g_notesListGeom[4] = {0};
int g_notesListColGeom[4] = { 150, 20, 20, 165 };


#define NRCDEFAULT_BODYCLR		RGB(255,255,0)

struct DateFormat
{
	LPCSTR lpszUI;
	LPCSTR lpszFmt;
}
static dateFormats[] =
{
	{ "1981-12-31", "yyyy'-'MM'-'dd" },
	{ "31-12-1981", "dd'-'MM'-'yyyy" },
	{ "12-31-1981", "MM'-'dd'-'yyyy" },
	{ "1981-dec-31", "yyyy'-'MMM'-'dd" },
	{ "31-dec-1981", "dd'-'MMM'-'yyyy" },
	{ "dec-31-1981", "MMM'-'dd'-'yyyy" },
	{ "1981/12/31", "yyyy'/'MM'/'dd" },
	{ "31/12/1981", "dd'/'MM'/'yyyy" },
	{ "12/31/1981", "MM'/'dd'/'yyyy" },
	{ "1981/dec/31", "yyyy'/'MMM'/'dd" },
	{ "31/dec/1981", "dd'/'MMM'/'yyyy" },
	{ "dec/31/1981", "MMM'/'dd'/'yyyy" },
	{ "1981 dec 31", "yyyy MMM dd" },
	{ "31 dec 1981", "dd MMM yyyy" },
	{ "dec 31 1981", "MMM dd yyyy" }
};

struct TimeFormat
{
	LPCSTR lpszUI;
	LPCSTR lpszFmt;
}
static timeFormats[] =
{
	{ "19:30:00", "HH':'mm':'ss" },
	{ "19:30", "HH':'mm'" },
	{ "7:30:00 PM", "hh':'mm':'ss tt" },
	{ "7:30 PM", "hh':'mm tt" },
	{ "7:30:00P", "hh':'mm':'sst" },
	{ "7:30P", "hh':'mmt" }
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
	// { LPGEN("Sticky Note Caption"), RGB(0,0,0), L"Terminal", 0, 6, LPGEN("Sticky Note Background Color")},
	// { LPGEN("Sticky Note Caption"), RGB(0,0,0), L"MS Serif", 0, 7, LPGEN("Sticky Note Background Color")},
	// { LPGEN("Sticky Note Body"), RGB(0,0,0), L"Tahoma", 0, 8, LPGEN("Sticky Note Background Color")},
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


LPCSTR GetDateFormatStr()
{
	return dateFormats[g_NoteTitleDate ? g_NoteTitleDate - 1 : 0].lpszFmt;
}

LPCSTR GetTimeFormatStr()
{
	return timeFormats[g_NoteTitleTime ? g_NoteTitleTime - 1 : 0].lpszFmt;
}

#if defined( _UNICODE )
static BYTE MsgDlgGetFontDefaultCharset(const wchar_t* szFont)
{
	return DEFAULT_CHARSET;
}
#else
// get font charset according to current CP
static BYTE MsgDlgGetCPDefaultCharset()
{
	switch (GetACP()) {
	case 1250:
		return EASTEUROPE_CHARSET;
	case 1251:
		return RUSSIAN_CHARSET;
	case 1252:
		return ANSI_CHARSET;
	case 1253:
		return GREEK_CHARSET;
	case 1254:
		return TURKISH_CHARSET;
	case 1255:
		return HEBREW_CHARSET;
	case 1256:
		return ARABIC_CHARSET;
	case 1257:
		return BALTIC_CHARSET;
	case 1361:
		return JOHAB_CHARSET;
	case 874:
		return THAI_CHARSET;
	case 932:
		return SHIFTJIS_CHARSET;
	case 936:
		return GB2312_CHARSET;
	case 949:
		return HANGEUL_CHARSET;
	case 950:
		return CHINESEBIG5_CHARSET;
	default:
		return DEFAULT_CHARSET;
	}
}

static int CALLBACK EnumFontFamExProc(const LOGFONT *, const TEXTMETRIC *, DWORD, LPARAM lParam)
{
	*(int*)lParam = 1;
	return 0;
}

// get font charset according to current CP, if available for specified font
static BYTE MsgDlgGetFontDefaultCharset(const char *szFont)
{
	LOGFONT lf = { 0 };
	int found = 0;

	mir_strcpy(lf.lfFaceName, szFont);
	lf.lfCharSet = MsgDlgGetCPDefaultCharset();

	// check if the font supports specified charset
	HDC hdc = GetDC(0);
	EnumFontFamiliesEx(hdc, &lf, &EnumFontFamExProc, (LPARAM)&found, 0);
	ReleaseDC(0, hdc);

	if (found)
		return lf.lfCharSet;
	else // no, give default
		return DEFAULT_CHARSET;
}
#endif


static void InitFonts()
{
	memset(&lfBody, 0, sizeof(LOGFONT));
	memset(&lfCaption, 0, sizeof(LOGFONT));

	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, (COLORREF*)&CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, (COLORREF*)&BodyFontColor);

	if (hBodyFont)
		DeleteObject(hBodyFont);
	if (hCaptionFont)
		DeleteObject(hCaptionFont);

	hBodyFont = CreateFontIndirect(&lfBody);
	hCaptionFont = CreateFontIndirect(&lfCaption);
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

	BodyColor = db_get_dw(NULL, MODULENAME, colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

	SaveNotes();
	LoadNotes(FALSE);

	return 0;
}

void RegisterFontServiceFonts()
{
	char szTemp[100];

	FontID fontid = { 0 };
	fontid.cbSize = sizeof(FontIDW);

	strncpy(fontid.group, SECTIONNAME, _countof(fontid.group));
	strncpy(fontid.backgroundGroup, SECTIONNAME, _countof(fontid.backgroundGroup));
	strncpy(fontid.dbSettingsGroup, MODULENAME, _countof(fontid.dbSettingsGroup));
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_SAVEPOINTSIZE;

	HDC hDC = GetDC(NULL);
	int nFontScale = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(NULL, hDC);

	for (int i = 0; i < _countof(fontOptionsList); i++) {
		fontid.order = i;
		mir_snprintf(szTemp, "Font%d", i);
		strncpy(fontid.prefix, szTemp, _countof(fontid.prefix));
		strncpy(fontid.name, fontOptionsList[i].szDescr, _countof(fontid.name));
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;

		fontid.deffontsettings.size = (char)-MulDiv(fontOptionsList[i].defSize, nFontScale, 72);
		//fontid.deffontsettings.size = fontOptionsList[i].defSize;

		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		fontid.deffontsettings.charset = MsgDlgGetFontDefaultCharset(fontOptionsList[i].szDefFace);
		strncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _countof(fontid.deffontsettings.szFace));
		strncpy(fontid.backgroundName, fontOptionsList[i].szBkgName, _countof(fontid.backgroundName));

		Font_Register(&fontid);
	}

	ColourID colorid = { 0 };
	colorid.cbSize = sizeof(ColourIDW);

	strncpy(colorid.group, SECTIONNAME, _countof(colorid.group));
	strncpy(colorid.dbSettingsGroup, MODULENAME, _countof(fontid.dbSettingsGroup));
	colorid.flags = 0;

	for (int i = 0; i < _countof(colourOptionsList); i++) {
		colorid.order = i;
		strncpy(colorid.name, colourOptionsList[i].szName, _countof(colorid.name));
		colorid.defcolour = colourOptionsList[i].defColour;
		strncpy(colorid.setting, colourOptionsList[i].szSettingName, _countof(colorid.setting));

		Colour_Register(&colorid);
	}

	hkFontChange = HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
	hkColorChange = HookEvent(ME_COLOUR_RELOAD, FS_ColorChanged);
}

void LoadNRFont(int i, LOGFONT *lf, COLORREF *colour)
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


INT_PTR CALLBACK DlgProcOptions(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_SETRANGE, TRUE, MAKELONG(0, 255 - MIN_ALPHA));
		SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_SETPOS, TRUE, 255 - g_Transparency);

		CheckDlgButton(hdlg, IDC_CHECK_HIDENOTES, !g_ShowNotesAtStart ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_MENUS, g_AddContListMI ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_BUTTONS, g_ShowNoteButtons ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_SCROLLBARS, g_ShowScrollbar ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_CLOSE, g_CloseAfterAddReminder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_MSI, !g_UseDefaultPlaySound ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hdlg, IDC_EDIT_WIDTH, g_NoteWidth, FALSE);
		SetDlgItemInt(hdlg, IDC_EDIT_HEIGHT, g_NoteHeight, FALSE);

		SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_RESETCONTENT, 0, 0);
		for (int i = 0; i < _countof(dateFormats); i++)
			SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_ADDSTRING, 0, (LPARAM)dateFormats[i].lpszUI);
		for (int i = 0; i < _countof(timeFormats); i++)
			SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_ADDSTRING, 0, (LPARAM)timeFormats[i].lpszUI);
		SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_ADDSTRING, 0, (LPARAM)Translate("None"));
		SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_ADDSTRING, 0, (LPARAM)Translate("None"));

		SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_SETCURSEL, (WPARAM)(g_NoteTitleDate ? g_NoteTitleDate - 1 : SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_GETCOUNT, 0, 0) - 1), 0);
		SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_SETCURSEL, (WPARAM)(g_NoteTitleTime ? g_NoteTitleTime - 1 : SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_GETCOUNT, 0, 0) - 1), 0);

		if (g_RemindSMS)
			SetDlgItemText(hdlg, IDC_EDIT_EMAILSMS, g_RemindSMS);
		else
			SetDlgItemText(hdlg, IDC_EDIT_EMAILSMS, "");

		SetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, g_lpszAltBrowser ? g_lpszAltBrowser : "");
		return TRUE;

	case WM_HSCROLL:
		SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			g_ShowNotesAtStart = !(BOOL)IsDlgButtonChecked(hdlg, IDC_CHECK_HIDENOTES);
			g_ShowNoteButtons = (BOOL)IsDlgButtonChecked(hdlg, IDC_CHECK_BUTTONS);
			g_ShowScrollbar = (BOOL)IsDlgButtonChecked(hdlg, IDC_CHECK_SCROLLBARS);
			g_AddContListMI = (BOOL)IsDlgButtonChecked(hdlg, IDC_CHECK_MENUS);
			BOOL LB;
			g_NoteWidth = GetDlgItemInt(hdlg, IDC_EDIT_WIDTH, &LB, FALSE);
			g_NoteHeight = GetDlgItemInt(hdlg, IDC_EDIT_HEIGHT, &LB, FALSE);
			g_Transparency = 255 - SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_GETPOS, 0, 0);
			g_CloseAfterAddReminder = (BOOL)IsDlgButtonChecked(hdlg, IDC_CHECK_CLOSE);
			g_UseDefaultPlaySound = !(BOOL)IsDlgButtonChecked(hdlg, IDC_CHECK_MSI);
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
				GetDlgItemText(hdlg, IDC_EDIT_EMAILSMS, g_RemindSMS, SzT + 1);
			}
			char *P = g_RemindSMS;
			db_set_blob(0, MODULENAME, "RemindEmail", P, SzT);

			SzT = (WORD)SendDlgItemMessage(hdlg, IDC_EDIT_ALTBROWSER, WM_GETTEXTLENGTH, 0, 0);
			if (SzT != 0) {
				g_lpszAltBrowser = (char*)mir_realloc(g_lpszAltBrowser, SzT + 1);
				GetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, g_lpszAltBrowser, SzT + 1);
				TrimString(g_lpszAltBrowser);
				if (!*g_lpszAltBrowser) {
					mir_free(g_lpszAltBrowser);
					g_lpszAltBrowser = NULL;
				}
			}
			else if (g_lpszAltBrowser) {
				mir_free(g_lpszAltBrowser);
				g_lpszAltBrowser = NULL;
			}
			SetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, g_lpszAltBrowser ? g_lpszAltBrowser : "");
			if (g_lpszAltBrowser)
				db_set_s(0, MODULENAME, "AltBrowser", g_lpszAltBrowser);
			else
				db_unset(0, MODULENAME, "AltBrowser");

			db_set_dw(0, MODULENAME, "ShowNotesAtStart", g_ShowNotesAtStart);
			db_set_dw(0, MODULENAME, "ShowNoteButtons", g_ShowNoteButtons);
			db_set_dw(0, MODULENAME, "ShowScrollbar", g_ShowScrollbar);
			db_set_dw(0, MODULENAME, "AddContactMenuItems", g_AddContListMI);
			db_set_dw(0, MODULENAME, "NoteWidth", g_NoteWidth);
			db_set_dw(0, MODULENAME, "NoteHeight", g_NoteHeight);
			db_set_dw(0, MODULENAME, "Transparency", g_Transparency);
			db_set_dw(0, MODULENAME, "NoteTitleDate", g_NoteTitleDate);
			db_set_dw(0, MODULENAME, "NoteTitleTime", g_NoteTitleTime);
			db_set_dw(0, MODULENAME, "CloseAfterAddReminder", g_CloseAfterAddReminder);
			db_set_dw(0, MODULENAME, "UseMCI", !g_UseDefaultPlaySound);
			SaveNotes();
			LoadNotes(FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BTN_BROWSEBROWSER:
			{
				char s[MAX_PATH];

				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hdlg;
				ofn.lpstrFilter = Translate("Executable Files\0*.exe\0All Files\0*.*\0\0");
				ofn.lpstrFile = s;
				ofn.nMaxFile = _countof(s);
				ofn.lpstrTitle = Translate("Select Executable");
				ofn.lpstrInitialDir = ".";
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_DONTADDTORECENT;

				GetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, s, ofn.nMaxFile);

				if (GetOpenFileName(&ofn)) {
					SetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, s);
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				}
			}
			break;

		case IDC_BUTTON_RESET:
			SAFE_FREE((void**)&g_RemindSMS);
			SetDlgItemText(hdlg, IDC_EDIT_EMAILSMS, "");
			if (g_lpszAltBrowser) {
				mir_free(g_lpszAltBrowser);
				g_lpszAltBrowser = NULL;
			}
			SetDlgItemText(hdlg, IDC_EDIT_ALTBROWSER, "");
			g_ShowNotesAtStart = TRUE;
			g_AddContListMI = TRUE;
			g_ShowScrollbar = TRUE;
			g_ShowNoteButtons = TRUE;
			g_NoteTitleDate = 1;
			g_NoteTitleTime = 1;
			g_CloseAfterAddReminder = TRUE;
			g_UseDefaultPlaySound = FALSE;
			CheckDlgButton(hdlg, IDC_CHECK_HIDENOTES, !g_ShowNotesAtStart ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_CHECK_MENUS, g_AddContListMI ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_CHECK_SCROLLBARS, g_ShowScrollbar ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_CHECK_BUTTONS, g_ShowNoteButtons ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_CHECK_CLOSE, g_CloseAfterAddReminder ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_CHECK_MSI, !g_UseDefaultPlaySound ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hdlg, IDC_COMBODATE, CB_SETCURSEL, (WPARAM)(g_NoteTitleDate - 1), 0);
			SendDlgItemMessage(hdlg, IDC_COMBOTIME, CB_SETCURSEL, (WPARAM)(g_NoteTitleTime - 1), 0);
			g_NoteWidth = 179;
			g_NoteHeight = 35;
			SetDlgItemInt(hdlg, IDC_EDIT_WIDTH, g_NoteWidth, FALSE);
			SetDlgItemInt(hdlg, IDC_EDIT_HEIGHT, g_NoteHeight, FALSE);
			g_Transparency = 255;
			SendDlgItemMessage(hdlg, IDC_SLIDER_TRANSPARENCY, TBM_SETPOS, TRUE, 0);
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

void InitSettings(void)
{
	void *P = NULL;
	short Sz1 = MAX_PATH;

	ReadSettingBlob(0, MODULENAME, "RemindEmail", (WORD*)&Sz1, &P);
	if (!(Sz1 && P))
		g_RemindSMS = NULL;
	else {
		g_RemindSMS = (char*)malloc(Sz1 + 1);
		memcpy(g_RemindSMS, P, Sz1);
		g_RemindSMS[Sz1] = 0;
		FreeSettingBlob(Sz1, P);
	}

	g_lpszAltBrowser = db_get_sa(0, MODULENAME, "AltBrowser");

	g_ShowNotesAtStart = (BOOL)db_get_dw(0, MODULENAME, "ShowNotesAtStart", 1);
	g_ShowNoteButtons = (BOOL)db_get_dw(0, MODULENAME, "ShowNoteButtons", 1);
	g_ShowScrollbar = (BOOL)db_get_dw(0, MODULENAME, "ShowScrollbar", 1);
	g_AddContListMI = (BOOL)db_get_dw(0, MODULENAME, "AddContactMenuItems", 1);
	g_NoteWidth = db_get_dw(0, MODULENAME, "NoteWidth", 179);
	g_NoteHeight = db_get_dw(0, MODULENAME, "NoteHeight", 50);
	g_Transparency = db_get_dw(0, MODULENAME, "Transparency", 255);
	g_NoteTitleDate = db_get_dw(0, MODULENAME, "NoteTitleDate", 1);
	g_NoteTitleTime = db_get_dw(0, MODULENAME, "NoteTitleTime", 1);
	g_CloseAfterAddReminder = (BOOL)db_get_dw(0, MODULENAME, "CloseAfterAddReminder", 1);
	g_UseDefaultPlaySound = !(BOOL)db_get_dw(0, MODULENAME, "UseMCI", 1);

	ReadSettingIntArray(0, MODULENAME, "ReminderListGeom", g_reminderListGeom, _countof(g_reminderListGeom));
	ReadSettingIntArray(0, MODULENAME, "ReminderListColGeom", g_reminderListColGeom, _countof(g_reminderListColGeom));
	ReadSettingIntArray(0, MODULENAME, "NotesListGeom", g_notesListGeom, _countof(g_notesListGeom));
	ReadSettingIntArray(0, MODULENAME, "NotesListColGeom", g_notesListColGeom, _countof(g_notesListColGeom));

	BodyColor = db_get_dw(NULL, MODULENAME, colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

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
		g_lpszAltBrowser = NULL;
	}
}
