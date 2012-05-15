#include "dlg_handlers.h"

PFolderItem lastItem = NULL;

static int bInitializing = 0;

int GetCurrentItemSelection(HWND hWnd)
{
	return SendDlgItemMessageW(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_GETCURSEL, 0, 0);
}

int GetCurrentSectionSelection(HWND hWnd)
{
	return SendDlgItemMessageW(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_GETCURSEL, 0, 0);
}

PFolderItem GetSelectedItem(HWND hWnd)
{
	WCHAR section[MAX_FOLDER_SIZE];
	WCHAR item[MAX_FOLDER_SIZE];
	GetCurrentItemText(hWnd, item, MAX_FOLDER_SIZE);
	GetCurrentSectionText(hWnd, section, MAX_FOLDER_SIZE);
	return lstRegisteredFolders.GetTranslated(section, item);
}

int GetCurrentItemText(HWND hWnd, WCHAR *buffer, int count)
{
	int index = GetCurrentItemSelection(hWnd);
	if (index != LB_ERR)
		{
			SendDlgItemMessageW(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_GETTEXT, index, (LPARAM) buffer);
			return 1;
		}
		else{
			buffer[0] = L'\0';
			return 0;
		}
}

int GetCurrentSectionText(HWND hWnd, WCHAR *buffer, int count)
{
	int index = GetCurrentSectionSelection(hWnd);
	if (index != LB_ERR)
		{
			SendDlgItemMessageW(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_GETTEXT, index, (LPARAM) buffer);
			return 1;
		}
		else{
			buffer[0] = L'0';
			return 0;
		}
}

//void GetEditText(HWND hWnd, char *buffer, int size)
//{
//	GetWindowText(GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer, size);
//	StrTrim(buffer, "\\\t \n");
//}

void GetEditTextW(HWND hWnd, wchar_t *buffer, int size)
{
	GetWindowTextW(GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer, size);
}

//void SetEditText(HWND hWnd, const char *buffer)
//{
//	bInitializing = 1;
//	SetWindowText(GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer);
//	bInitializing = 0;
////	SendDlgItemMessage(hWnd, IDC_FOLDER_EDIT, WM_SETTEXT, 0, (LPARAM) buffer);
//}

void SetEditTextW(HWND hWnd, const wchar_t *buffer)
{
	bInitializing = 1;
	SetWindowTextW(GetDlgItem(hWnd, IDC_FOLDER_EDIT), buffer);
	bInitializing = 0;
}

int ContainsSection(HWND hWnd, const WCHAR *section)
{
	int index = SendDlgItemMessageW(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_FINDSTRINGEXACT, -1, (LPARAM) section);
	return (index != LB_ERR);
}

void LoadRegisteredFolderSections(HWND hWnd)
{
	for (int i = 0; i < lstRegisteredFolders.Count(); i++)
		{
			PFolderItem tmp = lstRegisteredFolders.Get(i + 1);
			WCHAR *translated = mir_a2u(Translate(tmp->GetSection()));
			if (!ContainsSection(hWnd, translated))
				{
					SendDlgItemMessageW(hWnd, IDC_FOLDERS_SECTIONS_LIST, LB_ADDSTRING, 0, (LPARAM) translated);
				}
			mir_free(translated);
		}
}

void LoadRegisteredFolderItems(HWND hWnd)
{
	WCHAR buffer[MAX_FOLDER_SIZE];
	GetCurrentSectionText(hWnd, buffer, MAX_FOLDER_SIZE);
	SendDlgItemMessageW(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_RESETCONTENT, 0, 0);
	for (int i = 0; i < lstRegisteredFolders.Count(); i++)
		{
			PFolderItem item = lstRegisteredFolders.Get(i + 1);
			WCHAR *wide = mir_a2u(Translate(item->GetSection()));
			if (wcscmp(buffer, wide) == 0)
				{
					mir_free(wide);
					wide = mir_a2u(Translate(item->GetName()));
					SendDlgItemMessageW(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_ADDSTRING, 0, (LPARAM) wide);
				}
			mir_free(wide);
		}
	SendDlgItemMessageW(hWnd, IDC_FOLDERS_ITEMS_LIST, LB_SETCURSEL, 0, 0); //select the first item
	PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_FOLDERS_ITEMS_LIST, LBN_SELCHANGE), 0); //tell the dialog to refresh the preview
}

void LoadItem(HWND hWnd, PFolderItem item)
{
	if (item)
		{
			if (item->IsUnicode())
				{
					SetEditTextW(hWnd, item->GetFormatW());
				}
				else{
					WCHAR *buffer = mir_a2u(item->GetFormat());
					SetEditTextW(hWnd, buffer);
					mir_free(buffer);
				}
			RefreshPreview(hWnd);
		}
}

void SaveItem(HWND hWnd, PFolderItem item, int bEnableApply)
{
	if (item)
		{
			if (item->IsUnicode())
				{
					wchar_t buffer[MAX_FOLDER_SIZE];
					GetEditTextW(hWnd, buffer, MAX_FOLDER_SIZE);
					item->SetFormatW(buffer);
				}
				else{
					WCHAR buffer[MAX_FOLDER_SIZE];
					char ansi[MAX_FOLDER_SIZE];
					GetEditTextW(hWnd, buffer, MAX_FOLDER_SIZE);
					WideCharToMultiByte(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, buffer, -1, ansi, MAX_FOLDER_SIZE, NULL, NULL);
					item->SetFormat(ansi);
				}
			if (bEnableApply)
			{
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			}
		}
}

