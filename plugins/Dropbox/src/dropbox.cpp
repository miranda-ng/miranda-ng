#include "stdafx.h"

CDropbox::CDropbox() : transfers(1, HandleKeySortT)
{
	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEventObj(ME_SYSTEM_MODULESLOADED, GlobalEvent<&CDropbox::OnModulesLoaded>, this);

	hUploadedEventHook = CreateHookableEvent(ME_DROPBOX_UPLOADED);

	CreateServiceFunctionObj(MS_DROPBOX_UPLOAD, GlobalService<&CDropbox::UploadToDropbox>, this);
	CreateServiceFunctionObj(MS_DROPBOX_UPLOADASYNC, GlobalService<&CDropbox::UploadToDropboxAsync>, this);

	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	Proto_RegisterModule(&pd);

	CreateServiceFunction(MODULE PS_GETCAPS, ProtoGetCaps);
	CreateServiceFunction(MODULE PS_GETNAME, ProtoGetName);
	CreateServiceFunction(MODULE PS_LOADICON, ProtoLoadIcon);
	CreateServiceFunctionObj(MODULE PS_GETSTATUS, GlobalService<&CDropbox::ProtoGetStatus>, this);
	CreateServiceFunctionObj(MODULE PSS_FILE, GlobalService<&CDropbox::ProtoSendFile>, this);
	CreateServiceFunctionObj(MODULE PSS_FILECANCEL, GlobalService<&CDropbox::ProtoCancelFile>, this);
	CreateServiceFunctionObj(MODULE PSS_MESSAGE, GlobalService<&CDropbox::ProtoSendMessage>, this);
	CreateServiceFunction(MODULE PSR_MESSAGE, ProtoReceiveMessage);

	pd.szName = MODULE"Inteceptor";
	pd.type = PROTOTYPE_FILTER;
	Proto_RegisterModule(&pd);

	CreateServiceFunctionObj(MODULE "Inteceptor" PSS_FILE, GlobalService<&CDropbox::ProtoSendFileInterceptor>, this);

	InitializeMenus();

	hMessageProcess = 1;
}

CDropbox::~CDropbox()
{
	DestroyHookableEvent(hUploadedEventHook);
}

MCONTACT CDropbox::GetDefaultContact()
{
	if (!hDefaultContact)
		hDefaultContact = db_find_first(MODULE);

	if (!hDefaultContact) {
		hDefaultContact = db_add_contact();
		if (!Proto_AddToContact(hDefaultContact, MODULE)) {
			db_set_s(NULL, MODULE, "Nick", MODULE);
			db_set_s(hDefaultContact, MODULE, "Nick", MODULE);
			db_set_ws(hDefaultContact, "CList", "MyHandle", L"Dropbox");
		}
		db_set_w(hDefaultContact, MODULE, "Status", HasAccessToken() ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
	}

	return hDefaultContact;
}

bool CDropbox::HasAccessToken()
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	return token != NULL;
}

void CDropbox::RequestAccountInfo(void *p)
{
	CDropbox *instance = (CDropbox*)p;

	MCONTACT hContact = instance->GetDefaultContact();

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	GetCurrentAccountRequest request(token);
	NLHR_PTR response(request.Send(instance->hNetlibConnection));
	
	try
	{
		HandleHttpResponse(response);
	}
	catch (DropboxException &ex)
	{
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, ex.what());
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;

	JSONNode referral_link = root.at("referral_link");
	if (!referral_link.empty())
		db_set_s(hContact, MODULE, "Homepage", referral_link.as_string().c_str());

	JSONNode email = root.at("email");
	if (!email.empty())
		db_set_s(hContact, MODULE, "e-mail", email.as_string().c_str());

	JSONNode name = root.at("name");
	if (!name.empty()) {
		db_set_utf(hContact, MODULE, "FirstName", name.at("given_name").as_string().c_str());
		db_set_utf(hContact, MODULE, "LastName", name.at("surname").as_string().c_str());
	}

	JSONNode country = root.at("country");
	if (!country.empty()) {
		std::string isocode = country.as_string();

		if (isocode.empty())
			db_unset(hContact, MODULE, "Country");
		else {
			char *szCountry = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode.c_str(), 0);
			db_set_s(hContact, MODULE, "Country", szCountry);
		}
	}

	/*JSONNode quota_info = root.at("quota_info");
	if (!quota_info.empty()) {
		ULONG lTotalQuota = quota_info.at("quota").as_int();
		ULONG lNormalQuota = quota_info.at("normal").as_int();
		ULONG lSharedQuota = quota_info.at("shared").as_int();

		db_set_dw(hContact, MODULE, "SharedQuota", lSharedQuota);
		db_set_dw(hContact, MODULE, "NormalQuota", lNormalQuota);
		db_set_dw(hContact, MODULE, "TotalQuota", lTotalQuota);

		db_set_s(hContact, "CList", "StatusMsg", CMStringA(FORMAT, Translate("Free %ld of %ld MB"), (lTotalQuota - lNormalQuota) / (1024 * 1024), lTotalQuota / (1024 * 1024)));
	}*/
}

void CDropbox::DestroyAccessToken()
{
	RevokeAccessTokenRequest request;
	NLHR_PTR response(request.Send(hNetlibConnection));

	db_unset(NULL, MODULE, "TokenSecret");
	MCONTACT hContact = CDropbox::GetDefaultContact();
	if (hContact)
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) != ID_STATUS_OFFLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE);

	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_ONLINE, (WPARAM)ID_STATUS_OFFLINE);
}

UINT CDropbox::RequestAccessTokenAsync(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	CDropbox *instance = (CDropbox*)owner;

	EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), FALSE);
	SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("in process..."));

	if (instance->HasAccessToken())
		instance->DestroyAccessToken();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, requestToken, _countof(requestToken));

	GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(instance->hNetlibConnection));

	if (response == NULL || response->resultCode != HTTP_STATUS_OK) {
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, HttpStatusToText(HTTP_STATUS_ERROR));
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("server does not respond"));
		/*else
			ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);*/
		return 0;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, HttpStatusToText((HTTP_STATUS)response->resultCode));
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("server does not respond"));
		/*else
			ShowNotification((wchar_t*)error_description, MB_ICONERROR);*/
		return 0;
	}

	JSONNode node = root.at("error_description");
	if (node != JSONNULL) {
		ptrW error_description(mir_a2u_cp(node.as_string().c_str(), CP_UTF8));
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, HttpStatusToText((HTTP_STATUS)response->resultCode));
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, error_description);
		/*else
			ShowNotification((wchar_t*)error_description, MB_ICONERROR);*/
		return 0;
	}

	node = root.at("access_token");
	db_set_s(NULL, MODULE, "TokenSecret", node.as_string().c_str());
	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, (WPARAM)ID_STATUS_ONLINE);

	MCONTACT hContact = instance->GetDefaultContact();
	if (hContact) {
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) != ID_STATUS_ONLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
	}

	try {
		RequestAccountInfo(instance);
	}
	catch (DropboxException &ex) {
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, ex.what());
		return 0;
	}

	if (hwndDlg)
		SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you have been authorized"));
	/*else
		ShowNotification(TranslateT("you have been authorized"), MB_ICONINFORMATION);*/

	SetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, "");

	return 0;
}
