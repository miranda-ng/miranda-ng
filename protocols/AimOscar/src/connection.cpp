/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HNETLIBCONN CAimProto::aim_connect(const char* server, unsigned short port, bool use_ssl, const char* host)
{
	NETLIBOPENCONNECTION ncon = { 0 };
	ncon.cbSize = sizeof(ncon);
	ncon.szHost = server;
	ncon.wPort = port;
	ncon.timeout = 6;
	ncon.flags = NLOCF_V2;
	debugLogA("%s:%u", server, port);
	HNETLIBCONN con = Netlib_OpenConnection(m_hNetlibUser, &ncon);
	if (con && use_ssl) {
		if (!Netlib_StartSsl(con, host)) {
			Netlib_CloseHandle(con);
			con = NULL;
		}
	}
	return con;
}

HNETLIBCONN CAimProto::aim_peer_connect(const char* ip, unsigned short port)
{
	NETLIBOPENCONNECTION ncon = { 0 };
	ncon.cbSize = sizeof(ncon);
	ncon.flags = NLOCF_V2;
	ncon.szHost = ip;
	ncon.wPort = port;
	ncon.timeout = 3;
	return Netlib_OpenConnection(m_hNetlibPeer, &ncon);
}

HNETLIBCONN CAimProto::aim_peer_connect(unsigned long ip, unsigned short port)
{
	char ips[20];
	long_ip_to_char_ip(ip, ips);

	return aim_peer_connect(ips, port);
}

void CAimProto::aim_connection_authorization(void)
{
	if (m_iDesiredStatus != ID_STATUS_OFFLINE) {
		char *password = getStringA(AIM_KEY_PW);
		if (password != NULL) {
			mir_free(m_username);
			m_username = getStringA(AIM_KEY_SN);
			if (m_username != NULL) {
				HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(m_hServerConn, 2048 * 4);

				NETLIBPACKETRECVER packetRecv = {};
				packetRecv.dwTimeout = 5000;
				for (;;) {
					int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
					if (recvResult == 0) {
						debugLogA("Connection Closed: No Error? during Connection Authorization");
						break;
					}
					else if (recvResult < 0) {
						debugLogA("Connection Closed: Socket Error during Connection Authorization %d", WSAGetLastError());
						break;
					}
					else {
						unsigned short flap_length = 0;
						for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
							if (!packetRecv.buffer)
								break;

							FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], (unsigned short)(packetRecv.bytesAvailable - packetRecv.bytesUsed));
							if (!flap.len())
								break;

							flap_length += FLAP_SIZE + flap.len();
							if (flap.cmp(0x01)) {
								if (aim_send_connection_packet(m_hServerConn, m_seqno, flap.val()) == 0) // cookie challenge
									aim_authkey_request(m_hServerConn, m_seqno); // md5 authkey request
							}
							else if (flap.cmp(0x02)) {
								SNAC snac(flap.val(), flap.snaclen());
								if (snac.cmp(0x0017)) {
									snac_md5_authkey(snac, m_hServerConn, m_seqno, m_username, password);
									int authres = snac_authorization_reply(snac);
									switch (authres) {
									case 1:
										mir_free(password);
										Netlib_CloseHandle(hServerPacketRecver);
										debugLogA("Connection Authorization Thread Ending: Negotiation Beginning");
										return;

									case 2:
										ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
										goto exit;

									case 3:
										ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
										goto exit;
									}
								}
							}
							else if (flap.cmp(0x04)) {
								debugLogA("Connection Authorization Thread Ending: Flap 0x04");
								goto exit;
							}
						}
					}
				}
			exit:
				if (hServerPacketRecver)
					Netlib_CloseHandle(hServerPacketRecver);
			}
		}
		mir_free(password);
	}
	if (m_iStatus != ID_STATUS_OFFLINE)
		broadcast_status(ID_STATUS_OFFLINE);
	Netlib_CloseHandle(m_hServerConn);
	m_hServerConn = NULL;
	debugLogA("Connection Authorization Thread Ending: End of Thread");
}

