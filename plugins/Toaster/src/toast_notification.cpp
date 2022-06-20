#include "stdafx.h"
#include <wrl.h>
using namespace Microsoft::WRL;

ToastNotification::ToastNotification(
	_In_ wchar_t* text,
	_In_ wchar_t* caption, 
	_In_ wchar_t* imagePath,
	MCONTACT hContact,
	WNDPROC pWndProc,
	void *pData )
	: _text(text), _caption(caption), _imagePath(imagePath), _hContact(hContact), _pfnPopupProc(pWndProc), _pvPopupData(pData)
{
	{
		mir_cslock lck(csNotifications);
		lstNotifications.insert(this);
	}
	Windows::Foundation::GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &notificationManager);
	notificationManager->CreateToastNotifierWithId(StringReferenceWrapper(::AppUserModelID).Get(), &notifier);
	Create(&notification);

	notification->add_Activated(Callback<ToastActivationHandler>(this, &ToastNotification::OnActivate).Get(), &_ertActivated);
	notification->add_Dismissed(Callback<ToastDismissHandler>(this, &ToastNotification::OnDismiss).Get(), &_ertDismissed);
	notification->add_Failed(Callback<ToastFailHandler>(this, &ToastNotification::OnFail).Get(), &_ertFailed);
	notifier->Show(notification.Get());
	CallPopupProc(UM_INITPOPUP);
}

ToastNotification::~ToastNotification()
{
	_signature = 0;
	if (_pvPopupData != nullptr)
		CallPopupProc(UM_FREEPLUGINDATA);
	notifier->Hide(notification.Get());
}


HRESULT ToastNotification::CreateXml(_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml)
{
	ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocumentIO> xmlDocument;
	CHECKHR(Windows::Foundation::ActivateInstance(StringReferenceWrapper(RuntimeClass_Windows_Data_Xml_Dom_XmlDocument).Get(), &xmlDocument));

	TiXmlDocument doc;
	TiXmlElement *xmlToast = doc.NewElement("toast"); doc.InsertEndChild(xmlToast);

	TiXmlElement *xmlAudioNode = doc.NewElement("audio"); xmlToast->InsertEndChild(xmlAudioNode);
	xmlAudioNode->SetAttribute("silent", "true");

	TiXmlElement *xmlVisualNode = doc.NewElement("visual"); xmlToast->InsertEndChild(xmlVisualNode);

	TiXmlElement *xmlBindingNode = doc.NewElement("binding"); xmlVisualNode->InsertEndChild(xmlBindingNode);
	xmlBindingNode->SetAttribute("template", "ToastImageAndText02");
	
	if (_imagePath) {
		TiXmlElement *xmlImageNode = doc.NewElement("image"); xmlBindingNode->InsertEndChild(xmlImageNode);
		xmlImageNode->SetAttribute("id", 1);
		xmlImageNode->SetAttribute("src", CMStringW(FORMAT, L"file:///%s", _imagePath));
	}

	TiXmlElement *xmlTitleNode = doc.NewElement("text"); xmlBindingNode->InsertEndChild(xmlTitleNode);
	xmlTitleNode->SetAttribute("id", 1); xmlTitleNode->SetText(_caption != nullptr ? _caption : L"Miranda NG");
	
	if (_text) {
		TiXmlElement *xmlTextNode = doc.NewElement("text"); xmlBindingNode->InsertEndChild(xmlTextNode);
		xmlTextNode->SetAttribute("id", 2); xmlTextNode->SetText(_text);
	}

	tinyxml2::XMLPrinter printer(0, true);
	doc.Print(&printer);
	Utf2T xtmp(printer.CStr());
	CHECKHR(xmlDocument->LoadXml(StringReferenceWrapper(xtmp, mir_wstrlen(xtmp)).Get()));

	return xmlDocument.CopyTo(xml);
}

HRESULT ToastNotification::Create(_Outptr_ ABI::Windows::UI::Notifications::IToastNotification** _notification)
{
	ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocument> xml;
	CHECKHR(CreateXml(&xml));

	ComPtr<ABI::Windows::UI::Notifications::IToastNotificationFactory> factory;
	CHECKHR(Windows::Foundation::GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), &factory));

	return factory->CreateToastNotification(xml.Get(), _notification);
}

HRESULT ToastNotification::OnActivate(_In_ ABI::Windows::UI::Notifications::IToastNotification*, IInspectable*)
{
	CallPopupProc(WM_COMMAND);
	Destroy();
	return S_OK;
}

HRESULT ToastNotification::OnDismiss(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastDismissedEventArgs *e)
{
	if (_signature == TOAST_SIGNATURE) {
		ABI::Windows::UI::Notifications::ToastDismissalReason tdr;
		CHECKHR(e->get_Reason(&tdr));
		if (tdr == ABI::Windows::UI::Notifications::ToastDismissalReason_UserCanceled)
			CallPopupProc(WM_CONTEXTMENU);
		Destroy();
	}
	return S_OK;
}

HRESULT ToastNotification::OnFail(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastFailedEventArgs*)
{
	if (_signature == TOAST_SIGNATURE)
		Destroy();
	return S_OK;
}

void ToastNotification::Destroy()
{
	mir_cslock lck(csNotifications);
	lstNotifications.remove(this);
}