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

File contains realization of internal only available procedures 
for modern_ext_frames.c module.

This file have to be excluded from compilation and need to be adde to project via
#include preprocessor derective in modern_ext_frames.c

\**************************************************************************/

#include "..\commonheaders.h"  //only for precompiled headers

#ifdef __modern_ext_frames_c__include_c_file   //protection from adding to compilation
//  Closed module methods
static int _ExtFrames_GetMinParentSize(IN SortedList* pList, OUT SIZE * size )
{
	EXTFRAME * clcFrame=(EXTFRAME *) pList->items[pList->realCount-1];
	int minCX=clcFrame->minCX;
	int minCY=clcFrame->minCY;
	int i=pList->realCount-1;
	for (; i>0; --i)
	{
		EXTFRAME * extFrame=(EXTFRAME *)pList->items[i];
		if (extFrame && (extFrame->dwFlags&F_VISIBLE) && !extFrame->bFloat  && !extFrame->bNotRegistered)
		{
			if (extFrame->nType==EFT_VERTICAL)
			{
				minCX+=extFrame->minCX;
				minCY=max( minCY, extFrame->minCY );
			}
			else
			{
				minCY+=extFrame->minCY;
				minCX=max( minCX,extFrame->minCX );
			}
		}
	}
	if (size)
	{
		size->cx=minCX;
		size->cy=minCY;
	}
	return minCY;
}

static int _ExtFrames_CalcFramesRect(IN SortedList* pList, IN int width, IN int height, OUT RECT * pOutRect )
{
	int outHeight=height;
	int i;
	SIZE size;
	RECT outRect={0};
	int frmCount=pList->realCount-1;
	EXTFRAME * clcFrame=(EXTFRAME *)pList->items[frmCount];
	if (ExtFrames_GetMinWindowSize(&size))   //ensure that we provide normal size
	{
		width=max(size.cx,width);
		height=max(size.cy,height);
	}
	outRect.right=width;
	outRect.bottom=height;

	for (i=0; i<frmCount; i++)
	{
		EXTFRAME * extFrame=(EXTFRAME *)pList->items[i];
		if (extFrame && (extFrame->dwFlags&F_VISIBLE) && !extFrame->bFloat && !extFrame->bNotRegistered)
		{	
			extFrame->rcFrameRect=outRect;
			switch(extFrame->nEdge)
			{
			case EFP_LEFT:
				extFrame->rcFrameRect.right=extFrame->rcFrameRect.left+extFrame->minCX;
				outRect.left+=extFrame->minCX;
				break;
			case EFP_RIGHT:
				extFrame->rcFrameRect.left=extFrame->rcFrameRect.right-extFrame->minCX;
				outRect.right-=extFrame->minCX;
				break;
			case EFP_TOP:
				extFrame->rcFrameRect.bottom=extFrame->rcFrameRect.top+extFrame->minCY;
				outRect.top+=extFrame->minCY;
				break;
			case EFP_BOTTOM:
				extFrame->rcFrameRect.top=extFrame->rcFrameRect.bottom-extFrame->minCY;
				outRect.bottom-=extFrame->minCY;
				break;
			}
		}
	}
	clcFrame->rcFrameRect=outRect;
	if (pOutRect)
	{
		pOutRect->top=0;
		pOutRect->left=0;
		pOutRect->right=width;
		pOutRect->bottom=height;
	}
	outHeight=height;
	return height;
}

static void _ExtFrames_DestructorOf_EXTFRAMEWND(void * extFrame)
{
	EXTFRAMEWND * pExtFrameWnd = (EXTFRAMEWND *) extFrame;
	if (!pExtFrameWnd) return; //early exit
	if ( pExtFrameWnd->efrm.szFrameNameID ) mir_free( pExtFrameWnd->efrm.szFrameNameID );
	mir_free( pExtFrameWnd );
	return;
}

static void _ExtFrames_GetFrameDBOption(EXTFRAMEWND * pExtFrm)
{
	//   Each known frame order per 1 000 000
    //	 Each Unknown frame but absent during saving per 10 000
	//   Each new unknown per 100 
	static DWORD NextUnknownOrder=100;		
	char szKey[100]={0};
	DWORD dwOrderInDB;
	if (!mir_strcmpi(pExtFrm->efrm.szFrameNameID,"My Contacts"))
		dwOrderInDB=0xFFFFFFFF;
	else
	{
		_snprintf(szKey,sizeof(szKey), EXTFRAMEORDERDBPREFIX "%s",pExtFrm->efrm.szFrameNameID);
		dwOrderInDB=DBGetContactSettingDword(NULL,EXTFRAMEMODULE,szKey,0);
		if (!dwOrderInDB)
		{
			dwOrderInDB=NextUnknownOrder;
			NextUnknownOrder+=100;
		}
		else
		    NextUnknownOrder = dwOrderInDB + 100;
	}
	pExtFrm->efrm.dwOrder=dwOrderInDB;
	
}
static int  _ExtFramesUtils_CopmareFrames(void * first, void * last)
{
   EXTFRAMEWND * pExtFrmWnd1=(EXTFRAMEWND *)first;
   EXTFRAMEWND * pExtFrmWnd2=(EXTFRAMEWND *)last;
   if (!pExtFrmWnd1 || ! pExtFrmWnd2) return 0;
   return pExtFrmWnd1->efrm.dwOrder-pExtFrmWnd2->efrm.dwOrder;
}
static void _ExtFramesUtils_CheckAlighment(EXTFRAMEWND * pExtFrm)
{	
	if (!(pExtFrm->efrm.dwFlags&F_CANBEVERTICAL) && pExtFrm->efrm.nType == EFT_VERTICAL)
	{
		//issue have not be ever Vertical
		int minsize = pExtFrm->efrm.minCX;
		pExtFrm->efrm.minCX = pExtFrm->efrm.minCY;
		pExtFrm->efrm.minCY = minsize;
		
		pExtFrm->efrm.nType = EFT_HORIZONTAL;
		pExtFrm->efrm.nEdge&=(~alVertFrameMask);
		
	}
	else if (pExtFrm->efrm.dwFlags&F_CANBEVERTICAL && !(pExtFrm->efrm.dwFlags&F_CANNOTBEHORIZONTAL) && (pExtFrm->efrm.nType == EFT_HORIZONTAL))
	{
		//issue have not be Horizontal
		int minsize = pExtFrm->efrm.minCX;
		pExtFrm->efrm.minCX = pExtFrm->efrm.minCY;
		pExtFrm->efrm.minCY = minsize;

		pExtFrm->efrm.nType=EFT_VERTICAL;
		pExtFrm->efrm.nEdge|=alVertFrameMask;
	}
}

#endif