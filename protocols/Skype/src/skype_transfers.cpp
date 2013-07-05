#include "skype.h"

wchar_t *CSkypeProto::TransferFailureReasons[] =
{
	LPGENW("")															/* ---							*/,
	LPGENW("SENDER_NOT_AUTHORISED")										/* SENDER_NOT_AUTHORISED		*/,
    LPGENW("REMOTELY_CANCELLED")										/* REMOTELY_CANCELLED			*/,
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

			/*SEBinary guid;
			transfer->GetPropChatmsgGuid(guid);			

			MessageRef msgRef;
			this->GetMessageByGuid(guid, msgRef);
			
			uint oid = msgRef->getOID();*/

			uint oid = transfer->getOID();

			SEString data;
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(ptrW(::mir_utf8decodeW(data)));

			Transfer::STATUS status;
			transfer->GetPropStatus(status);
			switch(status)
			{
			case Transfer::CONNECTING:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)oid, 0);
				break;
			case Transfer::TRANSFERRING:
			case Transfer::TRANSFERRING_OVER_RELAY:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)oid, 0);
				break;
			case Transfer::FAILED:				
				transfer->GetPropFailurereason(reason);
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oid, (LPARAM)CSkypeProto::TransferFailureReasons[reason]);
				this->transferList.remove_val(transfer);
				break;
			case Transfer::COMPLETED:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)oid, 0);
				this->transferList.remove_val(transfer);
				break;
			case Transfer::CANCELLED:
			case Transfer::CANCELLED_BY_REMOTE:
				transfer->GetPropFailurereason(reason);
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)oid, (LPARAM)CSkypeProto::TransferFailureReasons[reason]);
				this->transferList.remove_val(transfer);
				break;
			}
		}
		break;

	case Transfer::P_BYTESTRANSFERRED:
		{
			SEString data;

			uint oid = transfer->getOID();

			PROTOFILETRANSFERSTATUS pfts = {0};
			pfts.cbSize = sizeof(pfts);
			pfts.flags = PFTS_UTF | PFTS_RECEIVING;
			pfts.totalFiles = 1;
			pfts.currentFileNumber = 0;
			
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(ptrW(::mir_utf8decodeW(data)));
			pfts.hContact = hContact;
		
			transfer->GetPropFilename(data);
			pfts.szCurrentFile = ::mir_strdup(data);

			pfts.pszFiles = &pfts.szCurrentFile;

			transfer->GetPropFilesize(data);
			pfts.totalBytes = pfts.currentFileSize = data.toUInt64();

			transfer->GetPropBytestransferred(data);
			pfts.totalProgress = pfts.currentFileProgress = data.toUInt64();

			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)oid, (LPARAM)&pfts);
		}
		break;
	}
}

void CSkypeProto::OnFileEvent(const ConversationRef &conversation, const MessageRef &message)
{
	SEString data;
	//bool isRecvFiles = false;
	Transfer::TYPE transferType;
	Transfer::STATUS transferStatus;

	TransferRefs transfers;
	message->GetTransfers(transfers);
	wchar_t **files = new wchar_t*[transfers.size()];
	for (size_t i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];

		transfer->GetPropFilename(data);
		files[i] = ::mir_utf8decodeW(data);

		// For incomings, we need to check for transfer status, just to be sure.
		// In some cases, a transfer can appear with STATUS == PLACEHOLDER
		// As such transfers cannot be accepted, we will need to just store
		// the reference to Transfer Object and then check for further
		// status changes in Transfer::OnChange
		transfer->GetPropStatus(transferStatus);
		if (transferStatus == Transfer::NEW)
		{
			transfer->GetPropType(transferType);
			if (transferType == Transfer::INCOMING)
			{
				//isRecvFiles = true;

				this->transferList.append(transfer);
				transfer.fetch();

				uint timestamp;
				message->GetPropTimestamp(timestamp);

				ContactRef author;
				message->GetPropAuthor(data);
				this->GetContact(data, author);

				HANDLE hContact = this->AddContact(author, true);

				PROTORECVFILET pre = {0};
				pre.flags = PREF_TCHAR;
				pre.fileCount = transfers.size();
				pre.timestamp = timestamp;
				pre.tszDescription = L"";
				pre.ptszFiles = files;
				pre.lParam = (LPARAM)transfer->getOID();
				::ProtoChainRecvFile(hContact, &pre);
			}
			else if (transferType == Transfer::PLACEHOLDER)
			{
				this->transferList.append(transfer);
				transfer.fetch();
			}
		}
	}
	/*files[transfers.size()] = NULL;

	if (isRecvFiles)
	{
		uint timestamp;
		message->GetPropTimestamp(timestamp);

		ContactRef author;
		message->GetPropAuthor(data);
		this->GetContact(data, author);

		HANDLE hContact = this->AddContact(author, true);

		PROTORECVFILET pre = {0};
		pre.flags = PREF_TCHAR;
		pre.fileCount = transfers.size();
		pre.timestamp = timestamp;
		pre.tszDescription = L"";
		pre.ptszFiles = files;
		pre.lParam = (LPARAM)message->getOID();
		::ProtoChainRecvFile(hContact, &pre);
	}*/
}