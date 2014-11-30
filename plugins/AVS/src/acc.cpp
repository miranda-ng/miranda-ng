/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-04 Miranda ICQ/IM project,
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

#define DM_AVATARCHANGED (WM_USER + 20)
#define DM_MYAVATARCHANGED (WM_USER + 21)

#define GIF_DISPOSAL_UNSPECIFIED	0
#define GIF_DISPOSAL_LEAVE			1
#define GIF_DISPOSAL_BACKGROUND		2
#define GIF_DISPOSAL_PREVIOUS		3

typedef struct
{
	MCONTACT hContact;
	char proto[64];
	HANDLE hHook;
	HANDLE hHookMy;
	HFONT hFont;   // font
	COLORREF borderColor;
	COLORREF bkgColor;
	COLORREF avatarBorderColor;
	int avatarRoundCornerRadius;
	TCHAR noAvatarText[128];
	BOOL respectHidden;
	BOOL resizeIfSmaller;
	BOOL fAero;
	BOOL showingAnimatedGif;

	struct {
		HBITMAP *hbms;
		int *times;

		FIMULTIBITMAP *multi;
		FIBITMAP *dib;
		int frameCount;
		int logicalWidth;
		int logicalHeight;
		BOOL loop;
		RGBQUAD background;
		BOOL started;

		struct {
			int num;
			int top;
			int left;
			int width;
			int height;
			int disposal_method;
		} frame;
	} ag;

} ACCData;


BOOL AnimatedGifGetData(ACCData *data)
{
	FIBITMAP *page = fei->FI_LockPage(data->ag.multi, 0);
	if (page == NULL)
		return FALSE;

	// Get info
	FITAG *tag = NULL;
	if (!fei->FI_GetMetadata(FIMD_ANIMATION, page, "LogicalWidth", &tag))
		goto ERR;
	data->ag.logicalWidth = *(WORD *)fei->FI_GetTagValue(tag);

	if (!fei->FI_GetMetadata(FIMD_ANIMATION, page, "LogicalHeight", &tag))
		goto ERR;
	data->ag.logicalHeight = *(WORD *)fei->FI_GetTagValue(tag);

	if (!fei->FI_GetMetadata(FIMD_ANIMATION, page, "Loop", &tag))
		goto ERR;
	data->ag.loop = (*(LONG *)fei->FI_GetTagValue(tag) > 0);

	if (fei->FI_HasBackgroundColor(page))
		fei->FI_GetBackgroundColor(page, &data->ag.background);

	fei->FI_UnlockPage(data->ag.multi, page, FALSE);
	return TRUE;

ERR:
	fei->FI_UnlockPage(data->ag.multi, page, FALSE);
	return FALSE;
}

void AnimatedGifDispodeFrame(ACCData *data)
{
	if (data->ag.frame.disposal_method == GIF_DISPOSAL_PREVIOUS) {
		// TODO
	}
	else if (data->ag.frame.disposal_method == GIF_DISPOSAL_BACKGROUND) {
		for (int y = 0; y < data->ag.frame.height; y++) {
			RGBQUAD *scanline = (RGBQUAD*)fei->FI_GetScanLine(data->ag.dib, data->ag.logicalHeight - (y + data->ag.frame.top) - 1) + data->ag.frame.left;
			for (int x = 0; x < data->ag.frame.width; x++)
				*scanline++ = data->ag.background;
		}
	}
}

