////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Bia³ek
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
#include <io.h>

#define WM_ADDIMAGE  WM_USER + 1
#define WM_SENDIMG	 WM_USER + 2
#define WM_CHOOSEIMG WM_USER + 3
#define TIMERID_FLASHWND WM_USER + 4

////////////////////////////////////////////////////////////////////////////
// Image Window : Data

// tablica zawiera uin kolesia i uchwyt do okna, okno zawiera GGIMAGEDLGDATA
// ktore przechowuje handle kontaktu, i wskaznik na pierwszy  obrazek
// obrazki sa poukladane jako lista jednokierunkowa.
// przy tworzeniu okna podaje sie handle do kontaktu
// dodajac obrazek tworzy sie element listy i wysyla do okna
// wyswietlajac obrazek idzie po liscie jednokierunkowej

typedef struct _GGIMAGEENTRY
{
	HBITMAP hBitmap;
	char *lpszFileName;
	char *lpData;
	unsigned long nSize;
	struct _GGIMAGEENTRY *lpNext;
	uint32_t crc32;
} GGIMAGEENTRY;

typedef struct
{
	HANDLE hContact;
	HANDLE hEvent;
	HWND hWnd;
	uin_t uin;
	int nImg, nImgTotal;
	GGIMAGEENTRY *lpImages;
	SIZE minSize;
	BOOL bReceiving;
	GGPROTO *gg;
} GGIMAGEDLGDATA;