bool parse_clientlogin_response(NETLIBHTTPREQUEST *nlhr, NETLIBHTTPHEADER *my_headers, CMStringA &token, CMStringA &secret, time_t &hosttime)
{
	//TODO: validate response
	//TODO: return false on errors
	//TODO: extract token, secret, hosttime from response

	int datalen = 0;
	for (int i = 0; i < nlhr->headersCount; i++) {
		if (!mir_strcmp(nlhr->headers[i].szName, "Set-Cookie")) {
			my_headers[0].szName = "Cookie";
			my_headers[0].szValue = mir_strdup(nlhr->headers[i].szValue);
		}
	}

	ptrW buf_w(mir_utf8decodeW(nlhr->pData));
	HXML root = xmlParseString(buf_w, &datalen, L"");
	if (!root)
		return false;

	HXML status = xmlGetChildByPath(root, L"response/statusCode", 0);
	if (!status)
		return false;

	LPCTSTR status_text = xmlGetText(status);
	// TODO: check other than 200 codes and print some debug info on errors
	if (wcscmp(status_text, L"200"))
		return false;

	HXML secret_node = xmlGetChildByPath(root, L"response/data/sessionSecret", 0);
	HXML hosttime_node = xmlGetChildByPath(root, L"response/data/hostTime", 0);
	if (!secret_node || !hosttime_node)
		return false;

	HXML token_node = xmlGetChildByPath(root, L"response/data/token/a", 0);
	if (!token_node)
		return false;

	LPCTSTR secret_w = xmlGetText(secret_node), token_w = xmlGetText(token_node), hosttime_w = xmlGetText(hosttime_node);
	if (!secret_w || !token_w || !hosttime_w)
		return false;

	secret = _T2A(secret_w);
	token = _T2A(token_w);
	hosttime = strtol(_T2A(hosttime_w), NULL, 10);
	return true;
}

void generate_signature(BYTE *signature, const char *method, const char *url, const char *parameters, char *session_key)
{
	CMStringA sig_base(FORMAT, "%s&%s&%s", method, ptrA(mir_urlEncode(url)), ptrA(mir_urlEncode(parameters)));
	mir_hmac_sha256(signature, (BYTE*)session_key, mir_strlen(session_key), (BYTE*)sig_base.GetString(), sig_base.GetLength());
}

void fill_session_url(CMStringA &buf, CMStringA &token, CMStringA &secret, time_t &hosttime, const char *password, bool encryption)
{
	/*
		AIM_SESSION_URL?query_string?sig_sha256=signature
		*/

	CMStringA query_string;
	query_string.Format("a=%s&distId=%s&f=xml&k=%s&ts=%llu&useTLS=%d", token.c_str(), AIM_DEFAULT_DISTID, AIM_DEFAULT_CLIENT_KEY, hosttime, (int)encryption);

	BYTE session_key[MIR_SHA256_HASH_SIZE], signature[MIR_SHA256_HASH_SIZE];
	mir_hmac_sha256(session_key, (BYTE*)password, mir_strlen(password), (BYTE*)secret.GetString(), secret.GetLength());

	ptrA szKey(mir_base64_encode(session_key, sizeof(session_key)));

	generate_signature(signature, "GET", AIM_SESSION_URL, query_string, szKey);

	ptrA szEncoded(mir_base64_encode(signature, sizeof(signature)));
	buf.Format("%s?%s&sig_sha256=%s", AIM_SESSION_URL, query_string.c_str(), (char*)szEncoded);
}

