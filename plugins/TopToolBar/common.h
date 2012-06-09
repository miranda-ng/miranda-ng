#ifndef TTB_COMMON_H
#define TTB_COMMON_H


#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <math.h>
#include <vsstyle.h>

#include "win2k.h"
#include "newpluginapi.h"
#include "m_system.h"
#include "m_system_cpp.h"
#include "m_database.h"
#include "m_clist.h"
#include "m_skin.h"
#include "m_utils.h"
#include "m_cluiframes.h"
#include "m_clui.h"
#include "m_clc.h"
#include "m_langpack.h"
#include "m_options.h"
#include "resource.h"
#include "m_protocols.h"
#include "m_protosvc.h"
#include "m_toptoolbar.h"
#include "m_button.h"
#include "m_icolib.h"
#include "BkgrCfg.h"

#define TTB_OPTDIR "TopToolBar"
#define TTBDEFAULT_BKBMPUSE      CLB_STRETCH


#define TTBDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define TTBDEFAULT_USEBITMAP     0
#define TTBDEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)

int TTBOptInit(WPARAM wParam, LPARAM lParam);
//append string
char __inline *AS(char *str, const char *setting, char *addstr);

int ttbOptionsChanged();

int ArrangeButtons();
int RecreateWindows();

struct TopButtonInt 
{
	HWND hwnd;
	HWND hwndTip;
	int id;
	BOOL bPushed;
	int dwFlags;
	int x, y;
	HBITMAP hbBitmapUp, hbBitmapDown;
	HBITMAP hbDefBitmapUp, hbDefBitmapDown;
	HBITMAP hbWBordBitmapUp, hbWBordBitmapDown;
	HICON hIconUp, hIconDn;
	
	char *pszServiceUp, *pszServiceDown;
	char *name;
	TCHAR *tooltip;
	
	LPARAM lParamUp;
	WPARAM wParamUp;
	LPARAM lParamDown;
	WPARAM wParamDown;
	WORD arrangedpos;
	TCHAR *UserDefinedbmUp;
	TCHAR *UserDefinedbmDown;
};

struct SortData
{
	int oldpos;
	int arrangeval;
};

#define MAX_BUTTONS		64
//#define BUTTWIDTH		20

//#define BUTTHEIGHT		16
#define SEPWIDTH		3

struct ButtonOptData
{
	char *name;
	int pos;
	bool show;
};

extern bool OptionsOpened, UseIcoLib, StopArrange;
extern HWND OptionshWnd;
extern HANDLE hHookTTBModuleLoaded;
extern HINSTANCE hInst;
extern LIST<void> arHooks, arServices;

int OptionsPageRebuild();
void lockbut();
void ulockbut();

//return - 0 success, -1 on error
#define TTB_ADDSEPARATOR "TTB/AddSeparator"
//wparam sepid
#define TTB_REMOVESEPARATOR "TTB/RemoveSeparator"


//append string
char *AS(char *str, const char *setting, char *addstr)
{
	if (str != NULL) {
		strcpy(str, setting);
		strcat(str, addstr);
	}
	return str;
}

#define TTB_LAUNCHSERVICE "TTB/LaunchSerice"
//wparam -id
//lparam  &LBTOPT
#define TTB_MODIFYLBUTTON "TTB/ModifyLButton"

//wparam -id
//lparam  &LBTOPT
#define TTB_GETLBUTTON "TTB/GetLButton"

#define TTB_ADDLBUTTON "TTB/AddLButton"
#define TTB_REMOVELBUTTON "TTB/RemoveLButton"
#define MYMIRANDABUTTONCLASS _T("MyMIRANDABUTTONCLASS")

int LoadInternalButtons( HWND );
int UnLoadInternalButtons( void );

int LoadButtonModule( void );
int UnloadButtonModule( void );

int LoadToolbarModule( void );
int UnloadToolbarModule( void );

int LoadAllSeparators( void );

int SetAllBitmaps( void );
int SaveAllLButs( void );
int SaveAllButtonsOptions( void );
int SaveAllSeparators( void );

int InitLBut();
int UnInitLBut();

void lockbut();
void ulockbut();
int applyuserbitmaps( int );

INT_PTR TTBAddButton(WPARAM, LPARAM);
INT_PTR TTBRemoveButton(WPARAM, LPARAM);
INT_PTR DeleteLBut(WPARAM, LPARAM);
INT_PTR ModifyLButton(WPARAM, LPARAM);
INT_PTR GetLButton(WPARAM, LPARAM);
INT_PTR InsertNewFreeSeparator(WPARAM, LPARAM);
INT_PTR DeleteSeparator(WPARAM, LPARAM);

int OnModulesLoad(WPARAM, LPARAM);

INT_PTR InsertLBut(int id);

struct LBUTOPT
{
	int hframe;
	TCHAR *lpath;
	char *name;
};

#define BM_SETPRIVATEICON (WM_USER + 6)
#define BM_SETIMLICON (WM_USER + 7)
#define UseIcoLibDefaultValue 0
#define UseMirandaButtonClassDefaultValue 1

#endif