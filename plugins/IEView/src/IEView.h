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
class IEView;

#ifndef IEVIEW_INCLUDED
#define IEVIEW_INCLUDED

#include "ieview_common.h"
#include "HTMLBuilder.h"
//#include "SmileyWindow.h"

static const CLSID CLSID_MozillaBrowser =
{ 0x1339B54C, 0x3453, 0x11D2,
{ 0x93, 0xB9, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00 } };

#ifndef DISPID_AMBIENT_DLCONTROL
#define DISPID_AMBIENT_DLCONTROL                (-5512)
#endif

#ifndef __IDocHostUIHandler_INTERFACE_DEFINED__
#define __IDocHostUIHandler_INTERFACE_DEFINED__

typedef struct _DOCHOSTUIINFO
{
	ULONG cbSize;
	DWORD dwFlags;
	DWORD dwDoubleClick;
	OLECHAR *pchHostCss;
	OLECHAR *pchHostNS;
} DOCHOSTUIINFO;

const IID IID_IDocHostUIHandler = { 0xbd3f23c0, 0xd43e, 0x11CF, { 0x89, 0x3b, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x1a } };

MIDL_INTERFACE("bd3f23c0-d43e-11cf-893b-00aa00bdce1a")
IDocHostUIHandler : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(
		/* [in] */ DWORD dwID,
		/* [in] */ POINT *ppt,
		/* [in] */ IUnknown *pcmdtReserved,
		/* [in] */ IDispatch *pdispReserved) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetHostInfo(
		/* [out][in] */ DOCHOSTUIINFO *pInfo) = 0;

	virtual HRESULT STDMETHODCALLTYPE ShowUI(
		/* [in] */ DWORD dwID,
		/* [in] */ IOleInPlaceActiveObject *pActiveObject,
		/* [in] */ IOleCommandTarget *pCommandTarget,
		/* [in] */ IOleInPlaceFrame *pFrame,
		/* [in] */ IOleInPlaceUIWindow *pDoc) = 0;

	virtual HRESULT STDMETHODCALLTYPE HideUI(void) = 0;

	virtual HRESULT STDMETHODCALLTYPE UpdateUI(void) = 0;

	virtual HRESULT STDMETHODCALLTYPE EnableModeless(
		/* [in] */ BOOL fEnable) = 0;

	virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate(
		/* [in] */ BOOL fActivate) = 0;

	virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(
		/* [in] */ BOOL fActivate) = 0;

	virtual HRESULT STDMETHODCALLTYPE ResizeBorder(
		/* [in] */ LPCRECT prcBorder,
		/* [in] */ IOleInPlaceUIWindow *pUIWindow,
		/* [in] */ BOOL fRameWindow) = 0;

	virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(
		/* [in] */ LPMSG lpMsg,
		/* [in] */ const GUID *pguidCmdGroup,
		/* [in] */ DWORD nCmdID) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath(
		/* [annotation][out] */
		LPOLESTR *pchKey,
		/* [in] */ DWORD dw) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetDropTarget(
		/* [in] */ IDropTarget *pDropTarget,
		/* [out] */ IDropTarget **ppDropTarget) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetExternal(
		/* [out] */ IDispatch **ppDispatch) = 0;

	virtual HRESULT STDMETHODCALLTYPE TranslateUrl(
		/* [in] */ DWORD dwTranslate,
		/* [annotation][in] */
		OLECHAR *pchURLIn,
		/* [annotation][out] */
		OLECHAR **ppchURLOut) = 0;

	virtual HRESULT STDMETHODCALLTYPE FilterDataObject(
		/* [in] */ IDataObject *pDO,
		/* [out] */ IDataObject **ppDORet) = 0;

};