bool parse_start_socar_session_response(const char *response, CMStringA &bos_host, unsigned short &bos_port, CMStringA &cookie, CMStringA &tls_cert_name, bool encryption = true)
{
	//TODO: extract bos_host, bos_port, cookie, tls_cert_name

	int datalen = 0;
	ptrW buf_w(mir_utf8decodeW(response));
	HXML root = xmlParseString(buf_w, &datalen, L"");
	if (!root)
		return false;

	HXML status = xmlGetChildByPath(root, L"response/statusCode", 0);
	if (!status)
		return false;

	LPCTSTR status_text = xmlGetText(status);
	//TODO: check other than 200 codes and print some debug info on errors
	if (wcscmp(status_text, L"200"))
		return false;

	HXML host_node = xmlGetChildByPath(root, L"response/data/host", 0);
	HXML port_node = xmlGetChildByPath(root, L"response/data/port", 0);
	HXML cookie_node = xmlGetChildByPath(root, L"response/data/cookie", 0);
	if (!host_node || !port_node || !cookie_node)
		return false;

	LPCTSTR host_w = xmlGetText(host_node), port_w = xmlGetText(port_node), cookie_w = xmlGetText(cookie_node);
	if (!host_w || !port_w || !cookie_w)
		return false;

	bos_host = _T2A(host_w);
	bos_port = atoi(_T2A(port_w));
	cookie = _T2A(cookie_w);

	if (encryption) {
		HXML tls_node = xmlGetChildByPath(root, L"response/data/tlsCertName", 0); //tls is optional, so this is not fatal error
		if (tls_node) {
			LPCTSTR certname_w = xmlGetText(tls_node);
			if (certname_w)
				tls_cert_name = _T2A(certname_w);
		}
	}

	return true;
}

