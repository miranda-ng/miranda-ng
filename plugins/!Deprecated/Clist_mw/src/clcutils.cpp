/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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
#include "clc.h"

//loads of stuff that didn't really fit anywhere else

int HitTest(HWND hwnd,struct ClcData *dat,int testx,int testy,struct ClcContact **contact,ClcGroup **group,DWORD *flags)
{
	struct ClcContact *hitcontact;
	ClcGroup *hitgroup;
	int hit,indent,width,i,cxSmIcon;
	int checkboxWidth, subident,ic = 0;
	SIZE textSize;
	HDC hdc;
	HFONT oldfont;
	RECT clRect;
	DWORD style = GetWindowLongPtr(hwnd,GWL_STYLE);

	if (flags) *flags = 0;
	GetClientRect(hwnd,&clRect);
	if (testx<0 || testy<0 || testy>=clRect.bottom || testx>=clRect.right) {
		if (flags) {
			if (testx<0) *flags |= CLCHT_TOLEFT;
			else if (testx>=clRect.right) *flags |= CLCHT_TORIGHT;
			if (testy<0) *flags |= CLCHT_ABOVE;
			else if (testy>=clRect.bottom) *flags |= CLCHT_BELOW;
		}
		return -1;
	}
	if (testx<dat->leftMargin) {
		if (flags) *flags |= CLCHT_INLEFTMARGIN|CLCHT_NOWHERE;
		return -1;
	}
	hit = GetRowByIndex(dat ,(testy+dat->yScroll)/dat->rowHeight,&hitcontact,&hitgroup);
	if (hit == -1) {
		if (flags) *flags |= CLCHT_NOWHERE|CLCHT_BELOWITEMS;
		return -1;
	}
	if (contact) *contact = hitcontact;
	if (group) *group = hitgroup;
	/////////
	if (hitcontact->type == CLCIT_CONTACT && hitcontact->isSubcontact)
		subident = dat->rowHeight/2;
	else
		subident = 0;

	for (indent = 0;hitgroup->parent;indent++,hitgroup = hitgroup->parent);
	if (testx<dat->leftMargin+indent*dat->groupIndent+subident) {
		if (flags) *flags |= CLCHT_ONITEMINDENT;
		return hit;
	}
	checkboxWidth = 0;
	if (style&CLS_CHECKBOXES && hitcontact->type == CLCIT_CONTACT) checkboxWidth = dat->checkboxSize+2;
	if (style&CLS_GROUPCHECKBOXES && hitcontact->type == CLCIT_GROUP) checkboxWidth = dat->checkboxSize+2;
	if (hitcontact->type == CLCIT_INFO && hitcontact->flags&CLCIIF_CHECKBOX) checkboxWidth = dat->checkboxSize+2;
	if (testx<dat->leftMargin+indent*dat->groupIndent+checkboxWidth+subident) {
		if (flags) *flags |= CLCHT_ONITEMCHECK;
		return hit;
	}
	if (testx<dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace+subident) {
		if (flags) *flags |= CLCHT_ONITEMICON;
		return hit;
	}

	hdc = GetDC(hwnd);
	GetTextExtentPoint32(hdc,hitcontact->szText,lstrlen(hitcontact->szText),&textSize);
	width = textSize.cx;

	cxSmIcon = GetSystemMetrics(SM_CXSMICON);

	for (i=0; i < dat->extraColumnsCount; i++) {
		int x;
		if (hitcontact->iExtraImage[i] == EMPTY_EXTRA_ICON)
			continue;

		if ((style & CLS_EX_MULTICOLUMNALIGNLEFT)) {
			x = (dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace-2+width);
			x += 16;
			x = x+dat->extraColumnSpacing*(ic);
			if (i == dat->extraColumnsCount-1) {x = clRect.right-18;}
		}
		else {
			int ir;
			if (dat->MetaIgnoreEmptyExtra) {
				ir = 0;
				for (int j = i;j<dat->extraColumnsCount;j++)
					if (hitcontact->iExtraImage[j] != EMPTY_EXTRA_ICON)
						ir++;
			}
			else ir = dat->extraColumnsCount-i;

			x = clRect.right-dat->extraColumnSpacing*ir;
		}
		ic++;

		if (testx >= x && testx < x + cxSmIcon) {
			if (flags)
				*flags |= CLCHT_ONITEMEXTRA|(i<<24);

			ReleaseDC(hwnd,hdc);
			return hit;
		}
	}

	if (hitcontact->type == CLCIT_GROUP)
		oldfont = (HFONT)SelectObject(hdc,dat->fontInfo[FONTID_GROUPS].hFont);
	else
		oldfont = (HFONT)SelectObject(hdc,dat->fontInfo[FONTID_CONTACTS].hFont);

	GetTextExtentPoint32(hdc,hitcontact->szText,lstrlen(hitcontact->szText),&textSize);
	width = textSize.cx;
	if (hitcontact->type == CLCIT_GROUP) {
		char *szCounts = pcli->pfnGetGroupCountsText(dat,hitcontact);
		if (szCounts[0]) {
			GetTextExtentPoint32A(hdc," ",1,&textSize);
			width += textSize.cx;
			SelectObject(hdc,dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
			GetTextExtentPoint32A(hdc,szCounts,lstrlenA(szCounts),&textSize);
			width += textSize.cx;
		}
	}
	SelectObject(hdc,oldfont);
	ReleaseDC(hwnd,hdc);
	if (testx<dat->leftMargin+indent*dat->groupIndent+checkboxWidth+dat->iconXSpace+width+4+subident) {
		if (flags) *flags |= CLCHT_ONITEMLABEL;
		return hit;
	}
	if (flags) *flags |= CLCHT_NOWHERE;
	return -1;
}

void ScrollTo(HWND hwnd,struct ClcData *dat,int desty,int noSmooth)
{
	DWORD startTick,nowTick;
	int oldy = dat->yScroll;
	RECT clRect,rcInvalidate;
	int maxy,previousy;

	if (dat->iHotTrack != -1 && dat->yScroll != desty) {
		pcli->pfnInvalidateItem(hwnd,dat,dat->iHotTrack);
		dat->iHotTrack = -1;
		ReleaseCapture();
	}
	GetClientRect(hwnd,&clRect);
	rcInvalidate = clRect;
	maxy = dat->rowHeight*GetGroupContentsCount(&dat->list,2)-clRect.bottom;
	if (desty>maxy) desty = maxy;
	if (desty<0) desty = 0;
	if (abs(desty-dat->yScroll)<4) noSmooth = 1;
	if ( !noSmooth && dat->exStyle&CLS_EX_NOSMOOTHSCROLLING) noSmooth = 1;
	previousy = dat->yScroll;
	if ( !noSmooth) {
		startTick = GetTickCount();
		for (;;) {
			nowTick = GetTickCount();
			if (nowTick>=startTick+dat->scrollTime) break;
			dat->yScroll = oldy+(desty-oldy)*(int)(nowTick-startTick)/dat->scrollTime;
			if (dat->backgroundBmpUse&CLBF_SCROLL || dat->hBmpBackground == NULL)
				ScrollWindowEx(hwnd,0,previousy-dat->yScroll,NULL,NULL,NULL,NULL,SW_INVALIDATE);
			else
				InvalidateRect(hwnd,NULL,FALSE);
			previousy = dat->yScroll;
		  	SetScrollPos(hwnd,SB_VERT,dat->yScroll,TRUE);
			UpdateWindow(hwnd);
		}
	}
	dat->yScroll = desty;
	if (dat->backgroundBmpUse&CLBF_SCROLL || dat->hBmpBackground == NULL)
		ScrollWindowEx(hwnd,0,previousy-dat->yScroll,NULL,NULL,NULL,NULL,SW_INVALIDATE);
	else
		InvalidateRect(hwnd,NULL,FALSE);
	SetScrollPos(hwnd,SB_VERT,dat->yScroll,TRUE);
}

void RecalcScrollBar(HWND hwnd,struct ClcData *dat)
{
	SCROLLINFO si = {0};
	RECT clRect;
	NMCLISTCONTROL nm;
	boolean sbar = FALSE;

	GetClientRect(hwnd,&clRect);

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = dat->rowHeight*GetGroupContentsCount(&dat->list,2)-1;
	si.nPage = clRect.bottom;
	si.nPos = dat->yScroll;

	nm.hdr.code = CLN_LISTSIZECHANGE;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	nm.pt.y = si.nMax;
	SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nm);

	GetClientRect(hwnd,&clRect);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = dat->rowHeight*GetGroupContentsCount(&dat->list,2)-1;
	si.nPage = clRect.bottom;
	si.nPos = dat->yScroll;

	sbar = (dat->noVScrollbar == 1||(int)si.nPage>si.nMax);

	ShowScrollBar(hwnd,SB_VERT,sbar? FALSE : TRUE);
	if ( !sbar) {
		if ( GetWindowLongPtr(hwnd,GWL_STYLE)&CLS_CONTACTLIST ) {
			if ( dat->noVScrollbar == 0 ) SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
			else SetScrollInfo(hwnd,SB_VERT,&si,FALSE);
		}
		else SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
	}
	ScrollTo(hwnd,dat,dat->yScroll,1);
}

