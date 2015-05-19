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
	HANDLE con = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&ncon);
	if (con && use_ssl) {
		NETLIBSSL ssl = {0};
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
	return (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibPeer, (LPARAM)&ncon);
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
			mir_free(username);
			username = getStringA(AIM_KEY_SN);
			if (username != NULL) {
				HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hServerConn, 2048 * 4);
				NETLIBPACKETRECVER packetRecv = {0};
				packetRecv.cbSize = sizeof(packetRecv);
				packetRecv.dwTimeout = 5000;
				for (;;) {
					int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM) hServerPacketRecver, (LPARAM) & packetRecv);
					if (recvResult == 0) {
						debugLogA("Connection Closed: No Error? during Connection Authorization");
						break;
					}
					else if (recvResult < 0) {
						debugLogA("Connection Closed: Socket Error during Connection Authorization %d", WSAGetLastError());
						break;
					}
					else {
						unsigned short flap_length=0;
						for (;packetRecv.bytesUsed<packetRecv.bytesAvailable;packetRecv.bytesUsed = flap_length) {
							if (!packetRecv.buffer)
								break;

							FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],(unsigned short)(packetRecv.bytesAvailable-packetRecv.bytesUsed));
							if (!flap.len())
								break;

							flap_length+=FLAP_SIZE+flap.len();
							if (flap.cmp(0x01)) {
								if (aim_send_connection_packet(hServerConn, seqno,flap.val())==0)//cookie challenge
									aim_authkey_request(hServerConn, seqno);//md5 authkey request
							}
							else if (flap.cmp(0x02)) {
								SNAC snac(flap.val(),flap.snaclen());
								if (snac.cmp(0x0017)) {
									snac_md5_authkey(snac,hServerConn,seqno, username, password);
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
	if (m_iStatus!=ID_STATUS_OFFLINE)
		broadcast_status(ID_STATUS_OFFLINE);
	Netlib_CloseHandle(hServerConn);
	hServerConn=NULL;
	debugLogA("Connection Authorization Thread Ending: End of Thread");
}

void __cdecl CAimProto::aim_protocol_negotiation( void* )
{
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hServerConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER*1000;	
	for (;;)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0)
		{
			debugLogA("Connection Closed: No Error during Connection Negotiation?");
			break;
		}
		else if (recvResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == ERROR_TIMEOUT)
			{
				if (aim_keepalive(hServerConn,seqno) < 0) 
					break;
			}
			else
			{
				debugLogA("Connection Closed: Socket Error during Connection Negotiation %d", WSAGetLastError());
				break;
			}
		}
		else if (recvResult>0)
		{
			unsigned short flap_length=0;
			for (;packetRecv.bytesUsed<packetRecv.bytesAvailable;packetRecv.bytesUsed=flap_length)
			{
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],packetRecv.bytesAvailable-packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length+=FLAP_SIZE+flap.len();
				if (flap.cmp(0x01))
				{
					aim_send_cookie(hServerConn,seqno,COOKIE_LENGTH,COOKIE);//cookie challenge
					mir_free(COOKIE);
					COOKIE=NULL;
					COOKIE_LENGTH=0;
				}
				else if (flap.cmp(0x02))
				{
					SNAC snac(flap.val(),flap.snaclen());
					if (snac.cmp(0x0001))
					{
						snac_supported_families(snac,hServerConn,seqno);
						snac_supported_family_versions(snac,hServerConn,seqno);
						snac_rate_limitations(snac,hServerConn,seqno);
						snac_service_redirect(snac);
						snac_self_info(snac);
						snac_error(snac);
					}
					else if (snac.cmp(0x0002))
					{
						snac_received_info(snac);
						snac_error(snac);
					}
					else if (snac.cmp(0x0003))
					{
						snac_user_online(snac);
						snac_user_offline(snac);
						snac_error(snac);
					}
					else if (snac.cmp(0x0004))
					{
						snac_icbm_limitations(snac,hServerConn,seqno);
						snac_message_accepted(snac);
						snac_received_message(snac,hServerConn,seqno);
						snac_typing_notification(snac);
						snac_error(snac);
						snac_file_decline(snac);
					}
					else if (snac.cmp(0x000A))
					{
						snac_email_search_results(snac);
						/* 
							If there's no match (error 0x14), AIM will pop up a message.
							Since it's annoying and there's no other errors that'll get
							generated, I just assume leave this commented out. It's here
							for consistency.
						*/
						//snac_error(snac); 
					}
					else if (snac.cmp(0x0013))
					{
						snac_contact_list(snac,hServerConn,seqno);
						snac_list_modification_ack(snac);
						snac_error(snac);
					}
				}
				else if (flap.cmp(0x04))
				{
					ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION);
					debugLogA("Connection Negotiation Thread Ending: Flap 0x04");
					goto exit;
				}
			}
		}
	}

