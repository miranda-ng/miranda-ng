#include "globals.h"

#ifndef MONITOR_DEFAULTTONULL
#define MONITOR_DEFAULTTONULL       0x00000000
#endif


// NotesData DB data params
#define DATATAG_TEXT		1	// %s
#define DATATAG_SCROLLPOS	2	// %u (specifies rich edit controls scroll post as first visible line)
#define DATATAG_BGCOL		3	// %x (custom background color)
#define DATATAG_FGCOL		4	// %x (custom text/fg colors)
#define DATATAG_TITLE		5	// %s (custom note title)
#define DATATAG_FONT		6	// %d:%u:%u:%s (custom font)


#define MAX_TITLE_LEN	63
#define MAX_NOTE_LEN	16384

// delay before saving note changes (ms)
#define NOTE_CHANGE_COMMIT_DELAY 1000


#ifndef WS_EX_NOACTIVATE
#define WS_EX_NOACTIVATE 0x08000000
#endif
#define IDM_REMOVENOTE 40001
#define IDM_HIDENOTE 40002
#define IDM_TOGGLEONTOP 40003
#define IDM_UNDO 40004
#define IDM_COPY 40005
#define IDM_PASTE 40006
#define IDM_CUT 40007
#define IDM_CLEAR 40008
#define WS_EX_LAYERED 0x00080000
#define LWA_ALPHA 0x00000002

#define IDC_LISTREMINDERS 1000
#define IDC_LISTREMINDERS_HEADER 2000
#define IDC_REMINDERDATA 1001
#define IDC_ADDNEWREMINDER 1002
#define IDC_CLOSE 1003
#define WM_RELOAD (WM_USER + 100)

#define NOTIFY_LIST() if (ListNotesVisible) PostMessage(LV,WM_RELOAD,0,0)

#define PENLINK ENLINK *

#define NOTE_WND_CLASS _T("MIM_StickyNote")


#define IDM_COLORPRESET_BG 41000
#define IDM_COLORPRESET_FG 41100


static BOOL ListNotesVisible = FALSE;
static HWND LV;


struct ColorPreset
{
	TCHAR *szName;
	COLORREF color;
};

static struct ColorPreset clrPresets[] =
{
	{LPGENT("Black"), RGB(0,0,0)},
	{LPGENT("Maroon"), RGB(128,0,0)},
	{LPGENT("Green"), RGB(0,128,0)},
	{LPGENT("Olive"), RGB(128,128,0)},
	{LPGENT("Navy"), RGB(0,0,128)},
	{LPGENT("Purple"), RGB(128,0,128)},
	{LPGENT("Teal"), RGB(0,128,128)},
	{LPGENT("Gray"), RGB(128,128,128)},
	{LPGENT("Silver"), RGB(192,192,192)},
	{LPGENT("Red"), RGB(255,0,0)},
	{LPGENT("Orange"), RGB(255,155,0)},
	{LPGENT("Lime"), RGB(0,255,0)},
	{LPGENT("Yellow"), RGB(255,255,0)},
	{LPGENT("Blue"), RGB(0,0,255)},
	{LPGENT("Fuchsia"), RGB(255,0,255)},
	{LPGENT("Aqua"), RGB(0,255,255)},
	{LPGENT("White"), RGB(255,255,255)}
};


TREEELEMENT *g_Stickies = NULL;


INT_PTR CALLBACK StickyNoteWndProc(HWND hdlg,UINT message,
								  WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK DlgProcViewNotes(HWND Dialog,UINT Message,WPARAM wParam,
								  LPARAM lParam);
void JustSaveNotes(void);
INT_PTR PluginMenuCommandAddNew(WPARAM w,LPARAM l);
INT_PTR PluginMenuCommandDeleteAll(WPARAM w,LPARAM l);
void GetTriggerTimeString(const ULARGE_INTEGER *When, char *s, UINT strSize, BOOL bUtc);
void OnListResize(HWND Dialog);
void UpdateGeomFromWnd(HWND Dialog, int *geom, int *colgeom, int nCols);
void FileTimeToTzLocalST(const FILETIME *lpUtc, SYSTEMTIME *tmLocal);


COLORREF GetCaptionColor(COLORREF bodyClr)
{
	const DWORD r = ((bodyClr & 0xff) * 4) / 5;
	const DWORD g = (((bodyClr & 0xff00) * 4) / 5) & 0xff00;
	const DWORD b = (((bodyClr & 0xff0000) * 4) / 5) & 0xff0000;

	return (COLORREF)(r|g|b);
}


static void EnsureUniqueID(STICKYNOTE *TSN)
{
	TREEELEMENT *TTE;

	if (!g_Stickies)
		return;

try_next:

	// check existing notes if id is in use
	TTE = g_Stickies;
	while (TTE)
	{
		if (((STICKYNOTE*)TTE->ptrdata)->ID.QuadPart == TSN->ID.QuadPart)
		{
			// id in use, try new (increases the ID/time stamp by 100 nanosecond steps until an unused time is found,
			// allthough it's very unlikely that there will be duplicated id's it's better to make 100% sure)
			TSN->ID.QuadPart++;
			goto try_next;
		}

		TTE = (TREEELEMENT*)TTE->next;
	}
}


static void InitNoteTitle(STICKYNOTE *TSN)
{
	if (g_NoteTitleDate)
	{
		char TempStr[MAX_PATH];
		SYSTEMTIME tm;
		LCID lc = GetUserDefaultLCID();

		TempStr[0] = 0;

		memset(&tm, 0, sizeof(tm));
		FileTimeToTzLocalST((FILETIME*)&TSN->ID, &tm);

		if ( GetDateFormat(lc, 0, &tm, GetDateFormatStr(), TempStr, MAX_PATH) )
		{
			// append time if requested
			if (g_NoteTitleTime)
			{
				int n = (int)strlen(TempStr);
				TempStr[n++] = ' ';
				TempStr[n] = 0;

				GetTimeFormat(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),0), 0, &tm, GetTimeFormatStr(), TempStr+n, MAX_PATH-n);
			}

			TSN->title = _strdup(TempStr);
		}
	}

	TSN->CustomTitle = FALSE;
}


