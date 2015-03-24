#include "stdhdr.h"

ThumbList thumbList;

/////////////////////////////////////////////////////////////////////////////
// ThumbInfo
static POINT ptOld;
static BOOL	bMouseDown		 = FALSE;
static BOOL	bMouseIn		 = FALSE;
static BOOL	bMouseMoved		 = FALSE;
static int	nLeft = 0;
static int  nTop  = 0;
static int  nOffs = 5;
static ThumbInfo *pThumbMouseIn	 = NULL;

static void SnapToScreen( RECT rcThumb, int nX, int nY, int *pX, int *pY )
{
	int nWidth;
	int nHeight;

	assert( NULL != pX );
	assert( NULL != pY );
	
	nWidth	 = rcThumb.right - rcThumb.left;
	nHeight = rcThumb.bottom - rcThumb.top;

	*pX = nX < ( nOffs + rcScreen.left ) ? rcScreen.left : nX;
	*pY = nY < ( nOffs + rcScreen.top ) ? rcScreen.top : nY;
	*pX = *pX > ( rcScreen.right - nWidth - nOffs ) ? ( rcScreen.right - nWidth ) : *pX;
	*pY = *pY > ( rcScreen.bottom - nHeight - nOffs ) ? ( rcScreen.bottom - nHeight ) : *pY;
}

ThumbInfo::ThumbInfo()
{
	dropTarget = new CDropTarget;
	dropTarget->AddRef();
	btAlpha = 255;
}

ThumbInfo::~ThumbInfo()
{
	if (pThumbMouseIn == this) {
		pThumbMouseIn = NULL;
		KillTimer(hwnd, TIMERID_LEAVE_T);
	}
	dropTarget->Release();
}

void ThumbInfo::GetThumbRect(RECT *rc)
{
	rc->left = ptPos.x;
	rc->top = ptPos.y;
	rc->right = ptPos.x + szSize.cx;
	rc->bottom = ptPos.y + szSize.cy;
}

