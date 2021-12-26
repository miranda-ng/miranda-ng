#ifndef TTB_COMMON_H
#define TTB_COMMON_H

#include <windows.h>
#include <commctrl.h>
#include <stddef.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_skin.h>
#include <m_clistint.h>
#include <m_cluiframes.h>
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

#define TTB_BUTTON_CLASS  L"TopToolbarButtonClass"

#define TTB_REPOSBUTTONS          (WM_USER+10)
#define TTB_UPDATEFRAMEVISIBILITY (WM_USER+11)

#define TTBDEFAULT_BKBMPUSE      CLB_STRETCH
#define TTBDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define TTBDEFAULT_USEBITMAP     0
#define TTBDEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)

#define TTBBF_INTERNAL 0x1000000
#define TTBBF_OPTIONAL 0x2000000

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

///////////////////////////////////////////////////////////////////////////////
// TopButtonInt class

struct TopButtonInt : public MZeroedObject
{
	~TopButtonInt();

	uint32_t CheckFlags(uint32_t Flags);
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
	wchar_t *ptszProgram;
	char  *pszName;
	wchar_t *ptszTooltip;

	LPARAM lParamUp;
	WPARAM wParamUp;
	LPARAM lParamDown;
	WPARAM wParamDown;

	HPLUGIN pPlugin;
	wchar_t *ptszTooltipUp, *ptszTooltipDn;
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
extern HBITMAP hBmpBackground;
extern mir_cs csButtonsHook;
extern pfnCustomProc g_CustomProc;
extern LPARAM g_CustomProcParam;
extern HANDLE hTTBModuleLoaded;
extern IconItem iconList[];

void AddToOptions(TopButtonInt* b);
void RemoveFromOptions(int id);

//append string
char *AS(char *str, const char *setting, char *addstr)
{
	if (str != nullptr) {
		mir_strcpy(str, setting);
		mir_strcat(str, addstr);
	}
	return str;
}

#define TTB_LAUNCHSERVICE "TTB/LaunchService"

TopButtonInt* idtopos(int id, int* pPos = nullptr);

INT_PTR TTBAddButton(WPARAM, LPARAM);
INT_PTR TTBRemoveButton(WPARAM, LPARAM);

void InitInternalButtons(void);

int  LoadToolbarModule(void);
int  UnloadToolbarModule(void);

int  SaveAllButtonsOptions(void);

void LoadAllSeparators();void LoadAllLButs();

#endif