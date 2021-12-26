/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

#define WM_WAITWHILEBUSY (WM_USER+600)

#define DISPID_BEFORENAVIGATE2      250   // hyperlink clicked on
#define DISPID_NAVIGATECOMPLETE2    252   // UIActivate new document
#define DISPID_DOCUMENTCOMPLETE     259   // new document goes ReadyState_Complete

IEView* IEView::list = nullptr;
mir_cs  IEView::mutex;

static LRESULT CALLBACK IEViewServerWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IEView *view = IEView::get(GetParent(GetParent(hwnd)));
	if (view != nullptr) {
		switch (message) {
		case WM_KEYUP:
			if (LOWORD(wParam) == VK_ESCAPE && !(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
				SendMessage(GetParent(GetParent(GetParent(hwnd))), WM_COMMAND, IDCANCEL, 0);
			break;

		case WM_KEYDOWN:
			view->translateAccelerator(message, wParam, lParam);
			break;

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

		case WM_LBUTTONDOWN:
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (view->mouseClick(pt))
				return TRUE;

			break;
		}
		return CallWindowProc(view->getServerWndProc(), hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK IEViewDocWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IEView *view = IEView::get(GetParent(hwnd));
	if (view != nullptr) {
		WNDPROC oldWndProc = view->getDocWndProc();
		if (message == WM_PARENTNOTIFY && wParam == WM_CREATE) {
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
			view->setServerWndProc((WNDPROC)SetWindowLongPtr((HWND)lParam, GWLP_WNDPROC, (LONG_PTR)IEViewServerWindowProcedure));
		}
		return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK IEViewWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IEView *view = IEView::get(hwnd);
	if (view != nullptr) {
		WNDPROC oldWndProc = view->getMainWndProc();
		if (message == WM_PARENTNOTIFY && wParam == WM_CREATE) {
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
			view->setDocWndProc((WNDPROC)SetWindowLongPtr((HWND)lParam, GWLP_WNDPROC, (LONG_PTR)IEViewDocWindowProcedure));
		}
		return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

IEViewSink::IEViewSink(IEView *smptr) : m_cRef(1)
{
	ieWindow = smptr;
}

IEViewSink::~IEViewSink() {}

STDMETHODIMP IEViewSink::QueryInterface(REFIID riid, PVOID *ppv)
{
	*ppv = nullptr;
	if (IID_IUnknown == riid)
		*ppv = (IUnknown *)this;

	if (IID_IDispatch == riid)
		*ppv = (IDispatch *)this;

	if (DIID_DWebBrowserEvents2 == riid)
		*ppv = (DWebBrowserEvents2*)this;

	if (nullptr != *ppv) {
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IEViewSink::AddRef(void)
{
	++m_cRef;
	return m_cRef;
}

STDMETHODIMP_(ULONG) IEViewSink::Release(void)
{
	--m_cRef;
	return m_cRef;
}

STDMETHODIMP IEViewSink::GetTypeInfoCount(UINT *) { return E_NOTIMPL; }
STDMETHODIMP IEViewSink::GetTypeInfo(UINT, LCID, LPTYPEINFO*) { return S_OK; }
STDMETHODIMP IEViewSink::GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) { return S_OK; }

STDMETHODIMP IEViewSink::Invoke(DISPID dispIdMember, REFIID, LCID, uint16_t, DISPPARAMS* pDispParams, VARIANT*, EXCEPINFO*, UINT*)
{
	if (!pDispParams) return E_INVALIDARG;
	switch (dispIdMember) {
	case DISPID_BEFORENAVIGATE2:
		BeforeNavigate2(pDispParams->rgvarg[6].pdispVal,
			pDispParams->rgvarg[5].pvarVal,
			pDispParams->rgvarg[4].pvarVal,
			pDispParams->rgvarg[3].pvarVal,
			pDispParams->rgvarg[2].pvarVal,
			pDispParams->rgvarg[1].pvarVal,
			pDispParams->rgvarg[0].pboolVal);
		return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}
// DWebBrowserEvents2

void IEViewSink::StatusTextChange(BSTR) {}
void IEViewSink::ProgressChange(long, long) {}
void IEViewSink::CommandStateChange(long, VARIANT_BOOL) {}
void IEViewSink::DownloadBegin() {}
void IEViewSink::DownloadComplete() {}
void IEViewSink::TitleChange(BSTR) {}
void IEViewSink::PropertyChange(BSTR) {}
void IEViewSink::BeforeNavigate2(IDispatch*, VARIANT* url, VARIANT*, VARIANT*, VARIANT*, VARIANT*, VARIANT_BOOL* cancel)
{
	if (mir_wstrcmp(url->bstrVal, L"about:blank")) {
		Utils_OpenUrlW(url->bstrVal);
		*cancel = VARIANT_TRUE;
	}
}

void IEViewSink::NewWindow2(IDispatch**, VARIANT_BOOL*) {}
void IEViewSink::NavigateComplete(IDispatch*, VARIANT*) {}
void IEViewSink::DocumentComplete(IDispatch*, VARIANT*) {}
void IEViewSink::OnQuit() {}
void IEViewSink::OnVisible(VARIANT_BOOL) {}
void IEViewSink::OnToolBar(VARIANT_BOOL) {}
void IEViewSink::OnMenuBar(VARIANT_BOOL) {}
void IEViewSink::OnStatusBar(VARIANT_BOOL) {}
void IEViewSink::OnFullScreen(VARIANT_BOOL) {}
void IEViewSink::OnTheaterMode(VARIANT_BOOL) {}
void IEViewSink::WindowSetResizable(VARIANT_BOOL) {}
void IEViewSink::WindowSetLeft(long) {}
void IEViewSink::WindowSetTop(long) {}
void IEViewSink::WindowSetWidth(long) {}
void IEViewSink::WindowSetHeight(long) {}
void IEViewSink::WindowClosing(VARIANT_BOOL, VARIANT_BOOL*) {}
void IEViewSink::ClientToHostWindow(long *, long *) {}
void IEViewSink::SetSecureLockIcon(long) {}
void IEViewSink::FileDownload(VARIANT_BOOL*) {}

void IEView::waitWhileBusy()
{
	VARIANT_BOOL busy;
	pWebBrowser->get_Busy(&busy);
	while (busy == VARIANT_TRUE) {
		Sleep(10);
		pWebBrowser->get_Busy(&busy);
	}
	PostMessage(hwnd, WM_WAITWHILEBUSY, 0, 0);
}

void IEView::setBorder()
{
	LONG style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	LONG oldStyle = style;
	if (Options::generalFlags & Options::GENERAL_NO_BORDER)
		style &= ~(WS_EX_STATICEDGE);
	else
		style |= (WS_EX_STATICEDGE);

	if (oldStyle != style) {
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
		SetWindowPos(getHWND(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	}
}

IEView::IEView(HWND _parent, HTMLBuilder *_builder, int x, int y, int cx, int cy)
{
	MSG msg;
	parent = _parent;
	builder = _builder;
	rcClient.left = x;
	rcClient.top = y;
	rcClient.right = x + cx;
	rcClient.bottom = y + cy;
	if (SUCCEEDED(pWebBrowser.CoCreateInstance(CLSID_WebBrowser, nullptr, CLSCTX_INPROC))) {
		CComPtr<IOleObject> pOleObject;
		if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleObject))) {
			pOleObject->SetClientSite(this);
			pOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, &msg, this, 0, this->parent, &rcClient);
		}

		CComPtr<IOleInPlaceObject> pOleInPlace;
		if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleInPlace)))
			pOleInPlace->GetWindow(&hwnd);

		setBorder();
		CComPtr<IConnectionPointContainer> pCPContainer;
		// Step 1: Get a pointer to the connection point container.
		if (SUCCEEDED(pWebBrowser.QueryInterface(&pCPContainer))) {
			// m_pConnectionPoint is defined like this:
			// Step 2: Find the connection point.
			if (SUCCEEDED(pCPContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pConnectionPoint))) {
				// Step 3: Advise the connection point that you
				// want to sink its events.
				sink = new IEViewSink(this);
				m_pConnectionPoint->Advise(sink, &m_dwCookie);
			}
		}
		setMainWndProc((WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)IEViewWindowProcedure));
	}
	{
		mir_cslock lck(mutex);
		next = list;
		if (next != nullptr)
			next->prev = this;
		list = this;
	}

	pWebBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
}

IEView::~IEView()
{
	{
		mir_cslock lck(mutex);
		if (list == this)
			list = next;
		else if (prev != nullptr)
			prev->next = next;

		if (next != nullptr)
			next->prev = prev;

		prev = nullptr;
		next = nullptr;
	}

	CComPtr<IOleObject> pOleObject;
	if (SUCCEEDED(pWebBrowser.QueryInterface(&pOleObject)))
		pOleObject->SetClientSite(nullptr);

	if (builder != nullptr) {
		delete builder;
		builder = nullptr;
	}

	if (m_pConnectionPoint != nullptr)
		m_pConnectionPoint->Unadvise(m_dwCookie);

	if (sink != nullptr)
		delete sink;
	DestroyWindow(hwnd);
}

void IEView::release()
{
	mir_cslock lck(mutex);
	while (list != nullptr)
		delete list;
	list = nullptr;
}

IEView* IEView::get(HWND hwnd)
{
	if (list == nullptr) return nullptr;

	mir_cslock lock(mutex);
	for (IEView *ptr = list; ptr != nullptr; ptr = ptr->next)
		if (ptr->hwnd == hwnd)
			return ptr;

	return nullptr;
}

void IEView::setMainWndProc(WNDPROC wndProc)
{
	mainWndProc = wndProc;
}

WNDPROC IEView::getMainWndProc()
{
	return mainWndProc;
}

void IEView::setDocWndProc(WNDPROC wndProc)
{
	docWndProc = wndProc;
}

WNDPROC IEView::getDocWndProc()
{
	return docWndProc;
}

void IEView::setServerWndProc(WNDPROC wndProc)
{
	serverWndProc = wndProc;
}

WNDPROC IEView::getServerWndProc()
{
	return serverWndProc;
}

// IUnknown
STDMETHODIMP IEView::QueryInterface(REFIID riid, PVOID *ppv)
{
	*ppv = nullptr;
	if (IID_IUnknown == riid)
		*ppv = this;
	if (IID_IOleClientSite == riid)
		*ppv = (IOleClientSite*)this;//Unknown)m_pIOleClientSite;
	if (IID_IOleWindow == riid || IID_IOleInPlaceSite == riid)
		*ppv = (IOleInPlaceSite*)this;//m_pIOleIPSite;
	if (IID_IDocHostUIHandler == riid)
		*ppv = (IDocHostUIHandler*)this;//m_pIOleIPSite;
	if (IID_IInternetSecurityManager == riid)
		*ppv = (IInternetSecurityManager*)this;
	if (IID_IServiceProvider == riid)
		*ppv = (IServiceProvider*)this;
	if (IID_IDispatch == riid)
		*ppv = (IDispatch*)this;

	if (nullptr != *ppv) {
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IEView::AddRef(void)
{
	++m_cRef;
	return m_cRef;
}

STDMETHODIMP_(ULONG) IEView::Release(void)
{
	--m_cRef;
	return m_cRef;
}

// IDispatch
STDMETHODIMP IEView::GetTypeInfoCount(UINT *pctinfo)
{
	if (pctinfo == nullptr) return E_INVALIDARG;
	*pctinfo = 4;
	return S_OK;
}
STDMETHODIMP IEView::GetTypeInfo(UINT, LCID, LPTYPEINFO*)
{
	return S_OK;
}
STDMETHODIMP IEView::GetIDsOfNames(REFIID /*riid*/, LPOLESTR *rgszNames, UINT cNames, LCID /*lcid*/, DISPID *rgDispId)
{
	HRESULT retval = S_OK;
	for (size_t i = 0; i < cNames; i++)
	{
		if (!wcscmp(L"db_get", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_DB_GET;
		else if (!wcscmp(L"db_set", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_DB_SET;

		else if (!wcscmp(L"win32_ShellExecute", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_WIN32_SHELL_EXECUTE;
		else if (!wcscmp(L"win32_CopyToClipboard", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_WIN32_COPY_TO_CLIPBOARD;

		else if (!wcscmp(L"IEView_GetCurrentContact", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_GET_CURRENTCONTACT;

		else if (!wcscmp(L"mir_CallService", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_CALLSERVICE;
		else if (!wcscmp(L"mir_CallContactService", rgszNames[i]))
			rgDispId[i] = DISPID_EXTERNAL_CALLCONTACTSERVICE;

		else
		{
			rgDispId[i] = NULL;
			retval = DISP_E_UNKNOWNNAME;
		}
	}
	return retval;
}

STDMETHODIMP IEView::Invoke(DISPID dispIdMember,
	REFIID /*riid*/,
	LCID /*lcid*/,
	uint16_t /*wFlags*/,
	DISPPARAMS *pDispParams,
	VARIANT *pVarResult,
	EXCEPINFO * /*pExcepInfo*/,
	UINT * /*puArgErr*/)
{

	switch (dispIdMember)
	{
	case DISPID_EXTERNAL_CALLSERVICE:
		return External::mir_CallService(pDispParams, pVarResult);
	case DISPID_EXTERNAL_CALLCONTACTSERVICE:
		return External::mir_CallContactService(pDispParams, pVarResult);

	case DISPID_EXTERNAL_DB_GET:
		return External::db_get(pDispParams, pVarResult);
	case DISPID_EXTERNAL_DB_SET:
		return External::db_set(pDispParams, pVarResult);

	case DISPID_EXTERNAL_WIN32_SHELL_EXECUTE:
		return External::win32_ShellExecute(pDispParams, pVarResult);
	case DISPID_EXTERNAL_WIN32_COPY_TO_CLIPBOARD:
		return External::win32_CopyToClipboard(pDispParams, pVarResult);

	case DISPID_EXTERNAL_GET_CURRENTCONTACT:
		return External::IEView_GetCurrentContact(this, pDispParams, pVarResult);
	}

	return DISP_E_MEMBERNOTFOUND;
}

// IOleWindow
STDMETHODIMP IEView::GetWindow(HWND *phwnd)
{
	*phwnd = parent;
	return S_OK;
}

STDMETHODIMP IEView::ContextSensitiveHelp(BOOL)
{
	return E_NOTIMPL;
}

// IOleInPlace
STDMETHODIMP IEView::CanInPlaceActivate(void)
{
	return S_OK;
}

STDMETHODIMP IEView::OnInPlaceActivate(void)
{
	m_bInPlaceActive = TRUE;
	return S_OK;
}

STDMETHODIMP IEView::OnUIActivate(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP IEView::GetWindowContext(IOleInPlaceFrame **, IOleInPlaceUIWindow **, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO)
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

STDMETHODIMP IEView::Scroll(SIZE)
{
	return E_NOTIMPL;
}

STDMETHODIMP IEView::OnUIDeactivate(BOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP IEView::OnInPlaceDeactivate(void)
{
	m_bInPlaceActive = FALSE;
	return S_OK;
}

STDMETHODIMP IEView::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP IEView::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP IEView::OnPosRectChange(LPCRECT)
{
	return E_NOTIMPL;
}

// IOleClientSite
STDMETHODIMP IEView::SaveObject(void)
{
	return E_NOTIMPL;
}
STDMETHODIMP IEView::GetMoniker(DWORD, DWORD, IMoniker **)
{
	return E_NOTIMPL;
}
STDMETHODIMP IEView::GetContainer(IOleContainer **)
{
	return E_NOTIMPL;
}
STDMETHODIMP IEView::ShowObject(void)
{
	return E_NOTIMPL;
}
STDMETHODIMP IEView::OnShowWindow(BOOL)
{
	return E_NOTIMPL;
}
STDMETHODIMP IEView::RequestNewObjectLayout(void)
{
	return E_NOTIMPL;
}
// IDocHostUIHandler
STDMETHODIMP IEView::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdTarget, IDispatch *)
{
	CComPtr<IOleCommandTarget> pOleCommandTarget;
	if (SUCCEEDED(pcmdTarget->QueryInterface(IID_IOleCommandTarget, (void**)&pOleCommandTarget))) {
		CComPtr<IOleWindow> pOleWindow;
		if (SUCCEEDED(pOleCommandTarget.QueryInterface(&pOleWindow))) {
			HWND hSPWnd;
			pOleWindow->GetWindow(&hSPWnd);

			HMENU hMenu = GetSubMenu(LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXTMENU)), 0);
			TranslateMenu(hMenu);
			if (dwID == 5) // anchor
				EnableMenuItem(hMenu, ID_MENU_COPYLINK, MF_BYCOMMAND | MF_ENABLED);
			else if (dwID == 4) // text select
				EnableMenuItem(hMenu, ID_MENU_COPY, MF_BYCOMMAND | MF_ENABLED);
			else if (dwID == 1) { // control (image) 
				EnableMenuItem(hMenu, ID_MENU_SAVEIMAGE, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(hMenu, ID_MENU_COPY, MF_BYCOMMAND | MF_ENABLED);
			}

			int iSelection = TrackPopupMenu(hMenu,
				TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
				ppt->x,
				ppt->y,
				0,
				hwnd,
				(RECT*)nullptr);
			DestroyMenu(hMenu);
			if (iSelection == ID_MENU_CLEARLOG)
				clear(nullptr);
			else
				SendMessage(hSPWnd, WM_COMMAND, iSelection, 0);
		}
	}

	return S_OK;
}
STDMETHODIMP IEView::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER;// | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE;
	if (builder == nullptr) {
		pInfo->dwFlags |= DOCHOSTUIFLAG_DIALOG;
	}
	return S_OK;
}

STDMETHODIMP IEView::ShowUI(DWORD, IOleInPlaceActiveObject *, IOleCommandTarget *, IOleInPlaceFrame *, IOleInPlaceUIWindow *)
{
	return S_OK;
}

STDMETHODIMP IEView::HideUI(void) { return S_OK; }
STDMETHODIMP IEView::UpdateUI(void) { return S_OK; }
STDMETHODIMP IEView::EnableModeless(BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEView::OnDocWindowActivate(BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEView::OnFrameWindowActivate(BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEView::ResizeBorder(LPCRECT, IOleInPlaceUIWindow *, BOOL) { return E_NOTIMPL; }
STDMETHODIMP IEView::TranslateAccelerator(LPMSG, const GUID *, DWORD) { return S_FALSE; }
STDMETHODIMP IEView::GetOptionKeyPath(LPOLESTR *, DWORD) { return E_NOTIMPL; }
STDMETHODIMP IEView::GetDropTarget(IDropTarget *, IDropTarget **ppDropTarget)
{
	*ppDropTarget = nullptr;
	return S_OK;
	//	return E_NOTIMPL;
}

STDMETHODIMP IEView::GetExternal(IDispatch **ppDispatch)
{
	*ppDispatch = this;
	return S_OK;
}
STDMETHODIMP IEView::TranslateUrl(DWORD, OLECHAR *, OLECHAR **) { return E_NOTIMPL; }
STDMETHODIMP IEView::FilterDataObject(IDataObject *, IDataObject **) { return E_NOTIMPL; }


/* IServiceProvider */
STDMETHODIMP IEView::QueryService(REFGUID guidService, REFIID riid, void** ppvObject)
{
	if (guidService == SID_SInternetSecurityManager && riid == IID_IInternetSecurityManager) {
		return (HRESULT)this->QueryInterface(riid, ppvObject);
	}
	else {
		*ppvObject = nullptr;
	}
	return E_NOINTERFACE;
}

/* IInternetSecurityManager */

STDMETHODIMP IEView::SetSecuritySite(IInternetSecurityMgrSite *)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::GetSecuritySite(IInternetSecurityMgrSite **)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::MapUrlToZone(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD)
{
	if (pdwZone != nullptr && pwszUrl != nullptr && !mir_wstrcmp(pwszUrl, L"about:blank")) {
		*pdwZone = URLZONE_LOCAL_MACHINE;
		return S_OK;
	}
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::GetSecurityId(LPCWSTR, uint8_t *, DWORD *, DWORD_PTR)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, uint8_t *pPolicy, DWORD cbPolicy, uint8_t *, DWORD, DWORD, DWORD)
{
	uint32_t dwPolicy = URLPOLICY_ALLOW;
	if (pwszUrl != nullptr && !mir_wstrcmp(pwszUrl, L"about:blank")) {
		if (dwAction <= URLACTION_ACTIVEX_MAX && dwAction >= URLACTION_ACTIVEX_MIN) {
			//dwPolicy = URLPOLICY_DISALLOW;
			//dwPolicy = URLPOLICY_ALLOW;
		}
		else if ((dwAction <= URLACTION_JAVA_MAX && dwAction >= URLACTION_JAVA_MIN) || URLACTION_HTML_JAVA_RUN == dwAction) {
			// dwPolicy = URLPOLICY_JAVA_PROHIBIT;
			return INET_E_DEFAULT_ACTION;
		}
		else if (dwAction <= URLACTION_SCRIPT_MAX && dwAction >= URLACTION_SCRIPT_MIN) {
			//dwPolicy = URLPOLICY_DISALLOW;
			//dwPolicy = URLPOLICY_ALLOW;
		}
		else if (dwAction <= URLACTION_HTML_MIN && dwAction >= URLACTION_HTML_MAX) {
			//dwPolicy = URLPOLICY_DISALLOW;
			//dwPolicy = URLPOLICY_ALLOW;
		}
		else return INET_E_DEFAULT_ACTION;

		if (cbPolicy >= sizeof(uint32_t)) {
			*(uint32_t*)pPolicy = dwPolicy;
			return S_OK;
		}

		return S_FALSE;
	}
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::QueryCustomPolicy(LPCWSTR, REFGUID, uint8_t **, DWORD *, uint8_t *, DWORD, DWORD)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::SetZoneMapping(DWORD, LPCWSTR, DWORD)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP IEView::GetZoneMappings(DWORD, IEnumString **, DWORD)
{
	return INET_E_DEFAULT_ACTION;
}

IHTMLDocument2* IEView::getDocument()
{
	CComPtr<IDispatch> dispatch;
	if (SUCCEEDED(pWebBrowser->get_Document(&dispatch)) && dispatch != nullptr) {
		CComPtr<IHTMLDocument2> document;
		dispatch.QueryInterface(&document);
		return document.Detach();
	}

	return nullptr;
}

void IEView::setWindowPos(int x, int y, int cx, int cy)
{
	rcClient.left = x;
	rcClient.top = y;
	rcClient.right = cx;
	rcClient.bottom = cy;

	SetWindowPos(getHWND(), HWND_TOP, x, y, cx, cy, 0);
	/*
	CComPtr<IOleInPlaceObject> inPlaceObject;
	if (SUCCEEDED(pWebBrowser.QueryInterface(&inPlaceObject)))
		inPlaceObject->SetObjectRects(&rcClient, &rcClient);
	*/
}

void IEView::scrollToTop()
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document) {
		CComPtr<IHTMLWindow2> pWindow;
		if (SUCCEEDED(document->get_parentWindow(&pWindow)) && pWindow != nullptr)
			pWindow->scrollBy(-0x01FFFFFF, -0x01FFFFFF);
	}
}

void IEView::scrollToBottomSoft()
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document) {
		CComPtr<IHTMLWindow2> pWindow;
		if (SUCCEEDED(document->get_parentWindow(&pWindow)) && pWindow != nullptr)
			pWindow->scrollBy(-0x01FFFFFF, 0x01FFFFFF);
	}
}

void IEView::scrollToBottom()
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document == nullptr)
		return;

	CComPtr<IHTMLElementCollection> collection;
	if (SUCCEEDED(document->get_all(&collection)) && collection != nullptr) {
		long len;
		if (SUCCEEDED(collection->get_length(&len))) {
			VARIANT variant;
			variant.vt = VT_I4;
			variant.lVal = len - 1;

			CComPtr<IDispatch> dispatch;
			if (SUCCEEDED(collection->item(variant, variant, &dispatch)) && dispatch != nullptr) {
				CComPtr<IHTMLElement> element;
				if (SUCCEEDED(dispatch.QueryInterface(&element)) && element != nullptr) {
					variant.vt = VT_BOOL;
					variant.boolVal = VARIANT_FALSE;
					element->scrollIntoView(variant);
				}
			}
		}
	}

	CComPtr<IHTMLWindow2> pWindow;
	if (SUCCEEDED(document->get_parentWindow(&pWindow)) && pWindow != nullptr)
		pWindow->scrollBy(-0x0000FFFF, 0x0000FFFF);
}

void IEView::write(const wchar_t *text)
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document == nullptr)
		return;

	SAFEARRAY *safe_array = ::SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (safe_array != nullptr) {
		VARIANT *variant;
		::SafeArrayAccessData(safe_array, (LPVOID *)&variant);
		variant->vt = VT_BSTR;
		variant->bstrVal = ::SysAllocString(text);
		::SafeArrayUnaccessData(safe_array);
		document->write(safe_array);
		// ::SysFreeString(bstr); // don't free it !!!!!!!
		::SafeArrayDestroy(safe_array);
	}
}

void IEView::write(const char *text)
{
	write(ptrW(mir_utf8decodeW(text)));
}

void IEView::writef(const char *fmt, ...)
{
	int strsize = 2048;
	va_list vararg;
	va_start(vararg, fmt);
	char *str = (char *)malloc(strsize);
	while (mir_vsnprintf(str, strsize, fmt, vararg) == -1)
		str = (char *)realloc(str, strsize += 2048);
	va_end(vararg);
	write(str);
	free(str);
}

void IEView::navigate(const char *url)
{
	pWebBrowser->Navigate(BSTR_PTR(_A2T(url)), nullptr, nullptr, nullptr, nullptr);
}

void IEView::navigate(const wchar_t *url)
{
	pWebBrowser->Navigate(BSTR_PTR(url), nullptr, nullptr, nullptr, nullptr);
}

void IEView::documentClose()
{
}

void IEView::appendEventOld(IEVIEWEVENT *event)
{
	if (clearRequired)
		clear(event);

	if (builder != nullptr)
		builder->appendEventOld(this, event);

	getFocus = false;
}

void IEView::appendEvent(IEVIEWEVENT *event)
{
	if (clearRequired)
		clear(event);

	if (event->eventData == nullptr)
		return;

	if (builder != nullptr)
		builder->appendEventNew(this, event);

	getFocus = false;
}

void IEView::clear(IEVIEWEVENT *event)
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document == nullptr) {
		pWebBrowser->Navigate(BSTR_PTR(L"about:blank"), nullptr, nullptr, nullptr, nullptr);
		HRESULT hr = S_OK;
		CComPtr<IHTMLDocument2> doc2;
		while ((doc2 == nullptr) && (hr == S_OK)) {
			Sleep(0);
			CComPtr<IDispatch> dispatch;
			if (SUCCEEDED(pWebBrowser->get_Document(&dispatch)) && dispatch != nullptr)
				dispatch.QueryInterface(&doc2);
		}
	}
	else {
		document->close();
		VARIANT open_name, open_features, open_replace;

		VariantInit(&open_name);
		open_name.vt = VT_BSTR;
		open_name.bstrVal = SysAllocString(L"_self");
		VariantInit(&open_features);
		VariantInit(&open_replace);

		CComPtr<IDispatch> open_window;
		document->open(BSTR_PTR(L"text/html"), open_name, open_features, open_replace, &open_window);
	}
	if (builder != nullptr)
		builder->clear(this, event);

	clearRequired = false;
	getFocus = false;
	setBorder();
}

wchar_t* IEView::selection()
{
	wszSelectedText = getSelection();
	return (mir_wstrlen(wszSelectedText) == 0) ? nullptr : wszSelectedText.get();
}

HWND IEView::getHWND()
{
	return hwnd;
}

void IEView::setContact(MCONTACT _hContact)
{
	hContact = _hContact;
	isContactSet = true;
}

void IEView::translateAccelerator(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

/**
 * Returns the selected text within the active document
 **/
wchar_t* IEView::getSelection()
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document == nullptr)
		return nullptr;

	CComPtr<IHTMLSelectionObject> pSelection;
	if (FAILED(document->get_selection(&pSelection)) || pSelection == nullptr)
		return nullptr;

	CComPtr<IDispatch> pDisp;
	if (FAILED(pSelection->createRange(&pDisp)) || pDisp == nullptr)
		return nullptr;

	CComPtr<IHTMLTxtRange> pRange;
	if (FAILED(pDisp.QueryInterface(&pRange)))
		return nullptr;

	BSTR text = nullptr;
	if (FAILED(pRange->get_text(&text)))
		return nullptr;

	wchar_t *res = mir_wstrdup(text);
	::SysFreeString(text);
	return res;
}


/**
 * Returns the destination url (href) of the given anchor element (or parent anchor element)
 **/
wchar_t* IEView::getHrefFromAnchor(CComPtr<IHTMLElement> element)
{
	if (element == nullptr)
		return nullptr;

	CComPtr<IHTMLAnchorElement> pAnchor;
	if (FAILED(element.QueryInterface(&pAnchor))) {
		VARIANT variant;
		wchar_t *url = nullptr;
		if (SUCCEEDED(element->getAttribute(BSTR_PTR(L"href"), 2, &variant)) && variant.vt == VT_BSTR) {
			url = mir_wstrdup(variant.bstrVal);
			::SysFreeString(variant.bstrVal);
		}

		return url;
	}

	CComPtr<IHTMLElement> pParent;
	if (SUCCEEDED(element->get_parentElement(&pParent)) && pParent != nullptr)
		return getHrefFromAnchor(pParent);

	return nullptr;
}

bool IEView::mouseActivate()
{
	if (GetFocus() != hwnd)
		getFocus = true;

	return false;
}

bool IEView::mouseClick(POINT pt)
{
	if (GetFocus() != hwnd)
		getFocus = true;

	CComPtr<IHTMLDocument2> document = getDocument();
	if (document == nullptr)
		return false;

	CComPtr<IHTMLElement> element;
	if (SUCCEEDED(document->elementFromPoint(pt.x, pt.y, &element)) && element != nullptr) {
		ptrW url(getHrefFromAnchor(element));
		if (url != NULL) {
			if ((GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
				SendMessage(GetParent(hwnd), WM_COMMAND, IDCANCEL, 0);

			Utils_OpenUrlW(url);
			return true;
		}
	}

	return false;
}

bool IEView::setFocus(HWND)
{
	if (GetFocus() != hwnd && !getFocus) {
		SetFocus(GetParent(getHWND()));
		return true;
	}

	getFocus = false;
	return false;
}

void IEView::saveDocument()
{
	CComPtr<IHTMLDocument2> document = getDocument();
	if (document == nullptr)
		return;

	VARIANT vValue;
	vValue.vt = VT_BOOL;
	vValue.boolVal = VARIANT_TRUE;

	VARIANT_BOOL vb;
	document->execCommand(BSTR_PTR(L"SaveAs"), VARIANT_FALSE, vValue, &vb);
}

void IEView::navigate(IEVIEWNAVIGATE *nav)
{
	if (nav->dwFlags & IENF_UNICODE)
		navigate(nav->url.w);
	else
		navigate(nav->url.a);
}