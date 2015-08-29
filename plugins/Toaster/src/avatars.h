class ToasterAvatar
{
	ptrW wszPath;
	int iFormat;
	MCONTACT hContact;
	FIBITMAP *bitmap;

public:
	ToasterAvatar(PROTO_AVATAR_INFORMATION *pai) : wszPath(pai->filename), iFormat(pai->format), hContact(pai->hContact)
	{
		bitmap = (FIBITMAP*)CallService(MS_IMG_LOAD, (WPARAM)wszPath, IMGL_WCHAR | IMGL_RETURNDIB);
	}
	
	~ToasterAvatar()
	{
		CallService(MS_IMG_UNLOAD, (WPARAM)bitmap);
	}

	INT_PTR Save(const wchar_t *wszSavePath)
	{
		IMGSRVC_INFO isi = { sizeof(isi) };
		isi.wszName = mir_wstrdup(wszSavePath);
		isi.dib = bitmap;
		isi.dwMask = IMGI_FBITMAP;
		isi.fif = FREE_IMAGE_FORMAT::FIF_PNG;
		return CallService(MS_IMG_SAVE, (WPARAM)&isi, IMGL_WCHAR);
	}
};