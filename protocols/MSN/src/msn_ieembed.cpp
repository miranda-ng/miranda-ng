/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2007-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include <exdispid.h>
#include "msn_proto.h"
#include "msn_ieembed.h"

IEEmbedSink::IEEmbedSink(IEEmbed *smptr)
{
	ieWindow = smptr;
}

IEEmbedSink::~IEEmbedSink() {}

STDMETHODIMP IEEmbedSink::QueryInterface(REFIID riid, PVOID *ppv)
{
	*ppv = NULL;
	if (IID_IUnknown == riid)
		*ppv = (IUnknown *)this;

	if (IID_IDispatch == riid)
		*ppv = (IDispatch *)this;

	if (DIID_DWebBrowserEvents2 == riid)
		*ppv = (DWebBrowserEvents2*)this;

	if (NULL != *ppv) {
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IEEmbedSink::AddRef(void)
{
	++m_cRef;
	return m_cRef;
}

STDMETHODIMP_(ULONG) IEEmbedSink::Release(void)
{
	--m_cRef;
	return m_cRef;
}

STDMETHODIMP IEEmbedSink::GetTypeInfoCount(UINT *) { return E_NOTIMPL; }
STDMETHODIMP IEEmbedSink::GetTypeInfo(UINT, LCID, LPTYPEINFO*) { return S_OK; }
STDMETHODIMP IEEmbedSink::GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) { return S_OK; }

STDMETHODIMP IEEmbedSink::Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, VARIANT*, EXCEPINFO*, UINT*)
{
	if (!pDispParams) return E_INVALIDARG;
	switch (dispIdMember) {
	case DISPID_DOCUMENTCOMPLETE:
		DocumentComplete(
			pDispParams->rgvarg[1].pdispVal,
			pDispParams->rgvarg[0].pvarVal);
		return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;
}
// DWebBrowserEvents2

void IEEmbedSink::StatusTextChange(BSTR) {}
void IEEmbedSink::ProgressChange(long, long) {}
void IEEmbedSink::CommandStateChange(long, VARIANT_BOOL) {}
void IEEmbedSink::DownloadBegin() {}
void IEEmbedSink::DownloadComplete() {}
void IEEmbedSink::TitleChange(BSTR) {}
void IEEmbedSink::PropertyChange(BSTR) {}
void IEEmbedSink::BeforeNavigate2(IDispatch*, VARIANT* , VARIANT*, VARIANT*, VARIANT*, VARIANT*, VARIANT_BOOL*) { }
void IEEmbedSink::NewWindow2(IDispatch**, VARIANT_BOOL*) {}
void IEEmbedSink::NavigateComplete(IDispatch*, VARIANT*) {}
void IEEmbedSink::DocumentComplete(IDispatch* , VARIANT* url) 
{
	HWND hWnd;
	ieWindow->GetWindow(&hWnd);
	SendMessage(hWnd, UM_DOCCOMPLETE, 0, (LPARAM)url->bstrVal);
}
void IEEmbedSink::OnQuit() {}
void IEEmbedSink::OnVisible(VARIANT_BOOL) {}
void IEEmbedSink::OnToolBar(VARIANT_BOOL) {}
void IEEmbedSink::OnMenuBar(VARIANT_BOOL) {}
void IEEmbedSink::OnStatusBar(VARIANT_BOOL) {}
void IEEmbedSink::OnFullScreen(VARIANT_BOOL) {}
void IEEmbedSink::OnTheaterMode(VARIANT_BOOL) {}
void IEEmbedSink::WindowSetResizable(VARIANT_BOOL) {}
void IEEmbedSink::WindowSetLeft(long) {}
void IEEmbedSink::WindowSetTop(long) {}
void IEEmbedSink::WindowSetWidth(long) {}
void IEEmbedSink::WindowSetHeight(long) {}
void IEEmbedSink::WindowClosing(VARIANT_BOOL, VARIANT_BOOL*) {}
void IEEmbedSink::ClientToHostWindow(long *, long *) {}
void IEEmbedSink::SetSecureLockIcon(long) {}
void IEEmbedSink::FileDownload(VARIANT_BOOL*) {}


static LRESULT CALLBACK IEEmbedServerWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IEEmbed **view = (IEEmbed **)GetWindowLongPtr(GetParent(GetParent(GetParent(hwnd))), GWLP_USERDATA);
	if (view && *view) {
		switch (message) {
		/*
		case WM_KEYUP:
			if (LOWORD(wParam) == VK_ESCAPE && !(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
				SendMessage(GetParent(GetParent(GetParent(hwnd))), WM_COMMAND, IDCANCEL, 0);
			break; */
		
		case WM_KEYDOWN:
			(*view)->translateAccelerator(message, wParam, lParam);
			break;
		
			/*
		case WM_SETFOCUS:
			RECT rcWindow;
			POINT cursor;
			GetWindowRect(hwnd, &rcWindow);
			GetCursorPos(&cursor);
			if (cursor.y <= rcWindow.bottom && cursor.y >= rcWindow.top && cursor.x <= rcWindow.right && cursor.x >= rcWindow.left)
				view->mouseActivate();

			if (view->setFocus((HWND)wParam))
				return TRUE;
			break;
			*/
		
		}
		return CallWindowProc((*view)->getServerWndProc(), hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK IEEmbedDocWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IEEmbed **view = (IEEmbed **)GetWindowLongPtr(GetParent(GetParent(hwnd)), GWLP_USERDATA);
	if (view && *view) {
		WNDPROC oldWndProc = (*view)->getDocWndProc();
		if (message == WM_PARENTNOTIFY && wParam == WM_CREATE) {
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
			(*view)->setServerWndProc((WNDPROC)SetWindowLongPtr((HWND)lParam, GWLP_WNDPROC, (LONG_PTR)IEEmbedServerWindowProcedure));
		}
		return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK IEEmbedWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IEEmbed **view = (IEEmbed **)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (view && *view) {
		WNDPROC oldWndProc = (*view)->getMainWndProc();
		if (message == WM_PARENTNOTIFY && wParam == WM_CREATE) {
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
			(*view)->setDocWndProc((WNDPROC)SetWindowLongPtr((HWND)lParam, GWLP_WNDPROC, (LONG_PTR)IEEmbedDocWindowProcedure));
		}
		return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


IEEmbed::IEEmbed(HWND _parent)
{
	MSG msg;
	parent = _parent;
	GetClientRect(_parent, &rcClient);
	if (SUCCEEDED(pWebBrowser.CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC))) {
		CComPtr<IOleObject> pOleObject;
		if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleObject))) {
			pOleObject->SetClientSite(this);
			pOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, &msg, this, 0, this->parent, &rcClient);
		}
		else MessageBox(NULL, TranslateT("IID_IOleObject failed."), TranslateT("RESULT"), MB_OK);

		CComPtr<IOleInPlaceObject> pOleInPlace;
		if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleInPlace)))
			pOleInPlace->GetWindow(&hwnd);
		else
			MessageBox(NULL, TranslateT("IID_IOleInPlaceObject failed."), TranslateT("RESULT"), MB_OK);

		//setBorder();
		CComPtr<IConnectionPointContainer> pCPContainer;
		// Step 1: Get a pointer to the connection point container.
		if (SUCCEEDED(pWebBrowser.QueryInterface(&pCPContainer))) {
			// m_pConnectionPoint is defined like this:
			// Step 2: Find the connection point.
			if (SUCCEEDED(pCPContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pConnectionPoint))) {
				// Step 3: Advise the connection point that you
				// want to sink its events.
				sink = new IEEmbedSink(this);
				if (FAILED(m_pConnectionPoint->Advise(sink, &m_dwCookie)))
					MessageBox(NULL, TranslateT("Failed to Advise"), TranslateT("C++ Event Sink"), MB_OK);
			}
		}
		setMainWndProc((WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)IEEmbedWindowProcedure));
		SetFocus(hwnd);
	}

	pWebBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
}

