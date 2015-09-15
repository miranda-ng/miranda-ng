extern wchar_t wszTempDir[];

class ToasterImage
{
	HBITMAP _hBitmap;
	ptrT tszId;

public:
	ToasterImage(HICON hIcon)
	{
		ICONINFO icon;
		if (GetIconInfo(hIcon, &icon))
		{
			_hBitmap = icon.hbmColor;
			DeleteObject(icon.hbmMask);
		}
		tszId = CMString(FORMAT, _T("%p"), hIcon).Detach();
	}

	inline ToasterImage(HBITMAP bmp) : _hBitmap(bmp)
	{ 
		tszId = CMString(FORMAT, _T("%p"), bmp).Detach(); 
	}

	ToasterImage(const char *szProto)
	{
		ICONINFO icon;
		if (GetIconInfo(Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE, 1), &icon))
		{
			_hBitmap = icon.hbmColor;
			DeleteObject(icon.hbmMask);
		}
		tszId = mir_a2t(szProto);
	}

	inline ~ToasterImage()
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

	inline operator wchar_t*()
	{	return Save();
	}
};