int ChangesNotSaved(HWND hWnd, PFolderItem item)
{
	int res = 0;
	if (item)
		{
			if (item->IsUnicode())
				{
					wchar_t buffer[MAX_FOLDER_SIZE];
					GetEditTextW(hWnd, buffer, MAX_FOLDER_SIZE);
					res = (wcscmp(item->GetFormatW(), buffer) != 0);
				}
				else{
					WCHAR buffer[MAX_FOLDER_SIZE];
					GetEditTextW(hWnd, buffer, MAX_FOLDER_SIZE);
					char *ansi = mir_u2a(buffer);

					res = (strcmp(item->GetFormat(), ansi) != 0);
					mir_free(ansi);
				}
		}

	return res;
}

void CheckForChanges(HWND hWnd, int bNeedConfirmation = 1)
{
	if (ChangesNotSaved(hWnd, lastItem))
		{
			if ((!bNeedConfirmation) || MessageBoxW(hWnd, TranslateW(L"Some changes weren't saved. Apply the changes now ?"), TranslateW(L"Changes not saved"), MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					SaveItem(hWnd, lastItem);
				}
		}
}

void RefreshPreview(HWND hWnd)
{
	wchar_t tmp[MAX_FOLDER_SIZE];
	wchar_t res[MAX_FOLDER_SIZE];
	GetEditTextW(hWnd, tmp, MAX_FOLDER_SIZE);
	ExpandPathW(res, tmp, MAX_FOLDER_SIZE);
	SetWindowTextW(GetDlgItem(hWnd, IDC_PREVIEW_EDIT), res);
	//SendDlgItemMessage(hWnd, IDC_PREVIEW_EDIT, WM_SETTEXT, 0, (LPARAM) res);
}


void LoadHelp(HWND hWnd)
{
	SETTEXTEX tmp = {0};
	tmp.flags = ST_SELECTION;
	tmp.codepage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	char *text =
#include "variablesHelp.inc"
;

	char buffer[2048];
	char line[2048];
	int len;

	char *p;
	while ((p = strchr(text, '\n')))
	{
		len = p - text + 1;
		memcpy(line, text, len);
		line[len] = 0;

		mir_snprintf(buffer, sizeof(buffer), "{\\rtf1\\ansi\\deff0\\pard\\li%u\\fi-%u\\ri%u\\tx%u\\fs19 %s\\par}", 60*15, 60*15, 5*15, 60*15, Translate(line));
		text = p + 1;

		SendDlgItemMessageW(hWnd, IDC_HELP_RICHEDIT, EM_SETTEXTEX, (WPARAM) &tmp, (LPARAM) buffer);
	}


}

/************************************** DIALOG HANDLERS *************************************/
#include "commctrl.h"

INT_PTR CALLBACK DlgProcOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					bInitializing = 1;
					lastItem = NULL;
					TranslateDialogDefault(hWnd);
					LoadRegisteredFolderSections(hWnd);
					bInitializing = 0;

					break;
				}

			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_FOLDER_EDIT:
								{
									switch (HIWORD(wParam))
										{
											case EN_CHANGE:
												{
													RefreshPreview(hWnd);
													if (!bInitializing)
													{
														SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0); //show the apply button.
													}

													break;
												}
										}

									break;
								}

							case IDC_REFRESH_BUTTON:
								{
									RefreshPreview(hWnd);

									break;
								}

							case IDC_HELP_BUTTON:
								{
									HWND helpDlg = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_VARIABLES_HELP), hWnd, DlgProcVariables);
									ShowWindow(helpDlg, SW_SHOW);

									break;
								}

							case IDC_FOLDERS_SECTIONS_LIST:
								{
									switch (HIWORD(wParam))
										{
											case LBN_SELCHANGE:
												{
													CheckForChanges(hWnd);
													LoadRegisteredFolderItems(hWnd);
													lastItem = NULL;
													SetEditTextW(hWnd, L"");
													RefreshPreview(hWnd);

													break;
												}
										}

									break;
								}

							case IDC_FOLDERS_ITEMS_LIST:
								{
									switch (HIWORD(wParam))
										{
											case LBN_SELCHANGE:
												{
													PFolderItem item = GetSelectedItem(hWnd);
													if (item != NULL)
														{
															CheckForChanges(hWnd);
															LoadItem(hWnd, item);
														}
													lastItem = item;

													break;
												}
										}

									break;
								}
						}

					break;
				}

			case WM_NOTIFY:
				{
//					Log("WM_NOTIFY %d %d", wParam, lParam);
					switch(((LPNMHDR)lParam)->idFrom)
					{
						case 0:
							switch (((LPNMHDR)lParam)->code)
							{
								case PSN_APPLY:
								{
									PFolderItem item = GetSelectedItem(hWnd);
									if (item)
									{
										SaveItem(hWnd, item, FALSE);
										LoadItem(hWnd, item);
									}

									lstRegisteredFolders.Save();
									CallPathChangedEvents();

									break;
								}
							}

						break;
					}
				}

				break;

			default:
				{

					break;
				}
		}

	return 0;
}

INT_PTR CALLBACK DlgProcVariables(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					TranslateDialogDefault(hWnd);
					LoadHelp(hWnd);

					break;
				}

			case WM_CLOSE:
				{
					DestroyWindow(hWnd);

					break;
				}

			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDCLOSE:
								{
									DestroyWindow(hWnd);

									break;
								}
						}

					break;
				}
		}

	return 0;
}