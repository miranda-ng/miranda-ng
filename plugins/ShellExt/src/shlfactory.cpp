#include "stdafx.h"
#include "shlcom.h"

/////////////////////////////////////////////////////////////////////////////////////////

TClassFactoryRec::TClassFactoryRec() :
	RefCount(0)
{
	DllFactoryCount++;
}

HRESULT TClassFactoryRec::QueryInterface(REFIID riid, void **ppvObject)
{
	if (riid == IID_IUnknown)
		logA("TClassFactoryRec retrieved as IUnknown: %d\n", RefCount);
	else if (riid == IID_IClassFactory)
		logA("TClassFactoryRec retrieved as IClassFactory: %d\n", RefCount);
	else {
		#ifdef LOG_ENABLED
			RPC_CSTR szGuid;
			UuidToStringA(&riid, &szGuid);
			logA("TClassFactoryRec::QueryInterface {%s} failed\n", szGuid);
			RpcStringFreeA(&szGuid);
		#endif
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	*ppvObject = this;
	return S_OK;
}

ULONG TClassFactoryRec::AddRef()
{
	return ++RefCount;
}

ULONG TClassFactoryRec::Release()
{
	ULONG result = --RefCount;
	if (result == 0) {
		logA("TClassFactoryRec released\n");
		delete this;
		DllFactoryCount--;
	}
	return result;
}

HRESULT TClassFactoryRec::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	if (ppvObject == 0)
		return E_POINTER;

	*ppvObject = NULL;
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;

	TShellExt *p = new TShellExt();
	if (p == NULL)
		return E_OUTOFMEMORY;

	HRESULT hr = p->QueryInterface(riid, ppvObject);
	if ( FAILED(hr))
		delete p;
	return hr;
}

HRESULT TClassFactoryRec::LockServer(BOOL)
{
	return E_NOTIMPL;
}
