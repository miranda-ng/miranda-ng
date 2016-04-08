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

HANDLE CAimProto::aim_connect(const char* server, unsigned short port, bool use_ssl, const char* host)
{
	NETLIBOPENCONNECTION ncon = { 0 };
	ncon.cbSize = sizeof(ncon);
	ncon.szHost = server;
	ncon.wPort = port;
	ncon.timeout = 6;
	ncon.flags = NLOCF_V2;
	debugLogA("%s:%u", server, port);
	HANDLE con = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&ncon);
	if (con && use_ssl) {
		NETLIBSSL ssl = { 0 };
		ssl.cbSize = sizeof(ssl);
		ssl.host = host;
		if (!CallService(MS_NETLIB_STARTSSL, (WPARAM)con, (LPARAM)&ssl)) {
			Netlib_CloseHandle(con);
			con = NULL;
		}
	}
	return con;
}

HANDLE CAimProto::aim_peer_connect(const char* ip, unsigned short port)
{
	NETLIBOPENCONNECTION ncon = { 0 };
	ncon.cbSize = sizeof(ncon);
	ncon.flags = NLOCF_V2;
	ncon.szHost = ip;
	ncon.wPort = port;
	ncon.timeout = 3;
	return (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibPeer, (LPARAM)&ncon);
}

HANDLE CAimProto::aim_peer_connect(unsigned long ip, unsigned short port)
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
				HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)m_hServerConn, 2048 * 4);
				NETLIBPACKETRECVER packetRecv = { 0 };
				packetRecv.cbSize = sizeof(packetRecv);
				packetRecv.dwTimeout = 5000;
				for (;;) {
					int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)& packetRecv);
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

void fill_post_request(char *buf, const char*password, const char* login)
{
	//TODO: fill post data
	/*
		devId=dev_key&f=xml&pwd=urlencoded_password&s=urlencoded_screenname
	*/
	char *_pas = mir_urlEncode(password);
	char *_login = mir_urlEncode(login);
	mir_snprintf(buf, 1023, "devId=%s&f=xml&pwd=%s&s=%s", AIM_DEFAULT_CLIENT_KEY, _pas, _login);
	mir_free(_pas);
	mir_free(_login);
}

bool parse_clientlogin_response(const char *buf, char *token, char *secret, time_t &hosttime)
{
	//TODO: validate response
	//TODO: return false on errors
	//TODO: extract token, secret, hosttime from response
	int datalen = 0;
	wchar_t *buf_w =mir_utf8decodeW(buf);
	HXML root = xmlParseString(buf_w, &datalen, _T(""));
	if(!root)
	{
		mir_free(buf_w);
		return false;
	}
	HXML status = xmlGetNthChild(root, _T("statusCode"), 0);
	if(!status)
	{
		mir_free(buf_w);
		return false;
	}
	LPCTSTR status_text = xmlGetText(status);
	//TODO: check other than 200 codes and print some debug info on errors
	if(wcscmp(status_text, L"200"))
	{
		mir_free(buf_w);
		return false;
	}
	HXML data = xmlGetNthChild(root, _T("data"), 0);
	if(!data)
	{
		mir_free(buf_w);
		return false;
	}
	HXML secret_node = xmlGetNthChild(data, _T("sessionSecret"), 0);
	HXML hosttime_node = xmlGetNthChild(data, _T("hostTime"), 0);
	HXML token_node = xmlGetNthChild(data, _T("token"), 0);
	if(!secret_node || !hosttime_node || !token_node)
	{
		mir_free(buf_w);
		return false;
	}
	token_node = xmlGetNthChild(token_node, _T("a"), 0);
	if(!token_node)
	{
		mir_free(buf_w);
		return false;
	}
	LPCTSTR secret_w = xmlGetText(secret_node), token_w = xmlGetText(token_node), hosttime_w = xmlGetText(hosttime_node);
	if(!secret_w || !token_w || !hosttime_w)
	{
		mir_free(buf_w);
		return false;
	}
	{
		char *tmp_secret = mir_t2a(secret_w);
		if(!tmp_secret)
		{
			mir_free(buf_w);
			return false;
		}
		char *tmp_token = mir_t2a(token_w);
		if(!tmp_token)
		{
			mir_free(tmp_secret);
			mir_free(buf_w);
			return false;
		}
		char *tmp_time = mir_t2a(hosttime_w);
		if(!tmp_time)
		{
			mir_free(tmp_token);
			mir_free(tmp_secret);
			mir_free(buf_w);
			return false;
		}
		mir_strcpy(secret, tmp_secret);
		mir_strcpy(token, tmp_token);
		hosttime = strtol(tmp_time, NULL, 10);
		mir_free(tmp_token);
		mir_free(tmp_secret);
		mir_free(tmp_time);
	}
	mir_free(buf_w);
	return true;
}

