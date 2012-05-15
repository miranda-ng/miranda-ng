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

****************************************************************************

File contains realization of service procedures 
for modern_ext_frames.c module.

This file have to be excluded from compilation and need to be adde to project via
#include preprocessor derective in modern_ext_frames.c

\**************************************************************************/

#include "..\commonheaders.h"  //only for precompiled headers

#ifdef __modern_ext_frames_c__include_c_file   //protection from adding to compilation

static void _ExtFrames_InitServices()
{
	//Create services here
	/*
	CreateServiceFunction( MS_CLIST_FRAMES_ADDFRAME,		_ExtFramesSrv_AddFrame		  );
	CreateServiceFunction( MS_CLIST_FRAMES_REMOVEFRAME,		_ExtFramesSrv_RemoveFrame	  );
	CreateServiceFunction( MS_CLIST_FRAMES_SHOWALLFRAMES,	_ExtFramesSrv_ShowAllFrames	  );
	CreateServiceFunction( MS_CLIST_FRAMES_SHOWALLFRAMESTB,	_ExtFramesSrv_ShowAllFramesTB );
	CreateServiceFunction( MS_CLIST_FRAMES_HIDEALLFRAMESTB,	_ExtFramesSrv_HideAllFramesTB );
	CreateServiceFunction( MS_CLIST_FRAMES_SHFRAME,			_ExtFramesSrv_SHFrame		  );
	CreateServiceFunction( MS_CLIST_FRAMES_SHFRAMETITLEBAR,	_ExtFramesSrv_SHFrameTitleBar );
	CreateServiceFunction( MS_CLIST_FRAMES_ULFRAME,			_ExtFramesSrv_ULFrame		  );
	CreateServiceFunction( MS_CLIST_FRAMES_UCOLLFRAME,		_ExtFramesSrv_UCollFrame	  );
	CreateServiceFunction( MS_CLIST_FRAMES_SETUNBORDER,		_ExtFramesSrv_SetUnBorder	  );
	CreateServiceFunction( MS_CLIST_FRAMES_UPDATEFRAME,		_ExtFramesSrv_UpdateFrame	  );
	CreateServiceFunction( MS_CLIST_FRAMES_GETFRAMEOPTIONS,	_ExtFramesSrv_GetFrameOptions );
	CreateServiceFunction( MS_CLIST_FRAMES_SETFRAMEOPTIONS,	_ExtFramesSrv_SetFrameOptions );
	
	//TODO Frame menu stuff services

	//register event hook here
	ExtFrames.hookSBShowTooltip = CreateHookableEvent( ME_CLIST_FRAMES_SB_SHOW_TOOLTIP );
	ExtFrames.hookSBHideTooltip = CreateHookableEvent( ME_CLIST_FRAMES_SB_HIDE_TOOLTIP );
	ExtFrames.hookPrebuildFrameMenu = CreateHookableEvent( ME_CLIST_PREBUILDFRAMEMENU );
	*/

	//Hook other events
	HookEvent(ME_OPT_INITIALISE,_ExtFrames_OptionsDlgInit);
}

static void _ExtFrames_UninitServices()
{
	if (ExtFrames.hookSBShowTooltip) UnhookEvent( ExtFrames.hookSBShowTooltip );
	if (ExtFrames.hookSBHideTooltip) UnhookEvent( ExtFrames.hookSBHideTooltip );
	if (ExtFrames.hookPrebuildFrameMenu) UnhookEvent( ExtFrames.hookPrebuildFrameMenu );

	ExtFrames.hookSBShowTooltip = NULL;
	ExtFrames.hookSBHideTooltip = NULL;
	ExtFrames.hookPrebuildFrameMenu = NULL;
}

//////////////////////////////////////////////////////////////////////////
//want show tooltip for statusbar
//wparam=(char *)protocolname
//lparam=0
//#define ME_CLIST_FRAMES_SB_SHOW_TOOLTIP							"CListFrames/StatusBarShowToolTip"

//////////////////////////////////////////////////////////////////////////
//want hide tooltip for statusbar
//wparam=lparam=0
//#define ME_CLIST_FRAMES_SB_HIDE_TOOLTIP							"CListFrames/StatusBarHideToolTip"

