/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#include "stdafx.h"

#define GIF_DISPOSAL_UNSPECIFIED	0
#define GIF_DISPOSAL_LEAVE			1
#define GIF_DISPOSAL_BACKGROUND		2
#define GIF_DISPOSAL_PREVIOUS		3

struct ACCData
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
	wchar_t noAvatarText[128];
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
};


BOOL AnimatedGifGetData(ACCData *data)
{
	FIBITMAP *page = FreeImage_LockPage(data->ag.multi, 0);
	if (page == nullptr)
		return FALSE;

	// Get info
	FITAG *tag = nullptr;
	if (!FreeImage_GetMetadata(FIMD_ANIMATION, page, "LogicalWidth", &tag))
		goto ERR;
	data->ag.logicalWidth = *(uint16_t *)FreeImage_GetTagValue(tag);

	if (!FreeImage_GetMetadata(FIMD_ANIMATION, page, "LogicalHeight", &tag))
		goto ERR;
	data->ag.logicalHeight = *(uint16_t *)FreeImage_GetTagValue(tag);

	if (!FreeImage_GetMetadata(FIMD_ANIMATION, page, "Loop", &tag))
		goto ERR;
	data->ag.loop = (*(LONG *)FreeImage_GetTagValue(tag) > 0);

	if (FreeImage_HasBackgroundColor(page))
		FreeImage_GetBackgroundColor(page, &data->ag.background);

	FreeImage_UnlockPage(data->ag.multi, page, FALSE);
	return TRUE;

ERR:
	FreeImage_UnlockPage(data->ag.multi, page, FALSE);
	return FALSE;
}

void AnimatedGifDispodeFrame(ACCData *data)
{
	if (data->ag.frame.disposal_method == GIF_DISPOSAL_PREVIOUS) {
		// TODO
	}
	else if (data->ag.frame.disposal_method == GIF_DISPOSAL_BACKGROUND) {
		for (int y = 0; y < data->ag.frame.height; y++) {
			RGBQUAD *scanline = (RGBQUAD*)FreeImage_GetScanLine(data->ag.dib, data->ag.logicalHeight - (y + data->ag.frame.top) - 1) + data->ag.frame.left;
			for (int x = 0; x < data->ag.frame.width; x++)
				*scanline++ = data->ag.background;
		}
	}
}

void AnimatedGifMountFrame(ACCData* data, int page)
{
	data->ag.frame.num = page;

	if (data->ag.hbms[page] != nullptr) {
		data->ag.frame.disposal_method = GIF_DISPOSAL_LEAVE;
		return;
	}

	FIBITMAP *dib = FreeImage_LockPage(data->ag.multi, data->ag.frame.num);
	if (dib == nullptr)
		return;

	FITAG *tag = nullptr;
	if (FreeImage_GetMetadata(FIMD_ANIMATION, dib, "FrameLeft", &tag))
		data->ag.frame.left = *(uint16_t *)FreeImage_GetTagValue(tag);
	else
		data->ag.frame.left = 0;

	if (FreeImage_GetMetadata(FIMD_ANIMATION, dib, "FrameTop", &tag))
		data->ag.frame.top = *(uint16_t *)FreeImage_GetTagValue(tag);
	else
		data->ag.frame.top = 0;

	if (FreeImage_GetMetadata(FIMD_ANIMATION, dib, "FrameTime", &tag))
		data->ag.times[page] = *(LONG *)FreeImage_GetTagValue(tag);
	else
		data->ag.times[page] = 0;

	if (FreeImage_GetMetadata(FIMD_ANIMATION, dib, "DisposalMethod", &tag))
		data->ag.frame.disposal_method = *(uint8_t *)FreeImage_GetTagValue(tag);
	else
		data->ag.frame.disposal_method = 0;

	data->ag.frame.width = FreeImage_GetWidth(dib);
	data->ag.frame.height = FreeImage_GetHeight(dib);

	//decode page
	RGBQUAD *pal = FreeImage_GetPalette(dib);
	bool have_transparent = false;
	int transparent_color = -1;
	if (FreeImage_IsTransparent(dib)) {
		int count = FreeImage_GetTransparencyCount(dib);
		uint8_t *table = FreeImage_GetTransparencyTable(dib);
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
		RGBQUAD *scanline = (RGBQUAD*)FreeImage_GetScanLine(data->ag.dib, data->ag.logicalHeight - (y + data->ag.frame.top) - 1) + data->ag.frame.left;
		uint8_t *pageline = FreeImage_GetScanLine(dib, data->ag.frame.height - y - 1);
		for (int x = 0; x < data->ag.frame.width; x++) {
			if (!have_transparent || *pageline != transparent_color) {
				*scanline = pal[*pageline];
				scanline->rgbReserved = 255;
			}
			scanline++;
			pageline++;
		}
	}

	data->ag.hbms[page] = FreeImage_CreateHBITMAPFromDIB(data->ag.dib);

	FreeImage_UnlockPage(data->ag.multi, dib, FALSE);
}

