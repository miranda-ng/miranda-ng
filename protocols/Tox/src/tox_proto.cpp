#include "common.h"

#define FRADDR_TOSTR_CHUNK_LEN 8 

static void fraddr_to_str(uint8_t *id_bin, char *id_str)
{
	uint32_t i, delta = 0, pos_extra, sum_extra = 0;

	for (i = 0; i < TOX_FRIEND_ADDRESS_SIZE; i++) {
		sprintf(&id_str[2 * i + delta], "%02hhX", id_bin[i]);

		if ((i + 1) == TOX_CLIENT_ID_SIZE)
			pos_extra = 2 * (i + 1) + delta;

		if (i >= TOX_CLIENT_ID_SIZE)
			sum_extra |= id_bin[i];

		if (!((i + 1) % FRADDR_TOSTR_CHUNK_LEN)) {
			id_str[2 * (i + 1) + delta] = ' ';
			delta++;
		}
	}

	id_str[2 * i + delta] = 0;

	if (!sum_extra)
		id_str[pos_extra] = 0;
} 

void get_id(Tox *m, char *data)
{
	int offset = strlen(data);
	uint8_t address[TOX_FRIEND_ADDRESS_SIZE];
	tox_get_address(m, address);
	fraddr_to_str(address, data + offset);
}

CToxProto::CToxProto(const char* protoName, const TCHAR* userName) :
	PROTO<CToxProto>(protoName, userName)
{
	tox = tox_new(1);

	tox_callback_friend_request(tox, OnFriendRequest, this);
	tox_callback_friend_message(tox, OnFriendMessage, this);
	tox_callback_friend_action(tox, OnAction, this);
	tox_callback_name_change(tox, OnFriendNameChange, this);
	tox_callback_status_message(tox, OnStatusMessageChanged, this);
	tox_callback_user_status(tox, OnUserStatusChanged, this);
	tox_callback_connection_status(tox, OnConnectionStatusChanged, this);

	char idstring[200] = { 0 };
	get_id(tox, idstring);

	CreateProtoService(PS_CREATEACCMGRUI, &CToxProto::CreateAccMgrUI);
}

CToxProto::~CToxProto()
{
	tox_kill(tox);
}

MCONTACT __cdecl CToxProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	MCONTACT hContact = AddContact(psr->id, psr->nick);

	return hContact;
}

MCONTACT __cdecl CToxProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent) { return 0; }

int __cdecl CToxProto::Authorize(HANDLE hDbEvent) { return 0; }
int __cdecl CToxProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason) { return 0; }
int __cdecl CToxProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT*) { return 0; }
int __cdecl CToxProto::AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage) { return 0; }

HANDLE __cdecl CToxProto::ChangeInfo(int iInfoType, void* pInfoData) { return 0; }

HANDLE __cdecl CToxProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath) { return 0; }
int __cdecl CToxProto::FileCancel(MCONTACT hContact, HANDLE hTransfer) { return 0; }
int __cdecl CToxProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason) { return 0; }
int __cdecl CToxProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename) { return 0; }

DWORD_PTR __cdecl CToxProto::GetCaps(int type, MCONTACT hContact)
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;
	case PFLAGNUM_4:
		return PF4_SUPPORTTYPING;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"User Id";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"UserId";
	case PFLAG_MAXLENOFMESSAGE:
		return TOX_MAX_MESSAGE_LENGTH;
	}

	return 0;
}
int __cdecl CToxProto::GetInfo(MCONTACT hContact, int infoType) { return 0; }

HANDLE __cdecl CToxProto::SearchBasic(const PROTOCHAR* id)
{
	//if ( !IsOnline()) return 0;

	this->ForkThread(&CToxProto::SearchByUidAsync, (void*)id);

	return (HANDLE)TOX_SEARCH_BYUID;
}

HANDLE __cdecl CToxProto::SearchByEmail(const PROTOCHAR* email) { return 0; }
HANDLE __cdecl CToxProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName) { return 0; }
HWND __cdecl CToxProto::SearchAdvanced(HWND owner) { return 0; }
HWND __cdecl CToxProto::CreateExtendedSearchUI(HWND owner) { return 0; }

int __cdecl CToxProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT*) { return 0; }
int __cdecl CToxProto::RecvFile(MCONTACT hContact, PROTOFILEEVENT*) { return 0; }
int __cdecl CToxProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT*) { return 0; }
int __cdecl CToxProto::RecvUrl(MCONTACT hContact, PROTORECVEVENT*) { return 0; }

int __cdecl CToxProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT* hContactsList) { return 0; }
HANDLE __cdecl CToxProto::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles) { return 0; }
int __cdecl CToxProto::SendMsg(MCONTACT hContact, int flags, const char* msg) { return 0; }
int __cdecl CToxProto::SendUrl(MCONTACT hContact, int flags, const char* url) { return 0; }

int __cdecl CToxProto::SetApparentMode(MCONTACT hContact, int mode) { return 0; }

int __cdecl CToxProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == this->m_iDesiredStatus)
		return 0;

	int old_status = this->m_iStatus;
	this->m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		// logout
		isTerminated = true;
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		return 0;
	}
	else
	{
		if (old_status == ID_STATUS_OFFLINE/* && !this->IsOnline()*/)
		{
			m_iStatus = ID_STATUS_CONNECTING;

			// login
			isTerminated = false;
			char *name = "my_nickname";
			int res = tox_set_name(tox, (uint8_t*)name, strlen(name));

			do_bootstrap(tox);

			time_t timestamp0 = time(NULL);
			int on = 0;

			while (1) {
				tox_do(tox);

				if (on == 0) {
					if (tox_isconnected(tox)) {
						on = 1;
					}
					else {
						time_t timestamp1 = time(NULL);

						if (timestamp0 + 10 < timestamp1) {
							timestamp0 = timestamp1;
							do_bootstrap(tox);
						}
					}
				}
			}

			res = tox_isconnected(tox);
			if (!res)
			{
				SetStatus(ID_STATUS_OFFLINE);
				return 0;
			}

			poolingThread = ForkThreadEx(&CToxProto::PollingThread, 0, NULL);
		}
		else
		{
			// set tox status
			TOX_USERSTATUS userstatus;
			switch (iNewStatus)
			{
			case ID_STATUS_ONLINE:
				userstatus = TOX_USERSTATUS_NONE;
				break;
			case ID_STATUS_AWAY:
				userstatus = TOX_USERSTATUS_AWAY;
				break;
			case ID_STATUS_OCCUPIED:
				userstatus = TOX_USERSTATUS_BUSY;
				break;
			default:
				userstatus = TOX_USERSTATUS_INVALID;
				break;
			}
			tox_set_user_status(tox, userstatus);

			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

			return 0;
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	return 0;
}

HANDLE __cdecl CToxProto::GetAwayMsg(MCONTACT hContact) { return 0; }
int __cdecl CToxProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt) { return 0; }
int __cdecl CToxProto::SetAwayMsg(int iStatus, const PROTOCHAR* msg) { return 0; }

int __cdecl CToxProto::UserIsTyping(MCONTACT hContact, int type) { return 0; }

int __cdecl CToxProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam) { return 0; }