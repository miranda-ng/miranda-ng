#include "stdafx.h"

using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;

ToastEventHandler::ToastEventHandler(_In_ ToastHandlerData *pData) : _ref(0), _thd(pData)
{
	if (_thd->pPopupProc)
		_thd->pPopupProc((HWND)this, UM_INITPOPUP, (WPARAM)this, 0);
}

ToastEventHandler::~ToastEventHandler()
{
	if (_thd->pPopupProc)
		_thd->pPopupProc((HWND)this, UM_FREEPLUGINDATA, 0, 0);
}

IFACEMETHODIMP_(ULONG) ToastEventHandler::AddRef()
{
	return InterlockedIncrement(&_ref);
}

IFACEMETHODIMP_(ULONG) ToastEventHandler::Release()
{
	ULONG l = InterlockedDecrement(&_ref);
	if (l == 0) 
		delete this;
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

	if (*ppv) ((IUnknown*)*ppv)->AddRef();
	return (*ppv ? S_OK : E_NOINTERFACE);
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification*, _In_ IInspectable*)
{
	if (_thd->pPopupProc)
		_thd->pPopupProc((HWND)this, WM_COMMAND, 0, 0);

	DestroyNotification();

	return S_OK;
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification*, _In_ IToastDismissedEventArgs*  e)
{
	ToastDismissalReason tdr;
	CHECKHR(e->get_Reason(&tdr));

	switch (tdr)
	{
	case ToastDismissalReason_UserCanceled:
		{
			if (_thd->pPopupProc)
				_thd->pPopupProc((HWND)this, WM_CONTEXTMENU, 0, 0);
			_thd->tstNotification->Hide();
			break;
		}
	case ToastDismissalReason_ApplicationHidden:
	case ToastDismissalReason_TimedOut:
		DestroyNotification();
		break;
	}

	return S_OK;
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification*, _In_ IToastFailedEventArgs*)
{
	DestroyNotification();
	return S_OK;
}

void ToastEventHandler::DestroyNotification()
{
	mir_cslock lck(csNotifications);
	lstNotifications.remove(_thd->tstNotification);
}

void* ToastEventHandler::GetPluginData()
{
	return _thd->vPopupData;
}

MCONTACT ToastEventHandler::GetContact()
{
	return _thd->hContact;
}