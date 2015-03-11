#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <richedit.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>
#include <m_icolib.h>

#include <m_toptoolbar.h>

#include "miscutils.h"
#include "resource.h"
#include "Version.h"

#define MODULENAME	"StickyNotes"
#define SECTIONNAME	LPGEN("Notes & Reminders")

// normal timer interval for reminder update processing
#define REMINDER_UPDATE_INTERVAL		10000
// short timer interval for reminder updates used as long as there are pending alarams in the event queue
#define REMINDER_UPDATE_INTERVAL_SHORT	5000


// font IDs used with LoadNRFont
#define NR_FONTID_CAPTION		0
#define NR_FONTID_BODY			1
#define NR_FONTID_MAX			NR_FONTID_BODY


typedef struct {
	HFONT hFont;
    char  size;
    BYTE  style;					// see the DBFONTF_* flags
    BYTE  charset;
    char  szFace[LF_FACESIZE];
} STICKYNOTEFONT;

typedef struct {
	HWND SNHwnd,REHwnd;
	BOOL Visible,OnTop;
	char *data;
	ULARGE_INTEGER ID;		// FILETIME in UTC
	char *title;
	BOOL CustomTitle;
	DWORD BgColor;			// custom bg color override (only valid if non-zero)
	DWORD FgColor;			// custom fg/text color override (only valid if non-zero)
	STICKYNOTEFONT *pCustomFont;// custom (body) font override (NULL if default font is used)
} STICKYNOTE;

typedef struct {
	HWND handle;
	BOOL RemVisible;
	DWORD uid;
	char *Reminder;
	ULARGE_INTEGER When;	// FILETIME in UTC
	UINT RepeatSound;
	UINT RepeatSoundTTL;
	int SoundSel;			// -1 if sound disabled
	BOOL SystemEventQueued;
} REMINDERDATA;


extern void CreateMsgWindow(void);
extern void DestroyMsgWindow(void);

extern STICKYNOTE* NewNote(int Ax,int Ay,int Aw,int Ah,char *Data,
						   ULARGE_INTEGER *ID,BOOL Visible,BOOL OnTop,int scrollV);
extern void LoadNotes(BOOL bIsStartup);
extern void SaveNotes(void);
extern void DeleteNotes(void);
extern void ShowHideNotes(void);
extern void ListNotes(void);

extern void NewReminder(void);
extern void LoadReminders(void);
extern void SaveReminders(void);
extern void DeleteReminders(void);
extern void ListReminders(void);
extern BOOL CheckRemindersAndStart(void);

extern void InitSettings(void);
extern void TermSettings(void);
extern INT_PTR CALLBACK DlgProcOptions(HWND hdlg,UINT message,
								   WPARAM wParam,LPARAM lParam);
extern void LoadNRFont(int i, LOGFONT *lf, COLORREF *colour);

extern BOOL WS_Init();
extern void WS_CleanUp();

extern LPCSTR GetDateFormatStr();
extern LPCSTR GetTimeFormatStr();

extern HINSTANCE hinstance;
extern HINSTANCE hmiranda;

extern BOOL g_CloseAfterAddReminder, g_UseDefaultPlaySound;
extern HICON g_hReminderIcon;

extern LOGFONT lfBody,lfCaption;
extern HFONT hBodyFont,hCaptionFont;

extern long BodyColor;
extern long CaptionFontColor,BodyFontColor;

extern BOOL g_ShowNotesAtStart,g_ShowScrollbar,g_AddContListMI,g_ShowNoteButtons;
extern int g_NoteTitleDate, g_NoteTitleTime;

extern int g_NoteWidth,g_NoteHeight;

extern int g_Transparency;

extern char *g_RemindSMS;

extern TCHAR *g_lpszAltBrowser;

extern int g_reminderListGeom[4];
extern int g_reminderListColGeom[2];
extern int g_notesListGeom[4];
extern int g_notesListColGeom[4];

extern HWND HKHwnd;
extern IconItem iconList[];

// these defs are only used to emphasize that SYSTEMTIMEtoFILETIME/FILETIMEtoSYSTEMTIME only convert the data type,
// it does not apply any time conversion/correction like UTC to local etc. (if input is local, then output is local too)
#define SYSTEMTIMEtoFILETIME SystemTimeToFileTime
#define FILETIMEtoSYSTEMTIME FileTimeToSystemTime
