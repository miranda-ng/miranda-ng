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

void CAimProto::snac_md5_authkey(SNAC &snac,HANDLE hServerConn,unsigned short &seqno, const char* username, const char* password)//family 0x0017
{
	if (snac.subcmp(0x0007))//md5 authkey string
	{
		unsigned short length=snac.ushort();
		char* authkey = snac.part(2, length);
		aim_auth_request(hServerConn, seqno, authkey, AIM_LANGUAGE, AIM_COUNTRY, username, password);
		mir_free(authkey);
	}
}

int CAimProto::snac_authorization_reply(SNAC &snac)//family 0x0017
{
	int res = 0;
	
	if (snac.subcmp(0x0003))
	{
		char* server = NULL;
		int address = 0;
		unsigned short port;
		unsigned char use_ssl = 0;

		while (address < snac.len())
		{
			TLV tlv(snac.val(address));
			if (tlv.cmp(0x0005))
				server = tlv.dup();
			else if (tlv.cmp(0x0006))
			{
				Netlib_CloseHandle(hServerConn);

				if (server == NULL) return 3;
				char* delim = strchr(server, ':');
				port = delim ? (unsigned short)atoi(delim + 1) : get_default_port();
				if (delim) *delim = 0;

				hServerConn = aim_connect(server, port, use_ssl != 0, "bos.oscar.aol.com");
				if (hServerConn)
				{
					mir_free(COOKIE);
					COOKIE_LENGTH = tlv.len();
					COOKIE = tlv.dup();
					ForkThread(&CAimProto::aim_protocol_negotiation, 0);
					res = 1;
				}
				else
					res = 3;
				break;
			}
			else if (tlv.cmp(0x0008))
			{
				login_error(tlv.ushort());
				res = 2;
				break;
			}
			else if (tlv.cmp(0x0011))
			{
				char* email = tlv.dup();
				setString(AIM_KEY_EM, email);
				mir_free(email);
			}
			else if (tlv.cmp(0x008e))
			{
				use_ssl = tlv.ubyte();
			}
			address += tlv.len() + 4;
		}
		mir_free(server);
	} 
	return res;
}
void CAimProto::snac_supported_families(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)//family 0x0001
{
	if (snac.subcmp(0x0003))//server supported service list
	{
		aim_send_service_request(hServerConn,seqno);
	}
}
void CAimProto::snac_supported_family_versions(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)//family 0x0001
{
	if (snac.subcmp(0x0018))//service list okayed
	{
		aim_request_rates(hServerConn,seqno);//request some rate crap
	}
}
void CAimProto::snac_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)// family 0x0001
{
	if (snac.subcmp(0x0007))
	{
		aim_accept_rates(hServerConn,seqno);
		aim_request_icbm(hServerConn,seqno);
	}
}
void CAimProto::snac_mail_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)// family 0x0001
{
	if (snac.subcmp(0x0007))
	{
		aim_accept_rates(hServerConn,seqno);
		aim_request_mail(hServerConn,seqno);
		aim_activate_mail(hServerConn,seqno);
		aim_mail_ready(hServerConn,seqno);
	}
}

void CAimProto::snac_avatar_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)// family 0x0001
{
	if (snac.subcmp(0x0007))
	{
		aim_accept_rates(hServerConn,seqno);
		aim_avatar_ready(hServerConn,seqno);
		SetEvent(hAvatarEvent);
	}
}

void CAimProto::snac_chatnav_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)// family 0x0001
{
	if (snac.subcmp(0x0007))
	{
		aim_accept_rates(hServerConn,seqno);
		aim_chatnav_request_limits(hChatNavConn,chatnav_seqno); // Get the max number of rooms we're allowed in.
	}
}

void CAimProto::snac_chat_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)// family 0x0001
{
	if (snac.subcmp(0x0007))
	{
		aim_accept_rates(hServerConn,seqno);
		aim_chat_ready(hServerConn,seqno);
	}
}

void CAimProto::snac_icbm_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)//family 0x0004
{
	if (snac.subcmp(0x0005))
	{
		switch (m_iDesiredStatus)
		{
		case ID_STATUS_ONLINE:
			broadcast_status(ID_STATUS_ONLINE);
			aim_set_status(hServerConn,seqno,AIM_STATUS_ONLINE);
			break;

		case ID_STATUS_INVISIBLE:
			broadcast_status(ID_STATUS_INVISIBLE);
			aim_set_status(hServerConn,seqno,AIM_STATUS_INVISIBLE);
			break;

		case ID_STATUS_OCCUPIED:
			broadcast_status(ID_STATUS_OCCUPIED);
			aim_set_status(hServerConn,seqno,AIM_STATUS_BUSY|AIM_STATUS_AWAY);
			break;

		case ID_STATUS_AWAY:
			broadcast_status(ID_STATUS_AWAY);
			aim_set_status(hServerConn,seqno,AIM_STATUS_AWAY);
			break;
		}

		char** msgptr = get_status_msg_loc(m_iDesiredStatus);
		mir_free(last_status_msg);
		last_status_msg = msgptr ? mir_strdup(*msgptr) : NULL;
		aim_set_statusmsg(hServerConn, seqno, last_status_msg);

		if (m_iDesiredStatus == ID_STATUS_AWAY)
			aim_set_away(hServerConn, seqno, last_status_msg, true);

		if (getByte(AIM_KEY_II,0))
		{
			unsigned long time = getDword(AIM_KEY_IIT, 0);
			aim_set_idle(hServerConn,seqno,time*60);
			instantidle=1;
		}
		aim_request_list(hServerConn,seqno);
	}
}

void CAimProto::snac_self_info(SNAC &snac)//family 0x0001
{
	if (snac.subcmp(0x000f))
	{
		int offset = snac.flags() & 0x8000 ? snac.ushort(0) + 2 : 0;

		unsigned char sn_len = snac.ubyte(offset++);
		char* sn = snac.part(offset, sn_len);
		offset += sn_len + 2;

		int tlv_count = snac.ushort(offset);
		offset += 2;

		for (int i = 0; i < tlv_count; i++)
		{
			TLV tlv(snac.val(offset));
			offset += TLV_HEADER_SIZE + tlv.len();

			if (tlv.cmp(0x000a))
			{
				detected_ip = tlv.ulong();
			}
		}
		mir_free(sn);
	}
}

