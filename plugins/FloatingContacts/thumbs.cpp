#include "stdhdr.h"
#include "Wingdi.h"

ThumbList thumbList;

/////////////////////////////////////////////////////////////////////////////
// ThumbInfo
static POINT	ptOld;
static BOOL	bMouseDown		= FALSE;
static BOOL	bMouseIn		= FALSE;
static BOOL	bMouseMoved		= FALSE;
static short	nLeft			= 0;
static short	nTop			= 0;
static int		nOffs			= 5;
static ThumbInfo *pThumbMouseIn	= NULL;

static void SnapToScreen( RECT rcThumb, int nX, int nY, int *pX, int *pY )
{
	int nWidth;
	int nHeight;

	assert( NULL != pX );
	assert( NULL != pY );
	
	nWidth	= rcThumb.right - rcThumb.left;
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
	if(pThumbMouseIn==this){
		pThumbMouseIn=NULL;
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

void ThumbInfo::PositionThumb(short nX, short nY)
{
	POINT pos = { nX, nY };
	HDWP hdwp;

	hdwp = BeginDeferWindowPos(1);

	ThumbInfo *pThumb = this;
	while (pThumb)
	{
		pThumb->PositionThumbWorker( (short)pos.x, (short)pos.y, &pos );

		DeferWindowPos(	hdwp,
						pThumb->hwnd, 
						HWND_TOPMOST, 
						pos.x, 
						pos.y, 
						0, 
						0,
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

		pThumb->ptPos = pos;
		pos.x += pThumb->szSize.cx;

		pThumb = fcOpt.bMoveTogether ? thumbList.FindThumb( pThumb->dockOpt.hwndRight ) : NULL;
	}

	EndDeferWindowPos(hdwp);
}

void ThumbInfo::PositionThumbWorker(short nX, short nY, POINT *newPos)
{
	RECT		rc;
	RECT		rcThumb;
	int			nNewX;
	int			nNewY;
	int			nWidth;
	int			nHeight;
	POINT		pt;
	RECT		rcLeft;
	RECT		rcTop;
	RECT		rcRight;
	RECT		rcBottom;
	BOOL		bDocked;
	BOOL		bDockedLeft;
	BOOL		bDockedRight;
	BOOL		bLeading;

	// Get thumb dimnsions
	GetThumbRect( &rcThumb );
	nWidth	= rcThumb.right - rcThumb.left;
	nHeight = rcThumb.bottom - rcThumb.top;

	// Docking and screen boundaries check
	SnapToScreen( rcThumb, nX, nY, &nNewX, &nNewY );
	
	bLeading = dockOpt.hwndRight != NULL;

	if ( fcOpt.bMoveTogether )
	{
		UndockThumbs( this, thumbList.FindThumb( dockOpt.hwndLeft ) );
	}


	for (int i = 0; i < thumbList.getCount(); ++i)
	{
		ThumbInfo	*pCurThumb	= thumbList[i];

		if ( pCurThumb != this )
		{
			GetThumbRect( &rcThumb );
			OffsetRect( &rcThumb, nX - rcThumb.left, nY - rcThumb.top );
			
			pCurThumb->GetThumbRect( &rc );

			// These are rects we will dock into
					
			rcLeft.left		= rc.left - nOffs;
			rcLeft.top		= rc.top - nOffs;
			rcLeft.right	= rc.left + nOffs;
			rcLeft.bottom	= rc.bottom + nOffs;
						
			rcTop.left		= rc.left - nOffs;
			rcTop.top		= rc.top - nOffs;
			rcTop.right		= rc.right + nOffs;
			rcTop.bottom	= rc.top + nOffs;

			rcRight.left	= rc.right - nOffs;
			rcRight.top		= rc.top - nOffs;
			rcRight.right	= rc.right + nOffs;
			rcRight.bottom	= rc.bottom + nOffs;

			rcBottom.left	= rc.left - nOffs;
			rcBottom.top	= rc.bottom - nOffs;
			rcBottom.right	= rc.right + nOffs;
			rcBottom.bottom = rc.bottom + nOffs;

			
			bDockedLeft		= FALSE;
			bDockedRight	= FALSE;

			// Upper-left
			pt.x	= rcThumb.left;
			pt.y	= rcThumb.top;
			bDocked	= FALSE;
			
			if ( PtInRect( &rcRight, pt ) )
			{
				nNewX	= rc.right;
				bDocked = TRUE;
			}
			
			if ( PtInRect( &rcBottom, pt ) )
			{
				nNewY = rc.bottom;

				if ( PtInRect( &rcLeft, pt ) )
				{
					nNewX = rc.left;
				}
			}

			if ( PtInRect( &rcTop, pt ) )
			{
				nNewY		= rc.top;
				bDockedLeft	= bDocked;
			}
			
			// Upper-right
			pt.x	= rcThumb.right;
			pt.y	= rcThumb.top;
			bDocked	= FALSE;

			if ( !bLeading && PtInRect( &rcLeft, pt ) )
			{
				if ( !bDockedLeft )
				{
					nNewX	= rc.left - nWidth;
					bDocked	= TRUE;
				}
				else if ( rc.right == rcThumb.left )
				{
					bDocked = TRUE;
				}
			}
			

			if ( PtInRect( &rcBottom, pt ) )
			{
				nNewY = rc.bottom;

				if ( PtInRect( &rcRight, pt ) )
				{
					nNewX = rc.right - nWidth;
				}
			}

			if ( !bLeading && PtInRect( &rcTop, pt ) )
			{
				nNewY			= rc.top;
				bDockedRight	= bDocked;
			}
			
			if ( fcOpt.bMoveTogether )
			{
				if ( bDockedRight )
				{
					DockThumbs( this, pCurThumb, TRUE );
				}
				
				if ( bDockedLeft )
				{
					DockThumbs( pCurThumb, this, FALSE );
				}
			}									

			// Lower-left
			pt.x = rcThumb.left;
			pt.y = rcThumb.bottom;

			if ( PtInRect( &rcRight, pt ) )
			{
				nNewX = rc.right;
			}

			if ( PtInRect( &rcTop, pt ) )
			{
				nNewY = rc.top - nHeight;

				if ( PtInRect( &rcLeft, pt ) )
				{
					nNewX = rc.left;
				}
			}
			

			// Lower-right
			pt.x = rcThumb.right;
			pt.y = rcThumb.bottom;

			if ( !bLeading && PtInRect( &rcLeft, pt ) )
			{
				nNewX = rc.left - nWidth;
			}

			if ( !bLeading && PtInRect( &rcTop, pt ) )
			{
				nNewY = rc.top - nHeight;

				if ( PtInRect( &rcRight, pt ) )
				{
					nNewX = rc.right - nWidth;
				}
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
	HDC		hdc			= NULL;
	HFONT	hOldFont	= NULL;
	POINT	ptText;
	SIZEL	sizeIcon;
	SIZEL	sizeText;
	RECT	rcThumb;
	int		index		= FLT_FONTID_NOTONLIST;

	ThumbInfo *pNextThumb = NULL;
	
	himl = ( HIMAGELIST )CallService( MS_CLIST_GETICONSIMAGELIST, 0, 0 );
	
	if ( himl == NULL ) return;
	
	ImageList_GetIconSize_my(himl, sizeIcon);

	hdc = GetWindowDC(hwnd);

	if (!DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
	{
		int nStatus;
		int nContactStatus;
		int nApparentMode;
		char* szProto;

		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		
		if ( NULL != szProto )
		{
			nStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			nContactStatus = DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			nApparentMode = DBGetContactSettingWord(hContact, szProto, "ApparentMode", 0);
			
			if (	(nStatus == ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_ONLINE)
				||	(nStatus != ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_OFFLINE)
				)
			{
				if (ID_STATUS_OFFLINE == nContactStatus)
				{
					index = FLT_FONTID_OFFINVIS;
				}
				else
				{
					index =	FLT_FONTID_INVIS;
				}
			}
			else if (ID_STATUS_OFFLINE == nContactStatus)
			{
				index = FLT_FONTID_OFFLINE;
			}
			else
			{
				index = FLT_FONTID_CONTACTS;
			}
		}
	}
	else
	{
		index = FLT_FONTID_NOTONLIST;
	}

	hOldFont = (HFONT)SelectObject( hdc, hFont[ index ] );
	
	// Get text and icon sizes
	GetTextExtentPoint32( hdc, ptszName, _tcslen( ptszName ), &sizeText );
	
	
	SelectObject( hdc, hOldFont );
	
	// Transform text size
	ptText.x = sizeText.cx;
	ptText.y = sizeText.cy;
	LPtoDP( hdc, &ptText, 1 );

	
	szSize.cx = fcOpt.bFixedWidth ? fcOpt.nThumbWidth : sizeIcon.cx + ptText.x + 10;
	szSize.cy = ( ( sizeIcon.cy > ptText.y ) ? sizeIcon.cy : ptText.y ) + 4;

	SetWindowPos(	hwnd, 
					HWND_TOPMOST, 
					0, 
					0, 
					szSize.cx, 
					szSize.cy,
					SWP_NOMOVE | /*SWP_NOZORDER |*/ SWP_NOACTIVATE );
	
	RefreshContactIcon(0xFFFFFFFF);
	
	ReleaseDC( hwnd, hdc );

	// Move the docked widnow if needed
	if (pNextThumb = thumbList.FindThumb(dockOpt.hwndRight))
	{
		GetThumbRect( &rcThumb );
		pNextThumb->PositionThumb( (short)rcThumb.right, (short)rcThumb.top );
	}
}

void ThumbInfo::RefreshContactIcon(int iIcon)
{
	if ( iIcon == 0xFFFFFFFF || ImageList_GetImageCount(himl)<=iIcon )
	{
		this->iIcon = CallService( MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0 );	
	}
	else
	{
		this->iIcon = iIcon;
	}

	UpdateContent();
}

void ThumbInfo::RefreshContactStatus(int idStatus)
{
	if ( IsStatusVisible( idStatus ) )
	{
		RegisterFileDropping( hwnd, dropTarget );
	}
	else
	{
		UnregisterFileDropping( hwnd );
	}

	ShowWindow( hwnd, fcOpt.bHideAll || HideOnFullScreen() || ( fcOpt.bHideOffline && ( !IsStatusVisible( idStatus ) ) ) || (fcOpt.bHideWhenCListShow && bIsCListShow) ? SW_HIDE : SW_SHOWNA );
}

void ThumbInfo::DeleteContactPos()
{
	DBDeleteContactSetting( hContact, sModule, "ThumbsPos" );
}

void ThumbInfo::OnLButtonDown(short nX, short nY)
{
	RECT rc;

	if(bEnableTip && fcOpt.bShowTip) KillTip();

//	ptOld.x = nX;
//	ptOld.y = nY;
	
//	ClientToScreen( hwnd, &ptOld );

	GetCursorPos(&ptOld);
	GetThumbRect(&rc);
	
	nLeft	= (short)rc.left;
	nTop	= (short)rc.top;
	
	//bMouseIn	= FALSE;
	bMouseDown	= TRUE;
	bMouseMoved	= FALSE;

//	SetCapture(hwnd);
}

void ThumbInfo::OnLButtonUp()
{
	RECT	rcMiranda;
	RECT	rcThumb;
	RECT	rcOverlap;

	if (!bMouseMoved && fcOpt.bUseSingleClick && bMouseIn){
		PopUpMessageDialog();
	}

	//ThumbDeselect( TRUE );

	if ( bMouseDown )
	{
		bMouseDown = FALSE;
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	
		// Check whether we shoud remove the window
		GetWindowRect( hwndMiranda, &rcMiranda );
		GetThumbRect( &rcThumb );

		if ( IntersectRect( &rcOverlap, &rcMiranda, &rcThumb ) )
		{
			if( IsWindowVisible( hwndMiranda ) )
			{
				DeleteContactPos( );
				thumbList.RemoveThumb( this );
			}
		}
	}

	SaveContactsPos();
}

void ThumbInfo::OnMouseMove(short nX, short nY, WPARAM wParam)
{
//	if (bMouseDown && !wParam&MK_LBUTTON) OnLButtonUp();

	int		dX;
	int		dY;
	POINT	ptNew;

	// Position thumb
	
	if( bMouseDown )
	{

		ptNew.x = nX;
		ptNew.y = nY;

		ClientToScreen( hwnd, &ptNew );				
		
		dX = ptNew.x - ptOld.x;
		dY = ptNew.y - ptOld.y;
		
		if(dX || dY){
			bMouseMoved	= TRUE;

			nLeft	+= (short)dX;
			nTop	+= (short)dY;

			PositionThumb( nLeft, nTop );
		}

		ptOld = ptNew;
	}
	else
	{
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	}

	// Update selection status
	if ( !pThumbMouseIn )//
	{
	    SetTimer( hwnd, TIMERID_LEAVE_T, 10, NULL ); 
		pThumbMouseIn=this;

		ThumbSelect( TRUE );
	}
	if(bEnableTip && fcOpt.bShowTip && !bMouseDown){
		WORD tmpTimeIn;
		POINT pt;
		RECT rc;

		GetCursorPos(&pt);
		GetThumbRect(&rc);
		if(!PtInRect(&rc,pt)){
			KillTip();
			return;
		}
		if(fTipTimerActive && abs(pt.x-ptTipSt.x)<5 && abs(pt.y-ptTipSt.x)<5){
			return;
		}
		ptTipSt=pt;

		if (fTipTimerActive) {
			KillTimer(hwnd, TIMERID_HOVER_T);
		}
		if (fTipActive) {
			return;
		}

		tmpTimeIn = (fcOpt.TimeIn>0)?fcOpt.TimeIn:CallService(MS_CLC_GETINFOTIPHOVERTIME, 0, 0);
		SetTimer(hwnd, TIMERID_HOVER_T, tmpTimeIn, 0);
		fTipTimerActive = TRUE;
	}
}

void ThumbInfo::ThumbSelect(BOOL bMouse)
{
	if ( bMouse )
	{
		bMouseIn = TRUE;
		SetCapture( hwnd );
	}

	SetThumbOpacity( 255 );
}

void ThumbInfo::ThumbDeselect(BOOL bMouse)
{
	if ( bMouse )
	{
		bMouseIn = FALSE;
		ReleaseCapture();
	}
	
	SetThumbOpacity( fcOpt.thumbAlpha );
}

void ThumbInfo::SetThumbOpacity(BYTE bAlpha)
{
	if ( pUpdateLayeredWindow && (bAlpha != btAlpha) )
	{
		btAlpha = bAlpha;
		UpdateContent();
	}
}

void ThumbInfo::KillTip()
{
	if (fTipTimerActive)
	{
		KillTimer(hwnd, TIMERID_HOVER_T);
		fTipTimerActive = FALSE;
	}

	if (fTipActive)
	{
		CallService("mToolTip/HideTip", 0, 0);
		fTipActive = FALSE;
	}
}

void ThumbInfo::UpdateContent()
{
	bmpContent.allocate(szSize.cx, szSize.cy);

	HFONT	hOldFont;
	SIZE	size;
	RECT	rc;
	RECT	rcText;
	DWORD	oldColor;
	int		oldBkMode, index = 0;// nStatus;
	UINT	fStyle = ILD_NORMAL;
	
	HDC		hdcDraw	= bmpContent.getDC();
	SetRect(&rc, 0, 0, szSize.cx, szSize.cy);

	if ( NULL != hBmpBackground )
	{
		RECT rcBkgnd;
		BITMAP bmp;
		HDC hdcBmp;
		HBITMAP hbmTmp;
		int x,y;
		int maxx,maxy;
		int destw,desth;
		int width;
		int height;

		SetRect(&rcBkgnd, 0, 0, szSize.cx, szSize.cy);
		if (NULL != hLTEdgesPen)
			InflateRect(&rcBkgnd, -1, -1);
		width = rcBkgnd.right - rcBkgnd.left;
		height = rcBkgnd.bottom - rcBkgnd.top;

		GetObject(hBmpBackground, sizeof(bmp), &bmp);
		hdcBmp	= CreateCompatibleDC( hdcDraw );
		hbmTmp = (HBITMAP)SelectObject( hdcBmp, hBmpBackground );

		maxx = (0 != (nBackgroundBmpUse & CLBF_TILEH) ? rcBkgnd.right : rcBkgnd.left + 1);
		maxy = (0 != (nBackgroundBmpUse & CLBF_TILEV) ? rcBkgnd.bottom : rcBkgnd.top + 1);
		switch (nBackgroundBmpUse & CLBM_TYPE)
		{
		case CLB_STRETCH:
			if (0 != (nBackgroundBmpUse & CLBF_PROPORTIONAL))
			{
				if (width * bmp.bmHeight < height * bmp.bmWidth)
				{
					desth = height;
					destw = desth * bmp.bmWidth / bmp.bmHeight;
				}
				else
				{
					destw = width;
					desth = destw * bmp.bmHeight / bmp.bmWidth;
				}
			}
			else
			{
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

		for (x = rcBkgnd.left; x < maxx; x += destw)
		{
			for (y = rcBkgnd.top; y < maxy; y += desth)
			{
				StretchBlt( hdcDraw, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY );
			}
		}

		SelectObject( hdcBmp, hbmTmp );
		DeleteDC( hdcBmp );
	}
	else
	{
		FillRect( hdcDraw, &rc, hBkBrush );
	}

	if (NULL != hLTEdgesPen)
	{
		HPEN  hOldPen = (HPEN)SelectObject( hdcDraw, hLTEdgesPen );

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
		//InflateRect(&rc, -1, -1);
	}

	bmpContent.setAlpha(btAlpha);

	ImageList_GetIconSize_my( himl, size );

	oldBkMode	= SetBkMode( hdcDraw, TRANSPARENT );

	if (!DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
	{
		int nStatus;
		int nContactStatus;
		int nApparentMode;
		char* szProto;


		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		
		if ( NULL != szProto )
		{
			nStatus			= CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			nContactStatus	= DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			nApparentMode	= DBGetContactSettingWord(hContact, szProto, "ApparentMode", 0);
			
			if (	(nStatus == ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_ONLINE) ||
					(nStatus != ID_STATUS_INVISIBLE && nApparentMode == ID_STATUS_OFFLINE) )
			{
				if (ID_STATUS_OFFLINE == nContactStatus)
				{
					index = FLT_FONTID_OFFINVIS;
				}
				else
				{
					index =	FLT_FONTID_INVIS;
					if(fcOpt.bShowIdle && DBGetContactSettingDword(hContact, szProto, "IdleTS", 0)){
						fStyle|=ILD_BLEND50;
					}
				}
			}
			else if (ID_STATUS_OFFLINE == nContactStatus)
			{
				index = FLT_FONTID_OFFLINE;
			}
			else
			{
				index = FLT_FONTID_CONTACTS;
				if(fcOpt.bShowIdle && DBGetContactSettingDword(hContact, szProto, "IdleTS", 0)){
					fStyle|=ILD_BLEND50;
				}
			}

		}
	}
	else
	{
		index = FLT_FONTID_NOTONLIST;
		fStyle|=ILD_BLEND50;
	}

	oldColor = SetTextColor( hdcDraw, tColor[ index ] );
	
/*	ImageList_DrawEx(	himl, 
						iIcon, 
						hdcDraw, 
						2, 
						( szSize.cy - size.cx ) / 2, 
						0, 
						0, 
						CLR_NONE, 
						CLR_NONE, 
						fStyle);
*/
	{ 
		HICON icon = ImageList_GetIcon(himl, iIcon, ILD_NORMAL);
		MyBitmap bmptmp(size.cx, size.cy);
		bmptmp.DrawIcon(icon,0,0);//bmpContent
		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = (fStyle&ILD_BLEND50)?128:255;
		blend.AlphaFormat = AC_SRC_ALPHA;
		AlphaBlend(hdcDraw, 2,( szSize.cy - size.cx ) / 2, bmptmp.getWidth(), bmptmp.getHeight(), bmptmp.getDC(), 0, 0, bmptmp.getWidth(), bmptmp.getHeight(), blend);
		DestroyIcon(icon);
	}
	SetRect(&rcText, 0, 0, szSize.cx, szSize.cy);
	rcText.left += size.cx + 4;

	hOldFont = (HFONT)SelectObject( hdcDraw, hFont[ index ] );

	SIZE szText;
	GetTextExtentPoint32(hdcDraw, ptszName, _tcslen(ptszName), &szText);
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
	
	SelectObject( hdcDraw, hOldFont );
	
	SetTextColor( hdcDraw, oldColor );
	SetBkMode( hdcDraw, oldBkMode );

	if (pUpdateLayeredWindow)
	{
		SetWindowLong( hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED );

		RECT rc; GetWindowRect(hwnd, &rc);
		POINT ptDst = {rc.left, rc.top};
		POINT ptSrc = {0, 0};

		BLENDFUNCTION blend;
		blend.BlendOp =             AC_SRC_OVER;
		blend.BlendFlags =          0;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat =         AC_SRC_ALPHA;

		pUpdateLayeredWindow(hwnd, NULL, &ptDst, &szSize, bmpContent.getDC(), &ptSrc, 0xffffffff, &blend, ULW_ALPHA);
	} else
	{
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		UpdateWindow(hwnd);
	}
}

void ThumbInfo::PopUpMessageDialog( )
{
	CallService( MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)hContact, (LPARAM)0 );
}

void ThumbInfo::OnTimer(BYTE idTimer)
{
	if(idTimer == TIMERID_SELECT_T){
		KillTimer( hwnd, TIMERID_SELECT_T );
		ThumbDeselect( FALSE );
	}
	if(idTimer == TIMERID_LEAVE_T && !bMouseDown){
		POINT pt;
		RECT rc;

		GetCursorPos(&pt);
		GetThumbRect(&rc);
		if(!PtInRect(&rc, pt)){
			KillTimer( hwnd, TIMERID_LEAVE_T );
			pThumbMouseIn	= NULL;
			ThumbDeselect( TRUE );
		}
	}
	if(bEnableTip && fcOpt.bShowTip && idTimer == TIMERID_HOVER_T){
		POINT pt;
		CLCINFOTIP ti = {0};
		ti.cbSize = sizeof(ti);

		KillTimer(hwnd, TIMERID_HOVER_T);
		fTipTimerActive = FALSE;
		GetCursorPos(&pt);
		if(abs(pt.x-ptTipSt.x)<5 && abs(pt.y-ptTipSt.y)<5){
			ti.ptCursor = pt;

			fTipActive = TRUE;
			ti.isGroup = 0;
			ti.hItem = hContact;
			ti.isTreeFocused = 0;
			CallService("mToolTip/ShowTip", 0, (LPARAM)&ti);
		}
	}
}

void DockThumbs( ThumbInfo *pThumbLeft, ThumbInfo *pThumbRight, BOOL bMoveLeft )
{
	if ( ( pThumbRight->dockOpt.hwndLeft == NULL ) && ( pThumbLeft->dockOpt.hwndRight == NULL ) )
	{
		pThumbRight->dockOpt.hwndLeft	= pThumbLeft->hwnd;
		pThumbLeft->dockOpt.hwndRight	= pThumbRight->hwnd;
	}
}


void UndockThumbs( ThumbInfo *pThumb1, ThumbInfo *pThumb2 )
{
	if ( ( pThumb1 == NULL ) || ( pThumb2 == NULL ) )
	{
		return;
	}

	if ( pThumb1->dockOpt.hwndRight == pThumb2->hwnd )
	{
		pThumb1->dockOpt.hwndRight = NULL;
	}

	if ( pThumb1->dockOpt.hwndLeft == pThumb2->hwnd )
	{
		pThumb1->dockOpt.hwndLeft = NULL;
	}
	
	if ( pThumb2->dockOpt.hwndRight == pThumb1->hwnd )
	{
		pThumb2->dockOpt.hwndRight = NULL;
	}

	if ( pThumb2->dockOpt.hwndLeft == pThumb1->hwnd )
	{
		pThumb2->dockOpt.hwndLeft = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// ThumbList
ThumbList::ThumbList(): LIST<ThumbInfo>(1, cmp)
{
}

ThumbList::~ThumbList()
{
	for (int i = 0; i < getCount(); ++i)
		delete (*this)[i];
	destroy();
}

ThumbInfo *ThumbList::AddThumb(HWND hwnd, TCHAR *ptszName, HANDLE hContact)
{
	ThumbInfo *pThumb	= NULL;

	if ( ptszName == NULL )	return( NULL );
	if ( hContact == NULL ) return( NULL );
	if ( hwnd == NULL )		return( NULL );

	pThumb = new ThumbInfo;
	 
	if ( pThumb != NULL )
	{
		_tcsncpy( pThumb->ptszName, ptszName, USERNAME_LEN - 1 );
		pThumb->hContact	= hContact;
		pThumb->hwnd		= hwnd;
		
		pThumb->dockOpt.hwndLeft	= NULL;
		pThumb->dockOpt.hwndRight	= NULL;

		pThumb->fTipActive = FALSE;

//		RegHotkey(szName, hwnd);
		RegHotkey(hContact, hwnd);
	}

	insert(pThumb);

	return( pThumb );
}

void ThumbList::RemoveThumb(ThumbInfo *pThumb)
{
	if (!pThumb) return;

	if (fcOpt.bMoveTogether)
	{
		UndockThumbs(pThumb, FindThumb(pThumb->dockOpt.hwndLeft));
		UndockThumbs(pThumb, FindThumb(pThumb->dockOpt.hwndRight));
	}

	remove(pThumb);

	UnregisterFileDropping( pThumb->hwnd );
	DestroyWindow( pThumb->hwnd );
	delete pThumb;
}

ThumbInfo *ThumbList::FindThumb(HWND hwnd)
{
	if (!hwnd) return NULL;

	for (int i = 0; i < getCount(); ++i)
		if ((*this)[i]->hwnd == hwnd)
			return (*this)[i];

	return NULL;
}

ThumbInfo *ThumbList::FindThumbByContact(HANDLE hContact)
{
	if (!hContact) return NULL;

	for (int i = 0; i < getCount(); ++i)
		if ((*this)[i]->hContact == hContact)
			return (*this)[i];

	return NULL;
}

int ThumbList::cmp(const ThumbInfo *p1, const ThumbInfo *p2)
{
	if ((DWORD)p1->hContact < (DWORD)p2->hContact) return -1;
	if ((DWORD)p1->hContact > (DWORD)p2->hContact) return +1;
	return 0;
}
