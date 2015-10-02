#include "stdafx.h"

using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;

ToastEventHandler::ToastEventHandler(_In_ ToastHandlerData *pData) : _ref(0), _thd(pData)
{
	CallPopupProc(UM_INITPOPUP);
}

ToastEventHandler::~ToastEventHandler()
{
	CallPopupProc(UM_FREEPLUGINDATA);
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

	if (*ppv) ((IUnknown*)*ppv)->AddRef();
	return (*ppv ? S_OK : E_NOINTERFACE);
}

IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification*, _In_ IInspectable*)
{
	CallPopupProc(WM_COMMAND);
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
			CallPopupProc(WM_CONTEXTMENU);
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
	if (_thd->tstNotification != nullptr)
	{
		mir_cslock lck(csNotifications);
		lstNotifications.remove(_thd->tstNotification);
	}
	_thd->tstNotification = nullptr;
}

LRESULT ToastEventHandler::CallPopupProc(UINT uMsg)
{
	return (_thd->pPopupProc ? _thd->pPopupProc((HWND)this, uMsg, 0, 0) : 0);
}

void* ToastEventHandler::GetPluginData()
{
	return _thd->vPopupData;
}

MCONTACT ToastEventHandler::GetContact()
{
	return _thd->hContact;
}