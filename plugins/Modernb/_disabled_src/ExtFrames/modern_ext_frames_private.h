/**************************************************************************\

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
all portions of this code base are copyrighted to Artem Shpynov and/or
the people listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

****************************************************************************

Created: Mar 19, 2007

Author and Copyright:  Artem Shpynov aka FYR:  ashpynov@gmail.com

\**************************************************************************/

#define eflock	  EnterCriticalSection(&ExtFrames.CS)
#define efunlock  LeaveCriticalSection(&ExtFrames.CS)
#define efcheck if (!ExtFrames.bModuleActive) return

#define EFT_VERTICAL	1
#define EFT_HORIZONTAL	0

#define EFP_LEFT	alLeft
#define EFP_TOP		alTop
#define EFP_RIGHT	alRight
#define EFP_BOTTOM	alBottom

#define EXTFRAMEMODULE "ExtFrames"
#define EXTFRAMEORDERDBPREFIX "FrameOrder/"

typedef struct tagExtFrameRectDef
{
	DWORD dwOrder;
	/* Used in both for frames and for options */
	int minCX;
	int minCY;
	BYTE nType;
	BYTE nEdge;
	BOOL bInPrevious;
	RECT rcFrameRect;
	char *szFrameNameID;   //to be dealloced in destructor

	/* Used for real frames and faked in options */
	DWORD dwFlags;
	BOOL bFloat;
	BOOL bNotRegistered;   // for options means frame is Not Registered
} EXTFRAME;

typedef struct tagExtFrameWndDef
{
	EXTFRAME efrm;				//have to be first element
	//EXTFRAMEWND* can be directly type casted to EXTFRAME*

	/* Used Only for real frames */
	HWND hwndFrame;
	HWND hwndTitle;
	DWORD dwFrameID;

} EXTFRAMEWND;

typedef struct tagExtFrameModule
{
	CRITICAL_SECTION CS;
	BOOL bModuleActive;
	SortedList * List;
	HANDLE hookSBShowTooltip;	    // ME_CLIST_FRAMES_SB_SHOW_TOOLTIP
	HANDLE hookSBHideTooltip;	    // ME_CLIST_FRAMES_SB_HIDE_TOOLTIP
	HANDLE hookPrebuildFrameMenu;	// ME_CLIST_PREBUILDFRAMEMENU
	DWORD  dwNextFrameID;			// unique number of registered frames
}EXTFRAMESMODULE;

typedef struct tagExtFramesOptDBEnumeration
{
	HWND hwndDlg;
	SortedList * pFrameList;
}EXTFRAMEOPTDBENUMERATION;

//////////////////////////////////////////////////////////////////////////
// Static Declarations

// modern_ext_frames_intern.c
static void _ExtFrames_DestructorOf_EXTFRAMEWND(void * extFrame);
static int	_ExtFrames_CalcFramesRect( IN SortedList* pList, IN int width, IN int height, OUT RECT * pWndRect );
static int	_ExtFrames_GetMinParentSize( IN SortedList* pList, OUT SIZE * size );
static void _ExtFrames_GetFrameDBOption( IN OUT EXTFRAMEWND * pExtFrm );
static void _ExtFramesUtils_CheckAlighment( IN OUT EXTFRAMEWND * extFrame );
static int  _ExtFramesUtils_CopmareFrames(void * first, void * last);


// modern_ext_frames_services.c
static void _ExtFrames_InitServices();
static void _ExtFrames_UninitServices();

// modern_ext_frames_opt.c
static int _ExtFrames_OptionsDlgInit(WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK _ExtFrames_DlgProcFrameOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// 
/**static*/ int _ExtFramesSrv_AddFrame(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_RemoveFrame(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_ShowAllFrames(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_ShowAllFramesTB(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_HideAllFramesTB(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_SHFrame(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_SHFrameTitleBar(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_ULFrame(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_UCollFrame(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_SetUnBorder(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_UpdateFrame(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_GetFrameOptions(WPARAM wParam, LPARAM lParam);
static int _ExtFramesSrv_SetFrameOptions(WPARAM wParam, LPARAM lParam);



//////////////////////////////////////////////////////////////////////////
// Static Local Global Variable
static EXTFRAMESMODULE ExtFrames={0};