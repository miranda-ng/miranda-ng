////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

#define MAX_LOADSTRING 100
#define HIMETRIC_INCH 2540
#define MAP_LOGHIM_TO_PIX(x,ppli) ( ((ppli)*(x) + HIMETRIC_INCH/2) / HIMETRIC_INCH )

////////////////////////////////////////////////////////////////////////////////
// User Util Dlg Page : Data

typedef struct _GGTOKENDLGDATA
{
	int width;
	int height;
	char id[256];
	char val[256];
	HBITMAP hBitmap;
} GGTOKENDLGDATA;

INT_PTR CALLBACK gg_tokendlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GGTOKENDLGDATA *dat = (GGTOKENDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			RECT rc;
			TranslateDialogDefault(hwndDlg);
			GetClientRect(GetDlgItem(hwndDlg, IDC_WHITERECT), &rc);
			InvalidateRect(hwndDlg, &rc, TRUE);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			return TRUE;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					GetDlgItemTextA(hwndDlg, IDC_TOKEN, dat->val, SIZEOF(dat->val));
					EndDialog(hwndDlg, IDOK);
					break;
				}
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					break;
			}
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC hdc = BeginPaint(hwndDlg, &paintStruct);
			RECT rc; GetClientRect(GetDlgItem(hwndDlg, IDC_WHITERECT), &rc);
			FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

			if (dat && dat->hBitmap)
			{
				HDC hdcBmp = NULL;
				int nWidth, nHeight;
				BITMAP bmp;

				GetObject(dat->hBitmap, sizeof(bmp), &bmp);
				nWidth = bmp.bmWidth; nHeight = bmp.bmHeight;

				if (hdcBmp = CreateCompatibleDC(hdc))
				{
					SelectObject(hdcBmp, dat->hBitmap);
					SetStretchBltMode(hdc, HALFTONE);
					BitBlt(hdc,
						(rc.left + rc.right - nWidth) / 2,
						(rc.top + rc.bottom - nHeight) / 2,
						nWidth, nHeight,
						hdcBmp, 0, 0, SRCCOPY);
					DeleteDC(hdcBmp);
				}
			}
			EndPaint(hwndDlg, &paintStruct);
			return 0;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Gets GG token
int GGPROTO::gettoken(GGTOKEN *token)
{
	struct gg_http *h = NULL;
	struct gg_token *t = NULL;
	IMGSRVC_MEMIO memio = {0};
	GGTOKENDLGDATA dat = {0};

	// Zero tokens
	mir_strcpy(token->id, "");
	mir_strcpy(token->val, "");

	if (!(h = gg_token(0)) || gg_token_watch_fd(h) || h->state == GG_STATE_ERROR || h->state != GG_STATE_DONE) {
		TCHAR error[128];
		mir_sntprintf(error, SIZEOF(error), TranslateT("Token retrieval failed because of error:\n\t%s"), http_error_string(h ? h->error : 0));
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		gg_free_pubdir(h);
		return FALSE;
	}

	if (!(t = (struct gg_token *)h->data) || (!h->body)) {
		TCHAR error[128];
		mir_sntprintf(error, SIZEOF(error), TranslateT("Token retrieval failed because of error:\n\t%s"), http_error_string(h ? h->error : 0));
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		gg_free_pubdir(h);
		return FALSE;
	}

	// Return token id
	strncpy(dat.id, t->tokenid, sizeof(dat.id));
	dat.width = t->width;
	dat.height = t->height;

	// Load bitmap
	memio.iLen = h->body_size;
	memio.pBuf = (void *)h->body;
	memio.fif = FIF_UNKNOWN; /* detect */
	memio.flags = 0;
	dat.hBitmap = (HBITMAP) CallService(MS_IMG_LOADFROMMEM, (WPARAM) &memio, 0);
	if (dat.hBitmap == NULL)
	{
		MessageBox(NULL, TranslateT("Could not load token image."), m_tszUserName, MB_OK | MB_ICONSTOP);
		gg_free_pubdir(h);
		return FALSE;
	}

	// Load token dialog
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_TOKEN), NULL, gg_tokendlgproc, (LPARAM)&dat) == IDCANCEL)
		return FALSE;

	// Fillup patterns
	strncpy(token->id, dat.id, sizeof(token->id));
	strncpy(token->val, dat.val, sizeof(token->val));

	return TRUE;
}
