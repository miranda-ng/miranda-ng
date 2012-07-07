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
#include "m_findadd.h"
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

#define TTBBF_INTERNAL 0x1000000
#define TTBBF_OPTIONAL 0x2000000

///////////////////////////////////////////////////////////////////////////////
// TopButtonInt class

struct TopButtonInt
{
	__inline void* operator new(size_t size) { return calloc( 1, size ); }
	__inline void operator delete(void* p) { free( p ); }
	~TopButtonInt();

	DWORD CheckFlags(DWORD Flags);
	void  CreateWnd(void);
	void  LoadSettings(void);
	void  SaveSettings(int *SepCnt, int *LaunchCnt);
	void  SetBitmap(void);

	__inline bool isSep() const
	{	return (dwFlags & TTBBF_ISSEPARATOR) != 0;
	}

	HWND hwnd;
	int id;
	BOOL bPushed;
	int dwFlags;
	int x, y, arrangedpos;
	HICON hIconUp, hIconDn;
	HANDLE hIconHandleUp, hIconHandleDn;
	
	char *pszService;
	TCHAR *program;
	char *name;
	TCHAR *tooltip;
	
	LPARAM lParamUp;
	WPARAM wParamUp;
	LPARAM lParamDown;
	WPARAM wParamDown;
};

///////////////////////////////////////////////////////////////////////////////

int TTBOptInit(WPARAM wParam, LPARAM lParam);
//append string
char __inline *AS(char *str, const char *setting, char *addstr);

TopButtonInt* CreateButton(TTBButton* but);

int ttbOptionsChanged();

int ArrangeButtons();

#define DEFBUTTWIDTH	20
#define DEFBUTTHEIGHT	16
#define DEFBUTTGAP		1

#define SEPWIDTH		3

extern LIST<TopButtonInt> Buttons;
extern bool StopArrange;
extern HWND hwndTopToolBar;
extern HANDLE hHookTTBModuleLoaded;
extern HINSTANCE hInst;
extern HBITMAP hBmpBackground, hBmpSeparator;
extern int BUTTWIDTH, BUTTHEIGHT, BUTTGAP;
extern CRITICAL_SECTION csButtonsHook;

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

INT_PTR TTBAddButton(WPARAM, LPARAM);
INT_PTR TTBRemoveButton(WPARAM, LPARAM);

int  LoadInternalButtons( HWND );
int  UnLoadInternalButtons( void );

int  LoadToolbarModule( void );
int  UnloadToolbarModule( void );

void SetAllBitmaps( void );
int  SaveAllLButs( void );
int  SaveAllButtonsOptions( void );

void InsertSeparator( int i );
void DeleteSeparator(int i);
void DeleteLBut(int i);
void InsertLBut(int i);

int OnModulesLoad(WPARAM, LPARAM);

#endif