void ThumbInfo::PositionThumb(int nX, int nY)
{
	POINT pos = { nX, nY };

	HDWP hdwp = BeginDeferWindowPos(1);

	ThumbInfo *pThumb = this;
	while (pThumb) {
		pThumb->PositionThumbWorker(pos.x, pos.y, &pos);
		if (NULL != pThumb->hwnd) /* Wine fix. */
			hdwp = DeferWindowPos(hdwp, pThumb->hwnd, HWND_TOPMOST, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		pThumb->ptPos = pos;
		pos.x += pThumb->szSize.cx;

		pThumb = fcOpt.bMoveTogether ? thumbList.FindThumb( pThumb->dockOpt.hwndRight ) : NULL;
	}

	EndDeferWindowPos(hdwp);
}

void ThumbInfo::PositionThumbWorker(int nX, int nY, POINT *newPos)
{
	RECT  rc;
	RECT  rcThumb;
	int   nNewX;
	int   nNewY;
	int   nWidth;
	int   nHeight;
	POINT pt;
	RECT	rcLeft;
	RECT	rcTop;
	RECT	rcRight;
	RECT	rcBottom;
	BOOL	bDocked;
	BOOL	bDockedLeft;
	BOOL	bDockedRight;
	BOOL	bLeading;

	// Get thumb dimnsions
	GetThumbRect( &rcThumb );
	nWidth	 = rcThumb.right - rcThumb.left;
	nHeight = rcThumb.bottom - rcThumb.top;

	// Docking and screen boundaries check
	SnapToScreen( rcThumb, nX, nY, &nNewX, &nNewY );
	
	bLeading = dockOpt.hwndRight != NULL;

	if ( fcOpt.bMoveTogether )
		UndockThumbs( this, thumbList.FindThumb( dockOpt.hwndLeft ));

	for (int i = 0; i < thumbList.getCount(); ++i) {
		ThumbInfo *pCurThumb	= &thumbList[i];
		if (pCurThumb == this)
			continue;

		GetThumbRect( &rcThumb );
		OffsetRect( &rcThumb, nX - rcThumb.left, nY - rcThumb.top );
			
		pCurThumb->GetThumbRect( &rc );

		// These are rects we will dock into
					
		rcLeft.left		 = rc.left - nOffs;
		rcLeft.top		 = rc.top - nOffs;
		rcLeft.right	 = rc.left + nOffs;
		rcLeft.bottom	 = rc.bottom + nOffs;
						
		rcTop.left		 = rc.left - nOffs;
		rcTop.top		 = rc.top - nOffs;
		rcTop.right		 = rc.right + nOffs;
		rcTop.bottom	 = rc.top + nOffs;

		rcRight.left	 = rc.right - nOffs;
		rcRight.top		 = rc.top - nOffs;
		rcRight.right	 = rc.right + nOffs;
		rcRight.bottom	 = rc.bottom + nOffs;

		rcBottom.left	 = rc.left - nOffs;
		rcBottom.top	 = rc.bottom - nOffs;
		rcBottom.right	 = rc.right + nOffs;
		rcBottom.bottom = rc.bottom + nOffs;
			
		bDockedLeft = bDockedRight	= FALSE;

		// Upper-left
		pt.x = rcThumb.left;
		pt.y = rcThumb.top;
		bDocked = FALSE;
			
		if ( PtInRect(&rcRight, pt)) {
			nNewX	 = rc.right;
			bDocked = TRUE;
		}
			
		if ( PtInRect(&rcBottom, pt)) {
			nNewY = rc.bottom;
			if ( PtInRect( &rcLeft, pt))
				nNewX = rc.left;
		}

		if ( PtInRect(&rcTop, pt)) {
			nNewY		 = rc.top;
			bDockedLeft	 = bDocked;
		}
			
		// Upper-right
		pt.x = rcThumb.right;
		pt.y = rcThumb.top;
		bDocked = FALSE;

		if ( !bLeading && PtInRect( &rcLeft, pt)) {
			if (!bDockedLeft) {
				nNewX	 = rc.left - nWidth;
				bDocked	 = TRUE;
			}
			else if ( rc.right == rcThumb.left )
				bDocked = TRUE;
		}
			

		if ( PtInRect( &rcBottom, pt)) {
			nNewY = rc.bottom;
			if ( PtInRect( &rcRight, pt))
				nNewX = rc.right - nWidth;
		}

		if ( !bLeading && PtInRect( &rcTop, pt)) {
			nNewY = rc.top;
			bDockedRight = bDocked;
		}
			
		if ( fcOpt.bMoveTogether ) {
			if (bDockedRight)
				DockThumbs(this, pCurThumb, TRUE);
				
			if (bDockedLeft)
				DockThumbs(pCurThumb, this, FALSE);
		}									

		// Lower-left
		pt.x = rcThumb.left;
		pt.y = rcThumb.bottom;

		if ( PtInRect( &rcRight, pt))
			nNewX = rc.right;

		if ( PtInRect( &rcTop, pt)) {
			nNewY = rc.top - nHeight;
			if ( PtInRect( &rcLeft, pt))
				nNewX = rc.left;
		}
			

		// Lower-right
		pt.x = rcThumb.right;
		pt.y = rcThumb.bottom;

		if ( !bLeading && PtInRect( &rcLeft, pt))
			nNewX = rc.left - nWidth;

		if ( !bLeading && PtInRect( &rcTop, pt)) {
			nNewY = rc.top - nHeight;

			if ( PtInRect( &rcRight, pt))
			{
				nNewX = rc.right - nWidth;
			}
		}
	}

	// Adjust coords once again
	SnapToScreen( rcThumb, nNewX, nNewY, &nNewX, &nNewY );
	
	newPos->x = nNewX;
	newPos->y = nNewY;
}

void ThumbInfo::ResizeThumb()
{
	int index = FLT_FONTID_NOTONLIST;
	
	himlMiranda = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	if (himlMiranda == NULL)
		return;
	
	SIZEL sizeIcon;
	ImageList_GetIconSize_my(himlMiranda, sizeIcon);

	HDC hdc = GetWindowDC(hwnd);
	if (!db_get_b(hContact, "CList", "NotOnList", 0)) {
		char *szProto = GetContactProto(hContact);
		if ( NULL != szProto )
		{
			int nStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			int nContactStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			int nApparentMode = db_get_w(hContact, szProto, "ApparentMode", 0);
			
			if ((nStatus == ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_ONLINE) ||
				 (nStatus != ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_OFFLINE))
			{
				if (ID_STATUS_OFFLINE == nContactStatus)
					index = FLT_FONTID_OFFINVIS;
				else
					index = FLT_FONTID_INVIS;
			}
			else if (ID_STATUS_OFFLINE == nContactStatus)
				index = FLT_FONTID_OFFLINE;
			else
				index = FLT_FONTID_CONTACTS;
		}
	}
	else index = FLT_FONTID_NOTONLIST;

	HFONT hOldFont = (HFONT)SelectObject( hdc, hFont[ index ] );
	
	// Get text and icon sizes
	SIZEL sizeText;
	GetTextExtentPoint32( hdc, ptszName, (int)_tcslen(ptszName), &sizeText);
	
	SelectObject(hdc, hOldFont);
	
	// Transform text size
	POINT ptText;
	ptText.x = sizeText.cx;
	ptText.y = sizeText.cy;
	LPtoDP( hdc, &ptText, 1);
	
	szSize.cx = fcOpt.bFixedWidth ? fcOpt.nThumbWidth : sizeIcon.cx + ptText.x + 10;
	szSize.cy = (( sizeIcon.cy > ptText.y ) ? sizeIcon.cy : ptText.y ) + 4;

	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, szSize.cx, szSize.cy, SWP_NOMOVE | SWP_NOACTIVATE);
	
	RefreshContactIcon(0xFFFFFFFF);
	
	ReleaseDC(hwnd, hdc);

	// Move the docked widnow if needed
	ThumbInfo *pNextThumb = thumbList.FindThumb(dockOpt.hwndRight);
	if (pNextThumb) {
		RECT rcThumb;
		GetThumbRect(&rcThumb);
		pNextThumb->PositionThumb(rcThumb.right, rcThumb.top);
	}
}