void AnimatedGifDeleteTmpValues(ACCData* data)
{
	if (data->ag.multi != nullptr) {
		FreeImage_CloseMultiBitmap(data->ag.multi, 0);
		data->ag.multi = nullptr;
	}

	if (data->ag.dib != nullptr) {
		FreeImage_Unload(data->ag.dib);
		data->ag.dib = nullptr;
	}
}

void DestroyAnimatedGif(ACCData* data)
{
	if (!data->showingAnimatedGif)
		return;

	AnimatedGifDeleteTmpValues(data);

	if (data->ag.hbms != nullptr) {
		for (int i = 0; i < data->ag.frameCount; i++)
			if (data->ag.hbms[i] != nullptr)
				DeleteObject(data->ag.hbms[i]);

		free(data->ag.hbms);
		data->ag.hbms = nullptr;
	}

	if (data->ag.times != nullptr) {
		free(data->ag.times);
		data->ag.times = nullptr;
	}

	data->showingAnimatedGif = FALSE;
	data->ag.started = FALSE;
}

void StartAnimatedGif(ACCData* data)
{
	AVATARCACHEENTRY *ace = nullptr;
	if (data->hContact != NULL)
		ace = (AVATARCACHEENTRY*)GetAvatarBitmap(data->hContact, 0);
	else
		ace = (AVATARCACHEENTRY*)GetMyAvatar(0, (LPARAM)data->proto);

	if (ace == nullptr)
		return;

	int format = ProtoGetAvatarFormat(ace->szFilename);
	if (format != PA_FORMAT_GIF)
		return;

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(ace->szFilename, 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilenameU(ace->szFilename);

	data->ag.multi = FreeImage_OpenMultiBitmapU(fif, ace->szFilename, FALSE, TRUE, FALSE, GIF_LOAD256);
	if (data->ag.multi == nullptr)
		return;

	data->ag.frameCount = FreeImage_GetPageCount(data->ag.multi);
	if (data->ag.frameCount <= 1)
		goto ERR;

	if (!AnimatedGifGetData(data))
		goto ERR;

	// allocate entire logical area
	data->ag.dib = FreeImage_Allocate(data->ag.logicalWidth, data->ag.logicalHeight, 32, 0, 0, 0);
	if (data->ag.dib == nullptr)
		goto ERR;

	// fill with background color to start
	for (int y = 0; y < data->ag.logicalHeight; y++) {
		RGBQUAD *scanline = (RGBQUAD*)FreeImage_GetScanLine(data->ag.dib, y);
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
	FreeImage_CloseMultiBitmap(data->ag.multi, 0);
	data->ag.multi = nullptr;
}

void DestroyAnimation(ACCData* data)
{
	DestroyAnimatedGif(data);
}

void StartAnimation(ACCData* data)
{
	StartAnimatedGif(data);
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
	InvalidateRect(hwnd, nullptr, TRUE);
}

static void NotifyAvatarChange(HWND hwnd)
{
	PSHNOTIFY pshn = {};
	pshn.hdr.idFrom = GetDlgCtrlID(hwnd);
	pshn.hdr.hwndFrom = hwnd;
	pshn.hdr.code = NM_AVATAR_CHANGED;
	pshn.lParam = 0;
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&pshn);
}

static void DrawText(HDC hdc, HFONT hFont, const RECT &rc, const wchar_t *text)
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
	RECT rc;

	switch (msg) {
	case WM_NCCREATE:
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | BS_OWNERDRAW);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

		data = (ACCData*)mir_calloc(sizeof(ACCData));
		if (data == nullptr)
			return FALSE;
		
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)data);
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
		if (data) {
			DestroyAnimation(data);
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

		if (data->hContact != (MCONTACT)lParam) {
			DestroyAnimation(data);

			data->hContact = lParam;
			mir_strncpy(data->proto, Proto_GetBaseAccountName(data->hContact), sizeof(data->proto));

			StartAnimation(data);

			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		return TRUE;

	case AVATAR_SETPROTOCOL:
		szProto = (lParam == NULL) ? "" : (char*)lParam;
		if (data->hContact != 0 || mir_strcmp(szProto, data->proto)) {
			DestroyAnimation(data);

			data->hContact = NULL;
			strncpy_s(data->proto, szProto, _TRUNCATE);

			StartAnimation(data);
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
		mir_wstrncpy(data->noAvatarText, TranslateW((wchar_t*)lParam), _countof(data->noAvatarText));
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
		GetClientRect(hwnd, &rc);
		{
			int *width = (int*)wParam;
			int *height = (int*)lParam;

			// Get avatar
			AVATARCACHEENTRY *ace;
			if (data->hContact == NULL)
				ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)data->proto);
			else
				ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)data->hContact, 0);

			if (ace == nullptr || ace->bmHeight == 0 || ace->bmWidth == 0 || (data->respectHidden && (ace->dwFlags & AVS_HIDEONCLIST))) {
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
			DestroyAnimation(data);
			StartAnimation(data);

			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		break;

	case DM_MYAVATARCHANGED:
		if (data->hContact == NULL && mir_strcmp(data->proto, (char*)wParam) == 0) {
			DestroyAnimation(data);
			StartAnimation(data);

			NotifyAvatarChange(hwnd);
			Invalidate(hwnd);
		}
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		{
			HDC hdc = BeginPaint(hwnd, &ps);
			if (hdc == nullptr)
				break;

			int oldBkMode = SetBkMode(hdc, TRANSPARENT);
			SetStretchBltMode(hdc, HALFTONE);

			GetClientRect(hwnd, &rc);

			// Draw background
			if (data->bkgColor != -1) {
				HBRUSH hbrush = CreateSolidBrush(data->bkgColor);
				FillRect(hdc, &rc, hbrush);
				DeleteObject(hbrush);
			}

			if (data->hContact == NULL && data->proto[0] == 0 && g_plugin.getByte("GlobalUserAvatarNotConsistent", 1))
				DrawText(hdc, data->hFont, rc, TranslateT("Protocols have different avatars"));

			// Has an animated gif
			// Has a "normal" image
			else {
				// Draw avatar
				AVATARDRAWREQUEST avdrq = { 0 };
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
						SetTimer(hwnd, 0, data->ag.times[data->ag.frame.num], nullptr);
						data->ag.started = TRUE;
					}
				}
				else ret = DrawAvatarPicture(0, (LPARAM)&avdrq);

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
		GetClientRect(hwnd, &rc);
		{
			HDC hdc = (HDC)wParam;

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
		InvalidateRect(hwnd, nullptr, TRUE);
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
		InvalidateRect(hwnd, nullptr, FALSE);

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
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(ACCData*);
	wc.style = CS_GLOBALCLASS;
	RegisterClassEx(&wc);
	return 0;
}
