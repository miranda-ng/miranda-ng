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
			SEBinary guid;
			transfer->GetPropChatmsgGuid(guid);

			Transfer::FAILUREREASON reason;

			MessageRef msgRef;
			this->GetMessageByGuid(guid, msgRef);
			
			uint oid = msgRef->getOID();

			SEString data;
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(ptrW(::mir_utf8decodeW(data)));

			Transfer::STATUS status;
			transfer->GetPropStatus(status);
			switch(status)
			{
			/*case CTransfer::NEW:
				break;*/
			/*case CTransfer::WAITING_FOR_ACCEPT:
				break;*/
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

			SEBinary guid;			
			transfer->GetPropChatmsgGuid(guid);

			MessageRef msgRef;
			this->GetMessageByGuid(guid, msgRef);
			
			uint oid = msgRef->getOID();			

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
	Transfer::TYPE transferType;
	Transfer::STATUS transferStatus;

	TransferRefs transfers;
	message->GetTransfers(transfers);
	for (size_t i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];

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
				this->transferList.append(transfer);
				transfer.fetch();

				uint timestamp;
				message->GetPropTimestamp(timestamp);

				SEString data;
				transfer->GetPropPartnerHandle(data);
				HANDLE hContact = this->GetContactBySid(ptrW(::mir_utf8decodeW(data)));

				transfer->GetPropFilename(data);
				wchar_t *fileName = ::mir_utf8decodeW(data);

				PROTORECVFILET pre = {0};
				pre.flags = PREF_TCHAR;
				pre.fileCount = 1;
				pre.timestamp = timestamp;
				pre.tszDescription = L"";
				pre.ptszFiles = &fileName;
				pre.lParam = (LPARAM)transfer->getOID();
				::ProtoChainRecvFile(hContact, &pre);
				::mir_free(fileName);
			}
			else if (transferType == Transfer::PLACEHOLDER)
			{
				this->transferList.append(transfer);
				transfer.fetch();
			}
		}
	}
}