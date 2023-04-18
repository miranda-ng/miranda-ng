#include "stdafx.h"

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

HANDLE CToxProto::OnSearchAdvanced(HWND owner)
{
	if (!IsOnline()) {
		// we cannot add someone to friend list while tox is offline
		return nullptr;
	}

	std::smatch match;
	std::regex regex("^\\s*([A-Fa-f0-9]{76})\\s*$");

	wchar_t text[MAX_PATH];
	GetDlgItemText(owner, IDC_SEARCH, text, _countof(text));

	std::string query = T2Utf(text).str();
	query = std::regex_replace(query, std::regex("\\s"), "");
	if (std::regex_search(query, match, regex)) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_UTF8;
		psr.id.a = mir_strdup(query.c_str());
		Contact::AddBySearch(m_szModuleName, &psr, owner);

		ProtoBroadcastAsync(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, this);
	}
	else ProtoBroadcastAsync(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, this, (LPARAM)TranslateT("Invalid search string"));
	
	return (HWND)this;
}

HWND CToxProto::OnCreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEARCH), owner, SearchDlgProc, (LPARAM)this);
}
