#ifndef _STATUS_ICON_INC
#define _STATUS_ICON_INC

extern HANDLE hHookIconPressedEvt;
extern int status_icon_list_size;

int InitStatusIcons();
int DeinitStatusIcons();

int  GetStatusIconsCount(HCONTACT hContact);
void DrawStatusIcons(HCONTACT hContact, HDC hdc, RECT r, int gap);
void CheckIconClick(HCONTACT hContact, HWND hwndFrom, POINT pt, RECT r, int gap, int flags);

#endif