void CAimProto::snac_user_online(SNAC &snac)//family 0x0003
{
	if (snac.subcmp(0x000b))
	{
		char client[100] = "";
		bool hiptop_user = false;
		bool bot_user = false;
		bool wireless_user = false;
		bool away_user = false;
		bool caps_included = false;
		unsigned long status_type = 0;	// 0 = online

		char *hash_sm = NULL, *hash_lg = NULL;

		unsigned char sn_len = snac.ubyte();
		char* sn = snac.part(1, sn_len);
		MCONTACT hContact = contact_from_sn(sn, true);

		int offset = sn_len + 3;
		int tlv_count = snac.ushort(offset);
		offset += 2;

		for (int i = 0; i < tlv_count; i++)
		{
			TLV tlv(snac.val(offset));
			offset += TLV_HEADER_SIZE;
			if (tlv.cmp(0x0001))//user m_iStatus
			{
				unsigned short status = tlv.ushort();
				int unconfirmed = status & 0x0001;
				int admin_aol   = status & 0x0002;
				int aol         = status & 0x0004;
				//int nonfree   = status & 0x0008;
				//int aim       = status & 0x0010;
				int away        = status & 0x0020;
				int icq         = status & 0x0040;
				int wireless    = status & 0x0080;
				int bot         = status & 0x0400;
				setString(hContact, AIM_KEY_NK, sn);

				if (icq)
					setString(hContact, "Transport", "ICQ");
				else
					delSetting(hContact, "Transport" );

				if (admin_aol)
					setByte(hContact, AIM_KEY_AC, ACCOUNT_TYPE_ADMIN);
				else if (aol)
					setByte(hContact, AIM_KEY_AC, ACCOUNT_TYPE_AOL);	
				else if (icq)
					setByte(hContact, AIM_KEY_AC, ACCOUNT_TYPE_ICQ);	
				else if (unconfirmed)
					setByte(hContact, AIM_KEY_AC, ACCOUNT_TYPE_UNCONFIRMED);
				else
					setByte(hContact, AIM_KEY_AC, ACCOUNT_TYPE_CONFIRMED);

				if (bot)
				{
					strcpy(client,CLIENT_BOT);
					bot_user=1;
				}
				if (wireless)
				{
					strcpy(client,CLIENT_SMS);
					wireless_user=1;
				}
				else if (away)
				{
					away_user=1;
				}
				setDword(hContact, AIM_KEY_IT, 0);//erase idle time
				setDword(hContact, AIM_KEY_OT, 0);//erase online time
			}
			else if (tlv.cmp(0x0006))	// Status
			{
				status_type = tlv.ulong() & 0x00000FFF;
			}
			else if (tlv.cmp(0x000d))
			{
				caps_included = true;

				for(int i = 0; i<tlv.len(); i += 16)
				{
					char* cap = tlv.part(i,16);
					if (memcmp(cap, "MirandaM", 8) == 0)
					{
						char a =cap[8];
						char b =cap[9];
						char c =cap[10];
						char d =cap[11];
						char e =cap[12];
						char f =cap[13];
						char g =cap[14];
						char h =cap[15];
						mir_snprintf(client,SIZEOF(client),CLIENT_OSCARJ,a&0x7f,b,c,d,alpha_cap_str(a),e&0x7f,f,g,h,alpha_cap_str(e));
					}
					else if (memcmp(cap, "MirandaA", 8) == 0)
					{
						char a =cap[8];
						char b =cap[9];
						char c =cap[10];
						char d =cap[11];
						char e =cap[12];
						char f =cap[13];
						char g =cap[14];
						char h =cap[15];
						mir_snprintf(client,SIZEOF(client),CLIENT_AIMOSCAR,a,b,c,d,e,f,g,h);
					}
					if (memcmp(cap, "sinj", 4) == 0)
					{
						char a =cap[4];
						char b =cap[5];
						char c =cap[6];
						char d =cap[7];
						char e =cap[8];
						char f =cap[9];
						char g =cap[10];
						char h =cap[11];
						mir_snprintf(client,SIZEOF(client),CLIENT_OSCARSN,a&0x7f,b,c,d,alpha_cap_str(a),e&0x7f,f,g,h,alpha_cap_str(e),secure_cap_str(&cap[12]));
					}
					if (memcmp(cap, "icqp", 4) == 0)
					{
						char a =cap[4];
						char b =cap[5];
						char c =cap[6];
						char d =cap[7];
						char e =cap[8];
						char f =cap[9];
						char g =cap[10];
						char h =cap[11];
						mir_snprintf(client,SIZEOF(client),CLIENT_OSCARPL,a&0x7f,b,c,d,alpha_cap_str(a),e&0x7f,f,g,h,alpha_cap_str(e),secure_cap_str(&cap[12]));
					}
					else if (memcmp(cap, "Kopete ICQ", 10) == 0)
					{
						strcpy(client,CLIENT_KOPETE);
					}
					else if (memcmp(&cap[7], "QIP", 3) == 0)
					{
						strcpy(client,CLIENT_QIP);
					}
					else if (memcmp(cap, "mICQ", 4) == 0)
					{
						strcpy(client,CLIENT_MICQ);
					}
					else if (cap_cmp(cap, AIM_CAP_IM2) == 0)
					{
						strcpy(client,CLIENT_IM2);
					}
					else if (memcmp(cap, "SIM client", 10) == 0)
					{
						strcpy(client,CLIENT_SIM);
					}
					else if (memcmp(cap+4, "naim", 4) == 0)
					{
						strcpy(client,CLIENT_NAIM);
					}
					else if (memcmp(cap, "digsby", 6) == 0)
					{
						strcpy(client,CLIENT_DIGSBY);
					}
					mir_free(cap);
				}
			}
			else if (tlv.cmp(0x0019))//new caps
			{
				caps_included=1;
				bool f002=0, f003=0, f004=0, f005=0, f007=0, f008=0, 
					O101=0, O102=0, O103=0, O104=0, O105=0, O107=0, O1ff=0, 
					O10a=0, O10c=0, O10d=0,
					l341=0, l343=0, l345=0, l346=0, l347=0, l348=0, l349=0, l34b=0, l34e=0;
					//utf8=0;//O actually means 0 in this case
				for(int i=0;i<tlv.len();i=i+2)
				{
					unsigned short cap=tlv.ushort(i);
					//if (cap==0x134E)
					//	utf8=1;
					if (cap==0xf002)
						f002=1;
					if (cap==0xf003)
						f003=1;
					if (cap==0xf004)
						f004=1;
					if (cap==0xf005)
						f005=1;
					if (cap==0xf007)
						f007=1;
					if (cap==0xf008)
						f008=1;
					if (cap==0x0101)
						O101=1;
					if (cap==0x0102)
						O102=1;
					if (cap==0x0103)
						O103=1;
					if (cap==0x0104)
						O104=1;
					if (cap==0x0105)
						O105=1;
					if (cap==0x0107)
						O107=1;
					if (cap==0x010a)
						O10a=1;
					if (cap==0x010c)
						O10c=1;
					if (cap==0x010d)
						O10d=1;
					if (cap==0x01ff)
						O1ff=1;
					if (cap==0x1323)
					{
						strcpy(client,CLIENT_GPRS);
						hiptop_user=1;
					}
					if (cap==0x1341)
						l341=1;
					if (cap==0x1343)
						l343=1;
					if (cap==0x1345)
						l345=1;
					if (cap==0x1346)
						l346=1;
					if (cap==0x1347)
						l347=1;
					if (cap==0x1348)
						l348=1;
					if (cap==0x1349)
						l349=1;
					if (cap==0x134b)
						l34b=1;
					if (cap==0x134e)
						l34e=1;
				}
				if (f002&&f003&&f004&&f005)
					strcpy(client,CLIENT_TRILLIAN_PRO);
				else if ((f004&&f005&&f007&&f008) || (f004&&f005&&O104&&O105))
					strcpy(client,CLIENT_ICHAT);
				else if (f003&f004&f005)
					strcpy(client,CLIENT_TRILLIAN);
				else if (l343&&O1ff&&tlv.len()==4)
					strcpy(client,CLIENT_TRILLIAN_ASTRA);
				else if (l343&&tlv.len()==2)
					strcpy(client,CLIENT_AIMTOC);
				else if (l343&&l345&&l346&&tlv.len()==6)
					strcpy(client,CLIENT_GAIM);
				else if (l343&&l345&&l346&&l34e&&tlv.len()==8)
					strcpy(client,CLIENT_PURPLE);
				else if (l343&&l345&&l346&&l349&&l34e&&tlv.len()==10)
					strcpy(client,CLIENT_PURPLE);
				else if (l343&&l345&&l34e&&tlv.len()==6)
					strcpy(client,CLIENT_ADIUM);
				else if (l343&&l346&&l34e&&tlv.len()==6)
					strcpy(client,CLIENT_TERRAIM);
				else if (tlv.len()==0 && getWord(hContact, AIM_KEY_ST,0)!=ID_STATUS_ONTHEPHONE)
					strcpy(client,CLIENT_AIMEXPRESS5);	
				else if (l34b&&l343&&O1ff&&l345&&l346&&tlv.len()==10)
					strcpy(client,CLIENT_AIMEXPRESS6);	
				else if (l34b&&l341&&l343&&O1ff&&l345&&l346&&l347)
					strcpy(client,CLIENT_AIM5);
				else if (l34b&&l341&&l343&&l345&l346&&l347&&l348)
					strcpy(client,CLIENT_AIM4);
				else if (O1ff&&l343&&O107&&l341&&O104&&O105&&O101&&l346)
				{
					if (O10d)
						strcpy(client,CLIENT_AIM6_9);
					else if (O10c)
						strcpy(client,CLIENT_AIM6_8);
					else if (O10a)
						strcpy(client,CLIENT_AIM6_5);
					else
						strcpy(client,CLIENT_AIM_TRITON);
				}
				else if (O1ff&&l343&&l341&&O104&&O105&&O101&&l346)
					strcpy(client,CLIENT_AIM7_0);
				else if (l346&&l34e&&tlv.len()==4)
					strcpy(client,CLIENT_MEEBO);
				else if (l34e&&tlv.len()==2)
					strcpy(client,CLIENT_BEEJIVE);
				else if (l34e&&l343&&tlv.len()==4)
					strcpy(client,CLIENT_BEEJIVE);

				//	setByte(hContact, AIM_KEY_US, utf8);
			}
			else if (tlv.cmp(0x001d)) //bart
			{
				if (hContact)
				{
					bool msg_exist = false;
					for (int i = 0; i < tlv.len(); )
					{
						// Bart header
						unsigned short type = tlv.ushort(i);
						unsigned char flags = tlv.ubyte(i + 2);
						unsigned char datalen = tlv.ubyte(i + 3);

						switch (type)
						{
						case 0x0001:
							hash_sm = bytes_to_string(tlv.val() + i + 4, datalen);
							break;

						case 0x000c:
							hash_lg = bytes_to_string(tlv.val() + i + 4, datalen);
							break;

						case 0x0002:
							if ((flags & 4) && datalen > 2)
							{
								unsigned short len = tlv.ushort(i + 4);
								if (len)
								{
									msg_exist = true;
									char* msg = tlv.part(i + 6, len);
									char* msg_s = process_status_msg(msg, sn);
									db_set_utf(hContact, MOD_KEY_CL, OTH_KEY_SM, msg_s);

									TCHAR* tszMsg = mir_utf8decodeT(msg_s);
									ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, NULL, (LPARAM)tszMsg);
									mir_free(tszMsg);
									mir_free(msg);
									mir_free(msg_s);
								}
							}
							break;
						}
						i += 4 + datalen;
					}

					if (!msg_exist)
						db_unset(hContact, MOD_KEY_CL, OTH_KEY_SM);
				}
			}
			else if (tlv.cmp(0x0004))//idle tlv
			{
				if (hContact)
				{
					time_t current_time;
					time(&current_time);
					setDword(hContact, AIM_KEY_IT, ((DWORD)current_time) - tlv.ushort() * 60);
				}
			}
			else if (tlv.cmp(0x0003))//online time tlv
			{
				if (hContact)
					setDword(hContact, AIM_KEY_OT, tlv.ulong());
			}
			else if (tlv.cmp(0x0005))//member since 
			{
				if (hContact) 
					setDword(hContact, AIM_KEY_MS, tlv.ulong()); 
			}  			
			offset += tlv.len();
		}

		if (status_type & AIM_STATUS_INVISIBLE)
			setWord(hContact, AIM_KEY_ST, ID_STATUS_INVISIBLE);
		else if (status_type & AIM_STATUS_BUSY)
			setWord(hContact, AIM_KEY_ST, ID_STATUS_OCCUPIED);
		else if (status_type & AIM_STATUS_AWAY || away_user)
			setWord(hContact, AIM_KEY_ST, ID_STATUS_AWAY);
		else if (wireless_user)
			setWord(hContact, AIM_KEY_ST, ID_STATUS_ONTHEPHONE);
		else
			setWord(hContact, AIM_KEY_ST, ID_STATUS_ONLINE);

		if (hash_lg)
			avatar_request_handler(hContact, hash_lg, 12);
		else
			avatar_request_handler(hContact, hash_sm, 1);

		if (bot_user)
			setByte(hContact, AIM_KEY_ET, EXTENDED_STATUS_BOT);
		else if (hiptop_user)
			setByte(hContact, AIM_KEY_ET, EXTENDED_STATUS_HIPTOP);
		if (caps_included)
			set_contact_icon(this, hContact);

		if (caps_included || client[0])
			setString(hContact, AIM_KEY_MV, client[0] ? client : "?");
		else if (atoi(sn))
			setString(hContact, AIM_KEY_MV, CLIENT_ICQ);
		else if (getBool(hContact, AIM_KEY_BLS, false))
			setString(hContact, AIM_KEY_MV, CLIENT_BLAST);
		else
			setString(hContact, AIM_KEY_MV, CLIENT_AIMEXPRESS7);

		mir_free(hash_lg);
		mir_free(hash_sm);
		mir_free(sn);
	}
}

