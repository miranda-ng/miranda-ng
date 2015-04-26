/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
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

void __cdecl CAimProto::aim_dc_helper(void* param) //only called when we are initiating a direct connection with someone else
{
	file_transfer *ft = (file_transfer*)param;	

	ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, ft, 0);

	NETLIBPACKETRECVER packetRecv = {0};
	packetRecv.cbSize = sizeof(packetRecv);
	packetRecv.dwTimeout = 350000;

	HANDLE hServerPacketRecver = (HANDLE) CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)ft->hConn, 2048 * 4);

	int result;
	if (ft->sending)//we are sending
		result = sending_file(ft, hServerPacketRecver, packetRecv);
	else 
		result = receiving_file(ft, hServerPacketRecver, packetRecv);

	Netlib_CloseHandle(hServerPacketRecver);
	Netlib_CloseHandle(ft->hConn);
	ft->hConn = NULL;

	if (result == 0)
	{
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	}
	else
	{
		if (!ft->requester && result == 1 && !Miranda_Terminated())
		{
			ft->accepted = false;
			HANDLE hConn = aim_peer_connect(AIM_PROXY_SERVER, get_default_port());
			if (hConn) 
			{
				debugLogA("Connected to proxy ip because we want to use a proxy for the file transfer.");
				ft->requester = true;
				ft->hConn = hConn;
				ForkThread(&CAimProto::aim_proxy_helper, ft);
				return;
			}
		}
		aim_file_ad(hServerConn, seqno, ft->sn, ft->icbm_cookie, true, 0);
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
	}

	ft_list.remove_by_ft(ft);
}

void aim_direct_connection_initiated(HANDLE hNewConnection, DWORD, void* extra)//for receiving stuff via dc
{
	 CAimProto* ppro = (CAimProto*)extra;
	 file_transfer *ft;

	NETLIBCONNINFO connInfo = { sizeof(connInfo) }; 
	CallService(MS_NETLIB_GETCONNECTIONINFO, (WPARAM)hNewConnection, (LPARAM)&connInfo);

	ppro->debugLogA("Buddy connected: %s", connInfo.szIpPort);

	// okay someone connected to us or we initiated the connection- we need to figure out who they are and if they belong
	ft = ppro->ft_list.find_by_port(connInfo.wPort);
	if (ft)
	{
		ft->hConn = hNewConnection;
		ppro->aim_dc_helper(ft);
	}
	else 
		Netlib_CloseHandle(hNewConnection);
}
