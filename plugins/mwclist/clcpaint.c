/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

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
*/
#include "commonheaders.h"
#include "m_clc.h"
#include "clc.h"
//#include <gdiplus.h>

extern HIMAGELIST himlCListClc;
static BYTE divide3[765]={255};
int MetaIgnoreEmptyExtra;

static void ChangeToFont(HDC hdc,struct ClcData *dat,int id,int *fontHeight)
{
	SelectObject(hdc,dat->fontInfo[id].hFont);
	SetTextColor(hdc,dat->fontInfo[id].colour);
	if(fontHeight) *fontHeight=dat->fontInfo[id].fontHeight;
}

static void __inline SetHotTrackColour(HDC hdc,struct ClcData *dat)
{
	if(dat->gammaCorrection) {
		COLORREF oldCol,newCol;
		int oldLum,newLum;

		oldCol=GetTextColor(hdc);
		oldLum=(GetRValue(oldCol)*30+GetGValue(oldCol)*59+GetBValue(oldCol)*11)/100;
		newLum=(GetRValue(dat->hotTextColour)*30+GetGValue(dat->hotTextColour)*59+GetBValue(dat->hotTextColour)*11)/100;
		if(newLum==0) {
			SetTextColor(hdc,dat->hotTextColour);
			return;
		}
		if(newLum>=oldLum+20) {
			oldLum+=20;
			newCol=RGB(GetRValue(dat->hotTextColour)*oldLum/newLum,GetGValue(dat->hotTextColour)*oldLum/newLum,GetBValue(dat->hotTextColour)*oldLum/newLum);
		}
		else if(newLum<=oldLum) {
			int r,g,b;
			r=GetRValue(dat->hotTextColour)*oldLum/newLum;
			g=GetGValue(dat->hotTextColour)*oldLum/newLum;
			b=GetBValue(dat->hotTextColour)*oldLum/newLum;
			if(r>255) {
				g+=(r-255)*3/7;
				b+=(r-255)*3/7;
				r=255;
			}
			if(g>255) {
				r+=(g-255)*59/41;
				if(r>255) r=255;
				b+=(g-255)*59/41;
				g=255;
			}
			if(b>255) {
				r+=(b-255)*11/89;
				if(r>255) r=255;
				g+=(b-255)*11/89;
				if(g>255) g=255;
				b=255;
			}
			newCol=RGB(r,g,b);
		}
		else newCol=dat->hotTextColour;
		SetTextColor(hdc,newCol);
	}
	else
		SetTextColor(hdc,dat->hotTextColour);
}

static int GetStatusOnlineness(int status)
{
	switch(status) {
		case ID_STATUS_FREECHAT:   return 110;
		case ID_STATUS_ONLINE:     return 100;
		case ID_STATUS_OCCUPIED:   return 60;
		case ID_STATUS_ONTHEPHONE: return 50;
		case ID_STATUS_DND:        return 40;
		case ID_STATUS_AWAY:	   return 30;
		case ID_STATUS_OUTTOLUNCH: return 20;
		case ID_STATUS_NA:		   return 10;
		case ID_STATUS_INVISIBLE:  return 5;
	}
	return 0;
}

static int GetGeneralisedStatus(void)
{
	int i,status,thisStatus,statusOnlineness,thisOnlineness;

	status=ID_STATUS_OFFLINE;
	statusOnlineness=0;

	for (i=0;i<pcli->hClcProtoCount;i++) {
		thisStatus = pcli->clcProto[i].dwStatus;
		if(thisStatus==ID_STATUS_INVISIBLE) return ID_STATUS_INVISIBLE;
		thisOnlineness=GetStatusOnlineness(thisStatus);
		if(thisOnlineness>statusOnlineness) {
			status=thisStatus;
			statusOnlineness=thisOnlineness;
		}
	}
	return status;
}

static int GetRealStatus(struct ClcContact * contact, int status) 
{
	int i;
	char *szProto=contact->proto;
	if (!szProto) return status;
	for (i=0;i<pcli->hClcProtoCount;i++) {
		if (!lstrcmpA(pcli->clcProto[i].szProto,szProto)) {
			return pcli->clcProto[i].dwStatus;
		}
	}
	return status;
}

