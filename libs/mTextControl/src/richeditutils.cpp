#include "stdafx.h"

class CREOleCallback : public IRichEditOleCallback
{
private:
	unsigned refCount = 1;
	IStorage *pictStg = nullptr;
	int nextStgId = 0;

public:
	CREOleCallback() {}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID * ppvObj)
	{
		if (IsEqualIID(riid, IID_IRichEditOleCallback)) {
			*ppvObj = this;
			this->AddRef();
			return S_OK;
		}
		*ppvObj = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		if (this->refCount == 0) {
			if (S_OK != StgCreateDocfile(nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &this->pictStg))
				this->pictStg = nullptr;
			this->nextStgId = 0;
		}
		return ++this->refCount;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		if (--this->refCount == 0) {
			if (this->pictStg)
				this->pictStg->Release();
		}
		return this->refCount;
	}

	HRESULT STDMETHODCALLTYPE  ContextSensitiveHelp(BOOL)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE  DeleteObject(LPOLEOBJECT)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE  GetClipboardData(CHARRANGE *, DWORD, LPDATAOBJECT *)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE  GetContextMenu(uint16_t, LPOLEOBJECT, CHARRANGE *, HMENU *)
	{
		return E_INVALIDARG;
	}

	HRESULT STDMETHODCALLTYPE  GetDragDropEffect(BOOL, DWORD, LPDWORD)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE  GetInPlaceContext(LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO)
	{
		return E_INVALIDARG;
	}

	HRESULT STDMETHODCALLTYPE  GetNewStorage(LPSTORAGE * lplpstg)
	{
		wchar_t sztName[64];
		mir_snwprintf(sztName, L"s%u", this->nextStgId);
		if (this->pictStg == nullptr)
			return STG_E_MEDIUMFULL;

		return this->pictStg->CreateStorage(sztName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);

	}

	HRESULT STDMETHODCALLTYPE  QueryAcceptData(LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE  QueryInsertObject(LPCLSID, LPSTORAGE, LONG)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE  ShowContainerUI(BOOL)
	{
		return S_OK;
	}
};

static CREOleCallback reOleCallback;

void InitRichEdit(ITextServices *ts)
{
	LRESULT lResult;
	ts->TxSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback, &lResult);
}

/////////////////////////////////////////////////////////////////////////////////////////

static ATOM winClass = 0;

static LRESULT CALLBACK RichEditProxyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ITextServices *ts = (ITextServices *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (ts && (msg != WM_DESTROY)) {
		LRESULT lResult;
		ts->TxSendMessage(msg, wParam, lParam, &lResult);
		return lResult;
	}
	return 1;
}

HWND CreateProxyWindow(ITextServices *ts)
{
	if (winClass == 0) {
		WNDCLASSEX wcl = {};
		wcl.cbSize = sizeof(wcl);
		wcl.lpfnWndProc = RichEditProxyWndProc;
		wcl.style = CS_GLOBALCLASS;
		wcl.hInstance = g_hInst;
		wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		wcl.lpszClassName = L"NBRichEditProxyWndClass";
		winClass = RegisterClassExW(&wcl);
	}

	HWND hwnd = CreateWindow(L"NBRichEditProxyWndClass", L"", 0, 0, 0, 0, 0, nullptr, nullptr, g_hInst, nullptr);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ts);
	return hwnd;
}

void DestroyProxyWindow()
{
	if (winClass != 0)
		UnregisterClassW(L"NBRichEditProxyWndClass", g_hInst);
}