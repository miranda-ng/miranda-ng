#pragma once

typedef __FITypedEventHandler_2_Windows__CUI__CNotifications__CToastNotification_IInspectable ToastActivationHandler;
typedef __FITypedEventHandler_2_Windows__CUI__CNotifications__CToastNotification_Windows__CUI__CNotifications__CToastDismissedEventArgs ToastDismissHandler;
typedef __FITypedEventHandler_2_Windows__CUI__CNotifications__CToastNotification_Windows__CUI__CNotifications__CToastFailedEventArgs ToastFailHandler;

class ToastNotification
{
private:

	MCONTACT _hContact;

	WNDPROC _pfnPopupProc;
	void *_pvPopupData;


	wchar_t* _text;
	wchar_t* _caption;
	wchar_t* _imagePath;

	EventRegistrationToken _ertActivated;
	EventRegistrationToken _ertDismissed;
	EventRegistrationToken _ertFailed;

	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics> notificationManager;
	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotifier> notifier;
	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotification> notification;

	HRESULT CreateXml(_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml);
	HRESULT Create(_Outptr_ ABI::Windows::UI::Notifications::IToastNotification** notification);

public:
	ToastNotification(_In_ wchar_t* text, _In_ wchar_t* caption = nullptr, _In_ wchar_t* imagePath = nullptr, MCONTACT hContact = 0, WNDPROC pWndProc = nullptr, void *pData = nullptr);
	~ToastNotification();

	inline void Destroy()
	{	mir_cslock lck(csNotifications); lstNotifications.remove(this);
	}

	HRESULT OnActivate(_In_ ABI::Windows::UI::Notifications::IToastNotification*, IInspectable*);
	HRESULT OnDismiss(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastDismissedEventArgs*);
	HRESULT OnFail(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastFailedEventArgs*);

	inline void* GetPluginData()
	{	return _pvPopupData;
	}
	inline MCONTACT GetContact()
	{	return _hContact;
	}

	inline LRESULT CallPopupProc(UINT uMsg)
	{	return (_pfnPopupProc ? _pfnPopupProc((HWND)this, uMsg, 0, 0) : 0);
	}

};
