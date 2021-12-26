#pragma once
class MirandaDialog : private MNonCopyable
{
public:
	MirandaDialog(void);
	virtual ~MirandaDialog(void);
	static wchar_t *Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem);
	static int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId);
protected:
	void changed(HWND window);
};