static void InitStickyNoteLogFont(STICKYNOTEFONT *pCustomFont, LOGFONT *lf)
{
	if (!pCustomFont->size)
	{
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
		lf->lfHeight = 10;
		HDC hdc = GetDC(0);
		lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(0, hdc);
	}
	else
	{
		lf->lfHeight = pCustomFont->size;
	}

	_tcscpy(lf->lfFaceName, pCustomFont->szFace);

	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = pCustomFont->style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (pCustomFont->style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (pCustomFont->style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = (pCustomFont->style & DBFONTF_STRIKEOUT) != 0;
	lf->lfCharSet = pCustomFont->charset;
	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
}

static BOOL CreateStickyNoteFont(STICKYNOTEFONT *pCustomFont, LOGFONT *plf)
{
	LOGFONT lf = {0};

	if (!plf)
	{
		InitStickyNoteLogFont(pCustomFont, &lf);
		plf = &lf;
	}

	if (pCustomFont->hFont)
		DeleteObject(pCustomFont->hFont);

	pCustomFont->hFont = CreateFontIndirect(plf);

	return pCustomFont->hFont != NULL;
}


STICKYNOTE* NewNoteEx(int Ax,int Ay,int Aw,int Ah,char *Data,ULARGE_INTEGER *ID,BOOL Visible,BOOL OnTop,int scrollV,COLORREF bgClr,COLORREF fgClr,char *Title,STICKYNOTEFONT *pCustomFont,BOOL bLoading)
{
	STICKYNOTE* TSN;
	WNDCLASSEX TWC = {0};
	WINDOWPLACEMENT TWP;
	DWORD L1,L2;
	SYSTEMTIME tm;
	char TempStr[MAX_PATH] = {0};
	char *TData;

	const BOOL bIsStartup = Visible & 0x10000;
	Visible &= ~0x10000;

	if (Data) TData = Data;	else TData = NULL;

	if (!GetClassInfoEx(hmiranda, NOTE_WND_CLASS, &TWC))
	{
		TWC.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
		TWC.cbClsExtra = 0;
		TWC.cbWndExtra = 0;
		TWC.hInstance = hmiranda;
		TWC.hIcon = LoadIcon(0,IDI_APPLICATION);
		TWC.hCursor = LoadCursor(0,IDC_ARROW);
		TWC.hbrBackground = 0;
		TWC.lpszMenuName = 0;
		TWC.lpszClassName = NOTE_WND_CLASS;
		TWC.cbSize = sizeof(WNDCLASSEX);
		TWC.lpfnWndProc = (WNDPROC)StickyNoteWndProc;
		if (!RegisterClassEx(&TWC)) return NULL;
	}

	if (!TData || Aw < 0 || Ah < 0)
	{
		TWP.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(GetDesktopWindow(),&TWP);
		Aw = g_NoteWidth; Ah = g_NoteHeight;
		Ax = ((TWP.rcNormalPosition.right - TWP.rcNormalPosition.left) / 2) - (Aw / 2);
		Ay = ((TWP.rcNormalPosition.bottom - TWP.rcNormalPosition.top) / 2) - (Ah / 2);
	}

	TSN = (STICKYNOTE*)malloc(sizeof(STICKYNOTE));

	if (ID)
	{
		TSN->ID = *ID;
	}
	else
	{
		GetSystemTime(&tm);
		SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&TSN->ID);
	}

	EnsureUniqueID(TSN);

	TreeAdd(&g_Stickies,TSN);

	if (!TData)
	{
		TData = _strdup("");
		TSN->data = TData;
	}
	else
		TSN->data = TData;

	// init note title (time-stamp)
	if (Title)
	{
		TSN->title = Title;
		TSN->CustomTitle = TRUE;
	}
	else
	{
		TSN->title = NULL;
		InitNoteTitle(TSN);
	}

	TSN->Visible = Visible;
	TSN->OnTop = OnTop;

	TSN->BgColor = bgClr;
	TSN->FgColor = fgClr;

	TSN->pCustomFont = pCustomFont;

	L1 = WS_EX_TOOLWINDOW;
	if (g_Transparency < 255) L1 |= WS_EX_LAYERED;
	if (OnTop) L1 |= WS_EX_TOPMOST;

	L2 = WS_POPUP | WS_THICKFRAME | WS_CAPTION;

	// NOTE: loaded note positions stem from GetWindowPlacement, which normally have a different coord space than
	//       CreateWindow/SetWindowPos, BUT since we now use WS_EX_TOOLWINDOW they use the same coord space so
	//       we don't have to worry about notes "drifting" between sessions
	TSN->SNHwnd = CreateWindowEx(L1, NOTE_WND_CLASS, _T("StickyNote"), L2, Ax,Ay,Aw,Ah, NULL, 0, hmiranda, TSN);

	if (g_Transparency < 255)
		SetLayeredWindowAttributes(TSN->SNHwnd,0,(BYTE)g_Transparency,LWA_ALPHA);

	// ensure that window is not placed off-screen (if previous session had different monitor count or resolution)
	// NOTE: SetWindowPlacement should do this, but it's extremly flakey
	if (Data)
	{
		if (!MonitorFromWindow(TSN->SNHwnd, MONITOR_DEFAULTTONULL) )
		{
			TWP.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(GetDesktopWindow(), &TWP);

			if (Aw > 500) Aw = 500;
			if (Ay < TWP.rcNormalPosition.left+10 || Ax > TWP.rcNormalPosition.right-120)
				Ax = ((TWP.rcNormalPosition.right - TWP.rcNormalPosition.left) / 2) - (Aw / 2) + (rand() & 0x3f);
			if (Ay < TWP.rcNormalPosition.top+50 || Ay > TWP.rcNormalPosition.bottom-50)
				Ay = ((TWP.rcNormalPosition.bottom - TWP.rcNormalPosition.top) / 4) + (rand() & 0x1f);

			SetWindowPos(TSN->SNHwnd, NULL, Ax, Ay, Aw, Ah, SWP_NOZORDER|SWP_NOACTIVATE);
		}
	}

	if (Visible)
	{
		ShowWindow(TSN->SNHwnd, SW_SHOWNA);

		// when loading notes (only at startup), place all non-top notes at the bottom so they don't cover other windows
		if (Data && !OnTop && bIsStartup)
			SetWindowPos(TSN->SNHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_ASYNCWINDOWPOS);
	}

	if (scrollV)
	{
		SendMessage(TSN->REHwnd, EM_LINESCROLL, 0, scrollV);
	}

	// make sure that any event triggered by init doesn't cause a meaningless save
	KillTimer(TSN->SNHwnd, 1025);

	if (!bLoading)
	{
		NOTIFY_LIST();
	}

	return TSN;
}

STICKYNOTE* NewNote(int Ax,int Ay,int Aw,int Ah,char *Data,ULARGE_INTEGER *ID,BOOL Visible,BOOL OnTop,int scrollV)
{
	return NewNoteEx(Ax,Ay,Aw,Ah,Data,ID,Visible,OnTop,scrollV,0,0,NULL,NULL,FALSE);
}

void LoadNotes(BOOL bIsStartup)
{
	int I;
	int NotesCount;
	WORD Size;
	char *Value = NULL, *TVal = NULL;
	char ValueName[32];

	g_Stickies = NULL;

	NotesCount = db_get_dw(0,MODULENAME,"NotesData",0);

	for (I = 0; I < NotesCount; I++)
	{
		char *DelPos;

		mir_snprintf(ValueName, SIZEOF(ValueName), "NotesData%d", I);

		if (Value)
		{
			FreeSettingBlob(Size, Value);
			Value = NULL;
		}

		Size = 65535; // does not get used

		ReadSettingBlob(0, MODULENAME, ValueName, &Size, (void**)&Value);

		if (!Size || !Value)
			continue; // the setting could not be read from DB -> skip

		if (Value[0] == 'X')
		{
			// new eXtended/fleXible data format

			STICKYNOTE note = {0};
			int i, rect[4];
			int scrollV = 0;
			STICKYNOTEFONT *pCustomFont = NULL;
			DWORD flags;

			DelPos = strchr(Value+1,0x1B);
			if (DelPos)
				*DelPos = 0;

			// id:x:y:w:h:flags

			TVal = strchr(Value+1, ':');
			if (!TVal || (DelPos && TVal > DelPos))
				continue;
			*TVal++ = 0;

			note.ID.QuadPart = _strtoui64(Value+1, NULL, 16);

			for (i=0; i<4; i++)
			{
				char *sep = strchr(TVal, ':');
				if (!sep || (DelPos && sep > DelPos))
					goto skip;
				*sep++ = 0;

				rect[i] = strtol(TVal, NULL, 10);

				TVal = sep;
			}

			flags = strtoul(TVal, NULL, 16);

			if (flags & 1)
				note.Visible = TRUE;
			if (flags & 2)
				note.OnTop = TRUE;

			// optional \033 separated params
			while (DelPos)
			{
				char *sep;
				UINT tag;

				TVal = DelPos + 1;
				// find param end and make sure it's null-terminated (if end of data then it's already null-terminated)
				DelPos = strchr(TVal, 0x1B);
				if (DelPos)
					*DelPos = 0;

				// tag:<data>

				sep = strchr(TVal, ':');
				if (!sep || (DelPos && sep > DelPos))
					goto skip;

				tag = strtoul(TVal, NULL, 10);
				TVal = sep + 1;

				switch (tag)
				{
				case DATATAG_TEXT:
					note.data = _strdup(TVal);
					break;

				case DATATAG_SCROLLPOS:
					scrollV = (int)strtoul(TVal, NULL, 10);
					break;

				case DATATAG_BGCOL:
					note.BgColor = strtoul(TVal, NULL, 16) | 0xff000000;
					break;

				case DATATAG_FGCOL:
					note.FgColor = strtoul(TVal, NULL, 16) | 0xff000000;
					break;

				case DATATAG_TITLE:
					if (strlen(TVal) > MAX_TITLE_LEN)
						TVal[MAX_TITLE_LEN] = 0;
					note.title = _strdup(TVal);
					note.CustomTitle = TRUE;
					break;

				case DATATAG_FONT:
					{
						int fsize;
						UINT fstyle, fcharset;

						char *TVal2 = TVal;
						sep = strchr(TVal2, ':');
						if (!sep || (DelPos && sep > DelPos))
							goto skip;
						*sep++ = 0;
						fsize = strtol(TVal2, NULL, 10);
						TVal2 = sep;

						sep = strchr(TVal2, ':');
						if (!sep || (DelPos && sep > DelPos))
							goto skip;
						*sep++ = 0;
						fstyle = strtoul(TVal2, NULL, 10);
						TVal2 = sep;

						sep = strchr(TVal2, ':');
						if (!sep || (DelPos && sep > DelPos))
							goto skip;
						*sep++ = 0;
						fcharset = strtoul(TVal2, NULL, 10);
						TVal2 = sep;

						if (TVal2 >= DelPos)
							goto skip;

						pCustomFont = (STICKYNOTEFONT*)malloc(sizeof(STICKYNOTEFONT));
						pCustomFont->size = (char)fsize;
						pCustomFont->style = (BYTE)fstyle;
						pCustomFont->charset = (BYTE)fcharset;
						_tcscpy(pCustomFont->szFace, TVal2);
						pCustomFont->hFont = NULL;

						if ( !CreateStickyNoteFont(pCustomFont, NULL) )
						{
							free(pCustomFont);
							pCustomFont = NULL;
						}
					}
					break;
				}
			}

			if (!note.data)
				note.data = _strdup("");

			note.Visible = note.Visible && (!bIsStartup || g_ShowNotesAtStart);
			if (bIsStartup)
				note.Visible |= 0x10000;

			NewNoteEx(rect[0],rect[1],rect[2],rect[3],note.data,&note.ID,note.Visible,note.OnTop,scrollV,note.BgColor,note.FgColor,note.title,pCustomFont,TRUE);
		}
		else
		{
			// old format (for DB backward compatibility)

			int Tx,Ty,Tw,Th,TV,OT;
			BOOL V;
			char *Data,*ID;
			ULARGE_INTEGER newid;

			OT = 1; TV = 1;
			Tx = 100; Ty = 100;
			Tw = 179; Th = 35;
			Data = NULL; ID = NULL;

			if (DelPos = strchr(Value,0x1B))
			{	// get first delimiter
				int	PartLen = DelPos - TVal;

				Data = NULL;
				ID = NULL;
				TVal = Value;
				DelPos[0] = 0x0;
				Tx = strtol(TVal, NULL, 10);

				TVal = DelPos + 1;
				DelPos = strchr(TVal, 0x1B);
				if (!DelPos) continue; // setting is broken, do not crash
				DelPos[0] = 0x0;
				Ty = strtol(TVal, NULL, 10);

				TVal = DelPos + 1;
				DelPos = strchr(TVal, 0x1B);
				if (!DelPos) continue; // setting is broken, do not crash
				DelPos[0] = 0x0;
				Tw = strtol(TVal, NULL, 10);

				TVal = DelPos + 1;
				DelPos = strchr(TVal, 0x1B);
				if (!DelPos) continue; // setting is broken, do not crash
				DelPos[0] = 0x0;
				Th = strtol(TVal, NULL, 10);

				TVal = DelPos + 1;
				DelPos = strchr(TVal, 0x1B);
				if (!DelPos) continue; // setting is broken, do not crash
				DelPos[0] = 0x0;
				TV = strtol(TVal, NULL, 10);

				TVal = DelPos + 1;
				DelPos = strchr(TVal, 0x1B);
				if (!DelPos) continue; // setting is broken, do not crash
				DelPos[0] = 0x0;
				OT = strtol(TVal, NULL, 10);

				TVal = DelPos + 1;
				DelPos = strchr(TVal, 0x1B);
				if (!DelPos) continue; // setting is broken, do not crash
				DelPos[0] = 0x0;
				Data = _strdup(TVal);

				TVal = DelPos + 1;
				ID = TVal;

				V = (BOOL)TV && (!bIsStartup || g_ShowNotesAtStart);

				if (bIsStartup)
					V |= 0x10000;

				// convert old ID format to new
				if ( strchr(ID, '-') )
				{
					// validate format (otherwise create new)
					if (strlen(ID) < 19 || ID[2] != '-' || ID[5] != '-' || ID[10] != ' ' || ID[13] != ':' || ID[16] != ':')
					{
						ID = NULL;
					}
					else
					{
						SYSTEMTIME tm;

						ID[2] = ID[5] = ID[10] = ID[13] = ID[16] = 0;

						memset(&tm, 0, sizeof(tm));
						tm.wDay = (WORD)strtoul(ID, NULL, 10);
						tm.wMonth = (WORD)strtoul(ID+3, NULL, 10);
						tm.wYear = (WORD)strtoul(ID+6, NULL, 10);
						tm.wHour = (WORD)strtoul(ID+11, NULL, 10);
						tm.wMinute = (WORD)strtoul(ID+14, NULL, 10);
						tm.wSecond = (WORD)strtoul(ID+17, NULL, 10);

						SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&newid);
					}
				}
				else
				{
					ID = NULL;
				}

				NewNoteEx(Tx,Ty,Tw,Th,Data,ID?&newid:NULL,V,(BOOL)OT,0,0,0,NULL,NULL,TRUE);
			}
		}
skip:;
	}

	if (Value)
		FreeSettingBlob(Size, Value); // we do not leak on bad setting

	NOTIFY_LIST();
}

