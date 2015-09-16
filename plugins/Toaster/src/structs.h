#pragma once

struct ToastData : public MZeroedObject
{
	MCONTACT hContact;
	TCHAR *tszTitle;
	TCHAR *tszText;
	union
	{
		HICON hIcon;
		HBITMAP hBitmap;
	};
	int iType; // 0 = none, 1 = hBitmap, 2 = hIcon

	WNDPROC pPopupProc;
	void *vPopupData;

	ToastData(MCONTACT _hContact, const TCHAR *_tszTitle, const TCHAR *_tszText, HICON _hIcon = NULL) : 
		hContact(_hContact),
		tszTitle(mir_tstrdup(_tszTitle)), 
		tszText(mir_tstrdup(_tszText)), 
		hIcon(_hIcon), 
		iType(_hIcon ? 2 : 0) 
	{}
	ToastData(MCONTACT _hContact, const TCHAR *_tszTitle, const TCHAR *_tszText, HBITMAP bmp = NULL) :
		hContact(_hContact),
		tszTitle(mir_tstrdup(_tszTitle)),
		tszText(mir_tstrdup(_tszText)),
		hBitmap(bmp),
		iType(bmp ? 1 : 0)
	{}
	~ToastData()
	{
		mir_free(tszTitle);
		mir_free(tszText);
	}
};

struct ClassData : public MZeroedObject
{
	int iFlags;
	HICON hIcon;

	WNDPROC pPopupProc;

	ClassData(int f, HICON h = NULL) : iFlags(f), hIcon(h) 
	{
	}
};

struct ToastHandlerData : public MZeroedObject
{
	MCONTACT hContact;

	WNDPROC pPopupProc;
	void *vPopupData;

	ToastNotification *tstNotification;
};