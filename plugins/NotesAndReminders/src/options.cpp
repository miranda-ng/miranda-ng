#include "globals.h"

#ifndef OFN_DONTADDTORECENT
#define OFN_DONTADDTORECENT          0x02000000
#endif


// min allowed alpha (don't want 0 because it's a waste of resources as well as might confuse user)
#define MIN_ALPHA 30


#define IDC_RESET 1007
#define IDC_SHOWNOTES 1010
#define IDC_ADDCONTACTMENU 1011
#define IDC_NOTEWIDTH 1012
#define IDC_NOTEHEIGHT 1013
#define IDC_TRANSTRACK 1014
#define IDC_REMINDEMAIL 1017
#define IDC_SHOWSCROLLBAR 1018
#define IDC_SHOWBUTTONS 1019
#define IDC_ADDREMINDERCLOSES 1020
#define IDC_USEMCI 1023
#define PSM_CHANGED (WM_USER + 104)


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
BOOL g_isWin2kPlus = TRUE;

TCHAR *g_lpszAltBrowser = NULL;

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
	TCHAR *szDescr;
	COLORREF defColour;
	TCHAR *szDefFace;
	BYTE defStyle;
	char defSize;
	TCHAR *szBkgName;
}
static fontOptionsList[] =
{
	{LPGENT("Sticky Note Caption"), RGB(0,0,0), _T("Small Fonts"), 0, 7, LPGENT("Sticky Note Background Color")},
	//{LPGENT("Sticky Note Caption"), RGB(0,0,0), _T("Terminal"), 0, 6, LPGENT("Sticky Note Background Color")},
	//{LPGENT("Sticky Note Caption"), RGB(0,0,0), _T("MS Serif"), 0, 7, LPGENT("Sticky Note Background Color")},
	//{LPGENT("Sticky Note Body"), RGB(0,0,0), _T("Tahoma"), 0, 8, LPGENT("Sticky Note Background Color")},
	{LPGENT("Sticky Note Body"), RGB(0,0,0), _T("System"), DBFONTF_BOLD, 10, LPGENT("Sticky Note Background Color")},
};


struct ColourOptionsList
{
	TCHAR *szName;
	char *szSettingName;
	COLORREF defColour;
}
static colourOptionsList[] =
{
	{LPGENT("Sticky Note Background Color"), "BodyColor", NRCDEFAULT_BODYCLR}
};


LPCSTR GetDateFormatStr()
{
	return dateFormats[g_NoteTitleDate ? g_NoteTitleDate-1 : 0].lpszFmt;
}

LPCSTR GetTimeFormatStr()
{
	return timeFormats[g_NoteTitleTime ? g_NoteTitleTime-1 : 0].lpszFmt;
}

#if defined( _UNICODE )
static BYTE MsgDlgGetFontDefaultCharset(const TCHAR* szFont)
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

static int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam)
{
	*(int*)lParam = 1;
	return 0;
}

// get font charset according to current CP, if available for specified font
static BYTE MsgDlgGetFontDefaultCharset(const TCHAR* szFont)
{
	HDC hdc;
	LOGFONT lf = {0};
	int found = 0;

	_tcscpy(lf.lfFaceName, szFont);
	lf.lfCharSet = MsgDlgGetCPDefaultCharset();

	// check if the font supports specified charset
	hdc = GetDC(0);
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
	ZeroMemory(&lfBody,sizeof(LOGFONT));
	ZeroMemory(&lfCaption,sizeof(LOGFONT));

	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, (COLORREF*)&CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, (COLORREF*)&BodyFontColor);

	if (hBodyFont)
		DeleteObject(hBodyFont); 
	if (hCaptionFont)
		DeleteObject(hCaptionFont); 

	hBodyFont = CreateFontIndirect(&lfBody);
	hCaptionFont = CreateFontIndirect(&lfCaption);
}


static int FS_FontsChanged(WPARAM wParam, LPARAM lParam)
{
	InitFonts();

	SaveNotes();
	LoadNotes(FALSE);

	return 0;
}