void CAimProto::aim_connection_clientlogin(void)
{
	pass_ptrA password(getStringA(AIM_KEY_PW));
	replaceStr(m_username, ptrA(getStringA(AIM_KEY_SN)));

	CMStringA buf;
	buf.Format("devId=%s&f=xml&pwd=%s&s=%s", AIM_DEFAULT_CLIENT_KEY, ptrA(mir_urlEncode(password)), ptrA(mir_urlEncode(m_username)));

	NETLIBHTTPHEADER headers[] = {
		{ "Content-Type", "application/x-www-form-urlencoded; charset=UTF-8" }
	};

	NETLIBHTTPREQUEST req = { 0 };
	req.cbSize = sizeof(req);
	req.flags = NLHRF_SSL;
	req.requestType = REQUEST_POST;
	req.szUrl = AIM_LOGIN_URL;
	req.headers = headers;
	req.headersCount = _countof(headers);
	req.pData = buf.GetBuffer();
	req.dataLength = buf.GetLength();

	NLHR_PTR resp(Netlib_HttpTransaction(m_hNetlibUser, &req));
	if (!resp || !resp->dataLength) {
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}

	time_t hosttime;
	CMStringA token, secret;
	if (!parse_clientlogin_response(resp, headers, token, secret, hosttime)) {
		//TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		mir_free(headers[0].szValue);
		return;
	}

	bool encryption = !getByte(AIM_KEY_DSSL, 0);
	CMStringA url;
	fill_session_url(url, token, secret, hosttime, password, encryption);

	// reuse NETLIBHTTPREQUEST
	req.requestType = REQUEST_GET;
	req.pData = NULL;
	req.flags |= NLHRF_MANUALHOST;
	req.dataLength = 0;
	req.headersCount = 1;
	req.szUrl = url.GetBuffer();
	{
		NETLIBHTTPHEADER headers2[] = {
			{ "Host", "api.oscar.aol.com" },
		};
		req.headers = headers2;

		resp = Netlib_HttpTransaction(m_hNetlibUser, &req);
	}

	if (!resp || !resp->dataLength) {
		// TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}

	CMStringA bos_host, cookie, tls_cert_name; //TODO: find efficient buf size
	unsigned short bos_port = 0;
	if (!parse_start_socar_session_response(resp->pData, bos_host, bos_port, cookie, tls_cert_name, encryption)) {
		// TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}
	if(COOKIE)
		mir_free(COOKIE);
	COOKIE = (char*)mir_base64_decode(cookie, (unsigned int*)&COOKIE_LENGTH);


	m_hServerConn = aim_connect(bos_host, bos_port, (tls_cert_name[0] && encryption) ? true : false, bos_host);
	if (!m_hServerConn) {
		// TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}



	ForkThread(&CAimProto::aim_protocol_negotiation, 0);
}

void __cdecl CAimProto::aim_protocol_negotiation(void*)
{
	HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(m_hServerConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {};
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;
	for (;;) {
		int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
		if (recvResult == 0) {
			debugLogA("Connection Closed: No Error during Connection Negotiation?");
			break;
		}
		else if (recvResult == SOCKET_ERROR) {
			if (WSAGetLastError() == ERROR_TIMEOUT) {
				if (aim_keepalive(m_hServerConn, m_seqno) < 0)
					break;
			}
			else {
				debugLogA("Connection Closed: Socket Error during Connection Negotiation %d", WSAGetLastError());
				break;
			}
		}
		else if (recvResult > 0) {
			unsigned short flap_length = 0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], packetRecv.bytesAvailable - packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01)) {
					if (getByte(AIM_KEY_CLIENTLOGIN, 1) != 1)
					{
						aim_send_cookie(m_hServerConn, m_seqno, COOKIE_LENGTH, COOKIE);//cookie challenge
						mir_free(COOKIE);
						COOKIE = NULL;
						COOKIE_LENGTH = 0;
					}
					else
					{
						unsigned short offset = 0;
						char client_id[64], mirver[64];
						Miranda_GetVersionText(mirver, sizeof(mirver));
						int client_id_len = mir_snprintf(client_id, "Miranda AIM, version %s", mirver);

						char* buf_ = (char*)alloca(SNAC_SIZE + TLV_HEADER_SIZE * 8 + COOKIE_LENGTH + client_id_len + 30); //TODO: correct length

						aim_writelong(0x01, offset, buf_);//protocol version number

						aim_writetlv(0x06, (unsigned short)COOKIE_LENGTH, COOKIE, offset, buf_);

						aim_writetlv(0x03, (unsigned short)client_id_len, client_id, offset, buf_);

						aim_writetlvshort(0x17, AIM_CLIENT_MAJOR_VERSION, offset, buf_);
						aim_writetlvshort(0x18, AIM_CLIENT_MINOR_VERSION, offset, buf_);
						aim_writetlvshort(0x19, AIM_CLIENT_LESSER_VERSION, offset, buf_);
						aim_writetlvshort(0x1A, AIM_CLIENT_BUILD_NUMBER, offset, buf_);
						aim_writetlvchar(0x4A, getByte(AIM_KEY_FSC, 0) ? 3 : 1, offset, buf_);

						aim_sendflap(m_hServerConn, 0x01, offset, buf_, m_seqno);
						mir_free(COOKIE);
						COOKIE = NULL;
						COOKIE_LENGTH = 0;

					}
				}
				else if (flap.cmp(0x02)) {
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001)) {
						snac_supported_families(snac, m_hServerConn, m_seqno);
						snac_supported_family_versions(snac, m_hServerConn, m_seqno);
						snac_rate_limitations(snac, m_hServerConn, m_seqno);
						snac_service_redirect(snac);
						snac_self_info(snac);
						snac_error(snac);
					}
					else if (snac.cmp(0x0002)) {
						snac_received_info(snac);
						snac_error(snac);
					}
					else if (snac.cmp(0x0003)) {
						snac_user_online(snac);
						snac_user_offline(snac);
						snac_error(snac);
					}
					else if (snac.cmp(0x0004)) {
						snac_icbm_limitations(snac, m_hServerConn, m_seqno);
						snac_message_accepted(snac);
						snac_received_message(snac, m_hServerConn, m_seqno);
						snac_typing_notification(snac);
						snac_error(snac);
						snac_file_decline(snac);
					}
					else if (snac.cmp(0x000A)) {
						snac_email_search_results(snac);
						/*
							If there's no match (error 0x14), AIM will pop up a message.
							Since it's annoying and there's no other errors that'll get
							generated, I just assume leave this commented out. It's here
							for consistency.
							*/
						//snac_error(snac); 
					}
					else if (snac.cmp(0x0013)) {
						snac_contact_list(snac, m_hServerConn, m_seqno);
						snac_list_modification_ack(snac);
						snac_error(snac);
					}
				}
				else if (flap.cmp(0x04)) {
					ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION);
					debugLogA("Connection Negotiation Thread Ending: Flap 0x04");
					goto exit;
				}
			}
		}
	}

exit:
	if (m_iStatus != ID_STATUS_OFFLINE) broadcast_status(ID_STATUS_OFFLINE);
	Netlib_CloseHandle(hServerPacketRecver); hServerPacketRecver = NULL;
	Netlib_CloseHandle(m_hServerConn); m_hServerConn = NULL;
	debugLogA("Connection Negotiation Thread Ending: End of Thread");
	offline_contacts();
}