void AnimatedGifMountFrame(ACCData* data, int page)
{
	data->ag.frame.num = page;

	if (data->ag.hbms[page] != NULL) {
		data->ag.frame.disposal_method = GIF_DISPOSAL_LEAVE;
		return;
	}

	FIBITMAP *dib = fei->FI_LockPage(data->ag.multi, data->ag.frame.num);
	if (dib == NULL)
		return;

	FITAG *tag = NULL;
	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "FrameLeft", &tag))
		data->ag.frame.left = *(WORD *)fei->FI_GetTagValue(tag);
	else
		data->ag.frame.left = 0;

	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "FrameTop", &tag))
		data->ag.frame.top = *(WORD *)fei->FI_GetTagValue(tag);
	else
		data->ag.frame.top = 0;

	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "FrameTime", &tag))
		data->ag.times[page] = *(LONG *)fei->FI_GetTagValue(tag);
	else
		data->ag.times[page] = 0;

	if (fei->FI_GetMetadata(FIMD_ANIMATION, dib, "DisposalMethod", &tag))
		data->ag.frame.disposal_method = *(BYTE *)fei->FI_GetTagValue(tag);
	else
		data->ag.frame.disposal_method = 0;

	data->ag.frame.width = fei->FI_GetWidth(dib);
	data->ag.frame.height = fei->FI_GetHeight(dib);

	//decode page
	int palSize = fei->FI_GetColorsUsed(dib);
	RGBQUAD *pal = fei->FI_GetPalette(dib);
	bool have_transparent = false;
	int transparent_color = -1;
	if (fei->FI_IsTransparent(dib)) {
		int count = fei->FI_GetTransparencyCount(dib);
		BYTE *table = fei->FI_GetTransparencyTable(dib);
		for (int i = 0; i < count; i++) {
			if (table[i] == 0) {
				have_transparent = true;
				transparent_color = i;
				break;
			}
		}
	}

	//copy page data into logical buffer, with full alpha opaqueness
	for (int y = 0; y < data->ag.frame.height; y++) {
		RGBQUAD *scanline = (RGBQUAD*)fei->FI_GetScanLine(data->ag.dib, data->ag.logicalHeight - (y + data->ag.frame.top) - 1) + data->ag.frame.left;
		BYTE *pageline = fei->FI_GetScanLine(dib, data->ag.frame.height - y - 1);
		for (int x = 0; x < data->ag.frame.width; x++) {
			if (!have_transparent || *pageline != transparent_color) {
				*scanline = pal[*pageline];
				scanline->rgbReserved = 255;
			}
			scanline++;
			pageline++;
		}
	}

	data->ag.hbms[page] = fei->FI_CreateHBITMAPFromDIB(data->ag.dib);

	fei->FI_UnlockPage(data->ag.multi, dib, FALSE);
}

void AnimatedGifDeleteTmpValues(ACCData* data)
{
	if (data->ag.multi != NULL) {
		fei->FI_CloseMultiBitmap(data->ag.multi, 0);
		data->ag.multi = NULL;
	}

	if (data->ag.dib != NULL) {
		fei->FI_Unload(data->ag.dib);
		data->ag.dib = NULL;
	}
}

void DestroyAnimatedGif(HWND hwnd, ACCData* data)
{
	if (!data->showingAnimatedGif)
		return;

	AnimatedGifDeleteTmpValues(data);

	if (data->ag.hbms != NULL) {
		for (int i = 0; i < data->ag.frameCount; i++)
			if (data->ag.hbms[i] != NULL)
				DeleteObject(data->ag.hbms[i]);

		free(data->ag.hbms);
		data->ag.hbms = NULL;
	}

	if (data->ag.times != NULL) {
		free(data->ag.times);
		data->ag.times = NULL;
	}

	data->showingAnimatedGif = FALSE;
	data->ag.started = FALSE;
}

void StartAnimatedGif(HWND hwnd, ACCData* data)
{
	if (fei == NULL)
		return;

	AVATARCACHEENTRY *ace = NULL;
	if (data->hContact != NULL)
		ace = (AVATARCACHEENTRY*)GetAvatarBitmap(data->hContact, 0);
	else
		ace = (AVATARCACHEENTRY*)GetMyAvatar(0, (LPARAM)data->proto);

	if (ace == NULL)
		return;

	int format = ProtoGetAvatarFormat(ace->szFilename);
	if (format != PA_FORMAT_GIF)
		return;

	FREE_IMAGE_FORMAT fif = fei->FI_GetFileTypeT(ace->szFilename, 0);
	if (fif == FIF_UNKNOWN)
		fif = fei->FI_GetFIFFromFilenameT(ace->szFilename);

	data->ag.multi = fei->FI_OpenMultiBitmapT(fif, ace->szFilename, FALSE, TRUE, FALSE, GIF_LOAD256);
	if (data->ag.multi == NULL)
		return;

	data->ag.frameCount = fei->FI_GetPageCount(data->ag.multi);
	if (data->ag.frameCount <= 1)
		goto ERR;

	if (!AnimatedGifGetData(data))
		goto ERR;

	// allocate entire logical area
	data->ag.dib = fei->FI_Allocate(data->ag.logicalWidth, data->ag.logicalHeight, 32, 0, 0, 0);
	if (data->ag.dib == NULL)
		goto ERR;

	// fill with background color to start
	for (int y = 0; y < data->ag.logicalHeight; y++) {
		RGBQUAD *scanline = (RGBQUAD*)fei->FI_GetScanLine(data->ag.dib, y);
		for (int x = 0; x < data->ag.logicalWidth; x++)
			*scanline++ = data->ag.background;
	}

	data->ag.hbms = (HBITMAP *)malloc(sizeof(HBITMAP) * data->ag.frameCount);
	memset(data->ag.hbms, 0, sizeof(HBITMAP) * data->ag.frameCount);

	data->ag.times = (int *)malloc(sizeof(int) * data->ag.frameCount);
	memset(data->ag.times, 0, sizeof(int) * data->ag.frameCount);

	AnimatedGifMountFrame(data, 0);

	data->showingAnimatedGif = TRUE;

	return;
ERR:
	fei->FI_CloseMultiBitmap(data->ag.multi, 0);
	data->ag.multi = NULL;
}

