#include "stdafx.h"
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

wchar_t *MirandaDialog::Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem) {

	int len = SendDlgItemMessage(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((len+1)*sizeof(wchar_t));
	memset(res, 0, ((len + 1) * sizeof(wchar_t)));
	GetDlgItemText(hwndDlg, nIDDlgItem, res, len+1);

	return res;
}

int MirandaDialog::ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId)
{
	HICON hIcon = Skin_LoadIcon(iconId);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}