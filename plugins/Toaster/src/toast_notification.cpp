#include "stdafx.h"

ToastNotification::ToastNotification(_In_ wchar_t* text, _In_ wchar_t* caption, _In_ wchar_t* imagePath)
	: _text(text), _caption(caption), _imagePath(imagePath)
{
}

ToastNotification::~ToastNotification()
{
}

HRESULT ToastNotification::Initialize()
{
	HRESULT hr = Windows::Foundation::GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &notificationManager);
	if (SUCCEEDED(hr))
	{
		hr = notificationManager->CreateToastNotifierWithId(StringReferenceWrapper(::AppUserModelID).Get(), &notifier);
		if (SUCCEEDED(hr))
		{
			hr = Create(&notification);
		}
	}
	return hr;
}

HRESULT ToastNotification::GetNodeByTag(_In_ HSTRING tagName, _Outptr_ ABI::Windows::Data::Xml::Dom::IXmlNode **node, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNodeList> nodeList;
	HRESULT hr = xml->GetElementsByTagName(tagName, &nodeList);
	if (SUCCEEDED(hr))
	{
		UINT32 length;
		hr = nodeList->get_Length(&length);
		if (SUCCEEDED(hr))
		{
			if (length)
				hr = nodeList->Item(0, node);
			else
				hr = E_INVALIDARG;
		}
	}
	return hr;
}

HRESULT ToastNotification::AddNode(_In_ HSTRING name, _Outptr_ ABI::Windows::Data::Xml::Dom::IXmlNode **node, _In_ ABI::Windows::Data::Xml::Dom::IXmlNode *rootNode, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlElement> element;
	HRESULT hr = xml->CreateElement(name, &element);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> tempNode;
	hr = element.As(&tempNode);
	if (FAILED(hr))
		return hr;

	return rootNode->AppendChild(tempNode.Get(), node);
}

HRESULT ToastNotification::SetNodeValueString(_In_ HSTRING inputString, _In_ ABI::Windows::Data::Xml::Dom::IXmlNode* node, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlText> inputText;
	HRESULT hr = xml->CreateTextNode(inputString, &inputText);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> inputTextNode;
	hr = inputText.As(&inputTextNode);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> pAppendedChild;
	return hr = node->AppendChild(inputTextNode.Get(), &pAppendedChild);
}

