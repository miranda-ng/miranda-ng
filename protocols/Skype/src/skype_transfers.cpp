#include "skype_proto.h"

void CSkypeProto::OnTransferChanged(CTransfer::Ref transfer, int prop)
{
	switch (prop)
	{
	case Transfer::P_STATUS:
		{
			SEBinary guid;
			transfer->GetPropChatmsgGuid(guid);

			CMessage::Ref message;
			this->GetMessageByGuid(guid, message);

			uint oid = message->getOID();

			SEString data;
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(mir_ptr<wchar_t>(::mir_utf8decodeW(data)));

			Transfer::STATUS status;
			transfer->GetPropStatus(status);
			switch(status)
			{
			/*case CTransfer::NEW:
				break;*/
			/*case CTransfer::WAITING_FOR_ACCEPT:
				break;*/
			case CTransfer::CONNECTING:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)oid, 0);
				break;
			case CTransfer::TRANSFERRING:
			case CTransfer::TRANSFERRING_OVER_RELAY:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)oid, 0);
				break;
			case CTransfer::FAILED:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oid, 0);
				this->transferList.remove_val(transfer);
				break;
			case CTransfer::COMPLETED:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)oid, 0);
				this->transferList.remove_val(transfer);
				break;
			case CTransfer::CANCELLED:
			case CTransfer::CANCELLED_BY_REMOTE:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)oid, 0);
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

			CMessage::Ref message;
			this->GetMessageByGuid(guid, message);

			uint oid = message->getOID();				

			PROTOFILETRANSFERSTATUS pfts = {0};
			pfts.cbSize = sizeof(pfts);
			pfts.flags = PFTS_UTF | PFTS_RECEIVING;
			pfts.totalFiles = 1;
			pfts.currentFileNumber = 0;
			
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(mir_ptr<wchar_t>(::mir_utf8decodeW(data)));
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
	CTransfer::Refs transfers;
	message->GetTransfers(transfers);
	//Sid::fetch(transferList);

	Transfer::TYPE transferType;
	Transfer::STATUS transferStatus;

	for (uint i = 0; i < transfers.size(); i++)
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
				transfer.fetch();
				transfer->SetOnTransferCallback(
					(CTransfer::OnTransfer)&CSkypeProto::OnTransferChanged,
					this);
				this->transferList.append(transfer);

				uint timestamp;
				message->GetPropTimestamp(timestamp);

				SEString data;
				transfer->GetPropPartnerHandle(data);
				HANDLE hContact = this->GetContactBySid(mir_ptr<wchar_t>(::mir_utf8decodeW(data)));

				transfer->GetPropFilename(data);
				wchar_t *path = ::mir_utf8decodeW(data);

				PROTORECVFILET pre = {0};
				pre.flags = PREF_TCHAR;
				pre.fileCount = 1;
				pre.timestamp = timestamp;
				pre.tszDescription = L" ";
				pre.ptszFiles =  &path;
				pre.lParam = (LPARAM)message->getOID();
				::ProtoChainRecvFile(hContact, &pre);
			}
		}
	}
}