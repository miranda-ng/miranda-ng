#ifndef _BUTTONSBAR_H
#define _BUTTONSBAR_H

#define MIN_CBUTTONID 4000
#define MAX_CBUTTONID 5000

struct CustomButtonData;
struct TWindowData;

void CB_DeInitCustomButtons();
void CB_InitCustomButtons();

void BB_InitDlgButtons(TWindowData *dat);
void BB_RefreshTheme(const TWindowData *dat);
BOOL BB_SetButtonsPos(TWindowData *dat);
void BB_RedrawButtons(TWindowData *dat);
void BB_CustomButtonClick(TWindowData *dat, DWORD idFrom, HWND hwndFrom, BOOL code);

void CB_DestroyAllButtons(HWND hwndDlg);
void CB_DestroyButton(HWND hwndDlg, TWindowData *dat, DWORD dwButtonCID, DWORD dwFlags);
void CB_ChangeButton(HWND hwndDlg, TWindowData *dat, CustomButtonData *cbd);

#endif
