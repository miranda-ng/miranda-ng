#include "jabber.h"
#include "MString.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CMBaseString 

CNilMStringData CMBaseString::m_nil;

CMStringData* CMBaseString::Allocate( int nChars, int nCharSize )
{
	CMStringData* pData;
	nChars++; // nil char
	size_t nDataBytes = nCharSize * nChars;
	size_t nTotalSize = nDataBytes + sizeof(CMStringData);

	pData = static_cast<CMStringData*>(malloc(nTotalSize));
	if (pData == NULL)
		return NULL;

	pData->nRefs = 1;
	pData->nAllocLength = nChars - 1;
	pData->nDataLength = 0;
	return pData;
}

void CMBaseString::Free(CMStringData* pData)
{
	free(pData);
}

CMStringData* CMBaseString::Realloc(CMStringData* pData, int nChars, int nCharSize)
{
	CMStringData* pNewData;
	nChars++; // nil char
	ULONG nDataBytes = nCharSize * nChars;
	ULONG nTotalSize = nDataBytes + sizeof(CMStringData);

	pNewData = static_cast<CMStringData*>(realloc(pData, nTotalSize));
	if (pNewData == NULL)
		return NULL;

	pNewData->nAllocLength = nChars - 1;
	return pNewData;
}

CMStringData* CMBaseString::GetNilString()
{
	m_nil.AddRef();
	return &m_nil;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CMStringData

void* CMStringData::data()
{
	return (this + 1);
}

void CMStringData::AddRef()
{
	InterlockedIncrement(&nRefs);
}

bool CMStringData::IsLocked() const
{
	return nRefs < 0;
}

bool CMStringData::IsShared() const
{
	return (nRefs > 1);
}

void CMStringData::Lock()
{
	nRefs--;  // Locked buffers can't be shared, so no interlocked operation necessary
	if( nRefs == 0 )
		nRefs = -1;
}

void CMStringData::Release()
{
	if (InterlockedDecrement(&nRefs) <= 0)
		CMBaseString::Free(this);
}

void CMStringData::Unlock()
{
	if (IsLocked())
	{
		nRefs++;  // Locked buffers can't be shared, so no interlocked operation necessary
		if (nRefs == 0)
			nRefs = 1;
	}
}

CNilMStringData::CNilMStringData()
{
	nRefs = 2;  // Never gets freed
	nDataLength = 0;
	nAllocLength = 0;
	achNil[0] = 0;
	achNil[1] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ChTraitsCRT<wchar_t>

#if _MSC_VER < 1400
static HINSTANCE hCrt = NULL;

typedef int ( __cdecl *_vscprintf_func )( LPCSTR pszFormat, va_list args );
static _vscprintf_func _vscprintf_ptr = NULL;

typedef int ( __cdecl *_vscwprintf_func )( LPCWSTR pszFormat, va_list args );
static _vscwprintf_func _vscwprintf_ptr = NULL;

typedef int ( __cdecl *_vsnprintf_func )( char*, size_t, const char*, va_list );
static _vsnprintf_func _vsnprintf_ptr = NULL;

typedef int ( __cdecl *_vsnwprintf_func )( wchar_t *, size_t, const wchar_t *, va_list );
static _vsnwprintf_func _vsnwprintf_ptr = NULL;

typedef int ( __cdecl *vswprintf_func )( wchar_t *, size_t, const wchar_t *, va_list );
static vswprintf_func vswprintf_ptr = NULL;

typedef int ( __cdecl *vsprintf_func )( char*, size_t, const char*, va_list );
static vsprintf_func vsprintf_ptr = NULL;

static void checkCrt( void )
{
	if ( hCrt == NULL ) {
		hCrt = GetModuleHandleA( "msvcrt.dll" );
		_vscprintf_ptr = (_vscprintf_func)GetProcAddress( hCrt, "_vscprintf" );
		_vscwprintf_ptr = (_vscwprintf_func)GetProcAddress( hCrt, "_vscwprintf" );
		_vsnprintf_ptr = (_vsnprintf_func)GetProcAddress( hCrt, "_vsnprintf" );
		_vsnwprintf_ptr = (_vsnwprintf_func)GetProcAddress( hCrt, "_vsnwprintf" );
		vswprintf_ptr = (vswprintf_func)GetProcAddress( hCrt, "vswprintf" );
		vsprintf_ptr = (vsprintf_func)GetProcAddress( hCrt, "vsprintf" );
}	}
#endif

int __stdcall ChTraitsCRT<wchar_t>::GetFormattedLength( LPCWSTR pszFormat, va_list args )
{
	#if _MSC_VER < 1400
		checkCrt();

		if ( _vscwprintf_ptr != NULL )
			return _vscwprintf_ptr( pszFormat, args );

		WCHAR buf[ 4000 ];
		return vswprintf_ptr( buf, SIZEOF(buf), pszFormat, args );
	#else
		return _vscwprintf( pszFormat, args );
	#endif
}

int __stdcall ChTraitsCRT<wchar_t>::Format( LPWSTR pszBuffer, size_t nLength, LPCWSTR pszFormat, va_list args)
{
	#if _MSC_VER < 1400
		checkCrt();

		if ( _vsnwprintf_ptr != NULL )
			return _vsnwprintf_ptr( pszBuffer, nLength, pszFormat, args );

		return vswprintf_ptr( pszBuffer, nLength, pszFormat, args );
	#else
		return _vsnwprintf( pszBuffer, nLength, pszFormat, args );
	#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// ChTraitsCRT<char>

int __stdcall ChTraitsCRT<char>::GetFormattedLength( LPCSTR pszFormat, va_list args )
{
	#if _MSC_VER < 1400
		checkCrt();

		if ( _vscprintf_ptr != NULL )
			return _vscprintf_ptr( pszFormat, args );

		char buf[4000];
		return vsprintf_ptr( buf, sizeof(buf), pszFormat, args );
	#else
		return _vscprintf( pszFormat, args );
	#endif
}

int __stdcall ChTraitsCRT<char>::Format( LPSTR pszBuffer, size_t nlength, LPCSTR pszFormat, va_list args )
{
	#if _MSC_VER < 1400
		checkCrt();

		return _vsnprintf( pszBuffer, nlength, pszFormat, args );
	#else
		return vsprintf_s( pszBuffer, nlength, pszFormat, args );
	#endif
}