void construct_query_string(char *buf, const char *token, time_t hosttime, bool encryption = true)
{
	//TODO: construct query string
	/*
		a=urlencoded_token&distId=0x00000611&f=xml&k=dev_key&ts=hosttime&useTLS=bool_encryption
	*/
	char *urlencoded_token = mir_urlEncode(token);
	mir_snprintf(buf, 1023, "a=%s&distId=0x00000611&f=xml&k=%s&ts=%d&useTLS=%d", urlencoded_token, AIM_DEFAULT_CLIENT_KEY, hosttime, encryption);
	mir_free(urlencoded_token);

}

void generate_signature(char *signature, const char *method, const char *url, const char *parameters, const char *session_key)
{
	char *encoded_url = mir_urlEncode(url);
	char *encoded_parameters = mir_urlEncode(parameters);
	char signature_base[1024];
	mir_snprintf(signature_base, 1023, "%s%s%s", method, encoded_url, encoded_parameters);
	mir_free(encoded_url);
	mir_free(encoded_parameters);
	hmac_sha256(session_key, signature_base, signature);

}

void fill_session_url(char *buf, char *token, char *secret, time_t &hosttime, const char *password, bool encryption = true)
{
	//TODO: construct url for get request
	/*
		AIM_SESSION_URL?query_string?sig_sha256=signature
	*/
	char query_string[1024];
	query_string[0] = 0;
	construct_query_string(query_string, token, hosttime, encryption);
	char signature[512];

	char session_key[1024];

	hmac_sha256(password, secret, session_key);

	generate_signature(signature, "GET", AIM_SESSION_URL, query_string, session_key);

	mir_snprintf(buf, 2023, "%s?%s&sig_sha256=%s", AIM_SESSION_URL, query_string, signature);

}

bool parse_start_socar_session_response(char *response, char *bos_host, unsigned short bos_port, char *cookie, char *tls_cert_name, bool encryption = true)
{
	//TODO: extract bos_host, bos_port, cookie, tls_cert_name

	int datalen = 0;
	wchar_t *buf_w =mir_utf8decodeW(response);
	HXML root = xmlParseString(buf_w, &datalen, _T(""));
	if(!root)
	{
		mir_free(buf_w);
		return false;
	}
	HXML status = xmlGetNthChild(root, _T("statusCode"), 0);
	if(!status)
	{
		mir_free(buf_w);
		return false;
	}
	LPCTSTR status_text = xmlGetText(status);
	//TODO: check other than 200 codes and print some debug info on errors
	if(wcscmp(status_text, L"200"))
	{
		mir_free(buf_w);
		return false;
	}
	HXML data = xmlGetNthChild(root, _T("data"), 0);
	if(!data)
	{
		mir_free(buf_w);
		return false;
	}

	HXML host_node = xmlGetNthChild(data, _T("host"), 0);
	HXML port_node = xmlGetNthChild(data, _T("port"), 0);
	HXML cookie_node = xmlGetNthChild(data, _T("cookie"), 0);
	if(!host_node || !port_node || !cookie_node)
	{
		mir_free(buf_w);
		return false;
	}
	LPCTSTR host_w = xmlGetText(host_node), port_w = xmlGetText(port_node), cookie_w = xmlGetText(cookie_node);
	if(!host_w || !port_w || !cookie_w)
	{
		mir_free(buf_w);
		return false;
	}
	char *tmp_host = mir_t2a(host_w);
	if(!tmp_host)
	{
		mir_free(buf_w);
		return false;
	}
	char *tmp_port = mir_t2a(port_w);
	if(!tmp_port)
	{
		mir_free(tmp_host);
		mir_free(buf_w);
		return false;
	}
	char *tmp_cookie = mir_t2a(cookie_w);
	if(!tmp_cookie)
	{
		mir_free(tmp_port);
		mir_free(tmp_host);
		mir_free(buf_w);
		return false;
	}
	mir_strcpy(bos_host, tmp_host);
	bos_port = atoi(tmp_port);
	mir_strcpy(cookie, tmp_cookie);
	mir_free(tmp_host); mir_free(tmp_port); mir_free(tmp_cookie);
	if (encryption)
	{
		HXML tls_node = xmlGetNthChild(data, _T("tlsCertName"), 0); //tls is optional, so this is not fatal error
		if (tls_node)
		{
			LPCTSTR certname_w = xmlGetText(tls_node);
			if (certname_w)
			{
				char *tmp_certname = mir_t2a(certname_w);
				if (tmp_certname)
				{
					mir_strcpy(tls_cert_name, tmp_certname);
					mir_free(tmp_certname);
				}
			}
		}
	}

	return true;
}

