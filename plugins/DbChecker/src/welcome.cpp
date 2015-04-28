/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

INT_PTR CALLBACK WelcomeDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HENHMETAFILE hEmfWatermark;
	static HFONT hTitleFont;

	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
	{
		HRSRC hRsrcWatermark = FindResourceA(hInst, MAKEINTRESOURCEA(IDE_WATERMARK), "EMF");
		HGLOBAL hGlobWatermark = LoadResource(hInst, hRsrcWatermark);
		hEmfWatermark = SetEnhMetaFileBits(SizeofResource(hInst, hRsrcWatermark), (PBYTE)LockResource(hGlobWatermark));
	}
	SendDlgItemMessage(hdlg, IDC_WATERMARK, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)hEmfWatermark);
	{
		NONCLIENTMETRICS ncm = { 0 };
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
		LOGFONT TitleLogFont = ncm.lfMessageFont;
		TitleLogFont.lfWeight = FW_BOLD;
		mir_tstrcpy(TitleLogFont.lfFaceName, TEXT("Verdana Bold"));

		HDC hdc = GetDC(NULL);
		INT FontSize = 12;
		TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * FontSize / 72;
		hTitleFont = CreateFontIndirect(&TitleLogFont);
		ReleaseDC(NULL, hdc);
	}
	SendDlgItemMessage(hdlg, IDC_TITLE, WM_SETFONT, (WPARAM)hTitleFont, 0);
	EnableWindow(GetDlgItem(GetParent(hdlg), IDC_BACK), FALSE);
	return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_SELECTDB, (LPARAM)SelectDbDlgProc);
			break;
		}
		break;

	case WM_DESTROY:
		DeleteEnhMetaFile(hEmfWatermark);
		DeleteObject(hTitleFont);
		break;
	}
	return FALSE;
}
