#pragma once

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <limits.h>
#include <time.h>
#include <richedit.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_clistint.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_gui.h>

#include <m_toptoolbar.h>

#include "miscutils.h"
#include "resource.h"
#include "version.h"

#define MODULENAME	"StickyNotes"
#define SECTIONNAME	LPGEN("Notes & Reminders")

#define MS_NOTES_NEW       MODULENAME"/MenuCommandAddNew"
#define MS_NOTES_SHOWHIDE  MODULENAME"/MenuCommandShowHide"
#define MS_NOTES_DISPLAY   MODULENAME"/MenuCommandBringAllFront"

#define MS_REMINDER_NEW    MODULENAME"/MenuCommandNewReminder"
#define MS_REMINDER_VIEW   MODULENAME"/MenuCommandViewReminders"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	CMOption<uint8_t> bShowNotesAtStart, bShowScrollbar, bShowNoteButtons;
	CMOption<uint8_t> bCloseAfterAddReminder, bUseMSI;

	int Load() override;
	int Unload() override;
};

// font IDs used with LoadNRFont
#define NR_FONTID_CAPTION		0
#define NR_FONTID_BODY			1
#define NR_FONTID_MAX			NR_FONTID_BODY

// normal timer interval for reminder update processing
#define REMINDER_UPDATE_INTERVAL		10000

// short timer interval for reminder updates used as long as there are pending alarams in the event queue
#define REMINDER_UPDATE_INTERVAL_SHORT	5000

extern void CreateMsgWindow(void);
extern void DestroyMsgWindow(void);

void NewNote(int Ax, int Ay, int Aw, int Ah, const wchar_t *pwszText, ULONGLONG *ID, BOOL Visible, BOOL bOnTop, int scrollV);
void LoadNotes(bool bIsStartup);
void SaveNotes(void);
void DeleteNotes(void);
void ShowHideNotes(void);

void LoadReminders(void);
void SaveReminders(void);
void DeleteReminders(void);
bool CheckRemindersAndStart(void);

void InitSettings(void);
void TermSettings(void);
void LoadNRFont(int i, LOGFONT *lf, COLORREF *colour);

wchar_t* GetDateFormatStr();
wchar_t* GetTimeFormatStr();

extern HINSTANCE hmiranda;

extern HICON g_hReminderIcon;

extern LOGFONT lfBody, lfCaption;
extern HFONT hBodyFont, hCaptionFont;

extern long BodyColor;
extern COLORREF CaptionFontColor, BodyFontColor;

extern int g_NoteTitleDate, g_NoteTitleTime;
extern int g_NoteWidth, g_NoteHeight;
extern int g_Transparency;

extern char *g_RemindSMS;
extern char *g_lpszAltBrowser;

extern IconItem iconList[];

INT_PTR PluginMenuCommandAddNew(WPARAM, LPARAM);
INT_PTR PluginMenuCommandShowHide(WPARAM, LPARAM);
INT_PTR PluginMenuCommandViewNotes(WPARAM, LPARAM);
INT_PTR PluginMenuCommandAllBringFront(WPARAM, LPARAM);
INT_PTR PluginMenuCommandDeleteNotes(WPARAM, LPARAM);

INT_PTR PluginMenuCommandNewReminder(WPARAM, LPARAM);
INT_PTR PluginMenuCommandViewReminders(WPARAM, LPARAM);
INT_PTR PluginMenuCommandDeleteReminders(WPARAM, LPARAM);

int OnOptInitialise(WPARAM, LPARAM);