// Prototypes
int gg_img_remove(GGIMAGEDLGDATA *dat);
INT_PTR gg_img_sendimg(GGPROTO *gg, WPARAM wParam, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////
// Image Module : Adding item to contact menu, creating sync objects
int gg_img_init(GGPROTO *gg)
{
	CLISTMENUITEM mi = {0};
	char service[64];

	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB;

	// Send image contact menu item
	mir_snprintf(service, sizeof(service), GGS_SENDIMAGE, GG_PROTO);
	CreateProtoServiceFunction(service, gg_img_sendimg, gg);
	mi.position = -2000010000;
	mi.icolibItem = GetIconHandle(IDI_IMAGE);
	mi.pszName = LPGEN("&Image");
	mi.pszService = service;
	mi.pszContactOwner = GG_PROTO;
	gg->hImageMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	// Receive image
	mir_snprintf(service, sizeof(service), GGS_RECVIMAGE, GG_PROTO);
	CreateProtoServiceFunction(service, gg_img_recvimage, gg);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Image Module : closing dialogs, sync objects
int gg_img_shutdown(GGPROTO *gg)
{
	list_t l;
#ifdef DEBUGMODE
	gg_netlog(gg, "gg_img_shutdown(): Closing all dialogs...");
#endif
	// Rather destroy window instead of just removing structures
	for (l = gg->imagedlgs; l;)
	{
		GGIMAGEDLGDATA *dat = (GGIMAGEDLGDATA *)l->data;
		l = l->next;

		if (dat && dat->hWnd)
		{
			if (IsWindow(dat->hWnd))
			{
				// Post message async, since it maybe be different thread
				if (!PostMessage(dat->hWnd, WM_CLOSE, 0, 0))
					gg_netlog(gg, "gg_img_shutdown(): Image dlg %x cannot be released !!", dat->hWnd);
			}
			else
				gg_netlog(gg, "gg_img_shutdown(): Image dlg %x not exists, but structure does !!", dat->hWnd);
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Image Module : destroying list
int gg_img_destroy(GGPROTO *gg)
{
	// Release all dialogs
	while (gg->imagedlgs && gg_img_remove((GGIMAGEDLGDATA *)gg->imagedlgs->data));

	// Destroy list
	list_destroy(gg->imagedlgs, 1);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)gg->hImageMenuItem, (LPARAM) 0);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Painting image
int gg_img_paint(HWND hwnd, GGIMAGEENTRY *dat)
{
	PAINTSTRUCT paintStruct;
	HDC hdc = BeginPaint(hwnd, &paintStruct);
	RECT rc;

	GetWindowRect(GetDlgItem(hwnd, IDC_IMG_IMAGE), &rc);
	ScreenToClient(hwnd, (POINT *)&rc.left);
	ScreenToClient(hwnd, (POINT *)&rc.right);
	FillRect(hdc, &rc, (HBRUSH)GetSysColorBrush(COLOR_WINDOW));

	if (dat->hBitmap)
	{
		HDC hdcBmp = NULL;
		int nWidth, nHeight;
		BITMAP bmp;

		GetObject(dat->hBitmap, sizeof(bmp), &bmp);
		nWidth = bmp.bmWidth; nHeight = bmp.bmHeight;

		hdcBmp = CreateCompatibleDC(hdc);
		SelectObject(hdcBmp, dat->hBitmap);
		if (hdcBmp)
		{
			SetStretchBltMode(hdc, HALFTONE);
			// Draw bitmap
			if (nWidth > (rc.right-rc.left) || nHeight > (rc.bottom-rc.top))
			{
				if ((double)nWidth / (double)nHeight > (double) (rc.right-rc.left) / (double)(rc.bottom-rc.top))
				{
					StretchBlt(hdc,
						rc.left,
						((rc.top + rc.bottom) - (rc.right - rc.left) * nHeight / nWidth) / 2,
						(rc.right - rc.left),
						(rc.right - rc.left) * nHeight / nWidth,
						hdcBmp, 0, 0, nWidth, nHeight, SRCCOPY);
				}
				else
				{
					StretchBlt(hdc,
						((rc.left + rc.right) - (rc.bottom - rc.top) * nWidth / nHeight) / 2,
						rc.top,
						(rc.bottom - rc.top) * nWidth / nHeight,
						(rc.bottom - rc.top),
						hdcBmp, 0, 0, nWidth, nHeight, SRCCOPY);
				}
			}
			else
			{
				BitBlt(hdc,
					(rc.left + rc.right - nWidth) / 2,
					(rc.top + rc.bottom - nHeight) / 2,
					nWidth, nHeight,
					hdcBmp, 0, 0, SRCCOPY);
			}
			DeleteDC(hdcBmp);
		}
	}
	EndPaint(hwnd, &paintStruct);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Returns supported image filters
char *gg_img_getfilter(char *szFilter, int nSize)
{
	char *szFilterName, *szFilterMask;
	char *pFilter = szFilter;

	// Match relative to ImgDecoder presence
	szFilterName = Translate("Image files (*.bmp,*.gif,*.jpeg,*.jpg,*.png)");
	szFilterMask = "*.bmp;*.gif;*.jpeg;*.jpg;*.png";

	// Make up filter
	strncpy(pFilter, szFilterName, nSize);
	pFilter += strlen(pFilter) + 1;
	if (pFilter >= szFilter + nSize) return NULL;
	strncpy(pFilter, szFilterMask, nSize - (pFilter - szFilter));
	pFilter += strlen(pFilter) + 1;
	if (pFilter >= szFilter + nSize) return NULL;
	*pFilter = 0;

	return szFilter;
}

////////////////////////////////////////////////////////////////////////////////
// Save specified image entry
int gg_img_saveimage(HWND hwnd, GGIMAGEENTRY *dat)
{
	OPENFILENAME ofn = {0};
	char szFileName[MAX_PATH];
	char szFilter[128];

	if (!dat) return FALSE;

	gg_img_getfilter(szFilter, sizeof(szFilter));
	strncpy(szFileName, dat->lpszFileName, sizeof(szFileName));
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFilter = szFilter;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&ofn))
	{
		FILE *fp = fopen(szFileName, "w+b");
		if (fp)
		{
			fwrite(dat->lpData, dat->nSize, 1, fp);
			fclose(fp);
			gg_netlog(((GGIMAGEDLGDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->gg, "gg_img_saveimage(): Image saved to %s.", szFileName);
		}
		else
		{
			gg_netlog(((GGIMAGEDLGDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->gg, "gg_img_saveimage(): Cannot save image to %s.", szFileName);
			MessageBox(hwnd, Translate("Image cannot be written to disk."), ((GGIMAGEDLGDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->gg->name, MB_OK | MB_ICONERROR);
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
// Fit window size to image size
BOOL gg_img_fit(HWND hwndDlg)
{
	GGIMAGEDLGDATA *dat = (GGIMAGEDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	RECT dlgRect, imgRect, wrkRect;
	int nWidth, nHeight;
	int rWidth = 0, rHeight = 0;
	int oWidth = 0, oHeight = 0;
	BITMAP bmp;
	GGIMAGEENTRY *img = NULL;
	HDC hdc;

	// Check if image is loaded
	if (!dat || !dat->lpImages || !dat->lpImages->hBitmap)
		return FALSE;

	img = dat->lpImages;

	// Go to last image
	while (img->lpNext && dat->lpImages->hBitmap)
		img = img->lpNext;

	// Get rects of display
	GetWindowRect(hwndDlg, &dlgRect);
	GetClientRect(GetDlgItem(hwndDlg, IDC_IMG_IMAGE), &imgRect);

	hdc = GetDC(hwndDlg);

	GetObject(img->hBitmap, sizeof(bmp), &bmp);
	nWidth = bmp.bmWidth; nHeight = bmp.bmHeight;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &wrkRect, 0);

	ReleaseDC(hwndDlg, hdc);

	if ((imgRect.right - imgRect.left) < nWidth)
		rWidth = nWidth - imgRect.right + imgRect.left;
	if ((imgRect.bottom - imgRect.top) < nWidth)
		rHeight = nHeight - imgRect.bottom + imgRect.top;

	// Check if anything needs resize
	if (!rWidth && !rHeight)
		return FALSE;

	oWidth = dlgRect.right - dlgRect.left + rWidth;
	oHeight = dlgRect.bottom - dlgRect.top + rHeight;

	if (oHeight > wrkRect.bottom - wrkRect.top)
	{
		oWidth = (int)((double)(wrkRect.bottom - wrkRect.top + imgRect.bottom - imgRect.top - dlgRect.bottom + dlgRect.top) * nWidth / nHeight)
			- imgRect.right + imgRect.left + dlgRect.right - dlgRect.left;
		if (oWidth < dlgRect.right - dlgRect.left)
			oWidth = dlgRect.right - dlgRect.left;
		oHeight = wrkRect.bottom - wrkRect.top;
	}
	if (oWidth > wrkRect.right - wrkRect.left)
	{
		oHeight = (int)((double)(wrkRect.right - wrkRect.left + imgRect.right - imgRect.left - dlgRect.right + dlgRect.left) * nHeight / nWidth)
			- imgRect.bottom + imgRect.top + dlgRect.bottom - dlgRect.top;
		if (oHeight < dlgRect.bottom - dlgRect.top)
			oHeight = dlgRect.bottom - dlgRect.top;
		oWidth = wrkRect.right - wrkRect.left;
	}
	SetWindowPos(hwndDlg, NULL,
		(wrkRect.left + wrkRect.right - oWidth) / 2,
		(wrkRect.top + wrkRect.bottom - oHeight) / 2,
		oWidth, oHeight,
		SWP_SHOWWINDOW | SWP_NOZORDER /* | SWP_NOACTIVATE */);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Dialog resizer procedure
static int sttImageDlgResizer(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL* urc)
{
	switch (urc->wId)
	{
		case IDC_IMG_PREV:
		case IDC_IMG_NEXT:
		case IDC_IMG_DELETE:
		case IDC_IMG_SAVE:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;
		case IDC_IMG_IMAGE:
			return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT | RD_ANCHORX_WIDTH;
		case IDC_IMG_SEND:
		case IDC_IMG_CANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

////////////////////////////////////////////////////////////////////////////
// Send / Recv main dialog procedure
static INT_PTR CALLBACK gg_img_dlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GGIMAGEDLGDATA *dat = (GGIMAGEDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
			{
				char *szName, szTitle[128];
				RECT rect;

				TranslateDialogDefault(hwndDlg);
				// This should be already initialized
				// InitCommonControls();

				// Get dialog data
				dat = (GGIMAGEDLGDATA *)lParam;
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

				// Save dialog handle
				dat->hWnd = hwndDlg;

				// Send event if someone's waiting
				if (dat->hEvent) SetEvent(dat->hEvent);
				else gg_netlog(dat->gg, "gg_img_dlgproc(): Creation event not found, but someone might be waiting.");

				// Making buttons flat
				SendDlgItemMessage(hwndDlg, IDC_IMG_PREV,	BUTTONSETASFLATBTN, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_IMG_NEXT,	BUTTONSETASFLATBTN, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_IMG_DELETE,	BUTTONSETASFLATBTN, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_IMG_SAVE,	BUTTONSETASFLATBTN, 0, 0);

				// Setting images for buttons
				SendDlgItemMessage(hwndDlg, IDC_IMG_PREV,	BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx("previous", FALSE));
				SendDlgItemMessage(hwndDlg, IDC_IMG_NEXT,	BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx("next", FALSE));
				SendDlgItemMessage(hwndDlg, IDC_IMG_DELETE,	BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx("delete", FALSE));
				SendDlgItemMessage(hwndDlg, IDC_IMG_SAVE,	BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx("save", FALSE));

				// Setting tooltips for buttons
				SendDlgItemMessage(hwndDlg, IDC_IMG_PREV,	BUTTONADDTOOLTIP, (WPARAM)TranslateT("Previous image"), BATF_TCHAR);
				SendDlgItemMessage(hwndDlg, IDC_IMG_NEXT,	BUTTONADDTOOLTIP, (WPARAM)TranslateT("Next image"), BATF_TCHAR);
				SendDlgItemMessage(hwndDlg, IDC_IMG_DELETE,	BUTTONADDTOOLTIP, (WPARAM)TranslateT("Delete image from the list"), BATF_TCHAR);
				SendDlgItemMessage(hwndDlg, IDC_IMG_SAVE,	BUTTONADDTOOLTIP, (WPARAM)TranslateT("Save image to disk"), BATF_TCHAR);

				// Set main window image
				WindowSetIcon(hwndDlg, "image");

				szName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)dat->hContact, 0);
				if (dat->bReceiving)
					mir_snprintf(szTitle, sizeof(szTitle), Translate("Image from %s"), szName);
				else
					mir_snprintf(szTitle, sizeof(szTitle), Translate("Image for %s"), szName);
				SetWindowText(hwndDlg, szTitle);

				// Store client extents
				GetClientRect(hwndDlg, &rect);
				dat->minSize.cx = rect.right - rect.left;
				dat->minSize.cy = rect.bottom - rect.top;
			}
			return TRUE;

		case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = {0};
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInstance;
			urd.hwndDlg = hwndDlg;
			urd.lpTemplate = dat->bReceiving ? MAKEINTRESOURCEA(IDD_IMAGE_RECV) : MAKEINTRESOURCEA(IDD_IMAGE_SEND);
			urd.pfnResizer = sttImageDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
				InvalidateRect(hwndDlg, NULL, FALSE);
			return 0;
		}

		case WM_SIZING:
			{
				RECT *pRect = (RECT *)lParam;
				if (pRect->right - pRect->left < dat->minSize.cx)
				{
					if (wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT)
						pRect->left = pRect->right - dat->minSize.cx;
					else
						pRect->right = pRect->left + dat->minSize.cx;
				}
				if (pRect->bottom - pRect->top < dat->minSize.cy)
				{
					if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOP || wParam == WMSZ_TOPRIGHT)
						pRect->top = pRect->bottom - dat->minSize.cy;
					else
						pRect->bottom = pRect->top + dat->minSize.cy;
				}
			}
			return TRUE;

		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			break;

		// Flash the window
		case WM_TIMER:
			if (wParam == TIMERID_FLASHWND)
				FlashWindow(hwndDlg, TRUE);
			break;

		// Kill the timer
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_ACTIVE)
				break;
		case WM_MOUSEACTIVATE:
			if (KillTimer(hwndDlg, TIMERID_FLASHWND))
				FlashWindow(hwndDlg, FALSE);
			break;

		case WM_PAINT:
			if (dat->lpImages)
			{
				GGIMAGEENTRY *img = dat->lpImages;
				int i;

				for (i = 1; img && (i < dat->nImg); i++)
					img = img->lpNext;

				if (!img)
				{
					gg_netlog(dat->gg, "gg_img_dlgproc(): Image was not found on the list. Cannot paint the window.");
					return FALSE;
				}

				if (dat->bReceiving)
				{
					char szTitle[128];
					mir_snprintf(szTitle, sizeof(szTitle),
						"%s (%d / %d)", img->lpszFileName, dat->nImg, dat->nImgTotal);
					SetDlgItemText(hwndDlg, IDC_IMG_NAME, szTitle);
				}
				else
					SetDlgItemText(hwndDlg, IDC_IMG_NAME, img->lpszFileName);
				gg_img_paint(hwndDlg, img);
			}
			break;

		case WM_DESTROY:
			if (dat)
			{
				// Deleting all image entries
				GGIMAGEENTRY *temp, *img = dat->lpImages;
				GGPROTO *gg = dat->gg;
				while (temp = img)
				{
					img = img->lpNext;
					gg_img_releasepicture(temp);
				}
				ReleaseIconEx("previous", FALSE);
				ReleaseIconEx("next", FALSE);
				ReleaseIconEx("delete", FALSE);
				ReleaseIconEx("save", FALSE);
				WindowFreeIcon(hwndDlg);
				EnterCriticalSection(&gg->img_mutex);
				list_remove(&gg->imagedlgs, dat, 1);
				LeaveCriticalSection(&gg->img_mutex);
			}
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_IMG_CANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;

				case IDC_IMG_PREV:
					if (dat->nImg > 1)
					{
						dat->nImg--;
						InvalidateRect(hwndDlg, NULL, FALSE);
					}
					return TRUE;

				case IDC_IMG_NEXT:
					if (dat->nImg < dat->nImgTotal)
					{
						dat->nImg++;
						InvalidateRect(hwndDlg, NULL, FALSE);
					}
					return TRUE;

				case IDC_IMG_DELETE:
					{
						GGIMAGEENTRY *del, *img = dat->lpImages;
						if (dat->nImg == 1)
						{
							del = dat->lpImages;
							dat->lpImages = img->lpNext;
						}
						else
						{
							int i;
							for (i = 1; img && (i < dat->nImg - 1); i++)
								img = img->lpNext;
							if (!img)
							{
								gg_netlog(dat->gg, "gg_img_dlgproc(): Image was not found on the list. Cannot delete it from the list.");
								return FALSE;
							}
							del = img->lpNext;
							img->lpNext = del->lpNext;
							dat->nImg --;
						}

						if ((-- dat->nImgTotal) == 0)
							EndDialog(hwndDlg, 0);
						else
							InvalidateRect(hwndDlg, NULL, FALSE);

						gg_img_releasepicture(del);
					}
					return TRUE;

				case IDC_IMG_SAVE:
					{
						GGIMAGEENTRY *img = dat->lpImages;
						int i;

						for (i = 1; img && (i < dat->nImg); i++)
							img = img->lpNext;
						if (!img)
						{
							gg_netlog(dat->gg, "gg_img_dlgproc(): Image was not found on the list. Cannot launch saving.");
							return FALSE;
						}
						gg_img_saveimage(hwndDlg, img);
					}
					return TRUE;

				case IDC_IMG_SEND:
					{
						unsigned char format[20];
						char *msg = "\xA0\0";
						GGPROTO *gg = dat->gg;

						if (dat->lpImages && gg_isonline(gg))
						{
							uin_t uin = (uin_t)DBGetContactSettingDword(dat->hContact, gg->proto.m_szModuleName, GG_KEY_UIN, 0);
							struct gg_msg_richtext_format *r = NULL;
							struct gg_msg_richtext_image *p = NULL;
							LPVOID pvData = NULL;
							int len;

							((struct gg_msg_richtext*)format)->flag = 2;

							r = (struct gg_msg_richtext_format *)(format + sizeof(struct gg_msg_richtext));
							r->position = 0;
							r->font = GG_FONT_IMAGE;

							p = (struct gg_msg_richtext_image *)(format + sizeof(struct gg_msg_richtext) + sizeof(struct gg_msg_richtext_format));
							p->unknown1 = 0x109;
							p->size = dat->lpImages->nSize;

							dat->lpImages->crc32 = p->crc32 = gg_fix32(gg_crc32(0, dat->lpImages->lpData, dat->lpImages->nSize));

							len = sizeof(struct gg_msg_richtext_format) + sizeof(struct gg_msg_richtext_image);
							((struct gg_msg_richtext*)format)->length = len;

							EnterCriticalSection(&gg->sess_mutex);
							gg_send_message_richtext(gg->sess, GG_CLASS_CHAT, (uin_t)uin, (unsigned char*)msg, format, len + sizeof(struct gg_msg_richtext));
							LeaveCriticalSection(&gg->sess_mutex);

							// Protect dat from releasing
							SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)0);

							EndDialog(hwndDlg, 0);
						}
						return TRUE;
					}
					break;
			}
			break;

		case WM_ADDIMAGE: // lParam == GGIMAGEENTRY *dat
			{
				GGIMAGEENTRY *lpImage = (GGIMAGEENTRY *)lParam;
				GGIMAGEENTRY *lpImages = dat->lpImages;

				if (!dat->lpImages) // first image entry
					dat->lpImages = lpImage;
				else // adding at the end of the list
				{
					while (lpImages->lpNext)
						lpImages = lpImages->lpNext;
					lpImages->lpNext = lpImage;
				}
				dat->nImg = ++ dat->nImgTotal;
			}
			// Fit window to image
			if (!gg_img_fit(hwndDlg))
				InvalidateRect(hwndDlg, NULL, FALSE);
			return TRUE;

		case WM_CHOOSEIMG:
		{
			char szFilter[128];
			char szFileName[MAX_PATH];
			OPENFILENAME ofn = {0};

			gg_img_getfilter(szFilter, sizeof(szFilter));
			*szFileName = 0;
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.hInstance = hInstance;
			ofn.lpstrFilter = szFilter;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = Translate("Select picture to send");
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
			if (GetOpenFileName(&ofn))
			{
				if (dat->lpImages)
					gg_img_releasepicture(dat->lpImages);
				if (!(dat->lpImages = (GGIMAGEENTRY *)gg_img_loadpicture(dat->gg, 0, szFileName)))
				{
					EndDialog(hwndDlg, 0);
					return FALSE;
				}
				if (!gg_img_fit(hwndDlg))
					InvalidateRect(hwndDlg, NULL, FALSE);
			}
			else
			{
				EndDialog(hwndDlg, 0);
				return FALSE;
			}
			return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Image dialog call thread
void __cdecl gg_img_dlgcallthread(GGPROTO *gg, void *param)
{
	HWND hMIWnd = 0; //(HWND) CallService(MS_CLUI_GETHWND, 0, 0);

	GGIMAGEDLGDATA *dat = (GGIMAGEDLGDATA *)param;
	DialogBoxParam(hInstance, dat->bReceiving ? MAKEINTRESOURCE(IDD_IMAGE_RECV) : MAKEINTRESOURCE(IDD_IMAGE_SEND),
		hMIWnd, gg_img_dlgproc, (LPARAM) dat);
}

////////////////////////////////////////////////////////////////////////////
// Open dialog receive for specified contact
GGIMAGEDLGDATA *gg_img_recvdlg(GGPROTO *gg, HANDLE hContact)
{
	// Create dialog data
	GGIMAGEDLGDATA *dat = (GGIMAGEDLGDATA *)calloc(1, sizeof(GGIMAGEDLGDATA));
	dat->hContact = hContact;
	dat->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	dat->bReceiving = TRUE;
	dat->gg = gg;
	ResetEvent(dat->hEvent);
	gg_forkthread(gg, gg_img_dlgcallthread, dat);
	return dat;
}

////////////////////////////////////////////////////////////////////////////
// Checks if an image is already saved to the specified path
// Returns 1 if yes, 0 if no or -1 if different image on this path is found
int gg_img_isexists(char *szPath, GGIMAGEENTRY *dat)
{
	struct _stat st;

	if (_stat(szPath, &st) != 0)
		return 0;

	if (st.st_size == dat->nSize)
	{
		char *lpData;
		FILE *fp = fopen(szPath, "rb");
		if (!fp) return 0;
		lpData = mir_alloc(dat->nSize);
		if (fread(lpData, 1, dat->nSize, fp) == dat->nSize)
		{
			if (dat->crc32 == gg_fix32(gg_crc32(0, lpData, dat->nSize)) ||
				memcmp(lpData, dat->lpData, dat->nSize) == 0)
			{
				mir_free(lpData);
				fclose(fp);
				return 1;
			}
		}
		mir_free(lpData);
		fclose(fp);
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////
// Determine if image's file name has the proper extension
char *gg_img_hasextension(const char *filename)
{
	if (filename != NULL && *filename != '\0')
	{
		char *imgtype = strrchr(filename, '.');
		if (imgtype != NULL)
		{
			size_t len = strlen(imgtype);
			imgtype++;
			if (len == 4 && (_stricmp(imgtype, "bmp") == 0 ||
							 _stricmp(imgtype, "gif") == 0 ||
							 _stricmp(imgtype, "jpg") == 0 ||
							 _stricmp(imgtype, "png") == 0))
				return --imgtype;
			if (len == 5 &&  _stricmp(imgtype, "jpeg") == 0)
				return --imgtype;
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Display received image using message with [img] BBCode
int gg_img_displayasmsg(GGPROTO *gg, HANDLE hContact, void *img)
{
	GGIMAGEENTRY *dat = (GGIMAGEENTRY *)img;
	char szPath[MAX_PATH], *path = (char*)alloca(MAX_PATH), *pImgext, imgext[6];
	size_t tPathLen;
	int i, res;

	if (gg->hImagesFolder == NULL || FoldersGetCustomPath(gg->hImagesFolder, path, MAX_PATH, ""))
	{
		char *tmpPath = Utils_ReplaceVars("%miranda_userdata%");
		tPathLen = mir_snprintf(szPath, MAX_PATH, "%s\\%s\\ImageCache", tmpPath, GG_PROTO);
		mir_free(tmpPath);
	}
	else
	{
		strcpy(szPath, path);
		tPathLen = strlen(szPath);
	}

	if (_access(szPath, 0))
		CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)szPath);

	mir_snprintf(szPath + tPathLen, MAX_PATH - tPathLen, "\\%s", dat->lpszFileName);
	if ((pImgext = gg_img_hasextension(szPath)) == NULL)
		pImgext = szPath + strlen(szPath);
	mir_snprintf(imgext, SIZEOF(imgext), "%s", pImgext);
	for (i = 1; ; ++i)
	{
		if ((res = gg_img_isexists(szPath, dat)) != -1) break;
		mir_snprintf(szPath, MAX_PATH, "%.*s (%u)%s", pImgext - szPath, szPath, i, imgext);
	}

	if (res == 0)
	{
		// Image file not found, thus create it
		FILE *fp = fopen(szPath, "w+b");
		if (fp)
		{
			res = fwrite(dat->lpData, dat->nSize, 1, fp) > 0;
			fclose(fp);
		}
	}

	if (res != 0)
	{
		char image_msg[MAX_PATH + 11];
		CCSDATA ccs = {0};
		PROTORECVEVENT pre = {0};

		ccs.szProtoService = PSR_MESSAGE;
		ccs.hContact = hContact;
		ccs.lParam = (LPARAM)&pre;
		mir_snprintf(image_msg, SIZEOF(image_msg), "[img]%s[/img]", szPath);
		pre.timestamp = time(NULL);
		pre.szMessage = image_msg;
		CallService(MS_PROTO_CHAINRECV, 0, (LPARAM) &ccs);
		gg_netlog(gg, "gg_img_displayasmsg: Image saved to %s.", szPath);
	}
	else
	{
		gg_netlog(gg, "gg_img_displayasmsg: Cannot save image to %s.", szPath);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
// Return if uin has it's window already opened
BOOL gg_img_opened(GGPROTO *gg, uin_t uin)
{
	list_t l = gg->imagedlgs;
	while (l)
	{
		GGIMAGEDLGDATA *dat = (GGIMAGEDLGDATA *)l->data;
		if (dat->uin == uin)
			return TRUE;
		l = l->next;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Image Module : Looking for window entry, create if not found
gg_img_display(GGPROTO *gg, HANDLE hContact, void *img)
{
	list_t l = gg->imagedlgs;
	GGIMAGEDLGDATA *dat;

	if (!img) return FALSE;

	// Look for already open dialog
	EnterCriticalSection(&gg->img_mutex);
	while (l)
	{
		dat = (GGIMAGEDLGDATA *)l->data;
		if (dat->bReceiving && dat->hContact == hContact)
			break;
		l = l->next;
	}

	if (!l) dat = NULL;

	if (!dat)
	{
		dat = gg_img_recvdlg(gg, hContact);
		dat->uin = DBGetContactSettingDword(hContact, gg->proto.m_szModuleName, GG_KEY_UIN, 0);

		while (WaitForSingleObjectEx(dat->hEvent, INFINITE, TRUE) != WAIT_OBJECT_0);
		CloseHandle(dat->hEvent);
		dat->hEvent = NULL;

		list_add(&gg->imagedlgs, dat, 0);
	}
	LeaveCriticalSection(&gg->img_mutex);

	SendMessage(dat->hWnd, WM_ADDIMAGE, 0, (LPARAM)img);
	if (/*DBGetContactSettingByte(NULL, "Chat", "FlashWindowHighlight", 0) != 0 && */
		GetActiveWindow() != dat->hWnd && GetForegroundWindow() != dat->hWnd)
		SetTimer(dat->hWnd, TIMERID_FLASHWND, 900, NULL);

	/* DEPRECATED: No more grabbing the focus... just flashing
	SetForegroundWindow(dat->hWnd);
	SetFocus(dat->hWnd);
	*/

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Image Window : Frees image entry structure
gg_img_releasepicture(void *img)
{
	if (!img)
		return FALSE;
	if (((GGIMAGEENTRY *)img)->lpszFileName)
		free(((GGIMAGEENTRY *)img)->lpszFileName);
	if (((GGIMAGEENTRY *)img)->hBitmap)
		DeleteObject(((GGIMAGEENTRY *)img)->hBitmap);
	if (((GGIMAGEENTRY *)img)->lpData)
		free(((GGIMAGEENTRY *)img)->lpData);
	free(img);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Helper function to determine image file format and the right extension
const char *gg_img_guessfileextension(const char *lpData)
{
	if (lpData != NULL)
	{
		if (memcmp(lpData, "BM", 2) == 0)
			return ".bmp";
		if (memcmp(lpData, "GIF8", 4) == 0)
			return ".gif";
		if (memcmp(lpData, "\xFF\xD8", 2) == 0)
			return ".jpg";
		if (memcmp(lpData, "\x89PNG", 4) == 0)
			return ".png";
	}
	return "";
}

////////////////////////////////////////////////////////////////////////////
// Image Window : Loading picture and sending for display
void *gg_img_loadpicture(GGPROTO *gg, struct gg_event* e, char *szFileName)
{
	GGIMAGEENTRY *dat;

	if (!szFileName &&
		(!e || !e->event.image_reply.size || !e->event.image_reply.image || !e->event.image_reply.filename))
		return NULL;

	dat = (GGIMAGEENTRY *)calloc(1, sizeof(GGIMAGEENTRY));
	if (dat == NULL)
		return NULL;

	// Copy the file name
	if (szFileName)
	{
		FILE *fp = fopen(szFileName, "rb");
		if (!fp)
		{
			free(dat);
			gg_netlog(gg, "gg_img_loadpicture(): fopen(\"%s\", \"rb\") failed.", szFileName);
			return NULL;
		}
		fseek(fp, 0, SEEK_END);
		dat->nSize = ftell(fp);
		if (dat->nSize <= 0)
		{
			fclose(fp);
			free(dat);
			gg_netlog(gg, "gg_img_loadpicture(): Zero file size \"%s\" failed.", szFileName);
			return NULL;
		}
		// Maximum acceptable image size
		if (dat->nSize > 255 * 1024)
		{
			fclose(fp);
			free(dat);
			gg_netlog(gg, "gg_img_loadpicture(): Image size of \"%s\" exceeds 255 KB.", szFileName);
			MessageBox(NULL, Translate("Image exceeds maximum allowed size of 255 KB."), GG_PROTONAME, MB_OK | MB_ICONEXCLAMATION);
			return NULL;
		}
		fseek(fp, 0, SEEK_SET);
		dat->lpData = malloc(dat->nSize);
		if (fread(dat->lpData, 1, dat->nSize, fp) < dat->nSize)
		{
			free(dat->lpData);
			fclose(fp);
			free(dat);
			gg_netlog(gg, "gg_img_loadpicture(): Reading file \"%s\" failed.", szFileName);
			return NULL;
		}
		fclose(fp);
		dat->lpszFileName = _strdup(szFileName);
	}
	// Copy picture from packet
	else if (e && e->event.image_reply.filename)
	{
		dat->nSize = e->event.image_reply.size;
		dat->lpData = malloc(dat->nSize);
		memcpy(dat->lpData, e->event.image_reply.image, dat->nSize);

		if (!gg_img_hasextension(e->event.image_reply.filename))
		{
			// Add missing file extension
			const char *szImgType = gg_img_guessfileextension(dat->lpData);
			if (*szImgType)
			{
				dat->lpszFileName = malloc(strlen(e->event.image_reply.filename) + strlen(szImgType) + 1);
				if (dat->lpszFileName != NULL)
				{
					strcpy(dat->lpszFileName, e->event.image_reply.filename);
					strcat(dat->lpszFileName, szImgType);
				}
			}
		}

		if (dat->lpszFileName == NULL)
			dat->lpszFileName = _strdup(e->event.image_reply.filename);
	}

	////////////////////////////////////////////////////////////////////
	// Loading picture using Miranda Image services

	// Load image from memory
	if (!szFileName)
	{
		IMGSRVC_MEMIO memio;
		memio.iLen = dat->nSize;
		memio.pBuf = (void *)dat->lpData;
		memio.fif = FIF_UNKNOWN; /* detect */
		memio.flags = 0;
		dat->hBitmap = (HBITMAP) CallService(MS_IMG_LOADFROMMEM, (WPARAM) &memio, 0);
	}
	// Load image from file
	else
		dat->hBitmap = (HBITMAP) CallService(MS_IMG_LOAD, (WPARAM) szFileName, 0);

	// If everything is fine return the handle
	if (dat->hBitmap) return dat;

	gg_netlog(gg, "gg_img_loadpicture(): MS_IMG_LOAD(MEM) failed.");
	if (dat)
	{
		if (dat->lpData)
			free(dat->lpData);
		if (dat->lpszFileName)
			free(dat->lpszFileName);
		free(dat);
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
// Image Recv : AddEvent proc
INT_PTR gg_img_recvimage(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT *)lParam;
	GGIMAGEENTRY *img = (GGIMAGEENTRY *)cle->lParam;

	gg_netlog(gg, "gg_img_recvimage(%x, %x): Popup new image.", wParam, lParam);
	if (!img) return FALSE;

	gg_img_display(gg, cle->hContact, img);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Windows queue management
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Removes dat structure
int gg_img_remove(GGIMAGEDLGDATA *dat)
{
	GGIMAGEENTRY *temp = NULL, *img = NULL;
	GGPROTO *gg;

	if (!dat) return FALSE;
	gg = dat->gg;

	EnterCriticalSection(&gg->img_mutex);

	// Remove the structure
	img = dat->lpImages;

	// Destroy picture handle
	while (temp = img)
	{
		img = img->lpNext;
		gg_img_releasepicture(temp);
	}

	// Remove from list
	list_remove(&gg->imagedlgs, dat, 1);
	LeaveCriticalSection(&gg->img_mutex);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
//
GGIMAGEDLGDATA *gg_img_find(GGPROTO *gg, uin_t uin, uint32_t crc32)
{
	int res = 0;
	list_t l = gg->imagedlgs;
	GGIMAGEDLGDATA *dat;

	EnterCriticalSection(&gg->img_mutex);
	while (l)
	{
		uin_t c_uin;

		dat = (GGIMAGEDLGDATA *)l->data;
		if (!dat) break;

		c_uin = DBGetContactSettingDword(dat->hContact, dat->gg->proto.m_szModuleName, GG_KEY_UIN, 0);

		if (!dat->bReceiving && dat->lpImages && dat->lpImages->crc32 == crc32 && c_uin == uin)
		{
			LeaveCriticalSection(&gg->img_mutex);
			return dat;
		}

		l = l->next;
	}
	LeaveCriticalSection(&gg->img_mutex);

	gg_netlog(gg, "gg_img_find(): Image not found on the list. It might be released before calling this function.");
	return NULL;
}


////////////////////////////////////////////////////////////////////////////
// Image Module : Send on Request
BOOL gg_img_sendonrequest(GGPROTO *gg, struct gg_event* e)
{
	GGIMAGEDLGDATA *dat = gg_img_find(gg, e->event.image_request.sender, e->event.image_request.crc32);

	if (!gg || !dat || !gg_isonline(gg)) return FALSE;

	EnterCriticalSection(&gg->sess_mutex);
	gg_image_reply(gg->sess, e->event.image_request.sender, dat->lpImages->lpszFileName, dat->lpImages->lpData, dat->lpImages->nSize);
	LeaveCriticalSection(&gg->sess_mutex);

	gg_img_remove(dat);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Send Image : Run (Thread and main)
INT_PTR gg_img_sendimg(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	GGIMAGEDLGDATA *dat = NULL;

	EnterCriticalSection(&gg->img_mutex);
	if (!dat)
	{
		dat = (GGIMAGEDLGDATA *)calloc(1, sizeof(GGIMAGEDLGDATA));
		dat->hContact = hContact;
		dat->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		dat->gg = gg;
		ResetEvent(dat->hEvent);

		// Create new dialog
		gg_forkthread(gg, gg_img_dlgcallthread, dat);

		while (WaitForSingleObjectEx(dat->hEvent, INFINITE, TRUE) != WAIT_OBJECT_0);
		CloseHandle(dat->hEvent);
		dat->hEvent = NULL;

		list_add(&gg->imagedlgs, dat, 0);
	}

	// Request choose dialog
	SendMessage(dat->hWnd, WM_CHOOSEIMG, 0, 0);
	LeaveCriticalSection(&gg->img_mutex);

	return 0;
}
