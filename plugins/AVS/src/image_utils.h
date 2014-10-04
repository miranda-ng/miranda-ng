#ifndef __IMAGE_UTILS_H__
# define __IMAGE_UTILS_H__

HBITMAP BmpFilterLoadBitmap(BOOL *bIsTransparent, const TCHAR *ptszFilename);
int     BmpFilterSaveBitmap(HBITMAP hBmp, const TCHAR *ptszFile, int flags);

HBITMAP CopyBitmapTo32(HBITMAP hBitmap);

BOOL PreMultiply(HBITMAP hBitmap);
BOOL MakeTransparentBkg(MCONTACT hContact, HBITMAP *hBitmap);
HBITMAP MakeGrayscale(MCONTACT hContact, HBITMAP hBitmap);
DWORD GetImgHash(HBITMAP hBitmap);

int SaveAvatar( const char* protocol, const TCHAR* tszFileName );

#endif // __IMAGE_UTILS_H__