void CAimProto::aim_connection_clientlogin(void)
{
	NETLIBHTTPREQUEST req = {0};
	req.cbSize = sizeof(req);
	req.flags = NLHRF_SSL;
	req.requestType = REQUEST_POST;
	req.szUrl = AIM_LOGIN_URL;
	NETLIBHTTPHEADER headers[1];
	headers[0].szName = "Content-Type";
	headers[0].szValue = "application/x-www-form-urlencoded; charset=UTF-8";
	req.headers = headers;
	req.headersCount = 1;
	char buf[1024];
	buf[0] = 0;
	char *password = getStringA(AIM_KEY_PW);
	mir_free(m_username);
	m_username = getStringA(AIM_KEY_SN);
	fill_post_request(buf, password, m_username);
	req.pData = buf;
	req.dataLength = (int)strlen(buf);

	bool encryption = !getByte(AIM_KEY_DSSL, 0); //TODO: make this configurable

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if(!resp)
	{
		//TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		mir_free(password);
		return;
	}
	if(!resp->dataLength)
	{
		//TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		mir_free(password);
		return;
	}
	char token[512], secret[512]; //TODO: find efficient buf size
	token[0] = 0;
	secret[0] = 0;
	time_t hosttime;
	if(!parse_clientlogin_response(resp->pData, token, secret, hosttime)) //TODO: check if data nullterminated
	{
		//TODO: handle error
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, (WPARAM)0, (LPARAM)&resp);
		broadcast_status(ID_STATUS_OFFLINE);
		mir_free(password);
		return;
	}
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, (WPARAM)0, (LPARAM)&resp);
	//reuse NETLIBHTTPREQUEST
	req.requestType = REQUEST_GET;
	req.pData = NULL;
	req.dataLength = 0;
	char url[2048];
	url[0] = 0;
	fill_session_url(url, token, secret, hosttime, password, encryption);
	mir_free(password);
	req.szUrl = url;
	resp = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if(!resp)
	{
		//TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}
	if(!resp->dataLength)
	{
		//TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}
	char bos_host[128], cookie[1024], tls_cert_name[128]; //TODO: find efficient buf size
	bos_host[0] = 0;
	cookie[0] = 0;
	tls_cert_name[0] = 0;
	unsigned short bos_port = 0;
	if(!parse_start_socar_session_response(resp->pData, bos_host, bos_port, cookie, tls_cert_name, encryption))
	{
		//TODO: handle error
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, (WPARAM)0, (LPARAM)&resp);
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, (WPARAM)0, (LPARAM)&resp);

	m_hServerConn = aim_connect(bos_host, bos_port, (tls_cert_name[0] && encryption) ? true : false, bos_host);
	if(!m_hServerConn)
	{
		//TODO: handle error
		broadcast_status(ID_STATUS_OFFLINE);
		return;
	}

	mir_free(COOKIE);
	COOKIE = mir_strdup(cookie); //TODO: check if it's null terminated
	COOKIE_LENGTH = (int)mir_strlen(cookie);

	ForkThread(&CAimProto::aim_protocol_negotiation, 0);

	
}

void __cdecl CAimProto::aim_protocol_negotiation(void*)
{
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)m_hServerConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;
	for (;;) {
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
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
					aim_send_cookie(m_hServerConn, m_seqno, COOKIE_LENGTH, COOKIE);//cookie challenge
					mir_free(COOKIE);
					COOKIE = NULL;
					COOKIE_LENGTH = 0;
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
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)m_hMailConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;
	while (m_iStatus != ID_STATUS_OFFLINE) {
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0) {
			break;
		}
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
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)m_hAvatarConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = 300000;//5 minutes connected
	for (;;) {
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)& packetRecv);
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
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)m_hChatNavConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;
	for (;;) {
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
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
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)item->hconn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER * 1000;
	for (;;) {
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
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
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)m_hAdminConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = { 0 };
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = 300000;//5 minutes connected
	for (;;) {
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)& packetRecv);
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
