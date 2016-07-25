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

	TCHAR szDirSave[1024];
	GetCurrentDirectory(1024, szDirSave);
	SetCurrentDirectory(SKIN_FOLDER);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"*.*", &ffd);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mir_tstrcmp(L".", ffd.cFileName) && mir_tstrcmp(L"..", ffd.cFileName)) 
		{
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

bool FileExists(TCHAR *filename)
{
	HANDLE hFile = CreateFile(filename, 0, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return true;
	}

	return false;
}

void ParseAboutPart(FILE *fp, TCHAR *buff, TCHAR *szSkinName)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[')
	{
		if (buff[0] != ';') 
		{
			TCHAR *pch = _tcschr(buff, '=');
			if (pch++) 
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch)
				{
					if (_tcsstr(buff, L"author"))
					{}
					else if (_tcsstr(buff, L"preview")) 
					{
						TCHAR szImgPath[1024];
						mir_sntprintf(szImgPath, L"%s\\%s\\%s", SKIN_FOLDER, szSkinName, pch);
						if (FileExists(szImgPath))
							mir_tstrcpy(opt.szPreviewFile, szImgPath);
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseImagePart(FILE *fp, TCHAR *buff, int iPart)
{
	opt.szImgFile[iPart] = NULL;
	opt.transfMode[iPart] = TM_NONE;
	opt.margins[iPart].left = 0;
	opt.margins[iPart].top = 0;
	opt.margins[iPart].right = 0;
	opt.margins[iPart].bottom = 0;

	myfgets(buff, 1024, fp);
	while (buff[0] != '[') 
	{
		if (buff[0] != ';')
		{
			TCHAR *pch = _tcschr(buff, '=');
			if (pch++) 
			{
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch)
				{
					if (_tcsstr(buff, L"image"))
					{
						TCHAR szImgPath[1024];
						mir_sntprintf(szImgPath, L"%s\\%s\\%s", SKIN_FOLDER, opt.szSkinName, pch);
						opt.szImgFile[iPart] = mir_tstrdup(szImgPath);
					}
					else if (_tcsstr(buff, L"tm"))
					{
						if (!mir_tstrcmpi(pch, L"TM_NONE"))
							opt.transfMode[iPart] = TM_NONE;
						else if (!mir_tstrcmpi(pch, L"TM_CENTRE"))
							opt.transfMode[iPart] = TM_CENTRE;
						else if (!mir_tstrcmpi(pch, L"TM_STRECH_ALL"))
							opt.transfMode[iPart] = TM_STRECH_ALL;
						else if (!mir_tstrcmpi(pch, L"TM_STRECH_HORIZONTAL"))
							opt.transfMode[iPart] = TM_STRECH_HORIZONTAL;
						else if (!mir_tstrcmpi(pch, L"TM_STRECH_VERTICAL"))
							opt.transfMode[iPart] = TM_STRECH_VERTICAL;
						else if (!mir_tstrcmpi(pch, L"TM_TILE_ALL"))
							opt.transfMode[iPart] = TM_TILE_ALL;
						else if (!mir_tstrcmpi(pch, L"TM_TILE_HORIZONTAL"))
							opt.transfMode[iPart] = TM_TILE_HORIZONTAL;
						else if (!mir_tstrcmpi(pch, L"TM_TILE_VERTICAL"))
							opt.transfMode[iPart] = TM_TILE_VERTICAL;
						else 
							opt.transfMode[iPart] = TM_NONE;
					}
					else if (_tcsstr(buff, L"left"))
						opt.margins[iPart].left = _ttoi(pch);
					else if (_tcsstr(buff, L"top"))
						opt.margins[iPart].top = _ttoi(pch);
					else if (_tcsstr(buff, L"right"))
						opt.margins[iPart].right = _ttoi(pch);
					else if (_tcsstr(buff, L"bottom"))
						opt.margins[iPart].bottom = _ttoi(pch);
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

char *GetSettingName(TCHAR *szValue, char *szPostfix, char *buff, size_t buffsize)
{
	buff[0] = 0;

	if (_tcsstr(szValue, L"traytitle"))
		mir_snprintf(buff, buffsize, "FontTrayTitle%s", szPostfix);
	else if (_tcsstr(szValue, L"title"))
		mir_snprintf(buff, buffsize, "FontFirst%s", szPostfix);
	else if (_tcsstr(szValue, L"label"))
		mir_snprintf(buff, buffsize, "FontLabels%s", szPostfix);
	else if (_tcsstr(szValue, L"value"))
		mir_snprintf(buff, buffsize, "FontValues%s", szPostfix);
	else if (_tcsstr(szValue, L"divider"))
		mir_snprintf(buff, buffsize, "Divider%s", szPostfix);

	return buff[0] ? buff : NULL;
}

void ParseFontPart(FILE *fp, TCHAR *buff)
{
	char szSetting[64];

	myfgets(buff, 1024, fp);
	while (buff[0] != '[')
	{
		if (buff[0] != ';') 
		{
			TCHAR *pch = _tcschr(buff, '=');
			if (pch++)
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch) 
				{
					if (_tcsstr(buff, L"face"))
					{
						if (GetSettingName(buff, "", szSetting, sizeof(szSetting) - 1)) 
						{
							if (mir_tstrlen(pch) > 32)
								pch[32] = 0;

							db_set_ts(0, MODULE, szSetting, pch);
						}
					} 
					else if (_tcsstr(buff, L"color"))
					{
						if (GetSettingName(buff, "Col", szSetting, sizeof(szSetting) - 1))
						{
							BYTE r = _ttoi(pch);
							pch = _tcschr(pch, ' ');
							if (++pch)
							{
								BYTE g = _ttoi(pch); 
								pch = _tcschr(pch, ' ');
								if (++pch) 
								{
									BYTE b = _ttoi(pch);
									COLORREF color = RGB(r, g ,b);
									db_set_dw(0, MODULE, szSetting, color);
								}
							}
						}
					} 
					else if (_tcsstr(buff, L"size"))
					{
						if (GetSettingName(buff, "Size", szSetting, sizeof(szSetting) - 1)) 
						{
							HDC hdc = GetDC(0);
							int size = -MulDiv(_ttoi(pch), GetDeviceCaps(hdc, LOGPIXELSY), 72);
							db_set_b(0, MODULE, szSetting, (BYTE)size);
							ReleaseDC(0, hdc);
						}
					} 
					else if (_tcsstr(buff, L"effect"))
					{
						if (GetSettingName(buff, "Sty", szSetting, sizeof(szSetting) - 1))
						{
							BYTE effect = 0;
							if (_tcsstr(pch, L"font_bold"))
								effect |= DBFONTF_BOLD;
							if (_tcsstr(pch, L"font_italic"))
								effect |= DBFONTF_ITALIC;
							if (_tcsstr(pch, L"font_underline"))
								effect |= DBFONTF_UNDERLINE;

							db_set_b(0, MODULE, szSetting, effect);
						}
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseAppearancePart(FILE *fp, TCHAR *buff)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[') 
	{
		if (buff[0] != ';') 
		{
			TCHAR *pch = _tcschr(buff, '=');
			if (pch++) 
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch)
				{
					if (_tcsstr(buff, L"general-padding"))
						opt.iPadding = _ttoi(pch);
					else if (_tcsstr(buff, L"title-indent"))
						opt.iTitleIndent = _ttoi(pch);
					else if (_tcsstr(buff, L"text-indent"))
						opt.iTextIndent = _ttoi(pch);
					else if (_tcsstr(buff, L"value-indent"))
						opt.iValueIndent = _ttoi(pch);
					else if (_tcsstr(buff, L"text-padding"))
						opt.iTextPadding = _ttoi(pch);
					else if (_tcsstr(buff, L"outer-avatar-padding"))
						opt.iOuterAvatarPadding = _ttoi(pch);
					else if (_tcsstr(buff, L"inner-avatar-padding"))
						opt.iInnerAvatarPadding = _ttoi(pch);
					else if (_tcsstr(buff, L"sidebar-width"))
						opt.iSidebarWidth = _ttoi(pch);
					else if (_tcsstr(buff, L"opacity"))
						opt.iOpacity = _ttoi(pch);
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseOtherPart(FILE *fp, TCHAR *buff)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[')
	{
		if (buff[0] != ';')
		{
			TCHAR *pch = _tcschr(buff, '=');
			if (pch++)
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch) 
				{
					if (_tcsstr(buff, L"enable-coloring"))
					{
						if (_tcsstr(pch, L"false"))
							opt.iEnableColoring = -1;
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseSkinFile(TCHAR *szSkinName, bool bStartup, bool bOnlyPreview)
{
	TCHAR szDirSave[1024], buff[1024];

	if (opt.skinMode == SM_OBSOLOTE && bStartup)
		return;
	
	if (!bStartup) opt.iEnableColoring = 0;
	opt.szPreviewFile[0] = 0;

	GetCurrentDirectory(1024, szDirSave);
	SetCurrentDirectory(SKIN_FOLDER);
	SetCurrentDirectory(szSkinName);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"*.tsf", &ffd);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		FILE *fp = _tfopen(ffd.cFileName, L"r");
		if (fp)
		{
			myfgets(buff, 1024, fp);
			while (!feof(fp)) 
			{
				if (buff[0] == '[') 
				{
					if (!mir_tstrcmp(L"[about]", buff)) 
					{
						ParseAboutPart(fp, buff, szSkinName);
						continue;
					} 
					else if (!mir_tstrcmp(L"[other]", buff)) 
					{
						ParseOtherPart(fp, buff);
						continue;
					} 
					else if (!bOnlyPreview) 
					{
						if (!mir_tstrcmp(L"[background]", buff))
						{
							ParseImagePart(fp, buff, SKIN_ITEM_BG);
							continue;
						} 
						else if (!mir_tstrcmp(L"[sidebar]", buff)) 
						{
							ParseImagePart(fp, buff, SKIN_ITEM_SIDEBAR);
							continue;
						}
						else if (!bStartup && opt.bLoadFonts && !mir_tstrcmp(L"[fonts]", buff))
						{
							ParseFontPart(fp, buff);
							continue;
						} 
						else if (!bStartup && opt.bLoadProportions && !mir_tstrcmp(L"[appearance]", buff))
						{
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
	else
	{
		opt.skinMode = SM_COLORFILL;
	}

	FindClose(hFind);
	SetCurrentDirectory(szDirSave);
}