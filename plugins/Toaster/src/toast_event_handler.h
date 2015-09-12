#ifndef _TOAST_EVENT_HANDLER_H_
#define _TOAST_EVENT_HANDLER_H_

typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ::IInspectable *> DesktopToastActivatedEventHandler;
typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ABI::Windows::UI::Notifications::ToastDismissedEventArgs *> DesktopToastDismissedEventHandler;
typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ABI::Windows::UI::Notifications::ToastFailedEventArgs *> DesktopToastFailedEventHandler;

struct ToastHandlerData : public MZeroedObject
{
	MCONTACT hContact;

	WNDPROC pPopupProc;
	void *vPopupData;

	ToastNotification *tstNotification;
};

class ToastEventHandler : public Microsoft::WRL::Implements<DesktopToastActivatedEventHandler, DesktopToastDismissedEventHandler, DesktopToastFailedEventHandler>
{
public:
	ToastHandlerData *_thd;

	ToastEventHandler::ToastEventHandler();
	ToastEventHandler::ToastEventHandler(_In_ ToastHandlerData *pData);
	~ToastEventHandler();

	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void** ppv);

	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification* sender, _In_ IInspectable* args);
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification* /* sender */, _In_ ABI::Windows::UI::Notifications::IToastDismissedEventArgs* e);
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification* /* sender */, _In_ ABI::Windows::UI::Notifications::IToastFailedEventArgs* /* e */);
private:
	ULONG _ref;

};

#endif //_TOAST_EVENT_HANDLER_H_