void ThumbInfo::RefreshContactIcon(int iIcon)
{
	if (iIcon == 0xFFFFFFFF || ImageList_GetImageCount(himlMiranda) <= iIcon)
		this->iIcon = CallService(MS_CLIST_GETCONTACTICON, hContact, 0);	
	else
		this->iIcon = iIcon;

	UpdateContent();
}

void ThumbInfo::RefreshContactStatus(int idStatus)
{
	if ( IsStatusVisible(idStatus))
		RegisterFileDropping( hwnd, dropTarget );
	else
		UnregisterFileDropping( hwnd );

	ShowWindow( hwnd, fcOpt.bHideAll || HideOnFullScreen() || ( fcOpt.bHideOffline && ( !IsStatusVisible( idStatus )) ) || (fcOpt.bHideWhenCListShow && bIsCListShow) ? SW_HIDE : SW_SHOWNA );
}

void ThumbInfo::DeleteContactPos()
{
	db_unset(hContact, MODULE, "ThumbsPos");
}

void ThumbInfo::OnLButtonDown(int nX, int nY)
{
	if (bEnableTip && fcOpt.bShowTip)
		KillTip();

	GetCursorPos(&ptOld);

	RECT rc;
	GetThumbRect(&rc);
	
	nLeft	 = rc.left;
	nTop	 = rc.top;
	
	bMouseDown	 = TRUE;
	bMouseMoved	 = FALSE;
}

