
#include "stdhdr.h"

#include "resource.h"
#include "fltcont.h"

/////////////////////////////////////////////////////////////////////////////
//

#define SAMEASF_FACE   1
#define SAMEASF_SIZE   2
#define SAMEASF_STYLE  4
#define SAMEASF_COLOUR 8

typedef struct _SFontSettings
{
	BYTE sameAsFlags,sameAs;
	COLORREF colour;
	char size;
	BYTE style;
	BYTE charset;
	char szFace[LF_FACESIZE];
} SFontSettings;

static SFontSettings s_rgFontSettings[FLT_FONTIDS];
static SFontSettings s_rgFontSettingsMiranda[FLT_FONTIDS];

#define FLT_SAME_AS_NONE	((BYTE)0xFF)
#define FLT_SAME_AS_MIRANDA	((BYTE)0xFE)

static char* s_rgszFontSizes[]={"8","10","14","16","18","20","24","28"};

static const TCHAR* s_rgszFontIdDescr[FLT_FONTIDS] =
{
	_T("Standard contacts"),
	_T("Online contacts to whom you have a different visibility"),
	_T("Offline contacts"),
	_T("Offline contacts to whom you have a different visibility"),
	_T("Contacts which are 'not on list'"),
};

static WORD s_rgwFontSameAsDefault[FLT_FONTIDS] =
{
	MAKEWORD(FLT_SAME_AS_MIRANDA, 0x0F),
	MAKEWORD(FLT_SAME_AS_MIRANDA, 0x0F),
	MAKEWORD(FLT_SAME_AS_MIRANDA, 0x0F),
	MAKEWORD(FLT_SAME_AS_MIRANDA, 0x0F),
	MAKEWORD(FLT_SAME_AS_MIRANDA, 0x0F),
};

static int s_rgnMirandaFontId[FLT_FONTIDS] =
{
	FONTID_CONTACTS,
	FONTID_INVIS,
	FONTID_OFFLINE,
	FONTID_OFFINVIS,
	FONTID_NOTONLIST
};

#define M_REBUILDFONTGROUP		(WM_USER + 10)
#define M_REMAKESAMPLE			(WM_USER + 11)
#define M_RECALCONEFONT			(WM_USER + 12)
#define M_RECALCOTHERFONTS		(WM_USER + 13)
#define M_SAVEFONT				(WM_USER + 14)
#define M_REFRESHSAMEASBOXES	(WM_USER + 15)
#define M_FILLSCRIPTCOMBO		(WM_USER + 16)
#define M_LOADFONT				(WM_USER + 17)
#define M_GUESSSAMEASBOXES		(WM_USER + 18)
#define M_SETSAMEASBOXES		(WM_USER + 19)

#define M_REFRESHBKGBOXES		(WM_USER + 20)
#define M_REFRESHBORDERPICKERS	(WM_USER + 21)

/////////////////////////////////////////////////////////////////////////////
//

static
LRESULT
APIENTRY
OptWndProc
	( IN HWND hwndDlg
	, IN UINT uMsg
	, IN WPARAM wParam
	, IN LPARAM lParam
	);

static
LRESULT
APIENTRY
OptSknWndProc
	( IN HWND hwndDlg
	, IN UINT uMsg
	, IN WPARAM wParam
	, IN LPARAM lParam
	);

/////////////////////////////////////////////////////////////////////////////
//

int
OnOptionsInitialize
	( IN WPARAM wParam
	, IN LPARAM lParam
	)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize			= sizeof(odp);
	odp.hInstance		= hInst;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_FLTCONT);
	odp.ptszTitle		= _T("Floating Contacts");
	odp.ptszGroup		= _T("Plugins");
	odp.ptszTab			= _T("Main Features");
	odp.flags			= ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.pfnDlgProc		= (DLGPROC)OptWndProc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize			= sizeof(odp);
	odp.hInstance		= hInst;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.ptszTitle		= _T("Floating Contacts");
	odp.ptszGroup		= _T("Plugins");
	odp.ptszTab			= _T("Appearance");
	odp.flags			= ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.pfnDlgProc		= (DLGPROC)OptSknWndProc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

static
int
CALLBACK
EnumFontsProc
	( IN ENUMLOGFONTEXA* lpelfe
	, IN NEWTEXTMETRICEXA* lpntme
	, IN int FontType
	, IN LPARAM lParam
	)
{
	if (IsWindow((HWND)lParam))
	{
		if (CB_ERR == SendMessageA((HWND)lParam, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)lpelfe->elfLogFont.lfFaceName))
			SendMessageA((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfLogFont.lfFaceName);
		return TRUE;
	}
	return FALSE;
}

void
FillFontListThread
	( IN HWND hwndDlg
	)
{
	LOGFONTA lf = {0};
	HDC hdc = GetDC(hwndDlg);

	lf.lfCharSet		= DEFAULT_CHARSET;
	lf.lfFaceName[0]	= 0;
	lf.lfPitchAndFamily	= 0;
	EnumFontFamiliesExA(hdc, &lf, (FONTENUMPROCA)EnumFontsProc, (LPARAM)GetDlgItem(hwndDlg,IDC_TYPEFACE), 0);
	ReleaseDC(hwndDlg, hdc);
	return;
}

static
int
CALLBACK
EnumFontScriptsProc
	( IN ENUMLOGFONTEX* lpelfe
	, IN NEWTEXTMETRICEX* lpntme
	, IN int FontType
	, IN LPARAM lParam
	)
{
	if (CB_ERR == SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)lpelfe->elfScript))
	{
		int i = SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfScript);
		SendMessage((HWND)lParam, CB_SETITEMDATA, i, lpelfe->elfLogFont.lfCharSet);
	}
	return TRUE;
}

