#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"
#include "m_api/m_xpTheme.h"


//statical

typedef struct _tagXPTObject
{
  HANDLE  hThemeHandle;
  HWND    hOwnerWindow;
  LPCWSTR lpcwClassObject;
} XPTObject;

static SortedList * xptObjectList=NULL;
static CRITICAL_SECTION xptCS;
static BOOL xptModuleLoaded=FALSE;


static HMODULE _xpt_ThemeAPIHandle = NULL; // handle to uxtheme.dll
static HANDLE   (WINAPI *_xpt_OpenThemeData)(HWND, LPCWSTR) = NULL;
static HRESULT  (WINAPI *_xpt_CloseThemeData)(HANDLE)= NULL;
static BOOL     (WINAPI *_xpt_IsThemeBackgroundPartiallyTransparent)(HANDLE, int,int)= NULL;
static BOOL		(WINAPI *_xpt_EnableThemeDialogTexture)(HANDLE, DWORD)=NULL;
static HRESULT  (WINAPI *_xpt_GetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT *prc, int eSize, SIZE *psz )= NULL;
static HRESULT  (WINAPI *_xpt_DrawThemeParentBackground)(HWND, HDC, const RECT *)= NULL;
static HRESULT  (WINAPI *_xpt_DrawThemeBackground)(HANDLE, HDC, int, int,const RECT *, const RECT *)= NULL;								 
static HRESULT  (WINAPI *_xpt_DrawThemeText)(HANDLE, HDC, int, int, LPCWSTR, int,DWORD, DWORD, const RECT *)= NULL;


#undef  MGPROC
#define MGPROC(x) GetProcAddress(_xpt_ThemeAPIHandle,x)

#define xptcheck  if (!xptModuleLoaded) return
#define xptlock() EnterCriticalSection(&xptCS)
#define xptunlock() LeaveCriticalSection(&xptCS)

static void _sttXptObjectDestructor (void * pt)
{
	mir_free(pt);
}
static int _xpt_ThemeSupport()
{
	if (IsWinVerXPPlus()) {
		if (!_xpt_ThemeAPIHandle) {
			_xpt_ThemeAPIHandle = GetModuleHandleA("uxtheme");
			if (_xpt_ThemeAPIHandle) {
				_xpt_OpenThemeData = (HANDLE(WINAPI *)(HWND, LPCWSTR))MGPROC("OpenThemeData");
				_xpt_CloseThemeData = (HRESULT(WINAPI *)(HANDLE))MGPROC("CloseThemeData");
				_xpt_IsThemeBackgroundPartiallyTransparent = (BOOL(WINAPI *)(HANDLE, int, int))MGPROC("IsThemeBackgroundPartiallyTransparent");
				_xpt_DrawThemeParentBackground = (HRESULT(WINAPI *)(HWND, HDC, const RECT *))MGPROC("DrawThemeParentBackground");
				_xpt_DrawThemeBackground = (HRESULT(WINAPI *)(HANDLE, HDC, int, int, const RECT *, const RECT *))MGPROC("DrawThemeBackground");
				_xpt_DrawThemeText = (HRESULT(WINAPI *)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT *))MGPROC("DrawThemeText");				
				_xpt_GetThemePartSize = (HRESULT(WINAPI *)(HTHEME , HDC , int , int , RECT *, int , SIZE * ))MGPROC("GetThemePartSize");
				_xpt_EnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD)) MGPROC("EnableThemeDialogTexture");
			}
		}
		// Make sure all of these methods are valid (i would hope either all or none work)
		if (_xpt_OpenThemeData && 
			_xpt_CloseThemeData && 
			_xpt_IsThemeBackgroundPartiallyTransparent && 
			_xpt_DrawThemeParentBackground && 
			_xpt_DrawThemeBackground && 
			_xpt_DrawThemeText && 
			_xpt_GetThemePartSize) 
		{
			return 1;
		}
	}
	return 0;
}

static void _sttXptCloseThemeData(XPTObject * xptObject)
{
	_xpt_CloseThemeData(xptObject->hThemeHandle);
	xptObject->hThemeHandle=NULL;
}
static void _sttXptReloadThemeData(XPTObject * xptObject)
{
	_xpt_CloseThemeData(xptObject->hThemeHandle);
	xptObject->hThemeHandle=_xpt_OpenThemeData(xptObject->hOwnerWindow, xptObject->lpcwClassObject);
}


HRESULT XPThemesLoadModule()
{
	 if (_xpt_ThemeSupport()) 
	 {
	     InitializeCriticalSection(&xptCS);
		 xptObjectList=li.List_Create(0,1);
		 xptModuleLoaded=TRUE;
	 }
	 return S_OK;
}

void XPThemesUnloadModule()
{
	xptcheck;
	xptlock();
	xptModuleLoaded=FALSE;
	xptunlock();
	li_ListDestruct(xptObjectList,_sttXptObjectDestructor);
	DeleteCriticalSection(&xptCS);
	FreeLibrary(_xpt_ThemeAPIHandle);
}


BOOL xpt_IsThemed(XPTHANDLE xptHandle)
{
	BOOL res=FALSE;
	xptcheck FALSE;
	if (!xptHandle) return FALSE;
	xptlock();
	{
		if (xpt_IsValidHandle(xptHandle) && ((XPTObject*) xptHandle)->hThemeHandle)
			res=TRUE;
	}
	xptunlock();
	return res;	
}

