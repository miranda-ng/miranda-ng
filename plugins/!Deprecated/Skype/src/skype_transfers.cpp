#include "skype.h"

wchar_t *CSkypeProto::TransferFailureReasons[] =
{
	LPGENW("")															/* ---							*/,
	LPGENW("SENDER_NOT_AUTHORIZED")										/* SENDER_NOT_AUTHORISED		*/,
	LPGENW("REMOTELY_CANCELED")											/* REMOTELY_CANCELLED			*/,
	LPGENW("FAILED_READ")												/* FAILED_READ					*/,
	LPGENW("FAILED_REMOTE_READ")										/* FAILED_REMOTE_READ			*/,
	LPGENW("FAILED_WRITE")												/* FAILED_WRITE					*/,
	LPGENW("FAILED_REMOTE_WRITE")										/* FAILED_REMOTE_WRITE			*/,
	LPGENW("REMOTE_DOES_NOT_SUPPORT_FT")								/* REMOTE_DOES_NOT_SUPPORT_FT	*/,
	LPGENW("REMOTE_OFFLINE_FOR_TOO_LONG")								/* REMOTE_OFFLINE_FOR_TOO_LONG	*/,
	LPGENW("TOO_MANY_PARALLEL")											/* TOO_MANY_PARALLEL			*/,
	LPGENW("PLACEHOLDER_TIMEOUT")										/* PLACEHOLDER_TIMEOUT			*/
};

void CSkypeProto::OnTransferChanged(const TransferRef &transfer, int prop)
{
	switch (prop)
	{
	case Transfer::P_STATUS:
		{
			Transfer::FAILUREREASON reason;

			SEBinary guid;
			transfer->GetPropChatmsgGuid(guid);			

			MessageRef msgRef;
			this->GetMessageByGuid(guid, msgRef);

			uint oid = msgRef->getOID();

			uint fOid = transfer->getOID();

			Transfer::STATUS status;
			transfer->GetPropStatus(status);
			switch(status)
			{
			case Transfer::CONNECTING:
				this->SendBroadcast(this->transferts[oid].pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)oid, 0);
				break;
			case Transfer::TRANSFERRING:
			case Transfer::TRANSFERRING_OVER_RELAY:
				this->SendBroadcast(this->transferts[oid].pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)oid, 0);
				break;
			case Transfer::FAILED:				
				transfer->GetPropFailurereason(reason);
				this->debugLogW(L"File transfer failed: %s", CSkypeProto::TransferFailureReasons[reason]);
				this->transferList.remove_val(transfer);
				this->transferts[oid].files[fOid].isCanceled = true;
				break;
			case Transfer::COMPLETED:
				this->transferList.remove_val(transfer);
				this->transferts[oid].files[fOid].isCompleted = true;
				this->transferts[oid].pfts.totalProgress += this->transferts[oid].files[fOid].size - this->transferts[oid].files[fOid].transfered;
				break;
			case Transfer::CANCELLED:
			case Transfer::CANCELLED_BY_REMOTE:
				transfer->GetPropFailurereason(reason);
				this->debugLogW(L"File transfer cancelled: %s", CSkypeProto::TransferFailureReasons[reason]);
				this->transferList.remove_val(transfer);
				this->transferts[oid].files[fOid].isCanceled = true;
				break;
			}

			int isNotAll = false;
			for (auto i = this->transferts[oid].files.begin(); i != this->transferts[oid].files.end(); i++)
				if ( !(i->second.isCanceled || i->second.isCompleted))
				{
					isNotAll = true;
					break;
				}
			if (isNotAll)
			{
				SEString data;
				uint fOid = transfer->getOID();
				transfer->GetPropBytestransferred(data);
				Sid::uint64 tb = data.toUInt64();
				this->transferts[oid].pfts.totalProgress += tb;
				this->transferts[oid].files[fOid].transfered = tb;

				this->SendBroadcast(this->transferts[oid].pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)oid, (LPARAM)&this->transferts[oid].pfts);
			}
			else
			{
				this->SendBroadcast(this->transferts[oid].pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)oid, 0);
				delete [] this->transferts[oid].pfts.ptszFiles;
				this->transferts.erase(this->transferts.find(oid));
			}			
		}
		break;

	case Transfer::P_BYTESTRANSFERRED:
		{
			SEString data;

			SEBinary guid;
			transfer->GetPropChatmsgGuid(guid);			

			MessageRef msgRef;
			this->GetMessageByGuid(guid, msgRef);

			uint oid = msgRef->getOID();

			uint fOid = transfer->getOID();
			transfer->GetPropBytestransferred(data);
			Sid::uint64 tb = data.toUInt64();
			this->transferts[oid].pfts.totalProgress += tb;
			this->transferts[oid].files[fOid].transfered = tb;

			this->SendBroadcast(this->transferts[oid].pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)oid, (LPARAM)&this->transferts[oid].pfts);
		}
		break;
	}
}

void CSkypeProto::OnFileEvent(const ConversationRef &conversation, const MessageRef &message)
{
	SEString data;
	Transfer::TYPE transferType;
	Transfer::STATUS transferStatus;

	ContactRef author;
	message->GetPropAuthor(data);
	this->GetContact(data, author);

	TransferRefs transfers;
	message->GetTransfers(transfers);

	FileTransferParam ftp;
	ftp.pfts.flags = PFTS_RECEIVING | PFTS_UNICODE;
	ftp.pfts.hContact = this->AddContact(author, true);
	ftp.pfts.totalFiles = (int)transfers.size();
	ftp.pfts.ptszFiles = new wchar_t*[transfers.size() + 1];

	int nifc = 0;
	for (size_t i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];

		// For incomings, we need to check for transfer status, just to be sure.
		// In some cases, a transfer can appear with STATUS == PLACEHOLDER
		// As such transfers cannot be accepted, we will need to just store
		// the reference to Transfer Object and then check for further
		// status changes in Transfer::OnChange
		transfer->GetPropStatus(transferStatus);
		if (transferStatus == Transfer::NEW || transferStatus == Transfer::PLACEHOLDER)
		{
			transfer->GetPropType(transferType);
			if (transferType == Transfer::INCOMING)
			{
				transfer->GetPropFilename(data);
				ftp.pfts.ptszFiles[nifc++] = ::mir_utf8decodeW(data);

				transfer.fetch();
				this->transferList.append(transfer);
				//transfer.fetch();

				transfer->GetPropFilesize(data);
				Sid::uint64 size = data.toUInt64();

				ftp.files.insert(std::make_pair(transfer->getOID(), FileParam(size)));
				ftp.pfts.totalBytes += size;		
			}
		}
	}

	if (nifc > 0)
	{
		uint timestamp;
		message->GetPropTimestamp(timestamp);

		auto oid = message->getOID();
		
		this->transferts.insert(std::make_pair(oid, ftp));

		PROTORECVFILET pre = {0};
		pre.flags = PREF_TCHAR;
		pre.fileCount = ftp.pfts.totalFiles;
		pre.timestamp = timestamp;
		pre.tszDescription = L"";
		pre.ptszFiles = ftp.pfts.ptszFiles;
		pre.lParam = (LPARAM)oid;
		::ProtoChainRecvFile(ftp.pfts.hContact, &pre);
	}
	else
	{
		delete [] ftp.pfts.ptszFiles;
	}
}