static int FS_ColorChanged(WPARAM wParam, LPARAM lParam)
{
	LoadNRFont(NR_FONTID_CAPTION, &lfCaption, (COLORREF*)&CaptionFontColor);
	LoadNRFont(NR_FONTID_BODY, &lfBody, (COLORREF*)&BodyFontColor);

	BodyColor = DBGetContactSettingDword(NULL, MODULENAME, colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

	SaveNotes();
	LoadNotes(FALSE);

	return 0;
}

void RegisterFontServiceFonts()
{
	HDC hDC;
	int nFontScale;
	FontIDT fontid = {0};
	ColourIDT colorid = {0};
	char szTemp[100];
	int i;

	fontid.cbSize = sizeof(FontIDT);

    mir_sntprintf(fontid.group, SIZEOF(fontid.group), _T("%s"), LPGENT(SECTIONNAME));
    mir_sntprintf(fontid.backgroundGroup, SIZEOF(fontid.backgroundGroup), _T("%s"), LPGENT(SECTIONNAME));
	strncpy(fontid.dbSettingsGroup, MODULENAME, SIZEOF(fontid.dbSettingsGroup));
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_SAVEPOINTSIZE;

	hDC = GetDC(NULL);
	nFontScale = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(NULL, hDC);

	for (i = 0; i < SIZEOF(fontOptionsList); i++)
	{
		fontid.order = i;
		mir_snprintf(szTemp, SIZEOF(szTemp), "Font%d", i);
		strncpy(fontid.prefix, szTemp, SIZEOF(fontid.prefix));
		_tcsncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;

		fontid.deffontsettings.size = (char)-MulDiv(fontOptionsList[i].defSize, nFontScale, 72);
		//fontid.deffontsettings.size = fontOptionsList[i].defSize;

		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		fontid.deffontsettings.charset = MsgDlgGetFontDefaultCharset(fontOptionsList[i].szDefFace);
		_tcsncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));
		_tcsncpy(fontid.backgroundName, fontOptionsList[i].szBkgName, SIZEOF(fontid.backgroundName));

		FontRegisterT(&fontid);
	}

	colorid.cbSize = sizeof(ColourIDT);

	mir_sntprintf(colorid.group, SIZEOF(colorid.group), _T("%s"), LPGENT(SECTIONNAME));
	strncpy(colorid.dbSettingsGroup, MODULENAME, SIZEOF(fontid.dbSettingsGroup));
	colorid.flags = 0;

	for (i = 0; i < SIZEOF(colourOptionsList); i++)
	{
		colorid.order = i;
		_tcsncpy(colorid.name, colourOptionsList[i].szName, SIZEOF(colorid.name));
		colorid.defcolour = colourOptionsList[i].defColour;
		strncpy(colorid.setting, colourOptionsList[i].szSettingName, SIZEOF(colorid.setting));

		ColourRegisterT(&colorid);
	}

	hkFontChange = HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
	hkColorChange = HookEvent(ME_COLOUR_RELOAD, FS_ColorChanged);
}

void LoadNRFont(int i, LOGFONT *lf, COLORREF *colour)
{
    FontIDT fontid = {0};

    fontid.cbSize = sizeof(fontid);
    _tcsncpy(fontid.group, LPGENT(SECTIONNAME), SIZEOF(fontid.group));
    _tcsncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));

    COLORREF col = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)lf);

	if (colour)
	{
		*colour = col;
	}
}


static void TrimString(TCHAR *s)
{
	TCHAR *start;
	TCHAR *end;
	UINT n;

	if (!s || !*s)
	{
		return;
	}

	start = s;
	n = _tcslen(s) - 1;

	end = s + n;

	if (!_istspace(*start) && !_istspace(*end))
	{
		// nothing to trim
		return;
	}

	// scan past leading spaces
	while (*start && _istspace(*start)) start++;

	if (!*start)
	{
		// empty string
		*s = 0;
		return;
	}

	// trim trailing spaces
	while ( _istspace(*end) ) end--;
	end[1] = 0;

	if (start > s)
	{
		// remove leading spaces
		memmove(s, start, ((UINT)(end-start)+2)*sizeof(TCHAR));
	}
}