void ThumbInfo::OnLButtonUp()
{
	RECT	rcMiranda;
	RECT	rcThumb;
	RECT	rcOverlap;

	if (!bMouseMoved && fcOpt.bUseSingleClick && bMouseIn)
		PopupMessageDialog();

	if (bMouseDown) {
		bMouseDown = FALSE;
		SetCursor( LoadCursor(NULL, IDC_ARROW));
	
		// Check whether we shoud remove the window
		GetWindowRect( hwndMiranda, &rcMiranda );
		GetThumbRect( &rcThumb );

		if ( IntersectRect(&rcOverlap, &rcMiranda, &rcThumb)) {
			if ( IsWindowVisible(hwndMiranda)) {
				DeleteContactPos();
				thumbList.RemoveThumb(this);
			}
		}
	}

	SaveContactsPos();
}

void ThumbInfo::OnMouseMove(int nX, int nY, WPARAM wParam)
{
	int		dX;
	int		dY;
	POINT	ptNew;

	// Position thumb
	
	if (bMouseDown) {
		ptNew.x = nX;
		ptNew.y = nY;

		ClientToScreen( hwnd, &ptNew );				
		
		dX = ptNew.x - ptOld.x;
		dY = ptNew.y - ptOld.y;
		
		if (dX || dY) {
			bMouseMoved	 = TRUE;

			nLeft	+= dX;
			nTop	+= dY;

			PositionThumb( nLeft, nTop );
		}

		ptOld = ptNew;
	}
	else SetCursor( LoadCursor(NULL, IDC_ARROW));

	// Update selection status
	if ( !pThumbMouseIn ) {
	    SetTimer( hwnd, TIMERID_LEAVE_T, 10, NULL ); 
		pThumbMouseIn = this;

		ThumbSelect(TRUE);
	}
	
	if (bEnableTip && fcOpt.bShowTip && !bMouseDown) {
		WORD tmpTimeIn;
		POINT pt;
		RECT rc;

		GetCursorPos(&pt);
		GetThumbRect(&rc);
		if (!PtInRect(&rc,pt)) {
			KillTip();
			return;
		}
		if (fTipTimerActive && abs(pt.x-ptTipSt.x)<5 && abs(pt.y-ptTipSt.x)<5)
			return;

		ptTipSt = pt;

		if (fTipTimerActive)
			KillTimer(hwnd, TIMERID_HOVER_T);

		if (fTipActive)
			return;

		tmpTimeIn = (fcOpt.TimeIn>0)?fcOpt.TimeIn:CallService(MS_CLC_GETINFOTIPHOVERTIME, 0, 0);
		SetTimer(hwnd, TIMERID_HOVER_T, tmpTimeIn, 0);
		fTipTimerActive = TRUE;
	}
}

void ThumbInfo::ThumbSelect(BOOL bMouse)
{
	if (bMouse) {
		bMouseIn = TRUE;
		SetCapture(hwnd);
	}

	SetThumbOpacity(255);
}

void ThumbInfo::ThumbDeselect(BOOL bMouse)
{
	if (bMouse) {
		bMouseIn = FALSE;
		ReleaseCapture();
	}
	
	SetThumbOpacity(fcOpt.thumbAlpha);
}

void ThumbInfo::SetThumbOpacity(BYTE bAlpha)
{
	if (bAlpha != btAlpha) {
		btAlpha = bAlpha;
		UpdateContent();
	}
}

void ThumbInfo::KillTip()
{
	if (fTipTimerActive) {
		KillTimer(hwnd, TIMERID_HOVER_T);
		fTipTimerActive = FALSE;
	}

	if (fTipActive) {
		CallService("mToolTip/HideTip", 0, 0);
		fTipActive = FALSE;
	}
}

