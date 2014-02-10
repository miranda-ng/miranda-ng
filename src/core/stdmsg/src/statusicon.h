#ifndef _STATUS_ICON_INC
#define _STATUS_ICON_INC

extern HANDLE hHookIconPressedEvt;
extern int status_icon_list_size;

int InitStatusIcons();
int DeinitStatusIcons();

int  GetStatusIconsCount(MCONTACT hContact);
void DrawStatusIcons(MCONTACT hContact, HDC hdc, RECT r, int gap);
void CheckIconClick(MCONTACT hContact, HWND hwndFrom, POINT pt, RECT r, int gap, int flags);

#endif