INT_PTR CALLBACK DlgProcOptions(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	BOOL LB;
	WORD SzT;
	void *P;
	int i;

	switch (message)
	{
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault(hdlg);
			SendDlgItemMessage(hdlg,IDC_TRANSTRACK,TBM_SETRANGE,TRUE,MAKELONG(0,255-MIN_ALPHA));
			SendDlgItemMessage(hdlg,IDC_TRANSTRACK,TBM_SETPOS,TRUE,255-g_Transparency);
			SendDlgItemMessage(hdlg,IDC_SHOWNOTES,BM_SETCHECK,(WPARAM)!g_ShowNotesAtStart,0);
			SendDlgItemMessage(hdlg,IDC_SHOWBUTTONS,BM_SETCHECK,(WPARAM)g_ShowNoteButtons,0);
			SendDlgItemMessage(hdlg,IDC_SHOWSCROLLBAR,BM_SETCHECK,(WPARAM)g_ShowScrollbar,0); // 4.2
			SendDlgItemMessage(hdlg,IDC_ADDCONTACTMENU,BM_SETCHECK,(WPARAM)g_AddContListMI,0);
			SetDlgItemInt(hdlg,IDC_NOTEWIDTH,g_NoteWidth,FALSE);
			SetDlgItemInt(hdlg,IDC_NOTEHEIGHT,g_NoteHeight,FALSE);
			SendDlgItemMessage(hdlg,IDC_ADDREMINDERCLOSES,BM_SETCHECK,(WPARAM)g_CloseAfterAddReminder,0);
			SendDlgItemMessage(hdlg,IDC_USEMCI,BM_SETCHECK,(WPARAM)!g_UseDefaultPlaySound,0);

			SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_RESETCONTENT,0,0);
			SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_RESETCONTENT,0,0);
			for (i=0; i<SIZEOF(dateFormats); i++)
				SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_ADDSTRING,0,(LPARAM)dateFormats[i].lpszUI);
			for (i=0; i<SIZEOF(timeFormats); i++)
				SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_ADDSTRING,0,(LPARAM)timeFormats[i].lpszUI);
			SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_ADDSTRING,0,(LPARAM)Translate("None"));
			SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_ADDSTRING,0,(LPARAM)Translate("None"));

			SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_SETCURSEL,(WPARAM)(g_NoteTitleDate ? g_NoteTitleDate-1 : SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_GETCOUNT,0,0)-1),0);
			SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_SETCURSEL,(WPARAM)(g_NoteTitleTime ? g_NoteTitleTime-1 : SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_GETCOUNT,0,0)-1),0);

			if (g_RemindSMS) 
				SetDlgItemText(hdlg,IDC_REMINDEMAIL,g_RemindSMS);
			else 
				SetDlgItemText(hdlg,IDC_REMINDEMAIL,"");

			SetDlgItemText(hdlg,IDC_EDIT_ALTBROWSER,g_lpszAltBrowser ? g_lpszAltBrowser : _T(""));
      if (!MySetLayeredWindowAttributes)
      { // layered UI not available
        EnableWindow(GetDlgItem(hdlg,IDC_TRANSTRACK), FALSE);
      }
			return TRUE;
		}
	case WM_HSCROLL:
		{
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			g_ShowNotesAtStart = !(BOOL)SendDlgItemMessage(hdlg,IDC_SHOWNOTES,BM_GETCHECK,0,0);
			g_ShowNoteButtons = (BOOL)SendDlgItemMessage(hdlg,IDC_SHOWBUTTONS,BM_GETCHECK,0,0);
			g_ShowScrollbar = (BOOL)SendDlgItemMessage(hdlg,IDC_SHOWSCROLLBAR,BM_GETCHECK,0,0); // 4.2
			g_AddContListMI = (BOOL)SendDlgItemMessage(hdlg,IDC_ADDCONTACTMENU,BM_GETCHECK,0,0);
			g_NoteWidth = GetDlgItemInt(hdlg,IDC_NOTEWIDTH,&LB,FALSE);
			g_NoteHeight = GetDlgItemInt(hdlg,IDC_NOTEHEIGHT,&LB,FALSE);
			g_Transparency = 255-SendDlgItemMessage(hdlg,IDC_TRANSTRACK,TBM_GETPOS,0,0);
			g_CloseAfterAddReminder = (BOOL)SendDlgItemMessage(hdlg,IDC_ADDREMINDERCLOSES,BM_GETCHECK,0,0);
			g_UseDefaultPlaySound = !(BOOL)SendDlgItemMessage(hdlg,IDC_USEMCI,BM_GETCHECK,0,0);
			g_NoteTitleDate = (SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_GETCURSEL,0,0) + 1) % SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_GETCOUNT,0,0);
			g_NoteTitleTime = (SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_GETCURSEL,0,0) + 1) % SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_GETCOUNT,0,0);
			if (g_NoteWidth < 179)
			{
				g_NoteWidth = 179;
				SetDlgItemInt(hdlg,IDC_NOTEWIDTH,g_NoteWidth,FALSE);
			}
			if (g_NoteHeight < 35) 
			{
				g_NoteHeight = 35;
				SetDlgItemInt(hdlg,IDC_NOTEHEIGHT,g_NoteHeight,FALSE);
			}
			SzT = (WORD)SendDlgItemMessage(hdlg,IDC_REMINDEMAIL,WM_GETTEXTLENGTH,0,0);
			if (SzT != 0) 
			{
				g_RemindSMS = (char*)realloc(g_RemindSMS,SzT+1);
				SendDlgItemMessage(hdlg,IDC_REMINDEMAIL,WM_GETTEXT,SzT+1,(LPARAM)g_RemindSMS);
			}
			P = g_RemindSMS;
			WriteSettingBlob(0,MODULENAME,"RemindEmail",SzT,P);

			SzT = (WORD)SendDlgItemMessage(hdlg,IDC_EDIT_ALTBROWSER,WM_GETTEXTLENGTH,0,0);
			if (SzT != 0) 
			{
				g_lpszAltBrowser = (TCHAR*)mir_realloc(g_lpszAltBrowser,SzT+1);
				SendDlgItemMessage(hdlg,IDC_EDIT_ALTBROWSER,WM_GETTEXT,SzT+1,(LPARAM)g_lpszAltBrowser);
				TrimString(g_lpszAltBrowser);
				if (!*g_lpszAltBrowser)
				{
					mir_free(g_lpszAltBrowser);
					g_lpszAltBrowser = NULL;
				}
			}
			else if (g_lpszAltBrowser)
			{
				mir_free(g_lpszAltBrowser);
				g_lpszAltBrowser = NULL;
			}
			SetDlgItemText(hdlg,IDC_EDIT_ALTBROWSER,g_lpszAltBrowser ? g_lpszAltBrowser : _T(""));
			if (g_lpszAltBrowser)
				DBWriteContactSettingString(0,MODULENAME,"AltBrowser",g_lpszAltBrowser);
			else
				DBDeleteContactSetting(0,MODULENAME,"AltBrowser");

			WriteSettingInt(0,MODULENAME,"ShowNotesAtStart",g_ShowNotesAtStart);
			WriteSettingInt(0,MODULENAME,"ShowNoteButtons",g_ShowNoteButtons);
			WriteSettingInt(0,MODULENAME,"ShowScrollbar",g_ShowScrollbar); // 4.2
			WriteSettingInt(0,MODULENAME,"AddContactMenuItems",g_AddContListMI);
			WriteSettingInt(0,MODULENAME,"NoteWidth",g_NoteWidth);
			WriteSettingInt(0,MODULENAME,"NoteHeight",g_NoteHeight);
			WriteSettingInt(0,MODULENAME,"Transparency",g_Transparency);
			WriteSettingInt(0,MODULENAME,"NoteTitleDate",g_NoteTitleDate);
			WriteSettingInt(0,MODULENAME,"NoteTitleTime",g_NoteTitleTime);
			WriteSettingInt(0,MODULENAME,"CloseAfterAddReminder",g_CloseAfterAddReminder);
			WriteSettingInt(0,MODULENAME,"UseMCI",!g_UseDefaultPlaySound);
			SaveNotes();
			LoadNotes(FALSE);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_BROWSEBROWSER:
			{
				TCHAR s[MAX_PATH];

				OPENFILENAME ofn = {0};
#if defined(WINVER) && _WIN32_WINNT >= 0x0500
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
				ofn.lStructSize = sizeof(ofn);
#endif
				ofn.hwndOwner = hdlg;
				ofn.lpstrFilter = _T("Executable Files\0*.exe\0All Files\0*.*\0\0");
				ofn.lpstrFile = s;
				ofn.nMaxFile = SIZEOF(s);
				ofn.lpstrTitle = TranslateT("Select Executable");
				ofn.lpstrInitialDir = _T(".");
				ofn.Flags = OFN_FILEMUSTEXIST|OFN_LONGNAMES;
				if ( IsWinVer98Plus() )
				{
					ofn.Flags |= OFN_ENABLESIZING;
					if (g_isWin2kPlus)
						ofn.Flags |= OFN_DONTADDTORECENT;
				}

				SendDlgItemMessage(hdlg,IDC_EDIT_ALTBROWSER,WM_GETTEXT,(WPARAM)ofn.nMaxFile,(LPARAM)s);

				if ( GetOpenFileName(&ofn) )
				{
					SetDlgItemText(hdlg,IDC_EDIT_ALTBROWSER,s);
				}
			}
			break;
		case IDC_RESET:
			{
				SAFE_FREE((void**)&g_RemindSMS);
				SetDlgItemText(hdlg,IDC_REMINDEMAIL,_T(""));
				if (g_lpszAltBrowser)
				{
					mir_free(g_lpszAltBrowser);
					g_lpszAltBrowser = NULL;
				}
				SetDlgItemText(hdlg,IDC_EDIT_ALTBROWSER,_T(""));
				g_ShowNotesAtStart = TRUE;
				g_AddContListMI = TRUE;
				g_ShowScrollbar = TRUE; // 4.2
				g_ShowNoteButtons = TRUE;
				g_NoteTitleDate = 1;
				g_NoteTitleTime = 1;
				g_CloseAfterAddReminder = TRUE;
				g_UseDefaultPlaySound = FALSE;
				SendDlgItemMessage(hdlg,IDC_SHOWNOTES,BM_SETCHECK,!g_ShowNotesAtStart,0);
				SendDlgItemMessage(hdlg,IDC_ADDCONTACTMENU,BM_SETCHECK,g_AddContListMI,0);
				SendDlgItemMessage(hdlg,IDC_SHOWSCROLLBAR,BM_SETCHECK,g_ShowScrollbar,0);
				SendDlgItemMessage(hdlg,IDC_SHOWBUTTONS,BM_SETCHECK,(WPARAM)g_ShowNoteButtons,0);
				SendDlgItemMessage(hdlg,IDC_ADDREMINDERCLOSES,BM_SETCHECK,g_CloseAfterAddReminder,0);
				SendDlgItemMessage(hdlg,IDC_USEMCI,BM_SETCHECK,!g_UseDefaultPlaySound,0);
				SendDlgItemMessage(hdlg,IDC_COMBODATE,CB_SETCURSEL,(WPARAM)(g_NoteTitleDate-1),0);
				SendDlgItemMessage(hdlg,IDC_COMBOTIME,CB_SETCURSEL,(WPARAM)(g_NoteTitleTime-1),0);
				g_NoteWidth = 179; // 4.2
				g_NoteHeight = 35;
				SetDlgItemInt(hdlg,IDC_NOTEWIDTH,g_NoteWidth,FALSE);
				SetDlgItemInt(hdlg,IDC_NOTEHEIGHT,g_NoteHeight,FALSE);
				g_Transparency = 255;
				SendDlgItemMessage(hdlg,IDC_TRANSTRACK,TBM_SETPOS,TRUE,0);
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0); // JK optim
				return TRUE;					
			}
		default:
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
	}
	return FALSE;
}