void CalcEipPosition( struct ClcData *dat, struct ClcContact *contact, ClcGroup *group, POINT *result)
{
	int indent;
	for (indent = 0; group->parent; indent++, group = group->parent);
	result->x = indent * dat->groupIndent + dat->iconXSpace - 2;
	result->y = dat->selection * dat->rowHeight - dat->yScroll;

	if (contact->type == CLCIT_CONTACT && contact->isSubcontact)
		result->x += dat->rowHeight/2;
}

void GetDefaultFontSetting(int i,LOGFONT *lf,COLORREF *colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),lf,FALSE);
	*colour = GetSysColor(COLOR_WINDOWTEXT);
	switch(i) {
	case FONTID_GROUPS:
		lf->lfWeight = FW_BOLD;
		break;
	case FONTID_GROUPCOUNTS:
		lf->lfHeight = (int)(lf->lfHeight*.75);
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	case FONTID_OFFINVIS:
	case FONTID_INVIS:
		lf->lfItalic = !lf->lfItalic;
		break;
	case FONTID_DIVIDERS:
		lf->lfHeight = (int)(lf->lfHeight*.75);
		break;
	case FONTID_NOTONLIST:
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
}	}

extern void ( *saveLoadClcOptions )(HWND hwnd,struct ClcData *dat);

void LoadClcOptions(HWND hwnd,struct ClcData *dat)
{
	int i;

	dat->style = GetWindowLongPtr(hwnd,GWL_STYLE);
	dat->MetaIgnoreEmptyExtra = db_get_b(NULL,"CLC","MetaIgnoreEmptyExtra",1);
	saveLoadClcOptions( hwnd, dat );

	for (i = 0; i <= FONTID_MAX; i++)
		if (dat->fontInfo[i].fontHeight > dat->rowHeight)
			dat->rowHeight = dat->fontInfo[i].fontHeight;
}