void __cdecl CAimProto::aim_mail_negotiation(void*)
{
	HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(m_hMailConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;

	while (m_iStatus != ID_STATUS_OFFLINE) {
		int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR) {
			if (WSAGetLastError() == ERROR_TIMEOUT) {
				if (aim_keepalive(m_hMailConn, m_mail_seqno) < 0)
					break;
			}
			else
				break;
		}
		if (recvResult > 0) {
			unsigned short flap_length = 0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
				if (!packetRecv.buffer)
					break;

				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], packetRecv.bytesAvailable - packetRecv.bytesUsed);
				if (!flap.len())
					break;

				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01)) {
					aim_send_cookie(m_hMailConn, m_mail_seqno, MAIL_COOKIE_LENGTH, MAIL_COOKIE);//cookie challenge
					mir_free(MAIL_COOKIE);
					MAIL_COOKIE = NULL;
					MAIL_COOKIE_LENGTH = 0;
				}
				else if (flap.cmp(0x02)) {
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001)) {
						snac_supported_families(snac, m_hMailConn, m_mail_seqno);
						snac_supported_family_versions(snac, m_hMailConn, m_mail_seqno);
						snac_mail_rate_limitations(snac, m_hMailConn, m_mail_seqno);
						snac_error(snac);
					}
					else if (snac.cmp(0x0018)) {
						snac_mail_response(snac);
					}
				}
				else if (flap.cmp(0x04))
					goto exit;
			}
		}
	}

exit:
	debugLogA("Mail Server Connection has ended");
	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(m_hMailConn);
	m_hMailConn = NULL;
}

void __cdecl CAimProto::aim_avatar_negotiation(void*)
{
	HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(m_hAvatarConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.dwTimeout = 300000;//5 minutes connected

	for (;;) {
		int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR)
			break;

		if (recvResult > 0) {
			unsigned short flap_length = 0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
				if (!packetRecv.buffer)
					break;

				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], packetRecv.bytesAvailable - packetRecv.bytesUsed);
				if (!flap.len())
					break;

				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01)) {
					aim_send_cookie(m_hAvatarConn, m_avatar_seqno, AVATAR_COOKIE_LENGTH, AVATAR_COOKIE); // cookie challenge
					mir_free(AVATAR_COOKIE);
					AVATAR_COOKIE = NULL;
					AVATAR_COOKIE_LENGTH = 0;
				}
				else if (flap.cmp(0x02)) {
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001)) {
						snac_supported_families(snac, m_hAvatarConn, m_avatar_seqno);
						snac_supported_family_versions(snac, m_hAvatarConn, m_avatar_seqno);
						snac_avatar_rate_limitations(snac, m_hAvatarConn, m_avatar_seqno);
						snac_error(snac);
					}
					if (snac.cmp(0x0010)) {
						snac_retrieve_avatar(snac);
						snac_upload_reply_avatar(snac);
					}
				}
				else if (flap.cmp(0x04))
					goto exit;
			}
		}
	}

exit:
	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(m_hAvatarConn);
	m_hAvatarConn = NULL;
	ResetEvent(m_hAvatarEvent);
	debugLogA("Avatar Server Connection has ended");
}

void __cdecl CAimProto::aim_chatnav_negotiation(void*)
{
	unsigned idle_chat = 0;
	HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(m_hChatNavConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;

	for (;;) {
		int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR) {
			if (WSAGetLastError() == ERROR_TIMEOUT) {
				if (m_chat_rooms.getCount())
					idle_chat = 0;
				else if (++idle_chat >= 6)
					break;

				if (aim_keepalive(m_hChatNavConn, m_chatnav_seqno) < 0)
					break;
			}
			else
				break;
		}

		if (recvResult > 0) {
			unsigned short flap_length = 0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], packetRecv.bytesAvailable - packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01)) {
					aim_send_cookie(m_hChatNavConn, m_chatnav_seqno, CHATNAV_COOKIE_LENGTH, CHATNAV_COOKIE);//cookie challenge
					mir_free(CHATNAV_COOKIE);
					CHATNAV_COOKIE = NULL;
					CHATNAV_COOKIE_LENGTH = 0;
				}
				else if (flap.cmp(0x02)) {
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001)) {
						snac_supported_families(snac, m_hChatNavConn, m_chatnav_seqno);
						snac_supported_family_versions(snac, m_hChatNavConn, m_chatnav_seqno);
						snac_chatnav_rate_limitations(snac, m_hChatNavConn, m_chatnav_seqno);
						snac_error(snac);
					}
					if (snac.cmp(0x000D)) {
						snac_chatnav_info_response(snac, m_hChatNavConn, m_chatnav_seqno);
						snac_error(snac);
					}
				}
				else if (flap.cmp(0x04))
					goto exit;
			}
		}
	}

