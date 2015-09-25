#ifndef _TOAST_EVENT_HANDLER_H_
#define _TOAST_EVENT_HANDLER_H_

typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ::IInspectable *> DesktopToastActivatedEventHandler;
typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ABI::Windows::UI::Notifications::ToastDismissedEventArgs *> DesktopToastDismissedEventHandler;
typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ABI::Windows::UI::Notifications::ToastFailedEventArgs *> DesktopToastFailedEventHandler;

struct ToastHandlerData;

class ToastEventHandler : public Microsoft::WRL::Implements<DesktopToastActivatedEventHandler, DesktopToastDismissedEventHandler, DesktopToastFailedEventHandler>
{
public:
	ToastEventHandler::ToastEventHandler(_In_ ToastHandlerData*);
	~ToastEventHandler();

	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void** ppv);

	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ IInspectable*);
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastDismissedEventArgs*);
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastFailedEventArgs*);

	void* GetPluginData();
	MCONTACT GetContact();

private:
	ULONG _ref;
	std::unique_ptr<ToastHandlerData> _thd;

	void DestroyNotification();
};

#endif //_TOAST_EVENT_HANDLER_H_