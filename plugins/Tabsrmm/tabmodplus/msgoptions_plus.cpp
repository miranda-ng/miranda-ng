/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: msgoptions_plus.cpp 13034 2010-10-24 20:39:04Z silvercircle $
 *
 * implements the "advanced tweak" option page
 *
 * originally developed by Mad Cluster for the tabSRMM "MADMOD" patch in
 * May 2008.
 *
 */

#include "../src/commonheaders.h"

extern HINSTANCE	hinstance;
extern BOOL			g_bIMGtagButton;
extern HIMAGELIST	g_himlOptions, CreateStateImageList();

INT_PTR CALLBACK PlusOptionsProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)	{

		case WM_INITDIALOG:	{
			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), GWL_STYLE) | (TVS_NOHSCROLL | TVS_CHECKBOXES));

			g_himlOptions = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_PLUS_CHECKTREE, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)CreateStateImageList());
			ImageList_Destroy(g_himlOptions);

			/*
			* fill the list box, create groups first, then add items
			*/

			SendMessage(hwndDlg, WM_USER + 100, 0, 0);

			return TRUE;
		}
		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->idFrom) {
				case IDC_PLUS_CHECKTREE:
					if (((LPNMHDR)lParam)->code == NM_CLICK || (((LPNMHDR)lParam)->code == TVN_KEYDOWN && ((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)) {
						TVHITTESTINFO hti;
						TVITEM item = {0};

						item.mask = TVIF_HANDLE | TVIF_STATE;
						item.stateMask = TVIS_STATEIMAGEMASK | TVIS_BOLD;
						hti.pt.x = (short)LOWORD(GetMessagePos());
						hti.pt.y = (short)HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
						if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti) || ((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
							if (((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
								hti.flags |= TVHT_ONITEMSTATEICON;
								item.hItem = TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom);
							} else
								item.hItem = (HTREEITEM)hti.hItem;
							SendDlgItemMessageA(hwndDlg, IDC_PLUS_CHECKTREE, TVM_GETITEMA, 0, (LPARAM)&item);
							if (item.state & TVIS_BOLD && hti.flags & TVHT_ONITEMSTATEICON) {
								item.state = INDEXTOSTATEIMAGEMASK(0) | TVIS_BOLD;
								SendDlgItemMessageA(hwndDlg, IDC_PLUS_CHECKTREE, TVM_SETITEMA, 0, (LPARAM)&item);
							} else if (hti.flags & TVHT_ONITEMSTATEICON) {

								if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 3) {
									item.state = INDEXTOSTATEIMAGEMASK(1);
									SendDlgItemMessageA(hwndDlg, IDC_PLUS_CHECKTREE, TVM_SETITEMA, 0, (LPARAM)&item);
								}

								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}
					}
					break;
				default:
					switch (((LPNMHDR) lParam)->code) {
						case PSN_APPLY: {
							int		i = 0;
							TVITEM	item = {0};
							DWORD	msgTimeout;
							/*
							* scan the tree view and obtain the options...
							*/
							TOptionListItem* lvItems = CTranslator::getTree(CTranslator::TREE_MODPLUS);

							while (lvItems[i].szName != NULL) {
								item.mask = TVIF_HANDLE | TVIF_STATE;
								item.hItem = (HTREEITEM)lvItems[i].handle;
								item.stateMask = TVIS_STATEIMAGEMASK;

								SendDlgItemMessageA(hwndDlg, IDC_PLUS_CHECKTREE, TVM_GETITEMA, 0, (LPARAM)&item);
								if (lvItems[i].uType == LOI_TYPE_SETTING)
									M->WriteByte(SRMSGMOD_T, (char *)lvItems[i].lParam, (BYTE)((item.state >> 12) == 3/*2*/ ? 1 : 0));  // NOTE: state image masks changed
								i++;
							}

							msgTimeout = 1000 * GetDlgItemInt(hwndDlg, IDC_SECONDS, NULL, FALSE);
							PluginConfig.m_MsgTimeout = msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout : SRMSGSET_MSGTIMEOUT_MIN;
							M->WriteDword(SRMSGMOD, SRMSGSET_MSGTIMEOUT, PluginConfig.m_MsgTimeout);

							M->WriteByte(SRMSGMOD_T, "historysize", (BYTE)SendDlgItemMessage(hwndDlg, IDC_HISTORYSIZESPIN, UDM_GETPOS, 0, 0));
							PluginConfig.reloadAdv();
							return TRUE;
						}
					}
					break;
				}
				break;
		case WM_COMMAND: {
			if(LOWORD(wParam) == IDC_PLUS_HELP) {
				CallService(MS_UTILS_OPENURL, 0, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/AdvancedTweaks");
				break;
			}
			else if(LOWORD(wParam) == IDC_PLUS_REVERT) {		// revert to defaults...
				int i = 0;

				TOptionListItem *lvItems = CTranslator::getTree(CTranslator::TREE_MODPLUS);

				while(lvItems[i].szName) {
					if(lvItems[i].uType == LOI_TYPE_SETTING)
						M->WriteByte(SRMSGMOD_T, (char *)lvItems[i].lParam, (BYTE)lvItems[i].id);
					i++;
				}
				TreeView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE));
				SendMessage(hwndDlg, WM_USER + 100, 0, 0);		// fill dialog
				break;
			}
			if (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus())
				return TRUE;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		/*
		 * fill dialog
		 */
		case WM_USER + 100: {
			TVINSERTSTRUCT tvi = {0};
			int		i = 0;

			TOptionListGroup *lvGroups = CTranslator::getGroupTree(CTranslator::TREE_MODPLUS);

			while (lvGroups[i].szName != NULL) {
				tvi.hParent = 0;
				tvi.hInsertAfter = TVI_LAST;
				tvi.item.mask = TVIF_TEXT | TVIF_STATE;
				tvi.item.pszText = lvGroups[i].szName;
				tvi.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_EXPANDED | TVIS_BOLD;
				tvi.item.state = INDEXTOSTATEIMAGEMASK(0) | TVIS_EXPANDED | TVIS_BOLD;
				lvGroups[i++].handle = (LRESULT)TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), &tvi);
			}

			i = 0;

			TOptionListItem *lvItems = CTranslator::getTree(CTranslator::TREE_MODPLUS);

			while (lvItems[i].szName != 0) {
				tvi.hParent = (HTREEITEM)lvGroups[lvItems[i].uGroup].handle;
				tvi.hInsertAfter = TVI_LAST;
				tvi.item.pszText = lvItems[i].szName;
				tvi.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
				tvi.item.lParam = i;
				tvi.item.stateMask = TVIS_STATEIMAGEMASK;
				//if (lvItems[i].uType == LOI_TYPE_FLAG)
				//	tvi.item.state = INDEXTOSTATEIMAGEMASK((dwFlags & (UINT)lvItems[i].lParam) ? 3 : 2);
				if (lvItems[i].uType == LOI_TYPE_SETTING)
					tvi.item.state = INDEXTOSTATEIMAGEMASK(M->GetByte((char *)lvItems[i].lParam, lvItems[i].id) ? 3 : 2);  // NOTE: was 2 : 1 without state image mask
				lvItems[i].handle = (LRESULT)TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), &tvi);
				i++;
			}
			PluginConfig.g_bClientInStatusBar = M->GetByte("adv_ClientIconInStatusBar", 0);

			SendDlgItemMessage(hwndDlg, IDC_TIMEOUTSPIN, UDM_SETRANGE, 0, MAKELONG(300, SRMSGSET_MSGTIMEOUT_MIN / 1000));
			SendDlgItemMessage(hwndDlg, IDC_TIMEOUTSPIN, UDM_SETPOS, 0, PluginConfig.m_MsgTimeout / 1000);

			SendDlgItemMessage(hwndDlg, IDC_HISTORYSIZESPIN, UDM_SETRANGE, 0, MAKELONG(255, 15));
			SendDlgItemMessage(hwndDlg, IDC_HISTORYSIZESPIN, UDM_SETPOS, 0, (int)M->GetByte("historysize", 0));

			return 0;
		}
		case WM_CLOSE:
			EndDialog(hwndDlg,0);
			return 0;
		}
	return 0;
}
