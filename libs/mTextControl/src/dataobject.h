#pragma once

class CDataObject : public IDataObject
{
public:
	//
	// IUnknown members
	//
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
	ULONG   __stdcall AddRef(void);
	ULONG   __stdcall Release(void);

	//
	// IDataObject members
	//
	HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
	HRESULT __stdcall GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
	HRESULT __stdcall QueryGetData(FORMATETC* pFormatEtc);
	HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut);
	HRESULT __stdcall SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease);
	HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc);
	HRESULT __stdcall DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
	HRESULT __stdcall DUnadvise(DWORD dwConnection);
	HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

	//
	// Constructor / Destructor
	//
	CDataObject();
	CDataObject(const FORMATETC* fmt, const STGMEDIUM* stgmed, int count);
	~CDataObject();

protected:
	void UpdateData(const FORMATETC* pFormatEtc, const STGMEDIUM* pMedium, int count);

private:

	int LookupFormatEtc(FORMATETC* pFormatEtc);

	//
	// any private members and functions
	//
	LONG	   m_lRefCount;

	FORMATETC* m_pFormatEtc;
	STGMEDIUM* m_pStgMedium;
	LONG	   m_nNumFormats;

};

class CEMFObject : public CDataObject
{
public:
	// IDataObject members
	HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);

	// Constructor / Destructor
	CEMFObject(HENHMETAFILE hEmf);
	~CEMFObject();

private:
	HENHMETAFILE m_hEmf;

};