IEEmbed::~IEEmbed()
{
	CComPtr<IOleObject> pOleObject;
	if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleObject)))
		pOleObject->SetClientSite(NULL);
	else
		MessageBox(NULL, TranslateT("IID_IOleObject failed."), TranslateT("RESULT"), MB_OK);

	if (m_pConnectionPoint != NULL)
		m_pConnectionPoint->Unadvise(m_dwCookie);

	if (sink != NULL)
		delete sink;
	DestroyWindow(hwnd);
}

void IEEmbed::ResizeBrowser()
{
	CComPtr<IOleInPlaceObject> pOleInPlace;

	GetClientRect(parent, &rcClient);
	if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleInPlace)))
		pOleInPlace->SetObjectRects(&rcClient, &rcClient);
}


// IUnknown
STDMETHODIMP IEEmbed::QueryInterface(REFIID riid, PVOID *ppv)
{
	*ppv = NULL;
	if (IID_IUnknown == riid)
		*ppv = this;
	if (IID_IOleClientSite == riid)
		*ppv = (IOleClientSite*)this;//Unknown)m_pIOleClientSite;
	if (IID_IOleWindow == riid || IID_IOleInPlaceSite == riid)
		*ppv = (IOleInPlaceSite*)this;//m_pIOleIPSite;

	if (NULL != *ppv) {
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IEEmbed::AddRef(void)
{
	++m_cRef;
	return m_cRef;
}

STDMETHODIMP_(ULONG) IEEmbed::Release(void)
{
	--m_cRef;
	return m_cRef;
}

// IDispatch
STDMETHODIMP IEEmbed::GetTypeInfoCount(UINT *) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::GetTypeInfo(UINT, LCID, LPTYPEINFO*) { return S_OK; }
STDMETHODIMP IEEmbed::GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) { return S_OK; }
STDMETHODIMP IEEmbed::Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) { return DISP_E_MEMBERNOTFOUND; }