exit:
	if (m_iStatus!=ID_STATUS_OFFLINE) broadcast_status(ID_STATUS_OFFLINE);
	Netlib_CloseHandle(hServerPacketRecver); hServerPacketRecver=NULL; 
	Netlib_CloseHandle(hServerConn); hServerConn=NULL;
	debugLogA("Connection Negotiation Thread Ending: End of Thread");
	offline_contacts();
}

void __cdecl CAimProto::aim_mail_negotiation( void* )
{
	HANDLE hServerPacketRecver = (HANDLE) CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hMailConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER*1000;
	while(m_iStatus!=ID_STATUS_OFFLINE)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0)
		{
			break;
		}
		if (recvResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == ERROR_TIMEOUT)
			{
				if (aim_keepalive(hMailConn, mail_seqno) < 0)
					break;
			}
			else
				break;
		}
		if (recvResult>0)
		{
			unsigned short flap_length=0;
			for (;packetRecv.bytesUsed<packetRecv.bytesAvailable;packetRecv.bytesUsed=flap_length)
			{
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],packetRecv.bytesAvailable-packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length+=FLAP_SIZE+flap.len();
				if (flap.cmp(0x01))
				{
					aim_send_cookie(hMailConn,mail_seqno,MAIL_COOKIE_LENGTH,MAIL_COOKIE);//cookie challenge
					mir_free(MAIL_COOKIE);
					MAIL_COOKIE=NULL;
					MAIL_COOKIE_LENGTH=0;
				}
				else if (flap.cmp(0x02))
				{
					SNAC snac(flap.val(),flap.snaclen());
					if (snac.cmp(0x0001))
					{
						snac_supported_families(snac,hMailConn,mail_seqno);
						snac_supported_family_versions(snac,hMailConn,mail_seqno);
						snac_mail_rate_limitations(snac,hMailConn,mail_seqno);
						snac_error(snac);
					}
					else if (snac.cmp(0x0018))
					{
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
	Netlib_CloseHandle(hMailConn);
	hMailConn=NULL;
}

void __cdecl CAimProto::aim_avatar_negotiation( void* )
{
	HANDLE hServerPacketRecver = (HANDLE) CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hAvatarConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = 300000;//5 minutes connected
	for (;;)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM) hServerPacketRecver, (LPARAM) & packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR)
			break;

		if (recvResult > 0)
		{
			unsigned short flap_length=0;
			for (; packetRecv.bytesUsed < packetRecv.bytesAvailable; packetRecv.bytesUsed = flap_length)
			{
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],packetRecv.bytesAvailable-packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length += FLAP_SIZE + flap.len();
				if (flap.cmp(0x01))
				{
					aim_send_cookie(hAvatarConn, avatar_seqno, AVATAR_COOKIE_LENGTH, AVATAR_COOKIE);//cookie challenge
					mir_free(AVATAR_COOKIE);
					AVATAR_COOKIE = NULL;
					AVATAR_COOKIE_LENGTH = 0;
				}
				else if (flap.cmp(0x02))
				{
					SNAC snac(flap.val(), flap.snaclen());
					if (snac.cmp(0x0001))
					{
						snac_supported_families(snac, hAvatarConn, avatar_seqno);
						snac_supported_family_versions(snac, hAvatarConn, avatar_seqno);
						snac_avatar_rate_limitations(snac, hAvatarConn, avatar_seqno);
						snac_error(snac);
					}
					if (snac.cmp(0x0010))
					{
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
	Netlib_CloseHandle(hAvatarConn);
	hAvatarConn=NULL;
	ResetEvent(hAvatarEvent);
	debugLogA("Avatar Server Connection has ended");
}

void __cdecl CAimProto::aim_chatnav_negotiation( void* )
{
	unsigned idle_chat = 0;
	HANDLE hServerPacketRecver = (HANDLE) CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hChatNavConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER*1000;
	for (;;)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM) hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == ERROR_TIMEOUT)
			{
				if (chat_rooms.getCount())
					idle_chat = 0;
				else if (++idle_chat >= 6)
					break;

				if (aim_keepalive(hChatNavConn, chatnav_seqno) < 0)
					break;
			}
			else
				break;
		}

		if (recvResult>0)
		{
			unsigned short flap_length=0;
			for (;packetRecv.bytesUsed<packetRecv.bytesAvailable;packetRecv.bytesUsed=flap_length)
			{
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],packetRecv.bytesAvailable-packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length+=FLAP_SIZE+flap.len();
				if (flap.cmp(0x01))
				{
					aim_send_cookie(hChatNavConn,chatnav_seqno,CHATNAV_COOKIE_LENGTH,CHATNAV_COOKIE);//cookie challenge
					mir_free(CHATNAV_COOKIE);
					CHATNAV_COOKIE=NULL;
					CHATNAV_COOKIE_LENGTH=0;
				}
				else if (flap.cmp(0x02))
				{
					SNAC snac(flap.val(),flap.snaclen());
					if (snac.cmp(0x0001))
					{
						snac_supported_families(snac,hChatNavConn,chatnav_seqno);
						snac_supported_family_versions(snac,hChatNavConn,chatnav_seqno);
						snac_chatnav_rate_limitations(snac,hChatNavConn,chatnav_seqno);
						snac_error(snac);
					}
					if (snac.cmp(0x000D))
					{
						snac_chatnav_info_response(snac,hChatNavConn,chatnav_seqno);
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
	Netlib_CloseHandle(hChatNavConn);
	hChatNavConn=NULL;
	ResetEvent(hChatNavEvent);
	debugLogA("Chat Navigation Server Connection has ended");
}

void __cdecl CAimProto::aim_chat_negotiation( void* param )
{
	chat_list_item *item = (chat_list_item*)param;
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)item->hconn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = DEFAULT_KEEPALIVE_TIMER*1000;
	for (;;)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == ERROR_TIMEOUT)
			{
				if (aim_keepalive(item->hconn, item->seqno) < 0)
					break;
			}
			else
				break;
		}

		if (recvResult>0)
		{
			unsigned short flap_length=0;
			for (;packetRecv.bytesUsed<packetRecv.bytesAvailable;packetRecv.bytesUsed=flap_length)
			{
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],packetRecv.bytesAvailable-packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length+=FLAP_SIZE+flap.len();
				if (flap.cmp(0x01))
				{
					aim_send_cookie(item->hconn,item->seqno,item->CHAT_COOKIE_LENGTH,item->CHAT_COOKIE);//cookie challenge
					mir_free(item->CHAT_COOKIE);
					item->CHAT_COOKIE=NULL;
					item->CHAT_COOKIE_LENGTH=0;
				}
				else if (flap.cmp(0x02))
				{
					SNAC snac(flap.val(),flap.snaclen());
					if (snac.cmp(0x0001))
					{
						snac_supported_families(snac,item->hconn,item->seqno);
						snac_supported_family_versions(snac,item->hconn,item->seqno);
						snac_chat_rate_limitations(snac,item->hconn,item->seqno);
						snac_error(snac);

					}
					if (snac.cmp(0x000E))
					{
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

void __cdecl CAimProto::aim_admin_negotiation( void* )
{
	HANDLE hServerPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hAdminConn, 2048 * 8);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = 300000;//5 minutes connected
	for (;;)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM) hServerPacketRecver, (LPARAM) & packetRecv);
		if (recvResult == 0)
			break;

		if (recvResult == SOCKET_ERROR)
			break;

		if (recvResult>0)
		{
			unsigned short flap_length=0;
			for (;packetRecv.bytesUsed<packetRecv.bytesAvailable;packetRecv.bytesUsed=flap_length)
			{
				if (!packetRecv.buffer)
					break;
				FLAP flap((char*)&packetRecv.buffer[packetRecv.bytesUsed],packetRecv.bytesAvailable-packetRecv.bytesUsed);
				if (!flap.len())
					break;
				flap_length+=FLAP_SIZE+flap.len();
				if (flap.cmp(0x01))
				{
					aim_send_cookie(hAdminConn,admin_seqno,ADMIN_COOKIE_LENGTH,ADMIN_COOKIE);//cookie challenge
					mir_free(ADMIN_COOKIE);
					ADMIN_COOKIE=NULL;
					ADMIN_COOKIE_LENGTH=0;
				}
				else if (flap.cmp(0x02))
				{
					SNAC snac(flap.val(),flap.snaclen());
					if (snac.cmp(0x0001))
					{
						snac_supported_families(snac,hAdminConn,admin_seqno);
						snac_supported_family_versions(snac,hAdminConn,admin_seqno);
						snac_admin_rate_limitations(snac,hAdminConn,admin_seqno);
						snac_error(snac);
					}
					if (snac.cmp(0x0007))
					{
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
	Netlib_CloseHandle(hAdminConn);
	hAdminConn=NULL;
	ResetEvent(hAdminEvent);
	debugLogA("Admin Server Connection has ended");
}
