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

#include "common.h"
#include "options.h"
#include "str_utils.h"

extern char SKIN_FOLDER[256];

int RefreshSkinList(HWND hwndDlg)
{
	HWND hwndSkins = GetDlgItem(hwndDlg, IDC_LB_SKINS);
	ListBox_ResetContent(hwndSkins);
	ListBox_AddString(hwndSkins, TranslateT("# Solid color fill"));

	char szDirSave[1024];
	GetCurrentDirectoryA(1024, szDirSave);
	SetCurrentDirectoryA(SKIN_FOLDER);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(_T("*.*"), &ffd);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && _tcscmp(_T("."), ffd.cFileName) && _tcscmp(_T(".."), ffd.cFileName)) 
		{
			SetCurrentDirectory(ffd.cFileName);
			WIN32_FIND_DATA ffd2;
			HANDLE hFindFile = FindFirstFile(_T("*.tsf"), &ffd2);
			if (hFindFile != INVALID_HANDLE_VALUE)
				ListBox_AddString(hwndSkins, ffd.cFileName);

			FindClose(hFindFile);
			SetCurrentDirectory(_T(".."));
		}

		if (!FindNextFile(hFind, &ffd))
			break;
	}

	FindClose(hFind);
	SetCurrentDirectoryA(szDirSave);

	TCHAR swzSkinName[256];
	a2t(opt.szSkinName, swzSkinName, 256);
	return ListBox_SelectString(GetDlgItem(hwndDlg, IDC_LB_SKINS), -1, swzSkinName);
}

bool FileExists(char *filename)
{
	HANDLE hFile = CreateFileA(filename, 0, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return true;
	}

	return false;
}

