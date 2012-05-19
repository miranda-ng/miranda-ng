#ifndef _ICONS_INC
#define _ICONS_INC

void InitIcons(void);

typedef enum {I_CHKUPD, I_RSTRT, I_CHKUPDEXT} IconIndex;

HICON LoadIconEx(IconIndex i, bool big = false);
HANDLE GetIconHandle(IconIndex i);
void ReleaseIconEx(HICON hIcon);


#endif
