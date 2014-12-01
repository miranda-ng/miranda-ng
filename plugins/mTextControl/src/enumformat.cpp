//
//	ENUMFORMAT.CPP
//
//	By J Brown 2004 
//
//	www.catch22.net
//
//	Implementation of the IEnumFORMATETC interface
//
//	For Win2K and above look at the SHCreateStdEnumFmtEtc API call!!
//
//	Apparently the order of formats in an IEnumFORMATETC object must be
//  the same as those that were stored in the clipboard
//
//

//#define STRICT

#include "headers.h"

class CEnumFormatEtc : public IEnumFORMATETC
{
public:
	//
	// IUnknown members
	//
	HRESULT __stdcall  QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall  AddRef (void);
	ULONG	__stdcall  Release (void);

	//
	// IEnumFormatEtc members
	//
	HRESULT __stdcall  Next  (ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
	HRESULT __stdcall  Skip  (ULONG celt); 
	HRESULT __stdcall  Reset (void);
	HRESULT __stdcall  Clone (IEnumFORMATETC ** ppEnumFormatEtc);

	//
	// Construction / Destruction
	//
	CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
	~CEnumFormatEtc();

private:

	LONG		m_lRefCount;		// Reference count for this COM interface
	ULONG		m_nIndex;			// current enumerator index
	ULONG		m_nNumFormats;		// number of FORMATETC members
	FORMATETC * m_pFormatEtc;		// array of FORMATETC objects
};

//
//	"Drop-in" replacement for SHCreateStdEnumFmtEtc. Called by CDataObject::EnumFormatEtc
//
HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc)
{
	if (nNumFormats == 0 || pFormatEtc == 0 || ppEnumFormatEtc == 0)
		return E_INVALIDARG;

	*ppEnumFormatEtc = new CEnumFormatEtc(pFormatEtc, nNumFormats);

	return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

//
//	Helper function to perform a "deep" copy of a FORMATETC
//
static void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
{
	// copy the source FORMATETC into dest
	*dest = *source;

	if (source->ptd) {
		// allocate memory for the DVTARGETDEVICE if necessary
		dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

		// copy the contents of the source DVTARGETDEVICE into dest->ptd
		*(dest->ptd) = *(source->ptd);
	}
}

//
//	Constructor 
//
CEnumFormatEtc::CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats)
{
	m_lRefCount = 1;
	m_nIndex = 0;
	m_nNumFormats = nNumFormats;
	m_pFormatEtc = new FORMATETC[nNumFormats];

	// copy the FORMATETC structures
	for (int i = 0; i < nNumFormats; i++) {
		DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
	}
}

//
//	Destructor
//
CEnumFormatEtc::~CEnumFormatEtc()
{
	if (m_pFormatEtc) {
		for (ULONG i = 0; i < m_nNumFormats; i++) {
			if (m_pFormatEtc[i].ptd)
				CoTaskMemFree(m_pFormatEtc[i].ptd);
		}

		delete[] m_pFormatEtc;
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CEnumFormatEtc::AddRef(void)
{
	// increment object reference count
	return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CEnumFormatEtc::Release(void)
{
	// decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);

	if (count == 0) {
		delete this;
		return 0;
	}
	else {
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject)
{
	// check to see what interface has been requested
	if (iid == IID_IEnumFORMATETC || iid == IID_IUnknown) {
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else {
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

//
//	IEnumFORMATETC::Next
//
//	If the returned FORMATETC structure contains a non-null "ptd" member, then
//  the caller must free this using CoTaskMemFree (stated in the COM documentation)
//
HRESULT __stdcall CEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtc, ULONG * pceltFetched)
{
	ULONG copied = 0;

	// validate arguments
	if (celt == 0 || pFormatEtc == 0)
		return E_INVALIDARG;

	// copy FORMATETC structures into caller's buffer
	while (m_nIndex < m_nNumFormats && copied < celt) {
		DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
		copied++;
		m_nIndex++;
	}

	// store result
	if (pceltFetched != 0)
		*pceltFetched = copied;

	// did we copy all that was requested?
	return (copied == celt) ? S_OK : S_FALSE;
}

//
//	IEnumFORMATETC::Skip
//
HRESULT __stdcall CEnumFormatEtc::Skip(ULONG celt)
{
	m_nIndex += celt;
	return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
}

//
//	IEnumFORMATETC::Reset
//
HRESULT __stdcall CEnumFormatEtc::Reset(void)
{
	m_nIndex = 0;
	return S_OK;
}

//
//	IEnumFORMATETC::Clone
//
HRESULT __stdcall CEnumFormatEtc::Clone(IEnumFORMATETC ** ppEnumFormatEtc)
{
	HRESULT hResult;

	// make a duplicate enumerator
	hResult = CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);

	if (hResult == S_OK) {
		// manually set the index state
		((CEnumFormatEtc *)*ppEnumFormatEtc)->m_nIndex = m_nIndex;
	}

	return hResult;
}