void ParseAboutPart(FILE *fp, char *buff, char *szSkinName)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[')
	{
		if (buff[0] != ';') 
		{
			char *pch = strchr(buff, '=');
			if (pch++) 
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch)
				{
					if (strstr(buff, "author"))
					{}
					else if (strstr(buff, "preview")) 
					{
						char szImgPath[1024];
						mir_snprintf(szImgPath, SIZEOF(szImgPath), "%s\\%s\\%s", SKIN_FOLDER, szSkinName, pch);
						if (FileExists(szImgPath))
							strcpy(opt.szPreviewFile, szImgPath);
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseImagePart(FILE *fp, char *buff, int iPart)
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
			char *pch = strchr(buff, '=');
			if (pch++) 
			{
				while (pch && (*pch == ' ' || *pch == '\t'))
					pch++;

				if (pch)
				{
					if (strstr(buff, "image"))
					{
						char szImgPath[1024];
						mir_snprintf(szImgPath, SIZEOF(szImgPath), "%s\\%s\\%s", SKIN_FOLDER, opt.szSkinName, pch);
						opt.szImgFile[iPart] = mir_strdup(szImgPath);
					}
					else if (strstr(buff, "tm"))
					{
						if (!lstrcmpiA(pch, "TM_NONE"))
							opt.transfMode[iPart] = TM_NONE;
						else if (!lstrcmpiA(pch, "TM_CENTRE"))
							opt.transfMode[iPart] = TM_CENTRE;
						else if (!lstrcmpiA(pch, "TM_STRECH_ALL"))
							opt.transfMode[iPart] = TM_STRECH_ALL;
						else if (!lstrcmpiA(pch, "TM_STRECH_HORIZONTAL"))
							opt.transfMode[iPart] = TM_STRECH_HORIZONTAL;
						else if (!lstrcmpiA(pch, "TM_STRECH_VERTICAL"))
							opt.transfMode[iPart] = TM_STRECH_VERTICAL;
						else if (!lstrcmpiA(pch, "TM_TILE_ALL"))
							opt.transfMode[iPart] = TM_TILE_ALL;
						else if (!lstrcmpiA(pch, "TM_TILE_HORIZONTAL"))
							opt.transfMode[iPart] = TM_TILE_HORIZONTAL;
						else if (!lstrcmpiA(pch, "TM_TILE_VERTICAL"))
							opt.transfMode[iPart] = TM_TILE_VERTICAL;
						else 
							opt.transfMode[iPart] = TM_NONE;
					}
					else if (strstr(buff, "left")) 
						opt.margins[iPart].left = atoi(pch);
					else if (strstr(buff, "top")) 
						opt.margins[iPart].top = atoi(pch);
					else if (strstr(buff, "right"))
						opt.margins[iPart].right = atoi(pch);
					else if (strstr(buff, "bottom")) 
						opt.margins[iPart].bottom = atoi(pch);
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

char *GetSettingName(char *szValue, char *szPostfix, char *buff)
{
	buff[0] = 0;

	if (strstr(szValue, "traytitle"))
		mir_snprintf(buff, 64, "FontTrayTitle%s", szPostfix);
	else if (strstr(szValue, "title"))
		mir_snprintf(buff, 64, "FontFirst%s", szPostfix);
	else if (strstr(szValue, "label"))
		mir_snprintf(buff, 64, "FontLabels%s", szPostfix);
	else if (strstr(szValue, "value"))
		mir_snprintf(buff, 64, "FontValues%s", szPostfix);
	else if (strstr(szValue, "divider"))
		mir_snprintf(buff, 64, "Divider%s", szPostfix);

	if (buff[0]) return buff;
	else return NULL;
}

void ParseFontPart(FILE *fp, char *buff)
{
	char szSetting[64];

	myfgets(buff, 1024, fp);
	while (buff[0] != '[')
	{
		if (buff[0] != ';') 
		{
			char *pch = strchr(buff, '=');
			if (pch++)
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch) 
				{
					if (strstr(buff, "face"))
					{
						if (GetSettingName(buff, "", szSetting)) 
						{
							if (strlen(pch) > 32)
								pch[32] = 0;

							TCHAR *tmp = mir_a2t(pch);
							DBWriteContactSettingTString(0, MODULE, szSetting, tmp);
							mir_free(tmp);
						}
					} 
					else if (strstr(buff, "color"))
					{
						if (GetSettingName(buff, "Col", szSetting))
						{
							BYTE r = atoi(pch);
							pch = strchr(pch, ' ');
							if (++pch)
							{
								BYTE g = atoi(pch); 
								pch = strchr(pch, ' ');
								if (++pch) 
								{
									BYTE b = atoi(pch);
									COLORREF color = RGB(r, g ,b);
									DBWriteContactSettingDword(0, MODULE, szSetting, color);
								}
							}
						}
					} 
					else if (strstr(buff, "size"))
					{
						if (GetSettingName(buff, "Size", szSetting)) 
						{
							HDC hdc = GetDC(0);
							int size = -MulDiv(atoi(pch), GetDeviceCaps(hdc, LOGPIXELSY), 72);
							DBWriteContactSettingByte(0, MODULE, szSetting, (BYTE)size);
							ReleaseDC(0, hdc);
						}
					} 
					else if (strstr(buff, "effect")) 
					{
						if (GetSettingName(buff, "Sty", szSetting))
						{
							BYTE effect = 0;
							if (strstr(pch, "font_bold"))
								effect |= DBFONTF_BOLD;
							if (strstr(pch, "font_italic"))
								effect |= DBFONTF_ITALIC;
							if (strstr(pch, "font_underline"))
								effect |= DBFONTF_UNDERLINE;

							DBWriteContactSettingByte(0, MODULE, szSetting, effect);
						}
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseAppearancePart(FILE *fp, char *buff)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[') 
	{
		if (buff[0] != ';') 
		{
			char *pch = strchr(buff, '=');
			if (pch++) 
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch)
				{
					if (strstr(buff, "general-padding"))
						opt.iPadding = atoi(pch);
					else if (strstr(buff, "title-indent"))
						opt.iTitleIndent = atoi(pch);
					else if (strstr(buff, "text-indent"))
						opt.iTextIndent = atoi(pch);
					else if (strstr(buff, "value-indent"))
						opt.iValueIndent = atoi(pch);
					else if (strstr(buff, "text-padding"))
						opt.iTextPadding = atoi(pch);
					else if (strstr(buff, "outer-avatar-padding"))
						opt.iOuterAvatarPadding = atoi(pch);
					else if (strstr(buff, "inner-avatar-padding"))
						opt.iInnerAvatarPadding = atoi(pch);
					else if (strstr(buff, "sidebar-width"))
						opt.iSidebarWidth = atoi(pch);
					else if (strstr(buff, "opacity"))
						opt.iOpacity = atoi(pch);
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseOtherPart(FILE *fp, char *buff)
{
	myfgets(buff, 1024, fp);
	while (buff[0] != '[')
	{
		if (buff[0] != ';')
		{
			char *pch = strchr(buff, '=');
			if (pch++)
			{
				while (pch && (*pch == ' ' || *pch == '\t')) 
					pch++;

				if (pch) 
				{
					if (strstr(buff, "enable-coloring"))
					{
						if (strstr(pch, "false"))
							opt.iEnableColoring = -1;
					}
				}
			}
		}

		if (feof(fp)) break;
		myfgets(buff, 1024, fp);
	}
}

void ParseSkinFile(char *szSkinName, bool bStartup, bool bOnlyPreview)
{
	char szDirSave[1024], buff[1024];

	if (opt.skinMode == SM_OBSOLOTE && bStartup)
		return;
	
	if (!bStartup) opt.iEnableColoring = 0;
	opt.szPreviewFile[0] = 0;

	GetCurrentDirectoryA(1024, szDirSave);
	SetCurrentDirectoryA(SKIN_FOLDER);
	SetCurrentDirectoryA(szSkinName);

	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA("*.tsf", &ffd);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		FILE *fp = fopen(ffd.cFileName, "r");
		if (fp)
		{
			myfgets(buff, 1024, fp);
			while (!feof(fp)) 
			{
				if (buff[0] == '[') 
				{
					if (!strcmp("[about]", buff)) 
					{
						ParseAboutPart(fp, buff, szSkinName);
						continue;
					} 
					else if (!strcmp("[other]", buff)) 
					{
						ParseOtherPart(fp, buff);
						continue;
					} 
					else if (!bOnlyPreview) 
					{
						if (!strcmp("[background]", buff))
						{
							ParseImagePart(fp, buff, SKIN_ITEM_BG);
							continue;
						} 
						else if (!strcmp("[sidebar]", buff)) 
						{
							ParseImagePart(fp, buff, SKIN_ITEM_SIDEBAR);
							continue;
						}
						else if (!bStartup && opt.bLoadFonts && !strcmp("[fonts]", buff))
						{
							ParseFontPart(fp, buff);
							continue;
						} 
						else if (!bStartup && opt.bLoadProportions && !strcmp("[appearance]", buff))
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
	SetCurrentDirectoryA(szDirSave);
}