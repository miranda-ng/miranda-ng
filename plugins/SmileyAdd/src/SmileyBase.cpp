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

#include "stdafx.h"

static void HiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
{
	HDC hDCScreen = GetDC(NULL);
	const int nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	const int nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	ReleaseDC(NULL, hDCScreen);

	lpSizeInPix->cx = (lpSizeInHiMetric->cx * nPixelsPerInchX + (2540 / 2)) / 2540;
	lpSizeInPix->cy = (lpSizeInHiMetric->cy * nPixelsPerInchY + (2540 / 2)) / 2540;
}

static LIST<ISmileyBase> regSmileys(10, PtrKeySortT);

// {105C56DF-6455-4705-A501-51F1CCFCF688}
const GUID IID_ISmileyAddSmiley =
{ 0x105c56df, 0x6455, 0x4705, { 0xa5, 0x1, 0x51, 0xf1, 0xcc, 0xfc, 0xf6, 0x88 } };

// {58B32D03-1BD2-4840-992E-9AE799FD4ADE}
const GUID IID_ITooltipData =
{ 0x58b32d03, 0x1bd2, 0x4840, { 0x99, 0x2e, 0x9a, 0xe7, 0x99, 0xfd, 0x4a, 0xde } };

ISmileyBase::ISmileyBase(void)
{
	m_spAdviseSink = NULL;
	m_spClientSite = NULL;
	m_spAdviseHolder = NULL;
	m_lRefCount = 1;
	m_advf = 0;
	m_smltxt = NULL;
	m_hwnd = NULL;
	m_visible = false;
	m_dirAniAllow = false;

	memset(&m_sizeExtent, 0, sizeof(m_sizeExtent));
	memset(&m_sizeExtentHiM, 0, sizeof(m_sizeExtentHiM));
	memset(&m_orect, 0, sizeof(m_orect));

	regSmileys.insert(this);
}

ISmileyBase::~ISmileyBase(void)
{
	free(m_smltxt);

	Close(OLECLOSE_NOSAVE);

	if (m_spClientSite) {
		m_spClientSite->Release();
		m_spClientSite = NULL;
	}
	
	if (m_spAdviseHolder) {
		m_spAdviseHolder->Release();
		m_spAdviseHolder = NULL;
	}
}

void ISmileyBase::OnClose(void)
{
	if (m_spAdviseHolder)
		m_spAdviseHolder->SendOnClose();
}

void ISmileyBase::SendOnViewChange(void)
{
	if (m_spAdviseSink) m_spAdviseSink->OnViewChange(DVASPECT_CONTENT, -1);
	if (m_advf & ADVF_ONLYONCE) {
		m_spAdviseSink->Release();
		m_spAdviseSink = NULL;
		m_advf = 0;
	}
}

bool ISmileyBase::QueryHitPointSpecial(int x, int y, HWND hwnd, TCHAR** smltxt)
{
	bool result = m_visible && m_hwnd == hwnd;
	if (result) {
		result = x >= m_orect.left && x <= m_orect.right &&
			y >= m_orect.top && y <= m_orect.bottom;
	}
	if (result) *smltxt = m_smltxt;
	return result;
}

void ISmileyBase::SetHint(TCHAR* smltxt)
{
	m_smltxt = _tcsdup(smltxt);
}


void ISmileyBase::SetPosition(HWND hwnd, LPCRECT lpRect)
{
	m_hwnd = hwnd;
	if (lpRect == NULL || (lpRect->top == -1 && lpRect->bottom == -1)) {
		m_visible = false;
	}
	else {
		m_visible = true;
		m_dirAniAllow = true;
		m_orect.left = lpRect->left;
		m_orect.right = lpRect->left + m_sizeExtent.cx;
		if (lpRect->top == -1) {
			m_orect.top = lpRect->bottom - m_sizeExtent.cy;
			m_orect.bottom = lpRect->bottom;
		}
		else if (lpRect->bottom == -1) {
			m_orect.top = lpRect->top;
			m_orect.bottom = lpRect->top + m_sizeExtent.cy;
		}
		else {
			m_orect.top = lpRect->bottom - m_sizeExtent.cy;
			m_orect.bottom = lpRect->bottom;
		}
	}
}


