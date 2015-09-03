#include "stdafx.h"

wchar_t* SaveBitmap(HBITMAP bmp, const char *szId)
{
	wchar_t wszSavePath[MAX_PATH];
	mir_snwprintf(wszSavePath, L"%s\\MirandaToaster.%s.png", wszTempDir, _A2T(szId));

	if (!(GetFileAttributes(wszSavePath) < 0xFFFFFFF))
	{
		IMGSRVC_INFO isi = { sizeof(isi) };
		isi.wszName = mir_wstrdup(wszSavePath);
		isi.hbm = bmp;
		isi.dwMask = IMGI_HBITMAP;
		isi.fif = FREE_IMAGE_FORMAT::FIF_PNG;
		CallService(MS_IMG_SAVE, (WPARAM)&isi, IMGL_WCHAR);
	}
	return mir_wstrdup(wszSavePath);
}

wchar_t* SaveHIcon(HICON hIcon, const char *szId)
{
	wchar_t *wszResult = NULL;
	ICONINFO icon;
	if (GetIconInfo(hIcon, &icon))
	{
		wszResult = SaveBitmap(icon.hbmColor, szId);

		DeleteObject(icon.hbmMask);
		DeleteObject(icon.hbmColor);
	}
	return wszResult;
}