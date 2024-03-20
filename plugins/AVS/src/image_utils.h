#ifndef __IMAGE_UTILS_H__
# define __IMAGE_UTILS_H__

HBITMAP BmpFilterLoadBitmap(BOOL *bIsTransparent, const wchar_t *ptszFilename);
int     BmpFilterSaveBitmap(HBITMAP hBmp, const wchar_t *ptszFile, int flags);

HBITMAP CopyBitmapTo32(HBITMAP hBitmap);

BOOL MakeTransparentBkg(MCONTACT hContact, HBITMAP *hBitmap);
HBITMAP MakeGrayscale(HBITMAP hBitmap);
uint32_t GetImgHash(HBITMAP hBitmap);

#endif // __IMAGE_UTILS_H__
