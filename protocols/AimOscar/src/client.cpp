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

int CAimProto::aim_send_connection_packet(HANDLE hServerConn,unsigned short &seqno,char *buf)
{
	return aim_sendflap(hServerConn,0x01,4,buf,seqno);
}

int CAimProto::aim_authkey_request(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE*3+mir_strlen(username));
	aim_writesnac(0x17,0x06,offset,buf);
	aim_writetlv(0x01,(unsigned short)mir_strlen(username),username,offset,buf);
	aim_writetlv(0x4B,0,0,offset,buf);
	aim_writetlv(0x5A,0,0,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_auth_request(HANDLE hServerConn,unsigned short &seqno,const char* key,const char* language,
								const char* country, const char* username, const char* password)
{
	unsigned short offset=0;
	BYTE pass_hash[16];
	BYTE auth_hash[16];
	mir_md5_state_t state;
	
	mir_md5_init(&state);
	mir_md5_append(&state,(const BYTE *)password, (int)mir_strlen(password));
	mir_md5_finish(&state,pass_hash);
	mir_md5_init(&state);
	mir_md5_append(&state,(BYTE*)key, (int)mir_strlen(key));
	mir_md5_append(&state,(BYTE*)pass_hash,MD5_HASH_LENGTH);
	mir_md5_append(&state,(BYTE*)AIM_MD5_STRING, sizeof(AIM_MD5_STRING)-1);
	mir_md5_finish(&state,auth_hash);

	char client_id[64], mirver[64];
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(mirver), (LPARAM)mirver);
	int client_id_len = mir_snprintf(client_id, SIZEOF(client_id), "Miranda AIM, version %s", mirver);

	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE*14+MD5_HASH_LENGTH+mir_strlen(username)+client_id_len+30+mir_strlen(language)+mir_strlen(country));

	aim_writesnac(0x17,0x02,offset,buf);
	aim_writetlv(0x01,(unsigned short)mir_strlen(username),username,offset,buf);
	aim_writetlv(0x25,MD5_HASH_LENGTH,(char*)auth_hash,offset,buf);
	aim_writetlv(0x4C,0,0,offset,buf);//signifies new password hash instead of old method
	aim_writetlv(0x03,(unsigned short)client_id_len,client_id,offset,buf);
	aim_writetlvshort(0x17,AIM_CLIENT_MAJOR_VERSION,offset,buf);
	aim_writetlvshort(0x18,AIM_CLIENT_MINOR_VERSION,offset,buf);
	aim_writetlvshort(0x19,AIM_CLIENT_LESSER_VERSION,offset,buf);
	aim_writetlvshort(0x1A,AIM_CLIENT_BUILD_NUMBER,offset,buf);
	aim_writetlvshort(0x16,AIM_CLIENT_ID_NUMBER,offset,buf);
	aim_writetlvlong(0x14,AIM_CLIENT_DISTRIBUTION_NUMBER,offset,buf);
	aim_writetlv(0x0F,(unsigned short)mir_strlen(language),language,offset,buf);
	aim_writetlv(0x0E,(unsigned short)mir_strlen(country),country,offset,buf);
	aim_writetlvchar(0x4A,getByte(AIM_KEY_FSC, 0) ? 3 : 1,offset,buf);
//    aim_writetlvchar(0x94,0,offset,buf);
	if (!getByte(AIM_KEY_DSSL, 0))
		aim_writetlv(0x8c,0,NULL,offset,buf);                       // Request SSL connection
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_send_cookie(HANDLE hServerConn,unsigned short &seqno,int cookie_size,char * cookie)
{
	unsigned short offset=0;
	char* buf=(char*)alloca(TLV_HEADER_SIZE*2+cookie_size);
	aim_writelong(0x01,offset,buf);//protocol version number
	aim_writetlv(0x06,(unsigned short)cookie_size,cookie,offset,buf);
	return aim_sendflap(hServerConn,0x01,offset,buf,seqno);
}