void DestroyAnimation(HWND hwnd, ACCData* data)
{
	DestroyAnimatedGif(hwnd, data);
}

void StartAnimation(HWND hwnd, ACCData* data)
{
	StartAnimatedGif(hwnd, data);
}

BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	POINT pt;
	pt.x = lpRect->left;
	pt.y = lpRect->top;

	BOOL ret = ScreenToClient(hWnd, &pt);
	if (!ret)
		return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;

	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;
	return ret;
}

static void Invalidate(HWND hwnd)
{
	ACCData *data = (ACCData*)GetWindowLongPtr(hwnd, 0);
	if (data->bkgColor == -1) {
		HWND parent = GetParent(hwnd);
		RECT rc;
		GetWindowRect(hwnd, &rc);
		ScreenToClient(parent, &rc);
		InvalidateRect(parent, &rc, TRUE);
	}
	InvalidateRect(hwnd, NULL, TRUE);
}

static void NotifyAvatarChange(HWND hwnd)
{
	PSHNOTIFY pshn = { 0 };
	pshn.hdr.idFrom = GetDlgCtrlID(hwnd);
	pshn.hdr.hwndFrom = hwnd;
	pshn.hdr.code = NM_AVATAR_CHANGED;
	pshn.lParam = 0;
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&pshn);
}

static void DrawText(HDC hdc, HFONT hFont, const RECT &rc, const TCHAR *text)
{
	HGDIOBJ oldFont = SelectObject(hdc, hFont);

	// Get text rectangle
	RECT tr = rc;
	tr.top += 10;
	tr.bottom -= 10;
	tr.left += 10;
	tr.right -= 10;

	// Calc text size
	RECT tr_ret = tr;
	DrawText(hdc, text, -1, &tr_ret, DT_WORDBREAK | DT_NOPREFIX | DT_CENTER | DT_CALCRECT);

	// Calc needed size
	tr.top += ((tr.bottom - tr.top) - (tr_ret.bottom - tr_ret.top)) / 2;
	tr.bottom = tr.top + (tr_ret.bottom - tr_ret.top);
	DrawText(hdc, text, -1, &tr, DT_WORDBREAK | DT_NOPREFIX | DT_CENTER);

	SelectObject(hdc, oldFont);
}