void ThumbInfo::UpdateContent()
{
	bmpContent.allocate(szSize.cx, szSize.cy);

	HFONT hOldFont;
	SIZE  size;
	RECT  rc, rcText;
	DWORD oldColor;
	int   oldBkMode, index = 0;// nStatus;
	UINT	fStyle = ILD_NORMAL;
	
	HDC		hdcDraw	 = bmpContent.getDC();
	SetRect(&rc, 0, 0, szSize.cx, szSize.cy);

	if (NULL != hBmpBackground) {
		RECT rcBkgnd;
		SetRect(&rcBkgnd, 0, 0, szSize.cx, szSize.cy);
		if (NULL != hLTEdgesPen)
			InflateRect(&rcBkgnd, -1, -1);
		int width = rcBkgnd.right - rcBkgnd.left;
		int height = rcBkgnd.bottom - rcBkgnd.top;

		BITMAP bmp;
		GetObject(hBmpBackground, sizeof(bmp), &bmp);
		HDC hdcBmp = CreateCompatibleDC(hdcDraw);
		HBITMAP hbmTmp = (HBITMAP)SelectObject(hdcBmp, hBmpBackground);

		int maxx = (0 != (nBackgroundBmpUse & CLBF_TILEH) ? rcBkgnd.right : rcBkgnd.left + 1);
		int maxy = (0 != (nBackgroundBmpUse & CLBF_TILEV) ? rcBkgnd.bottom : rcBkgnd.top + 1);
		int destw,desth;
		switch (nBackgroundBmpUse & CLBM_TYPE) {
		case CLB_STRETCH:
			if (0 != (nBackgroundBmpUse & CLBF_PROPORTIONAL)) {
				if (width * bmp.bmHeight < height * bmp.bmWidth) {
					desth = height;
					destw = desth * bmp.bmWidth / bmp.bmHeight;
				}
				else {
					destw = width;
					desth = destw * bmp.bmHeight / bmp.bmWidth;
				}
			}
			else {
				destw = width;
				desth = height;
			}
			break;

		case CLB_STRETCHH:
			destw = width;
			if (0 != (nBackgroundBmpUse & CLBF_PROPORTIONAL))
				desth = destw * bmp.bmHeight / bmp.bmWidth;
			else
				desth = bmp.bmHeight;
			break;

		case CLB_STRETCHV:
			desth = height;
			if (0 != (nBackgroundBmpUse & CLBF_PROPORTIONAL))
				destw = desth * bmp.bmWidth / bmp.bmHeight;
			else
				destw = bmp.bmWidth;
			break;

		default:    //clb_topleft
			destw = bmp.bmWidth;
			desth = bmp.bmHeight;
			break;
		}
		SetStretchBltMode(hdcBmp, STRETCH_HALFTONE);

		for (int x = rcBkgnd.left; x < maxx; x += destw)
			for (int y = rcBkgnd.top; y < maxy; y += desth)
				StretchBlt( hdcDraw, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY );

		SelectObject(hdcBmp, hbmTmp);
		DeleteDC(hdcBmp);
	}
	else FillRect(hdcDraw, &rc, hBkBrush);

	if (NULL != hLTEdgesPen) {
		HPEN  hOldPen = (HPEN)SelectObject(hdcDraw, hLTEdgesPen);

		MoveToEx(hdcDraw, 0, 0, NULL);
		LineTo(hdcDraw, szSize.cx, 0);
		MoveToEx(hdcDraw, 0, 0, NULL);
		LineTo(hdcDraw, 0, szSize.cy);

		SelectObject(hdcDraw, hRBEdgesPen);

		MoveToEx(hdcDraw, 0, szSize.cy - 1, NULL);
		LineTo(hdcDraw, szSize.cx - 1, szSize.cy - 1);
		MoveToEx(hdcDraw, szSize.cx - 1, szSize.cy - 1, NULL);
		LineTo(hdcDraw, szSize.cx - 1, 0);

		SelectObject(hdcDraw, hOldPen);
	}

	bmpContent.setAlpha(btAlpha);

	ImageList_GetIconSize_my(himlMiranda, size);

	oldBkMode = SetBkMode(hdcDraw, TRANSPARENT);

	if (!db_get_b(hContact, "CList", "NotOnList", 0)) {
		char *szProto = GetContactProto(hContact);
		if (NULL != szProto) {
			int nStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			int nContactStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			int nApparentMode = db_get_w(hContact, szProto, "ApparentMode", 0);
			
			if ((nStatus == ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_ONLINE) ||
				 (nStatus != ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_OFFLINE))
			{
				if (ID_STATUS_OFFLINE == nContactStatus)
					index = FLT_FONTID_OFFINVIS;
				else {
					index  = 	FLT_FONTID_INVIS;
					if (fcOpt.bShowIdle && db_get_dw(hContact, szProto, "IdleTS", 0))
						fStyle |= ILD_BLEND50;
				}
			}
			else if (ID_STATUS_OFFLINE == nContactStatus) {
				index = FLT_FONTID_OFFLINE;
			}
			else {
				index = FLT_FONTID_CONTACTS;
				if (fcOpt.bShowIdle && db_get_dw(hContact, szProto, "IdleTS", 0))
					fStyle |= ILD_BLEND50;
			}

		}
	}
	else {
		index = FLT_FONTID_NOTONLIST;
		fStyle |= ILD_BLEND50;
	}

	oldColor = SetTextColor(hdcDraw, tColor[index]);
	
	HICON icon = ImageList_GetIcon(himlMiranda, iIcon, ILD_NORMAL);
	MyBitmap bmptmp(size.cx, size.cy);
	bmptmp.DrawIcon(icon,0,0);//bmpContent
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = (fStyle&ILD_BLEND50)?128:255;
	blend.AlphaFormat = AC_SRC_ALPHA;
	AlphaBlend(hdcDraw, 2,( szSize.cy - size.cx ) / 2, bmptmp.getWidth(), bmptmp.getHeight(), bmptmp.getDC(), 0, 0, bmptmp.getWidth(), bmptmp.getHeight(), blend);
	DestroyIcon(icon);

	SetRect(&rcText, 0, 0, szSize.cx, szSize.cy);
	rcText.left += size.cx + 4;

	hOldFont = (HFONT)SelectObject(hdcDraw, hFont[index]);

	SIZE szText;
	GetTextExtentPoint32(hdcDraw, ptszName, (int)_tcslen(ptszName), &szText);
	SetTextColor(hdcDraw, bkColor);

	// simple border
	bmpContent.DrawText(ptszName, rcText.left-1, (rcText.top + rcText.bottom - szText.cy)/2);
	bmpContent.DrawText(ptszName, rcText.left+1, (rcText.top + rcText.bottom - szText.cy)/2);
	bmpContent.DrawText(ptszName, rcText.left, (rcText.top + rcText.bottom - szText.cy)/2-1);
	bmpContent.DrawText(ptszName, rcText.left, (rcText.top + rcText.bottom - szText.cy)/2+1);

	// blurred border
	// bmpContent.DrawText(ptszName, rcText.left, (rcText.top + rcText.bottom - szText.cy)/2, 3);

	// text itself
	SetTextColor(hdcDraw, tColor[index]);
	bmpContent.DrawText(ptszName, rcText.left, (rcText.top + rcText.bottom - szText.cy)/2);
	
	SelectObject(hdcDraw, hOldFont);
	
	SetTextColor(hdcDraw, oldColor);
	SetBkMode(hdcDraw, oldBkMode);

	SetWindowLongPtr( hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED );

	GetWindowRect(hwnd, &rc);
	POINT ptDst = {rc.left, rc.top};
	POINT ptSrc = {0, 0};

	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	UpdateLayeredWindow(hwnd, NULL, &ptDst, &szSize, bmpContent.getDC(), &ptSrc, 0xffffffff, &blend, ULW_ALPHA);
}

