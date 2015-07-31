#ifndef _TOAST_EVENT_HANDLER_H_
#define _TOAST_EVENT_HANDLER_H_

typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, ::IInspectable*> DesktopToastActivatedEventHandler;

class ToastEventHandler : public Microsoft::WRL::Implements<ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, ::IInspectable*>>
{
public:
	ToastEventHandler::ToastEventHandler();
	ToastEventHandler::ToastEventHandler(_In_ pEventHandler callback, _In_ void* arg = nullptr);
	~ToastEventHandler();

	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void** ppv);

	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification* sender, _In_ IInspectable* args);

private:
	ULONG _ref;
	void* _arg;
	pEventHandler _callback;
};

#endif //_TOAST_EVENT_HANDLER_H_