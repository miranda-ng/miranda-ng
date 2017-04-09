#include "stdafx.h"

void CDropbox::CommandHelp(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	CMStringA help = (char*)T2Utf(TranslateT("Dropbox supports the following commands:"));
	help += "\n";
	help += "\"/list [path]\" \t- "; help += T2Utf(TranslateT("shows all files in folder \"path\" (\"path\" is relative from root and can be omitted for root folder)"));
	help += "\n";
	help += "\"/share <path>\" \t- "; help += T2Utf(TranslateT("returns download link for file or folder with specified path (\"path\" is relative from root folder)"));
	help += "\n";
	help += "\"/search <query>\" \t- "; help += T2Utf(TranslateT("searches for file or folder matched by query (\"query\" is split on spaces into multiple tokens)"));
	help += "\n";
	help += "\"/delete <path>\" \t- "; help += T2Utf(TranslateT("deletes file or folder with specified path (\"path\" is relative from root folder)"));

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help.GetBuffer());
}

void CDropbox::CommandList(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	char path[MAX_PATH];
	PreparePath((char*)param->data, path);
	if (path[0] == NULL) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/list");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());
		return;
	}

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ListFolderRequest request(token, path);
	NLHR_PTR response(request.Send(param->instance->hNetlibConnection));

	if (response == NULL || response->resultCode != HTTP_STATUS_OK) {
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
		return;
	}

	CMStringA message;
	JSONNode entries = root.at("entries").as_array();
	if (entries.empty())
		message.AppendFormat("\"%s\" %s", path, T2Utf(TranslateT("is empty")));
	else
	{
		for (size_t i = 0; i < entries.size(); i++) {
			JSONNode entry = entries[i];
			CMStringA subName(entry.at("path_lower").as_string().c_str());
			message.Append((subName[0] == '/') ? subName.Mid(1) : subName);
			message.AppendChar('\n');
		}
	}

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());
}

void CDropbox::CommandShare(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	char path[MAX_PATH];
	PreparePath((char*)param->data, path);
	if (path[0] == NULL) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/share");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());
		return;
	}

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	GetTemporaryLinkRequest request(token, path);
	NLHR_PTR response(request.Send(param->instance->hNetlibConnection));

	if (response == NULL || response->resultCode != HTTP_STATUS_OK) {
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
		return;
	}

	CMStringA link = root.at("link").as_string().c_str();
	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)link.GetBuffer());
}

void CDropbox::CommandSearch(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	char *query = (char*)param->data;
	if (query == NULL) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/search");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());
		return;
	}

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	//bool useShortUrl = db_get_b(NULL, MODULE, "UseSortLinks", 1) > 0;
	SearchRequest request(token, query);
	NLHR_PTR response(request.Send(param->instance->hNetlibConnection));

	if (response == NULL || response->resultCode != HTTP_STATUS_OK) {
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
		return;
	}

	CMStringA message;
	JSONNode matches = root.at("matches").as_array();
	if (matches.empty())
		message.AppendFormat("\"%s\" %s", query, T2Utf(TranslateT("is not found")));
	else
	{
		for (size_t i = 0; i < matches.size(); i++) {
			JSONNode metadata = matches[i].at("metadata").as_node();
			message.AppendFormat("%s\n", metadata.at("path_lower").as_string().c_str());
		}
	}

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());
}

void CDropbox::CommandDelete(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	char path[MAX_PATH];
	PreparePath((char*)param->data, path);
	if (path[0] == NULL) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/delete");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());
		return;
	}

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	DeleteRequest request(token, path);
	NLHR_PTR response(request.Send(param->instance->hNetlibConnection));

	try
	{
		HandleJsonResponse(response);

		CMStringA message(FORMAT, "%s %s", path, T2Utf(TranslateT("is deleted")));
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		ProtoChainSend(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());
	}
	catch (DropboxException &ex)
	{
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, (LPARAM)ex.what());
		return;
	}
	
	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
}