//
// IUnknown members
//
ULONG ISmileyBase::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG ISmileyBase::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);
	if (count == 0)
		delete this;
	return count;
}


HRESULT ISmileyBase::QueryInterface(REFIID iid, void ** ppvObject)
{
	// check to see what interface has been requested
	if (ppvObject == NULL) return E_POINTER;
	if (iid == IID_ISmileyAddSmiley)
		*ppvObject = this;
	else if (iid == IID_ITooltipData)
		*ppvObject = static_cast<ITooltipData*>(this);
	else if (iid == IID_IViewObject)
		*ppvObject = static_cast<IViewObject2*>(this);
	else if (iid == IID_IOleObject)
		*ppvObject = static_cast<IOleObject*>(this);
	else if (iid == IID_IUnknown)
		*ppvObject = this;
	else if (iid == IID_IViewObject2)
		*ppvObject = static_cast<IViewObject2*>(this);
	else {
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

//
// IOleObject members
//
HRESULT ISmileyBase::SetClientSite(IOleClientSite *pClientSite)
{
	if (m_spClientSite != NULL) m_spClientSite->Release();
	m_spClientSite = pClientSite;
	if (m_spClientSite != NULL) m_spClientSite->AddRef();
	return S_OK;
}

HRESULT ISmileyBase::GetClientSite(IOleClientSite **ppClientSite)
{
	if (ppClientSite == NULL) return E_POINTER;
	*ppClientSite = m_spClientSite;
	if (m_spClientSite != NULL) m_spClientSite->AddRef();
	return S_OK;
}

HRESULT ISmileyBase::SetHostNames(LPCOLESTR /* szContainerApp */, LPCOLESTR /* szContainerObj */)
{
	return S_OK;
}

HRESULT ISmileyBase::Close(DWORD /* dwSaveOption */)
{
	regSmileys.remove(this);

	if (m_spAdviseSink) {
		m_spAdviseSink->Release();
		m_spAdviseSink = NULL;
	}

	return S_OK;
}

HRESULT ISmileyBase::SetMoniker(DWORD /* dwWhichMoniker */, IMoniker* /* pmk */)
{
	return E_NOTIMPL;
}

HRESULT ISmileyBase::GetMoniker(DWORD /* dwAssign */, DWORD /* dwWhichMoniker */, IMoniker** /* ppmk */)
{
	return E_NOTIMPL;
}

HRESULT ISmileyBase::InitFromData(IDataObject* /* pDataObject */, BOOL /* fCreation */, DWORD /* dwReserved */)
{
	return E_NOTIMPL;
}

HRESULT ISmileyBase::GetClipboardData(DWORD /* dwReserved */, IDataObject** /* ppDataObject */)
{
	return E_NOTIMPL;
}

HRESULT ISmileyBase::DoVerb(LONG /* iVerb */, LPMSG /* pMsg */, IOleClientSite* /* pActiveSite */, LONG /* lindex */,
	HWND /* hwndParent */, LPCRECT /* lprcPosRect */)
{
	return E_NOTIMPL;
}

HRESULT ISmileyBase::EnumVerbs(IEnumOLEVERB** /*ppEnumOleVerb*/) { return E_NOTIMPL; }
HRESULT ISmileyBase::Update(void) { return S_OK; }
HRESULT ISmileyBase::IsUpToDate(void) { return S_OK; }

HRESULT ISmileyBase::GetUserClassID(CLSID *pClsid)
{
	if (!pClsid) return E_POINTER;
	*pClsid = CLSID_NULL;
	return S_OK;
}

HRESULT ISmileyBase::GetUserType(DWORD /*dwFormOfType*/, LPOLESTR* /*pszUserType*/)
{
	return E_NOTIMPL;
}

HRESULT ISmileyBase::SetExtent(DWORD dwDrawAspect, SIZEL* psizel)
{
	if (dwDrawAspect != DVASPECT_CONTENT) return E_FAIL;
	if (psizel == NULL) return E_POINTER;

	HiMetricToPixel(psizel, &m_sizeExtent);
	m_sizeExtentHiM = *psizel;
	return S_OK;
}

HRESULT ISmileyBase::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
	if (dwDrawAspect != DVASPECT_CONTENT) return E_FAIL;
	if (psizel == NULL) return E_POINTER;

	*psizel = m_sizeExtentHiM;
	return S_OK;
}

HRESULT ISmileyBase::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	HRESULT hr = S_OK;
	if (m_spAdviseHolder == NULL)
		hr = CreateOleAdviseHolder(&m_spAdviseHolder);
	if (SUCCEEDED(hr))
		hr = m_spAdviseHolder->Advise(pAdvSink, pdwConnection);
	else
		m_spAdviseHolder = NULL;
	return hr;
}