typedef enum tagDOCHOSTUIFLAG
{
	DOCHOSTUIFLAG_DIALOG = 0x1,
	DOCHOSTUIFLAG_DISABLE_HELP_MENU = 0x2,
	DOCHOSTUIFLAG_NO3DBORDER = 0x4,
	DOCHOSTUIFLAG_SCROLL_NO = 0x8,
	DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE = 0x10,
	DOCHOSTUIFLAG_OPENNEWWIN = 0x20,
	DOCHOSTUIFLAG_DISABLE_OFFSCREEN = 0x40,
	DOCHOSTUIFLAG_FLAT_SCROLLBAR = 0x80,
	DOCHOSTUIFLAG_DIV_BLOCKDEFAULT = 0x100,
	DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY = 0x200,
	DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY = 0x400,
	DOCHOSTUIFLAG_CODEPAGELINKEDFONTS = 0x800,
	DOCHOSTUIFLAG_URL_ENCODING_DISABLE_UTF8 = 0x1000,
	DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8 = 0x2000,
	DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE = 0x4000,
	DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION = 0x10000,
	DOCHOSTUIFLAG_IME_ENABLE_RECONVERSION = 0x20000,
	DOCHOSTUIFLAG_THEME = 0x40000,
	DOCHOSTUIFLAG_NOTHEME = 0x80000,
	DOCHOSTUIFLAG_NOPICS = 0x100000,
	DOCHOSTUIFLAG_NO3DOUTERBORDER = 0x200000,
	DOCHOSTUIFLAG_DISABLE_EDIT_NS_FIXUP = 0x400000,
	DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK = 0x800000,
	DOCHOSTUIFLAG_DISABLE_UNTRUSTEDPROTOCOL = 0x1000000,
	DOCHOSTUIFLAG_HOST_NAVIGATES = 0x2000000,
	DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION = 0x4000000,
	DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL = 0x8000000,
	DOCHOSTUIFLAG_USE_WINDOWED_SELECTCONTROL = 0x10000000,
	DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE = 0x20000000,
	DOCHOSTUIFLAG_DPI_AWARE = 0x40000000
} DOCHOSTUIFLAG;

#endif 	/* __IDocHostUIHandler_INTERFACE_DEFINED__ */

#ifndef __IHTMLAnchorElement_INTERFACE_DEFINED__
#define __IHTMLAnchorElement_INTERFACE_DEFINED__

/* interface IHTMLAnchorElement */
/* [object][uuid][dual][oleautomation] */ 


EXTERN_C const IID IID_IHTMLAnchorElement;

