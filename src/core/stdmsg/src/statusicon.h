#ifndef _STATUS_ICON_INC
#define _STATUS_ICON_INC

extern HANDLE hHookIconPressedEvt;
extern int status_icon_list_size;

int InitStatusIcons();
int DeinitStatusIcons();

int  GetStatusIconsCount(MCONTACT hContact);
void DrawStatusIcons(MCONTACT hContact, HDC hdc, const RECT &rc, int gap);
void CheckStatusIconClick(MCONTACT hContact, HWND hwndFrom, POINT pt, const RECT &rc, int gap, int flags);

#endif