void ThumbInfo::PopupMessageDialog()
{
	CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
}

void ThumbInfo::OnTimer(BYTE idTimer)
{
	if (idTimer == TIMERID_SELECT_T) {
		KillTimer( hwnd, TIMERID_SELECT_T );
		ThumbDeselect( FALSE );
	}
	if (idTimer == TIMERID_LEAVE_T && !bMouseDown) {
		POINT pt;
		RECT rc;

		GetCursorPos(&pt);
		GetThumbRect(&rc);
		if (!PtInRect(&rc, pt)) {
			KillTimer( hwnd, TIMERID_LEAVE_T );
			pThumbMouseIn	 = NULL;
			ThumbDeselect( TRUE );
		}
	}
	if (bEnableTip && fcOpt.bShowTip && idTimer == TIMERID_HOVER_T) {
		POINT pt;
		CLCINFOTIP ti = {0};
		ti.cbSize = sizeof(ti);

		KillTimer(hwnd, TIMERID_HOVER_T);
		fTipTimerActive = FALSE;
		GetCursorPos(&pt);
		if (abs(pt.x-ptTipSt.x)<5 && abs(pt.y-ptTipSt.y)<5) {
			ti.ptCursor = pt;

			fTipActive = TRUE;
			ti.isGroup = 0;
			ti.hItem = (HANDLE)hContact;
			ti.isTreeFocused = 0;
			CallService("mToolTip/ShowTip", 0, (LPARAM)&ti);
		}
	}
}

