#ifndef _TOAST_NOTIFICATION_H_
#define _TOAST_NOTIFICATION_H_

class ToastNotification
{
private:
	wchar_t* _text;
	wchar_t* _caption;
	wchar_t* _imagePath;

	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics> notificationManager;
	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotifier> notifier;
	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotification> notification;

	HRESULT CreateXml(_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml);
	HRESULT Create(_Outptr_ ABI::Windows::UI::Notifications::IToastNotification** notification);

public:
	COLORREF background;
	COLORREF foreground;

	ToastNotification(_In_ wchar_t* text, _In_ wchar_t* caption = nullptr, _In_ wchar_t* imagePath = nullptr);
	HRESULT Initialize();
	~ToastNotification();

	HRESULT Show();
	HRESULT Show(_In_ ToastEventHandler* handler);
	HRESULT Hide();
};

#endif //_TOAST_NOTIFICATION_H_