void CloseNotesList()
{
	if (ListNotesVisible)
	{
		DestroyWindow(LV);
		ListNotesVisible = FALSE;
	}
}

static void PurgeNotesTree()
{
	STICKYNOTE *pt;

	while (g_Stickies) // empty whole tree
	{
		pt = (STICKYNOTE*)g_Stickies->ptrdata;
		if (pt->SNHwnd) DestroyWindow(pt->SNHwnd);
		SAFE_FREE((void**)&pt->title);
		SAFE_FREE((void**)&pt->data);
		if (pt->pCustomFont)
		{
			DeleteObject(pt->pCustomFont->hFont);
			free(pt->pCustomFont);
		}
		TreeDelete(&g_Stickies,pt);
		SAFE_FREE((void**)&pt);
	}
	g_Stickies = NULL;
}

void SaveNotes(void)
{
	JustSaveNotes();
	PurgeNotesTree();
}

void PurgeNotes(void)
{
	int NotesCount, I;
	char ValueName[16];

	NotesCount = db_get_dw(0,MODULENAME,"NotesData",0);
	for(I = 0; I < NotesCount; I++)
	{
		mir_snprintf(ValueName, SIZEOF(ValueName), "NotesData%d", I);
		db_unset(0,MODULENAME,ValueName);
	}
}

void OnDeleteNote(HWND hdlg, STICKYNOTE *SN)
{
	if (MessageBox(hdlg, TranslateT("Are you sure you want to delete this note?"), TranslateT(SECTIONNAME), MB_OKCANCEL) == IDOK)
	{
		if (SN->SNHwnd)
			DestroyWindow(SN->SNHwnd);
		TreeDelete(&g_Stickies,SN);
		SAFE_FREE((void**)&SN->data);
		if (SN->pCustomFont)
		{
			DeleteObject(SN->pCustomFont->hFont);
			free(SN->pCustomFont);
		}
		SAFE_FREE((void**)&SN);
		JustSaveNotes();
		NOTIFY_LIST();
	}
}

void DeleteNotes(void)
{
	PurgeNotes();
	db_set_dw(0, MODULENAME, "NotesData", 0);
	PurgeNotesTree();
	NOTIFY_LIST();
}

void ShowHideNotes(void)
{
	BOOL Visible;

	if (!g_Stickies)
		return;

	// if some notes are hidden but others visible then first make all visible
	// only toggle vis state if all are hidden or all are visible

	UINT nHideCount  = 0, nVisCount = 0;
	TREEELEMENT *TTE = g_Stickies;
	while (TTE)
	{
		if (((STICKYNOTE*)TTE->ptrdata)->Visible)
			nVisCount++;
		else
			nHideCount++;

		TTE = (TREEELEMENT*)TTE->next;
	}

	if (!nVisCount)
		Visible = TRUE;
	else if (!nHideCount)
		Visible = FALSE;
	else
		Visible = TRUE;

	int bShow = Visible ? SW_SHOWNA : SW_HIDE;

	TTE = g_Stickies;
	while (TTE)
	{
		STICKYNOTE *SN = (STICKYNOTE*)TTE->ptrdata;

		if ((!Visible) != (!SN->Visible))
		{
			ShowWindow(SN->SNHwnd, bShow);
			SN->Visible = Visible;
		}

		TTE = (TREEELEMENT*)TTE->next;
	}

	JustSaveNotes();
}