static LRESULT CALLBACK ACCWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ACCData *data = (ACCData*)GetWindowLongPtr(hwnd, 0);
	char *szProto;

	switch (msg) {
	case WM_NCCREATE:
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | BS_OWNERDRAW);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

		data = (ACCData*)mir_alloc(sizeof(ACCData));
		if (data == NULL)
			return FALSE;
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)data);

		ZeroMemory(data, sizeof(ACCData));
		data->hHook = HookEventMessage(ME_AV_AVATARCHANGED, hwnd, DM_AVATARCHANGED);
		data->hHookMy = HookEventMessage(ME_AV_MYAVATARCHANGED, hwnd, DM_MYAVATARCHANGED);
		data->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		data->borderColor = -1;
		data->bkgColor = -1;
		data->avatarBorderColor = -1;
		data->respectHidden = TRUE;
		data->resizeIfSmaller = TRUE;
		data->showingAnimatedGif = FALSE;
		data->fAero = FALSE;
		return TRUE;

	case WM_NCDESTROY:
		DestroyAnimation(hwnd, data);
		if (data) {
			UnhookEvent(data->hHook);
			UnhookEvent(data->hHookMy);
			mir_free(data);
		}
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)NULL);
		break;

	case WM_SETFONT:
		data->hFont = (HFONT)wParam;
		Invalidate(hwnd);
		break;

	case AVATAR_SETCONTACT:
		if (lParam == 0)
			return FALSE;

		if (data->hContact != lParam) {
			DestroyAnimation(hwnd, data);

			data->hContact = lParam;
			if (lParam == NULL)
				data->proto[0] = '\0';
			else
				mir_strncpy(data->proto, GetContactProto(data->hContact), sizeof(data->proto));

			StartAnimation(hwnd, data);

			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		return TRUE;

	case AVATAR_SETPROTOCOL:
		szProto = (lParam == NULL) ? "" : (char*)lParam;
		if (data->hContact != 0 || strcmp(szProto, data->proto)) {
			DestroyAnimation(hwnd, data);

			data->hContact = NULL;
			strncpy_s(data->proto, szProto, _TRUNCATE);

			StartAnimation(hwnd, data);
			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		return TRUE;

	case AVATAR_SETBKGCOLOR:
		data->bkgColor = (COLORREF)lParam;
		NotifyAvatarChange(hwnd);
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_SETBORDERCOLOR:
		data->borderColor = (COLORREF)lParam;
		NotifyAvatarChange(hwnd);
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_SETAVATARBORDERCOLOR:
		data->avatarBorderColor = (COLORREF)lParam;
		NotifyAvatarChange(hwnd);
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_SETAVATARROUNDCORNERRADIUS:
		data->avatarRoundCornerRadius = (int)lParam;
		NotifyAvatarChange(hwnd);
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_SETNOAVATARTEXT:
		mir_tstrncpy(data->noAvatarText, TranslateTS((TCHAR*)lParam), SIZEOF(data->noAvatarText));
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_RESPECTHIDDEN:
		data->respectHidden = (BOOL)lParam;
		NotifyAvatarChange(hwnd);
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_SETRESIZEIFSMALLER:
		data->resizeIfSmaller = (BOOL)lParam;
		NotifyAvatarChange(hwnd);
		Invalidate(hwnd);
		return TRUE;

	case AVATAR_SETAEROCOMPATDRAWING:
		data->fAero = lParam;
		return(TRUE);

	case AVATAR_GETUSEDSPACE:
		{
			int *width = (int*)wParam;
			int *height = (int*)lParam;

			RECT rc;
			GetClientRect(hwnd, &rc);

			// Get avatar
			avatarCacheEntry *ace;
			if (data->hContact == NULL)
				ace = (avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)data->proto);
			else
				ace = (avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)data->hContact, 0);

			if (ace == NULL || ace->bmHeight == 0 || ace->bmWidth == 0
				 || (data->respectHidden && (ace->dwFlags & AVS_HIDEONCLIST))) {
				*width = 0;
				*height = 0;
				return TRUE;
			}

			// Get its size
			int targetWidth = rc.right - rc.left;
			int targetHeight = rc.bottom - rc.top;

			if (!data->resizeIfSmaller && ace->bmHeight <= targetHeight && ace->bmWidth <= targetWidth) {
				*height = ace->bmHeight;
				*width = ace->bmWidth;
			}
			else if (ace->bmHeight > ace->bmWidth) {
				float dScale = targetHeight / (float)ace->bmHeight;
				*height = targetHeight;
				*width = (int)(ace->bmWidth * dScale);
			}
			else {
				float dScale = targetWidth / (float)ace->bmWidth;
				*height = (int)(ace->bmHeight * dScale);
				*width = targetWidth;
			}
		}
		return TRUE;

	case DM_AVATARCHANGED:
		if (data->hContact == wParam) {
			DestroyAnimation(hwnd, data);
			StartAnimation(hwnd, data);

			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		break;

	case DM_MYAVATARCHANGED:
		if (data->hContact == NULL && strcmp(data->proto, (char*)wParam) == 0) {
			DestroyAnimation(hwnd, data);
			StartAnimation(hwnd, data);

			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			if (hdc == NULL)
				break;

			int oldBkMode = SetBkMode(hdc, TRANSPARENT);
			SetStretchBltMode(hdc, HALFTONE);

			RECT rc;
			GetClientRect(hwnd, &rc);

			// Draw background
			if (data->bkgColor != -1) {
				HBRUSH hbrush = CreateSolidBrush(data->bkgColor);
				FillRect(hdc, &rc, hbrush);
				DeleteObject(hbrush);
			}

			if (data->hContact == NULL && data->proto[0] == 0 && db_get_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1))
				DrawText(hdc, data->hFont, rc, TranslateT("Protocols have different avatars"));

			// Has an animated gif
			// Has a "normal" image
			else {
				// Draw avatar
				AVATARDRAWREQUEST avdrq = { 0 };
				avdrq.cbSize = sizeof(avdrq);
				avdrq.rcDraw = rc;
				avdrq.hContact = data->hContact;
				avdrq.szProto = data->proto;
				avdrq.hTargetDC = hdc;
				avdrq.dwFlags = AVDRQ_HIDEBORDERONTRANSPARENCY
					| (data->respectHidden ? AVDRQ_RESPECTHIDDEN : 0)
					| (data->hContact != NULL ? 0 : AVDRQ_OWNPIC)
					| (data->avatarBorderColor == -1 ? 0 : AVDRQ_DRAWBORDER)
					| (data->avatarRoundCornerRadius <= 0 ? 0 : AVDRQ_ROUNDEDCORNER)
					| (data->fAero ? AVDRQ_AERO : 0)
					| (data->resizeIfSmaller ? 0 : AVDRQ_DONTRESIZEIFSMALLER);
				avdrq.clrBorder = data->avatarBorderColor;
				avdrq.radius = data->avatarRoundCornerRadius;

				INT_PTR ret;
				if (data->showingAnimatedGif) {
					InternalDrawAvatar(&avdrq, data->ag.hbms[data->ag.frame.num], data->ag.logicalWidth, data->ag.logicalHeight, 0);
					ret = 1;

					if (!data->ag.started) {
						SetTimer(hwnd, 0, data->ag.times[data->ag.frame.num], NULL);
						data->ag.started = TRUE;
					}
				}
				else
					ret = DrawAvatarPicture(0, (LPARAM)&avdrq);

				if (ret == 0)
					DrawText(hdc, data->hFont, rc, data->noAvatarText);
			}

			// Draw control border
			if (data->borderColor != -1) {
				HBRUSH hbrush = CreateSolidBrush(data->borderColor);
				FrameRect(hdc, &rc, hbrush);
				DeleteObject(hbrush);
			}

			SetBkMode(hdc, oldBkMode);

			EndPaint(hwnd, &ps);
		}
		return TRUE;

	case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;
			RECT rc;
			GetClientRect(hwnd, &rc);

			// Draw background
			if (data->bkgColor != -1) {
				HBRUSH hbrush = CreateSolidBrush(data->bkgColor);
				FillRect(hdc, &rc, hbrush);
				DeleteObject(hbrush);
			}

			// Draw control border
			if (data->borderColor != -1) {
				HBRUSH hbrush = CreateSolidBrush(data->borderColor);
				FrameRect(hdc, &rc, hbrush);
				DeleteObject(hbrush);
			}
		}
		return TRUE;

	case WM_SIZE:
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case WM_TIMER:
		if (wParam != 0)
			break;
		KillTimer(hwnd, 0);

		if (!data->showingAnimatedGif)
			break;

		AnimatedGifDispodeFrame(data);

		int frame = data->ag.frame.num + 1;
		if (frame >= data->ag.frameCount) {
			// Don't need fi data no more
			AnimatedGifDeleteTmpValues(data);
			frame = 0;
		}
		AnimatedGifMountFrame(data, frame);

		data->ag.started = FALSE;
		InvalidateRect(hwnd, NULL, FALSE);

		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int LoadACC()
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = AVATAR_CONTROL_CLASS;
	wc.lpfnWndProc = ACCWndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(ACCData*);
	wc.hbrBackground = 0;
	wc.style = CS_GLOBALCLASS;
	RegisterClassEx(&wc);
	return 0;
}
