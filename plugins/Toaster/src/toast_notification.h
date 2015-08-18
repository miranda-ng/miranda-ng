#ifndef _TOAST_NOTIFICATION_H_
#define _TOAST_NOTIFICATION_H_

class ToastNotification
{
private:
	wchar_t* _text;
	wchar_t* _caption;
	wchar_t* _imagePath;

	HRESULT GetNodeByTag(_In_ HSTRING tagName, _Outptr_ ABI::Windows::Data::Xml::Dom::IXmlNode **node, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml);
	HRESULT AddNode(_In_ HSTRING name, _Outptr_ ABI::Windows::Data::Xml::Dom::IXmlNode **node, _In_ ABI::Windows::Data::Xml::Dom::IXmlNode *rootNode, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml);
	HRESULT SetNodeValueString(_In_ HSTRING inputString, _In_ ABI::Windows::Data::Xml::Dom::IXmlNode* node, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml);
	HRESULT SetTextValues(_In_reads_(textValuesCount) wchar_t** textValues, _In_ UINT32 textValuesCount, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml);
	HRESULT SetImageSrc(_In_z_ wchar_t* imagePath, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml);
	HRESULT Setup(_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml);
	HRESULT CreateXml(_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml);
	HRESULT Create(_Outptr_ ABI::Windows::UI::Notifications::IToastNotification** notification);

public:
	COLORREF background;
	COLORREF foreground;

	ToastNotification(_In_ wchar_t* text, _In_ wchar_t* caption = nullptr, _In_ wchar_t* imagePath = nullptr);
	~ToastNotification();

	HRESULT Show();
	HRESULT Show(_In_ ToastEventHandler* handler);
};

#endif //_TOAST_NOTIFICATION_H_