/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

//code taken partly from public example on the internet, source unknown.

#pragma once

class ImageDataObject : IDataObject
{
public:
  // returns true on success, false on failure
	static bool InsertBitmap(IRichEditOle* pRichEditOle, HBITMAP hBitmap);
  // returns true on success, false on failure
    static bool InsertIcon(IRichEditOle* pRichEditOle, HICON hIcon,
		COLORREF backgroundColor, int sizeX = 0, int sizeY = 0);

private:
	ULONG	m_ulRefCnt;
	BOOL	m_bRelease;

	STGMEDIUM m_stgmed;
	FORMATETC m_format;

   // ULONG       m_nIndex;           // current enumerator index

public:
	ImageDataObject() : m_ulRefCnt(0)
	{
		m_bRelease = FALSE;
	}

	~ImageDataObject()
	{
		if (m_bRelease)
			::ReleaseStgMedium(&m_stgmed);
	}

	// IUnknown interface
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		if (iid == IID_IUnknown || iid == IID_IDataObject)
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		else
			return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)(void)
	{
		m_ulRefCnt++;
		return m_ulRefCnt;
	}

	STDMETHOD_(ULONG, Release)(void)
	{
    m_ulRefCnt--;
		if (m_ulRefCnt == 0)
			delete this;
		return m_ulRefCnt;
	}

	// IDataObject Interface
	STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
		HANDLE hDst;
		hDst = ::OleDuplicateData(m_stgmed.hBitmap, CF_BITMAP, 0);
		if (hDst == NULL)
			return E_HANDLE;

		pmedium->tymed = TYMED_GDI;
		pmedium->hBitmap = (HBITMAP)hDst;
		pmedium->pUnkForRelease = NULL;
		return S_OK;
	}

	STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM*  pmedium ) {
		return E_NOTIMPL;
	}

	STDMETHOD(QueryGetData)(FORMATETC*  pformatetc ) {
		return E_NOTIMPL;
	}

	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  pformatectIn ,FORMATETC* pformatetcOut ) 	{
		return E_NOTIMPL;
	}

	STDMETHOD(SetData)(FORMATETC* pformatetc , STGMEDIUM*  pmedium , BOOL  fRelease ) {
		m_format = *pformatetc;
		m_stgmed = *pmedium;

		return S_OK;
	}

	STDMETHOD(EnumFormatEtc)(DWORD  dwDirection , IEnumFORMATETC**  ppenumFormatEtc ) {
		return E_NOTIMPL;
	}

	STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
		DWORD *pdwConnection) {
		return E_NOTIMPL;
	}

	STDMETHOD(DUnadvise)(DWORD dwConnection) {
		return E_NOTIMPL;
	}

	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) {
		return E_NOTIMPL;
	}

  // Other
	void SetBitmap(HBITMAP hBitmap);
	IOleObject *GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage);
};