void InitSettings(void)
{
	void *P = NULL;
	short Sz1;

	Sz1 = MAX_PATH; P = NULL;
	ReadSettingBlob(0, MODULENAME, "RemindEmail", (WORD*)&Sz1, &P);
	if (!(Sz1 && P)) 
		g_RemindSMS = NULL;
	else 
	{
		g_RemindSMS = (char*)malloc(Sz1+1);
		ZeroMemory(g_RemindSMS,Sz1+1);
		memcpy(g_RemindSMS,P,Sz1);
		FreeSettingBlob(Sz1,P);
	}

	g_lpszAltBrowser = DBGetString(0,MODULENAME,"AltBrowser");

	g_ShowNotesAtStart = (BOOL)ReadSettingInt(0,MODULENAME,"ShowNotesAtStart",1);
	g_ShowNoteButtons = (BOOL)ReadSettingInt(0,MODULENAME,"ShowNoteButtons",1);
	g_ShowScrollbar = (BOOL)ReadSettingInt(0,MODULENAME,"ShowScrollbar",1);
	g_AddContListMI = (BOOL)ReadSettingInt(0,MODULENAME,"AddContactMenuItems",1);
	g_NoteWidth = ReadSettingInt(0,MODULENAME,"NoteWidth",179);
	g_NoteHeight = ReadSettingInt(0,MODULENAME,"NoteHeight",50);
	g_Transparency = ReadSettingInt(0,MODULENAME,"Transparency",255);
	g_NoteTitleDate = ReadSettingInt(0,MODULENAME,"NoteTitleDate",1);
	g_NoteTitleTime = ReadSettingInt(0,MODULENAME,"NoteTitleTime",1);
	g_CloseAfterAddReminder = (BOOL)ReadSettingInt(0,MODULENAME,"CloseAfterAddReminder",1);
	g_UseDefaultPlaySound = !(BOOL)ReadSettingInt(0,MODULENAME,"UseMCI",1);

	ReadSettingIntArray(0,MODULENAME,"ReminderListGeom",g_reminderListGeom,SIZEOF(g_reminderListGeom));
	ReadSettingIntArray(0,MODULENAME,"ReminderListColGeom",g_reminderListColGeom,SIZEOF(g_reminderListColGeom));
	ReadSettingIntArray(0,MODULENAME,"NotesListGeom",g_notesListGeom,SIZEOF(g_notesListGeom));
	ReadSettingIntArray(0,MODULENAME,"NotesListColGeom",g_notesListColGeom,SIZEOF(g_notesListColGeom));

	BodyColor = DBGetContactSettingDword(NULL, MODULENAME, colourOptionsList[0].szSettingName, colourOptionsList[0].defColour);

	InitFonts();

	g_hReminderIcon = Skin_GetIconByHandle(hIconLibItem[10]);

	if (g_Transparency < MIN_ALPHA)
		g_Transparency = MIN_ALPHA;
	else if (g_Transparency > 255)
		g_Transparency = 255;
}

void TermSettings(void)
{
	if (g_reminderListGeom[2] > 0 && g_reminderListGeom[3] > 0)
	{
		WriteSettingIntArray(0,MODULENAME,"ReminderListGeom",g_reminderListGeom,SIZEOF(g_reminderListGeom));
		WriteSettingIntArray(0,MODULENAME,"ReminderListColGeom",g_reminderListColGeom,SIZEOF(g_reminderListColGeom));
	}
	if (g_notesListGeom[2] > 0 && g_notesListGeom[3] > 0)
	{
		WriteSettingIntArray(0,MODULENAME,"NotesListGeom",g_notesListGeom,SIZEOF(g_notesListGeom));
		WriteSettingIntArray(0,MODULENAME,"NotesListColGeom",g_notesListColGeom,SIZEOF(g_notesListColGeom));
	}

	if (g_lpszAltBrowser)
	{
		mir_free(g_lpszAltBrowser);
		g_lpszAltBrowser = NULL;
	}
}