// IOleWindow
STDMETHODIMP IEEmbed::GetWindow(HWND *phwnd)
{
	*phwnd = parent;
	return S_OK;
}

STDMETHODIMP IEEmbed::ContextSensitiveHelp(BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::CanInPlaceActivate(void) { return S_OK; }
STDMETHODIMP IEEmbed::OnInPlaceActivate(void) {	return S_OK; }
STDMETHODIMP IEEmbed::OnUIActivate(void) { return E_NOTIMPL; }

STDMETHODIMP IEEmbed::GetWindowContext(IOleInPlaceFrame **, IOleInPlaceUIWindow **, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO)
{
	lprcPosRect->left = rcClient.left;
	lprcPosRect->top = rcClient.top;
	lprcPosRect->right = rcClient.right;
	lprcPosRect->bottom = rcClient.bottom;
	lprcClipRect->left = rcClient.left;
	lprcClipRect->top = rcClient.top;
	lprcClipRect->right = rcClient.right;
	lprcClipRect->bottom = rcClient.bottom;
	return S_OK;
}

STDMETHODIMP IEEmbed::Scroll(SIZE) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::OnUIDeactivate(BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::OnInPlaceDeactivate(void) { return S_OK; }
STDMETHODIMP IEEmbed::DiscardUndoState(void) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::DeactivateAndUndo(void) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::OnPosRectChange(LPCRECT) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::SaveObject(void) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::GetMoniker(DWORD, DWORD, IMoniker **) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::GetContainer(IOleContainer **) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::ShowObject(void) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::OnShowWindow(BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEEmbed::RequestNewObjectLayout(void) { return E_NOTIMPL; }


void IEEmbed::setMainWndProc(WNDPROC wndProc)
{
	mainWndProc = wndProc;
}

WNDPROC IEEmbed::getMainWndProc()
{
	return mainWndProc;
}

void IEEmbed::setDocWndProc(WNDPROC wndProc)
{
	docWndProc = wndProc;
}

WNDPROC IEEmbed::getDocWndProc()
{
	return docWndProc;
}

void IEEmbed::setServerWndProc(WNDPROC wndProc)
{
	serverWndProc = wndProc;
}

WNDPROC IEEmbed::getServerWndProc()
{
	return serverWndProc;
}

void IEEmbed::write(const wchar_t *text)
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (!document) return;

	SAFEARRAY *safe_array = ::SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (safe_array != NULL) {
		VARIANT *variant;
		::SafeArrayAccessData(safe_array, (LPVOID *)&variant);
		variant->vt = VT_BSTR;
		variant->bstrVal = ::SysAllocString(text);
		::SafeArrayUnaccessData(safe_array);
		document->write(safe_array);
		// ::SysFreeString(bstr); // don't free it !!!!!!!
		::SafeArrayDestroy(safe_array);
		document->close();
	}
}

void IEEmbed::addCookie(const wchar_t *cookieString)
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (!document) return;
	BSTR cookie = SysAllocString(cookieString);

	document->put_cookie(cookie);

	SysFreeString(cookie);
}

BSTR IEEmbed::getCookies()
{
	CComPtr<IHTMLDocument2> document = getDocument();
	BSTR cookie = NULL;

	if (!document) return NULL;
	document->get_cookie(&cookie);
	return cookie;
}

IHTMLDocument2* IEEmbed::getDocument()
{
	CComPtr<IDispatch> dispatch;
	if (SUCCEEDED(pWebBrowser->get_Document(&dispatch)) && dispatch != NULL) {
		CComPtr<IHTMLDocument2> document;
		dispatch.QueryInterface(&document);
		return document.Detach();
	}

	return NULL;
}

void IEEmbed::navigate(const wchar_t *url)
{
	pWebBrowser->Navigate((WCHAR *)url, NULL, NULL, NULL, NULL);
}

void IEEmbed::navigate(char *url)
{
	wchar_t *pwszUrl = mir_a2u(url);
	navigate(pwszUrl);
	mir_free(pwszUrl);
}

void IEEmbed::navigate(NETLIBHTTPREQUEST *nlhr)
{
	WCHAR *szUrl = mir_a2u(nlhr->szUrl);
	BSTR bstrHeaders;
    LPSAFEARRAY psa;
	LPSTR pPostData;
    VARIANT vPostData = {0}, vHeaders = {0};

	bstrHeaders = SysAllocString(L"Content-Type: application/x-www-form-urlencoded\r\n");
    V_VT(&vHeaders) = VT_BSTR;
    V_BSTR(&vHeaders) = bstrHeaders;
    VariantInit(&vPostData);
    psa = SafeArrayCreateVector(VT_UI1, 0, nlhr->dataLength);
    SafeArrayAccessData(psa, (LPVOID*)&pPostData);
	memcpy(pPostData, nlhr->pData, nlhr->dataLength);
    SafeArrayUnaccessData(psa);
    V_VT(&vPostData) = VT_ARRAY | VT_UI1;
    V_ARRAY(&vPostData) = psa;
	pWebBrowser->Navigate(szUrl, NULL, NULL, &vPostData, &vHeaders);
	SysFreeString(bstrHeaders);
	VariantClear(&vPostData);
	mir_free(szUrl);
}

char *IEEmbed::GetHTMLDoc() {
	CComPtr<IDispatch> spDispDoc;
	char *pszRet = NULL;

	if (SUCCEEDED(pWebBrowser->get_Document(&spDispDoc))) {
		CComPtr<IHTMLDocument3> spDoc;

        if (SUCCEEDED(spDispDoc->QueryInterface(IID_IHTMLDocument3, (void**)&spDoc))) {
			CComPtr<IHTMLElement> spRootElement;
			if (SUCCEEDED(spDoc->get_documentElement(&spRootElement)))
			{
				BSTR bstrDoc;
				if (SUCCEEDED(spRootElement->get_outerHTML(&bstrDoc)))
				{
					pszRet = mir_u2a(bstrDoc);
					SysFreeString(bstrDoc);
				}
			}
		}
	}
	return pszRet;
}

void IEEmbed::translateAccelerator(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CComPtr<IOleInPlaceActiveObject> pIOIPAO;
	if (SUCCEEDED(pWebBrowser.QueryInterface(&pIOIPAO))) {
		MSG msg;
		msg.message = uMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;
		pIOIPAO->TranslateAccelerator(&msg);
	}
}