static
void
GetDefaultFontSetting
	( IN BOOL bFltContacts
	, IN int nFontId
	, IN LOGFONTA* lf
	, IN COLORREF* colour
	)
{
	SystemParametersInfoA(SPI_GETICONTITLELOGFONT, sizeof(LOGFONTA), lf, FALSE);
	*colour = GetSysColor(COLOR_WINDOWTEXT);
	if (bFltContacts)
	{
		switch (nFontId)
		{
		case FLT_FONTID_OFFINVIS:
		case FLT_FONTID_INVIS:
			lf->lfItalic = !lf->lfItalic;
			break;

		case FLT_FONTID_NOTONLIST:
			*colour = GetSysColor(COLOR_3DSHADOW);
			break;
		}
	}
	else
	{
		switch (s_rgnMirandaFontId[nFontId])
		{
		case FONTID_OFFINVIS:
		case FONTID_INVIS:
			lf->lfItalic = !lf->lfItalic;
			break;

		case FONTID_NOTONLIST:
			*colour = GetSysColor(COLOR_3DSHADOW);
			break;
		}
	}
}

void
GetFontSetting
	( IN BOOL bFltContacts
	, IN int nFontId
	, IN LOGFONTA* lf
	, IN COLORREF* colour
	)
{
	DBVARIANT dbv;
	char idstr[10];
	BYTE style;
	const char* pModule = (bFltContacts ? sModule : "CLC");

	GetDefaultFontSetting(bFltContacts, nFontId, lf, colour);
	if (!bFltContacts)
		nFontId = s_rgnMirandaFontId[nFontId];
	wsprintfA(idstr, "Font%dName", nFontId);
	if (!DBGetContactSetting(NULL, pModule, idstr, &dbv))
	{
		lstrcpyA(lf->lfFaceName, dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	wsprintfA(idstr, "Font%dCol", nFontId);
	*colour = DBGetContactSettingDword(NULL, pModule, idstr, *colour);

	wsprintfA(idstr, "Font%dSize", nFontId);
	lf->lfHeight = (char)DBGetContactSettingByte(NULL, pModule, idstr, lf->lfHeight);

	wsprintfA(idstr, "Font%dSty", nFontId);
	style = (BYTE)DBGetContactSettingByte(NULL, pModule, idstr
							, (lf->lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD)
								| (lf->lfItalic ? DBFONTF_ITALIC : 0)
								| (lf->lfUnderline ? DBFONTF_UNDERLINE : 0)
							);
	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = ((style & DBFONTF_BOLD) ? FW_BOLD : FW_NORMAL);
	lf->lfItalic = (0 != (style & DBFONTF_ITALIC));
	lf->lfUnderline = (0 != (style & DBFONTF_UNDERLINE));
	lf->lfStrikeOut = 0;

	wsprintfA(idstr, "Font%dSet", nFontId);
	lf->lfCharSet = (BYTE)DBGetContactSettingByte(NULL, pModule, idstr, lf->lfCharSet);
	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	if (bFltContacts)
	{
		WORD wSameAs;
		BYTE bySameAs;
		BYTE bySameAsFlags;

		wsprintfA(idstr, "Font%dAs", nFontId);
		wSameAs = (WORD)DBGetContactSettingWord(NULL, sModule, idstr, s_rgwFontSameAsDefault[nFontId]);
		bySameAs=LOBYTE(wSameAs);
		bySameAsFlags=HIBYTE(wSameAs);

		if (FLT_SAME_AS_MIRANDA == bySameAs)
		{
			LOGFONTA lfMiranda;
			COLORREF colourMiranda;

			GetFontSetting(FALSE, nFontId, &lfMiranda, &colourMiranda);
			if (bySameAsFlags & SAMEASF_FACE)
			{
				lstrcpyA(lf->lfFaceName, lfMiranda.lfFaceName);
				lf->lfCharSet = lfMiranda.lfCharSet;
			}
			if (bySameAsFlags & SAMEASF_SIZE)
				lf->lfHeight = lfMiranda.lfHeight;
			if (bySameAsFlags & SAMEASF_STYLE)
			{
				lf->lfWeight = lfMiranda.lfWeight;
				lf->lfItalic = lfMiranda.lfItalic;
				lf->lfUnderline = lfMiranda.lfUnderline;
			}
			if (bySameAsFlags & SAMEASF_COLOUR)
				*colour = colourMiranda;
		}
	}
}

static
LRESULT
APIENTRY
OptWndProc
	( IN HWND hwndDlg
	, IN UINT uMsg
	, IN WPARAM wParam
	, IN LPARAM lParam
	)
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			// Properties
			CheckDlgButton(hwndDlg, IDC_CHK_HIDE_OFFLINE, (fcOpt.bHideOffline ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_CHK_HIDE_ALL, (fcOpt.bHideAll ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_CHK_HIDE_WHEN_FULSCREEN, (fcOpt.bHideWhenFullscreen ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_CHK_STICK, (fcOpt.bMoveTogether ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_CHK_WIDTH, (fcOpt.bFixedWidth ? BST_CHECKED : BST_UNCHECKED));
			
			EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_WIDTH), fcOpt.bFixedWidth);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_WIDTH), fcOpt.bFixedWidth);
			EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTHSPIN), fcOpt.bFixedWidth);

			SendDlgItemMessage(hwndDlg, IDC_WIDTHSPIN, UDM_SETRANGE, 0, MAKELONG(255,0));
			SendDlgItemMessage(hwndDlg, IDC_WIDTHSPIN, UDM_SETPOS, 0, fcOpt.nThumbWidth);

			CheckDlgButton(hwndDlg, IDC_CHK_TIP, (fcOpt.bShowTip ? BST_CHECKED : BST_UNCHECKED));
		
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_TIP), bEnableTip);
			
			EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN), bEnableTip && fcOpt.bShowTip);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN_CMT), bEnableTip && fcOpt.bShowTip);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TIMEIN), bEnableTip && fcOpt.bShowTip);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEINSPIN), bEnableTip && fcOpt.bShowTip);

			SendDlgItemMessage(hwndDlg, IDC_TIMEINSPIN, UDM_SETRANGE, 0, MAKELONG(5000,0));
			SendDlgItemMessage(hwndDlg, IDC_TIMEINSPIN, UDM_SETPOS, 0, fcOpt.TimeIn);

			CheckDlgButton(hwndDlg, IDC_CHK_TOTOP, (fcOpt.bToTop ? BST_CHECKED : BST_UNCHECKED));

			EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TOTOP), fcOpt.bToTop);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TOTOPTIME), fcOpt.bToTop);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOTOPTIMESPIN), fcOpt.bToTop);

			SendDlgItemMessage(hwndDlg, IDC_TOTOPTIMESPIN, UDM_SETRANGE, 0, MAKELONG(TOTOPTIME_MAX,1));
			SendDlgItemMessage(hwndDlg, IDC_TOTOPTIMESPIN, UDM_SETPOS, 0, fcOpt.ToTopTime);

			CheckDlgButton(hwndDlg, IDC_CHK_HIDE_WHEN_CLISTSHOW, (fcOpt.bHideWhenCListShow ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_CHK_SINGLECLK, (fcOpt.bUseSingleClick ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_CHK_SHOWIDLE, (fcOpt.bShowIdle ? BST_CHECKED : BST_UNCHECKED));

			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CHK_WIDTH:
				{
					if (BN_CLICKED == HIWORD(wParam))
					{
						BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_WIDTH);
						
						EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_WIDTH ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_WIDTH ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTHSPIN), bChecked);
					}
					break;
				}
				case IDC_TXT_WIDTH:
				{
					if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
						return 0;
					break;
				}
				case IDC_CHK_TIP:
				{
					if (BN_CLICKED == HIWORD(wParam))
					{
						BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TIP);
						
						EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN_CMT ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TIMEIN ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEINSPIN), bChecked);
					}
					break;
				}
				case IDC_TXT_TIMEIN:
				{
					if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
						return 0;
					break;
				}
				case IDC_CHK_TOTOP:
				{
					if (BN_CLICKED == HIWORD(wParam))
					{
						BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TOTOP);
						
						EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TOTOP ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TOTOPTIME ), bChecked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TOTOPTIMESPIN), bChecked);
					}
					break;
				}
				case IDC_TXT_TOTOPTIME:
				{
					if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
						return 0;
					break;
				}
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR phdr = (LPNMHDR)(lParam);
			
			if (0 == phdr->idFrom)
			{
				switch (phdr->code) 
				{
					case PSN_APPLY:
					{
						BOOL bSuccess = FALSE;

						fcOpt.bHideOffline = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_OFFLINE);
						DBWriteContactSettingByte(NULL, sModule, "HideOffline", (BYTE)fcOpt.bHideOffline);
						
						fcOpt.bHideAll = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_ALL);
						DBWriteContactSettingByte(NULL, sModule, "HideAll", (BYTE)fcOpt.bHideAll);
						
						fcOpt.bHideWhenFullscreen = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_WHEN_FULSCREEN);
						DBWriteContactSettingByte(NULL, sModule, "HideWhenFullscreen", (BYTE)fcOpt.bHideWhenFullscreen);
						
						fcOpt.bMoveTogether = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_STICK);
						DBWriteContactSettingByte(NULL, sModule, "MoveTogether", (BYTE)fcOpt.bMoveTogether);
						
						fcOpt.bFixedWidth = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_WIDTH);
						DBWriteContactSettingByte(NULL, sModule, "FixedWidth", (BYTE)fcOpt.bFixedWidth);
						fcOpt.nThumbWidth	= GetDlgItemInt(hwndDlg, IDC_TXT_WIDTH, &bSuccess, FALSE);
						DBWriteContactSettingDword(NULL, sModule, "Width", fcOpt.nThumbWidth );
						
						if(bEnableTip)
						{
							fcOpt.bShowTip = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TIP);
							DBWriteContactSettingByte(NULL, sModule, "ShowTip", (BYTE)fcOpt.bShowTip);
							fcOpt.TimeIn	= GetDlgItemInt(hwndDlg, IDC_TXT_TIMEIN, &bSuccess, FALSE);
							DBWriteContactSettingWord(NULL, sModule, "TimeIn", fcOpt.TimeIn );
						}

						fcOpt.bToTop = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TOTOP);
						DBWriteContactSettingByte(NULL, sModule, "ToTop", (BYTE)fcOpt.bToTop);
						fcOpt.ToTopTime	= GetDlgItemInt(hwndDlg, IDC_TXT_TOTOPTIME, &bSuccess, FALSE);
						DBWriteContactSettingWord(NULL, sModule, "ToTopTime", fcOpt.ToTopTime );
	
						fcOpt.bHideWhenCListShow = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_WHEN_CLISTSHOW);
						DBWriteContactSettingByte(NULL, sModule, "HideWhenCListShow", (BYTE)fcOpt.bHideWhenCListShow);
						
						fcOpt.bUseSingleClick = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_SINGLECLK);
						DBWriteContactSettingByte(NULL, sModule, "UseSingleClick", (BYTE)fcOpt.bUseSingleClick);

						fcOpt.bShowIdle = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWIDLE);
						DBWriteContactSettingByte(NULL, sModule, "ShowIdle", (BYTE)fcOpt.bShowIdle);

						ApplyOptionsChanges();
						OnStatusChanged();
						return TRUE;
					}
				}
			}
			break;
		}
	}
	return FALSE;
}

