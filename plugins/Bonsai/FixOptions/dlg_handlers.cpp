/*
Bonsai plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#include "dlg_handlers.h"
#include "commctrl.h"

#define CMP(a, b) ( ((a == b) ? 0 : ((a < b) ? -1 : 1)) )

HWND hOptDlg = NULL;

const char *szColumns[] = {"Plugin", "Current group", "Current title", "Initial group", "Initial title"};
const int cxColumns[] = {100, 120, 120, 120, 120};
const int cColumns = sizeof(szColumns) / sizeof(szColumns[0]);

WNDPROC oldPluginsListProc = NULL;

inline int SettingChanged(WCHAR *defaultSetting, WCHAR *currentSetting, int nullTrueValue, int trueValue, int falseValue)
{
	if (currentSetting == NULL)
	{
		return ((defaultSetting != NULL) && (wcslen(defaultSetting) != 0)) ? nullTrueValue : falseValue;
	}
	else{
		return (_wcsicmp(currentSetting, defaultSetting)) ? trueValue : falseValue;
	}
}

void UpdatePlugin(HWND hList, int index, PPluginOptions plugin, int newlyModified)
{
	const int MAX_SIZE = 256;
	WCHAR buffer[MAX_SIZE];
	
	GetPluginGroup(plugin, buffer, MAX_SIZE);
	ListView_SetItemTextW(hList, index, 1, buffer);
	
	GetPluginTitle(plugin, buffer, MAX_SIZE);
	ListView_SetItemTextW(hList, index, 2, buffer);
	
	if (newlyModified)
	{
		LVITEM item = {0};
		item.iItem = index;
		item.mask = LVIF_IMAGE;
		item.iImage = 3;
		ListView_SetItem(hList, &item);
	}
	
	ListView_SetItemTextW(hList, index, 3, plugin->szGroup);
	
	ListView_SetItemTextW(hList, index, 4, plugin->szTitle);
}

int ItemOptionEntryChanged(int index, PPluginOptions plugin)
{
	const int MAX_SIZE = 256;
	WCHAR buffer[MAX_SIZE];
	WCHAR *res;

	int changed = 0;
	
	res = GetPluginGroup(plugin, buffer, MAX_SIZE);
	changed = SettingChanged(plugin->szGroup, res, 1, 1, 0);
//	changed = (res == NULL) ? plugin->szGroup != NULL : (_wcsicmp(buffer, plugin->szGroup)) ? 1 : 0;
	
	res = GetPluginTitle(plugin, buffer, MAX_SIZE);
	changed = SettingChanged(plugin->szTitle, res, 2, 1, changed);
//	changed = (res == NULL) ? plugin->szTitle != NULL : (_wcsicmp(buffer, plugin->szTitle)) ? 1 : changed;
	
	return changed;
}

void LoadPlugins(HWND hWnd, int bAll = 1)
{
	HWND hList = GetDlgItem(hWnd, IDC_LIST_PLUGINS);

	if (bAll)
	{
		ListView_DeleteAllItems(hList);
	}
	
	int i;
	int init = (bAll) ? 0 : ListView_GetItemCount(hList);
	PPluginOptions plugin = NULL;
	
	LVITEMW item = {0};
	item.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
	for (i = init; i < lstPluginOptions.Count(); i++)
	{
		plugin = lstPluginOptions[i];
		item.iItem = i;
		item.lParam = (LPARAM) plugin;
		item.pszText = plugin->szPlugin;
		item.iImage = ItemOptionEntryChanged(i, plugin);
		ListView_InsertItemW(hList, &item);
		
		UpdatePlugin(hList, i, plugin, 0);
	}
}

struct SortParams{
	HWND hList;
	int column;
};

static int lastColumn = -1;

int CALLBACK PluginsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	SortParams params = *(SortParams *) myParam;
	int res;
	const int MAX_SIZE = 1024;
	
	wchar_t text1[MAX_SIZE];
	wchar_t text2[MAX_SIZE];
	
	ListView_GetItemTextW(params.hList, (int) lParam1, params.column, text1, MAX_SIZE);
	ListView_GetItemTextW(params.hList, (int) lParam2, params.column, text2, MAX_SIZE);
	
	res = _wcsicmp(text1, text2);
	
	res = (params.column == lastColumn) ? -res : res;
	
	return res;
}

int LoadColumns(HWND hList)
{
	int i;
	int start = 0;
	LVCOLUMN col;
	col.mask = LVCF_TEXT | LVCF_WIDTH;
	
	while (ListView_DeleteColumn(hList, 1)) //delete all columns
	{
		start = 1;
	}
	
	for (i = start; i < cColumns; i++)
	{
		col.pszText = TranslateTS(szColumns[i]);
	
		col.cx = cxColumns[i];
	
		ListView_InsertColumn(hList, i, &col);
	}
	
	return 0;
}

void LoadPluginInfo(HWND hWnd, PPluginOptions plugin)
{
	if (plugin)
	{
		const int MAX_SIZE = 256;
		WCHAR buffer[MAX_SIZE];
		
		GetPluginGroup(plugin, buffer, MAX_SIZE);
		SetWindowTextW(GetDlgItem(hWnd, IDC_GROUP), buffer);
		
		GetPluginTitle(plugin, buffer, MAX_SIZE);
		SetWindowTextW(GetDlgItem(hWnd, IDC_TITLE), buffer);
	}
}

PPluginOptions GetSelectedPlugin(HWND hWnd, int *index = NULL)
{
	int i;
	HWND hList = GetDlgItem(hWnd, IDC_LIST_PLUGINS);
	int count = ListView_GetItemCount(hList);
	for (i = 0; i < count; i++)
	{
		if (ListView_GetItemState(hList, i, LVIS_SELECTED) == LVIS_SELECTED)
		{
			LVITEM item = {0};
			item.iItem = i;
			item.mask = LVIF_PARAM;
			
			if (index)
			{
				*index = i;
			}
			
			ListView_GetItem(hList, &item);
			return (PPluginOptions) item.lParam;
		}
	}
	
	if (index)
	{
		*index = -1;
	}
	return NULL;
}

INT_PTR CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			bInitializing = 1;
			hOptDlg = hWnd;
			HWND hList = GetDlgItem(hWnd, IDC_LIST_PLUGINS);
			
			TranslateDialogDefault(hWnd);

			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
			//ListView_SetImageList(hList, imgList, LVSIL_NORMAL);
			ImageList_SetBkColor(imgList, RGB(255, 255, 255));
			ListView_SetImageList(hList, imgList, LVSIL_SMALL);

			//ListView_SetImageList(hList, imgList, LVSIL_STATE);
			
			LoadColumns(hList);
			
			LoadPlugins(hWnd);
			
			SortParams params = {0};
			params.hList = hList;
			params.column = DBGetContactSettingByte(NULL, ModuleName, "LastSortColumn", -1);
			ListView_SortItemsEx(hList, PluginsCompare, &params);
			
			EnableWindow(GetDlgItem(hWnd, IDC_SAVE), 0);
			EnableWindow(GetDlgItem(hWnd, IDC_CANCEL), 0);
			EnableWindow(GetDlgItem(hWnd, IDC_GROUP), 0);
			EnableWindow(GetDlgItem(hWnd, IDC_TITLE), 0);
			
			bInitializing = 0;
			return TRUE;
			break;
		}
		
		case WM_DESTROY:
		{
			DBWriteContactSettingByte(NULL, ModuleName, "LastSortColumn", lastColumn);
			hOptDlg = NULL;
			
			lastColumn = -1;
		
			break;
		}
		
		case FOM_UPDATEPLUGINS:
		{
			LoadPlugins(hWnd, 0);
			
			break;
		}
		
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_GROUP:
				case IDC_TITLE:
				{
					if ((HIWORD(wParam) == EN_CHANGE))
					{
						if (!bInitializing)
						{
							EnableWindow(GetDlgItem(hWnd, IDC_SAVE), 1);
							EnableWindow(GetDlgItem(hWnd, IDC_CANCEL), 1);
						}
					}
					
					break;
				}
				
				case IDC_SAVE:
				{
					int index;
					PPluginOptions plugin = GetSelectedPlugin(hWnd, &index);
					if (plugin)
					{
						WCHAR buffer[256];
						int changed = 0;
						
						GetWindowTextW(GetDlgItem(hWnd, IDC_GROUP), buffer, 256);
						SavePluginGroup(plugin, buffer);
						changed = (_wcsicmp(plugin->szGroup, buffer) != 0) ? 1 : 0;
						
						GetWindowTextW(GetDlgItem(hWnd, IDC_TITLE), buffer, 256);
						SavePluginTitle(plugin, buffer);
						changed = (_wcsicmp(plugin->szTitle, buffer) != 0) ? 1 : changed;
						
						UpdatePlugin(GetDlgItem(hWnd, IDC_LIST_PLUGINS), index, plugin, 1);
						
					}
					EnableWindow(GetDlgItem(hWnd, IDC_SAVE), 0);
					
					break;
				}
				
				case IDC_CANCEL:
				{
					int state = GetKeyState(VK_CONTROL);
					if (state & (1 << 8)) //ctrl pressed
					{
						if (MessageBox(hWnd, Translate("This will reset all plugins to their default group and title.\nDo you wish to reset all plugins to their default group and title ?"), Translate("Proceed ?"), MB_YESNO | MB_ICONWARNING) == IDYES)
						{
							int i;
							HWND hList = GetDlgItem(hWnd, IDC_LIST_PLUGINS);
							int count = ListView_GetItemCount(hList);
							LVITEM item = {0};
							item.mask = LVIF_PARAM;
							
							PPluginOptions plugin;
							
							for (i = 0; i < count; i++)
							{
								item.iItem = i;
								ListView_GetItem(hList, &item);
								plugin = (PPluginOptions) item.lParam;
								SavePluginGroup(plugin, plugin->szGroup);
								SavePluginTitle(plugin, plugin->szTitle);
								UpdatePlugin(hList, i, plugin, 1);
							}
							
							LoadPluginInfo(hWnd, GetSelectedPlugin(hWnd));
						}
					}
					else{
						int index;
						PPluginOptions plugin = GetSelectedPlugin(hWnd, &index);
						if (plugin)
						{
							SavePluginGroup(plugin, plugin->szGroup);
							SavePluginTitle(plugin, plugin->szTitle);
							
							UpdatePlugin(GetDlgItem(hWnd, IDC_LIST_PLUGINS), index, plugin, 1);
							LoadPluginInfo(hWnd, GetSelectedPlugin(hWnd));
						}
						
						EnableWindow(GetDlgItem(hWnd, IDC_SAVE), 0);
						EnableWindow(GetDlgItem(hWnd, IDC_CANCEL), 0);
					}
					
					break;
				}
			}
			
			break;
		}
		
		case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							//<TODO>
						
							break;
						}
					}
					
					break;
				}
				
				case IDC_LIST_PLUGINS:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case LVN_COLUMNCLICK:
						{
							LPNMLISTVIEW lv = (LPNMLISTVIEW) lParam;
							int column = lv->iSubItem;
							SortParams params = {0};
							params.hList = GetDlgItem(hWnd, IDC_LIST_PLUGINS);
							params.column = column;
							
							ListView_SortItemsEx(params.hList, PluginsCompare, (LPARAM) &params);
							lastColumn = (params.column == lastColumn) ? -1 : params.column;

							break;
						}
						
						case LVN_ITEMCHANGED:
						{
							NMLISTVIEW *data = (NMLISTVIEW *) lParam;
							if (data->uNewState & LVIS_SELECTED) //new item
							{
								LoadPluginInfo(hWnd, GetSelectedPlugin(hWnd));
								
								EnableWindow(GetDlgItem(hWnd, IDC_SAVE), 0);
								EnableWindow(GetDlgItem(hWnd, IDC_CANCEL), 1);
								EnableWindow(GetDlgItem(hWnd, IDC_GROUP), 1);
								EnableWindow(GetDlgItem(hWnd, IDC_TITLE), 1);
							}
							else{ //selected item
								if (data->uOldState & LVIS_SELECTED)
								{
									const int MAX_SIZE = 256;
									PPluginOptions plugin = (PPluginOptions) data->lParam;
									if (plugin)
									{
										WCHAR group[MAX_SIZE];
										WCHAR title[MAX_SIZE];
										WCHAR newGroup[MAX_SIZE];
										WCHAR newTitle[MAX_SIZE];
										
										GetPluginGroup(plugin, group, MAX_SIZE);
										GetPluginTitle(plugin, title, MAX_SIZE);
										GetWindowTextW(GetDlgItem(hWnd, IDC_GROUP), newGroup, MAX_SIZE);
										GetWindowTextW(GetDlgItem(hWnd, IDC_TITLE), newTitle, MAX_SIZE);
										
										if ((wcscmp(group, newGroup)) || (wcscmp(title, newTitle)))
										{
											if (MessageBox(hWnd, Translate("Do you want to save the changes now ?"), Translate("Changes not saved"), MB_YESNO | MB_ICONINFORMATION) == IDYES)
											{
												SavePluginGroup(plugin, newGroup);
												SavePluginTitle(plugin, newTitle);
												
												UpdatePlugin(GetDlgItem(hWnd, IDC_LIST_PLUGINS), data->iItem, plugin, 1);
											}
										}
									}
								}
							}

							break;
						}
					}
				
					break;
				}
			}
			
			break;
		}
	}
	
	return 0;
}