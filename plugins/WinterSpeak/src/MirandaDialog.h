#pragma once
class MirandaDialog
{
public:
	MirandaDialog(void);
	virtual ~MirandaDialog(void);
	static WCHAR *MirandaDialog::Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem);
	static int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId);
protected:
	void changed(HWND window);
};

