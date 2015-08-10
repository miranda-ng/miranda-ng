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

class IEEmbed;

#include <mshtmhst.h>
#include <mshtml.h>
#include <exdisp.h>

#include <atlbase.h>	// CComPtr

#ifndef IEEMBED_INCLUDED
#define IEEMBED_INCLUDED

#define UM_DOCCOMPLETE (WM_USER+600)

class IEEmbedSink :public  DWebBrowserEvents2 {
private:
	int		m_cRef;
	IEEmbed *ieWindow;
public:
	IEEmbedSink(IEEmbed *);
	virtual ~IEEmbedSink();
	// IDispatch
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	STDMETHOD(GetTypeInfoCount)(UINT*);
	STDMETHOD(GetTypeInfo)(UINT, LCID, LPTYPEINFO*);
	STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*);
	STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);
	// DWebBrowserEvents2
	STDMETHODIMP_(void)StatusTextChange(BSTR);
	STDMETHODIMP_(void)ProgressChange(long, long);
	STDMETHODIMP_(void)CommandStateChange(long, VARIANT_BOOL);
	STDMETHODIMP_(void)DownloadBegin();
	STDMETHODIMP_(void)DownloadComplete();
	STDMETHODIMP_(void)TitleChange(BSTR Text);
	STDMETHODIMP_(void)PropertyChange(BSTR Text);
	STDMETHODIMP_(void)BeforeNavigate2(IDispatch*, VARIANT*, VARIANT*, VARIANT*, VARIANT*, VARIANT*, VARIANT_BOOL*);
	STDMETHODIMP_(void)NewWindow2(IDispatch**, VARIANT_BOOL*);
	STDMETHODIMP_(void)NavigateComplete(IDispatch*, VARIANT*);
	STDMETHODIMP_(void)DocumentComplete(IDispatch*, VARIANT*);
	STDMETHODIMP_(void)OnQuit();
	STDMETHODIMP_(void)OnVisible(VARIANT_BOOL);
	STDMETHODIMP_(void)OnToolBar(VARIANT_BOOL);
	STDMETHODIMP_(void)OnMenuBar(VARIANT_BOOL);
	STDMETHODIMP_(void)OnStatusBar(VARIANT_BOOL);
	STDMETHODIMP_(void)OnFullScreen(VARIANT_BOOL);
	STDMETHODIMP_(void)OnTheaterMode(VARIANT_BOOL);
	STDMETHODIMP_(void)WindowSetResizable(VARIANT_BOOL);
	STDMETHODIMP_(void)WindowSetLeft(long);
	STDMETHODIMP_(void)WindowSetTop(long);
	STDMETHODIMP_(void)WindowSetWidth(long);
	STDMETHODIMP_(void)WindowSetHeight(long);
	STDMETHODIMP_(void)WindowClosing(VARIANT_BOOL, VARIANT_BOOL*);
	STDMETHODIMP_(void)ClientToHostWindow(long*, long*);
	STDMETHODIMP_(void)SetSecureLockIcon(long);
	STDMETHODIMP_(void)FileDownload(VARIANT_BOOL*);
};

class IEEmbed :public IDispatch, public IOleClientSite, public IOleInPlaceSite //, public IDocHostUIHandler
{
public:
	HWND parent;
	HWND hwnd;
	int m_cRef;
	RECT rcClient;
	DWORD m_dwCookie;
	CComPtr<IConnectionPoint> m_pConnectionPoint;
	CComPtr<IWebBrowser2> pWebBrowser;
	IEEmbedSink *sink;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT*);
	STDMETHOD(GetTypeInfo)(UINT, LCID, LPTYPEINFO*);
	STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*);
	STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);
	// IOleWindow
	STDMETHOD(GetWindow)(HWND *phwnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	// IOleInPlace
	STDMETHOD(CanInPlaceActivate)(void);
	STDMETHOD(OnInPlaceActivate)(void);
	STDMETHOD(OnUIActivate)(void);
	STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect, LPRECT lprcClipRect,
		LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(Scroll)(SIZE scrollExtant);

	STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
	STDMETHOD(OnInPlaceDeactivate)(void);
	STDMETHOD(DiscardUndoState)(void);
	STDMETHOD(DeactivateAndUndo)(void);
	STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);
	// IOleClientSite
	STDMETHOD(SaveObject)(void);
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
	STDMETHOD(GetContainer)(IOleContainer **ppContainer);
	STDMETHOD(ShowObject)(void);
	STDMETHOD(OnShowWindow)(BOOL fShow);
	STDMETHOD(RequestNewObjectLayout)(void);

	IHTMLDocument2 *getDocument();
	IEEmbed(HWND _parent);
	virtual ~IEEmbed();

	void	ResizeBrowser();
	void    navigate(const wchar_t *);
	void	navigate(char *url);
	void	navigate(NETLIBHTTPREQUEST *nlhr);
	void    write(const wchar_t *text);
	void    addCookie(const wchar_t *cookieString);
	BSTR    getCookies();
	char*	GetHTMLDoc();
};
#endif
