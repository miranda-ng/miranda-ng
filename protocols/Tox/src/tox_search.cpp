#include "stdafx.h"

ToxHexAddress ResolveToxAddressFromToxme(HNETLIBUSER hNetlib, const char *query)
{
	ToxHexAddress address = ToxHexAddress::Empty();

	HttpRequest request(REQUEST_POST, "https://toxme.io/api");
	JSONNode root(JSON_NODE);
	root
		<< JSONNode("action", 3)
		<< JSONNode("name", query);
	json_string data = root.write();
	request.SetData(data.c_str(), data.length());

	NLHR_PTR response(request.Send(hNetlib));
	if (response == nullptr)
		return ToxHexAddress::Empty();

	if (response->resultCode != HTTP_CODE_OK || !response->pData)
		return ToxHexAddress::Empty();

	root = JSONNode::parse(response->pData);
	if (root.empty())
		return ToxHexAddress::Empty();

	json_string id = root.at("tox_id").as_string();
	return ToxHexAddress(id.c_str());
}

void CToxProto::SearchByNameAsync(void *arg)
{
	Thread_SetName(MODULE ": SearchByNameThread");

	char *query = (char*)arg;
	char *name = strtok(query, "@");

	ToxHexAddress address = ResolveToxAddressFromToxme(m_hNetlibUser, query);
	if (!address.IsEmpty()) {
		PROTOSEARCHRESULT psr = { sizeof(PROTOSEARCHRESULT) };
		psr.flags = PSR_UTF8;
		psr.id.a = mir_strdup(address);
		psr.nick.a = mir_strdup(name);

		char email[MAX_PATH];
		mir_snprintf(email, "%s@toxme.io", name);
		psr.email.a = mir_strdup(email);

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	mir_free(arg);
}

INT_PTR CToxProto::SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			//ShowWindow(GetDlgItem(GetParent(hwnd), 1408/*IDC_BYCUSTOM*/), SW_HIDE);
			//ShowWindow(GetDlgItem(GetParent(hwnd), 1402/*IDC_ADVANCEDGROUP*/), SW_HIDE);
			SetDlgItemText(GetParent(hwnd), 1408/*IDC_BYCUSTOM*/, TranslateT("Query"));
		}
		return TRUE;
	}

	return FALSE;
}

HWND CToxProto::OnSearchAdvanced(HWND owner)
{
	if (!IsOnline()) {
		// we cannot add someone to friend list while tox is offline
		return nullptr;
	}

	std::smatch match;
	std::regex regex("^\\s*([A-Fa-f0-9]{76})\\s*$");

	wchar_t text[MAX_PATH];
	GetDlgItemText(owner, IDC_SEARCH, text, _countof(text));

	const std::string query = T2Utf(text).str();
	if (std::regex_search(query, match, regex)) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_UTF8;
		psr.id.a = mir_strdup(query.c_str());
		Contact::AddBySearch(m_szModuleName, &psr, owner);

		ProtoBroadcastAsync(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1);
	}
	else {
		regex = "^\\s*(([^ @/:;()\"']+)(@[A-Za-z]+.[A-Za-z]{2,6})?)\\s*$";
		if (std::regex_search(query, match, regex))
			ForkThread(&CToxProto::SearchByNameAsync, mir_strdup(query.c_str()));
		else
			ProtoBroadcastAsync(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1);
	}
	return (HWND)1;
}

HWND CToxProto::OnCreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEARCH), owner, SearchDlgProc, (LPARAM)this);
}
