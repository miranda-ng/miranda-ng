#ifndef __IMAGE_UTILS_H__
# define __IMAGE_UTILS_H__

#define _WIN32_WINNT 0x0501
#include <windows.h>

#include <m_avatars.h>


// Load an image
// wParam = NULL
// lParam = filename
INT_PTR BmpFilterLoadBitmap32(WPARAM wParam,LPARAM lParam);

// Save an HBITMAP to an image
// wParam = HBITMAP
// lParam = full path of filename
INT_PTR BmpFilterSaveBitmap(WPARAM wParam,LPARAM lParam);
#if defined(_UNICODE)
	INT_PTR BmpFilterSaveBitmapW(WPARAM wParam,LPARAM lParam);
	#define BmpFilterSaveBitmapT BmpFilterSaveBitmapW
#else
	#define BmpFilterSaveBitmapT BmpFilterSaveBitmap
#endif

// Returns != 0 if can save that type of image, = 0 if cant
// wParam = 0
// lParam = PA_FORMAT_*   // image format
INT_PTR BmpFilterCanSaveBitmap(WPARAM wParam,LPARAM lParam);

// Returns a copy of the bitmap with the size especified or the original bitmap if nothing has to be changed
// wParam = ResizeBitmap *
// lParam = NULL
INT_PTR BmpFilterResizeBitmap(WPARAM wParam,LPARAM lParam);


int BmpFilterSaveBitmap(HBITMAP hBmp, char *szFile, int flags);
#if defined(_UNICODE)
	int BmpFilterSaveBitmapW(HBITMAP hBmp, wchar_t *wszFile, int flags);
	#define BmpFilterSaveBitmapT BmpFilterSaveBitmapW
#else
	#define BmpFilterSaveBitmapT BmpFilterSaveBitmap
#endif

HBITMAP CopyBitmapTo32(HBITMAP hBitmap);

BOOL PreMultiply(HBITMAP hBitmap);
BOOL MakeTransparentBkg(HANDLE hContact, HBITMAP *hBitmap);
HBITMAP MakeGrayscale(HANDLE hContact, HBITMAP hBitmap);
DWORD GetImgHash(HBITMAP hBitmap);

int AVS_pathIsAbsolute(const TCHAR *path);
size_t AVS_pathToRelative(const TCHAR *sPrc, TCHAR *pOut);
size_t AVS_pathToAbsolute(const TCHAR *pSrc, TCHAR *pOut);

int SaveAvatar( const char* protocol, const TCHAR* tszFileName );

#endif // __IMAGE_UTILS_H__