int CAimProto::aim_send_service_request(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*12];
	aim_writesnac(0x01,0x17,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writefamily(AIM_SERVICE_SSI,offset,buf);
	aim_writefamily(AIM_SERVICE_LOCATION,offset,buf);
	aim_writefamily(AIM_SERVICE_BUDDYLIST,offset,buf);
	aim_writefamily(AIM_SERVICE_MESSAGING,offset,buf);
	aim_writefamily(AIM_SERVICE_ICQ,offset,buf);
	aim_writefamily(AIM_SERVICE_INVITATION,offset,buf);
	aim_writefamily(AIM_SERVICE_POPUP,offset,buf);
	aim_writefamily(AIM_SERVICE_BOS,offset,buf);
	aim_writefamily(AIM_SERVICE_USERLOOKUP,offset,buf);
	aim_writefamily(AIM_SERVICE_STATS,offset,buf);
	aim_writefamily(AIM_SERVICE_UNKNOWN,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_new_service_request(HANDLE hServerConn,unsigned short &seqno,unsigned short service)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+2+TLV_HEADER_SIZE];
	aim_writesnac(0x01,0x04,offset,buf);
	aim_writeshort(service,offset,buf);
	if (!getByte(AIM_KEY_DSSL, 0))
		aim_writetlv(0x8c,0,NULL,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_request_rates(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x01,0x06,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_accept_rates(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE*2];
	aim_writesnac(0x01,0x08,offset,buf);
	aim_writegeneric(10,AIM_SERVICE_RATES,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_request_icbm(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x04,0x04,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_set_icbm(HANDLE hServerConn,unsigned short &seqno)
{
	const unsigned icbm_flags = ICBM_CHANNEL_MSGS_ALLOWED | ICBM_MISSED_CALLS_ENABLED | 
		ICBM_EVENTS_ALLOWED | ICBM_SMS_SUPPORTED | ICBM_OFFLINE_MSGS_ALLOWED;
//	const unsigned icbm_flags = 0x3db;

	unsigned short offset=0;
	char buf[SNAC_SIZE+16];
	aim_writesnac(0x04,0x02,offset,buf);
	aim_writeshort(0,offset,buf);           //channel
	aim_writelong(icbm_flags,offset,buf);   //flags
	aim_writeshort(0x1f40,offset,buf);      //max snac size              8000
	aim_writeshort(0x03e7,offset,buf);      //max sender warning level   999 (0-1000) WinAim default
	aim_writeshort(0x03e7,offset,buf);      //max receiver warning level 999 (0-1000) WinAim default
	aim_writelong(0,offset,buf);            //min message interval, ms   0
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_request_offline_msgs(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x04,0x10,offset,buf); // Subtype for offline messages 0x10
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_request_list(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x13,0x04,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_activate_list(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x13,0x07,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

/*
0000   00 05 00 02 00 17 00 06 00 03 00 00 00 00 07 00  ................
0010   01 00 00 08 00 01 01 00 0a 00 14 00 02 00 08 66  ...............f
0020   61 63 65 62 6f 6f 6b 00 06 67 6f 6f 67 6c 65     acebook..google

int CAimProto::aim_request_rights(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+50];
	aim_writesnac(0x03,0x02,offset,buf);
	aim_writetlvshort(0x05,0x17,offset,buf);
	aim_writetlv(0x06,3,"\x0\x0",offset,buf);
	aim_writetlvchar(0x07,0x01,offset,buf);
	aim_writetlvshort(0x05,0x17,offset,buf);
	aim_writetlvshort(0x05,0x17,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}
*/
int CAimProto::aim_set_caps(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	int i=1;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*3+AIM_CAPS_LENGTH*50+sizeof(AIM_MSG_TYPE)];
	char temp[AIM_CAPS_LENGTH*20];
	memcpy(temp,AIM_CAP_SHORT_CAPS,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_HOST_STATUS_TEXT_AWARE,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_SMART_CAPS,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_FILE_TRANSFER,AIM_CAPS_LENGTH);
//	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_HAS_MICROPHONE,AIM_CAPS_LENGTH);
//	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_RTCAUDIO,AIM_CAPS_LENGTH);
//	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_HAS_CAMERA,AIM_CAPS_LENGTH);
//	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_RTCVIDEO,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_BUDDY_ICON,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_CHAT,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_SUPPORT_ICQ,AIM_CAPS_LENGTH);
//	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_ICQ_SERVER_RELAY,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_UTF8,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_MIRANDA,AIM_CAPS_LENGTH);
	if (getByte(AIM_KEY_HF, 0))
		memcpy(&temp[AIM_CAPS_LENGTH*i++],AIM_CAP_HIPTOP,AIM_CAPS_LENGTH);
	aim_writesnac(0x02,0x04,offset,buf);
	aim_writetlv(0x05,(unsigned short)(AIM_CAPS_LENGTH*i),temp,offset,buf);

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_set_profile(HANDLE hServerConn,unsigned short &seqno, char* amsg)//user info
{
	aimString str(amsg);
	const char *charset = str.isUnicode() ? AIM_MSG_TYPE_UNICODE : AIM_MSG_TYPE;
	const unsigned short charset_len = (unsigned short)mir_strlen(charset);

	const char* msg = str.getBuf();
	const unsigned short msg_len = str.getSize();

	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE*3+1+charset_len+msg_len);
	unsigned short offset=0;

	aim_writesnac(0x02,0x04,offset,buf);
	aim_writetlvchar(0x0c,1,offset,buf);
	aim_writetlv(0x01,charset_len,charset,offset,buf);
	aim_writetlv(0x02,msg_len,msg,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_client_ready(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset = 0;

	NETLIBCONNINFO connInfo = { sizeof(connInfo) }; 
	CallService(MS_NETLIB_GETCONNECTIONINFO, (WPARAM)hServerConn, (LPARAM)&connInfo);
	
	internal_ip = connInfo.dwIpv4;

	char buf[SNAC_SIZE+TLV_HEADER_SIZE*22];
	aim_writesnac(0x01,0x02,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_SSI,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_LOCATION,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_BUDDYLIST,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_MESSAGING,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_ICQ,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_INVITATION,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	//removed extra generic server 
	aim_writefamily(AIM_SERVICE_POPUP,offset,buf);
	aim_writegeneric(4,"\x01\x04\0\x01",offset,buf);//different version number like trillian 3.1
	aim_writefamily(AIM_SERVICE_BOS,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_USERLOOKUP,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_STATS,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_mail_ready(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*4];
	aim_writesnac(0x01,0x02,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_MAIL,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_avatar_ready(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*4];
	aim_writesnac(0x01,0x02,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_AVATAR,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chatnav_ready(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*4];
	aim_writesnac(0x01,0x02,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_CHATNAV,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chat_ready(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*4];
	aim_writesnac(0x01,0x02,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_CHAT,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_send_message(HANDLE hServerConn,unsigned short &seqno,const char* sn,char* amsg,bool auto_response, bool blast)
{	
	aimString str(amsg);

	const char* msg = str.getBuf();
	const unsigned short msg_len = str.getSize();

	unsigned short tlv_offset=0;
	char* tlv_buf=(char*)alloca(5+msg_len+8);

	char icbm_cookie[8];
	CallService(MS_UTILS_GETRANDOM, 8, (LPARAM)icbm_cookie);
 
	aim_writegeneric(5,"\x05\x01\x00\x01\x01",tlv_offset,tlv_buf);   // icbm im capabilities
	aim_writeshort(0x0101,tlv_offset,tlv_buf);                       // icbm im text tag
	aim_writeshort(msg_len+4,tlv_offset,tlv_buf);                    // icbm im text tag length
	aim_writeshort(str.isUnicode()?2:0,tlv_offset,tlv_buf);          // character set
	aim_writeshort(0,tlv_offset,tlv_buf);                            // language

	aim_writegeneric(msg_len,msg,tlv_offset,tlv_buf);                // message text
	
	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	char* buf= (char*)alloca(SNAC_SIZE+8+3+sn_length+TLV_HEADER_SIZE*3+tlv_offset);
	
	aim_writesnac(0x04,0x06,offset,buf,get_random());
	aim_writegeneric(8,icbm_cookie,offset,buf);                      // icbm cookie
	aim_writeshort(0x01,offset,buf);                                 // channel
	aim_writechar((unsigned char)sn_length,offset,buf);              // screen name len
	aim_writegeneric(sn_length,sn,offset,buf);                       // screen name

	aim_writetlv(0x02,tlv_offset,tlv_buf,offset,buf);

	if (!blast)
	{
		if (auto_response)
			aim_writetlv(0x04,0,0,offset,buf);                       // auto-response message
		else
		{
			aim_writetlv(0x03,0,0,offset,buf);                       // message ack request
			aim_writetlv(0x06,0,0,offset,buf);                       // offline message storage
		}
	}
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno) ? 0 : *(int*)icbm_cookie & 0x7fffffff;
}

int CAimProto::aim_query_profile(HANDLE hServerConn,unsigned short &seqno,char* sn)
{
	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	char* buf=(char*)alloca(SNAC_SIZE+5+sn_length);
	aim_writesnac(0x02,0x15,offset,buf);
	aim_writelong(0x01,offset,buf);
	aim_writechar((unsigned char)sn_length,offset,buf);
	aim_writegeneric(sn_length,sn,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno)==0;
}

int CAimProto::aim_delete_contact(HANDLE hServerConn, unsigned short &seqno, char* sn, unsigned short item_id,
								  unsigned short group_id, unsigned short list, bool nil)
{
	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	char* buf=(char*)alloca(SNAC_SIZE+sn_length+10);
	aim_writesnac(0x13,0x0a,offset,buf, get_random());              // SSI Delete
	aim_writeshort(sn_length,offset,buf);                           // screen name length
	aim_writegeneric(sn_length,sn,offset,buf);                      // screen name
	aim_writeshort(group_id,offset,buf);                            // group id
	aim_writeshort(item_id,offset,buf);                             // buddy id
	aim_writeshort(list,offset,buf);                                // buddy type
	aim_writeshort(nil?4:0,offset,buf);                             // length of extra data
	if (nil) aim_writetlv(0x6a,0,NULL,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_add_contact(HANDLE hServerConn, unsigned short &seqno, const char* sn, unsigned short item_id,
							   unsigned short group_id, unsigned short list, char* nick, char* note)
{
	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	unsigned short nick_length = (unsigned short)mir_strlen(nick);
	unsigned short note_length = (unsigned short)mir_strlen(note);
	unsigned short tlv_len = nick || note ? TLV_HEADER_SIZE * 2 + nick_length + note_length : 0;

	char* buf=(char*)alloca(SNAC_SIZE + sn_length + 10 + tlv_len);
	aim_writesnac(0x13,0x08,offset,buf, get_random());               // SSI Add
	aim_writeshort(sn_length,offset,buf);                            // screen name length
	aim_writegeneric(sn_length,sn,offset,buf);                       // screen name
	aim_writeshort(group_id,offset,buf);                             // group id
	aim_writeshort(item_id,offset,buf);                              // buddy id
	aim_writeshort(list,offset, buf);                                // buddy type
	aim_writeshort(tlv_len,offset,buf);                              // length of extra data

	if (nick || note)
	{
		aim_writetlv(0x13c,note_length,note,offset,buf);
		aim_writetlv(0x131,nick_length,nick,offset,buf);
	}
	return aim_sendflap(hServerConn, 0x02, offset, buf, seqno);
}

int CAimProto::aim_mod_group(HANDLE hServerConn, unsigned short &seqno, const char* name, unsigned short group_id,
							 char* members, unsigned short members_length)
{
	unsigned short offset=0;
	unsigned short name_length=(unsigned short)mir_strlen(name);
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE+name_length+members_length+10);
	aim_writesnac(0x13,0x09,offset,buf, get_random());              // SSI Edit
	aim_writeshort(name_length,offset,buf);                         // group name length
	aim_writegeneric(name_length,name,offset,buf);                  // group name 
	aim_writeshort(group_id,offset,buf);                            // group id
	aim_writeshort(0,offset,buf);                                   // buddy id
	aim_writeshort(1,offset,buf);                                   // buddy type: Group
	aim_writeshort(TLV_HEADER_SIZE+members_length,offset,buf);      // length of extra data 
	aim_writetlv(0xc8,members_length,members,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_mod_buddy(HANDLE hServerConn, unsigned short &seqno, const char* sn, 
							 unsigned short buddy_id, unsigned short group_id,
							 char* nick, char* note)
{
	unsigned short offset=0;
	unsigned short sn_length = (unsigned short)mir_strlen(sn);
	unsigned short nick_length = (unsigned short)mir_strlen(nick);
	unsigned short note_length = (unsigned short)mir_strlen(note);
	unsigned short tlv_len = TLV_HEADER_SIZE * 2 + nick_length + note_length;
	
	char* buf=(char*)alloca(SNAC_SIZE+sn_length+10+tlv_len);
	aim_writesnac(0x13,0x09,offset,buf, get_random());              // SSI Edit
	aim_writeshort(sn_length,offset,buf);                           // screen name length
	aim_writegeneric(sn_length,sn,offset,buf);                      // screen name 
	aim_writeshort(buddy_id,offset,buf);                            // buddy id
	aim_writeshort(group_id,offset,buf);                            // group id
	aim_writeshort(0,offset,buf);                                   // buddy type: Buddy
	aim_writeshort(tlv_len,offset,buf);                             // length of extra data 

	aim_writetlv(0x13c,note_length,note,offset,buf);
	aim_writetlv(0x131,nick_length,nick,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_set_pd_info(HANDLE hServerConn, unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*3+20];
	unsigned short req = 0x09;
	if (pd_info_id == 0)
	{
		pd_info_id = get_random();
		req = 0x08;
	}
	aim_writesnac(0x13,req,offset,buf, get_random());               // SSI Edit/Add
	aim_writeshort(0,offset,buf);                                   // name length
	aim_writeshort(0,offset,buf);                                   // group id (root)
	aim_writeshort(pd_info_id,offset,buf);                          // buddy id
	aim_writeshort(0x4,offset,buf);                                 // pd info id
	aim_writeshort(0x15,offset,buf);                                // size
	aim_writetlvchar(0xca,pd_mode,offset,buf);                      // pd mode
	aim_writetlvlong(0xcb,0xffffffff,offset,buf);                   // pd mask
	aim_writetlvlong(0xcc,pd_flags,offset,buf);                     // pd flags
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_ssi_update_preferences(HANDLE hServerConn, unsigned short &seqno)
{
	unsigned short offset = 0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*4+100];
	unsigned short req = 0x09;
	if (pref1_id == 0)
	{
		pref1_id = get_random();
		req = 0x08;
	}
	aim_writesnac(0x13,req,offset,buf, get_random());               // SSI Edit/Add
	aim_writeshort(0,offset,buf);									// group name length
	aim_writeshort(0,offset,buf);									// group id (root)
	aim_writeshort(pref1_id,offset,buf);                            // buddy id
	aim_writeshort(5,offset,buf);                                   // buddy type: Presence

	unsigned short tlv_len = TLV_HEADER_SIZE * 2 + 8;
	if (pref2_len) tlv_len += TLV_HEADER_SIZE + pref2_len;
	if (pref2_set_len) tlv_len += TLV_HEADER_SIZE + pref2_set_len;

	aim_writeshort(tlv_len,offset,buf);								// length of extra data
	aim_writetlvlong(0xc9,pref1_flags,offset,buf);					// Update Buddy preferences 1
	aim_writetlvlong(0xd6,pref1_set_flags,offset,buf);				// Update Buddy preferences 1
	if (pref2_len) 
		aim_writetlv(0xd7,pref2_len,pref2_flags,offset,buf);		// Update Buddy preferences 2
	if (pref2_set_len) 
		aim_writetlv(0xd8,pref2_set_len,pref2_set_flags,offset,buf);// Update Buddy preferences 2
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_ssi_update(HANDLE hServerConn, unsigned short &seqno, bool start)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x13,start?0x11:0x12,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno)==0;
}

int CAimProto::aim_keepalive(HANDLE hServerConn,unsigned short &seqno)
{
	return aim_sendflap(hServerConn,0x05,4,"\x0\x0\x0\xEE",seqno);
}

// used when requesting a regular file transfer
int CAimProto::aim_send_file(HANDLE hServerConn, unsigned short &seqno,
							 unsigned long ip, unsigned short port, 
							 bool force_proxy, file_transfer *ft)
{	
	char msg_frag[2048];
	unsigned short frag_offset=0;

	aim_writeshort(0,frag_offset,msg_frag);                             // request type
	aim_writegeneric(8,ft->icbm_cookie,frag_offset,msg_frag);           // icbm cookie
	aim_writegeneric(AIM_CAPS_LENGTH, AIM_CAP_FILE_TRANSFER,
		frag_offset, msg_frag);                                         // uuid
	aim_writetlvshort(0x0a,++ft->req_num,frag_offset,msg_frag);         // request number
//    aim_writetlvshort(0x12,2,frag_offset,msg_frag);                     // max protocol version

	aim_writetlvlong(0x02,ip,frag_offset,msg_frag);                     // ip
	aim_writetlvlong(0x16,~ip,frag_offset,msg_frag);                    // ip check

	aim_writetlvshort(0x05,port,frag_offset,msg_frag);                  // port
	aim_writetlvshort(0x17,~port,frag_offset,msg_frag);                 // port ip check
	
	if (force_proxy)
		aim_writetlv(0x10,0,0,frag_offset,msg_frag);                    // request proxy transfer
	else
		aim_writetlvlong(0x03,internal_ip,frag_offset,msg_frag);        // ip

	if (ft->req_num == 1)
	{
		if (ft->message)
		{
			aimString dscr(ft->message);

			const char* charset = dscr.isUnicode() ? "unicode-2-0" : "us-ascii";
			const unsigned short charset_len = (unsigned short)mir_strlen(charset);

			const char* desc_msg = dscr.getBuf();
			const unsigned short desc_len = dscr.getSize();

			aim_writetlv(0x0e,2,"en",frag_offset,msg_frag);             // language used by the data
			aim_writetlv(0x0d,charset_len,charset,frag_offset,msg_frag);// charset used by the data
			aim_writetlv(0x0c,desc_len,desc_msg,frag_offset,msg_frag);  // invitaion text
		}

		aim_writetlv(0x0f,0,0,frag_offset,msg_frag);                    // request host check

		const char* fname = get_fname(ft->file);
		const unsigned short fnlen = (unsigned short)mir_strlen(fname);

		char* fblock = (char*)alloca(9 + fnlen);
		*(unsigned short*)&fblock[0] = _htons(ft->pfts.totalFiles > 1 ? 2 : 1);     // single file transfer
		*(unsigned short*)&fblock[2] = _htons(ft->pfts.totalFiles);     // number of files
		*(unsigned long*) &fblock[4] = _htonl(ft->pfts.totalBytes);     // total bytes in files
		memcpy(&fblock[8], fname, fnlen + 1);

		const char* enc = is_utf(fname) ? "utf-8" : "us-ascii";
		aim_writetlv(0x2711,9+fnlen,fblock,frag_offset,msg_frag);               // extra data, file names, size
		aim_writetlv(0x2712,8,enc,frag_offset,msg_frag);                        // character set used by data
//        aim_writetlvlong64(0x2713,ft->pfts.totalBytes,frag_offset,msg_frag);    // file length

		debugLogA("Attempting to Send a file to a buddy.");
	}
	else
	{
		aim_writetlvshort(0x14,0x0a,frag_offset,msg_frag);              // Counter proposal reason
	}

	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(ft->sn);
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE*2+12+frag_offset+sn_length);
	aim_writesnac(0x04,0x06,offset,buf);                                // msg to host
	aim_writegeneric(8,ft->icbm_cookie,offset,buf);                     // icbm cookie
	aim_writeshort(2,offset,buf);                                       // icbm channel
	aim_writechar((unsigned char)sn_length,offset,buf);                 // screen name length
	aim_writegeneric(sn_length,ft->sn,offset,buf);                      // screen name
	aim_writetlv(0x05,frag_offset,msg_frag,offset,buf);                 // icbm tags
	aim_writetlv(0x03,0,0,offset,buf);                                  // request ack

	char cip[20];
	long_ip_to_char_ip(ip, cip);
	debugLogA("IP for Buddy to connect to: %s:%u", cip, port);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno)==0;
}


int CAimProto::aim_file_ad(HANDLE hServerConn,unsigned short &seqno,char* sn, char* icbm_cookie, bool deny, unsigned short)
{	
	unsigned short frag_offset=0;
	char msg_frag[10+AIM_CAPS_LENGTH+TLV_HEADER_SIZE*2+6];
	aim_writeshort(deny ? 1 : 2,frag_offset,msg_frag);              // icbm accept / deny
	aim_writegeneric(8,icbm_cookie,frag_offset,msg_frag);           // icbm cookie
	aim_writegeneric(AIM_CAPS_LENGTH,
		AIM_CAP_FILE_TRANSFER,frag_offset,msg_frag);                // uuid

//    if (max_ver > 1)
//        aim_writetlvshort(0x12,2,frag_offset,msg_frag);             // max protocol version

	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	unsigned short offset=0;
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE+21+frag_offset+sn_length);
	aim_writesnac(0x04,0x06,offset,buf);                            // msg to host
	aim_writegeneric(8,icbm_cookie,offset,buf);                     // icbm cookie
	aim_writeshort(2,offset,buf);                                   // icbm channel
	aim_writechar((unsigned char)sn_length,offset,buf);             // screen name length
	aim_writegeneric(sn_length,sn,offset,buf);                      // screen name
	aim_writetlv(0x05,frag_offset,msg_frag,offset,buf);             // icbm tags

	debugLogA("%s a file transfer.", deny ? "Denying" : "Accepting");
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno)==0;
}

int CAimProto::aim_typing_notification(HANDLE hServerConn,unsigned short &seqno,char* sn,unsigned short type)
{
	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	char* buf= (char*)alloca(SNAC_SIZE+sn_length+13);
	aim_writesnac(0x04,0x14,offset,buf);
	aim_writegeneric(8,"\0\0\0\0\0\0\0\0",offset,buf);              // icbm cookie
	aim_writeshort(1,offset,buf);                                   // icbm channel
	aim_writechar((unsigned char)sn_length,offset,buf);             // screen name length
	aim_writegeneric(sn_length,sn,offset,buf);                      // screen name
	aim_writeshort(type,offset,buf);                                // typing event
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_set_idle(HANDLE hServerConn,unsigned short &seqno,unsigned long seconds)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+4];
	aim_writesnac(0x01,0x11,offset,buf);
	aim_writelong(seconds,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_request_mail(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+34];
	aim_writesnac(0x18,0x06,offset,buf);
	aim_writegeneric(34,
		"\x00\x02"
		"\xb3\x80\x9a\xd8\x0d\xba\x11\xd5\x9f\x8a\x00\x60\xb0\xee\x06\x31"
		"\x5d\x5e\x17\x08\x55\xaa\x11\xd3\xb1\x43\x00\x60\xb0\xfb\x1e\xcb",
		offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_activate_mail(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+17];
	aim_writesnac(0x18,0x16,offset,buf);
	aim_writegeneric(17,"\x02\x04\x00\x00\x00\x04\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00",offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_request_avatar(HANDLE hServerConn,unsigned short &seqno, const char* sn, unsigned short bart_type, const char* hash, unsigned short hash_size)
{
	unsigned short offset=0;
	unsigned char sn_length=(unsigned char)mir_strlen(sn);
	char* buf= (char*)alloca(SNAC_SIZE+sn_length+hash_size+12);
	aim_writesnac(0x10,0x06,offset,buf);
	aim_writechar(sn_length,offset,buf);                            // screen name length
	aim_writegeneric(sn_length,sn,offset,buf);                      // screen name
	aim_writechar(1,offset,buf);                                    // number of BART ID
	aim_writebartid(bart_type,0,hash_size,hash,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_set_avatar_hash(HANDLE hServerConn, unsigned short &seqno, char flags, unsigned short bart_type, unsigned short &id, char size, const char* hash)
{
	unsigned short offset=0;
	
	char bart_type_txt[8];
	ultoa(bart_type, bart_type_txt, 10);
	unsigned short bart_type_len = (unsigned short)mir_strlen(bart_type_txt);

	unsigned short req = 0x09;
	if (id == 0)
	{
		id = get_random();
		req = 0x08;
	}

	char* buf = (char*)alloca(SNAC_SIZE + TLV_HEADER_SIZE * 2 + 20 + size + bart_type_len);
	aim_writesnac(0x13,req,offset,buf, get_random());               // SSI Edit/Add
	aim_writeshort(bart_type_len,offset,buf);                       // name length
	aim_writegeneric(bart_type_len,bart_type_txt,offset,buf);       // name 
	aim_writeshort(0,offset,buf);                                   // group id
	aim_writeshort(id,offset,buf);                                  // buddy id
	aim_writeshort(0x14,offset,buf);                                // buddy type: Buddy Icon
	aim_writeshort(2+size+TLV_HEADER_SIZE,offset,buf);              // length of extra data

	char* buf2 = (char*)alloca(2+size);
	buf2[0] = flags;
	buf2[1] = (char)size;
	memcpy(&buf2[2], hash, size);
	aim_writetlv(0xd5, 2+size, buf2, offset, buf);                  // BART

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_delete_avatar_hash(HANDLE hServerConn, unsigned short &seqno, char /*flags*/, unsigned short bart_type, unsigned short & id)
{
	unsigned short offset=0;

	if (id == 0) return -1;
	id = 0;
	
	char bart_type_txt[8];
	ultoa(bart_type, bart_type_txt, 10);
	unsigned short bart_type_len = (unsigned short)mir_strlen(bart_type_txt);

	char* buf = (char*)alloca(SNAC_SIZE + 20 + bart_type_len);
	aim_writesnac(0x13,0x0a,offset,buf, get_random());              // SSI Delete
	aim_writeshort(bart_type_len,offset,buf);                       // name length
	aim_writegeneric(bart_type_len,bart_type_txt,offset,buf);       // name 
	aim_writeshort(0,offset,buf);                                   // group id
	aim_writeshort(id,offset,buf);                                  // buddy id
	aim_writeshort(0x14,offset,buf);                                // buddy type: Buddy Icon
	aim_writeshort(0,offset,buf);                                   // length of extra data

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_upload_avatar(HANDLE hServerConn, unsigned short &seqno, unsigned short bart_type, const char* avatar, unsigned short avatar_size)
{
	unsigned short offset=0;
	char* buf=(char*)alloca(SNAC_SIZE+22+avatar_size);
	aim_writesnac(0x10,0x02,offset,buf);
	aim_writeshort(bart_type,offset,buf);                                   // BART id
	aim_writeshort(avatar_size,offset,buf);
	aim_writegeneric(avatar_size,avatar,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_search_by_email(HANDLE hServerConn,unsigned short &seqno, const char* email)
{
	unsigned short offset=0;
	char em_length=(char)mir_strlen(email);
	char* buf= (char*)alloca(SNAC_SIZE+em_length);
	aim_writesnac(0x0a,0x02,offset,buf);	                        // Email search
	aim_writegeneric(em_length,email,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chatnav_request_limits(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE];
	aim_writesnac(0x0d,0x02,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno) ? -1 : 0;
}

int CAimProto::aim_chatnav_create(HANDLE hServerConn,unsigned short &seqno, char* room, unsigned short exchage)
{
	//* Join Pseudo Room (Get's the info we need for the real connection)
	unsigned short room_len = (unsigned short)mir_strlen(room);

	unsigned short offset=0;
	char* buf=(char*)alloca(SNAC_SIZE+10+room_len+26);
	aim_writesnac(0x0d,0x08,offset,buf);
	aim_writeshort(exchage,offset,buf);           				    // Exchange
	aim_writechar(6,offset,buf);		        				    // Command Length
	aim_writegeneric(6,"create",offset,buf);					    // Command
	aim_writeshort(0xffff,offset,buf);      					    // Last Instance
	aim_writechar(1,offset,buf);        						    // Detail
	aim_writeshort(3,offset,buf);          				            // Number of TLVs
	aim_writetlv(0xd3,room_len,room,offset,buf);                    // Room Name
	aim_writetlv(0xd6,8,"us-ascii",offset,buf);		                // Character Set
	aim_writetlv(0xd7,2,"en",offset,buf);			                // Language Encoding

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chatnav_room_info(HANDLE hServerConn,unsigned short &seqno, char* chat_cookie, unsigned short exchange, unsigned short instance) 
{ 
	unsigned short offset=0; 
	unsigned short chat_cookie_len = (unsigned short)mir_strlen(chat_cookie); 
	char* buf=(char*)alloca(SNAC_SIZE+7+chat_cookie_len); 
	aim_writesnac(0x0d,0x04,offset,buf); 
	aim_writeshort(exchange,offset,buf);                            // Exchange 
	aim_writechar((unsigned char)chat_cookie_len,offset,buf);       // Chat Cookie Length 
	aim_writegeneric(chat_cookie_len,chat_cookie,offset,buf);       // Chat Cookie 
	aim_writeshort(instance,offset,buf);                            // Last Instance 
	aim_writechar(1,offset,buf);                                    // Detail 
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno); 
}  

int CAimProto::aim_chat_join_room(HANDLE hServerConn,unsigned short &seqno, char* chat_cookie, 
								  unsigned short exchange, unsigned short instance, unsigned short id)
{
	unsigned short offset=0;
	unsigned short cookie_len = (unsigned short)mir_strlen(chat_cookie);
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE*2+cookie_len+8);
	aim_writesnac(0x01,0x04,offset,buf,id);
	aim_writeshort(0x0e,offset,buf);	        			        // Service request for Chat

	aim_writeshort(0x01,offset,buf);						        // Tag
	aim_writeshort(cookie_len+5,offset,buf);				        // Length
	aim_writeshort(exchange,offset,buf);					        // Value - Exchange
	aim_writechar((unsigned char)cookie_len,offset,buf);	        // Value - Cookie Length
	aim_writegeneric(cookie_len,chat_cookie,offset,buf);	        // Value - Cookie
	aim_writeshort(instance,offset,buf);					        // Value - Instance

	if (!getByte(AIM_KEY_DSSL, 0))
		aim_writetlv(0x8c,0,NULL,offset,buf);                       // Request SSL connection

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chat_send_message(HANDLE hServerConn, unsigned short &seqno, char *amsg)
{
	aimString str(amsg);

	const char* charset = str.isUnicode() ? "unicode-2-0" : "us-ascii";
	const unsigned short chrset_len = (unsigned short)mir_strlen(charset);

	const char* msg = str.getBuf();
	const unsigned short msg_len = str.getSize();

	unsigned short tlv_offset=0;
	char* tlv_buf=(char*)alloca(TLV_HEADER_SIZE*4+chrset_len+msg_len+20);
	aim_writetlv(0x04,13,"text/x-aolrtf",tlv_offset,tlv_buf);   // Format
	aim_writetlv(0x02,chrset_len,charset,tlv_offset,tlv_buf);   // Character Set
	aim_writetlv(0x03,2,"en",tlv_offset,tlv_buf);			    // Language Encoding
	aim_writetlv(0x01,msg_len,msg,tlv_offset,tlv_buf);			// Message

	unsigned short offset=0;
	char* buf=(char*)alloca(SNAC_SIZE+8+2+TLV_HEADER_SIZE*3+tlv_offset);
	aim_writesnac(0x0e,0x05,offset,buf);
	aim_writegeneric(8,"\0\0\0\0\0\0\0\0",offset,buf);			// Message Cookie (can be random)
	aim_writeshort(0x03,offset,buf);							// Message Channel (Always 3 for chat)
	aim_writetlv(0x01,0,NULL,offset,buf);						// Public/Whisper flag
	aim_writetlv(0x06,0,NULL,offset,buf);						// Enable Reflection flag
	aim_writetlv(0x05,tlv_offset,tlv_buf,offset,buf);			// Message Information TLV

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chat_invite(HANDLE hServerConn,unsigned short &seqno, char* chat_cookie, unsigned short exchange, unsigned short instance, char* sn, char* msg)
{
	unsigned short offset=0;
	unsigned short chat_cookie_len = (unsigned short)mir_strlen(chat_cookie);
	unsigned short sn_len = (unsigned short)mir_strlen(sn);
	unsigned short msg_len = (unsigned short)mir_strlen(msg);
	char* buf=(char*)alloca(SNAC_SIZE+64+chat_cookie_len+sn_len+msg_len);
	aim_writesnac(0x04,0x06,offset,buf);
	aim_writegeneric(8,"\0\0\0\0\0\0\0\0",offset,buf);		    // ICBM Cookie
	aim_writeshort(2,offset,buf);				                // ICBM Channel
	aim_writechar((unsigned char)sn_len,offset,buf);		    // Screen Name Length
	aim_writegeneric(sn_len,sn,offset,buf);					    // Screen Name

	aim_writeshort(0x05,offset,buf);						    // Rendezvous Message Data TLV
	aim_writeshort(49+msg_len+chat_cookie_len,offset,buf);	    // TLV size
	
	aim_writeshort(0,offset,buf);							    // Message Type (0) - Request
	aim_writegeneric(8,"\0\0\0\0\0\0\0\0",offset,buf);		    // ICBM Cookie (same as above)
	aim_writegeneric(16,AIM_CAP_CHAT,offset,buf);			    // Capability

	aim_writetlvshort(0x0a,1,offset,buf);				        // Sequence Number TLV
	aim_writetlv(0x0f,0,NULL,offset,buf);					    // Request Host Caps Check TLV
	aim_writetlv(0x0c,msg_len,msg,offset,buf);				    // Invitation Message TLV

	aim_writeshort(0x2711,offset,buf);							// Capability TLV
	aim_writeshort(chat_cookie_len+5,offset,buf);				// Length
	aim_writeshort(exchange,offset,buf);						// Value - Exchange
	aim_writechar((unsigned char)chat_cookie_len,offset,buf);	// Value - Cookie Length
	aim_writegeneric(chat_cookie_len,chat_cookie,offset,buf);	// Value - Cookie
	aim_writeshort(instance,offset,buf);						// Value - Instance

	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_chat_deny(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie)
{	
	unsigned short offset=0;
	unsigned short sn_length=(unsigned short)mir_strlen(sn);
	char* buf=(char*)alloca(SNAC_SIZE+20+sn_length);
	aim_writesnac(0x04,0x0b,offset,buf);
	aim_writegeneric(8,icbm_cookie,offset,buf);             // ICBM Cookie
	aim_writeshort(2,offset,buf);                           // Channel
	aim_writechar((unsigned char)sn_length,offset,buf);     // Screen Name length
	aim_writegeneric(sn_length,sn,offset,buf);              // Screen Name
	aim_writeshort(3,offset,buf);                           // Error code
	aim_writeshort(2,offset,buf);                           // Error code
	aim_writeshort(1,offset,buf);                           // Error code
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno)==0;
}

int CAimProto::aim_admin_ready(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE*4];
	aim_writesnac(0x01,0x02,offset,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	aim_writefamily(AIM_SERVICE_ADMIN,offset,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_admin_format_name(HANDLE hServerConn,unsigned short &seqno, const char* sn)
{
	unsigned short offset=0;
	unsigned short sn_len = (unsigned short)mir_strlen(sn);
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE+sn_len);
	aim_writesnac(0x07,0x04,offset,buf);
	aim_writetlv(0x01,sn_len,sn,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_admin_change_email(HANDLE hServerConn,unsigned short &seqno, const char* email)
{
	unsigned short offset=0;
	unsigned short email_len = (unsigned short)mir_strlen(email);
	char* buf=(char*)alloca(SNAC_SIZE+TLV_HEADER_SIZE+email_len);
	aim_writesnac(0x07,0x04,offset,buf);
	aim_writetlv(0x11,email_len,email,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_admin_change_password(HANDLE hServerConn,unsigned short &seqno, const char* cur_pw, const char* new_pw)
{
	unsigned short offset=0;
	unsigned short cur_pw_len = (unsigned short)mir_strlen(cur_pw);
	unsigned short new_pw_len = (unsigned short)mir_strlen(new_pw);
	char* buf=(char*)alloca(SNAC_SIZE+2*TLV_HEADER_SIZE+cur_pw_len+new_pw_len);
	aim_writesnac(0x07,0x04,offset,buf);
	aim_writetlv(0x02,new_pw_len,new_pw,offset,buf);
	aim_writetlv(0x12,cur_pw_len,cur_pw,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_admin_request_info(HANDLE hServerConn,unsigned short &seqno, const unsigned short &type)
{
	// types: 0x01 - nickname, 0x11 - email info, 0x13 - registration status
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE];
	aim_writesnac(0x07,0x02,offset,buf);
	aim_writetlv(type,0,NULL,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}

int CAimProto::aim_admin_account_confirm(HANDLE hServerConn,unsigned short &seqno)
{
	unsigned short offset=0;
	char buf[SNAC_SIZE+TLV_HEADER_SIZE];
	aim_writesnac(0x07,0x06,offset,buf);
	return aim_sendflap(hServerConn,0x02,offset,buf,seqno);
}