void DockThumbs( ThumbInfo *pThumbLeft, ThumbInfo *pThumbRight, BOOL bMoveLeft )
{
	if (pThumbRight->dockOpt.hwndLeft == NULL && pThumbLeft->dockOpt.hwndRight == NULL) {
		pThumbRight->dockOpt.hwndLeft	 = pThumbLeft->hwnd;
		pThumbLeft->dockOpt.hwndRight	 = pThumbRight->hwnd;
	}
}


void UndockThumbs(ThumbInfo *pThumb1, ThumbInfo *pThumb2)
{
	if (pThumb1 == NULL || pThumb2 == NULL)
		return;

	if (pThumb1->dockOpt.hwndRight == pThumb2->hwnd)
		pThumb1->dockOpt.hwndRight = NULL;

	if (pThumb1->dockOpt.hwndLeft == pThumb2->hwnd)
		pThumb1->dockOpt.hwndLeft = NULL;
	
	if (pThumb2->dockOpt.hwndRight == pThumb1->hwnd)
		pThumb2->dockOpt.hwndRight = NULL;

	if (pThumb2->dockOpt.hwndLeft == pThumb1->hwnd)
		pThumb2->dockOpt.hwndLeft = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// ThumbList

ThumbList::ThumbList(): OBJLIST<ThumbInfo>(1, NumericKeySortT)
{
}

ThumbList::~ThumbList()
{
}

ThumbInfo *ThumbList::AddThumb(HWND hwnd, TCHAR *ptszName, MCONTACT hContact)
{
	if (ptszName == NULL || hContact == NULL || hwnd == NULL)
		return NULL;

	ThumbInfo *pThumb = new ThumbInfo;
	_tcsncpy( pThumb->ptszName, ptszName, USERNAME_LEN - 1);
	pThumb->hContact = hContact;
	pThumb->hwnd = hwnd;

	pThumb->dockOpt.hwndLeft = NULL;
	pThumb->dockOpt.hwndRight = NULL;

	pThumb->fTipActive = FALSE;
	RegHotkey(hContact, hwnd);

	insert(pThumb);
	return pThumb;
}

void ThumbList::RemoveThumb(ThumbInfo *pThumb)
{
	if (!pThumb) return;

	if (fcOpt.bMoveTogether) {
		UndockThumbs(pThumb, FindThumb(pThumb->dockOpt.hwndLeft));
		UndockThumbs(pThumb, FindThumb(pThumb->dockOpt.hwndRight));
	}

	UnregisterFileDropping(pThumb->hwnd);
	DestroyWindow(pThumb->hwnd);
	remove(pThumb);
}

ThumbInfo* ThumbList::FindThumb(HWND hwnd)
{
	if (!hwnd) return NULL;

	for (int i = 0; i < getCount(); ++i)
		if (items[i]->hwnd == hwnd)
			return items[i];

	return NULL;
}

ThumbInfo *ThumbList::FindThumbByContact(MCONTACT hContact)
{
	if (!hContact) return NULL;

	for (int i = 0; i < getCount(); ++i)
		if (items[i]->hContact == hContact)
			return items[i];

	return NULL;
}