//////////////////////////////////////////////////////////////////////////
//adds a frame window
//wParam=(CLISTFrame*)
//lParam=0
//returns an integer, the frame id.
//#define MS_CLIST_FRAMES_ADDFRAME			"CListFrames/AddFrame"
/**static*/  int _ExtFramesSrv_AddFrame(WPARAM wParam, LPARAM lParam)
{
	CLISTFrame *clfrm=(CLISTFrame *)wParam;
	int frameId = -1;
	
	// the clfrm->name is used as id in DB and frames without it will not be supported
	if ( !clfrm || pcli->hwndContactList == NULL || clfrm->cbSize!=sizeof(CLISTFrame) || clfrm->name==NULL) 
		return frameId;

	efcheck frameId;
	eflock;
	{
	  	EXTFRAMEWND * pExtFrm=(EXTFRAMEWND *)mir_alloc(sizeof(EXTFRAMEWND));
		memset(pExtFrm,0,sizeof(EXTFRAMEWND));
		pExtFrm->dwFrameID = ExtFrames.dwNextFrameID++;
		pExtFrm->hwndFrame = clfrm->hWnd;
		pExtFrm->efrm.szFrameNameID = mir_strdup(clfrm->name);

		//fill frame info by caller provided values
		pExtFrm->efrm.dwFlags = clfrm->Flags;
		// frame alignment
		pExtFrm->efrm.nEdge = clfrm->align;
		pExtFrm->efrm.nType = (clfrm->align&alVertFrameMask) ? EFT_VERTICAL : EFT_HORIZONTAL;

		if (pExtFrm->efrm.nType == EFT_VERTICAL) 
			pExtFrm->efrm.minCX = clfrm->minSize;
		else
			pExtFrm->efrm.minCY = clfrm->minSize;		

		_ExtFrames_GetFrameDBOption(pExtFrm);
		_ExtFramesUtils_CheckAlighment(pExtFrm);
		li.List_Insert(ExtFrames.List, pExtFrm, 0);
		//clfrm->hIcon
		//clfrm->TBname

		frameId=pExtFrm->dwFrameID;		
	}
	efunlock;
	return frameId; //frame id
}
//////////////////////////////////////////////////////////////////////////
// remove frame. It does not destroy your window
//
//#define MS_CLIST_FRAMES_REMOVEFRAME			"CListFrames/RemoveFrame"
static int _ExtFramesSrv_RemoveFrame(WPARAM wParam, LPARAM lParam)
{
	efcheck 0;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0; 
}
//////////////////////////////////////////////////////////////////////////
//shows all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_SHOWALLFRAMES		"CListFrames/ShowALLFrames"
static int _ExtFramesSrv_ShowAllFrames(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//shows the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_SHOWALLFRAMESTB		"CListFrames/ShowALLFramesTB"
static int _ExtFramesSrv_ShowAllFramesTB(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//hides the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_HIDEALLFRAMESTB		"CListFrames/HideALLFramesTB"
static int _ExtFramesSrv_HideAllFramesTB(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//shows the frame if it is hidden,
//hides the frame if it is shown
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_SHFRAME				"CListFrames/SHFrame"
static int _ExtFramesSrv_SHFrame(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//shows the frame titlebar if it is hidden,
//hides the frame titlebar if it is shown
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_SHFRAMETITLEBAR		"CListFrame/SHFrameTitleBar"
static int _ExtFramesSrv_SHFrameTitleBar(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//locks the frame if it is unlocked,
//unlock the frame if it is locked
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_ULFRAME				"CListFrame/ULFrame"
static int _ExtFramesSrv_ULFrame(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//collapses the frame if it is uncollapsed,
//uncollapses the frame if it is collapsed
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_UCOLLFRAME			"CListFrame/UCOLLFrame"
static int _ExtFramesSrv_UCollFrame(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//trigger border flags
//wparam=frameid
//lparam=0
//#define MS_CLIST_FRAMES_SETUNBORDER			"CListFrame/SetUnBorder"
static int _ExtFramesSrv_SetUnBorder(WPARAM wParam, LPARAM lParam)
{
	efcheck -1;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//redraws the frame
//wParam=FrameId, -1 for all frames
//lparam=FU_flags
//returns a pointer to option, -1 on failure
//#define MS_CLIST_FRAMES_UPDATEFRAME			"CListFrame/UpdateFrame"
static int _ExtFramesSrv_UpdateFrame(WPARAM wParam, LPARAM lParam)
{
	int ret=-1;
	efcheck ret;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return ret;
}
//////////////////////////////////////////////////////////////////////////
//gets the frame options
//(HIWORD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=0
//returns a pointer to option, -1 on failure
//#define MS_CLIST_FRAMES_GETFRAMEOPTIONS			"CListFrame/GetFrameOptions"
static int _ExtFramesSrv_GetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	int ret=-1;
	efcheck ret;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return ret;
}
//sets the frame options
//(HIWORLD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=value
//returns 0 on success, -1 on failure
//#define MS_CLIST_FRAMES_SETFRAMEOPTIONS			"CListFrame/SetFrameOptions"
static int _ExtFramesSrv_SetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	int ret=-1;
	efcheck ret;
	eflock;
	{
		// DO HERE
	}
	efunlock;
	return ret;
}
//////////////////////////////////////////////////////////////////////////
//Frames related menu stuff
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//add a new item to the context frame menu
//wParam=0
//lParam=(LPARAM)(CLISTMENUITEM*)&mi
//returns a handle to the new item
//popupposition=frameid
//contactowner=advanced parameter
//#define MS_CLIST_ADDCONTEXTFRAMEMENUITEM			"CList/AddContextFrameMenuItem"

//////////////////////////////////////////////////////////////////////////
//remove a item from context frame menu
//wParam=hMenuItem returned by MS_CLIST_ADDCONTACTMENUITEM
//lParam=0
//returns 0 on success, nonzero on failure
//#define MS_CLIST_REMOVECONTEXTFRAMEMENUITEM			"CList/RemoveContextFrameMenuItem"

//////////////////////////////////////////////////////////////////////////
//builds the context menu for a frame
//wparam=frameid
//lParam=0
//returns a HMENU on success, or NULL on failure
//#define MS_CLIST_MENUBUILDFRAMECONTEXT				"CList/BuildContextFrameMenu"

//////////////////////////////////////////////////////////////////////////
//	the frame menu is about to be built
//		wparam=frameid 
//		lparam=
//	-1 for build from titlebar,
//		use
//			MS_CLIST_ADDCONTEXTFRAMEMENUITEM 
//			MS_CLIST_REMOVECONTEXTFRAMEMENUITEM
//
//	>0 for build in main menu, 
//		must be popupname=lparam to place your items in right popup of main menu.
//		use
//			MS_CLIST_ADDMAINMENUITEM
//			MS_CLIST_REMOVEMAINMENUITEM
//
//#define ME_CLIST_PREBUILDFRAMEMENU					"CList/PreBuildFrameMenu"

//////////////////////////////////////////////////////////////////////////
//needed by cluiframes module to add frames menu to main menu.
//it just calls NotifyEventHooks(hPreBuildFrameMenuEvent,wParam,lParam);
//#define MS_CLIST_FRAMEMENUNOTIFY					"CList/ContextFrameMenuNotify"
#endif