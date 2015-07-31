#include "stdafx.h"

ToastEventHandler::ToastEventHandler() : _ref(1), _callback(nullptr), _arg(nullptr)
{
}

ToastEventHandler::ToastEventHandler(_In_ pEventHandler callback, _In_ void* arg) : _ref(1), _callback(callback), _arg(arg)
{
}

ToastEventHandler::~ToastEventHandler()
{
}

IFACEMETHODIMP_(ULONG) ToastEventHandler::AddRef()
{
	return InterlockedIncrement(&_ref);
}

IFACEMETHODIMP_(ULONG) ToastEventHandler::Release()
{
	ULONG l = InterlockedDecrement(&_ref);
	if (l == 0) delete this;
	return l;
}

IFACEMETHODIMP ToastEventHandler::QueryInterface(_In_ REFIID riid, _COM_Outptr_ void **ppv)
{
	if (IsEqualIID(riid, IID_IUnknown))
		*ppv = static_cast<IUnknown*>(static_cast<DesktopToastActivatedEventHandler*>(this));
	else if (IsEqualIID(riid, __uuidof(DesktopToastActivatedEventHandler)))
		*ppv = static_cast<DesktopToastActivatedEventHandler*>(this);
	else *ppv = nullptr;

	if (*ppv)
	{
		reinterpret_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification* /* sender */, _In_::IInspectable* /* args */)
{
	if (_callback != nullptr)
		_callback(_arg);

	return S_OK;
}
