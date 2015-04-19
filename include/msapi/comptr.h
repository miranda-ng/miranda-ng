// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.

#ifndef __ATLCOMCLI_H__
#define __ATLCOMCLI_H__

#pragma once

#include <assert.h>
#include <ole2.h>
#include <olectl.h>

#pragma warning (push)
#pragma warning (disable: 4127)  // conditional expression constant
#pragma warning (disable: 4571)  //catch(...) blocks compiled with /EHs do NOT catch or re-throw Structured Exceptions

template <class T>
class _NoAddRefReleaseOnCComPtr :
	public T
{
private:
	STDMETHOD_(ULONG, AddRef)() = 0;
	STDMETHOD_(ULONG, Release)() = 0;
};

template <class T>
class CComPtrBase
{
protected:
	CComPtrBase() throw()
	{
		p = NULL;
	}
	CComPtrBase(_Inout_opt_ T* lp) throw()
	{
		p = lp;
		if (p != NULL)
			p->AddRef();
	}
public:
	typedef T _PtrClass;
	~CComPtrBase() throw()
	{
		if (p)
			p->Release();
	}
	operator T*() const throw()
	{
		return p;
	}
	T& operator*() const
	{
		return *p;
	}
	T** operator&() throw()
	{
		return &p;
	}
	_NoAddRefReleaseOnCComPtr<T>* operator->() const throw()
	{
		return (_NoAddRefReleaseOnCComPtr<T>*)p;
	}
	bool operator!() const throw()
	{
		return (p == NULL);
	}
	bool operator<(_In_opt_ T* pT) const throw()
	{
		return p < pT;
	}
	bool operator!=(_In_opt_ T* pT) const
	{
		return !operator==(pT);
	}
	bool operator==(_In_opt_ T* pT) const throw()
	{
		return p == pT;
	}

	// Release the interface and set to NULL
	void Release() throw()
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	// Compare two objects for equivalence
	bool IsEqualObject(_Inout_opt_ IUnknown* pOther) throw()
	{
		if (p == NULL && pOther == NULL)
			return true;	// They are both NULL objects

		if (p == NULL || pOther == NULL)
			return false;	// One is NULL the other is not

		CComPtr<IUnknown> punk1;
		CComPtr<IUnknown> punk2;
		p->QueryInterface(__uuidof(IUnknown), (void**)&punk1);
		pOther->QueryInterface(__uuidof(IUnknown), (void**)&punk2);
		return punk1 == punk2;
	}
	// Attach to an existing interface (does not AddRef)
	void Attach(_In_opt_ T* p2) throw()
	{
		if (p)
			p->Release();
		p = p2;
	}
	// Detach the interface (does not Release)
	T* Detach() throw()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	_Check_return_ HRESULT CopyTo(_Deref_out_opt_ T** ppT) throw()
	{
		ASSERT(ppT != NULL);
		if (ppT == NULL)
			return E_POINTER;
		*ppT = p;
		if (p)
			p->AddRef();
		return S_OK;
	}
	_Check_return_ HRESULT SetSite(_Inout_opt_ IUnknown* punkParent) throw()
	{
		return AtlSetChildSite(p, punkParent);
	}
	_Check_return_ HRESULT Advise(
		_Inout_ IUnknown* pUnk,
		_In_ const IID& iid,
		_Out_ LPDWORD pdw) throw()
	{
		return AtlAdvise(p, pUnk, iid, pdw);
	}
	_Check_return_ HRESULT CoCreateInstance(
		_In_ REFCLSID rclsid,
		_Inout_opt_ LPUNKNOWN pUnkOuter = NULL,
		_In_ DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
	}
	_Check_return_ HRESULT CoCreateInstance(
		_In_z_ LPCOLESTR szProgID,
		_Inout_opt_ LPUNKNOWN pUnkOuter = NULL,
		_In_ DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
		if (SUCCEEDED(hr))
			hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
		return hr;
	}
	template <class Q>
	_Check_return_ HRESULT QueryInterface(_Deref_out_ Q** pp) const throw()
	{
		return p->QueryInterface(__uuidof(Q), (void**)pp);
	}
	T* p;
};

template <class T>
class CComPtr :
	public CComPtrBase<T>
{
public:
	CComPtr() throw()
	{
	}
	CComPtr(_Inout_opt_ T* lp) throw() :
		CComPtrBase<T>(lp)
	{
	}
	CComPtr(_Inout_ const CComPtr<T>& lp) throw() :
		CComPtrBase<T>(lp.p)
	{
	}
	T* operator=(_Inout_opt_ T* lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}
	template <typename Q>
	T* operator=(_Inout_ const CComPtr<Q>& lp) throw()
	{
        if( !IsEqualObject(lp) )
        {
    		return static_cast<T*>(AtlComQIPtrAssign((IUnknown**)&p, lp, __uuidof(T)));
        }
        return *this;
	}
	T* operator=(_Inout_ const CComPtr<T>& lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}
	CComPtr(_Inout_ CComPtr<T>&& lp) throw() :
		CComPtrBase<T>()
	{
		p = lp.p;
		lp.p = NULL;
	}
	T* operator=(_Inout_ CComPtr<T>&& lp) throw()
	{
		if (*this != lp)
		{
			if (p != NULL)
				p->Release();

			p = lp.p;
			lp.p = NULL;
		}
		return *this;
	}
};

template <class T, const IID* piid = &__uuidof(T)>
class CComQIPtr :
	public CComPtr<T>
{
public:
	CComQIPtr() throw()
	{
	}
	CComQIPtr(_Inout_opt_ T* lp) throw() :
		CComPtr<T>(lp)
	{
	}
	CComQIPtr(_Inout_ const CComQIPtr<T,piid>& lp) throw() :
		CComPtr<T>(lp.p)
	{
	}
	CComQIPtr(_Inout_opt_ IUnknown* lp) throw()
	{
		if (lp != NULL)
			lp->QueryInterface(*piid, (void **)&p);
	}
	T* operator=(_Inout_opt_ T* lp) throw()
	{
        if(*this!=lp)
        {
		    return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}
	T* operator=(_Inout_ const CComQIPtr<T,piid>& lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp.p));
        }
        return *this;
	}
	T* operator=(_Inout_opt_ IUnknown* lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComQIPtrAssign((IUnknown**)&p, lp, *piid));
        }
        return *this;
	}
};

#pragma warning (pop)

#endif	// __ATLCOMCLI_H__