MIDL_INTERFACE("3050f1da-98b5-11cf-bb82-00aa00bdce0b")
IHTMLAnchorElement : public IDispatch
{
public:
	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_href( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_href( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_target( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_target( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_rel( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_rel( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_rev( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_rev( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_urn( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_urn( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_Methods( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_Methods( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_name( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_name( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_host( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_host( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_hostname( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_hostname( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_pathname( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_pathname( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_port( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_port( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_protocol( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_protocol( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_search( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_search( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_hash( 
		/* [in] */  BSTR v) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_hash( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onblur( 
		/* [in] */ VARIANT v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onblur( 
		/* [out][retval] */  VARIANT *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onfocus( 
		/* [in] */ VARIANT v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onfocus( 
		/* [out][retval] */  VARIANT *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_accessKey( 
		/* [in] */  BSTR v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_accessKey( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_protocolLong( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_mimeType( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_nameProp( 
		/* [out][retval] */  BSTR *p) = 0;

	virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_tabIndex( 
		/* [in] */ short v) = 0;

	virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_tabIndex( 
		/* [out][retval] */  short *p) = 0;

	virtual /* [id] */ HRESULT STDMETHODCALLTYPE focus( void) = 0;

	virtual /* [id] */ HRESULT STDMETHODCALLTYPE blur( void) = 0;

};

#endif 	/* __IHTMLAnchorElement_INTERFACE_DEFINED__ */

#ifndef __IInternetSecurityManager_INTERFACE_DEFINED__
#define __IInternetSecurityManager_INTERFACE_DEFINED__

typedef enum tagURLZONE {
	URLZONE_INVALID = -1,
	URLZONE_PREDEFINED_MIN = 0,
	URLZONE_LOCAL_MACHINE = 0,
	URLZONE_INTRANET,
	URLZONE_TRUSTED,
	URLZONE_INTERNET,
	URLZONE_UNTRUSTED,
	URLZONE_PREDEFINED_MAX = 999,
	URLZONE_USER_MIN = 1000,
	URLZONE_USER_MAX = 10000
} URLZONE;

#define URLACTION_ACTIVEX_MIN                                  0x00001200
#define URLACTION_ACTIVEX_MAX                                  0x000013ff
#define URLACTION_SCRIPT_MIN                                   0x00001400
#define URLACTION_SCRIPT_MAX                                   0x000015ff
#define URLACTION_HTML_MIN                                     0x00001600
#define URLACTION_HTML_JAVA_RUN                                0x00001605 // derive from Java custom policy
#define URLACTION_HTML_MAX                                     0x000017ff
#define URLACTION_JAVA_MIN                                     0x00001C00
#define URLACTION_JAVA_MAX                                     0x00001Cff

#define INET_E_USE_DEFAULT_PROTOCOLHANDLER 0x800C0011L
#define INET_E_USE_DEFAULT_SETTING         0x800C0012L
#define INET_E_DEFAULT_ACTION              INET_E_USE_DEFAULT_PROTOCOLHANDLER
#define INET_E_QUERYOPTION_UNKNOWN         0x800C0013L
#define INET_E_REDIRECTING                 0x800C0014L

// Permissions
#define URLPOLICY_ALLOW                0x00
#define URLPOLICY_QUERY                0x01
#define URLPOLICY_DISALLOW             0x03

static const CLSID IID_IInternetSecurityManager=
{ 0x79eac9ee, 0xbaf9, 0x11ce,
{ 0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b } };

#define SID_SInternetSecurityManager         IID_IInternetSecurityManager

#ifdef __cplusplus
extern "C" {
#endif
#define INTERFACE IInternetSecurityMgrSite
	DECLARE_INTERFACE_(IInternetSecurityMgrSite,IUnknown)
	{
		STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
		STDMETHOD_(ULONG,AddRef)(THIS) PURE;
		STDMETHOD_(ULONG,Release)(THIS) PURE;
		STDMETHOD(GetWindow)(THIS_ HWND*) PURE;
		STDMETHOD(EnableModeless)(THIS_ BOOL) PURE;
	};
#undef INTERFACE

#define INTERFACE IInternetSecurityManager
	DECLARE_INTERFACE_(IInternetSecurityManager,IUnknown)
	{
		STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
		STDMETHOD_(ULONG,AddRef)(THIS) PURE;
		STDMETHOD_(ULONG,Release)(THIS) PURE;

		STDMETHOD(SetSecuritySite)(THIS_ IInternetSecurityMgrSite*) PURE;
		STDMETHOD(GetSecuritySite)(THIS_ IInternetSecurityMgrSite**) PURE;
		STDMETHOD(MapUrlToZone)(THIS_ LPCWSTR,DWORD*,DWORD) PURE;
		STDMETHOD(GetSecurityId)(THIS_ LPCWSTR,BYTE*,DWORD*,DWORD_PTR) PURE;
		STDMETHOD(ProcessUrlAction)(THIS_ LPCWSTR,DWORD,BYTE*,DWORD,BYTE*,DWORD,DWORD,DWORD) PURE;
		STDMETHOD(QueryCustomPolicy)(THIS_ LPCWSTR,REFGUID,BYTE**,DWORD*,BYTE*,DWORD,DWORD) PURE;
		STDMETHOD(SetZoneMapping)(THIS_ DWORD,LPCWSTR,DWORD) PURE;
		STDMETHOD(GetZoneMappings)(THIS_ DWORD,IEnumString**,DWORD) PURE;
	};
#undef INTERFACE


#ifdef __cplusplus
}
#endif

#endif

class IEViewSink :public  DWebBrowserEvents2 {
private:
	int		m_cRef;
	IEView *ieWindow;
public:
	IEViewSink(IEView *);
	virtual ~IEViewSink();
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

class IEView :public IDispatch, public IOleClientSite, public IOleInPlaceSite, public IDocHostUIHandler, public IInternetSecurityManager, public IServiceProvider, public MZeroedObject
{
private:
	static IEView *list;
	static mir_cs mutex;
	HWND parent;
	HWND hwnd;
	IEView *prev, *next;
	int m_cRef;
	RECT rcClient;
	BOOL m_bInPlaceActive;
	DWORD m_dwCookie;
	CComPtr<IConnectionPoint> m_pConnectionPoint;
	CComPtr<IWebBrowser2> pWebBrowser;
	IEViewSink *sink;
	HTMLBuilder *builder;

	WNDPROC mainWndProc, docWndProc, serverWndProc;
	bool getFocus;
	bool clearRequired;
	BSTR selectedText;
	bool isContactSet;
	MCONTACT hContact;

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

	// IDocHostUIHandler
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO *pInfo);
	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
		IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc);
	STDMETHOD(HideUI)(void);
	STDMETHOD(UpdateUI)(void);
	STDMETHOD(EnableModeless)(BOOL fEnable);
	STDMETHOD(OnDocWindowActivate)(BOOL fEnable);
	STDMETHOD(OnFrameWindowActivate)(BOOL fEnable);
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);
	STDMETHOD(GetOptionKeyPath)(LPOLESTR *pchKey, DWORD dw);
	STDMETHOD(GetDropTarget)(IDropTarget *pDropTarget, IDropTarget **ppDropTarget);
	STDMETHOD(GetExternal)(IDispatch **ppDispatch);
	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
	STDMETHOD(FilterDataObject)(IDataObject *pDO, IDataObject **ppDORet);
	// IServiceProvider
	STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void** ppvObject);
	// IInternetSecurityManager
	STDMETHOD(SetSecuritySite)(IInternetSecurityMgrSite *pSite);
	STDMETHOD(GetSecuritySite)(IInternetSecurityMgrSite **ppSite);
	STDMETHOD(MapUrlToZone)(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags);
	STDMETHOD(GetSecurityId)(LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved);
	STDMETHOD(ProcessUrlAction)(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved);
	STDMETHOD(QueryCustomPolicy)(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy, DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwReserved);
	STDMETHOD(SetZoneMapping)(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags);
	STDMETHOD(GetZoneMappings)(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags);

	IHTMLDocument2 *getDocument();
	WCHAR*  getHrefFromAnchor(CComPtr<IHTMLElement> element);
	WCHAR*  getSelection();
	void    setBorder();
protected:
	void    navigate(const char *);
	void    navigate(const wchar_t *);
public:
	IEView(HWND parent, HTMLBuilder* builder, int x, int y, int cx, int cy);
	//	IEView::IEView(HWND parent, SmileyWindow* smileyWindow, int x, int y, int cx, int cy);
	virtual ~IEView();

	void    waitWhileBusy();
	HWND    getHWND();
	void    translateAccelerator(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool    mouseClick(POINT pt);
	bool    mouseActivate();
	bool    setFocus(HWND prevFocus);
	void    setWindowPos(int x, int y, int cx, int cy);
	HTMLBuilder* getBuilder();

	void    write(const wchar_t *text);
	void    write(const char *text);
	void    writef(const char *fmt, ...);
	void    documentClose();
	void    rebuildLog();
	void    scrollToBottom();
	void    scrollToBottomSoft();
	void    scrollToTop();

	void    setMainWndProc(WNDPROC);
	WNDPROC getMainWndProc();
	void    setDocWndProc(WNDPROC);
	WNDPROC getDocWndProc();
	void    setServerWndProc(WNDPROC);
	WNDPROC getServerWndProc();

	void    appendEventOld(IEVIEWEVENT * event);
	void    appendEvent(IEVIEWEVENT * event);
	void    clear(IEVIEWEVENT * event);
	void*   getSelection(IEVIEWEVENT * event);
	void    navigate(IEVIEWNAVIGATE * nav);
	void    saveDocument();

	void    setContact(MCONTACT hContact);

	static IEView*	get(HWND);
	static void		release();
	static void		setOptions();

};
#endif
