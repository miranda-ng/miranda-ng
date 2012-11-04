#include "../commonheaders.h"

extern int DefaultImageListColorDepth;

static HANDLE hExtraImageListRebuilding,hExtraImageApplying;
HANDLE hExtraImageClick;

static HIMAGELIST hExtraImageList;
extern HINSTANCE g_hInst;
extern HIMAGELIST hCListImages;

extern int ClcIconsChanged(WPARAM,LPARAM);

void SetAllExtraIcons(HWND hwndList,HANDLE hContact)
{
	/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

boolean ImageCreated = FALSE;
void ReloadExtraIcons();

int ColumnNumToExtra(int column)
{
	return column+1;
}
