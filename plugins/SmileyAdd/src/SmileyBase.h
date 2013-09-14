/*
Miranda SmileyAdd Plugin
Copyright (C) 2008 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define OLEIVERB_SETOWNER		(-24)

EXTERN_C const IID IID_ITooltipData;

class ITooltipData : public IUnknown 
{
public:
	STDMETHOD(SetTooltip) (BSTR bstrHint)   PURE;
	STDMETHOD(GetTooltip) (BSTR * bstrHint) PURE; 
};


EXTERN_C const IID IID_ISmileyAddSmiley;

class ISmileyBase :
	public IOleObject, public IViewObject2, public ITooltipData
{
protected:
	IOleAdviseHolder* m_spAdviseHolder;
	IAdviseSink* m_spAdviseSink;
	IOleClientSite* m_spClientSite;
	DWORD m_advf;
	LONG m_lRefCount;

	SIZEL m_sizeExtent;
	SIZEL m_sizeExtentHiM;
	RECT  m_orect;

	TCHAR* m_smltxt;
	HWND m_hwnd;

	bool m_visible;
	bool m_dirAniAllow;

public:
	ISmileyBase(void);
	virtual ~ISmileyBase(void);

	void OnClose(void);
	void SendOnViewChange(void);

	bool QueryHitPointSpecial(int x, int y, HWND hwnd, TCHAR** smltxt);
	void SetHint(TCHAR* smltxt);

	virtual void SetPosition(HWND hwnd, LPCRECT lpRect);

    //
	// IUnknown members
	//
	STDMETHOD_(ULONG, AddRef)(void); 
	STDMETHOD_(ULONG, Release)(void);
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

	//
	// IOleObject members
	//
	STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
	STDMETHOD(GetClientSite)(IOleClientSite **ppClientSite);
	STDMETHOD(SetHostNames)(LPCOLESTR /* szContainerApp */, LPCOLESTR /* szContainerObj */);
	STDMETHOD(Close)(DWORD /* dwSaveOption */);
	STDMETHOD(SetMoniker)(DWORD /* dwWhichMoniker */, IMoniker* /* pmk */);
	STDMETHOD(GetMoniker)(DWORD /* dwAssign */, DWORD /* dwWhichMoniker */, IMoniker** /* ppmk */);
	STDMETHOD(InitFromData)(IDataObject* /* pDataObject */, BOOL /* fCreation */, DWORD /* dwReserved */);
	STDMETHOD(GetClipboardData)(DWORD /* dwReserved */, IDataObject** /* ppDataObject */);
	STDMETHOD(DoVerb)(LONG /*iVerb*/, LPMSG /* pMsg */, IOleClientSite* /* pActiveSite */, LONG /* lindex */,
									 HWND /*hwndParent*/, LPCRECT /*lprcPosRect*/);
	STDMETHOD(EnumVerbs)(IEnumOLEVERB** /*ppEnumOleVerb*/);
	STDMETHOD(Update)(void);
	STDMETHOD(IsUpToDate)(void);
	STDMETHOD(GetUserClassID)(CLSID *pClsid);
	STDMETHOD(GetUserType)(DWORD /*dwFormOfType*/, LPOLESTR* /*pszUserType*/);
	STDMETHOD(SetExtent)(DWORD /*dwDrawAspect*/, SIZEL* /*psizel*/);
	STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
	STDMETHOD(Advise)(IAdviseSink *pAdvSink, DWORD *pdwConnection);
	STDMETHOD(Unadvise)(DWORD dwConnection);
	STDMETHOD(EnumAdvise)(IEnumSTATDATA **ppEnumAdvise);
	STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
	STDMETHOD(SetColorScheme)(LOGPALETTE* /* pLogpal */);

	//
	// IViewObject members
	//
	STDMETHOD(SetAdvise)(DWORD aspect, DWORD advf, IAdviseSink* pAdvSink);
	STDMETHOD(GetAdvise)(DWORD* /*pAspects*/, DWORD* /*pAdvf*/, IAdviseSink** ppAdvSink);
	STDMETHOD(Freeze)(DWORD, long, void*, DWORD*);
	STDMETHOD(Unfreeze)(DWORD);
	STDMETHOD(GetColorSet)(DWORD, long, void*, DVTARGETDEVICE*, HDC, 
		LOGPALETTE**);

	//
	// IViewObject2 members
	//
	STDMETHOD(GetExtent)(DWORD aspect, long, DVTARGETDEVICE*, SIZEL* pSize);

	//
	// ITooltipData members
	//
	STDMETHOD(SetTooltip)(BSTR bstrHint);
	STDMETHOD(GetTooltip)(BSTR * bstrHint); 
};

int CheckForTip(int x, int y, HWND hwnd, TCHAR** smltxt);
void CloseSmileys(void);

