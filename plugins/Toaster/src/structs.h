#pragma once

struct ToastData : public MZeroedObject
{
	MCONTACT hContact;
	wchar_t *tszTitle;
	wchar_t *tszText;
	union
	{
		HICON hIcon;
		HBITMAP hBitmap;
	};
	int iType; // 0 = none, 1 = hBitmap, 2 = hIcon

	WNDPROC pPopupProc;
	void *vPopupData;

	ToastData(MCONTACT _hContact, const wchar_t *_tszTitle, const wchar_t *_tszText, HICON _hIcon = NULL) : 
		hContact(_hContact),
		tszTitle(mir_wstrdup(_tszTitle)), 
		tszText(mir_wstrdup(_tszText)), 
		hIcon(_hIcon), 
		iType(_hIcon ? 2 : 0) ,
		pPopupProc(NULL),
		vPopupData(NULL)
	{}
	ToastData(MCONTACT _hContact, const wchar_t *_tszTitle, const wchar_t *_tszText, HBITMAP bmp = NULL) :
		hContact(_hContact),
		tszTitle(mir_wstrdup(_tszTitle)),
		tszText(mir_wstrdup(_tszText)),
		hBitmap(bmp),
		iType(bmp ? 1 : 0),
		pPopupProc(NULL),
		vPopupData(NULL)
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

	ClassData(int f, HICON h = NULL) : iFlags(f), hIcon(h), pPopupProc(NULL)
	{
	}
};