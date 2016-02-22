#include "stdafx.h"

void CDropbox::CommandHelp(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	CMStringA help = (char*)T2Utf(TranslateT("Dropbox supports the following commands:"));
	help += "\n";
	help += "\"/list [dir]\" \t- "; help += T2Utf(TranslateT("shows all files in folder \"path\" (\"path\" is relative from root folder)"));
	help += "\n";
	help += "\"/share <path>\" \t- "; help += T2Utf(TranslateT("returns download link for file or folder with specified path (\"path\" is relative from root folder)"));
	help += "\n";
	help += "\"/delete <path>\" \t- "; help += T2Utf(TranslateT("deletes file or folder with specified path (\"path\" is relative from root folder)"));

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help.GetBuffer());
}

void CDropbox::CommandContent(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	CMStringA path = PreparePath((char*)param->data);
	if (path.IsEmpty()) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/share");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());

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
	JSONNode content = root.at("entries").as_array();
	for (size_t i = 0; i < content.size(); i++) {
		JSONNode item = content[i];
		if (item.empty()) {
			if (i == 0)
				message.AppendFormat("\"%s\" %s", path, T2Utf(TranslateT("is empty")));
			break;
		}

		CMStringA subName(item.at("path_lower ").as_string().c_str());
		message.AppendFormat("%s\n", (subName[0] == '/') ? subName.Mid(1) : subName);
	}

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());
}

void CDropbox::CommandShare(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	CMStringA path = PreparePath((char*)param->data);
	if (path.IsEmpty()) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/share");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());

		return;
	}

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	//bool useShortUrl = db_get_b(NULL, MODULE, "UseSortLinks", 1) > 0;
	ShareRequest request(token, path);
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

	CMStringA link = root.at("url").as_string().c_str();
	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
	CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)link.GetBuffer());
}

void CDropbox::CommandDelete(void *arg)
{
	CommandParam *param = (CommandParam*)arg;

	CMStringA path = PreparePath((char*)param->data);
	if (path.IsEmpty()) {
		CMStringA error(FORMAT, T2Utf(TranslateT("\"%s\" command has invalid parameter.\nUse \"/help\" for more info.")), "/delete");
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)error.GetBuffer());

		return;
	}
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	DeleteRequest request(token, path);
	NLHR_PTR response(request.Send(param->instance->hNetlibConnection));

	try
	{
		HandleJsonResponseError(response);

		CMStringA message(FORMAT, "%s %s", path, T2Utf(TranslateT("is deleted")));
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
		CallContactService(param->instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());
	}
	catch (DropboxException &ex)
	{
		ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, (LPARAM)ex.what());
		return;
	}
	
	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
}