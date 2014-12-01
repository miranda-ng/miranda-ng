#include "Common.h"
#include "MirandaDialog.h"


MirandaDialog::MirandaDialog(void)
{
}


MirandaDialog::~MirandaDialog(void)
{
}

void MirandaDialog::changed(HWND window)
{
	SendMessage(GetParent(window), PSM_CHANGED, 0, 0);
}

WCHAR *MirandaDialog::Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem) {

	int len = SendDlgItemMessage(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	WCHAR *res = (WCHAR*)mir_alloc((len+1)*sizeof(WCHAR));
	memset(res, 0, ((len + 1) * sizeof(WCHAR)));
	GetDlgItemText(hwndDlg, nIDDlgItem, res, len+1);

	return res;
}

int MirandaDialog::ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId)
{
	HICON hIcon = LoadSkinnedIcon(iconId);
	int res = ImageList_AddIcon(hIml, hIcon);
	Skin_ReleaseIcon(hIcon);
	return res;
}