void BringAllNotesToFront(STICKYNOTE *pActive)
{
	TREEELEMENT *TTE;

	if (!g_Stickies)
		return;

	// NOTE: for some reason there are issues when bringing to top through hotkey while another app (like Explorer)
	//       is active, it refuses to move notes to top like it should with HWND_TOP. as a workaround still doesn't
	//       work 100% of the time, but at least more often, we first move not to top-most then for non-always-on-top
	//       notes we demote them back as a non top-most window

	TTE = g_Stickies;
	while (TTE)
	{
		STICKYNOTE *SN = (STICKYNOTE*)TTE->ptrdata;

		if (SN->Visible && pActive != SN)
		{
			SetWindowPos(SN->SNHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			if (!SN->OnTop)
				SetWindowPos(SN->SNHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		}

		TTE = (TREEELEMENT*)TTE->next;
	}

	if (pActive)
	{
		SetWindowPos(pActive->SNHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		if (!pActive->OnTop)
			SetWindowPos(pActive->SNHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}

static void JustSaveNotesEx(STICKYNOTE *pModified)
{
	// pModified optionally points to the modified note that invoked the JustSaveNotesEx call

	TREEELEMENT *TTE;
	int I, NotesCount = TreeGetCount(g_Stickies);
	int n, l;
	char ValueName[32];
	WINDOWPLACEMENT wp;
	int TX,TY,TW,TH;
	DWORD flags;
	int SzT;
	int scrollV;
	char *tData, *Value;

	const int OldNotesCount = db_get_dw(0, MODULENAME, "NotesData", 0);

	db_set_dw(0, MODULENAME, "NotesData", NotesCount);

	for (TTE = g_Stickies, I = 0; TTE; TTE = (TREEELEMENT*)TTE->next, I++)
	{
		STICKYNOTE *pNote = (STICKYNOTE*)TTE->ptrdata;
		BOOL bDeleteTData = TRUE;
		scrollV = 0;
		tData = NULL;

		// window pos and size
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(pNote->SNHwnd, &wp);
		TX = wp.rcNormalPosition.left;
		TY = wp.rcNormalPosition.top;
		TW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		TH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

		// set flags
		flags = 0;
		if (pNote->Visible) flags |= 1;
		if (pNote->OnTop) flags |= 2;

		// get note text
		SzT = GetWindowTextLength(pNote->REHwnd);
		if (SzT) // TODO: change to support unicode and rtf, use EM_STREAMOUT
		{
			if (SzT > MAX_NOTE_LEN) SzT = MAX_NOTE_LEN; // we want to be far below the 64k limit
			tData = (char*)malloc(SzT + 1);
			if (tData)
				GetWindowText(pNote->REHwnd, tData, SzT + 1);
		}

		if (pNote == pModified)
		{
			// update the data of the modified note
			if (pNote->data)
				free(pNote->data);
			pNote->data = tData ? tData : _strdup("");
			bDeleteTData = FALSE;
		}

		if (!tData)
			// empty note
			SzT = 0;
		else
			// get current scroll position
			scrollV = SendMessage(pNote->REHwnd, EM_GETFIRSTVISIBLELINE, 0, 0);

		//

		Value = (char*)malloc(SzT + 512);
		if (!Value) {
			if (bDeleteTData)
				SAFE_FREE((void**)&tData);
			continue;
		}

		n = 0;

		// data header
		l = sprintf(Value, "X%I64x:%d:%d:%d:%d:%x", pNote->ID.QuadPart, TX, TY, TW, TH, flags); //!!!!!!!!!!!!
		if (l > 0) n += l;

		// scroll pos
		if (scrollV > 0)
		{
			l = sprintf(Value+n, "\033""%u:%u", DATATAG_SCROLLPOS, (UINT)scrollV); //!!!!!!!!!!
			if (l > 0) n += l;
		}

		// custom bg color
		if (pNote->BgColor)
		{
			l = sprintf(Value+n, "\033""%u:%x", DATATAG_BGCOL, (UINT)(pNote->BgColor&0xffffff)); //!!!!!!!!!!!!!
			if (l > 0) n += l;
		}

		// custom fg color
		if (pNote->FgColor)
		{
			l = sprintf(Value+n, "\033""%u:%x", DATATAG_FGCOL, (UINT)(pNote->FgColor&0xffffff)); //!!!!!!!!!!!!!
			if (l > 0) n += l;
		}

		if (pNote->pCustomFont)
		{
			l = sprintf(Value+n, "\033""%u:%d:%u:%u:%s", DATATAG_FONT,
				(int)pNote->pCustomFont->size, (UINT)pNote->pCustomFont->style, (UINT)pNote->pCustomFont->charset,
				pNote->pCustomFont->szFace); //!!!!!!!!!!!!!!!
			if (l > 0) n += l;
		}

		// custom title
		if (pNote->CustomTitle && pNote->title)
		{
			l = sprintf(Value+n, "\033""%u:%s", DATATAG_TITLE, pNote->title); //!!!!!!!!!!!!!
			if (l > 0) n += l;
		}

		// note text (ALWAYS PUT THIS PARAM LAST)
		if (tData)
		{
			l = sprintf(Value+n, "\033""%u:%s", DATATAG_TEXT, tData); //!!!!!!!!!!!!
			if (l > 0) n += l;
		}

		// clamp data size to WORD (including null terminator)
		if (n >= 0xffff)
		{
			// huston, we have a problem, strip some reminder text
			n = 0xfffe;
			Value[0xffff] = 0;
		}

		mir_snprintf(ValueName, SIZEOF(ValueName), "NotesData%d", NotesCount - I - 1); // we do not reverse notes in DB

		db_set_blob(0, MODULENAME, ValueName, Value, n+1);

		SAFE_FREE((void**)&Value);
		if (bDeleteTData)
			SAFE_FREE((void**)&tData);

		// make no save is queued for the note
		if (pNote->SNHwnd)
			KillTimer(pNote->SNHwnd, 1025);
	}

	// delete any left over DB note entries
	for(; I < OldNotesCount; I++)
	{
		mir_snprintf(ValueName, SIZEOF(ValueName), "NotesData%d", I);
		db_unset(0,MODULENAME,ValueName);
	}

	NOTIFY_LIST();
}

__inline void JustSaveNotes(void)
{
	JustSaveNotesEx(NULL);
}


/////////////////////////////////////////////////////////////////////
// Note Window

static int FindMenuItem(HMENU h, LPTSTR lpszName)
{
	UINT i;
	TCHAR s[128];

	int n = GetMenuItemCount(h);

	if (n <= 0)
	{
		return -1;
	}

	// searches for a menu item based on name (used to avoid hardcoding item indices for sub-menus)
	for (i=0; i<(UINT)n; i++)
	{
		if ( GetMenuString(h, i, s, 128, MF_BYPOSITION) )
		{
			if ( !_tcscmp(s, lpszName) )
			{
				return (int)i;
			}
		}
	}

	return -1;
}

static BOOL DoContextMenu(HWND AhWnd,WPARAM wParam,LPARAM lParam)
{
	int n, i;
	STICKYNOTE *SN = (STICKYNOTE*)GetProp(AhWnd, _T("ctrldata"));

	HMENU hMenuLoad, FhMenu, hSub;
	hMenuLoad = LoadMenu(hinstance, _T("MNU_NOTEPOPUP"));
	FhMenu = GetSubMenu(hMenuLoad,0);

	if (SN->OnTop)
		CheckMenuItem(FhMenu, IDM_TOGGLEONTOP, MF_CHECKED|MF_BYCOMMAND);

	EnableMenuItem(FhMenu, ID_CONTEXTMENUNOTEPOPUP_PASTETITLE, MF_BYCOMMAND | (IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED));

	if (!SN->CustomTitle)
		EnableMenuItem(FhMenu, ID_CONTEXTMENUNOTEPOPUP_RESETTITLE, MF_BYCOMMAND | MF_GRAYED);

	// NOTE: names used for FindMenuItem would need to include & chars if such shortcuts are added to the menus

	n = FindMenuItem(FhMenu, _T("Appearance"));
	if (n >= 0 && (hSub = GetSubMenu(FhMenu, n)))
	{
		HMENU hBg = GetSubMenu(hSub, FindMenuItem(hSub, _T("Background Color")));
		HMENU hFg = GetSubMenu(hSub, FindMenuItem(hSub, _T("Text Color")));

		for (i=0; i<SIZEOF(clrPresets); i++)
			InsertMenu(hBg, i, MF_BYPOSITION|MF_OWNERDRAW, IDM_COLORPRESET_BG+i, TranslateTS(clrPresets[i].szName));

		for (i=0; i<SIZEOF(clrPresets); i++)
			InsertMenu(hFg, i, MF_BYPOSITION|MF_OWNERDRAW, IDM_COLORPRESET_FG+i, TranslateTS(clrPresets[i].szName));
	}

    CallService(MS_LANGPACK_TRANSLATEMENU,(DWORD)FhMenu,0);
	TrackPopupMenu(FhMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,LOWORD(lParam),HIWORD(lParam),0,AhWnd,0);
	DestroyMenu(hMenuLoad);

	return TRUE;
}

static void MeasureColorPresetMenuItem(HWND hdlg, LPMEASUREITEMSTRUCT lpMeasureItem, struct ColorPreset *clrPresets)
{
	HDC hdc = GetDC(hdlg);
	LPTSTR lpsz = TranslateTS(clrPresets->szName);
	SIZE sz;
	GetTextExtentPoint32(hdc, lpsz, (int)_tcslen(lpsz), &sz);
	ReleaseDC(hdlg, hdc);

	lpMeasureItem->itemWidth = 50 + sz.cx;
	lpMeasureItem->itemHeight = (sz.cy+2)>18 ? sz.cy+2 : 18;
}

static void PaintColorPresetMenuItem(LPDRAWITEMSTRUCT lpDrawItem, struct ColorPreset *clrPresets)
{
	UINT n = lpDrawItem->itemID - IDM_COLORPRESET_BG;
	RECT rect;
	rect.left = lpDrawItem->rcItem.left + 50;
	rect.top = lpDrawItem->rcItem.top;
	rect.right = lpDrawItem->rcItem.right;
	rect.bottom = lpDrawItem->rcItem.bottom;

	if (lpDrawItem->itemState & ODS_SELECTED) {
		SetDCBrushColor(lpDrawItem->hDC, GetSysColor(COLOR_MENUHILIGHT));
		FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, (HBRUSH)GetStockObject(DC_BRUSH));

		SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else {
		SetDCBrushColor(lpDrawItem->hDC, GetSysColor(COLOR_MENU));
		FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, (HBRUSH)GetStockObject(DC_BRUSH));

		SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_MENUTEXT));
	}

	SetBkMode(lpDrawItem->hDC, TRANSPARENT);
	DrawText(lpDrawItem->hDC, clrPresets->szName, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

	int h = lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top;
	rect.left = lpDrawItem->rcItem.left + 5;
	rect.top = lpDrawItem->rcItem.top + ((h-14)>>1);
	rect.right = rect.left + 40;
	rect.bottom = rect.top + 14;

	FrameRect(lpDrawItem->hDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	rect.left++; rect.top++;
	rect.right--; rect.bottom--;
	SetDCBrushColor(lpDrawItem->hDC, clrPresets->color);
	FillRect(lpDrawItem->hDC, &rect, (HBRUSH)GetStockObject(DC_BRUSH));
}

static BOOL GetClipboardText_Title(char *pOut, int size)
{
	BOOL bResult = FALSE;

	if ( OpenClipboard(NULL) )
	{
		HANDLE hData = GetClipboardData(CF_TEXT);
		LPCSTR buffer;

		if (hData && (buffer = (LPCSTR)GlobalLock(hData)))
		{
			// trim initial white spaces
			while (*buffer && isspace(*buffer))
				buffer++;

			size_t n = strlen(buffer);
			if (n >= size)
				n = size-1;
			memcpy(pOut, buffer, n);
			pOut[n] = 0;

			// end string on line break and convert tabs to spaces
			char *p = pOut;
			while (*p)
			{
				if (*p == '\r' || *p == '\n')
				{
					*p = 0;
					n = strlen(pOut);
					break;
				}
				else if (*p == '\t')
				{
					*p = ' ';
				}
				p++;
			}

			// trim trailing white spaces
			rtrim(pOut);
			if (pOut[0])
				bResult = TRUE;

			GlobalUnlock(hData);
		}

		CloseClipboard();
	}

	return bResult;
}

static void SetNoteTextControl(STICKYNOTE *SN)
{
    CHARFORMAT CF = {0};
	CF.cbSize = sizeof(CHARFORMAT);
	CF.dwMask = CFM_COLOR;
	CF.crTextColor = SN->FgColor ? (SN->FgColor&0xffffff) : BodyFontColor;
	SendMessage(SN->REHwnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&CF);

	if (SN->data) // TODO: use EM_STREAMIN
		SetWindowText(SN->REHwnd, SN->data);
}


static UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		// hide color selector
		ShowWindow(GetDlgItem(hdlg,0x443), SW_HIDE);
		ShowWindow(GetDlgItem(hdlg,0x473), SW_HIDE);
		TranslateDialogDefault(hdlg);
	}

	return 0;
}


INT_PTR CALLBACK StickyNoteWndProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		return TRUE;

	case WM_SIZE:
		{
			RECT SZ;

			GetClientRect(hdlg,&SZ);
			HWND H = GetDlgItem(hdlg,1);
			MoveWindow(H, 0, 0, SZ.right,SZ.bottom, TRUE);

			KillTimer(hdlg, 1025);
			SetTimer(hdlg, 1025, NOTE_CHANGE_COMMIT_DELAY, 0);

			return TRUE;
		}
    case WM_TIMER:
		if (wParam == 1025)
		{
			STICKYNOTE *SN = (STICKYNOTE*)GetProp(hdlg,_T("ctrldata"));

			KillTimer(hdlg, 1025);
			JustSaveNotesEx(SN);
		}
		break;
    case WM_MOVE:
		{
			KillTimer(hdlg, 1025);
			SetTimer(hdlg, 1025, NOTE_CHANGE_COMMIT_DELAY, 0);
			return TRUE;
		}
    case WM_CREATE:
		{
			STICKYNOTE *SN = (STICKYNOTE*)GetProp(hdlg,_T("ctrldata"));

			CREATESTRUCT *CS = (CREATESTRUCT *)lParam;
			HWND H;
			DWORD mystyle;

			SN = (STICKYNOTE*)CS->lpCreateParams;
			SetProp(hdlg,_T("ctrldata"),(HANDLE)SN);
			BringWindowToTop(hdlg);
			mystyle = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN;
			if (g_ShowScrollbar) mystyle |= WS_VSCROLL;
			H = CreateWindow(RICHEDIT_CLASS, 0, mystyle, 0, 0, CS->cx-3-3, CS->cy-3-(3+14), hdlg, (HMENU)1, hmiranda, 0);
			SN->REHwnd = H;
			SendMessage(H, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
			SendMessage(H, EM_LIMITTEXT, MAX_NOTE_LEN, 0);
			SendMessage(H, WM_SETFONT, (WPARAM)(SN->pCustomFont ? SN->pCustomFont->hFont : hBodyFont), 1);
			SendMessage(H, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_LINK);
			SendMessage(H, EM_SETBKGNDCOLOR, 0, SN->BgColor ? (SN->BgColor&0xffffff) : BodyColor);
			SendMessage(H, EM_AUTOURLDETECT, 1, 0);
			SetNoteTextControl(SN);
			return TRUE;
		}
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mm = (MINMAXINFO*)lParam;
			// min width accomodates frame, buttons and some extra space for sanity
			mm->ptMinTrackSize.x = 48+3+3+8 + 40;
			// min height allows collapsing entire client area, only leaving frame and caption
			mm->ptMinTrackSize.y = 3+3+14;
		}
		return 0;
	case WM_ERASEBKGND:
		// no BG needed as edit control takes up entire client area
		return TRUE;
	case WM_NCPAINT:
		// make window borders have the same color as caption
		{
			STICKYNOTE *SN = (STICKYNOTE*)GetProp(hdlg,"ctrldata");

			HBRUSH hBkBrush;
			RECT rect, wr, r;
			//HDC hdc = GetDCEx(hdlg, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
			HDC hdc = GetWindowDC(hdlg);

			GetWindowRect(hdlg, &wr);
			if (wParam && wParam != 1)
			{
				SelectClipRgn(hdc, (HRGN)wParam);
				OffsetClipRgn(hdc, -wr.left, -wr.top);
			}

			rect = wr;
			OffsetRect(&rect, -wr.left, -wr.top);

			hBkBrush = (HBRUSH)GetStockObject(DC_BRUSH);
			SetDCBrushColor(hdc, GetCaptionColor((SN && SN->BgColor) ? SN->BgColor : BodyColor));

			// draw all frame sides separately to avoid filling client area (which flickers)
			{
			// top
			r.left = rect.left; r.right = rect.right;
			r.top = rect.top; r.bottom = r.top + 3+14;
			FillRect(hdc, &r, hBkBrush);
			// bottom
			r.top = rect.bottom - 3; r.bottom = rect.bottom;
			FillRect(hdc, &r, hBkBrush);
			// left
			r.left = rect.left; r.right = r.left + 3;
			r.top = rect.top + 3+14; r.bottom = rect.bottom - 3;
			FillRect(hdc, &r, hBkBrush);
			// right
			r.left = rect.right - 3; r.right = rect.right;
			FillRect(hdc, &r, hBkBrush);
			}

			// paint title bar contents (time stamp and buttons)

			if (SN && SN->title) {
				RECT R;
				SelectObject(hdc,hCaptionFont);
				R.top = 3+1; R.bottom = 3+11; R.left = 3+2; R.right = rect.right-3-1;
				if (g_ShowNoteButtons)
					R.right -= 48;

				SetTextColor(hdc,SN->FgColor ? (SN->FgColor&0xffffff) : CaptionFontColor);
				SetBkMode(hdc, TRANSPARENT);
				DrawText(hdc, SN->title, -1, &R, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
			}

			if (g_ShowNoteButtons) {
				HICON hcIcon;
				if (SN->OnTop)
					hcIcon = Skin_GetIconByHandle(iconList[4].hIcolib);
				else
					hcIcon = Skin_GetIconByHandle(iconList[7].hIcolib);
				DrawIcon(hdc, wr.right - wr.left - 16, 0 + 3, hcIcon);
				Skin_ReleaseIcon(hcIcon);

				hcIcon = Skin_GetIconByHandle(iconList[9].hIcolib);
				DrawIcon(hdc, wr.right - wr.left - 32, 1 + 3, hcIcon);
				Skin_ReleaseIcon(hcIcon);

				hcIcon = Skin_GetIconByHandle(iconList[8].hIcolib);
				DrawIcon(hdc, wr.right - wr.left - 48, 1 + 3, hcIcon);
				Skin_ReleaseIcon(hcIcon);
			}

			if (wParam && wParam != 1)
				SelectClipRgn(hdc, NULL);

			ReleaseDC(hdlg, hdc);
		}
		return TRUE;

	case WM_NCCALCSIZE:
		{
			RECT *pRect = wParam ? &((NCCALCSIZE_PARAMS*)lParam)->rgrc[0] : (RECT*)lParam;
			pRect->bottom -= 3;
			pRect->right -= 3;
			pRect->left += 3;
			pRect->top += 3+14;
			return WVR_REDRAW;
		}
	case WM_NCACTIVATE:
		// update window (so that parts that potentially became visible through activation get redrawn immediately)
		RedrawWindow(hdlg, NULL, NULL, RDW_UPDATENOW);
		return TRUE;
    case WM_NOTIFY:
        if (LOWORD(wParam) == 1)
		{
		    char *Buff;
			PENLINK PEnLnk = (PENLINK)lParam;

			if (PEnLnk->msg == WM_LBUTTONDOWN)
			{
				SendDlgItemMessage(hdlg,1,EM_EXSETSEL,0,(LPARAM)&(PEnLnk->chrg));
				Buff = (char*)malloc(PEnLnk->chrg.cpMax - PEnLnk->chrg.cpMin + 1);
				SendDlgItemMessage(hdlg,1,EM_GETSELTEXT,0,(LPARAM)Buff);
				if ((GetAsyncKeyState(VK_CONTROL) >> 15) != 0)
					ShellExecute(hdlg, _T("open"), _T("iexplore"), Buff, _T("") ,SW_SHOWNORMAL);
				else if (g_lpszAltBrowser && *g_lpszAltBrowser)
					ShellExecute(hdlg,_T("open"), g_lpszAltBrowser, Buff, _T("") ,SW_SHOWNORMAL);
				else
					ShellExecute(hdlg, _T("open"), Buff, _T(""), _T(""), SW_SHOWNORMAL);
				SAFE_FREE((void**)&Buff);
				return TRUE;
			}
			return FALSE;
        }
		break;
	case WM_NCHITTEST:
		{
		int r = DefWindowProc(hdlg,message,wParam,lParam);
		// filter out potential hits on windows default title bar buttons
		switch (r)
		{
		case HTSYSMENU:
		case HTCLOSE:
		case HTMINBUTTON:
		case HTMAXBUTTON:
			return HTCAPTION;
		}
		return r;
		}
	case WM_NCLBUTTONDOWN:
		if (wParam == HTCAPTION && g_ShowNoteButtons)
		{
			long X,Y;
			RECT rect;
			int Tw;

			GetWindowRect(hdlg, &rect);
			Tw = rect.right - rect.left;

			X = LOWORD(lParam) - rect.left;
			Y = HIWORD(lParam) - rect.top;

			if (X > Tw - 16)
			{
				SendMessage(hdlg,WM_COMMAND,IDM_TOGGLEONTOP,0);
				return TRUE;
			}
			else if (X > Tw - 31 && X < Tw - 16)
			{
				SendMessage(hdlg,WM_COMMAND,IDM_REMOVENOTE,0);
				return TRUE;
			}
			else if (X > Tw - 48 && X < Tw - 32)
			{
				SendMessage(hdlg,WM_COMMAND,IDM_HIDENOTE,0);
				return TRUE;
			}
		}
		return DefWindowProc(hdlg,message,wParam,lParam);
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;

			if (lpMeasureItem->CtlType != ODT_MENU)
				break;

			if (lpMeasureItem->itemID >= IDM_COLORPRESET_BG && lpMeasureItem->itemID <= IDM_COLORPRESET_BG+SIZEOF(clrPresets))
			{
				MeasureColorPresetMenuItem(hdlg, lpMeasureItem, clrPresets + (lpMeasureItem->itemID - IDM_COLORPRESET_BG));
				return TRUE;
			}
			else if (lpMeasureItem->itemID >= IDM_COLORPRESET_FG && lpMeasureItem->itemID <= IDM_COLORPRESET_FG+SIZEOF(clrPresets))
			{
				MeasureColorPresetMenuItem(hdlg, lpMeasureItem, clrPresets + (lpMeasureItem->itemID - IDM_COLORPRESET_FG));
				return TRUE;
			}
		}
		break;
	case WM_DRAWITEM:
		if (!wParam)
		{
			LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

			if (lpDrawItem->CtlType != ODT_MENU)
				break;

			if (lpDrawItem->itemID >= IDM_COLORPRESET_BG && lpDrawItem->itemID <= IDM_COLORPRESET_BG+SIZEOF(clrPresets))
			{
				PaintColorPresetMenuItem(lpDrawItem, clrPresets + (lpDrawItem->itemID - IDM_COLORPRESET_BG));
				return TRUE;
			}
			else if (lpDrawItem->itemID >= IDM_COLORPRESET_FG && lpDrawItem->itemID <= IDM_COLORPRESET_FG+SIZEOF(clrPresets))
			{
				PaintColorPresetMenuItem(lpDrawItem, clrPresets + (lpDrawItem->itemID - IDM_COLORPRESET_FG));
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		{
			STICKYNOTE *SN = (STICKYNOTE*)GetProp(hdlg, _T("ctrldata"));

			HWND H;
			UINT id;

			switch ( HIWORD(wParam) )
			{
			case EN_CHANGE:
			case EN_VSCROLL:
			case EN_HSCROLL:
				{
					KillTimer(hdlg,1025);
					SetTimer(hdlg, 1025, NOTE_CHANGE_COMMIT_DELAY, 0);
				}
				break;
			}

			id = (UINT) LOWORD(wParam);

			H = SN->REHwnd;

			if (id >= IDM_COLORPRESET_BG && id <= IDM_COLORPRESET_BG+SIZEOF(clrPresets))
			{
				SN->BgColor = clrPresets[id-IDM_COLORPRESET_BG].color | 0xff000000;
				SendMessage(H, EM_SETBKGNDCOLOR, 0, (LPARAM)(SN->BgColor&0xffffff));
				RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
				JustSaveNotes();
				return FALSE;
			}
			else if (id >= IDM_COLORPRESET_FG && id <= IDM_COLORPRESET_FG+SIZEOF(clrPresets))
			{
				CHARFORMAT CF = {0};
				SN->FgColor = clrPresets[id-IDM_COLORPRESET_FG].color | 0xff000000;
				CF.cbSize = sizeof(CHARFORMAT);
				CF.dwMask = CFM_COLOR;
				CF.crTextColor = SN->FgColor & 0xffffff;
				SendMessage(H, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&CF);
				RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
				JustSaveNotes();
				return FALSE;
			}

			switch (id)
			{
			case ID_CONTEXTMENUNOTEPOPUP_NEWNOTE:
				{
					PluginMenuCommandAddNew(0,0);
				}
				break;
			case ID_APPEARANCE_CUSTOMBG:
				{
					COLORREF custclr[16] = {0};
					CHOOSECOLOR cc = {0};
					COLORREF orgclr = SN->BgColor ? (COLORREF)(SN->BgColor&0xffffff) : (COLORREF)(BodyColor&0xffffff);
					cc.lStructSize = sizeof(cc);
					cc.hwndOwner = SN->SNHwnd;
					cc.rgbResult = orgclr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
					cc.lpCustColors = custclr;

					if (ChooseColor(&cc) && cc.rgbResult != orgclr)
					{
						SN->BgColor = cc.rgbResult | 0xff000000;
						SendMessage(H, EM_SETBKGNDCOLOR, 0, (LPARAM)(SN->BgColor&0xffffff));
						RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
						JustSaveNotes();
					}
				}
				break;
			case ID_APPEARANCE_CUSTOMTEXT:
				{
					COLORREF custclr[16] = {0};
					CHOOSECOLOR cc = {0};
					COLORREF orgclr = SN->FgColor ? (COLORREF)(SN->FgColor&0xffffff) : (COLORREF)(BodyFontColor&0xffffff);
					cc.lStructSize = sizeof(cc);
					cc.hwndOwner = SN->SNHwnd;
					cc.rgbResult = orgclr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
					cc.lpCustColors = custclr;

					if (ChooseColor(&cc) && cc.rgbResult != orgclr)
					{
						CHARFORMAT CF = {0};
						SN->FgColor = cc.rgbResult | 0xff000000;
						CF.cbSize = sizeof(CHARFORMAT);
						CF.dwMask = CFM_COLOR;
						CF.crTextColor = SN->FgColor & 0xffffff;
						SendMessage(H, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&CF);
						RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
						JustSaveNotes();
					}
				}
				break;
			case ID_APPEARANCE_CUSTOMFONT:
				{
					CHOOSEFONT cf = {0};
					LOGFONT lf = {0};

					if (SN->pCustomFont)
						InitStickyNoteLogFont(SN->pCustomFont, &lf);
					else
						LoadNRFont(NR_FONTID_BODY, &lf, NULL);

					cf.lStructSize = sizeof(cf);
					cf.hwndOwner = SN->SNHwnd;
					cf.lpLogFont = &lf;
					cf.Flags = CF_EFFECTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_ENABLEHOOK;
					cf.lpfnHook = CFHookProc;

					if ( ChooseFont(&cf) )
					{
						if (!SN->pCustomFont)
						{
							SN->pCustomFont = (STICKYNOTEFONT*)malloc(sizeof(STICKYNOTEFONT));
							SN->pCustomFont->hFont = NULL;
						}

						SN->pCustomFont->size = (char)lf.lfHeight;
						SN->pCustomFont->style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);
						SN->pCustomFont->charset = lf.lfCharSet;
						_tcscpy(SN->pCustomFont->szFace, lf.lfFaceName);

						if ( !CreateStickyNoteFont(SN->pCustomFont, &lf) )
						{
							// failed
							free(SN->pCustomFont);
							SN->pCustomFont = NULL;
						}

						// clear text first to force a reformatting w.r.t scrollbar
						SetWindowText(H, "");
						SendMessage(H, WM_SETFONT, (WPARAM)(SN->pCustomFont ? SN->pCustomFont->hFont : hBodyFont), FALSE);
						SetNoteTextControl(SN);
						RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
						JustSaveNotes();
					}
				}
				break;
			case ID_BACKGROUNDCOLOR_RESET:
				{
					SN->BgColor = 0;
					SendMessage(H, EM_SETBKGNDCOLOR, 0, (LPARAM)BodyColor);
					RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
					JustSaveNotes();
				}
				break;
			case ID_TEXTCOLOR_RESET:
				{
					CHARFORMAT CF = {0};
					SN->FgColor = 0;
					CF.cbSize = sizeof(CHARFORMAT);
					CF.dwMask = CFM_COLOR;
					CF.crTextColor = BodyFontColor;
					SendMessage(H, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&CF);
					RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
					JustSaveNotes();
				}
				break;
			case ID_FONT_RESET:
				{
					if (SN->pCustomFont)
					{
						DeleteObject(SN->pCustomFont->hFont);
						free(SN->pCustomFont);
						SN->pCustomFont = NULL;

						// clear text first to force a reformatting w.r.t scrollbar
						SetWindowText(H, "");
						SendMessage(H, WM_SETFONT, (WPARAM)hBodyFont, FALSE);
						SetNoteTextControl(SN);
						RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
						JustSaveNotes();
					}
				}
				break;
			case ID_CONTEXTMENUNOTEPOPUP_PASTETITLE:
				{
					char s[MAX_TITLE_LEN+1];
					if ( GetClipboardText_Title(s, sizeof(s)) )
					{
						if (SN->title)
							free(SN->title);
						SN->title = _strdup(s);
						SN->CustomTitle = TRUE;
						RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
						JustSaveNotes();
					}
				}
				break;
			case ID_CONTEXTMENUNOTEPOPUP_RESETTITLE:
				if (SN->CustomTitle)
				{
					if (SN->title)
					{
						free(SN->title);
						SN->title = NULL;
					}
					InitNoteTitle(SN);
					RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
					JustSaveNotes();
				}
				break;
			case IDM_REMOVENOTE:
				OnDeleteNote(hdlg, SN);
				break;
			case IDM_HIDENOTE:
				{
					SN->Visible = FALSE;
					ShowWindow(hdlg,SW_HIDE);
					JustSaveNotes();
				}
				break;
			case IDM_COPY: SendMessage(H,WM_COPY,0,0); break;
			case IDM_PASTE: SendMessage(H,WM_PASTE,0,0); break;
			case IDM_CUT: SendMessage(H,WM_CUT,0,0); break;
			case IDM_CLEAR: SendMessage(H,WM_CLEAR,0,0); break;
			case IDM_UNDO: SendMessage(H,WM_UNDO,0,0); break;
			case IDM_TOGGLEONTOP:
				{
					SN->OnTop = !SN->OnTop;
					SetWindowPos(hdlg, SN->OnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
					RedrawWindow(hdlg, NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
					JustSaveNotes();
				}
				break;
			case ID_CONTEXTMENUNOTEPOPUP_VIEWNOTES:
				ListNotes();
				break;
			case ID_CONTEXTMENUNOTEPOPUP_BRINGALLTOTOP:
				BringAllNotesToFront(SN);
				break;
			}
			return TRUE;
		}
	case WM_NCDESTROY:
		{
			RemoveProp(hdlg, _T("ctrldata"));
		}
		break;
    case WM_CONTEXTMENU:
		if (DoContextMenu(hdlg,wParam,lParam)) return FALSE;

	default:
		return DefWindowProc(hdlg,message,wParam,lParam);
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// Notes List Dialog (uses same dialog template as reminder list)

void ListNotes(void)
{
	if (!ListNotesVisible)
	{
		CreateDialog(hinstance, MAKEINTRESOURCE(IDD_LISTREMINDERS), 0, DlgProcViewNotes);
		ListNotesVisible = TRUE;
	}
	else
	{
		BringWindowToTop(LV);
	}
}

static void EditNote(STICKYNOTE *SN)
{
	if (!SN)
		return;

	if (!SN->Visible)
	{
		SN->Visible = TRUE;
		JustSaveNotes();
	}

	SetWindowPos(SN->SNHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	if (!SN->OnTop)
		SetWindowPos(SN->SNHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	SetFocus(SN->REHwnd);
}

char* GetPreviewString(const char *lpsz)
{
	int l;
	char *p;
	const int MaxLen = 80;
	static char s[80+8];

	if (!lpsz)
		return "";

	// trim leading spaces
	while ( iswspace(*lpsz) )
		lpsz++;

	l = (int)strlen(lpsz);

	if (!l)
		return "";

	if (l <= MaxLen)
	{
		strcpy(s, lpsz);
	}
	else
	{
		memcpy(s, lpsz, MaxLen);
		s[MaxLen] = '.';
		s[MaxLen+1] = '.';
		s[MaxLen+2] = '.';
		s[MaxLen+3] = 0;
	}

	if (!s)
		return NULL;

	// convert line breaks and tabs to spaces

	p = s;

	while (*p)
	{
		if ( iswspace(*p) )
			*p = ' ';
		p++;
	}

	return s;
}

static void InitListView(HWND AHLV)
{
	LV_ITEM lvTIt;
	int I;
    char *S;
	char S1[128];
	STICKYNOTE *pNote;
	TREEELEMENT *TTE;

	char *V = Translate("Visible");
	char *T = Translate("Top");

	ListView_SetHoverTime(AHLV,700);
	ListView_SetExtendedListViewStyle(AHLV,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TRACKSELECT);
	ListView_DeleteAllItems(AHLV);

	I = 0;
	TTE = g_Stickies;
	while (TTE)
	{
		pNote = (STICKYNOTE*)TTE->ptrdata;

		lvTIt.mask = LVIF_TEXT;

		if (!pNote->CustomTitle || !pNote->title)
			GetTriggerTimeString(&pNote->ID, S1, sizeof(S1), TRUE);

		lvTIt.iItem = I;
		lvTIt.iSubItem = 0;
		lvTIt.pszText = (pNote->CustomTitle && pNote->title) ? pNote->title : S1;
		ListView_InsertItem(AHLV,&lvTIt);

		if (pNote->Visible)
		{
			lvTIt.iItem = I;
			lvTIt.iSubItem = 1;
			lvTIt.pszText = V;
			ListView_SetItem(AHLV,&lvTIt);
		}

		if (pNote->OnTop)
		{
			lvTIt.iItem = I;
			lvTIt.iSubItem = 2;
			lvTIt.pszText = T;
			ListView_SetItem(AHLV,&lvTIt);
		}

		S = GetPreviewString(pNote->data);
		lvTIt.iItem = I;
		lvTIt.iSubItem = 3;
		lvTIt.pszText = S;
		ListView_SetItem(AHLV,&lvTIt);

		I++;
		TTE = (TREEELEMENT*)TTE->next;
	}

	ListView_SetItemState(AHLV,0,LVIS_SELECTED,LVIS_SELECTED);
}

static BOOL DoListContextMenu(HWND AhWnd,WPARAM wParam,LPARAM lParam,STICKYNOTE *pNote)
{
	HWND hwndListView;
    HMENU hMenuLoad,FhMenu;
	MENUITEMINFO mii;

	hwndListView = (HWND)wParam;
	if (hwndListView != GetDlgItem(AhWnd,IDC_LISTREMINDERS)) return FALSE;
	hMenuLoad = LoadMenu(hinstance,"MNU_NOTELISTPOPUP");
	FhMenu = GetSubMenu(hMenuLoad,0);

	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_DEFAULT;
	if (!pNote)
		mii.fState |= MFS_GRAYED;
	SetMenuItemInfo(FhMenu, ID_CONTEXTMENUNOTELISTVIEW_EDITNOTE, FALSE, &mii);

	if (!pNote)
	{
		EnableMenuItem(FhMenu, IDM_REMOVENOTE, MF_GRAYED|MF_BYCOMMAND);
		EnableMenuItem(FhMenu, ID_CONTEXTMENUNOTELISTVIEW_TOGGLEVISIBILITY, MF_GRAYED|MF_BYCOMMAND);
		EnableMenuItem(FhMenu, IDM_TOGGLEONTOP, MF_GRAYED|MF_BYCOMMAND);
	}
	else
	{
		if (pNote->Visible)
			CheckMenuItem(FhMenu, ID_CONTEXTMENUNOTELISTVIEW_TOGGLEVISIBILITY, MF_CHECKED|MF_BYCOMMAND);
		if (pNote->OnTop)
			CheckMenuItem(FhMenu, IDM_TOGGLEONTOP, MF_CHECKED|MF_BYCOMMAND);
	}

    CallService(MS_LANGPACK_TRANSLATEMENU,(DWORD)FhMenu,0);
	TrackPopupMenu(FhMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,LOWORD(lParam),HIWORD(lParam),0,AhWnd,0);
	DestroyMenu(hMenuLoad);

	return TRUE;
}


INT_PTR CALLBACK DlgProcViewNotes(HWND Dialog,UINT Message,WPARAM wParam,LPARAM lParam)
{
    LV_COLUMN lvCol;
    NMLISTVIEW *NM;
    TCHAR *S;
    int I;

	switch (Message)
	{
	case WM_SIZE:
		{
			OnListResize(Dialog);
			UpdateGeomFromWnd(Dialog, g_notesListGeom, NULL, 0);
			break;
		}
	case WM_MOVE:
		UpdateGeomFromWnd(Dialog, g_notesListGeom, NULL, 0);
		break;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mm = (MINMAXINFO*)lParam;
			mm->ptMinTrackSize.x = 394;
			mm->ptMinTrackSize.y = 300;
		}
		return 0;
	case WM_RELOAD:
		{
			SetDlgItemText(Dialog,IDC_REMINDERDATA,"");
			InitListView(GetDlgItem(Dialog,IDC_LISTREMINDERS));
			return TRUE;
		}
	case WM_CONTEXTMENU:
		{
			HWND H;
			STICKYNOTE *pNote = NULL;

			H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
			if ( ListView_GetSelectedCount(H) )
			{
				I = ListView_GetSelectionMark(H);
				if (I != -1)
				{
					pNote = (STICKYNOTE*)TreeGetAt(g_Stickies, I);
				}
			}

			if (DoListContextMenu(Dialog, wParam, lParam, pNote))
				return TRUE;
		}
		break;
	case WM_INITDIALOG:
		{
			HWND H;

			HICON hIcon = Skin_GetIconByHandle(iconList[13].hIcolib, ICON_SMALL);
			SendMessage(Dialog, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
			hIcon = Skin_GetIconByHandle(iconList[13].hIcolib, ICON_BIG);
			SendMessage(Dialog, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);

			SetWindowText(Dialog, LPGENT("Notes"));

			TranslateDialogDefault(Dialog);

			SetDlgItemText(Dialog,IDC_REMINDERDATA, _T(""));

			H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH;

			S = TranslateT("Note text");
			lvCol.pszText = S;
			lvCol.cx = g_notesListColGeom[3];
			ListView_InsertColumn(H,0,&lvCol);
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH;

			S = TranslateT("Top");
			lvCol.pszText = S;
			lvCol.cx = g_notesListColGeom[2];
			ListView_InsertColumn(H,0,&lvCol);
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH;

			S = TranslateT("Visible");
			lvCol.pszText = S;
			lvCol.cx = g_notesListColGeom[1];
			ListView_InsertColumn(H,0,&lvCol);
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH;

			S = TranslateT("Date/Title");
			lvCol.pszText = S;
			lvCol.cx = g_notesListColGeom[0];
			ListView_InsertColumn(H,0,&lvCol);

			InitListView(H);
			SetWindowLongPtr(GetDlgItem(H, 0), GWL_ID, IDC_LISTREMINDERS_HEADER);
			LV = Dialog;

			if (g_notesListGeom[1] && g_notesListGeom[2])
			{
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(Dialog, &wp);
				wp.rcNormalPosition.left = g_notesListGeom[0];
				wp.rcNormalPosition.top = g_notesListGeom[1];
				wp.rcNormalPosition.right = g_notesListGeom[2] + g_notesListGeom[0];
				wp.rcNormalPosition.bottom = g_notesListGeom[3] + g_notesListGeom[1];
				SetWindowPlacement(Dialog, &wp);
			}
			return TRUE;
		}
	case WM_CLOSE:
		DestroyWindow(Dialog);
		ListNotesVisible = FALSE;
		return TRUE;

	case WM_DESTROY:
		ListNotesVisible = FALSE;
		Skin_ReleaseIcon((HICON)SendMessage(Dialog, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(Dialog, WM_SETICON, ICON_SMALL, 0));
		return TRUE;

	case WM_NOTIFY:
		{
			if (wParam == IDC_LISTREMINDERS)
			{
				NM = (NMLISTVIEW *)lParam;
				switch (NM->hdr.code)
				{
				case LVN_ITEMCHANGED:
					{
						S = ((STICKYNOTE*)TreeGetAt(g_Stickies,NM->iItem))->data;
					    SetDlgItemText(Dialog,IDC_REMINDERDATA,S);
					}
					break;
				case NM_DBLCLK:
					{
						HWND H;

						H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
						if ( ListView_GetSelectedCount(H) )
						{
							I = ListView_GetSelectionMark(H);
							if (I != -1)
							{
								EditNote((STICKYNOTE *)TreeGetAt(g_Stickies, I));
							}
						}
					}
					break;
				}
			}
			else if (wParam == IDC_LISTREMINDERS_HEADER)
			{
				NMHEADER *NM = (NMHEADER*)lParam;
				switch (NM->hdr.code)
				{
				case HDN_ENDTRACK:
					UpdateGeomFromWnd(Dialog, NULL, g_notesListColGeom, SIZEOF(g_notesListColGeom));
					break;
				}
			}
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_CONTEXTMENUNOTELISTVIEW_EDITNOTE:
				{
					HWND H;

					H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
					if ( ListView_GetSelectedCount(H) )
					{
						I = ListView_GetSelectionMark(H);
						if (I != -1)
						{
							EditNote((STICKYNOTE*)TreeGetAt(g_Stickies, I));
						}
					}
				}
				return TRUE;
			case ID_CONTEXTMENUNOTELISTVIEW_TOGGLEVISIBILITY:
				{
					HWND H;

					H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
					if ( ListView_GetSelectedCount(H) )
					{
						I = ListView_GetSelectionMark(H);
						if (I != -1)
						{
							STICKYNOTE *SN = (STICKYNOTE*)TreeGetAt(g_Stickies, I);
							SN->Visible = !SN->Visible;
							ShowWindow(SN->SNHwnd,SN->Visible?SW_SHOWNA:SW_HIDE);
							JustSaveNotes();
						}
					}
				}
				return TRUE;
			case IDM_TOGGLEONTOP:
				{
					HWND H;

					H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
					if ( ListView_GetSelectedCount(H) )
					{
						I = ListView_GetSelectionMark(H);
						if (I != -1)
						{
							STICKYNOTE *SN = (STICKYNOTE*)TreeGetAt(g_Stickies,I);
							SN->OnTop = !SN->OnTop;
							SetWindowPos(SN->SNHwnd, SN->OnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
							RedrawWindow(SN->SNHwnd, NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
							JustSaveNotes();
						}
					}
				}
				return TRUE;
			case IDC_CLOSE:
				{
					DestroyWindow(Dialog);
					ListNotesVisible = FALSE;
					return TRUE;
				}
			case ID_CONTEXTMENUNOTEPOPUP_NEWNOTE:
			case IDC_ADDNEWREMINDER:
				{
					PluginMenuCommandAddNew(0,0);
					return TRUE;
				}
			case ID_CONTEXTMENUNOTELISTVIEW_DELETEALLNOTES:
				{
					PluginMenuCommandDeleteAll(0,0);
					return TRUE;
				}
			case IDM_REMOVENOTE:
				{
					HWND H;

					H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
					if ( ListView_GetSelectedCount(H) )
					{
						I = ListView_GetSelectionMark(H);
						if (I != -1)
						{
							OnDeleteNote(Dialog, (STICKYNOTE*)TreeGetAt(g_Stickies, I));
						}
					}
				}
				return TRUE;
			case ID_CONTEXTMENUNOTELISTVIEW_SHOW:
				{
					ShowHideNotes();
					return TRUE;
				}
			case ID_CONTEXTMENUNOTEPOPUP_BRINGALLTOTOP:
				{
					BringAllNotesToFront(NULL);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
