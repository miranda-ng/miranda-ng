#include "stdafx.h"

class CREOleCallback : public IRichEditOleCallback
{
private:
	unsigned refCount = 1;
	IStorage *pictStg = nullptr;
	int nextStgId = 0;

public:
	CREOleCallback() {}

	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj) override
	{
		if (IsEqualIID(riid, IID_IRichEditOleCallback)) {
			*ppvObj = this;
			AddRef();
			return S_OK;
		}
		*ppvObj = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef() override
	{
		if (refCount == 0) {
			if (S_OK != StgCreateDocfile(nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &this->pictStg))
				pictStg = nullptr;
			nextStgId = 0;
		}
		return ++refCount;
	}

	ULONG STDMETHODCALLTYPE Release() override
	{
		if (--refCount == 0) {
			if (pictStg)
				pictStg->Release();
		}
		return refCount;
	}

	STDMETHODIMP ContextSensitiveHelp(BOOL) override
	{
		return S_OK;
	}

	STDMETHODIMP DeleteObject(LPOLEOBJECT) override
	{
		return S_OK;
	}

	STDMETHODIMP GetClipboardData(CHARRANGE *, DWORD, LPDATAOBJECT *) override
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetContextMenu(uint16_t, LPOLEOBJECT, CHARRANGE *, HMENU *) override
	{
		return E_INVALIDARG;
	}

	STDMETHODIMP GetDragDropEffect(BOOL, DWORD, LPDWORD) override
	{
		return S_OK;
	}

	STDMETHODIMP GetInPlaceContext(LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO) override
	{
		return E_INVALIDARG;
	}

	STDMETHODIMP GetNewStorage(LPSTORAGE *lplpstg) override
	{
		wchar_t sztName[64];
		mir_snwprintf(sztName, L"s%u", this->nextStgId);
		if (this->pictStg == nullptr)
			return STG_E_MEDIUMFULL;

		return this->pictStg->CreateStorage(sztName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);

	}

	STDMETHODIMP QueryAcceptData(LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL) override
	{
		return S_OK;
	}

	STDMETHODIMP QueryInsertObject(LPCLSID, LPSTORAGE, LONG) override
	{
		return S_OK;
	}

	STDMETHODIMP ShowContainerUI(BOOL) override
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