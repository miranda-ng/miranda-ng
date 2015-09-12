#include "stdafx.h"

using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;

ToastEventHandler::ToastEventHandler() : _ref(1)
{
}

ToastEventHandler::ToastEventHandler(_In_ ToastHandlerData *pData) : _ref(1), _thd(pData)
{
	if (_thd->pPopupProc)
		_thd->pPopupProc((HWND)this, UM_INITPOPUP, 0, 0);
}

ToastEventHandler::~ToastEventHandler()
{
	_thd->pPopupProc((HWND)this, UM_FREEPLUGINDATA, 0, 0);

	mir_cslock lck(csNotifications);
	lstNotifications.remove(_thd->tstNotification);
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
		*ppv = (IUnknown*)(DesktopToastActivatedEventHandler*)(this);
	else if (IsEqualIID(riid, __uuidof(DesktopToastActivatedEventHandler)))
		*ppv = (DesktopToastActivatedEventHandler*)(this);
	else if (IsEqualIID(riid, __uuidof(DesktopToastDismissedEventHandler)))
		*ppv = (DesktopToastDismissedEventHandler*)(this);
	else if (IsEqualIID(riid, __uuidof(DesktopToastFailedEventHandler)))
		*ppv = (DesktopToastFailedEventHandler*)(this);
	else *ppv = nullptr;

	if (*ppv) {
		((IUnknown*)*ppv)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification * /*sender*/, _In_ IInspectable* /*args*/)
{
	if (_thd->pPopupProc)
		_thd->pPopupProc((HWND)this, WM_COMMAND, 0, 0);
	delete this;
	return S_OK;
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification* /* sender */, _In_ IToastDismissedEventArgs*  /*e*/)
{
	if (_thd->pPopupProc)
		_thd->pPopupProc((HWND)this, WM_CONTEXTMENU, 0, 0);
	_thd->tstNotification->Hide();
	delete this;
	return S_OK;
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification* /* sender */, _In_ IToastFailedEventArgs*  /*e*/ )
{
	delete this;
	return S_OK;
}