wchar_t* SaveBitmap(HBITMAP bmp, const char *szId);
wchar_t* SaveHIcon(HICON hIcon, const char *szId);
__forceinline wchar_t* ProtoIcon(const char *szProto)
{	return SaveHIcon(Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE, 1), szProto);
}
