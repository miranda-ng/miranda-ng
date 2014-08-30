#include "common.h"

void CToxProto::SearchByIdAsync(void*)
{
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HWND)1, 0);
}

void CToxProto::SearchByNameAsync(void* arg)
{
	NETLIBHTTPREQUEST request = { sizeof(NETLIBHTTPREQUEST) };
	request.requestType = REQUEST_POST;
	request.szUrl = "https://toxme.se/api";
	request.flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

	request.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)* 2);
	request.headers[0].szName = "Content-Type";
	request.headers[0].szValue = "text/plain; charset=utf-8";
	request.headersCount = 1;

	std::string query = "{\"action\":3,\"name\":\"";
	query += (char*)arg;
	query += "\"}";

	request.dataLength = query.length();
	request.pData = (char*)query.c_str();

	NETLIBHTTPREQUEST* response = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlib, (LPARAM)&request);

	if (response)
	{
		std::smatch match;
		std::regex regex("\"public_key\": \"(.+?)\"");

		const std::string content = response->pData;

		if (std::regex_search(content, match, regex))
		{
			std::string toxId = match[1];

			PROTOSEARCHRESULT psr = { sizeof(PROTOSEARCHRESULT) };
			psr.flags = PSR_TCHAR;
			psr.id = mir_a2t(toxId.c_str());

			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
			return;
		}
	}

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	mir_free(request.headers);

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1, 0);
	mir_free(arg);
}

INT_PTR CToxProto::SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
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