exit:
	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(m_hChatNavConn);
	m_hChatNavConn = NULL;
	ResetEvent(m_hChatNavEvent);
	debugLogA("Chat Navigation Server Connection has ended");
}

void __cdecl CAimProto::aim_chat_negotiation(void* param)
{
	chat_list_item *item = (chat_list_item*)param;
	HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(item->hconn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;

	for (;;) {
		int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR) {
			if (WSAGetLastError() == ERROR_TIMEOUT) {
				if (aim_keepalive(item->hconn, item->seqno) < 0)
					break;
			}
			else
				break;
		}

		if (recvResult > 0) {
			unsigned short flap_length = 0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], packetRecv.bytesAvailable - packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01)) {
					aim_send_cookie(item->hconn, item->seqno, item->CHAT_COOKIE_LENGTH, item->CHAT_COOKIE);//cookie challenge
					mir_free(item->CHAT_COOKIE);
					item->CHAT_COOKIE = NULL;
					item->CHAT_COOKIE_LENGTH = 0;
				}
				else if (flap.cmp(0x02)) {
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001)) {
						snac_supported_families(snac, item->hconn, item->seqno);
						snac_supported_family_versions(snac, item->hconn, item->seqno);
						snac_chat_rate_limitations(snac, item->hconn, item->seqno);
						snac_error(snac);

					}
					if (snac.cmp(0x000E)) {
						snac_chat_received_message(snac, item);
						snac_chat_joined_left_users(snac, item);
						snac_error(snac);
					}
				}
				else if (flap.cmp(0x04))
					goto exit;
			}
		}
	}

exit:
	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(item->hconn);
	chat_leave(item->id);
	remove_chat_by_ptr(item);
	debugLogA("Chat Server Connection has ended");
}

void __cdecl CAimProto::aim_admin_negotiation(void*)
{
	HANDLE hServerPacketRecver = Netlib_CreatePacketReceiver(m_hAdminConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {};
	packetRecv.dwTimeout = 300000; // 5 minutes connected

	for (;;) {
		int recvResult = Netlib_GetMorePackets(hServerPacketRecver, &packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR)
			break;

		if (recvResult > 0) {
			unsigned short flap_length = 0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length) {
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed], packetRecv.bytesAvailable - packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01)) {
					aim_send_cookie(m_hAdminConn, m_admin_seqno, ADMIN_COOKIE_LENGTH, ADMIN_COOKIE);//cookie challenge
					mir_free(ADMIN_COOKIE);
					ADMIN_COOKIE = NULL;
					ADMIN_COOKIE_LENGTH = 0;
				}
				else if (flap.cmp(0x02)) {
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001)) {
						snac_supported_families(snac, m_hAdminConn, m_admin_seqno);
						snac_supported_family_versions(snac, m_hAdminConn, m_admin_seqno);
						snac_admin_rate_limitations(snac, m_hAdminConn, m_admin_seqno);
						snac_error(snac);
					}
					if (snac.cmp(0x0007)) {
						snac_admin_account_infomod(snac);
						snac_admin_account_confirm(snac);
						snac_error(snac);
					}
				}
				else if (flap.cmp(0x04))
					goto exit;
			}
		}
	}

exit:
	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(m_hAdminConn);
	m_hAdminConn = NULL;
	ResetEvent(m_hAdminEvent);
	debugLogA("Admin Server Connection has ended");
}