static HMODULE  themeAPIHandle = NULL; // handle to uxtheme.dll
static HANDLE   (WINAPI *MyOpenThemeData)(HWND,LPCWSTR);
static HRESULT  (WINAPI *MyCloseThemeData)(HANDLE);
static HRESULT  (WINAPI *MyDrawThemeBackground)(HANDLE,HDC,int,int,const RECT *,const RECT *);

#define MGPROC(x) GetProcAddress(themeAPIHandle,x)
void InternalPaintClc(HWND hwnd,struct ClcData *dat,HDC hdc,RECT *rcPaint)
{
	HDC hdcMem;
	HBITMAP oldbmp;
	HBITMAP oldbm;
	HFONT oldfont;
	RECT clRect;
	int y,indent,index,fontHeight, subindex, subident;
	struct ClcContact *Drawing;
	struct ClcGroup *group;
	HBITMAP hBmpOsb;
	DWORD style=GetWindowLong(hwnd,GWL_STYLE);
	int status=GetGeneralisedStatus();
	int grey=0,groupCountsFontTopShift;
	HBRUSH hBrushAlternateGrey=NULL;
	POINT pt;
	RECT testrc;		


	// yes I know about GetSysColorBrush()
	COLORREF tmpbkcolour = style&CLS_CONTACTLIST ? ( /*dat->useWindowsColours ? GetSysColor(COLOR_3DFACE) :*/ dat->bkColour ) : dat->bkColour;
//


	if(dat->greyoutFlags & pcli->pfnClcStatusToPf2(status) || style&WS_DISABLED) grey=1;
	else if(GetFocus()!=hwnd && dat->greyoutFlags&GREYF_UNFOCUS) grey=1;
	GetClientRect(hwnd,&clRect);
	
	if(rcPaint==NULL) rcPaint=&clRect;
	//rcPaint=&clRect;
	
	if(IsRectEmpty(rcPaint)) return;
	GetCursorPos(&pt);
	ScreenToClient(hwnd,&pt);


	y=-dat->yScroll;
	hdcMem=CreateCompatibleDC(hdc);
	hBmpOsb=CreateBitmap(clRect.right,clRect.bottom,1,GetDeviceCaps(hdc,BITSPIXEL),NULL);
	oldbmp=(HBITMAP)SelectObject(hdcMem,hBmpOsb);
	{	TEXTMETRIC tm;
		
		
		oldfont=SelectObject(hdcMem,dat->fontInfo[FONTID_GROUPS].hFont);
		
		GetTextMetrics(hdcMem,&tm);
		groupCountsFontTopShift=tm.tmAscent;
		SelectObject(hdcMem,dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
		GetTextMetrics(hdcMem,&tm);
		groupCountsFontTopShift-=tm.tmAscent;
		SelectObject(hdcMem,oldfont);
	}
	if(style&CLS_GREYALTERNATE)
		hBrushAlternateGrey=CreateSolidBrush(GetNearestColor(hdcMem,RGB(GetRValue(tmpbkcolour)-10,GetGValue(tmpbkcolour)-10,GetBValue(tmpbkcolour)-10)));
	ChangeToFont(hdcMem,dat,FONTID_CONTACTS,&fontHeight);
	SetBkMode(hdcMem,TRANSPARENT);
	{	HBRUSH hBrush,hoBrush;

		hBrush=CreateSolidBrush(tmpbkcolour);
		hoBrush=(HBRUSH)SelectObject(hdcMem,hBrush);
		FillRect(hdcMem,rcPaint,hBrush);
		SelectObject(hdcMem,hoBrush);
		DeleteObject(hBrush);
		if(dat->hBmpBackground) {
			BITMAP bmp;
			HDC hdcBmp;
			int x,y;
			int maxx,maxy;
			int destw,desth;

			// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
			SetStretchBltMode(hdcMem, HALFTONE);


			GetObject(dat->hBmpBackground,sizeof(bmp),&bmp);
			hdcBmp=CreateCompatibleDC(hdcMem);
			oldbm=SelectObject(hdcBmp,dat->hBmpBackground);
			y=dat->backgroundBmpUse&CLBF_SCROLL?-dat->yScroll:0;
			maxx=dat->backgroundBmpUse&CLBF_TILEH?clRect.right:1;
			maxy=dat->backgroundBmpUse&CLBF_TILEV?maxy=rcPaint->bottom:y+1;
			switch(dat->backgroundBmpUse&CLBM_TYPE) {
				case CLB_STRETCH:
					if(dat->backgroundBmpUse&CLBF_PROPORTIONAL) {
						if(clRect.right*bmp.bmHeight<clRect.bottom*bmp.bmWidth) {
							desth=clRect.bottom;
							destw=desth*bmp.bmWidth/bmp.bmHeight;
						}
						else {
							destw=clRect.right;
							desth=destw*bmp.bmHeight/bmp.bmWidth;
						}
					}
					else {
						destw=clRect.right;
						desth=clRect.bottom;
					}
					break;
				case CLB_STRETCHH:
					if(dat->backgroundBmpUse&CLBF_PROPORTIONAL) {
						destw=clRect.right;
						desth=destw*bmp.bmHeight/bmp.bmWidth;
					}
					else {
						destw=clRect.right;
						desth=bmp.bmHeight;
							if (dat->backgroundBmpUse&CLBF_TILEVTOROWHEIGHT)
							{
								desth=dat->rowHeight;
							}	

					}
					break;
				case CLB_STRETCHV:
					if(dat->backgroundBmpUse&CLBF_PROPORTIONAL) {
						desth=clRect.bottom;
						destw=desth*bmp.bmWidth/bmp.bmHeight;
					}
					else {
						destw=bmp.bmWidth;
						desth=clRect.bottom;
					}
					break;
				default:    //clb_topleft
					destw=bmp.bmWidth;
					desth=bmp.bmHeight;
					if (dat->backgroundBmpUse&CLBF_TILEVTOROWHEIGHT)
					{
						desth=dat->rowHeight;
					}							
					break;
			}
			for(;y<maxy;y+=desth) {
				if(y<rcPaint->top-desth) continue;
				for(x=0;x<maxx;x+=destw)
					StretchBlt(hdcMem,x,y,destw,desth,hdcBmp,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
			}
			SelectObject(hdcBmp,oldbm);
			DeleteDC(hdcBmp);
		}
	}
	group=&dat->list;
	group->scanIndex=0;
	indent=0;
	subindex=-1;
	for(index=0;y<rcPaint->bottom;) 
	{
		if (subindex==-1)
			if (group->scanIndex==group->cl.count) 
			{
				group=group->parent;
				indent--;
				if(group==NULL) break;
				group->scanIndex++;
				continue;
			}
		//if(group==NULL) break;
		if(y>rcPaint->top-dat->rowHeight ) 
		{


			int iImage=-1;
			int selected;
			int hottrack;
			SIZE textSize,countsSize,spaceSize;
			int width,checkboxWidth;
			char *szCounts;
			
			if (subindex==-1)
			{
				Drawing=group->cl.items[group->scanIndex];
				subident=0;
			}
			else
			{
				Drawing=&(group->cl.items[group->scanIndex]->subcontacts[subindex]);
				subident=dat->rowHeight/2;
			}
			//alternating grey

			selected=index==dat->selection && (dat->showSelAlways || dat->exStyle&CLS_EX_SHOWSELALWAYS || GetFocus()==hwnd) && Drawing->type!=CLCIT_DIVIDER;
			hottrack=dat->exStyle&CLS_EX_TRACKSELECT && Drawing->type!=CLCIT_DIVIDER && dat->iHotTrack==index;
			
			if(style&CLS_GREYALTERNATE && index&1) {
				RECT rc;
				rc.top=y; rc.bottom=rc.top+dat->rowHeight;
				rc.left=0; rc.right=clRect.right;
				FillRect(hdcMem,&rc,hBrushAlternateGrey);
			}

			//setup
			if(Drawing->type==CLCIT_GROUP)
			{
				if (Drawing->group->expanded) 
				{
					ChangeToFont(hdcMem,dat,FONTID_GROUPS,&fontHeight);
				}
				else 
				{
					ChangeToFont(hdcMem,dat,FONTID_GROUPSCLOSED,&fontHeight);
				}
			}
			else if(Drawing->type==CLCIT_INFO) {
				if(Drawing->flags&CLCIIF_GROUPFONT) ChangeToFont(hdcMem,dat,FONTID_GROUPS,&fontHeight);
				else ChangeToFont(hdcMem,dat,FONTID_CONTACTS,&fontHeight);
			}
			else if(Drawing->type==CLCIT_DIVIDER)
				ChangeToFont(hdcMem,dat,FONTID_DIVIDERS,&fontHeight);
			else if(Drawing->type==CLCIT_CONTACT && (Drawing->flags&CONTACTF_NOTONLIST) && (!Drawing->isSubcontact) )
				ChangeToFont(hdcMem,dat,FONTID_NOTONLIST,&fontHeight);
			else if ( Drawing->type==CLCIT_CONTACT && 
				(	
					(Drawing->flags&CONTACTF_INVISTO && (!Drawing->isSubcontact) && GetRealStatus(group->cl.items[group->scanIndex], status) != ID_STATUS_INVISIBLE )
					||
					(Drawing->flags&CONTACTF_VISTO && (!Drawing->isSubcontact) && GetRealStatus(group->cl.items[group->scanIndex], status)==ID_STATUS_INVISIBLE)
				) 
			) 
			{
				// the contact is in the always visible list and the proto is invisible
				// the contact is in the always invisible and the proto is in any other mode
				ChangeToFont(hdcMem,dat, Drawing->flags&CONTACTF_ONLINE ? FONTID_INVIS:FONTID_OFFINVIS ,&fontHeight);
			}
			else if(Drawing->type==CLCIT_CONTACT && (!(Drawing->flags&CONTACTF_ONLINE) && (!Drawing->isSubcontact)))
				ChangeToFont(hdcMem,dat,FONTID_OFFLINE,&fontHeight);
			else
				ChangeToFont(hdcMem,dat,FONTID_CONTACTS,&fontHeight);
			GetTextExtentPoint32(hdcMem,Drawing->szText,lstrlen(Drawing->szText),&textSize);
			width=textSize.cx;
			if(Drawing->type==CLCIT_GROUP) {
		
				szCounts = pcli->pfnGetGroupCountsText(dat,group->cl.items[group->scanIndex]);
				if(szCounts[0]) {
					GetTextExtentPoint32A(hdcMem," ",1,&spaceSize);
					ChangeToFont(hdcMem,dat,FONTID_GROUPCOUNTS,&fontHeight);
					GetTextExtentPoint32A(hdcMem,szCounts,lstrlenA(szCounts),&countsSize);
					width+=spaceSize.cx+countsSize.cx;
				}
			}

			if((style&CLS_CHECKBOXES && Drawing->type==CLCIT_CONTACT) ||
			   (style&CLS_GROUPCHECKBOXES && Drawing->type==CLCIT_GROUP) ||
			   (Drawing->type==CLCIT_INFO && Drawing->flags&CLCIIF_CHECKBOX))
				checkboxWidth=dat->checkboxSize+2;
			else checkboxWidth=0;

			//background
			if(selected) {
				switch (dat->HiLightMode)
				{
				case 0:
						{
						int x=dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace-2+subident;
						ImageList_DrawEx(dat->himlHighlight,0,hdcMem,x,y,min(width+5,clRect.right-x),dat->rowHeight,CLR_NONE,CLR_NONE,dat->exStyle&CLS_EX_NOTRANSLUCENTSEL?ILD_NORMAL:ILD_BLEND25);
						SetTextColor(hdcMem,dat->selTextColour);
						break;
						}
				case 1:
					{
   					  ImageList_DrawEx(dat->himlHighlight,0,hdcMem,0,y,clRect.right,
						  dat->rowHeight,CLR_NONE,CLR_NONE,
						  dat->exStyle&CLS_EX_NOTRANSLUCENTSEL?ILD_NORMAL:ILD_BLEND25);
						 SetTextColor(hdcMem,dat->selTextColour);
						 break;
					}

				case 2:
					{
						  ImageList_DrawEx(dat->himlHighlight,0,hdcMem,1,y+1,clRect.right-2,
						  dat->rowHeight-1,CLR_NONE,CLR_NONE,
						  dat->exStyle&CLS_EX_NOTRANSLUCENTSEL?ILD_NORMAL:ILD_BLEND25);
						  SetTextColor(hdcMem,dat->selTextColour);
						  break;
					}
				case 3:
					{
						SetTextColor(hdcMem,dat->selTextColour);	
						break;
					}
				}
			}
			else if(hottrack)
				SetHotTrackColour(hdcMem,dat);

			//checkboxes
			if(checkboxWidth && (subindex==-1 ||1)) {
				RECT rc;
				HANDLE hTheme = NULL;

				// THEME
				if (IsWinVerXPPlus()) {
					if (!themeAPIHandle) {
						themeAPIHandle = GetModuleHandleA("uxtheme");
						if (themeAPIHandle) {
							MyOpenThemeData = (HANDLE (WINAPI *)(HWND,LPCWSTR))MGPROC("OpenThemeData");
							MyCloseThemeData = (HRESULT (WINAPI *)(HANDLE))MGPROC("CloseThemeData");
							MyDrawThemeBackground = (HRESULT (WINAPI *)(HANDLE,HDC,int,int,const RECT *,const RECT *))MGPROC("DrawThemeBackground");
						}
					}
					// Make sure all of these methods are valid (i would hope either all or none work)
					if (MyOpenThemeData
							&&MyCloseThemeData
							&&MyDrawThemeBackground) {
						hTheme = MyOpenThemeData(hwnd,L"BUTTON");
					}
				}
				rc.left=dat->leftMargin+indent*dat->groupIndent+subident;
				rc.right=rc.left+dat->checkboxSize;
				rc.top=y+((dat->rowHeight-dat->checkboxSize)>>1);
				rc.bottom=rc.top+dat->checkboxSize;
				if (hTheme) {
					MyDrawThemeBackground(hTheme, hdcMem, BP_CHECKBOX, Drawing->flags&CONTACTF_CHECKED?(hottrack?CBS_CHECKEDHOT:CBS_CHECKEDNORMAL):(hottrack?CBS_UNCHECKEDHOT:CBS_UNCHECKEDNORMAL), &rc, &rc);
				}
				else DrawFrameControl(hdcMem,&rc,DFC_BUTTON,DFCS_BUTTONCHECK|DFCS_FLAT|(Drawing->flags&CONTACTF_CHECKED?DFCS_CHECKED:0)|(hottrack?DFCS_HOT:0));
				if (hTheme&&MyCloseThemeData) {
					MyCloseThemeData(hTheme);
					hTheme = NULL;
				}
			}

			//icon
			if(Drawing->type==CLCIT_GROUP)
				iImage=Drawing->group->expanded?IMAGE_GROUPOPEN:IMAGE_GROUPSHUT;
			else if(Drawing->type==CLCIT_CONTACT)
				iImage=Drawing->iImage;
			if(iImage!=-1) {
				/*COLORREF colourFg=dat->selBkColour;
				int mode=ILD_NORMAL;
				if(selected) mode=ILD_SELECTED;
				else if(hottrack) {mode=ILD_FOCUS; colourFg=dat->hotTextColour;}
				else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST) {colourFg=dat->fontInfo[FONTID_NOTONLIST].colour; mode=ILD_BLEND50;}
				ImageList_DrawEx(himlCListClc,iImage,hdcMem,dat->leftMargin+indent*dat->groupIndent+checkboxWidth,y+((dat->rowHeight-16)>>1),0,0,CLR_NONE,colourFg,mode);
				*/
				COLORREF colourFg=dat->selBkColour;
				int mode=ILD_NORMAL;
				if(hottrack) {colourFg=dat->hotTextColour;}
				else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST && (!Drawing->isSubcontact)) {colourFg=dat->fontInfo[FONTID_NOTONLIST].colour; mode=ILD_BLEND50;}
				if (Drawing->type==CLCIT_CONTACT && dat->showIdle && (Drawing->flags&CONTACTF_IDLE)&& (!Drawing->isSubcontact) && GetRealStatus(group->cl.items[group->scanIndex],ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
					mode=ILD_SELECTED;
				ImageList_DrawEx(himlCListClc,iImage,hdcMem,dat->leftMargin+subident+indent*dat->groupIndent+checkboxWidth,y+((dat->rowHeight-16)>>1),0,0,CLR_NONE,colourFg,mode);
			}

			//text
			if(Drawing->type==CLCIT_DIVIDER) {
				RECT rc;
				rc.top=y+(dat->rowHeight>>1); rc.bottom=rc.top+2;
				rc.left=dat->leftMargin+indent*dat->groupIndent;
				rc.right=rc.left+((clRect.right-rc.left-textSize.cx)>>1)-3;
				DrawEdge(hdcMem,&rc,BDR_SUNKENOUTER,BF_RECT);
				TextOut(hdcMem,rc.right+3,y+((dat->rowHeight-fontHeight)>>1),Drawing->szText,lstrlen(Drawing->szText));
				rc.left=rc.right+6+textSize.cx;
				rc.right=clRect.right;
				DrawEdge(hdcMem,&rc,BDR_SUNKENOUTER,BF_RECT);
			}
			else if(Drawing->type==CLCIT_GROUP) {
				RECT rc;
				if(szCounts[0]) {
					struct ClcGroup *clcg;

					clcg=(struct ClcGroup *)Drawing;

					fontHeight=dat->fontInfo[FONTID_GROUPS].fontHeight;

					rc.left=dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace;
					rc.right=min(clRect.right-countsSize.cx,rc.left+textSize.cx+spaceSize.cx);
					rc.top=y+((dat->rowHeight-fontHeight)>>1);
					rc.bottom=rc.top+textSize.cy;
					if(rc.right<rc.left+4) rc.right=clRect.right+1;
					else TextOutA(hdcMem,rc.right,rc.top+groupCountsFontTopShift,szCounts,lstrlenA(szCounts));

					//ChangeToFont(hdcMem,dat,FONTID_GROUPS,&fontHeight);
					if (Drawing->group->expanded) 
					{
						ChangeToFont(hdcMem,dat,FONTID_GROUPS,&fontHeight);
					}
					else 
					{
						ChangeToFont(hdcMem,dat,FONTID_GROUPSCLOSED,&fontHeight);
					}

					if(selected)
						SetTextColor(hdcMem,dat->selTextColour);
					else if(hottrack)
						SetHotTrackColour(hdcMem,dat);
					rc.right--;
					ExtTextOut(hdcMem,rc.left,rc.top,ETO_CLIPPED,&rc,Drawing->szText,lstrlen(Drawing->szText),NULL);
				}
				else TextOut(hdcMem,dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace,y+((dat->rowHeight-fontHeight)>>1),Drawing->szText,lstrlen(Drawing->szText));
				if(dat->exStyle&CLS_EX_LINEWITHGROUPS) {
					//calc if extra icons present
					int enabledextraicons=0;
					for(iImage=0;iImage<dat->extraColumnsCount;iImage++) {
						if(Drawing->iExtraImage[iImage]==0xFF) continue;
						enabledextraicons++;
					}
					rc.top=y+(dat->rowHeight>>1); rc.bottom=rc.top+2;
					rc.left=dat->leftMargin+subident+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace+width+3;
					rc.right=clRect.right-1-dat->extraColumnSpacing*enabledextraicons;
					if(rc.right-rc.left>1) DrawEdge(hdcMem,&rc,BDR_SUNKENOUTER,BF_RECT);
				}
			}
			else
			{
				RECT rc;
				rc.left=dat->leftMargin+subident+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace;
				rc.top=y+((dat->rowHeight-fontHeight)>>1);
				rc.bottom=rc.top+dat->rowHeight;
				rc.right=clRect.right;
				if (Drawing->type=CLCIT_CONTACT)
				{
					if (hottrack/*PtInRect(&rc,pt)*/)
					{
						ChangeToFont(hdcMem,dat,FONTID_CONTACTSHOVER,&fontHeight);
					}
			}

				DrawText(hdcMem,Drawing->szText,lstrlen(Drawing->szText),&rc,DT_END_ELLIPSIS);

			}
			if(selected) {
				if(Drawing->type!=CLCIT_DIVIDER) {
					SetTextColor(hdcMem,dat->quickSearchColour);
					TextOut(hdcMem,dat->leftMargin+subident+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace,y+((dat->rowHeight-fontHeight)>>1),Drawing->szText,lstrlen(dat->szQuickSearch));
				}
			}

			if (dat->style&CLS_SHOWSTATUSMESSAGES)
			{							
				// status message
				if (group->cl.items[group->scanIndex]->type==CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags & CONTACTF_STATUSMSG) {
					TCHAR * szText = group->cl.items[group->scanIndex]->szStatusMsg;
					RECT rc;
					rc.left=dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace;
					rc.top=y+dat->rowHeight+((dat->rowHeight-fontHeight)>>1);
					rc.right=(clRect.right - clRect.left);
					rc.bottom=rc.top+dat->rowHeight;
					ChangeToFont(hdcMem,dat,FONTID_STATUSMSG,&fontHeight);
					//ExtTextOut(hdcMem,rc.left,rc.top,ETO_CLIPPED,&rc,szText,lstrlen(szText),NULL);
					DrawText(hdcMem, szText, lstrlen(szText), &rc, DT_SINGLELINE | DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS);
				}		
			}

			if (!Drawing->isSubcontact || (DBGetContactSettingByte(NULL,"CLC","MetaHideExtra",0)==0))
			{

				//extra icons
				if (!(style&CLS_EX_MULTICOLUMNALIGNLEFT))
				{
					int c=dat->extraColumnsCount;
					for(iImage=dat->extraColumnsCount-1;iImage>=0;iImage--) {
						COLORREF colourFg=dat->selBkColour;
						int mode=ILD_NORMAL;

						if(Drawing->iExtraImage[iImage]==0xFF) continue;
						if(selected) mode=ILD_SELECTED;
						else if(hottrack) {mode=ILD_FOCUS; colourFg=dat->hotTextColour;}
						else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST) {colourFg=dat->fontInfo[FONTID_NOTONLIST].colour; mode=ILD_BLEND50;}
						{				  

						}
						if (dat->MetaIgnoreEmptyExtra) c--; else c=iImage;

						testrc.left=clRect.right-dat->extraColumnSpacing*(dat->extraColumnsCount-c);
						testrc.top=(y+((dat->rowHeight-16)>>1));
						testrc.right=testrc.left+16;
						testrc.bottom=testrc.top+16;
						if (!PtInRect(&testrc,pt))
						{
							mode=ILD_NORMAL;
						}

						ImageList_DrawEx(dat->himlExtraColumns,Drawing->iExtraImage[iImage],hdcMem,clRect.right-dat->extraColumnSpacing*(dat->extraColumnsCount-c),y+((dat->rowHeight-16)>>1),0,0,CLR_NONE,colourFg,mode);
					}
				}
				else
				{
					int ic=0;	
					for(iImage=0;iImage<dat->extraColumnsCount;iImage++) {
						COLORREF colourFg=dat->selBkColour;
						int mode=ILD_NORMAL;
						int x;

						//if(group->cl.items[group->scanIndex].iExtraImage[iImage]==0xFF) continue;
						/*	
						if(selected) mode=ILD_SELECTED;
						else if(hottrack) {mode=ILD_FOCUS; colourFg=dat->hotTextColour;}
						else if(group->cl.items[group->scanIndex]->type==CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags&CONTACTF_NOTONLIST) {colourFg=dat->fontInfo[FONTID_NOTONLIST].colour; mode=ILD_BLEND50;}

						x=(dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace-2+width);
						x+=16;
						x=x+dat->extraColumnSpacing*(ic);
						if (iImage==dat->extraColumnsCount-1) {x=clRect.right-18;}
						ImageList_DrawEx(dat->himlExtraColumns,group->cl.items[group->scanIndex].iExtraImage[iImage],hdcMem,
						x,
						y+((dat->rowHeight-16)>>1),0,0,CLR_NONE,colourFg,mode);
						*/
						if(Drawing->iExtraImage[iImage]==0xFF) continue;
						//if(selected) mode=ILD_SELECTED;
						else if(hottrack) {mode=ILD_FOCUS; colourFg=dat->hotTextColour;}
						else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST) {colourFg=dat->fontInfo[FONTID_NOTONLIST].colour; mode=ILD_BLEND50;}

						x=(dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace-2+width);
						x+=16;
						x=x+dat->extraColumnSpacing*(ic);
						if (iImage==dat->extraColumnsCount-1) {x=clRect.right-18;}
						ImageList_DrawEx(dat->himlExtraColumns,Drawing->iExtraImage[iImage],hdcMem,
							x,
							y+((dat->rowHeight-16)>>1),0,0,CLR_NONE,colourFg,mode);

						ic++;
					}
				}
			}
		}
		index++;
		y+=dat->rowHeight;

		if (group->cl.items[group->scanIndex]->type==CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags & CONTACTF_STATUSMSG) {
			y+=dat->rowHeight;
			index++;
		}

		//increment by subcontacts
		if (group->cl.items[group->scanIndex]->subcontacts!=NULL && group->cl.items[group->scanIndex]->type!=CLCIT_GROUP)
			if (group->cl.items[group->scanIndex]->SubExpanded)
				if (subindex<group->cl.items[group->scanIndex]->SubAllocated-1)
					subindex++;
				else subindex=-1;

		if(subindex==-1) {
			if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP && group->cl.items[group->scanIndex]->group->expanded) 
			{
				group=group->cl.items[group->scanIndex]->group;
				indent++;
				group->scanIndex=0;
				subindex=-1;
				continue;
			}
			group->scanIndex++;
		}
	}
	if(dat->iInsertionMark!=-1) {	//insertion mark
		HBRUSH hBrush,hoBrush;
		POINT pts[8];
		HRGN hRgn;

		pts[0].x=dat->leftMargin; pts[0].y=dat->iInsertionMark*dat->rowHeight-dat->yScroll-4;
		pts[1].x=pts[0].x+2;      pts[1].y=pts[0].y+3;
		pts[2].x=clRect.right-4;  pts[2].y=pts[1].y;
		pts[3].x=clRect.right-1;  pts[3].y=pts[0].y-1;
		pts[4].x=pts[3].x;        pts[4].y=pts[0].y+7;
		pts[5].x=pts[2].x+1;      pts[5].y=pts[1].y+2;
		pts[6].x=pts[1].x;        pts[6].y=pts[5].y;
		pts[7].x=pts[0].x;        pts[7].y=pts[4].y;
		hRgn=CreatePolygonRgn(pts,SIZEOF(pts),ALTERNATE);
		hBrush=CreateSolidBrush(dat->fontInfo[FONTID_CONTACTS].colour);
		hoBrush=(HBRUSH)SelectObject(hdcMem,hBrush);
		FillRgn(hdcMem,hRgn,hBrush);
		SelectObject(hdcMem,hoBrush);
		DeleteObject(hBrush);
	}
	if(!grey)
		BitBlt(hdc,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,hdcMem,rcPaint->left,rcPaint->top,SRCCOPY);
	//DeleteDC(hdcMem);
	if(hBrushAlternateGrey) DeleteObject(hBrushAlternateGrey);
	if(grey) {
		PBYTE bits;
		BITMAPINFOHEADER bmih={0};
		int i;
		int greyRed,greyGreen,greyBlue;
		COLORREF greyColour;
		bmih.biBitCount=32;
		bmih.biSize=sizeof(bmih);
		bmih.biCompression=BI_RGB;
		bmih.biHeight=-clRect.bottom;
		bmih.biPlanes=1;
		bmih.biWidth=clRect.right;
		bits=(PBYTE)mir_alloc(4*bmih.biWidth*-bmih.biHeight);
		GetDIBits(hdc,hBmpOsb,0,clRect.bottom,bits,(BITMAPINFO*)&bmih,DIB_RGB_COLORS);
		greyColour=GetSysColor(COLOR_3DFACE);
		greyRed=GetRValue(greyColour)*2;
		greyGreen=GetGValue(greyColour)*2;
		greyBlue=GetBValue(greyColour)*2;
		if(divide3[0]==255) {
			for(i=0; i < SIZEOF(divide3); i++) divide3[i]=(i+1)/3;
		}
		for(i=4*clRect.right*clRect.bottom-4;i>=0;i-=4) {
			bits[i]=divide3[bits[i]+greyBlue];
			bits[i+1]=divide3[bits[i+1]+greyGreen];
			bits[i+2]=divide3[bits[i+2]+greyRed];
		}
		SetDIBitsToDevice(hdc,0,0,clRect.right,clRect.bottom,0,0,0,clRect.bottom,bits,(BITMAPINFO*)&bmih,DIB_RGB_COLORS);
		mir_free(bits);
	}
	SelectObject(hdcMem,oldfont);
	SelectObject(hdcMem,oldbmp);
	DeleteObject(hBmpOsb);
	DeleteDC(hdcMem);
}

void PaintClc(HWND hwnd,struct ClcData *dat,HDC hdc,RECT *rcPaint)
{
	if (SED.cbSize==sizeof(SED)&&SED.PaintClc!=NULL)
	{
		SED.PaintClc(hwnd,dat,hdc,rcPaint,pcli->hClcProtoCount,pcli->clcProto,himlCListClc);
		return;
	}
	InternalPaintClc(hwnd,dat,hdc,rcPaint);
}