void CAimProto::snac_user_offline(SNAC &snac)//family 0x0003
{
	if (snac.subcmp(0x000c)) {
		unsigned char buddy_length = snac.ubyte();
		char* buddy=snac.part(1,buddy_length);
		MCONTACT hContact=contact_from_sn(buddy, true);
		if (hContact)
			offline_contact(hContact,0);
		mir_free(buddy);
	}
}
void CAimProto::snac_error(SNAC &snac)//family 0x0003 or 0x0004
{
	if (snac.subcmp(0x0001))
		get_error(snac.ushort());
}

void CAimProto::process_ssi_list(SNAC &snac, int &offset)
{
	unsigned short name_length = snac.ushort(offset);
	char* name = snac.part(offset+2, name_length);
	unsigned short group_id = snac.ushort(offset+ 2 +name_length);
	unsigned short item_id = snac.ushort(offset+4+name_length);
	unsigned short type = snac.ushort(offset+6+name_length);
	unsigned short tlv_size = snac.ushort(offset+8+name_length);
	const int tlv_base = offset + name_length + 10; 

	switch (type) {
	case 0x0000: //buddy record
		{
			MCONTACT hContact = contact_from_sn(name, true);
			if (hContact) {
				int i;
				for (i = 1;; i++) {
					if (!getBuddyId(hContact, i)) {
						setBuddyId(hContact, i, item_id);
						setGroupId(hContact, i, group_id);
						break;
					}
				}
				if (i == 1 && getByte(AIM_KEY_MG, 1)) {
					const char* group = group_list.find_name(group_id);
					if (group) {
						bool ok = false;
						DBVARIANT dbv;
						if (!db_get_utf(hContact, MOD_KEY_CL, OTH_KEY_GP, &dbv) && dbv.pszVal[0]) {
							ok = strcmp(group, dbv.pszVal) == 0;
							db_free(&dbv);
						}
						else ok = strcmp(group, AIM_DEFAULT_GROUP) == 0;

						if (!ok) {
							if (strcmp(group, AIM_DEFAULT_GROUP))
								db_set_utf(hContact, MOD_KEY_CL, OTH_KEY_GP, group);
							else
								db_unset(hContact, MOD_KEY_CL, OTH_KEY_GP);
						}
					}
				}
				setWord(hContact, AIM_KEY_ST, ID_STATUS_OFFLINE);

				bool nickfound = false;
				for (int tlv_offset = 0; tlv_offset < tlv_size;) {
					TLV tlv(snac.val(tlv_base + tlv_offset));

					if (tlv.cmp(0x0131) && tlv.len()) {
						char* nick = tlv.dup();
						db_set_utf(hContact, MOD_KEY_CL, "MyHandle", nick);
						mir_free(nick);
						nickfound = true;
					}
					else if (tlv.cmp(0x7b))
						setByte(hContact, AIM_KEY_BLS, 1);
					else if (tlv.cmp(0x6a))
						setByte(hContact, AIM_KEY_NIL, 1);

					tlv_offset += TLV_HEADER_SIZE + tlv.len();
				}
				if (!nickfound && getDword(AIM_KEY_LV, 0) >= 0x80500)
					db_unset(hContact, MOD_KEY_CL, "MyHandle");
			}
	}
		break;

	case 0x0001: //group record
		if (group_id) {
			group_list.add(name, group_id);
			if (getByte(AIM_KEY_MG, 1))
				create_group(name);
		}
		break;

	case 0x0002: //permit record
		allow_list.add(name, item_id);
		break;

	case 0x0003: //deny record
		block_list.add(name, item_id);
		break;

	case 0x0004: //privacy record
		if (group_id == 0) {
			pd_info_id = item_id;

			for (int tlv_offset = 0; tlv_offset < tlv_size;) {
				TLV tlv(snac.val(tlv_base + tlv_offset));

				if (tlv.cmp(0x00ca))
					pd_mode = tlv.ubyte();
				else if (tlv.cmp(0x00cc))
					pd_flags = tlv.ulong();

				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
		}
		break;

	case 0x0005: //prefernces record
		if (group_id == 0) {
			pref1_id = item_id;

			for (int tlv_offset = 0; tlv_offset < tlv_size;) {
				TLV tlv(snac.val(tlv_base + tlv_offset));

				if (tlv.cmp(0x00c9))
					pref1_flags = tlv.ulong();
				else if (tlv.cmp(0x00d6))
					pref1_set_flags = tlv.ulong();
				else if (tlv.cmp(0x00d7)) {
					mir_free(pref2_flags);
					pref2_flags = tlv.dup();
					pref2_len = tlv.len();
				}
				else if (tlv.cmp(0x00d8)) {
					mir_free(pref2_set_flags);
					pref2_set_flags = tlv.dup();
					pref2_set_len = tlv.len();
				}

				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
		}
		break;

	case 0x0014: //avatar record
		if (!mir_strcmp(name, "1") || !mir_strcmp(name, "12")) {
			if (name_length == 1)
				avatar_id_sm = item_id;
			else
				avatar_id_lg = item_id;

			for (int tlv_offset = 0; tlv_offset < tlv_size;) {
				TLV tlv(snac.val(tlv_base + tlv_offset));

				if (tlv.cmp(0x00d5) && tlv.len() > 2) {
					if (name_length == 1) {
						mir_free(hash_sm);
						hash_sm = bytes_to_string(tlv.val() + 2, tlv.ubyte(1));
					}
					else {
						mir_free(hash_lg);
						hash_lg = bytes_to_string(tlv.val() + 2, tlv.ubyte(1));
					}
				}

				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
			if (list_received)
				avatar_request_handler(NULL, NULL, 0);
		}
		break;

	case 0x001D: // Vanity information
		if (group_id == 0) {
			for (int tlv_offset = 0; tlv_offset < tlv_size;) {
				TLV tlv(snac.val(tlv_base + tlv_offset));

				if (tlv.cmp(0x0150))		// Number of IMs sent
					setDword(AIM_KEY_TIS, tlv.ulong());
				else if (tlv.cmp(0x0151))	// Number of seconds a user is online
					setDword(AIM_KEY_TTO, tlv.ulong());
				else if (tlv.cmp(0x0152))	// Number of times a user has the away message set
					setDword(AIM_KEY_TAM, tlv.ulong());
				else if (tlv.cmp(0x0153))	// Number of IMs received
					setDword(AIM_KEY_TIR, tlv.ulong());

				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
		}
		break;
	}

	mir_free(name);

	offset = tlv_base + tlv_size;
}

void CAimProto::modify_ssi_list(SNAC &snac, int &offset)
{
	unsigned short name_length = snac.ushort(offset);
	char* name = snac.part(offset+2, name_length);
	unsigned short group_id = snac.ushort(offset+ 2 +name_length);
	unsigned short item_id = snac.ushort(offset+4+name_length);
	unsigned short type = snac.ushort(offset+6+name_length);
	unsigned short tlv_size = snac.ushort(offset+8+name_length);
	const int tlv_base = offset + name_length + 10; 

	switch (type)
	{
	case 0x0000: //buddy record
		{
			MCONTACT hContact = contact_from_sn(name, true);
			if (hContact)
			{
				for (int tlv_offset = 0; tlv_offset < tlv_size; )
				{
					TLV tlv(snac.val(tlv_base + tlv_offset));

					if (tlv.cmp(0x0131) && tlv.len())
					{
						char* nick = tlv.dup();
						if (nick)
							db_set_utf(hContact, MOD_KEY_CL, "MyHandle", nick);
						else
							db_unset(hContact, MOD_KEY_CL, "MyHandle");
						mir_free(nick);
					}

					tlv_offset += TLV_HEADER_SIZE + tlv.len();
				}
			}
			break;
		}

	case 0x0004: //privacy record
		if (group_id == 0)
		{
			pd_info_id = item_id;

			for (int tlv_offset = 0; tlv_offset < tlv_size; )
			{
				TLV tlv(snac.val(tlv_base + tlv_offset));

				if (tlv.cmp(0x00ca))
					pd_mode = tlv.ubyte();
				else if (tlv.cmp(0x00cc))
					pd_flags = tlv.ulong();

				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
		}
		break;

	case 0x0005: //prefernces record
		if (group_id == 0)
		{
			pref1_id = item_id;

			for (int tlv_offset = 0; tlv_offset < tlv_size; )
			{
				TLV tlv(snac.val(tlv_base + tlv_offset));

				if (tlv.cmp(0x00c9))
					pref1_flags = tlv.ulong();
				else if (tlv.cmp(0x00d6))
					pref1_set_flags = tlv.ulong();
				else if (tlv.cmp(0x00d7))
				{
					mir_free(pref2_flags);
					pref2_flags = tlv.dup();
					pref2_len = tlv.len();
				}
				else if (tlv.cmp(0x00d8))
				{
					mir_free(pref2_set_flags);
					pref2_set_flags = tlv.dup();
					pref2_set_len = tlv.len();
				}

				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
		}
		break;

	case 0x0014: //avatar record
		if (!mir_strcmp(name, "1") || !mir_strcmp(name, "12"))
		{
			if (name_length == 1)
				avatar_id_sm = item_id;
			else
				avatar_id_lg = item_id;

			for (int tlv_offset = 0; tlv_offset < tlv_size; )
			{
				TLV tlv(snac.val( tlv_base + tlv_offset));

				if (tlv.cmp(0x00d5) && tlv.len() > 2)
				{
					unsigned char type = tlv.ubyte(0);
					if (name_length == 1)
					{
						mir_free(hash_sm);
						hash_sm = bytes_to_string(tlv.val() + 2, tlv.ubyte(1));
					}
					else
					{
						mir_free(hash_lg);
						hash_lg = bytes_to_string(tlv.val() + 2, tlv.ubyte(1));
					}
				}
				tlv_offset += TLV_HEADER_SIZE + tlv.len();
			}
			avatar_request_handler(NULL, NULL, 0);
		}
		break;
	}

	mir_free(name);
}

void CAimProto::delete_ssi_list(SNAC &snac, int &offset)
{
	int i;

	unsigned short name_length=snac.ushort(offset);
	char* name=snac.part(offset+2,name_length);
	unsigned short group_id=snac.ushort(offset+2+name_length);
	unsigned short item_id=snac.ushort(offset+4+name_length);
	unsigned short type=snac.ushort(offset+6+name_length);

	MCONTACT hContact = contact_from_sn(name);

	switch (type) {
	case 0x0000: //buddy record
		for(i=1;;++i)
		{
			unsigned short item_id_st = getBuddyId(hContact, i);
			if (item_id_st == 0) break;

			if (item_id == item_id_st)
			{
				deleteBuddyId(hContact, i);
				deleteGroupId(hContact, i);
				--i;
			}
		}
		if (i == 1)
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		break;

	case 0x0001: //group record
		group_list.remove_by_id(group_id);
		break;

	case 0x0014: //avatar record
		if (mir_strcmp(name, "1"))
		{
			avatar_id_sm = 0;
			mir_free(hash_sm);
			hash_sm = NULL;
		}
		else if (!mir_strcmp(name, "12"))
		{
			avatar_id_lg = 0;
			mir_free(hash_lg);
			hash_lg = NULL;
		}
		avatar_request_handler(NULL, NULL, 0);
		break;
	}
	mir_free(name) ;
}

void CAimProto::snac_contact_list(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)//family 0x0013
{
	if (snac.subcmp(0x0006))  //contact list
	{
		debugLogA("Contact List Received");
//      unsigned char ver = snac.ubyte();
		int num_obj = snac.ushort(1);

		int offset=3;
		for (int i=0; i<num_obj; ++i)
			process_ssi_list(snac, offset);

		if (!list_received)//because they can send us multiple buddy list packets
		{//only want one finished connection
			list_received=1;
			aim_activate_list(hServerConn,seqno);
			aim_set_caps(hServerConn,seqno);
			aim_set_icbm(hServerConn,seqno);
			aim_client_ready(hServerConn,seqno);
			aim_request_offline_msgs(hServerConn,seqno);

			DBVARIANT dbv;
			if (!db_get_utf(NULL, m_szModuleName, AIM_KEY_PR, &dbv))
			{
				aim_set_profile(hServerConn, seqno, dbv.pszVal);
				db_free(&dbv);
			}

			if (getDword(AIM_KEY_LV, 0) < 0x80500)
			{
				upload_nicks();
				setDword(AIM_KEY_LV, (DWORD)CallService(MS_SYSTEM_GETVERSION,0,0));
			}

			if (getByte(AIM_KEY_CM, 0))
				aim_new_service_request(hServerConn, seqno, 0x0018);//mail

			avatar_request_handler(NULL, NULL, 0);

			debugLogA("Connection Negotiation Finished");
			state = 1;
		}
	}
	else if (snac.subcmp(0x0008)) // add buddy
	{
		int offset=8;
		process_ssi_list(snac, offset);
	}
	else if (snac.subcmp(0x0009)) // modify buddy
	{
		int offset=8;
		modify_ssi_list(snac, offset);
	}
	else if (snac.subcmp(0x000a)) // delete buddy
	{
		int offset=8;
		delete_ssi_list(snac, offset);
	}
}

void CAimProto::snac_message_accepted(SNAC &snac)//family 0x004
{
	if (snac.subcmp(0x000c))
	{

		char* icbm_cookie = snac.part(0,8);
		unsigned char sn_length=snac.ubyte(10);
		char* sn = snac.part(11,sn_length);

		MCONTACT hContact = contact_from_sn(sn);
		if (hContact)
		{
			msg_ack_param *msg_ack = (msg_ack_param*)mir_alloc(sizeof(msg_ack_param));
			msg_ack->hContact = hContact;
			msg_ack->id = *(int*)icbm_cookie & 0x7fffffff;
			msg_ack->msg = NULL;
			msg_ack->success = true;
			ForkThread(&CAimProto::msg_ack_success, msg_ack);
		}

		mir_free(sn);
		mir_free(icbm_cookie);
	}
}
void CAimProto::snac_received_message(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)//family 0x0004
{
	if (snac.subcmp(0x0007))
	{   
		unsigned short channel  = snac.ushort(8);
		unsigned char sn_length = snac.ubyte(10);
		char* sn                = snac.part(11,sn_length);

		MCONTACT hContact = contact_from_sn(sn, true, true), hMsgContact = NULL;

		int offset=15+sn_length;

		char* msg_buf=NULL;
		unsigned long offline_timestamp = 0;
		bool is_offline = false;
		//file transfer stuff
		char* icbm_cookie = NULL;
		char* filename = NULL;
		unsigned __int64 file_size=0;
		bool auto_response=false;
		bool force_proxy=false;
		bool descr_included=false;
		bool utf_fname=false;
		bool unicode_descr=false;
		short rdz_msg_type=-1;
		unsigned short request_num=0;
		unsigned long local_ip=0, verified_ip=0, proxy_ip=0;
		unsigned short port = 0;
		unsigned short max_ver = 0;
		unsigned short num_files = 0;
		//end file transfer stuff

		unsigned short tlv_head_num=snac.ushort(offset-2);
		for (int i=0;i<tlv_head_num;i++)
		{ // skip server-added TLVs - prevent another problems with parsing
			TLV tlv(snac.val(offset));
			offset+=TLV_HEADER_SIZE+tlv.len();
			// some extra sanity
			if (offset>=snac.len()) break;
		}
		
		while (offset < snac.len())
		{
			TLV tlv(snac.val(offset));
			offset += TLV_HEADER_SIZE;
			if (tlv.cmp(0x0004)&&!tlv.len())//auto response flag
				auto_response = 1;

			if (tlv.cmp(0x0002)) //msg
			{
				unsigned short caps_length=tlv.ushort(2);
				unsigned short msg_length=tlv.ushort(6+caps_length)-4;
				unsigned short encoding=tlv.ushort(8+caps_length);
				char* buf = tlv.part(12+caps_length,msg_length);
				if (hContact) {
					wchar_t* wbuf;
					hMsgContact = hContact;
					switch (encoding) {
					case 2:
						wbuf = (wchar_t*)buf;
						wcs_htons(wbuf);

						msg_buf = mir_utf8encodeW(wbuf);
						mir_free(wbuf);
						break;

					case 3:
						wbuf = mir_a2u_cp(buf, 28591);

						msg_buf = mir_utf8encodeW(wbuf);
						mir_free(wbuf);
						mir_free(buf);
						break;

					default:
						msg_buf = buf;
						break;
					}
				}
			}

			if (tlv.cmp(0x0004) && !tlv.len())//auto response flag
				auto_response = 1;

			if (channel == 2 && tlv.cmp(0x0005)) { //recv rendervous packet
				rdz_msg_type = snac.ushort(offset);
				icbm_cookie = snac.part(offset+2,8);
				if (cap_cmp(snac.val(offset+10), AIM_CAP_FILE_TRANSFER) == 0) {
					for (int i = 26; i < tlv.len(); ) {
						TLV tlv(snac.val(offset+i));
						if (tlv.cmp(0x000A))
							request_num=tlv.ushort();//for file transfer
						else if (tlv.cmp(0x0002))//proxy ip
							proxy_ip = tlv.ulong();
						else if (tlv.cmp(0x0003))//client ip
							local_ip = tlv.ulong();
						else if (tlv.cmp(0x0004))//verified ip
							verified_ip = tlv.ulong();
						else if (tlv.cmp(0x0005))
							port=tlv.ushort();
						else if (tlv.cmp(0x0010))
							force_proxy=1;
						else if (tlv.cmp(0x0012))
							max_ver = tlv.ushort();
						else if (tlv.cmp(0x2711)) {
							num_files = tlv.ushort(2);
							file_size = tlv.ulong(4);
							filename  = tlv.part(8, tlv.len()-8);
						}
						else if (tlv.cmp(0x2712)) {
							char* enc = tlv.dup();
							utf_fname = strcmp(enc, "utf-8") == 0;
							mir_free(enc);
						}
						else if (tlv.cmp(0x2713)) {
							file_size = tlv.u64();
						}
						else if (tlv.cmp(0x000c)) {
							msg_buf = unicode_descr ? tlv.dupw() : tlv.dup();
							html_decode(msg_buf);
							descr_included = true;
							if (strstr(msg_buf, "<ICQ_COOL_FT>")) {
								char* beg = strstr(msg_buf, "<DESC>");
								char* end = strstr(msg_buf, "</DESC>");
								if (beg && end && beg < end) {
									beg += 6;
									end[0] = 0;
									memmove(msg_buf, beg, end - beg + 1);
								}
								else descr_included = false;
							}
						}
						else if (tlv.cmp(0x000d)) {
							char* enc = tlv.dup();
							unicode_descr = strcmp(enc, "unicode-2-0") == 0;
							mir_free(enc);
						}
						i += TLV_HEADER_SIZE + tlv.len();
					}
				}
				else if (cap_cmp(snac.val(offset+10), AIM_CAP_RTCAUDIO) == 0 || cap_cmp(snac.val(offset+10), AIM_CAP_RTCVIDEO) == 0) {
					for (int i = 26; i < tlv.len(); ) {
						TLV tlv(snac.val(offset+i));
						if (tlv.cmp(0x000A))
							request_num=tlv.ushort();//for file transfer
						else if (tlv.cmp(0x0002))//proxy ip
							proxy_ip = tlv.ulong();
						else if (tlv.cmp(0x0003))//client ip
							local_ip = tlv.ulong();
						else if (tlv.cmp(0x0004))//verified ip
							verified_ip = tlv.ulong();
						else if (tlv.cmp(0x0005))
							port=tlv.ushort();
					}
					channel = 0;
					break;
				}
				else if (cap_cmp(snac.val(offset+10), AIM_CAP_CHAT) == 0) { //it's a chat invite request
					for(int i=26; i < tlv.len();) {
						TLV tlv(snac.val(offset+i));
						if (tlv.cmp(0x000c)) //optional message
							msg_buf = tlv.dup();		
						else if (tlv.cmp(0x2711)) { //room information
							int cookie_len=tlv.ubyte(2);
							chatnav_param* par = 
								new chatnav_param(tlv.part(3,cookie_len), tlv.ushort(), tlv.ushort(3+cookie_len),
								msg_buf, sn, icbm_cookie);

							invite_chat_req_param* chat_rq = new invite_chat_req_param(par, this, msg_buf, sn, icbm_cookie);
							CallFunctionAsync(chat_request_cb, chat_rq);
						}
						i+=TLV_HEADER_SIZE+tlv.len();
					}
				}
				else {
					channel = 0;
					break;
				}
			}

			if (channel == 6 && tlv.cmp(0x0005))//audio/video tunnel
				msg_buf = tlv.dup();		

			if (tlv.cmp(0x0006))//Offline message flag
				is_offline = true;

			if (tlv.cmp(0x0016))//Offline message timestamp
				offline_timestamp = tlv.ulong(0);

			offset += (tlv.len());
		}

		if (channel == 1) { //Message not file
			if (auto_response) { //this message must be an autoresponse
				char* away = mir_utf8encodeT(TranslateT("[Auto-Response]:"));
				size_t len = strlen(msg_buf) + strlen(away) + 2;
				char* buf = (char*)mir_alloc(len);
				mir_snprintf(buf, len, "%s %s", away, msg_buf);
				mir_free(away);
				mir_free(msg_buf);
				msg_buf = buf;
			}

			// Okay we are setting up the structure to give the message back to miranda's core
			CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hMsgContact, 0);
			{
				PROTORECVEVENT pre = { 0 };
				pre.flags = PREF_UTF;
				pre.timestamp = (is_offline) ? offline_timestamp : (DWORD)time(0);
				pre.szMessage = msg_buf;
				ProtoChainRecvMsg(hMsgContact, &pre);
			}

			if (m_iStatus==ID_STATUS_AWAY && !auto_response && !getByte(AIM_KEY_DM,0))
			{
				unsigned long msg_time = getDword(hContact, AIM_KEY_LM, 0);
				unsigned long away_time = getDword(AIM_KEY_LA, 0);
				char** msgptr = get_status_msg_loc(m_iStatus);
				if (away_time > msg_time && *msgptr)
				{
					char* s_msg = process_status_msg(*msgptr, sn);

					char* away = mir_utf8encodeT(TranslateT("[Auto-Response]:"));
					size_t len = strlen(s_msg) + strlen(away) + 2;
					char* buf = (char*)alloca(len);
					mir_snprintf(buf, len, "%s %s", away, s_msg);
					mir_free(away);

					DBEVENTINFO dbei = { sizeof(dbei) };
					dbei.szModule = m_szModuleName;
					dbei.timestamp = (DWORD)time(NULL);
					dbei.flags = DBEF_SENT | DBEF_UTF;
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.cbBlob = (int)len;
					dbei.pBlob = (PBYTE)buf;
					db_event_add(hContact, &dbei);

					aim_send_message(hServerConn, seqno, sn, s_msg, true, getBool(hContact, AIM_KEY_BLS, false));
					mir_free(s_msg);
				}
				setDword(hContact, AIM_KEY_LM, (DWORD)time(NULL));
			}
		}
		else if (channel == 2) // File Transfer
		{
			if (rdz_msg_type == 0 && request_num == 1) //buddy wants to send us a file
			{
				debugLogA("Buddy Wants to Send us a file. Request 1");
				debugLogA(force_proxy ? "Forcing a Proxy File transfer." : "Not forcing Proxy File transfer.");

				file_transfer* ft = new file_transfer(hContact, sn, icbm_cookie);

				ft->me_force_proxy = getByte(AIM_KEY_FP, 0) != 0;
				ft->peer_force_proxy = force_proxy;
				ft->local_ip = local_ip;
				ft->verified_ip = verified_ip;
				ft->proxy_ip = proxy_ip;
				ft->port = port;
				ft->max_ver = max_ver;
				ft->req_num = request_num;

				ft->file = mir_strdup(filename);
				
				ft->pfts.totalBytes = file_size;
				ft->pfts.totalFiles = num_files;

				ft_list.insert(ft);

				if (!descr_included) msg_buf = NULL;

				TCHAR* filenameT = mir_utf8decodeT(filename);

				PROTORECVFILET pre = {0};
				pre.flags = PREF_TCHAR;
				pre.fileCount = 1;
				pre.timestamp = time(NULL);
				pre.tszDescription = mir_utf8decodeT(msg_buf);
				pre.ptszFiles = &filenameT;
				pre.lParam = (LPARAM)ft;
				ProtoChainRecvFile(hContact, &pre);

				mir_free(pre.tszDescription);
				mir_free(filenameT);

				char cip[20];
				debugLogA("Local IP: %s:%u", long_ip_to_char_ip(local_ip, cip), port);
				debugLogA("Verified IP: %s:%u", long_ip_to_char_ip(verified_ip, cip), port);
				debugLogA("Proxy IP: %s:%u", long_ip_to_char_ip(proxy_ip, cip), port);
			}
			else if (rdz_msg_type == 0)
			{
				debugLogA("We are sending a file. Buddy wants us to connect to them. Request %d", request_num);
				debugLogA(force_proxy ? "Forcing a Proxy File transfer." : "Not forcing Proxy File transfer.");

				file_transfer* ft = ft_list.find_by_cookie(icbm_cookie, hContact);
				if (ft)
				{
					ft->hContact = hContact;

					ft->me_force_proxy |= (request_num > 2);
					ft->peer_force_proxy = force_proxy;
					ft->local_ip = local_ip;
					ft->verified_ip = verified_ip;
					ft->proxy_ip = proxy_ip;
					ft->port = port;
					ft->requester = false;
					ft->req_num = request_num;
					ft->max_ver = max_ver;

					char cip[20];
					debugLogA("Local IP: %s:%u", long_ip_to_char_ip(local_ip, cip), port);
					debugLogA("Verified IP: %s:%u", long_ip_to_char_ip(verified_ip, cip), port);
					debugLogA("Proxy IP: %s:%u", long_ip_to_char_ip(proxy_ip, cip), port);

					ForkThread(&CAimProto::accept_file_thread, ft);
				}
				else
				{
					debugLogA("Unknown File transfer, thus denied.");
					aim_file_ad(hServerConn, seqno, sn, icbm_cookie, true, 0);
				}
			}
			else if (rdz_msg_type == 1)//buddy cancelled or denied file transfer
			{
				debugLogA("File transfer cancelled or denied.");

				file_transfer* ft = ft_list.find_by_cookie(icbm_cookie, hContact);
				ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ft, 0);
				ft_list.remove_by_ft(ft);
			}
			else if (rdz_msg_type == 2)//buddy accepts our file transfer request
			{
				debugLogA("File transfer accepted");
				file_transfer* ft = ft_list.find_by_cookie(icbm_cookie, hContact);
				if (ft) 
				{
					ft->accepted  = true;
					ft->max_ver = max_ver;
				}
				else
					aim_file_ad(hServerConn, seqno, sn, icbm_cookie, true, 0);
			}
		}
		else if (channel == 6) // Audio/Video call
		{
			aim_file_ad(hServerConn, seqno, sn, icbm_cookie, true, 0);
			ShowPopup(LPGEN("Contact tried to open an audio/video conference (not currently supported)"), ERROR_POPUP);
		}

		mir_free(sn);
		mir_free(msg_buf);
		mir_free(filename);
		mir_free(icbm_cookie);
	}
}

void CAimProto::snac_file_decline(SNAC &snac)//family 0x0004
{
	if (snac.subcmp(0x000b))
	{ 
		char *icbm_cookie = snac.part(0, 8);
		int channel = snac.ushort(8);
		if (channel == 0x01)
		{
			int sn_len = snac.ubyte(10);
			char* sn   = snac.part(11, sn_len);
			int reason = snac.ushort(11 + sn_len);
			MCONTACT hContact = contact_from_sn(sn);

			msg_ack_param *msg_ack = (msg_ack_param*)mir_alloc(sizeof(msg_ack_param));
			msg_ack->hContact = hContact;
			msg_ack->msg = NULL;
			msg_ack->id = *(int*)icbm_cookie & 0x7fffffff;
			msg_ack->success = false;
			ForkThread(&CAimProto::msg_ack_success, msg_ack);
		}
		if (channel == 0x02)
		{
			int sn_len = snac.ubyte(10);
			char* sn   = snac.part(11, sn_len);
			int reason = snac.ushort(11 + sn_len);
			if (reason == 0x03)
			{
				int error = snac.ushort(13 + sn_len);
				if (error == 0x02)
				{
					debugLogA("File Transfer declied");
					MCONTACT hContact = contact_from_sn(sn);
					file_transfer *ft = ft_list.find_by_cookie(icbm_cookie, hContact);
					if (ft)
					{
						ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ft, 0);
						if (ft->hConn) Netlib_Shutdown(ft->hConn);
						else ft_list.remove_by_ft(ft);
					}
				}
			}
			mir_free(sn);
		}
		mir_free(icbm_cookie);
	}
}
void CAimProto::snac_received_info(SNAC &snac)//family 0x0002
{
	if (snac.subcmp(0x0006))
	{   
		unsigned short offset = 0;
		int i = 0;
		bool away_message_received = false;
		bool away_message_unicode = false;
		bool away_message_utf = false;
		bool profile_received = false;
		bool profile_unicode = false;
		bool profile_utf = false;
		unsigned char sn_length = snac.ubyte();
		char* sn = snac.part(1, sn_length);
		unsigned short tlv_count = snac.ushort(3 + sn_length);
		offset = 5 + sn_length;
		MCONTACT hContact = contact_from_sn(sn, true, true);
		
		while (offset < snac.len())
		{
			TLV tlv(snac.val(offset));

			if (++i > tlv_count)
			{
				if (tlv.cmp(0x0001))//profile encoding
				{
					char* enc = tlv.dup();
					profile_unicode = strstr(enc, "unicode-2-0") != NULL;
					profile_utf = strstr(enc, "utf-8") != NULL;
					mir_free(enc);
				}
				else if (tlv.cmp(0x0002))//profile message string
				{
					char* msg = profile_unicode ? tlv.dupw() : tlv.dup();

					profile_received = true;
					write_profile(sn, msg, profile_unicode | profile_utf);
					mir_free(msg);
				}
				else if (tlv.cmp(0x0003))//away message encoding
				{
					char* enc = tlv.dup();
					away_message_unicode = strstr(enc, "unicode-2-0") != NULL;
					away_message_utf = strstr(enc, "utf-8") != NULL;
					mir_free(enc);
				}
				else if (tlv.cmp(0x0004))//away message string
				{
					char* msg = away_message_unicode ? tlv.dupw() : tlv.dup();

					away_message_received = true;
					write_away_message(sn, msg, away_message_unicode | away_message_utf);
					mir_free(msg);
				}
			}
			offset += TLV_HEADER_SIZE + tlv.len();
		}
		if (hContact)
		{
			if (getWord(hContact,AIM_KEY_ST,ID_STATUS_OFFLINE) == ID_STATUS_AWAY)
			{
				if (!away_message_received && request_away_message)
					write_away_message(sn,Translate("No information has been provided by the server."),false);
				request_away_message = 0;
			}
			if (!profile_received&&request_HTML_profile)
				write_profile(sn,"No Profile",false);
			request_HTML_profile=0;
		}
		mir_free(sn);
	}
}
void CAimProto::snac_typing_notification(SNAC &snac)//family 0x004
{
	if (snac.subcmp(0x0014))
	{
		unsigned char sn_length=snac.ubyte(10);
		char* sn=snac.part(11,sn_length);
		MCONTACT hContact=contact_from_sn(sn);
		if (hContact)
		{
			unsigned short type=snac.ushort(11+sn_length);
			if (type==0x0000)//typing finished
				CallService(MS_PROTO_CONTACTISTYPING,hContact,(WPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			else if (type==0x0001)//typed
				CallService(MS_PROTO_CONTACTISTYPING,hContact,PROTOTYPE_CONTACTTYPING_INFINITE);
			else if (type==0x0002)//typing
				CallService(MS_PROTO_CONTACTISTYPING,hContact,(LPARAM)60);
		}
		mir_free(sn);
	}
}
void CAimProto::snac_list_modification_ack(SNAC &snac)//family 0x0013
{
	if (snac.subcmp(0x000e))
	{
		unsigned short id = snac.id();
		TLV tlv(snac.val(2));
		unsigned short code = snac.ushort(6 + tlv.len());
//		ssi_queue.execute(this, code == 0);
		switch (id)		
		{
		case 0x000a:
			switch (code)
			{
			case 0x0000:
//				ShowPopup(LPGEN("Successfully removed buddy from list."), ERROR_POPUP);
				break;

			case 0x0002:
				ShowPopup(LPGEN("Item you want to delete not found in list."), ERROR_POPUP);
				break;

			default:
				char msg[64];
				mir_snprintf(msg, SIZEOF(msg), "Error removing buddy from list. Error code %#x", code);
				ShowPopup(msg, ERROR_POPUP);
				break;
			}
			break;

		case 0x0008:
			switch (code)
			{
			case 0x0000:
//				ShowPopup("Successfully added buddy to list.", ERROR_POPUP);
				break;

			case 0x0003:
				ShowPopup(LPGEN("Failed to add buddy to list: Item already exist."), ERROR_POPUP);
				break;

			case 0x000a:
				ShowPopup(LPGEN("Error adding buddy (invalid ID or already in list?)"), ERROR_POPUP);
				break;

			case 0x000c:
				ShowPopup(LPGEN("Cannot add buddy. Limit for this type of item exceeded."), ERROR_POPUP);
				break;

			case 0x000d:
				ShowPopup(LPGEN("Error? Attempting to add ICQ contact to an AIM list."), ERROR_POPUP);
				break;

			case 0x000e:
				ShowPopup(LPGEN("Cannot add this buddy because it requires authorization."), ERROR_POPUP);
				break;

			default:
				char msg[64];
				mir_snprintf(msg, SIZEOF(msg), Translate("Unknown error when adding buddy to list. Error code %#x"), code);
				ShowPopup(msg, ERROR_POPUP);
				break;
			}
			break;

		case 0x0009:
			switch (code)
			{
			case 0x0000:
			case 0x000e:
//				ShowPopup(LPGEN("Successfully modified group."), ERROR_POPUP);
				break;

			case 0x0002:
				ShowPopup(LPGEN("Item you want to modify not found in list."), ERROR_POPUP);
				break;

			default:
				char msg[64];
				mir_snprintf(msg, SIZEOF(msg), Translate("Unknown error when attempting to modify a group. Error code %#x"), code);
				ShowPopup(msg, ERROR_POPUP);
				break;
			}
			break;
		}
	}
}

void CAimProto::snac_service_redirect(SNAC &snac)//family 0x0001
{
	if (snac.subcmp(0x0005))
	{
		char* server=NULL;
		char* local_cookie=NULL;
		char* host=NULL;
		int local_cookie_length=0;
		unsigned short family=0;
		unsigned char use_ssl=0;

		int offset=2; // skip number of bytes in family version tlv
		while (offset < snac.len())
		{
			TLV tlv(snac.val(offset));
			if (tlv.cmp(0x000d))
			{
				family=tlv.ushort();
			}
			else if (tlv.cmp(0x0005))
			{
				server=tlv.dup();
			}
			else if (tlv.cmp(0x0006))
			{
				local_cookie=tlv.dup();
				local_cookie_length=tlv.len();
			}
			else if (tlv.cmp(0x008d))
			{
				host=tlv.dup();
			}
			else if (tlv.cmp(0x008e))
			{
				use_ssl=tlv.ubyte();
			}
			offset+=TLV_HEADER_SIZE+tlv.len();
		}
		if (family == 0x0018)
		{
			hMailConn = aim_connect(server, get_default_port(), false/*use_ssl != 0*/, host);
			if (hMailConn)
			{
				debugLogA("Successfully Connected to the Mail Server.");
				MAIL_COOKIE=local_cookie;
				MAIL_COOKIE_LENGTH=local_cookie_length;
				ForkThread( &CAimProto::aim_mail_negotiation, 0 );
			}
			else
				debugLogA("Failed to connect to the Mail Server.");
		}
		else if (family == 0x0010)
		{
			hAvatarConn = aim_connect(server, get_default_port(), false/*use_ssl != 0*/);
			if (hAvatarConn)
			{
				debugLogA("Successfully Connected to the Avatar Server.");
				AVATAR_COOKIE = local_cookie;
				AVATAR_COOKIE_LENGTH = local_cookie_length;
				ForkThread( &CAimProto::aim_avatar_negotiation, 0 );
			}
			else
				debugLogA("Failed to connect to the Avatar Server.");
		}
		else if (family == 0x000D)
		{
			hChatNavConn = aim_connect(server, get_default_port(), use_ssl != 0, host);
			if (hChatNavConn)
			{
				debugLogA("Successfully Connected to the Chat Navigation Server.");
				CHATNAV_COOKIE = local_cookie;
				CHATNAV_COOKIE_LENGTH = local_cookie_length;
				ForkThread( &CAimProto::aim_chatnav_negotiation, 0 );
			}
			else
				debugLogA("Failed to connect to the Chat Navigation Server.");

		}
		else if (family == 0x000E)
		{
			chat_list_item* item = find_chat_by_cid(snac.idh());
			if (item)
			{
				item->hconn = aim_connect(server, get_default_port(), use_ssl != 0, host);
				if (item->hconn)
				{
					debugLogA("Successfully Connected to the Chat Server.");
					chat_start(item->id, item->exchange);
					item->CHAT_COOKIE = local_cookie;
					item->CHAT_COOKIE_LENGTH = local_cookie_length;
					ForkThread( &CAimProto::aim_chat_negotiation, item );
				}
				else
					debugLogA("Failed to connect to the Chat Server.");
			}
		}
		else if (family == 0x0007)
		{
			hAdminConn = aim_connect(server, get_default_port(), false /*use_ssl != 0*/);
			if (hAdminConn)
			{
				debugLogA("Successfully Connected to the Admin Server.");
				ADMIN_COOKIE = local_cookie;
				ADMIN_COOKIE_LENGTH = local_cookie_length;
				ForkThread( &CAimProto::aim_admin_negotiation, 0 );
			}
			else
				debugLogA("Failed to connect to the Admin Server.");
		}
		mir_free(server);
		mir_free(host);
	}
}

void CAimProto::snac_mail_response(SNAC &snac)//family 0x0018
{
	if (snac.subcmp(0x0007))
	{
		char* sn = NULL;
		time_t time = 0;
		unsigned short num_msgs = 0;
		unsigned short flags = 0;
		char new_mail = 0;
		char* url = NULL;
		char* address = NULL;

		int position = 26;
		int num_tlvs = snac.ushort(24);
		for (int i = 0; i < num_tlvs; i++)
		{
			TLV tlv(snac.val(position));
			if (tlv.cmp(0x0009))
			{
				sn = tlv.dup();
			}
			else if (tlv.cmp(0x001d))
			{
				time = tlv.ulong();
			}
			else if (tlv.cmp(0x0080))
			{
				num_msgs = tlv.ushort();
			}
			else if (tlv.cmp(0x0081))
			{
				new_mail = tlv.ubyte();
			}
			else if (tlv.cmp(0x0084))
			{
				flags = tlv.ushort();
			}
			else if (tlv.cmp(0x0007))
			{
				url = tlv.dup();
			}
			else if (tlv.cmp(0x0082))
			{
				address = tlv.dup();
			}
			position += TLV_HEADER_SIZE + tlv.len();
		}
		if (new_mail && num_msgs)
		{
			TCHAR msg[1024];

			int len = mir_sntprintf(msg, SIZEOF(msg), _T("%S@%S (%d)\r\n%s "), sn, address, num_msgs,
				TranslateT("You've got mail! Checked at")) ;

			SYSTEMTIME stLocal;
			GetLocalTime(&stLocal);
			GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stLocal, NULL, msg + len, SIZEOF(msg) - len);

			ShowPopup((char*)msg, MAIL_POPUP | TCHAR_POPUP, url);
		}
		mir_free(sn);
		mir_free(address);
		mir_free(url);
	}
}
void CAimProto::snac_retrieve_avatar(SNAC &snac)//family 0x0010
{
	if (snac.subcmp(0x0007))
	{
		int sn_len = snac.ubyte(0);
		char* sn = snac.part(1, sn_len);

		int parse_off = sn_len + 4;
		parse_off += snac.ubyte(parse_off);

		int hash_size=snac.ubyte(5+parse_off);
		char* hash_string=bytes_to_string(snac.val(6+parse_off), hash_size);
		parse_off += hash_size + 6; 

		int icon_length=snac.ushort(parse_off);
		char* icon_data=snac.val(parse_off+2);

		avatar_retrieval_handler(sn, hash_string, icon_data, icon_length);

		mir_free(hash_string);
		mir_free(sn);
	}
}
void CAimProto::snac_upload_reply_avatar(SNAC &snac)//family 0x0010
{
	if (snac.subcmp(0x0003))
	{
		int code = snac.ubyte(0);
		switch (code)
		{
		case 0: 
			break;
		case 3:
			ShowPopup(LPGEN("Error uploading avatar. (Too small)"), ERROR_POPUP);
			break;
		case 4:
			ShowPopup(LPGEN("Error uploading avatar. (Too big)"), ERROR_POPUP);
			break;
		case 5:
			ShowPopup(LPGEN("Error uploading avatar. (Wrong type)"), ERROR_POPUP);
			break;
		case 6:
			ShowPopup(LPGEN("Error uploading avatar. (Is banned)"), ERROR_POPUP);
			break;
		default:
			ShowPopup(LPGEN("Error uploading avatar. (Unknown error)"), ERROR_POPUP);
			break;
		}
	}
}
void CAimProto::snac_email_search_results(SNAC &snac)//family 0x000A
{
	if (snac.subcmp(0x0003)) // Found some buddies
	{
		PROTOSEARCHRESULT psr = {0};
		psr.cbSize = sizeof(psr);

		unsigned short offset=0;
		while(offset<snac.len())	// Loop through all the TLVs and pull out the buddy name
		{
			TLV tlv(snac.val(offset));
			offset+=TLV_HEADER_SIZE;
			psr.id = (TCHAR*)tlv.dup();
			offset+=tlv.len();
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);
			mir_free(psr.nick);
		}
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	}
	else // If no match, stop the search.
		CAimProto::ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

void CAimProto::snac_chatnav_info_response(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)//family 0x000D
{
	if (snac.subcmp(0x0009))
	{
		debugLogA("Chat Info Received");

		unsigned short offset_info=0;
		while (offset_info < snac.len())	// Loop through all the TLVs and pull out the buddy name
		{
			TLV info_tlv(snac.val(offset_info));
			if (info_tlv.cmp(0x0001)) // Redirect
			{
	//			char redirect = info_tlv.ubyte();	
			}
			else if (info_tlv.cmp(0x0002)) // Max Concurrent Rooms (usually 10)
			{
				// This typecasting pointer to number and as such bogus
				MAX_ROOMS = info_tlv.ubyte();

				aim_chatnav_ready(hServerConn,seqno);
				SetEvent(hChatNavEvent);
			}
			else if (info_tlv.cmp(0x0003)) // Exchanges
			{
			}
			else if (info_tlv.cmp(0x0004)) // Room Info
			{
				// Main TLV info
				unsigned short exchange = 0;
				unsigned short cookie_len = 0;
				char* cookie = 0;
				unsigned short instance = 0;
				unsigned short num_tlv = 0;
				unsigned short tlv_offset = 0;

				exchange = info_tlv.ushort(0);				// Exchange
				cookie_len = info_tlv.ubyte(2);				// Cookie Length
				cookie = info_tlv.part(3,cookie_len);		// Cookie String
				instance = info_tlv.ushort(3+cookie_len);	// Instance
				num_tlv = info_tlv.ushort(6+cookie_len);	// Number of TLVs
				tlv_offset = 8+cookie_len;					// We're looking at any remaining TLVs

				char* name = 0;
/*
				unsigned short max_occupancy = 0;
				char* fqn = 0;
				unsigned short flags = 0;
				unsigned long create_time = 0;
				unsigned short max_msg_len = 0;
				unsigned char create_perms = 0;
*/
				for (int i = 0; i < num_tlv; i++)	// Loop through all the TLVs
				{
					TLV tlv(info_tlv.val() + tlv_offset);
					
					// TLV List
					if (tlv.cmp(0x00d3))
						name = tlv.dup();
/*
					else if (tlv.cmp(0x00d2))
						max_occupancy = tlv.ushort();
					else if (tlv.cmp(0x006a))
						fqn = tlv.dup();
					else if (tlv.cmp(0x00c9))
						flags = tlv.ushort();
					else if (tlv.cmp(0x00ca))
						create_time = tlv.ulong();
					else if (tlv.cmp(0x00d1))
						max_msg_len = tlv.ushort();
					else if (tlv.cmp(0x00d5))
						create_perms = tlv.ubyte();
*/
					tlv_offset+=TLV_HEADER_SIZE+tlv.len();
				}

				chat_list_item *item = find_chat_by_id(name);
				if (item == NULL)
				{
					item = new chat_list_item(name, cookie, exchange, instance); 
					chat_rooms.insert(item); 

					//Join the actual room
					aim_chat_join_room(CAimProto::hServerConn, CAimProto::seqno, cookie, exchange, instance, item->cid);
				}

				mir_free(name);
				mir_free(cookie);
			}
			offset_info += TLV_HEADER_SIZE + info_tlv.len();
		}
	}
}
void CAimProto::snac_chat_joined_left_users(SNAC &snac,chat_list_item* item)//family 0x000E
{	// Handles both joining and leaving users.
	if (snac.subcmp(0x0003) || snac.subcmp(0x0004))
	{
		int offset = 0;
		while (offset < snac.len())
		{
			int sn_len = snac.ubyte(offset);
			char* sn = snac.part(offset+1, sn_len);				// Most important part (screenname)

			chat_event(item->id, sn, snac.subcmp(0x0003) ? GC_EVENT_JOIN : GC_EVENT_PART);

			mir_free(sn);

//          int warning = snac.ushort(offset+1+sn_len);
			int num_tlv = snac.ushort(offset+3+sn_len);
			offset += 5+sn_len;			                    // We're looking at any remaining TLVs
/*
			unsigned short user_class = 0;
			unsigned long idle_time = 0;
			unsigned long signon_time = 0;
			unsigned long creation_time = 0;				// Server uptime?
*/
			for (int i = 0; i < num_tlv; i++)		        // Loop through all the TLVs
			{
				TLV tlv(snac.val(offset));
/*
				if (tlv.cmp(0x0001))
					user_class = tlv.ushort();
				else if (tlv.cmp(0x0003))
					signon_time = tlv.ulong();
				else if (tlv.cmp(0x0005))
					creation_time = tlv.ulong();
				else if (tlv.cmp(0x000F))
					idle_time = tlv.ulong();
*/
				offset += TLV_HEADER_SIZE + tlv.len();
			}
		}
	}		
}
void CAimProto::snac_chat_received_message(SNAC &snac,chat_list_item* item)//family 0x000E
{
	if (snac.subcmp(0x0006))
	{
		TCHAR* message = NULL;
		char* sn = NULL;

//		unsigned long cookie = snac.ulong(0);
//		unsigned short channel = snac.ushort(8);

		int tlv_offset = 10;
		while (tlv_offset < snac.len())
		{
			TLV tlv(snac.val(tlv_offset));

			if (tlv.cmp(0x0003))    // Sender information
			{
				int sn_len = tlv.ubyte(0);
				sn = tlv.part(1, sn_len);
		/*		
				unsigned short warning = tlv.ushort(1+sn_len);
				int num_tlv = tlv.ushort(3+sn_len);
				
				int offset = 19 + sn_len;

				unsigned short user_class = 0;
				unsigned long  idle_time = 0;
				unsigned long  signon_time = 0;
				unsigned long  creation_time = 0;					//Server uptime?
		
				for (int i = 0; i < num_tlv; i++)			// Loop through all the TLVs
				{
					TLV info_tlv(tlv.val() + offset);
					
					// TLV List
					if (info_tlv.cmp(0x0001))
						user_class = info_tlv.ushort();
					else if (info_tlv.cmp(0x0003))
						signon_time = info_tlv.ulong();
					else if (info_tlv.cmp(0x0005))
						creation_time = info_tlv.ulong();
					else if (info_tlv.cmp(0x000F))
						idle_time = info_tlv.ulong();

					 offset += TLV_HEADER_SIZE + info_tlv.len();
				}
		*/
			}
			else if (tlv.cmp(0x0001))  // Public/Whisper flag
			{
			}
			else if (tlv.cmp(0x0005))  // Message information
			{
				bool uni = false;
				bool utf = false;
//		        char* language = NULL;

				int offset = 0;
				while (offset < tlv.len())
				{
					TLV msg_tlv(tlv.val() + offset);
					
					// TLV List
					if (msg_tlv.cmp(0x0001))
					{
						if (uni) 
						{
							char* msg = msg_tlv.dupw();
							html_decode(msg);
							message = mir_utf8decodeT(msg);                    
							mir_free(msg);
						}
						else if (utf)
						{
							char* msg = msg_tlv.dup();
							html_decode(msg);
							message = mir_utf8decodeT(msg);
							mir_free(msg);
						}
						else
						{
							char* msg = msg_tlv.dup();
							html_decode(msg);
							message = mir_a2t(msg);
							mir_free(msg);
						}
					}
					else if (msg_tlv.cmp(0x0002))
					{
						char* enc = msg_tlv.dup();
						uni = strstr(enc, "unicode-2-0") != NULL;
						utf = strstr(enc, "utf-8") != NULL;
						mir_free(enc);
					}
//			        else if (msg_tlv.cmp(0x0003))
//				        language = msg_tlv.dup();

					offset += TLV_HEADER_SIZE + msg_tlv.len();
				}
			}
			
			tlv_offset += TLV_HEADER_SIZE + tlv.len();
		}

		chat_event(item->id, sn, GC_EVENT_MESSAGE, message);

		mir_free(message);
		mir_free(sn);
	}
}

void CAimProto::snac_admin_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno)// family 0x0001
{
	if (snac.subcmp(0x0007))
	{
		aim_accept_rates(hServerConn,seqno);
		aim_admin_ready(hServerConn,seqno);
		SetEvent(hAdminEvent);
	}
}

void CAimProto::snac_admin_account_infomod(SNAC &snac)//family 0x0007
{
	if (snac.subcmp(0x0003) || snac.subcmp(0x0005)) // Handles info response and modification response
	{
		bool err = false;
		bool req_email = false;
		unsigned short perms = 0;
		unsigned short num_tlv = 0;
		
		perms = snac.ushort();				// Permissions
		num_tlv = snac.ushort(2);			// Number of TLVs

		char* sn = NULL;					// Screen Name
		char* email = NULL;					// Email address
		//unsigned short status = 0;		// Account status

		unsigned short offset = 0;
		for (int i = 0; i < num_tlv; i++)	// Loop through all the TLVs
		{
			TLV tlv(snac.val(4+offset));
			
			// TLV List
			if (tlv.cmp(0x0001))
				sn = tlv.dup();
			if (tlv.cmp(0x0011))
			{
				req_email = true;
				email = tlv.dup();
			}
			//if (tlv.cmp(0x0013))
			//	status = tlv.ushort();
			if (tlv.cmp(0x0008))			// Handles any problems when requesting/changing information
			{
				err = true;
				admin_error(tlv.ushort());
			}
			//if (tlv.cmp(0x0004))
				//error description

			offset += TLV_HEADER_SIZE + tlv.len();
		}

		if (snac.subcmp(0x0003) && !err)	// Requested info
		{
			// Display messages
			if (email)
				setString(AIM_KEY_EM,email); // Save our email for future reference.
			if (sn)
				setString(AIM_KEY_SN,sn); // Update the database to reflect the formatted name.
			ProtoBroadcastAck( NULL, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0 );
			
		}
		else if (snac.subcmp(0x0005) && !err) // Changed info
		{
			// Display messages
			if (email && req_email)	// We requested to change the email
				ShowPopup(LPGEN("A confirmation message has been sent to the new email address. Please follow its instructions."), 0);
			else if (sn)
			{
				setString(AIM_KEY_SN,sn); // Update the database to reflect the formatted name.
				//ShowPopup("Your Screen Name has been successfully formatted.", 0);
			}
		}
		mir_free(sn);
		mir_free(email);
	}
}

void CAimProto::snac_admin_account_confirm(SNAC &snac)//family 0x0007
{
	if (snac.subcmp(0x0007))
	{
		unsigned short status = 0;

		status = snac.ushort();

		switch (status)
		{
		case 0:
			ShowPopup(LPGEN("A confirmation message has been sent to your email address. Please follow its instructions."), 0);
			break;

		case 0x13:
			ShowPopup(LPGEN("Unable to confirm at this time. Please try again later."), 0);
			break;

		case 0x1e:
			ShowPopup(LPGEN("Your account has already been confirmed."), 0);
			break;

		case 0x23:
			ShowPopup(LPGEN("Can't start the confirmation procedure."), 0);
			break;
		}

		//TLV tlv(snac.val(2));
		//if (tlv.cmp(0x0004))
			//error description
	}
}


/*void CAimProto::snac_delete_contact(SNAC &snac, char* buf)//family 0x0013
{
	if (snac.subcmp(0x000a))
	{
		char sn[33];
		int sn_length=buf[SNAC_SIZE*2];
		MCONTACT hContact;
		memset(sn, 0, sizeof(sn));
		memcpy(sn,&buf[SNAC_SIZE*2+1],sn_length);
		hContact=find_contact(sn);
		if (hContact)
		{
			unsigned short* type=(unsigned short*)&buf[SNAC_SIZE*2+1+sn_length];
			*type=htons(*type);
			if (*type==0x0000)//typing finished
				CallService(MS_PROTO_CONTACTISTYPING,hContact,(WPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			else if (*type==0x0001)//typed
				CallService(MS_PROTO_CONTACTISTYPING,hContact,PROTOTYPE_CONTACTTYPING_INFINITE);
			else if (*type==0x0002)//typing
				CallService(MS_PROTO_CONTACTISTYPING,hContact,(LPARAM)60);
		}
	}
}*/
