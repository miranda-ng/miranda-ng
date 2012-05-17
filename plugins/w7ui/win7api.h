#ifndef win7api_h__
#define win7api_h__

#define WM_DWMSENDICONICTHUMBNAIL           0x0323
#define WM_DWMSENDICONICLIVEPREVIEWBITMAP   0x0326

#define DWMWA_HAS_ICONIC_BITMAP 10

#define DWM_SIT_DISPLAYFRAME 0x00000001

enum TBATFLAG
{	TBATF_USEMDITHUMBNAIL	= 0x1,
	TBATF_USEMDILIVEPREVIEW	= 0x2
};

#define THBN_CLICKED 0x1800

extern HRESULT (WINAPI *dwmInvalidateIconicBitmaps)(HWND);
extern HRESULT (WINAPI *dwmSetIconicThumbnail)(HWND, HBITMAP, DWORD);
extern HRESULT (WINAPI *dwmSetIconicLivePreviewBitmap)(HWND, HBITMAP, LPPOINT, DWORD);

extern HANDLE (STDAPICALLTYPE *openThemeData)(HWND, LPCWSTR);
extern HRESULT (STDAPICALLTYPE *drawThemeTextEx)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const struct _DTTOPTS *);
extern HRESULT (STDAPICALLTYPE *closeThemeData)(HANDLE);

extern "C" const IID IID_ICustomDestinationList;
extern "C" const IID IID_IObjectArray;
extern "C" const IID IID_IObjectCollection;
extern "C" const IID IID_ITaskbarList3;
extern "C" const CLSID CLSID_CustomDestinationList;
extern "C" const CLSID CLSID_EnumerableObjectCollection;

#include "win7api_IObjectArray.h"
#include "win7api_IObjectCollection.h"
#include "win7api_ICustomDestinationList.h"
//#include "win7api_ITaskbarList3.h"

#endif // win7api_h__
