/*
Miranda SmileyAdd Plugin
Copyright (C) 2004 Rein-Peter de Boer (peacow)

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

//code taken partly from public example on the internet, source unknown.

class CImageDataObject : IDataObject
{
public:
  // returns true on success, false on failure
	static bool InsertBitmap(IRichEditOle* pRichEditOle, HBITMAP hBitmap);
  // returns true on success, false on failure
    //static bool InsertIcon(IRichEditOle* pRichEditOle, HICON hIcon,
    //COLORREF backgroundColor, int sizeX = 0, int sizeY = 0);

private:
	ULONG	m_ulRefCnt;
	BOOL	m_bRelease;

	STGMEDIUM m_stgmed;
	FORMATETC m_format;

public:
	CImageDataObject() : m_ulRefCnt(0)
	{
		m_bRelease = FALSE;
	}

	~CImageDataObject()
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
	STDMETHOD(GetData)(FORMATETC*, STGMEDIUM *pmedium) {
		HANDLE hDst;
		hDst = ::OleDuplicateData(m_stgmed.hBitmap, CF_BITMAP, 0);
		if (hDst == NULL)
			return E_HANDLE;

		pmedium->tymed = TYMED_GDI;
		pmedium->hBitmap = (HBITMAP)hDst;
		pmedium->pUnkForRelease = NULL;

		return S_OK;
	}

	STDMETHOD(GetDataHere)(FORMATETC*, STGMEDIUM*) {
		return E_NOTIMPL;
	}

	STDMETHOD(QueryGetData)(FORMATETC*) {
		return E_NOTIMPL;
	}

	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*, FORMATETC*) 	{
		return E_NOTIMPL;
	}

	STDMETHOD(SetData)(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL) {
		m_format = *pformatetc;
		m_stgmed = *pmedium;
		return S_OK;
	}

	STDMETHOD(EnumFormatEtc)(DWORD, IEnumFORMATETC**) {
		return E_NOTIMPL;
	}

	STDMETHOD(DAdvise)(FORMATETC*, DWORD, IAdviseSink*, DWORD*) {
		return E_NOTIMPL;
	}

	STDMETHOD(DUnadvise)(DWORD) {
		return E_NOTIMPL;
	}

	STDMETHOD(EnumDAdvise)(IEnumSTATDATA**) {
		return E_NOTIMPL;
	}

  // Other
	void SetBitmap(HBITMAP hBitmap);
	IOleObject *GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage);
};