HRESULT ToastNotification::SetTextValues(_In_reads_(textValuesCount) wchar_t** textValues, _In_ UINT32 textValuesCount, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
	HRESULT hr = textValues != nullptr && textValuesCount > 0 ? S_OK : E_INVALIDARG;
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNodeList> nodeList;
	hr = xml->GetElementsByTagName(StringReferenceWrapper(L"text").Get(), &nodeList);
	if (FAILED(hr))
		return hr;

	UINT32 nodeListLength;
	hr = nodeList->get_Length(&nodeListLength);
	if (FAILED(hr))
		return hr;

	hr = textValuesCount <= nodeListLength ? S_OK : E_INVALIDARG;
	if (FAILED(hr))
		return hr;

	for (UINT i = 0; i < textValuesCount; i++)
	{
		Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> textNode;
		hr = nodeList->Item(i, &textNode);
		if (FAILED(hr))
			return hr;

		hr = SetNodeValueString(StringReferenceWrapper(textValues[i], (UINT32)(mir_wstrlen(textValues[i]))).Get(), textNode.Get(), xml);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

HRESULT ToastNotification::SetImageSrc(_In_z_ wchar_t* imagePath, _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
	wchar_t imageSrc[MAX_PATH];
	mir_snwprintf(imageSrc, _T("file:///%s"), imagePath);

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> imageNode;
	HRESULT hr = GetNodeByTag(StringReferenceWrapper(L"image").Get(), &imageNode, xml);

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNamedNodeMap> attributes;
	hr = imageNode->get_Attributes(&attributes);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> srcAttribute;
	hr = attributes->GetNamedItem(StringReferenceWrapper(L"src").Get(), &srcAttribute);
	if (FAILED(hr))
		return hr;

	return hr = SetNodeValueString(StringReferenceWrapper(imageSrc).Get(), srcAttribute.Get(), xml);
}

HRESULT ToastNotification::Setup(_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> toastNode;
	HRESULT hr = GetNodeByTag(StringReferenceWrapper(L"toast").Get(), &toastNode, xml);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> audioNode;
	hr = AddNode(StringReferenceWrapper(L"audio").Get(), &audioNode, toastNode.Get(), xml);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNamedNodeMap> attributes;
	hr = audioNode->get_Attributes(&attributes);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlAttribute> silentAttribute;
	hr = xml->CreateAttribute(StringReferenceWrapper(L"silent").Get(), &silentAttribute);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> silentAttributeNode;
	hr = silentAttribute.As(&silentAttributeNode);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> tempNode;
	hr = attributes->SetNamedItem(silentAttributeNode.Get(), &tempNode);
	if (FAILED(hr))
		return hr;

	return SetNodeValueString(StringReferenceWrapper(L"true").Get(), silentAttributeNode.Get(), xml);
}

HRESULT ToastNotification::CreateXml(_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml)
{
	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocumentIO> xmlDocument;
	HRESULT hr = Windows::Foundation::ActivateInstance(StringReferenceWrapper(RuntimeClass_Windows_Data_Xml_Dom_XmlDocument).Get(), &xmlDocument);
	if (FAILED(hr))
		return hr;

	if(_imagePath == nullptr)
	{
		hr = xmlDocument->LoadXml(StringReferenceWrapper(L"<toast><visual><binding template=\"ToastText02\"><text id=\"1\"></text><text id=\"2\"></text></binding></visual></toast>").Get());
	}
	else
	{
		hr = xmlDocument->LoadXml(StringReferenceWrapper(L"<toast><visual><binding template=\"ToastImageAndText02\"><image id=\"1\" src=\"\"/><text id=\"1\"></text><text id=\"2\"></text></binding></visual></toast>").Get());
	}
	if (FAILED(hr))
		return hr;

	hr = xmlDocument.CopyTo(xml);
	if (FAILED(hr))
		return hr;

	/*ABI::Windows::UI::Notifications::ToastTemplateType templateId = _imagePath == nullptr
		? ABI::Windows::UI::Notifications::ToastTemplateType_ToastText02
		: ABI::Windows::UI::Notifications::ToastTemplateType_ToastImageAndText02;

	HRESULT hr = notificationManager->GetTemplateContent(templateId, xml);
	if (FAILED(hr))
		return hr;*/

	hr = Setup(*xml);
	if (FAILED(hr))
		return hr;

	if (_imagePath)
	{
		hr = SetImageSrc(_imagePath, *xml);
		if (FAILED(hr))
			return hr;
	}

	wchar_t* textValues[] =
	{
		_caption == nullptr ? L"Miranda NG" : _caption,
		_text
	};

	return SetTextValues(textValues, _countof(textValues), *xml);
}

HRESULT ToastNotification::Create(_Outptr_ ABI::Windows::UI::Notifications::IToastNotification** _notification)
{
	Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocument> xml;
	HRESULT hr = CreateXml(&xml);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationFactory> factory;
	hr = Windows::Foundation::GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), &factory);
	if (FAILED(hr))
		return hr;
	return hr = factory->CreateToastNotification(xml.Get(), _notification);
}

HRESULT ToastNotification::Show()
{
	return Show(new ToastEventHandler(nullptr));
}

HRESULT ToastNotification::Show(_In_ ToastEventHandler* handler)
{
	EventRegistrationToken activatedToken, dismissedToken, failedToken;
	Microsoft::WRL::ComPtr<ToastEventHandler> eventHandler(handler);

	notification->add_Activated(eventHandler.Get(), &activatedToken);
	notification->add_Dismissed(eventHandler.Get(), &dismissedToken);
	notification->add_Failed(eventHandler.Get(), &failedToken);

	return notifier->Show(notification.Get());
}

HRESULT ToastNotification::Hide()
{
	return notifier->Hide(notification.Get());
}