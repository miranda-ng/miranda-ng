#include "common.h"

void WhatsAppProto::stayConnectedLoop(void*)
{
	ptrA cc(getStringA(WHATSAPP_KEY_CC));
	if (mir_strlen(cc) == 0) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a country code."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	ptrA in(getStringA(WHATSAPP_KEY_LOGIN));
	if (mir_strlen(in) == 0) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a phone number without country code."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	m_szPhoneNumber = std::string(cc) + std::string(in);
	m_szJid = m_szPhoneNumber + "@s.whatsapp.net";

	ptrA szNick(getStringA(WHATSAPP_KEY_NICK));
	if (mir_strlen(szNick) == 0) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a nickname."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}
	m_szNick = szNick;

	ptrA szPassword(getStringA(WHATSAPP_KEY_PASS));
	if (mir_strlen(szPassword) == 0) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a password."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	// -----------------------------
	Mutex writerMutex;
	bool error = false;

	m_pSocket = NULL;

	while (true) {
		if (m_pConnection != NULL) {
			delete m_pConnection;
			m_pConnection = NULL;
		}
		if (m_pSocket != NULL) {
			delete m_pSocket;
			m_pSocket = NULL;
		}

		if (m_iDesiredStatus == ID_STATUS_OFFLINE || error) {
			debugLogA("Set status to offline");
			SetAllContactStatuses(ID_STATUS_OFFLINE, true);
			ToggleStatusMenuItems(false);
			int prevStatus = m_iStatus;
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)prevStatus, m_iStatus);
			break;
		}

		debugLogA("Connecting...");

		try {
			unsigned passLen;
			ptrA passBin((char*)mir_base64_decode(szPassword, &passLen));
			std::string password(passBin, passLen), resource = ACCOUNT_RESOURCE;
			int portNumber;
			if (getByte(WHATSAPP_KEY_SSL, 0))
				portNumber = 443, resource += "-443";
			else
				portNumber = 5222, resource += "-5222";

			m_pSocket = new WASocketConnection("c.whatsapp.net", portNumber);
			m_pConnection = new WAConnection(m_szPhoneNumber, resource, &connMutex, &writerMutex, m_pSocket, this, this);
			{
				WALogin login(m_pConnection, password);

				m_Challenge = login.login(m_Challenge);
				m_pConnection->setLogin(&login);
			}
			m_pConnection->nick = m_szNick;
			if (m_iDesiredStatus != ID_STATUS_INVISIBLE)
				m_pConnection->sendAvailableForChat();

			debugLogA("Set status to online");
			m_iStatus = m_iDesiredStatus;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
			ToggleStatusMenuItems(true);

			ForkThread(&WhatsAppProto::ProcessBuddyList, NULL);

			// #TODO Move out of try block. Exception is expected on disconnect
			while (true) {
				m_tLastWriteTime = time(NULL);
				if (!m_pConnection->read())
					break;
			}
			debugLogA("Exit from read-loop");
		}
		catch (WAException &e) {
			debugLogA("Exception: %s", e.what());
			error = true;
		}
		catch (exception &e) {
			debugLogA("Exception: %s", e.what());
			error = true;
		}
		catch (...) {
			debugLogA("Unknown exception");
			error = true;
		}
	}
	debugLogA("Break out from loop");
}

void WhatsAppProto::sentinelLoop(void*)
{
	while (WaitForSingleObjectEx(update_loop_lock_, 1000, true) == WAIT_TIMEOUT) {
		if (m_iStatus != ID_STATUS_OFFLINE && m_pConnection != NULL && m_iDesiredStatus == m_iStatus) {
			// #TODO Quiet after pong or tree read?
			int quietInterval = difftime(time(NULL), m_tLastWriteTime);
			if (quietInterval >= MAX_SILENT_INTERVAL) {
				try {
					debugLogA("send ping");
					m_tLastWriteTime = time(NULL);
					m_pConnection->sendPing();
				}
				catch (exception &e) {
					debugLogA("Exception: %s", e.what());
				}
			}
		}
	}
	ResetEvent(update_loop_lock_);
	debugLogA("Exiting sentinel loop");
}

void WhatsAppProto::onPing(const std::string &id)
{
	if (isOnline()) {
		try {
			debugLogA("Sending pong with id %s", id.c_str());
			m_pConnection->sendPong(id);
		}
		CODE_BLOCK_CATCH_ALL
	}
}