HRESULT ISmileyBase::Unadvise(DWORD dwConnection)
{
	return m_spAdviseHolder ? m_spAdviseHolder->Unadvise(dwConnection) : E_FAIL;
}

HRESULT ISmileyBase::EnumAdvise(IEnumSTATDATA **ppEnumAdvise)
{
	if (ppEnumAdvise == NULL) return E_POINTER;
	return m_spAdviseHolder ? m_spAdviseHolder->EnumAdvise(ppEnumAdvise) : E_FAIL;
}

HRESULT ISmileyBase::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
	if (pdwStatus == NULL) return E_POINTER;
	if (dwAspect == DVASPECT_CONTENT) {
		*pdwStatus = OLEMISC_STATIC | OLEMISC_INVISIBLEATRUNTIME |
			OLEMISC_CANTLINKINSIDE | OLEMISC_NOUIACTIVATE;
		return S_OK;
	}
	else {
		*pdwStatus = 0;
		return E_FAIL;
	}
}

HRESULT ISmileyBase::SetColorScheme(LOGPALETTE* /* pLogpal */)
{
	return E_NOTIMPL;
}

//
// IViewObject members
//
HRESULT ISmileyBase::SetAdvise(DWORD aspect, DWORD advf, IAdviseSink* pAdvSink)
{
	if (aspect != DVASPECT_CONTENT) return DV_E_DVASPECT;
	m_advf = advf;
	if (m_spAdviseSink) m_spAdviseSink->Release();
	m_spAdviseSink = pAdvSink;
	if (advf & ADVF_PRIMEFIRST) SendOnViewChange();
	return S_OK;
}
HRESULT ISmileyBase::GetAdvise(DWORD* /*pAspects*/, DWORD* /*pAdvf*/, IAdviseSink** ppAdvSink)
{
	if (!ppAdvSink) return E_POINTER;
	*ppAdvSink = m_spAdviseSink;
	if (m_spAdviseSink) m_spAdviseSink->AddRef();
	return S_OK;
}

HRESULT ISmileyBase::Freeze(DWORD, long, void*, DWORD*) { return E_NOTIMPL; }
HRESULT ISmileyBase::Unfreeze(DWORD) { return E_NOTIMPL; }
HRESULT ISmileyBase::GetColorSet(DWORD, long, void*, DVTARGETDEVICE*, HDC, LOGPALETTE**) { return E_NOTIMPL; }

//
// IViewObject2 members
//
HRESULT ISmileyBase::GetExtent(DWORD aspect, long, DVTARGETDEVICE*, SIZEL* pSize)
{
	if (pSize == NULL) return E_POINTER;
	if (aspect != DVASPECT_CONTENT) return DV_E_DVASPECT;
	*pSize = m_sizeExtent;
	return S_OK;
}


//
// ITooltipData members
//
HRESULT ISmileyBase::SetTooltip(BSTR /* bstrHint */)
{
	return S_OK;
}

HRESULT ISmileyBase::GetTooltip(BSTR *bstrHint)
{
	if (bstrHint == NULL) return E_POINTER;
	*bstrHint = SysAllocString(m_smltxt);
	return S_OK;
}


void CloseSmileys(void)
{
	for (int i = regSmileys.getCount() - 1; i >= 0; i--) {
		regSmileys[i]->OnClose();
		regSmileys[i]->Close(OLECLOSE_NOSAVE);
	}
}

int CheckForTip(int x, int y, HWND hwnd, TCHAR** smltxt)
{
	for (int i = 0; i < regSmileys.getCount(); i++)
		if (regSmileys[i]->QueryHitPointSpecial(x, y, hwnd, smltxt))
			return i;

	return -1;
}
