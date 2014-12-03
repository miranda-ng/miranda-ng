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
static mir_cs xptCS;

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

void XPThemesUnloadModule()
{
	xptObjectList.destroy();
}

BOOL xpt_IsThemed(XPTHANDLE xptHandle)
{
	if (!xptHandle) return FALSE;

	mir_cslock lck(xptCS);
	return xpt_IsValidHandle(xptHandle) && ((XPTObject*)xptHandle)->hThemeHandle;
}

BOOL xpt_IsValidHandle(XPTHANDLE xptHandle)
{
	if (!xptHandle) return FALSE;

	mir_cslock lck(xptCS);
	return xptObjectList.indexOf((XPTObject*)xptHandle) != -1;
}

XPTHANDLE xpt_AddThemeHandle(HWND hwnd, LPCWSTR className)
{
	mir_cslock lck(xptCS);
	XPTObject* xptObject = new XPTObject;
	xptObject->lpcwClassObject = className;
	xptObject->hOwnerWindow = hwnd;
	_sttXptReloadThemeData(xptObject);
	xptObjectList.insert(xptObject);
	return (XPTHANDLE)xptObject;
}

void xpt_FreeThemeHandle(XPTHANDLE xptHandle)
{
	mir_cslock lck(xptCS);
	if (xpt_IsValidHandle(xptHandle)) {
		XPTObject* xptObject = (XPTObject*)xptHandle;
		_sttXptCloseThemeData(xptObject);
		mir_free(xptHandle);
		xptObjectList.remove(xptObjectList.indexOf(xptObject));
	}
}

void xpt_FreeThemeForWindow(HWND hwnd)
{
	mir_cslock lck(xptCS);
	for (int i = 0; i < xptObjectList.getCount();) {
		XPTObject& xptObject = xptObjectList[i];
		if (xptObject.hOwnerWindow == hwnd) {
			_sttXptCloseThemeData(&xptObject);
			xptObjectList.remove(i);
		}
		else i++;
	}
}

void xpt_OnWM_THEMECHANGED()
{
	mir_cslock lck(xptCS);

	for (int i = 0; i < xptObjectList.getCount(); i++)
		_sttXptReloadThemeData(&xptObjectList[i]);
}

HRESULT	xpt_DrawThemeBackground(XPTHANDLE xptHandle, HDC hdc, int type, int state, const RECT *sizeRect, const RECT *clipRect)
{
	mir_cslock lck(xptCS);
	if (xpt_IsThemed(xptHandle))
		return DrawThemeBackground(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, sizeRect, clipRect);

	return S_FALSE;
}

BOOL xpt_IsThemeBackgroundPartiallyTransparent(XPTHANDLE xptHandle, int type, int state)
{
	mir_cslock lck(xptCS);
	if (xpt_IsThemed(xptHandle))
		return IsThemeBackgroundPartiallyTransparent(((XPTObject*)xptHandle)->hThemeHandle, type, state);

	return FALSE;
}

HRESULT	xpt_DrawTheme(XPTHANDLE xptHandle, HWND hwnd, HDC hdc, int type, int state, const RECT *sizeRect, const RECT *clipRect)
{
	mir_cslock lck(xptCS);
	if (xpt_IsThemed(xptHandle)) {
		if (IsThemeBackgroundPartiallyTransparent(((XPTObject*)xptHandle)->hThemeHandle, type, state)) {
			DrawThemeParentBackground(hwnd, hdc, sizeRect);
			return DrawThemeBackground(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, sizeRect, clipRect);
		}
	}

	return S_FALSE;
}

HRESULT xpt_DrawThemeText(XPTHANDLE xptHandle, HDC hdc, int type, int state, LPCTSTR lpStr, int len, DWORD flag1, DWORD flag2, const RECT *textRect)
{
	mir_cslock lck(xptCS);
	if (xpt_IsThemed(xptHandle))
		DrawThemeText(((XPTObject*)xptHandle)->hThemeHandle, hdc, type, state, (LPCWSTR)lpStr, len, flag1, flag2, textRect);
	else
		ske_DrawText(hdc, lpStr, len, (RECT*)textRect, flag1);

	return S_OK;
}

BOOL xpt_EnableThemeDialogTexture(HWND hwnd, DWORD flags)
{
	mir_cslock lck(xptCS);
	return SUCCEEDED(EnableThemeDialogTexture(hwnd, flags));
}
