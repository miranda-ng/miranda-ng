#pragma once

DEFINE_PROPERTYKEY(PKEY_AppUserModel_ID, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 5);
HRESULT TryCreateShortcut();

extern wchar_t wszTempDir[];

class ToasterImage
{
	HBITMAP _hBitmap;
	ptrW tszId;

public:
	__inline explicit ToasterImage(HICON hIcon) : _hBitmap(NULL), tszId(CMStringW(FORMAT, L"%p", hIcon).Detach())
	{
		ICONINFO icon = { 0 };
		if (GetIconInfo(hIcon, &icon)) {
			_hBitmap = icon.hbmColor;
			DeleteObject(icon.hbmMask);
		}
	}

	__inline explicit ToasterImage(HBITMAP bmp) : _hBitmap(bmp), tszId(CMStringW(FORMAT, L"%p", bmp).Detach())
	{}

	__inline explicit ToasterImage(const char *szProto) : _hBitmap(NULL), tszId(mir_a2u(szProto))
	{
		ICONINFO icon = { 0 };
		if (GetIconInfo(Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE, 1), &icon)) {
			_hBitmap = icon.hbmColor;
			DeleteObject(icon.hbmMask);
		}
	}

	__inline ~ToasterImage()
	{
		DeleteObject(_hBitmap);
	}

	wchar_t* Save()
	{
		wchar_t wszSavePath[MAX_PATH];
		mir_snwprintf(wszSavePath, L"%s\\MirandaToaster.%s.png", wszTempDir, tszId.get());

		if (!(GetFileAttributes(wszSavePath) < 0xFFFFFFF)) {
			IMGSRVC_INFO isi = { sizeof(isi) };
			isi.pwszName = wszSavePath;
			isi.hbm = _hBitmap;
			isi.dwMask = IMGI_HBITMAP;
			isi.fif = FREE_IMAGE_FORMAT::FIF_PNG;
			Image_Save(&isi);
		}
		return mir_wstrdup(wszSavePath);
	}

	__inline operator wchar_t*()
	{
		return Save();
	}
};
