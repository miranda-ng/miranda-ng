#include "stdafx.h"

HANDLE CSkypeProto::SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles)
{
	if (IsOnline())
	{
		CFileUploadParam *fup = new CFileUploadParam(hContact, szDescription, ppszFiles);
		ForkThread(&CSkypeProto::SendFileThread, (void*)fup);
		return (HANDLE)fup;
	}
	return INVALID_HANDLE_VALUE;
}

void CSkypeProto::SendFileThread(void *p)
{
	CFileUploadParam *fup = (CFileUploadParam *)p;
	if (!IsOnline()) {
		//SendFileFiled(fup, VKERR_OFFLINE);
		return;
	}
	if (!fup->IsAccess()) {
		//SendFileFiled(fup, VKERR_FILE_NOT_EXIST);
		return;
	}

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);

	SendRequest(new ASMObjectCreateRequest(li, CMStringA(FORMAT, "%d:%s", isChatRoom(fup->hContact) ? 19 : 8, ptrA(getStringA(fup->hContact, SKYPE_SETTINGS_ID)))), &CSkypeProto::OnASMObjectCreated, fup);
}

void CSkypeProto::OnASMObjectCreated(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response->pData)
	{
		JSONNode node = JSONNode::parse((char*)response->pData);
		std::string strObjectId = node["id"].as_string();

		FILE *pFile = _tfopen(fup->tszFileName, _T("rb"));
		if (pFile == NULL) return;

		fseek(pFile, 0, SEEK_END);
		long lFileLen = ftell(pFile);

		if(lFileLen < 1) {
			fclose(pFile);
			return;
		}

		fseek(pFile, 0, SEEK_SET);

		mir_ptr<BYTE> pData((PBYTE)mir_alloc(lFileLen));
		long lBytes = (long)fread(pData, sizeof(BYTE), lFileLen, pFile);

		if (lBytes != lFileLen) {
			mir_free(pData);
			return;
		}
		SendRequest(new ASMObjectUploadRequest(li, strObjectId.c_str(), pData, lBytes));
	}
}

void CSkypeProto::OnASMObjectUploaded(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	char *szUrl = response->szUrl;
}