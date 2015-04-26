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

void __cdecl CAimProto::accept_file_thread(void* param)//buddy sending file
{
	file_transfer *ft = (file_transfer*)param;	

	HANDLE hConn = NULL;
	if (ft->peer_force_proxy)  //peer is forcing proxy
	{
		hConn = aim_peer_connect(ft->proxy_ip, get_default_port());
		if (hConn) 
		{
			debugLogA("Connected to proxy ip that buddy specified.");
			ft->hConn = hConn;
			ForkThread(&CAimProto::aim_proxy_helper, ft);
			ft->stop_listen();
		}
	}
	else if (ft->me_force_proxy) //we are forcing proxy
	{
		hConn = aim_peer_connect(AIM_PROXY_SERVER, get_default_port());
		if (hConn) 
		{
			debugLogA("Connected to proxy ip because we want to use a proxy for the file transfer.");
			ft->requester = true;
			ft->hConn = hConn;
			ForkThread(&CAimProto::aim_proxy_helper, ft);
			ft->stop_listen();
		}
	}
	else 
	{
		bool verif = ft->verified_ip != detected_ip;
		hConn = aim_peer_connect(verif ? ft->verified_ip : ft->local_ip, ft->port);
		if (hConn) 
		{
			debugLogA("Connected to buddy over P2P port via %s ip.", verif ? "verified": "local");
			ft->accepted = true;
			ft->hConn = hConn;
			aim_file_ad(hServerConn, seqno, ft->sn, ft->icbm_cookie, false, ft->max_ver);
			ForkThread(&CAimProto::aim_dc_helper, ft);
			ft->stop_listen();
		}
		else if (ft->sending)
		{
			hConn = aim_peer_connect(AIM_PROXY_SERVER, get_default_port());
			if (hConn) 
			{
				ft->hConn = hConn;
				ft->requester = true;
				ForkThread(&CAimProto::aim_proxy_helper, ft);
				ft->stop_listen();
			}
		}
		else
		{
			debugLogA("Failed to connect to buddy- asking buddy to connect to us.");
			ft->listen(this);
			ft->requester = true;
			aim_send_file(hServerConn, seqno, detected_ip, ft->local_port, false, ft);
			return;
		}
	}

	if (hConn == NULL)
	{
		if (ft->req_num)
		{
			aim_file_ad(hServerConn, seqno, ft->sn, ft->icbm_cookie, true, 0);
		}
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		ft_list.remove_by_ft(ft);
	}
}
