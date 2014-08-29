#include "common.h"

void CToxProto::SendFileAsync(void* arg)
{
	CFile *file = (CFile*)arg;
	const CFileTransfer *transfer = file->GetTransfer();
	CToxProto *proto = (CToxProto*)transfer->GetProtoInstance();
}

void CToxProto::SendFilesAsync(void* arg)
{
	CFileTransfer *transfer = (CFileTransfer*)arg;

	std::string toxId(getStringA(transfer->GetContactHandle(), TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());

	for (int i = 0; transfer->GetFileCount(); i++)
	{
		CFile *file = transfer->GetFileAt(i);

		int hFile = tox_new_file_sender(tox, number, file->GetSize(), (uint8_t*)file->GetName(), strlen(file->GetName()));
		if (hFile < 0)
		{
			debugLogA("CToxProto::SendFilesAsync: cannot send file");
		}
		file->SetNumber(hFile);

		transfer->Wait();
	}
}

CFileTransfer *CToxProto::GetFileTransferByFileNumber(int fileNumber)
{
	for (int i = 0; fileTransfers.getCount(); i++)
	{
		if (fileTransfers[i]->HasFile(fileNumber))
		{
			return fileTransfers[i];
		}
	}
	return NULL;
}