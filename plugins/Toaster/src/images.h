extern wchar_t wszTempDir[];

class ToasterImage
{
	HBITMAP _hBitmap;
	ptrT tszId;

public:
	__inline explicit ToasterImage(HICON hIcon) : _hBitmap(NULL), tszId(CMString(FORMAT, _T("%p"), hIcon).Detach())
	{
		ICONINFO icon = { 0 };
		if (GetIconInfo(hIcon, &icon))
		{
			_hBitmap = icon.hbmColor;
			DeleteObject(icon.hbmMask);
		}
	}

	__inline explicit ToasterImage(HBITMAP bmp) : _hBitmap(bmp), tszId(CMString(FORMAT, _T("%p"), bmp).Detach())
	{  
	}

	__inline explicit ToasterImage(const char *szProto) : _hBitmap(NULL), tszId(mir_a2t(szProto))
	{
		ICONINFO icon = { 0 };
		if (GetIconInfo(Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE, 1), &icon))
		{
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
		mir_snwprintf(wszSavePath, L"%s\\MirandaToaster.%s.png", wszTempDir, tszId);

		if (!(GetFileAttributes(wszSavePath) < 0xFFFFFFF))
		{
			IMGSRVC_INFO isi = { sizeof(isi) };
			isi.wszName = mir_wstrdup(wszSavePath);
			isi.hbm = _hBitmap;
			isi.dwMask = IMGI_HBITMAP;
			isi.fif = FREE_IMAGE_FORMAT::FIF_PNG;
			__try
			{
				CallService(MS_IMG_SAVE, (WPARAM)&isi, IMGL_WCHAR);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return nullptr;
			}
		}
		return mir_wstrdup(wszSavePath);
	}

	__inline operator wchar_t*()
	{	return Save();
	}
};