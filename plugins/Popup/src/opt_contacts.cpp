/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/opt_contacts.cpp $
Revision       : $Revision: 1610 $
Last change on : $Date: 2010-06-23 00:55:13 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

static void sttResetListOptions(HWND hwndList);
static void sttSetAllContactIcons(HWND hwndList);

INT_PTR CALLBACK DlgProcContactOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);

			SendMessage(GetDlgItem(hwnd, IDC_ICO_AUTO),			STM_SETICON, (WPARAM)IcoLib_GetIcon(ICO_OPT_DEF,0), 0);
			SendMessage(GetDlgItem(hwnd, IDC_ICO_FAVORITE),		STM_SETICON, (WPARAM)IcoLib_GetIcon(ICO_OPT_FAV,0), 0);
			SendMessage(GetDlgItem(hwnd, IDC_ICO_FULLSCREEN),	STM_SETICON, (WPARAM)IcoLib_GetIcon(ICO_OPT_FULLSCREEN,0), 0);
			SendMessage(GetDlgItem(hwnd, IDC_ICO_BLOCK),		STM_SETICON, (WPARAM)IcoLib_GetIcon(ICO_OPT_BLOCK,0), 0);

			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),
					(IsWinVerXPPlus()?ILC_COLOR32:ILC_COLOR16)|ILC_MASK,5,5);
			ImageList_AddIcon(hIml, LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
			ImageList_AddIcon(hIml, IcoLib_GetIcon(ICO_OPT_DEF,0));
			ImageList_AddIcon(hIml, IcoLib_GetIcon(ICO_OPT_FAV,0));
			ImageList_AddIcon(hIml, IcoLib_GetIcon(ICO_OPT_FULLSCREEN,0));
			ImageList_AddIcon(hIml, IcoLib_GetIcon(ICO_OPT_BLOCK,0));
			SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRACOLUMNS, 4 /*SIZEOF(sttIcons)*/, 0);
			sttResetListOptions(GetDlgItem(hwnd, IDC_LIST));
			sttSetAllContactIcons(GetDlgItem(hwnd, IDC_LIST));

			break;
		}

		case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case IDC_LIST:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case CLN_NEWCONTACT:
						case CLN_LISTREBUILT:
							sttSetAllContactIcons(GetDlgItem(hwnd,IDC_LIST));
							break;
						case CLN_OPTIONSCHANGED:
							sttResetListOptions(GetDlgItem(hwnd,IDC_LIST));
							break;
						case NM_CLICK:
						{
							HANDLE hItem;
							NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
							DWORD hitFlags;
							int iImage;

							if(nm->iColumn==-1) break;
							hItem=(HANDLE)SendDlgItemMessage(hwnd,IDC_LIST,CLM_HITTEST,(WPARAM)&hitFlags,MAKELPARAM(nm->pt.x,nm->pt.y));
							if(hItem==NULL) break;
							if(!(hitFlags&CLCHT_ONITEMEXTRA)) break;

							iImage=SendDlgItemMessage(hwnd,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(nm->iColumn,0));
							if (iImage != 0xFF)
							{
								for (int i = 0; i < 4 /*SIZEOF(sttIcons)*/; ++i)
									//hIml element [0]    = SKINICON_OTHER_SMALLDOT
									//hIml element [1..5] = IcoLib_GetIcon(....)   ~ old sttIcons
									SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, (i==nm->iColumn)?i+1:0));
							}
/*
							{
								while (hItem=(HANDLE)SendDlgItemMessage(hwnd,IDC_LIST,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem))
								{
									for (int i = 0; i < SIZEOF(sttIcons); ++i)
										SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, (i==nm->iColumn)?i+1:0));
									hItem = (HANDLE)SendDlgItemMessage(hwnd,IDC_LIST,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
								}
							}
*/
							SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
							break;
						}
					}
					break;
				}

				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
							for (HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0); hContact;
									hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
							{
								HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
								for (int i = 0; i < 4 /*SIZEOF(sttIcons)*/; ++i)
								{
									if (SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0)))
									{
										DBWriteContactSettingByte(hContact, MODULNAME, "ShowMode", i);
										break;
									}
								}
							}
							return TRUE;
						}
					}
					break;
				}
			}
			break;
		}
	}

	return FALSE;
}

static void sttResetListOptions(HWND hwndList)
{
	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,4,0);
	SendMessage(hwndList,CLM_SETINDENT,20,0);
	SendMessage(hwndList,CLM_SETHIDEEMPTYGROUPS,1,0);
	for(int i=0;i<=FONTID_MAX;i++)
		SendMessage(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
}

static void sttSetAllContactIcons(HWND hwndList)
{
	for (HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0); hContact;
			hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
		DWORD dwMode = DBGetContactSettingByte(hContact, MODULNAME, "ShowMode", 0);
		for (int i = 0; i < 4 /*SIZEOF(sttIcons)*/; ++i)
			//hIml element [0]    = SKINICON_OTHER_SMALLDOT
			//hIml element [1..5] = IcoLib_GetIcon(....)   ~ old sttIcons
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, (dwMode==i)?i+1:0));
	}
}
