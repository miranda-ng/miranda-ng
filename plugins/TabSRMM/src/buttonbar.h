#ifndef _BUTTONSBAR_H
#define _BUTTONSBAR_H

struct TWindowData;

void CB_InitCustomButtons();

void BB_InitDlgButtons(TWindowData *dat);
void BB_RefreshTheme(const TWindowData *dat);
BOOL BB_SetButtonsPos(TWindowData *dat);
void BB_RedrawButtons(TWindowData *dat);

void CB_DestroyAllButtons(HWND hwndDlg);
void CB_DestroyButton(HWND hwndDlg, TWindowData *dat, DWORD dwButtonCID, DWORD dwFlags);
void CB_ChangeButton(HWND hwndDlg, TWindowData *dat, CustomButtonData *cbd);

#endif
