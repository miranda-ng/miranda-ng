/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

int RefreshSkinList(HWND hwndDlg)
{
	HWND hwndSkins = GetDlgItem(hwndDlg, IDC_LB_SKINS);
	ListBox_ResetContent(hwndSkins);
	ListBox_AddString(hwndSkins, TranslateT("# Solid color fill"));

	wchar_t szDirSave[1024];
	GetCurrentDirectory(1024, szDirSave);
	SetCurrentDirectory(SKIN_FOLDER);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"*.*", &ffd);
	while (hFind != INVALID_HANDLE_VALUE) {
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mir_wstrcmp(L".", ffd.cFileName) && mir_wstrcmp(L"..", ffd.cFileName)) {
			SetCurrentDirectory(ffd.cFileName);
			WIN32_FIND_DATA ffd2;
			HANDLE hFindFile = FindFirstFile(L"*.tsf", &ffd2);
			if (hFindFile != INVALID_HANDLE_VALUE)
				ListBox_AddString(hwndSkins, ffd.cFileName);

			FindClose(hFindFile);
			SetCurrentDirectory(L"..");
		}

		if (!FindNextFile(hFind, &ffd))
			break;
	}

	FindClose(hFind);
	SetCurrentDirectory(szDirSave);

	return ListBox_SelectString(GetDlgItem(hwndDlg, IDC_LB_SKINS), -1, opt.szSkinName);
}

bool FileExists(wchar_t *filename)
{
	HANDLE hFile = CreateFile(filename, 0, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return true;
	}

	return false;
}