BOOL xpt_IsValidHandle(XPTHANDLE xptHandle)
{
	BOOL res=FALSE;
	xptcheck FALSE;
	if (!xptHandle) return FALSE;
	xptlock();
	{
		if (li.List_IndexOf(xptObjectList, (void*)xptHandle)!=-1) 
			res=TRUE;
	}
	xptunlock();
	return res;
}

XPTHANDLE xpt_AddThemeHandle(HWND hwnd, LPCWSTR className)
{
	XPTHANDLE res = NULL;
	xptcheck NULL;
	xptlock();
	{
		XPTObject* xptObject=(XPTObject*)mir_calloc(sizeof(XPTObject));
		xptObject->lpcwClassObject=className;
		xptObject->hOwnerWindow=hwnd;
		_sttXptReloadThemeData(xptObject);
		li.List_InsertPtr(xptObjectList, (void*)xptObject);
		res=(XPTHANDLE)xptObject;
	}
	xptunlock();
	return res;
}

void xpt_FreeThemeHandle(XPTHANDLE xptHandle)
{
   xptcheck;
   xptlock();
   if (xpt_IsValidHandle(xptHandle))
   {
	   XPTObject* xptObject=(XPTObject*)xptHandle;
	   _sttXptCloseThemeData(xptObject);
	   _sttXptObjectDestructor((void *) xptHandle);
	   li.List_Remove(xptObjectList, li.List_IndexOf(xptObjectList,(void *) xptHandle));
   }
   xptunlock();
}
void xpt_FreeThemeForWindow(HWND hwnd)
{
	xptcheck;
	xptlock();
	{
		int i=0;
		while (i<xptObjectList->realCount)
		{
		   XPTObject* xptObject=(XPTObject*)xptObjectList->items[i];
		   if (xptObject->hOwnerWindow==hwnd)
		   {
		   	   _sttXptCloseThemeData(xptObject);
			   _sttXptObjectDestructor((void *) xptObject);
			   li.List_Remove(xptObjectList, i);		   
		   }
		   else 
			   i++;
		}
	}
	xptunlock();
}

void xpt_OnWM_THEMECHANGED()
{
	xptcheck;
	xptlock();
	{
		int i;
		for (i=0; i<xptObjectList->realCount; i++)
			_sttXptReloadThemeData((XPTObject*)xptObjectList->items[i]);
	}
	xptunlock();
}
HRESULT	xpt_DrawThemeBackground(XPTHANDLE xptHandle, HDC hdc, int type, int state, const RECT * sizeRect, const RECT * clipRect)
{
	HRESULT res=S_FALSE;
	xptcheck S_FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
		res=_xpt_DrawThemeBackground(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, sizeRect, clipRect);
	xptunlock();
	return res;
}
HRESULT	xpt_DrawThemeParentBackground(HWND hWnd, HDC hdc, const RECT * sizeRect)
{
	xptcheck S_FALSE;
	return xpt_DrawThemeParentBackground(hWnd, hdc, sizeRect);
}
BOOL xpt_IsThemeBackgroundPartiallyTransparent(XPTHANDLE xptHandle, int type,  int state)
{
	BOOL res=FALSE;
	xptcheck FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
		res=_xpt_IsThemeBackgroundPartiallyTransparent(((XPTObject*)xptHandle)->hThemeHandle,  type, state);
	xptunlock();
	return res;
}
HRESULT	xpt_DrawTheme(XPTHANDLE xptHandle, HWND hwnd, HDC hdc, int type, int state, const RECT *sizeRect, const RECT * clipRect)
{
	HRESULT res=S_FALSE;
	xptcheck S_FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
	{
		if (_xpt_IsThemeBackgroundPartiallyTransparent(((XPTObject*)xptHandle)->hThemeHandle,  type, state))
			res=_xpt_DrawThemeParentBackground(hwnd,hdc,sizeRect);
			res=_xpt_DrawThemeBackground(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, sizeRect, clipRect);
	}
	xptunlock();
	return res;
}

HRESULT xpt_DrawThemeText(XPTHANDLE xptHandle, HDC hdc, int type, int state, LPCTSTR lpStr, int len, DWORD flag1, DWORD flag2, const RECT * textRect)
{
#ifndef _UNICODE
	ske_DrawText(hdc,lpStr,len, (RECT*)textRect, flag1);
	return S_OK;
#else
	HRESULT res=S_FALSE;
	xptcheck S_FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
		_xpt_DrawThemeText(((XPTObject*)xptHandle)->hThemeHandle,  hdc,  type,  state,  (LPCWSTR)lpStr,  len,  flag1,  flag2,  textRect);
	else
		ske_DrawText(hdc,lpStr,len, (RECT*)textRect, flag1);
	xptunlock();
	return S_OK;
#endif
}
BOOL xpt_EnableThemeDialogTexture(HWND hwnd, DWORD flags)
{
	BOOL res=FALSE;
	xptcheck res;
	xptlock();
	res=_xpt_EnableThemeDialogTexture(hwnd, flags);
	xptunlock();
	return res;
}
//usage outside
// add theme data
// in WM_DESTROY - release theme data
// in paint xpt_DrawTheme

