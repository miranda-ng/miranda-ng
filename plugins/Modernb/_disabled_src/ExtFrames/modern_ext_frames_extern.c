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

File contains realization of externaly available procedures 
for modern_ext_frames.c module.

This file have to be excluded from compilation and need to be adde to project via
#include preprocessor derective in modern_ext_frames.c

\**************************************************************************/

#include "..\commonheaders.h"  //only for precompiled headers

#ifdef __modern_ext_frames_c__include_c_file   //protection from adding to compilation
int ExtFrames_Init()
{
	if (ExtFrames.bModuleActive) return 0;
	InitializeCriticalSection(&ExtFrames.CS);
	ExtFrames.List=li.List_Create(0,1);	
	ExtFrames.List->sortFunc=_ExtFramesUtils_CopmareFrames;
	_ExtFrames_InitServices();
	ExtFrames.bModuleActive = TRUE;
	return 1;		
}

int ExtFrames_Uninit()
{
	efcheck 0;
	eflock;
	{	
		ExtFrames.bModuleActive = FALSE;
		_ExtFrames_UninitServices();
		li_ListDestruct(ExtFrames.List, _ExtFrames_DestructorOf_EXTFRAMEWND);
		ExtFrames.bModuleActive = FALSE;
	}	
	efunlock;
	DeleteCriticalSection(&ExtFrames.CS);
	return 1;
}
int ExtFrames_GetMinWindowSize( OUT SIZE * size )
{
	int minCX=0;
	int minCY=0;

	efcheck 0;
	eflock;
	minCY=_ExtFrames_GetMinParentSize(ExtFrames.List, size);
	efunlock;
	return minCY;
}

int ExtFrames_GetMaxCLCHeight( IN int iMaxDueDesk )
{
	int maxHeight=iMaxDueDesk;
	efcheck 0;
	eflock;
	{
		int i=ExtFrames.List->realCount-1;
		maxHeight=max(iMaxDueDesk, _ExtFrames_GetMinParentSize(ExtFrames.List,NULL));		
		for (; i>0; --i)
		{
			EXTFRAMEWND * extFrame=(EXTFRAMEWND *)ExtFrames.List->items[i];
			if (extFrame && (extFrame->efrm.dwFlags&F_VISIBLE) && !extFrame->efrm.bFloat && !extFrame->efrm.bNotRegistered)
				if (extFrame->efrm.nType==EFT_HORIZONTAL)
					maxHeight-=extFrame->efrm.minCY;
		}
	}
	efunlock;
	return maxHeight;
}

#endif