#include "common.h"

void CDropbox::CommandContent(void *arg)
{
	MessageParam *param = (MessageParam*)arg;

	char *name = (char*)param->data;

	CMStringA url = DROPBOX_API_URL "/metadata/" DROPBOX_API_ROOT;
	if (name)
		url.AppendFormat("/%s", name);

	HttpRequest *request = new HttpRequest(INSTANCE->hNetlibUser, REQUEST_GET, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	if (response && response->resultCode == HTTP_STATUS::OK)
	{
		CMStringA message;

		JSONNODE *root = json_parse(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "is_dir");
			bool isDir = json_as_bool(node);
			if (!isDir)
				message.AppendFormat("\"%s\" %s", name, Translate("is file"));
			else
			{
				JSONNODE *content = json_as_array(json_get(root, "contents"));
				for (int i = 0;; i++)
				{
					JSONNODE *item = json_at(content, i);
					if (item == NULL)
					{
						if (i == 0)
							message.AppendFormat("\"%s\" %s", name, Translate("is empty"));
						break;
					}

					ptrA subName(mir_u2a(json_as_string(json_get(item, "path"))));
					message.AppendFormat("%s\n", (subName[0] == '/') ? &subName[1] : subName);
				}
			}

			ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
			CallContactService(INSTANCE->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());

			return;
		}
	}

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
}

void CDropbox::CommandShare(void *arg)
{
	MessageParam *param = (MessageParam*)arg;

	char *name = (char*)param->data;

	CMStringA url = DROPBOX_API_URL "/shares/" DROPBOX_API_ROOT;
	if (name)
		url.AppendFormat("/%s", name);

	HttpRequest *request = new HttpRequest(INSTANCE->hNetlibUser, REQUEST_POST, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	if (response && response->resultCode == HTTP_STATUS::OK)
	{
		CMStringA link;

		JSONNODE *root = json_parse(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "url");
			link = mir_u2a(json_as_string(node));
			ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
			CallContactService(INSTANCE->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)link.GetBuffer());

			return;
		}
	}

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
}

void CDropbox::CommandDelete(void *arg)
{
	MessageParam *param = (MessageParam*)arg;

	char *name = (char*)param->data;

	CMStringA pparam = CMStringA("root=" DROPBOX_API_ROOT "&path=") + name;

	HttpRequest *request = new HttpRequest(INSTANCE->hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/fileops/delete");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(pparam);
	request->dataLength = pparam.GetLength();

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	if (response && response->resultCode == HTTP_STATUS::OK)
	{
		JSONNODE *root = json_parse(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "is_deleted");
			bool isDeleted = json_as_bool(node);
			CMStringA message;
			message.AppendFormat("%s %s", name, !isDeleted ? Translate("is not deleted") : Translate("is deleted"));
			ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hProcess, 0);
			CallContactService(INSTANCE->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)message.GetBuffer());

			return;
		}
	}

	ProtoBroadcastAck(MODULE, param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hProcess, 0);
}