#include "skype_proto.h"

//LIST<FileTransfer> CSkypeProto::fileTransferList(1);

FileTransfer *CSkypeProto::FindTransfer(SEBinary guid)
{
	for (int i = 0; i < this->fileTransferList.getCount(); i++)
	{
		if (this->fileTransferList[i]->guid == guid)
		{
			return this->fileTransferList[i];
		}
	}
	
	return NULL;
}

FileTransfer *CSkypeProto::FindFileTransfer(CTransfer::Ref transfer)
{
	for (int i = 0; i < this->fileTransferList.getCount(); i++)
	{
		if (this->fileTransferList[i]->transfers.contains(transfer))
		{
			return this->fileTransferList[i];
		}
	}
	
	return NULL;
}