static
LRESULT
APIENTRY
OptSknWndProc
	( IN HWND hwndDlg
	, IN UINT uMsg
	, IN WPARAM wParam
	, IN LPARAM lParam
	)
{
	static HFONT hFontSample;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			BYTE btOpacity;
			char szPercent[20];

			TranslateDialogDefault(hwndDlg);

			// Border
			CheckDlgButton(hwndDlg, IDC_DRAWBORDER
				, DBGetContactSettingByte(NULL, sModule, "DrawBorder", FLT_DEFAULT_DRAWBORDER)
					? BST_CHECKED : BST_UNCHECKED
				);
			SendMessage(hwndDlg, M_REFRESHBORDERPICKERS, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_LTEDGESCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_SETCOLOUR, 0
				, DBGetContactSettingDword(NULL, sModule, "LTEdgesColor", FLT_DEFAULT_LTEDGESCOLOR)
				);
			SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_RBEDGESCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_SETCOLOUR, 0
				, DBGetContactSettingDword(NULL, sModule, "RBEdgesColor", FLT_DEFAULT_RBEDGESCOLOR)
				);

			// Background
			CheckDlgButton(hwndDlg, IDC_CHK_WIDTH, (fcOpt.bFixedWidth ? BST_CHECKED : BST_UNCHECKED));

			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_BKGNDCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0
				, DBGetContactSettingDword(NULL, sModule, "BkColor", FLT_DEFAULT_BKGNDCOLOR)
				);
			CheckDlgButton(hwndDlg, IDC_BITMAP
				, DBGetContactSettingByte(NULL, sModule, "BkUseBitmap", FLT_DEFAULT_BKGNDUSEBITMAP)
					? BST_CHECKED : BST_UNCHECKED
				);
			SendMessage(hwndDlg, M_REFRESHBKGBOXES, 0, 0);
			{
				DBVARIANT dbv;

				if (!DBGetContactSetting(NULL, sModule, "BkBitmap", &dbv))
				{
					SetDlgItemTextA(hwndDlg, IDC_FILENAME, dbv.pszVal);
					DBFreeVariant(&dbv);
				}
			}
			{
				WORD bmpUse = (WORD)DBGetContactSettingWord(NULL, sModule, "BkBitmapOpt", FLT_DEFAULT_BKGNDBITMAPOPT);

				CheckDlgButton(hwndDlg, IDC_STRETCHH, ((bmpUse & CLB_STRETCHH) ? BST_CHECKED : BST_UNCHECKED));
				CheckDlgButton(hwndDlg, IDC_STRETCHV, ((bmpUse & CLB_STRETCHV) ? BST_CHECKED : BST_UNCHECKED));
				CheckDlgButton(hwndDlg, IDC_TILEH, ((bmpUse & CLBF_TILEH) ? BST_CHECKED : BST_UNCHECKED));
				CheckDlgButton(hwndDlg, IDC_TILEV, ((bmpUse & CLBF_TILEV) ? BST_CHECKED : BST_UNCHECKED));
				CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, ((bmpUse & CLBF_PROPORTIONAL) ? BST_CHECKED : BST_UNCHECKED));
			}
			{
				HRESULT (STDAPICALLTYPE *MySHAutoComplete)(HWND,DWORD);

				MySHAutoComplete = (HRESULT (STDAPICALLTYPE*)(HWND,DWORD))GetProcAddress(GetModuleHandle(_T("shlwapi")), "SHAutoComplete");
				if (MySHAutoComplete)
					MySHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
			}

			// Windows 2K/XP
			btOpacity = (BYTE)DBGetContactSettingByte(NULL, sModule, "Opacity", 100);
			SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
			SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_SETPOS, TRUE, btOpacity);
				
			wsprintfA(szPercent, "%d%%", btOpacity);
			SetDlgItemTextA(hwndDlg, IDC_OPACITY, szPercent);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SLIDER_OPACITY), (NULL != pSetLayeredWindowAttributes));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPACITY), (NULL != pSetLayeredWindowAttributes));

			// Fonts
			hFontSample = NULL;
			SetDlgItemText(hwndDlg, IDC_SAMPLE, TranslateT("Sample"));
			FillFontListThread(hwndDlg);
			{
				int i;
				int itemId;
				LOGFONTA lf;
				COLORREF colour;
				WORD sameAs;
				char str[32];

				for (i = 0; i < FLT_FONTIDS; i++)
				{
					// Floating contacts fonts
					GetFontSetting(TRUE, i, &lf, &colour);
					wsprintfA(str, "Font%dAs", i);
					sameAs = (WORD)DBGetContactSettingWord(NULL, sModule, str, s_rgwFontSameAsDefault[i]);
					s_rgFontSettings[i].sameAs = LOBYTE(sameAs);
					s_rgFontSettings[i].sameAsFlags = HIBYTE(sameAs);
					s_rgFontSettings[i].style = (FW_NORMAL == lf.lfWeight? 0 : DBFONTF_BOLD)
													| (lf.lfItalic ? DBFONTF_ITALIC : 0)
													| (lf.lfUnderline ? DBFONTF_UNDERLINE : 0);
					if (lf.lfHeight < 0)
					{
						HDC hdc;
						SIZE size;
						HFONT hFont = CreateFontIndirectA(&lf);

						hdc=GetDC(hwndDlg);
						SelectObject(hdc, hFont);
						GetTextExtentPoint32A(hdc, "x", 1, &size);
						ReleaseDC(hwndDlg, hdc);
						DeleteObject(hFont);
						s_rgFontSettings[i].size = (char)size.cy;
					}
					else
						s_rgFontSettings[i].size = (char)lf.lfHeight;
					s_rgFontSettings[i].charset = lf.lfCharSet;
					s_rgFontSettings[i].colour = colour;
					lstrcpyA(s_rgFontSettings[i].szFace, lf.lfFaceName);
					itemId = SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_ADDSTRING, 0, (LPARAM)TranslateTS(s_rgszFontIdDescr[i]));
					SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_SETITEMDATA, itemId, i);

					// Miranda contact list fonts
					GetFontSetting(FALSE, i, &lf, &colour);
					s_rgFontSettingsMiranda[i].sameAs = 0;
					s_rgFontSettingsMiranda[i].sameAsFlags = 0;
					lstrcpyA(s_rgFontSettingsMiranda[i].szFace, lf.lfFaceName);
					s_rgFontSettingsMiranda[i].charset = lf.lfCharSet;
					s_rgFontSettingsMiranda[i].style = (FW_NORMAL == lf.lfWeight? 0 : DBFONTF_BOLD)
															| (lf.lfItalic ? DBFONTF_ITALIC : 0)
															| (lf.lfUnderline ? DBFONTF_UNDERLINE : 0);
					if (lf.lfHeight < 0)
					{
						HDC hdc;
						SIZE size;
						HFONT hFont = CreateFontIndirectA(&lf);

						hdc = GetDC(hwndDlg);
						SelectObject(hdc, hFont);
						GetTextExtentPoint32A(hdc, "x", 1, &size);
						ReleaseDC(hwndDlg, hdc);
						DeleteObject(hFont);
						s_rgFontSettingsMiranda[i].size = (char)size.cy;
					}
					else
						s_rgFontSettingsMiranda[i].size = (char)lf.lfHeight;
					s_rgFontSettingsMiranda[i].colour = colour;
				}
				SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_SETCURSEL, 0, 0);
				for (i = 0; i < sizeof(s_rgszFontSizes)/sizeof(s_rgszFontSizes[0]); i++)
					SendDlgItemMessageA(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)s_rgszFontSizes[i]);
			}
			SendMessage(hwndDlg, M_REBUILDFONTGROUP, 0, 0);
			SendMessage(hwndDlg, M_SAVEFONT, 0, 0);
			return TRUE;
		}
		case M_REFRESHBKGBOXES:
		{
			BOOL bEnable = IsDlgButtonChecked(hwndDlg, IDC_BITMAP);

			EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHH), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHV), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TILEH), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TILEV), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROPORTIONAL), bEnable);
			break;
		}
		case M_REFRESHBORDERPICKERS:
		{
			BOOL bEnable = IsDlgButtonChecked(hwndDlg, IDC_DRAWBORDER);

			EnableWindow(GetDlgItem(hwndDlg, IDC_LTEDGESCOLOR), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RBEDGESCOLOR), bEnable);
			break;
		}
		// remake all the needed controls when the user changes the font selector at the top
		case M_REBUILDFONTGROUP:
		{
			int i = SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_GETCURSEL, 0, 0);
			int j;
			int itemId;
			int nSameAs = FLT_SAME_AS_NONE;
			char szText[256];

			SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_RESETCONTENT, 0, 0);
			itemId = SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_ADDSTRING, 0, (LPARAM)TranslateT("<none>"));
			SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_SETITEMDATA, itemId, FLT_SAME_AS_NONE);
			SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_SETCURSEL, itemId, 0);
			itemId = SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Contact List Text>"));
			SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_SETITEMDATA, itemId, FLT_SAME_AS_MIRANDA);
			if (FLT_SAME_AS_MIRANDA == s_rgFontSettings[i].sameAs)
			{
				SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_SETCURSEL, itemId, 0);
				nSameAs = FLT_SAME_AS_MIRANDA;
			}
			for (j = 0; j < FLT_FONTIDS; j++)
			{
				int nDependsOn = j;

				while (nDependsOn != i)
				{
					if (FLT_SAME_AS_NONE == nDependsOn || FLT_SAME_AS_MIRANDA == nDependsOn)
					{
						SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_GETLBTEXT, j, (LPARAM)szText);
						itemId = SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_ADDSTRING, 0, (LPARAM)szText);
						SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_SETITEMDATA, itemId, j);
						if (j == s_rgFontSettings[i].sameAs)
						{
							SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_SETCURSEL, itemId, 0);
							nSameAs = j;
						}
						break;
					}
					else
						nDependsOn = s_rgFontSettings[nDependsOn].sameAs;
				}
			}
			if (FLT_SAME_AS_NONE == nSameAs)
				s_rgFontSettings[i].sameAsFlags = 0;

			SendMessage(hwndDlg, M_LOADFONT, i, 0);
			SendMessage(hwndDlg, M_SETSAMEASBOXES, i, 0);
			SendMessage(hwndDlg, M_REFRESHSAMEASBOXES, i, 0);
			SendMessage(hwndDlg, M_REMAKESAMPLE, 0, 0);
			break;
		}
		//fill the script combo box and set the selection to the value for fontid wParam
		case M_FILLSCRIPTCOMBO:
		{	
			int i;
			HDC hdc = GetDC(hwndDlg);
			LOGFONT lf = {0};

			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfPitchAndFamily = 0;
			GetDlgItemText(hwndDlg, IDC_TYPEFACE, lf.lfFaceName, sizeof(lf.lfFaceName));
			SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_RESETCONTENT, 0, 0);
			EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontScriptsProc, (LPARAM)GetDlgItem(hwndDlg, IDC_SCRIPT), 0);
			ReleaseDC(hwndDlg, hdc);
			for (i = SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--)
			{
				if (SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETITEMDATA, i, 0) == s_rgFontSettings[wParam].charset)
				{
					SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_SETCURSEL, i, 0);
					break;
				}
			}
			if (i < 0)
				SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_SETCURSEL, 0, 0);
			break;
		}
		// set the check mark in the 'same as' boxes to the right value for fontid wParam
		case M_SETSAMEASBOXES:
		{
			CheckDlgButton(hwndDlg, IDC_SAMETYPE, (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_FACE ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_SAMESIZE, (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_SIZE ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_SAMESTYLE, (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_STYLE ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_SAMECOLOUR, (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_COLOUR ? BST_CHECKED : BST_UNCHECKED));
			break;
		}
		// set the disabled flag on the 'same as' checkboxes to the values for fontid wParam
		case M_REFRESHSAMEASBOXES:
		{
			BOOL bSameAsNone = (FLT_SAME_AS_NONE == s_rgFontSettings[wParam].sameAs);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SAMETYPE), !bSameAsNone);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SAMESIZE), !bSameAsNone);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SAMESTYLE), !bSameAsNone);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SAMECOLOUR), !bSameAsNone);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEFACE), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_FACE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCRIPT), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_FACE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_FONTSIZE), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_SIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BOLD), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_STYLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ITALIC), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_STYLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_UNDERLINE), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_STYLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOUR), bSameAsNone || !(s_rgFontSettings[wParam].sameAsFlags & SAMEASF_COLOUR));
			break;
		}
		// remake the sample edit box font based on the settings in the controls
		case M_REMAKESAMPLE:
		{
			LOGFONTA lf;

			if (hFontSample)
			{
				SendDlgItemMessage(hwndDlg, IDC_SAMPLE, WM_SETFONT, SendDlgItemMessage(hwndDlg, IDC_FONTID, WM_GETFONT, 0, 0), 0);
				DeleteObject(hFontSample);
			}
			lf.lfHeight = GetDlgItemInt(hwndDlg, IDC_FONTSIZE, NULL, FALSE);
			{
				HDC hdc=GetDC(NULL);				
				lf.lfHeight=-MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				ReleaseDC(NULL,hdc);				
			}
			lf.lfWidth = 0;
			lf.lfEscapement = 0;
			lf.lfOrientation = 0;
			lf.lfWeight = (IsDlgButtonChecked(hwndDlg, IDC_BOLD) ? FW_BOLD : FW_NORMAL);
			lf.lfItalic = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ITALIC);
			lf.lfUnderline = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_UNDERLINE);
			lf.lfStrikeOut = 0;
			lf.lfCharSet = (BYTE)SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCURSEL, 0, 0), 0);
			lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfQuality = DEFAULT_QUALITY;
			lf.lfPitchAndFamily = (DEFAULT_PITCH | FF_DONTCARE);
			GetDlgItemTextA(hwndDlg, IDC_TYPEFACE, lf.lfFaceName, sizeof(lf.lfFaceName));
			if (NULL != (hFontSample = CreateFontIndirectA(&lf)))
				SendDlgItemMessage(hwndDlg, IDC_SAMPLE, WM_SETFONT, (WPARAM)hFontSample, TRUE);
			break;
		}
		// copy the 'same as' settings for fontid wParam from their sources
		case M_RECALCONEFONT:
		{
			if (FLT_SAME_AS_NONE != s_rgFontSettings[wParam].sameAs)
			{
				SFontSettings* pSameAs = ((FLT_SAME_AS_MIRANDA == s_rgFontSettings[wParam].sameAs)
											? &s_rgFontSettingsMiranda[wParam]
											: &s_rgFontSettings[s_rgFontSettings[wParam].sameAs]
											);

				if (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_FACE)
				{
					lstrcpyA(s_rgFontSettings[wParam].szFace, pSameAs->szFace);
					s_rgFontSettings[wParam].charset = pSameAs->charset;
				}
				if (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_SIZE)
					s_rgFontSettings[wParam].size = pSameAs->size;
				if (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_STYLE)
					s_rgFontSettings[wParam].style = pSameAs->style;
				if (s_rgFontSettings[wParam].sameAsFlags & SAMEASF_COLOUR)
					s_rgFontSettings[wParam].colour = pSameAs->colour;
			}
			break;
		}
		// recalculate the 'same as' settings for all fonts but wParam
		case M_RECALCOTHERFONTS:
		{	
			int nFont;
			int nDepth;
			int nRecalcedFonts = 1;
			int nRecalcDepth;
			int nFontId = (int)wParam;
			int nSameAs;

			for (nRecalcDepth = 0; nRecalcedFonts < FLT_FONTIDS && nRecalcDepth < FLT_FONTIDS; nRecalcDepth++)
			{
				for (nFont = 0; nFont < FLT_FONTIDS; nFont++)
				{
					if (nFontId == nFont)
						continue;

					nSameAs = s_rgFontSettings[nFont].sameAs;
					for (nDepth = 0; nDepth < nRecalcDepth; nDepth++)
					{
						if (FLT_SAME_AS_NONE == nSameAs || FLT_SAME_AS_MIRANDA == nSameAs || nFontId == nSameAs)
							break;

						nSameAs = s_rgFontSettings[nSameAs].sameAs;
					}
					if (nDepth == nRecalcDepth)
					{
						if (nFontId == nSameAs)
						{
							SendMessage(hwndDlg, M_RECALCONEFONT, nFont, 0);
							nRecalcedFonts++;
						}
						else if (FLT_SAME_AS_NONE == nSameAs || FLT_SAME_AS_MIRANDA == nSameAs)
							nRecalcedFonts++;
					}
				}
			}
			break;
		}
		//save the font settings from the controls to font wParam
		case M_SAVEFONT:
		{
			s_rgFontSettings[wParam].sameAsFlags = (IsDlgButtonChecked(hwndDlg, IDC_SAMETYPE) ? SAMEASF_FACE : 0)
													| (IsDlgButtonChecked(hwndDlg, IDC_SAMESIZE) ? SAMEASF_SIZE : 0)
													| (IsDlgButtonChecked(hwndDlg, IDC_SAMESTYLE) ? SAMEASF_STYLE : 0)
													| (IsDlgButtonChecked(hwndDlg, IDC_SAMECOLOUR) ? SAMEASF_COLOUR : 0);
			s_rgFontSettings[wParam].sameAs = (BYTE)SendDlgItemMessage(hwndDlg
														, IDC_SAMEAS
														, CB_GETITEMDATA
														, SendDlgItemMessage(hwndDlg, IDC_SAMEAS, CB_GETCURSEL, 0, 0)
														, 0
														);
			GetDlgItemTextA(hwndDlg, IDC_TYPEFACE, s_rgFontSettings[wParam].szFace, sizeof(s_rgFontSettings[wParam].szFace));
			s_rgFontSettings[wParam].charset = (BYTE)SendDlgItemMessage(hwndDlg
														, IDC_SCRIPT
														, CB_GETITEMDATA
														, SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCURSEL, 0, 0)
														, 0
														);
			s_rgFontSettings[wParam].size = (char)GetDlgItemInt(hwndDlg, IDC_FONTSIZE, NULL, FALSE);
			s_rgFontSettings[wParam].style= (IsDlgButtonChecked(hwndDlg, IDC_BOLD) ? DBFONTF_BOLD : 0)
												| (IsDlgButtonChecked(hwndDlg, IDC_ITALIC) ? DBFONTF_ITALIC : 0)
												| (IsDlgButtonChecked(hwndDlg, IDC_UNDERLINE) ? DBFONTF_UNDERLINE : 0);
			s_rgFontSettings[wParam].colour = SendDlgItemMessage(hwndDlg, IDC_COLOUR, CPM_GETCOLOUR, 0, 0);
			break;
		}
		// load font wParam into the controls
		case M_LOADFONT:
		{
			LOGFONTA lf;
			COLORREF colour;

			SetDlgItemTextA(hwndDlg, IDC_TYPEFACE, s_rgFontSettings[wParam].szFace);
			SendMessage(hwndDlg, M_FILLSCRIPTCOMBO, wParam, 0);
			SetDlgItemInt(hwndDlg, IDC_FONTSIZE, s_rgFontSettings[wParam].size, FALSE);
			CheckDlgButton(hwndDlg, IDC_BOLD, ((s_rgFontSettings[wParam].style & DBFONTF_BOLD) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_ITALIC, ((s_rgFontSettings[wParam].style & DBFONTF_ITALIC) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_UNDERLINE, ((s_rgFontSettings[wParam].style & DBFONTF_UNDERLINE) ? BST_CHECKED : BST_UNCHECKED));
			GetDefaultFontSetting(TRUE, wParam, &lf, &colour);
			SendDlgItemMessage(hwndDlg, IDC_COLOUR, CPM_SETDEFAULTCOLOUR, 0, colour);
			SendDlgItemMessage(hwndDlg, IDC_COLOUR, CPM_SETCOLOUR, 0, s_rgFontSettings[wParam].colour);
			break;
		}
		// guess suitable values for the 'same as' checkboxes for fontId wParam
		case M_GUESSSAMEASBOXES:
		{
			s_rgFontSettings[wParam].sameAsFlags = 0;
			if (FLT_SAME_AS_NONE != s_rgFontSettings[wParam].sameAs)
			{
				SFontSettings* pSameAs = ((FLT_SAME_AS_MIRANDA == s_rgFontSettings[wParam].sameAs)
											? &s_rgFontSettingsMiranda[wParam]
											: &s_rgFontSettings[s_rgFontSettings[wParam].sameAs]
											);

				if (!lstrcmpA(s_rgFontSettings[wParam].szFace, pSameAs->szFace) && s_rgFontSettings[wParam].charset == pSameAs->charset)
    				s_rgFontSettings[wParam].sameAsFlags |= SAMEASF_FACE;
				if (s_rgFontSettings[wParam].size == pSameAs->size)
    				s_rgFontSettings[wParam].sameAsFlags |= SAMEASF_SIZE;
				if (s_rgFontSettings[wParam].style == pSameAs->style)
    				s_rgFontSettings[wParam].sameAsFlags |= SAMEASF_STYLE;
				if (s_rgFontSettings[wParam].colour == pSameAs->colour)
    				s_rgFontSettings[wParam].sameAsFlags |= SAMEASF_COLOUR;
				SendMessage(hwndDlg,M_SETSAMEASBOXES,wParam,0);
			}
			break;
		}
		case WM_CTLCOLORSTATIC:
		{
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SAMPLE))
			{
				SetTextColor((HDC)wParam, SendDlgItemMessage(hwndDlg, IDC_COLOUR, CPM_GETCOLOUR, 0, 0));
				SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));
				return (BOOL)GetSysColorBrush(COLOR_3DFACE);
			}
			break;
		}
		case WM_HSCROLL:
		{
			if (wParam != TB_ENDTRACK)
			{
				int nPos;
				char szPercent[20];

				nPos = (BYTE)SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0);
				fcOpt.thumbAlpha	= (BYTE)(( nPos * 255 ) / 100 );
				SetThumbsOpacity(fcOpt.thumbAlpha);
				
				wsprintfA(szPercent, "%d%%", nPos);
				SetDlgItemTextA(hwndDlg, IDC_OPACITY, szPercent);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		}
		case WM_COMMAND:
		{
			int nFontId = SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_GETITEMDATA
								, SendDlgItemMessage(hwndDlg, IDC_FONTID, CB_GETCURSEL, 0, 0)
								, 0
								);

			switch (LOWORD(wParam))
			{
				case IDC_DRAWBORDER:
				{
					SendMessage(hwndDlg, M_REFRESHBORDERPICKERS, 0, 0);
					break;
				}
				case IDC_BROWSE:
				{
					char str[MAX_PATH];
					OPENFILENAMEA ofn={0};
					char filter[512];

					GetDlgItemTextA(hwndDlg, IDC_FILENAME, str, sizeof(str));
					ofn.lStructSize = sizeof(OPENFILENAMEA);
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof(filter), (LPARAM)filter);
					ofn.lpstrFilter = filter;
					ofn.lpstrFile = str;
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					ofn.nMaxFile = sizeof(str);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = "bmp";
					if (!GetOpenFileNameA(&ofn))
						return FALSE;
					SetDlgItemTextA(hwndDlg, IDC_FILENAME, str);
					break;
				}
				case IDC_FILENAME:
				{
					if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
						return FALSE;
					break;
				}
				case IDC_BITMAP:
				{
					SendMessage(hwndDlg, M_REFRESHBKGBOXES, 0, 0);
					break;
				}
				case IDC_FONTID:
				{
					if (CBN_SELCHANGE == HIWORD(wParam))
						SendMessage(hwndDlg, M_REBUILDFONTGROUP, 0, 0);
					return FALSE;
				}
				case IDC_SAMETYPE:
				case IDC_SAMESIZE:
				case IDC_SAMESTYLE:
				case IDC_SAMECOLOUR:
				{
					SendMessage(hwndDlg, M_SAVEFONT, nFontId, 0);
					SendMessage(hwndDlg, M_RECALCONEFONT, nFontId, 0);
					SendMessage(hwndDlg, M_RECALCOTHERFONTS, nFontId, 0);
					SendMessage(hwndDlg, M_LOADFONT, nFontId, 0);
					SendMessage(hwndDlg, M_REFRESHSAMEASBOXES, nFontId, 0);
					SendMessage(hwndDlg, M_REMAKESAMPLE, 0, 0);
					break;
				}
				case IDC_SAMEAS:
				{
					if (CBN_SELCHANGE != HIWORD(wParam))
						return FALSE;
					SendMessage(hwndDlg, M_SAVEFONT, nFontId, 0);
					SendMessage(hwndDlg, M_GUESSSAMEASBOXES, nFontId, 0);
					SendMessage(hwndDlg, M_REFRESHSAMEASBOXES, nFontId, 0);
					break;
				}
				case IDC_TYPEFACE:
				case IDC_SCRIPT:
				case IDC_FONTSIZE:
				{
					if (CBN_EDITCHANGE != HIWORD(wParam) && CBN_SELCHANGE != HIWORD(wParam))
						return FALSE;
					if (CBN_SELCHANGE == HIWORD(wParam))
						SendDlgItemMessage(hwndDlg, LOWORD(wParam), CB_SETCURSEL, SendDlgItemMessage(hwndDlg, LOWORD(wParam), CB_GETCURSEL, 0, 0), 0);
					if (IDC_TYPEFACE == LOWORD(wParam))
						SendMessage(hwndDlg, M_FILLSCRIPTCOMBO, nFontId, 0);
					// FALL THRU
				}
				case IDC_BOLD:
				case IDC_ITALIC:
				case IDC_UNDERLINE:
				case IDC_COLOUR:
				{
					SendMessage(hwndDlg, M_SAVEFONT, nFontId, 0);
					//SendMessage(hwndDlg, M_GUESSSAMEASBOXES, nFontId, 0);
					//SendMessage(hwndDlg, M_REFRESHSAMEASBOXES, nFontId, 0);
					SendMessage(hwndDlg, M_RECALCOTHERFONTS, nFontId, 0);
					SendMessage(hwndDlg, M_REMAKESAMPLE, 0, 0);
					break;
				}
				case IDC_SAMPLE:
					return 0;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR phdr = (LPNMHDR)(lParam);
			
			if (0 == phdr->idFrom)
			{
				switch (phdr->code) 
				{
					case PSN_APPLY:
					{
						int i;
						char str[20];
						//BOOL bSuccess = FALSE;

						// Border
						DBWriteContactSettingByte(NULL, sModule, "DrawBorder", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAWBORDER));
						{	
							COLORREF col;

							col = SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_GETCOLOUR, 0, 0);
							DBWriteContactSettingDword(NULL, sModule, "LTEdgesColor", col);
							col = SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_GETCOLOUR, 0, 0);
							DBWriteContactSettingDword(NULL, sModule, "RBEdgesColor", col);
						}

						// Backgroud
						{	
							COLORREF col;

							col = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
							DBWriteContactSettingDword(NULL, sModule, "BkColor", col);
						}
						DBWriteContactSettingByte(NULL, sModule, "BkUseBitmap", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
						{	
							char str[MAX_PATH];

							GetDlgItemTextA(hwndDlg, IDC_FILENAME, str, sizeof(str));
							DBWriteContactSettingString(NULL, sModule, "BkBitmap", str);
						}
						{
							WORD flags = 0;

							if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHH))
								flags |= CLB_STRETCHH;
							if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHV))
								flags |= CLB_STRETCHV;
							if (IsDlgButtonChecked(hwndDlg, IDC_TILEH))
								flags |= CLBF_TILEH;
							if (IsDlgButtonChecked(hwndDlg, IDC_TILEV))
								flags |= CLBF_TILEV;
							if (IsDlgButtonChecked(hwndDlg, IDC_PROPORTIONAL))
								flags |= CLBF_PROPORTIONAL;
							DBWriteContactSettingWord(NULL, sModule, "BkBitmapOpt", flags);
						}

						DBWriteContactSettingByte(NULL, sModule, "Opacity"
							, (BYTE)SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0)
							);

						for (i = 0; i < FLT_FONTIDS; i++)
						{
							wsprintfA(str, "Font%dName", i);
							DBWriteContactSettingString(NULL, sModule, str, s_rgFontSettings[i].szFace);
							wsprintfA(str, "Font%dSet", i);
							DBWriteContactSettingByte(NULL, sModule, str, s_rgFontSettings[i].charset);
							wsprintfA(str, "Font%dSize", i);
							DBWriteContactSettingByte(NULL, sModule, str, s_rgFontSettings[i].size);
							wsprintfA(str, "Font%dSty", i);
							DBWriteContactSettingByte(NULL, sModule, str, s_rgFontSettings[i].style);
							wsprintfA(str, "Font%dCol", i);
							DBWriteContactSettingDword(NULL, sModule, str, s_rgFontSettings[i].colour);
							wsprintfA(str, "Font%dAs", i);
							DBWriteContactSettingWord(NULL, sModule, str, MAKEWORD(s_rgFontSettings[i].sameAs, s_rgFontSettings[i].sameAsFlags));
						}

						ApplyOptionsChanges();
						OnStatusChanged();
						return TRUE;
					}
					case PSN_RESET:
					//case PSN_KILLACTIVE:
					{
						fcOpt.thumbAlpha = (BYTE)((double)DBGetContactSettingByte(NULL, sModule, "Opacity", 100) * 2.55);
						SetThumbsOpacity(fcOpt.thumbAlpha);
						break;
					}
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			if (hFontSample)
			{
				SendDlgItemMessage(hwndDlg, IDC_SAMPLE, WM_SETFONT, SendDlgItemMessage(hwndDlg, IDC_FONTID, WM_GETFONT, 0, 0), 0);
				DeleteObject(hFontSample);
			}
			break;
		}

	}
	return FALSE;
}

