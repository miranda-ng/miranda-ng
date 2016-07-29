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
	if (_pvPopupData != nullptr)
		CallPopupProc(UM_FREEPLUGINDATA);
	notifier->Hide(notification.Get());
}


HRESULT ToastNotification::CreateXml(_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml)
{
	ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocumentIO> xmlDocument;
	CHECKHR(Windows::Foundation::ActivateInstance(StringReferenceWrapper(RuntimeClass_Windows_Data_Xml_Dom_XmlDocument).Get(), &xmlDocument));

	HXML xmlToast = xmlCreateNode(L"toast", NULL, 0);

	HXML xmlAudioNode = xmlAddChild(xmlToast, L"audio", NULL);
	xmlAddAttr(xmlAudioNode, L"silent", L"true");

	HXML xmlVisualNode = xmlAddChild(xmlToast, L"visual", NULL);

	HXML xmlBindingNode = xmlAddChild(xmlVisualNode, L"binding", NULL);
	xmlAddAttr(xmlBindingNode, L"template", L"ToastImageAndText02");
	if (_imagePath)
	{
		HXML xmlImageNode = xmlAddChild(xmlBindingNode, L"image", NULL);
		xmlAddAttr(xmlImageNode, L"id", L"1");
		xmlAddAttr(xmlImageNode, L"src", CMStringW(FORMAT, L"file:///%s", _imagePath));
	}

	HXML xmlTitleNode = xmlAddChild(xmlBindingNode, L"text", _caption != NULL ? _caption : L"Miranda NG");
	xmlAddAttr(xmlTitleNode, L"id", L"1");
	if (_text)
	{
		HXML xmlTextNode = xmlAddChild(xmlBindingNode, L"text", _text);
		xmlAddAttr(xmlTextNode, L"id", L"2");
	}

	int nLength;
	ptrW xtmp(xmlToString(xmlToast, &nLength));
	xmlDestroyNode(xmlToast);

	CHECKHR(xmlDocument->LoadXml(StringReferenceWrapper(xtmp, nLength).Get()));

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
	ABI::Windows::UI::Notifications::ToastDismissalReason tdr;
	CHECKHR(e->get_Reason(&tdr));
	if (tdr == ABI::Windows::UI::Notifications::ToastDismissalReason_UserCanceled)
		CallPopupProc(WM_CONTEXTMENU);
	Destroy();
	return S_OK;
}

HRESULT ToastNotification::OnFail(_In_ ABI::Windows::UI::Notifications::IToastNotification*, _In_ ABI::Windows::UI::Notifications::IToastFailedEventArgs*)
{
	Destroy();
	return S_OK;
}

void ToastNotification::Destroy()
{
	mir_cslock lck(csNotifications);
	lstNotifications.remove(this);
}