void ParseAboutPart(FILE *fp, wchar_t *buff, wchar_t *szSkinName)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[') {
		if (buff[0] != ';') {
			wchar_t *pch = wcschr(buff, '=');
			if (pch++) {
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch) {
					if (wcsstr(buff, L"author")) {
					}
					else if (wcsstr(buff, L"preview")) {
						wchar_t szImgPath[1024];
						mir_snwprintf(szImgPath, L"%s\\%s\\%s", SKIN_FOLDER, szSkinName, pch);
						if (FileExists(szImgPath))
							mir_wstrcpy(opt.szPreviewFile, szImgPath);
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseImagePart(FILE *fp, wchar_t *buff, int iPart)
{
	opt.szImgFile[iPart] = nullptr;
	opt.transfMode[iPart] = TM_NONE;
	opt.margins[iPart].left = 0;
	opt.margins[iPart].top = 0;
	opt.margins[iPart].right = 0;
	opt.margins[iPart].bottom = 0;

	myfgets(buff, 1024, fp);
	while (buff[0] != '[') {
		if (buff[0] != ';') {
			wchar_t *pch = wcschr(buff, '=');
			if (pch++) {
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch) {
					if (wcsstr(buff, L"image")) {
						wchar_t szImgPath[1024];
						mir_snwprintf(szImgPath, L"%s\\%s\\%s", SKIN_FOLDER, opt.szSkinName, pch);
						opt.szImgFile[iPart] = mir_wstrdup(szImgPath);
					}
					else if (wcsstr(buff, L"tm")) {
						if (!mir_wstrcmpi(pch, L"TM_NONE"))
							opt.transfMode[iPart] = TM_NONE;
						else if (!mir_wstrcmpi(pch, L"TM_CENTRE"))
							opt.transfMode[iPart] = TM_CENTRE;
						else if (!mir_wstrcmpi(pch, L"TM_STRECH_ALL"))
							opt.transfMode[iPart] = TM_STRECH_ALL;
						else if (!mir_wstrcmpi(pch, L"TM_STRECH_HORIZONTAL"))
							opt.transfMode[iPart] = TM_STRECH_HORIZONTAL;
						else if (!mir_wstrcmpi(pch, L"TM_STRECH_VERTICAL"))
							opt.transfMode[iPart] = TM_STRECH_VERTICAL;
						else if (!mir_wstrcmpi(pch, L"TM_TILE_ALL"))
							opt.transfMode[iPart] = TM_TILE_ALL;
						else if (!mir_wstrcmpi(pch, L"TM_TILE_HORIZONTAL"))
							opt.transfMode[iPart] = TM_TILE_HORIZONTAL;
						else if (!mir_wstrcmpi(pch, L"TM_TILE_VERTICAL"))
							opt.transfMode[iPart] = TM_TILE_VERTICAL;
						else
							opt.transfMode[iPart] = TM_NONE;
					}
					else if (wcsstr(buff, L"left"))
						opt.margins[iPart].left = _wtoi(pch);
					else if (wcsstr(buff, L"top"))
						opt.margins[iPart].top = _wtoi(pch);
					else if (wcsstr(buff, L"right"))
						opt.margins[iPart].right = _wtoi(pch);
					else if (wcsstr(buff, L"bottom"))
						opt.margins[iPart].bottom = _wtoi(pch);
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

char *GetSettingName(wchar_t *szValue, char *szPostfix, char *buff, size_t buffsize)
{
	buff[0] = 0;

	if (wcsstr(szValue, L"traytitle"))
		mir_snprintf(buff, buffsize, "FontTrayTitle%s", szPostfix);
	else if (wcsstr(szValue, L"title"))
		mir_snprintf(buff, buffsize, "FontFirst%s", szPostfix);
	else if (wcsstr(szValue, L"label"))
		mir_snprintf(buff, buffsize, "FontLabels%s", szPostfix);
	else if (wcsstr(szValue, L"value"))
		mir_snprintf(buff, buffsize, "FontValues%s", szPostfix);
	else if (wcsstr(szValue, L"divider"))
		mir_snprintf(buff, buffsize, "Divider%s", szPostfix);

	return buff[0] ? buff : nullptr;
}

void ParseFontPart(FILE *fp, wchar_t *buff)
{
	char szSetting[64];

	myfgets(buff, 1024, fp);
	while (buff[0] != '[') {
		if (buff[0] != ';') {
			wchar_t *pch = wcschr(buff, '=');
			if (pch++) {
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch) {
					if (wcsstr(buff, L"face")) {
						if (GetSettingName(buff, "", szSetting, sizeof(szSetting) - 1)) {
							if (mir_wstrlen(pch) > 32)
								pch[32] = 0;

							g_plugin.setWString(szSetting, pch);
						}
					}
					else if (wcsstr(buff, L"color")) {
						if (GetSettingName(buff, "Col", szSetting, sizeof(szSetting) - 1)) {
							uint8_t r = _wtoi(pch);
							pch = wcschr(pch, ' ');
							if (++pch) {
								uint8_t g = _wtoi(pch);
								pch = wcschr(pch, ' ');
								if (++pch) {
									uint8_t b = _wtoi(pch);
									COLORREF color = RGB(r, g, b);
									g_plugin.setDword(szSetting, color);
								}
							}
						}
					}
					else if (wcsstr(buff, L"size")) {
						if (GetSettingName(buff, "Size", szSetting, sizeof(szSetting) - 1)) {
							HDC hdc = GetDC(nullptr);
							int size = -MulDiv(_wtoi(pch), GetDeviceCaps(hdc, LOGPIXELSY), 72);
							g_plugin.setByte(szSetting, (uint8_t)size);
							ReleaseDC(nullptr, hdc);
						}
					}
					else if (wcsstr(buff, L"effect")) {
						if (GetSettingName(buff, "Sty", szSetting, sizeof(szSetting) - 1)) {
							uint8_t effect = 0;
							if (wcsstr(pch, L"font_bold"))
								effect |= DBFONTF_BOLD;
							if (wcsstr(pch, L"font_italic"))
								effect |= DBFONTF_ITALIC;
							if (wcsstr(pch, L"font_underline"))
								effect |= DBFONTF_UNDERLINE;

							g_plugin.setByte(szSetting, effect);
						}
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseAppearancePart(FILE *fp, wchar_t *buff)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[') {
		if (buff[0] != ';') {
			wchar_t *pch = wcschr(buff, '=');
			if (pch++) {
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch) {
					if (wcsstr(buff, L"general-padding"))
						opt.iPadding = _wtoi(pch);
					else if (wcsstr(buff, L"title-indent"))
						opt.iTitleIndent = _wtoi(pch);
					else if (wcsstr(buff, L"text-indent"))
						opt.iTextIndent = _wtoi(pch);
					else if (wcsstr(buff, L"value-indent"))
						opt.iValueIndent = _wtoi(pch);
					else if (wcsstr(buff, L"text-padding"))
						opt.iTextPadding = _wtoi(pch);
					else if (wcsstr(buff, L"outer-avatar-padding"))
						opt.iOuterAvatarPadding = _wtoi(pch);
					else if (wcsstr(buff, L"inner-avatar-padding"))
						opt.iInnerAvatarPadding = _wtoi(pch);
					else if (wcsstr(buff, L"sidebar-width"))
						opt.iSidebarWidth = _wtoi(pch);
					else if (wcsstr(buff, L"opacity"))
						opt.iOpacity = _wtoi(pch);
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseOtherPart(FILE *fp, wchar_t *buff)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[') {
		if (buff[0] != ';') {
			wchar_t *pch = wcschr(buff, '=');
			if (pch++) {
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch) {
					if (wcsstr(buff, L"enable-coloring")) {
						if (wcsstr(pch, L"false"))
							opt.iEnableColoring = -1;
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseSkinFile(wchar_t *szSkinName, bool bStartup, bool bOnlyPreview)
{
	wchar_t szDirSave[1024], buff[1024];

	if (opt.skinMode == SM_OBSOLOTE && bStartup)
		return;

	if (!bStartup) opt.iEnableColoring = 0;
	opt.szPreviewFile[0] = 0;

	GetCurrentDirectory(1024, szDirSave);
	SetCurrentDirectory(SKIN_FOLDER);
	SetCurrentDirectory(szSkinName);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"*.tsf", &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		FILE *fp = _wfopen(ffd.cFileName, L"r");
		if (fp) {
			myfgets(buff, 1024, fp);
			while (!feof(fp)) {
				if (buff[0] == '[') {
					if (!mir_wstrcmp(L"[about]", buff)) {
						ParseAboutPart(fp, buff, szSkinName);
						continue;
					}
					else if (!mir_wstrcmp(L"[other]", buff)) {
						ParseOtherPart(fp, buff);
						continue;
					}
					else if (!bOnlyPreview) {
						if (!mir_wstrcmp(L"[background]", buff)) {
							ParseImagePart(fp, buff, SKIN_ITEM_BG);
							continue;
						}
						else if (!mir_wstrcmp(L"[sidebar]", buff)) {
							ParseImagePart(fp, buff, SKIN_ITEM_SIDEBAR);
							continue;
						}
						else if (!bStartup && opt.bLoadFonts && !mir_wstrcmp(L"[fonts]", buff)) {
							ParseFontPart(fp, buff);
							continue;
						}
						else if (!bStartup && opt.bLoadProportions && !mir_wstrcmp(L"[appearance]", buff)) {
							ParseAppearancePart(fp, buff);
							continue;
						}
					}
				}

				myfgets(buff, 1024, fp);
			}
			fclose(fp);
		}
	}
	else {
		opt.skinMode = SM_COLORFILL;
	}

	FindClose(hFind);
	SetCurrentDirectory(szDirSave);
}
