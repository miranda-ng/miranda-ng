#ifndef M_FOLDERS_DLG_HANDLERS_H
#define M_FOLDERS_DLG_HANDLERS_H

//#include "commonheaders.h"
#include "services.h"
#include "events.h"
#include <richedit.h>

INT_PTR CALLBACK DlgProcOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcVariables(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int GetCurrentItemSelection(HWND hWnd);
int GetCurrentSectionSelection(HWND hWnd);
PFolderItem GetSelectedItem(HWND hWnd);
int GetCurrentSectionText(HWND hWnd, WCHAR *buffer, int count);
int GetCurrentItemText(HWND hWnd, WCHAR *buffer, int count);
//void GetEditText(HWND hWnd, char *buffer, int size);
void GetEditTextW(HWND hWnd, wchar_t *buffer, int size);
//void SetEditText(HWND hWnd, const char *buffer);
void SetEditTextW(HWND hWnd, const wchar_t *buffer);
void LoadRegisteredFolderSections(HWND hWnd);
void LoadRegisteredFolderItems(HWND hWnd);
void LoadItem(HWND hWnd, PFolderItem item);
void SaveItem(HWND hWnd, PFolderItem item, int bEnableApply = TRUE);
int ChangesNotSaved(HWND hWnd);
void RefreshPreview(HWND hWnd);

#endif //M_FOLDERS_DLG_HANDLERS_H