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

static OBJLIST<XPTObject> xptObjectList(1);
static CRITICAL_SECTION xptCS;

#define xptlock() EnterCriticalSection(&xptCS)
#define xptunlock() LeaveCriticalSection(&xptCS)

static void _sttXptCloseThemeData(XPTObject * xptObject)
{
	CloseThemeData(xptObject->hThemeHandle);
	xptObject->hThemeHandle = NULL;
}
static void _sttXptReloadThemeData(XPTObject * xptObject)
{
	CloseThemeData(xptObject->hThemeHandle);
	xptObject->hThemeHandle = OpenThemeData(xptObject->hOwnerWindow, xptObject->lpcwClassObject);
}


HRESULT XPThemesLoadModule()
{
	InitializeCriticalSection(&xptCS);
	return S_OK;
}

void XPThemesUnloadModule()
{
	xptlock();
	xptunlock();
	xptObjectList.destroy();
	DeleteCriticalSection(&xptCS);
}


BOOL xpt_IsThemed(XPTHANDLE xptHandle)
{
	BOOL res = FALSE;
	if (!xptHandle) return FALSE;
	xptlock();
	{
		if (xpt_IsValidHandle(xptHandle) && ((XPTObject*) xptHandle)->hThemeHandle)
			res = TRUE;
	}
	xptunlock();
	return res;	
}

BOOL xpt_IsValidHandle(XPTHANDLE xptHandle)
{
	BOOL res = FALSE;
	if (!xptHandle) return FALSE;
	xptlock();
	{
		if (xptObjectList.indexOf((XPTObject*)xptHandle) != -1) 
			res = TRUE;
	}
	xptunlock();
	return res;
}

XPTHANDLE xpt_AddThemeHandle(HWND hwnd, LPCWSTR className)
{
	XPTHANDLE res = NULL;
	xptlock();
	{
		XPTObject* xptObject = new XPTObject;
		xptObject->lpcwClassObject = className;
		xptObject->hOwnerWindow = hwnd;
		_sttXptReloadThemeData(xptObject);
		xptObjectList.insert(xptObject);
		res = (XPTHANDLE)xptObject;
	}
	xptunlock();
	return res;
}

void xpt_FreeThemeHandle(XPTHANDLE xptHandle)
{
   xptlock();
   if (xpt_IsValidHandle(xptHandle))
   {
	   XPTObject* xptObject = (XPTObject*)xptHandle;
	   _sttXptCloseThemeData(xptObject);
	   mir_free(xptHandle);
		xptObjectList.remove( xptObjectList.indexOf(xptObject));
   }
   xptunlock();
}
void xpt_FreeThemeForWindow(HWND hwnd)
{
	xptlock();
	{
		for (int i=0; i < xptObjectList.getCount(); )
		{
		   XPTObject& xptObject = xptObjectList[i];
		   if (xptObject.hOwnerWindow == hwnd)
			{
				_sttXptCloseThemeData(&xptObject);
				xptObjectList.remove(i);		   
			}
			else i++;
		}
	}
	xptunlock();
}

void xpt_OnWM_THEMECHANGED()
{
	xptlock();
	{
		for (int i=0; i < xptObjectList.getCount(); i++)
			_sttXptReloadThemeData(&xptObjectList[i]);
	}
	xptunlock();
}

HRESULT	xpt_DrawThemeBackground(XPTHANDLE xptHandle, HDC hdc, int type, int state, const RECT *sizeRect, const RECT *clipRect)
{
	HRESULT res = S_FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
		res = DrawThemeBackground(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, sizeRect, clipRect);
	xptunlock();
	return res;
}

BOOL xpt_IsThemeBackgroundPartiallyTransparent(XPTHANDLE xptHandle, int type,  int state)
{
	BOOL res = FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
		res = IsThemeBackgroundPartiallyTransparent(((XPTObject*)xptHandle)->hThemeHandle,  type, state);
	xptunlock();
	return res;
}

HRESULT	xpt_DrawTheme(XPTHANDLE xptHandle, HWND hwnd, HDC hdc, int type, int state, const RECT *sizeRect, const RECT *clipRect)
{
	HRESULT res = S_FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
	{
		if (IsThemeBackgroundPartiallyTransparent(((XPTObject*)xptHandle)->hThemeHandle,  type, state)) {
			DrawThemeParentBackground(hwnd,hdc,sizeRect);
			res = DrawThemeBackground(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, sizeRect, clipRect);
		}
	}
	xptunlock();
	return res;
}

HRESULT xpt_DrawThemeText(XPTHANDLE xptHandle, HDC hdc, int type, int state, LPCTSTR lpStr, int len, DWORD flag1, DWORD flag2, const RECT *textRect)
{
	HRESULT res = S_FALSE;
	xptlock();
	if (xpt_IsThemed(xptHandle))
		DrawThemeText(((XPTObject*)xptHandle)->hThemeHandle,  hdc,  type,  state,  (LPCWSTR)lpStr,  len,  flag1,  flag2,  textRect);
	else
		ske_DrawText(hdc,lpStr,len, (RECT*)textRect, flag1);
	xptunlock();
	return S_OK;
}
BOOL xpt_EnableThemeDialogTexture(HWND hwnd, DWORD flags)
{
	BOOL res = FALSE;
	xptlock();
	res = EnableThemeDialogTexture(hwnd, flags);
	xptunlock();
	return res;
}
//usage outside
// add theme data
// in WM_DESTROY - release theme data
// in paint xpt_DrawTheme

