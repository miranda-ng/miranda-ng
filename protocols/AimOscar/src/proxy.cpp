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

void __cdecl CAimProto::aim_proxy_helper(void* param)
{
	file_transfer *ft = (file_transfer*)param;

	if (ft->requester) 
	{
		if (proxy_initialize_send(ft->hConn, username, ft->icbm_cookie))
			return;//error
	}
	else
	{
		if (proxy_initialize_recv(ft->hConn, username, ft->icbm_cookie, ft->port)) 
			return;//error
	}

	//start listen for packets stuff
	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = INFINITE;

	HANDLE hServerPacketRecver = (HANDLE) CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)ft->hConn, 2048 * 4);
	for (;;)
	{
		int recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0) 
		{
			ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			break;
		}
		if (recvResult == SOCKET_ERROR) 
		{
			ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			break;
		}
		if (recvResult > 0) 
		{
			unsigned short length = _htons(*(unsigned short*)&packetRecv.buffer[0]);
			packetRecv.bytesUsed = length + 2;
			unsigned short type = _htons(*(unsigned short*)&packetRecv.buffer[4]);
			if (type == 0x0001)
			{
				unsigned short error = _htons(*(unsigned short*)&packetRecv.buffer[12]);
				switch (error)
				{
				case 0x000D:
					ShowPopup("Proxy Server File Transfer Error: Bad Request.", ERROR_POPUP);
					break;

				case 0x0010:
					ShowPopup("Proxy Server File Transfer Error: Initial Request Timed Out.", ERROR_POPUP);
					break;

				case 0x001A:
					ShowPopup("Proxy Server File Transfer Error: Accept Period Timed Out.", ERROR_POPUP);
					break;

				case 0x000e:
					ShowPopup("Proxy Server File Transfer Error: Incorrect command syntax.", ERROR_POPUP);
					break;

				case 0x0016:
					ShowPopup("Proxy Server File Transfer Error: Unknown command issued.", ERROR_POPUP);
					break;
				}

			}
			else if (type == 0x0003)
			{
				unsigned short port = _htons(*(unsigned short*)&packetRecv.buffer[12]);
				unsigned long  ip   = _htonl(*(unsigned long*)&packetRecv.buffer[14]);
				
				aim_send_file(hServerConn, seqno, ip, port, true, ft);
				debugLogA("Stage %d Proxy ft and we are not the sender.", ft->req_num);
			}
			else if (type == 0x0005) 
			{
				if (!ft->requester) 
				{
					aim_file_ad(hServerConn, seqno, ft->sn, ft->icbm_cookie, false, ft->max_ver);
					ft->accepted = true;
				}

				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, ft, 0);

				int i;
				for (i = 21; --i; )
				{
					if (Miranda_Terminated()) return;
					Sleep(100);
					if (ft->accepted) break;
				}
				if (i == 0) 
				{
					ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
					break;
				}

				packetRecv.dwTimeout = 350000;

				int result;
				if (ft->sending)//we are sending
					result = sending_file(ft, hServerPacketRecver, packetRecv);
				else 
					result = receiving_file(ft, hServerPacketRecver, packetRecv);

				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, result ? ACKRESULT_FAILED : ACKRESULT_SUCCESS, ft, 0);
				break;
			}
		}
	}
	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(ft->hConn);

	ft_list.remove_by_ft(ft);
}


int proxy_initialize_send(HANDLE connection, char* sn, char* cookie)
{
	const char sn_length = (char)strlen(sn);
	const int len = sn_length + 21 + TLV_HEADER_SIZE + AIM_CAPS_LENGTH;

	char* buf= (char*)alloca(len);
	unsigned short offset=0;

	aim_writeshort(len-2, offset, buf);
	aim_writegeneric(10, "\x04\x4a\0\x02\0\0\0\0\0\0", offset, buf);
	aim_writechar((unsigned char)sn_length, offset, buf);               // screen name len
	aim_writegeneric(sn_length, sn, offset, buf);                       // screen name
	aim_writegeneric(8, cookie, offset, buf);                           // icbm cookie
	aim_writetlv(1, AIM_CAPS_LENGTH, AIM_CAP_FILE_TRANSFER, offset, buf);

	return Netlib_Send(connection, buf, offset, 0) >= 0 ? 0 : -1; 
}

int proxy_initialize_recv(HANDLE connection,char* sn, char* cookie,unsigned short port_check)
{
	const char sn_length = (char)strlen(sn);
	const int len = sn_length + 23 + TLV_HEADER_SIZE + AIM_CAPS_LENGTH;

	char* buf= (char*)alloca(len);
	unsigned short offset=0;

	aim_writeshort(len-2, offset, buf);
	aim_writegeneric(10, "\x04\x4a\0\x04\0\0\0\0\0\0", offset, buf);
	aim_writechar((unsigned char)sn_length, offset, buf);               // screen name len
	aim_writegeneric(sn_length, sn, offset, buf);                       // screen name
	aim_writeshort(port_check, offset, buf);
	aim_writegeneric(8, cookie, offset, buf);                           // icbm cookie
	aim_writetlv(1, AIM_CAPS_LENGTH, AIM_CAP_FILE_TRANSFER, offset, buf);

	return Netlib_Send(connection, buf, offset, 0) >= 0 ? 0 : -1; 
}
