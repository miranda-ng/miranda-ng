#include "common.h"

int CToxProto::FileSendQueueCompare(const CFile* p1, const CFile* p2)
{
	return 0;
}

void CToxProto::SendFilesAsync(void* arg)
{
	CFileTransfer *ftp = (CFileTransfer*)arg;

	std::string toxId(getStringA(ftp->pfts.hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());

	for (int i = 0; ftp->GetFileCount(); i++)
	{
		CFile *file = ftp->GetFileAt(i);
		int hFile = tox_new_file_sender(tox, number, file->GetSize(), (uint8_t*)file->GetName(), strlen(file->GetName()));
		if (hFile < 0)
		{
			debugLogA("CToxProto::SendFilesAsync: cannot send file");
		}
		file->SetHandle((HANDLE)hFile);
	}
}