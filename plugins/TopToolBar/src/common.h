#ifndef TTB_COMMON_H
#define TTB_COMMON_H


#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <stddef.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_skin.h>
#include <m_cluiframes.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_findadd.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_button.h>
#include <m_icolib.h>

#include <m_toptoolbar.h>

#include "resource.h"
#include "version.h"
#include "BkgrCfg.h"

#define TTB_BUTTON_CLASS  _T("TopToolbarButtonClass")

#define TTB_REPOSBUTTONS          (WM_USER+10)
#define TTB_UPDATEFRAMEVISIBILITY (WM_USER+11)

#define TTBDEFAULT_BKBMPUSE      CLB_STRETCH
#define TTBDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define TTBDEFAULT_USEBITMAP     0
#define TTBDEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)

#define TTBBF_INTERNAL 0x1000000
#define TTBBF_OPTIONAL 0x2000000

///////////////////////////////////////////////////////////////////////////////
// TopButtonInt class

struct TopButtonInt : public MZeroedObject
{
	~TopButtonInt();

	DWORD CheckFlags(DWORD Flags);
	void  CreateWnd(void);
	void  LoadSettings(void);
	void  SaveSettings(int *SepCnt, int *LaunchCnt);
	void  SetBitmap(void);

	__inline bool isSep() const
	{	return (dwFlags & TTBBF_ISSEPARATOR) != 0;
	}

	__inline bool isVisible() const
	{	return (dwFlags & TTBBF_VISIBLE) != 0;
	}

	HWND   hwnd;
	int    id;
	BOOL   bPushed;
	int    dwFlags;
	int    x, y, arrangedpos;
	HICON  hIconUp, hIconDn;
	HANDLE hIconHandleUp, hIconHandleDn;

	char  *pszService;
	TCHAR *ptszProgram;
	char  *pszName;
	TCHAR *ptszTooltip;

	LPARAM lParamUp;
	WPARAM wParamUp;
	LPARAM lParamDown;
	WPARAM wParamDown;

	int    hLangpack;
	TCHAR *ptszTooltipUp, *ptszTooltipDn;
};

///////////////////////////////////////////////////////////////////////////////

int TTBOptInit(WPARAM wParam, LPARAM lParam);
//append string
char __inline *AS(char *str, const char *setting, char *addstr);

TopButtonInt* CreateButton(TTBButton* but);

int LoadBackgroundOptions();

int ArrangeButtons();

#define DEFBUTTWIDTH	20
#define DEFBUTTHEIGHT	20
#define DEFBUTTGAP		1

#define SEPWIDTH		3

extern TTBCtrl* g_ctrl;

extern LIST<TopButtonInt> Buttons;
extern HINSTANCE hInst;
extern HBITMAP hBmpBackground, hBmpSeparator;
extern CRITICAL_SECTION csButtonsHook;
extern pfnCustomProc g_CustomProc;
extern LPARAM g_CustomProcParam;
extern HANDLE hTTBModuleLoaded;

void AddToOptions(TopButtonInt* b);
void RemoveFromOptions(int id);

//append string
char *AS(char *str, const char *setting, char *addstr)
{
	if (str != NULL) {
		strcpy(str, setting);
		strcat(str, addstr);
	}
	return str;
}

#define TTB_LAUNCHSERVICE "TTB/LaunchService"

TopButtonInt* idtopos(int id, int* pPos=NULL);

INT_PTR TTBAddButton(WPARAM, LPARAM);
INT_PTR TTBRemoveButton(WPARAM, LPARAM);

void InitInternalButtons(void);

int  LoadToolbarModule(void);
int  UnloadToolbarModule(void);

void SetAllBitmaps(void);
int  SaveAllLButs(void);
int  SaveAllButtonsOptions(void);

void InsertSeparator( int i );
void DeleteSeparator(int i);
void DeleteLBut(int i);
void InsertLBut(int i);

int OnModulesLoad(WPARAM, LPARAM);

#endif