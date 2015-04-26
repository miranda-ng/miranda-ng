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

char** CAimProto::get_status_msg_loc(int status)
{
	static const int modes[] =
	{
		ID_STATUS_ONLINE,
		ID_STATUS_AWAY,
		ID_STATUS_DND, 
		ID_STATUS_NA,
		ID_STATUS_OCCUPIED, 
		ID_STATUS_FREECHAT,
		ID_STATUS_INVISIBLE,
		ID_STATUS_ONTHEPHONE,
		ID_STATUS_OUTTOLUNCH, 
	};

	for (int i=0; i<9; i++) 
		if (modes[i] == status) return &modeMsgs[i];

	return NULL;
}

int CAimProto::aim_set_away(HANDLE hServerConn, unsigned short &seqno, const char *amsg, bool set)//user info
{
	unsigned short offset = 0;
	char* html_msg = NULL;
	size_t msg_size = 0;
	if (set)
	{
		if (!amsg) return -1;
		setDword(AIM_KEY_LA, (DWORD)time(NULL));
		html_msg = html_encode(amsg && amsg[0] ? amsg : DEFAULT_AWAY_MSG);
		msg_size = strlen(html_msg);
	}

	aimString str(html_msg);
	const char *charset = str.isUnicode() ? AIM_MSG_TYPE_UNICODE : AIM_MSG_TYPE;
	const unsigned short charset_len = (unsigned short)strlen(charset);

	const char* msg = str.getBuf();
	const unsigned short msg_len = str.getSize();

	char* buf = (char*)alloca(SNAC_SIZE + TLV_HEADER_SIZE * 3 + charset_len + msg_len + 1);

	aim_writesnac(0x02, 0x04, offset, buf);
	aim_writetlv(0x03, charset_len, charset, offset, buf);
	aim_writetlv(0x04, (unsigned short)msg_len, msg, offset, buf);

	mir_free(html_msg);

	return aim_sendflap(hServerConn, 0x02, offset, buf, seqno);
}

int CAimProto::aim_set_status(HANDLE hServerConn, unsigned short &seqno, unsigned long status_type)
{
	unsigned short offset = 0;
	char buf[SNAC_SIZE + TLV_HEADER_SIZE * 2];
	aim_writesnac(0x01, 0x1E, offset, buf);
	aim_writetlvlong(0x06, status_type, offset, buf);
	return aim_sendflap(hServerConn, 0x02, offset, buf, seqno);
}

int CAimProto::aim_set_statusmsg(HANDLE hServerConn, unsigned short &seqno, const char *msg)//user info
{
	size_t msg_size = mir_strlen(msg);

	unsigned short msgoffset = 0;
	char* msgbuf = (char*)alloca(10 + msg_size);
	if (msg_size) {
		char* msgb = (char*)alloca(4 + msg_size);
		msgb[0] = (unsigned char)(msg_size >> 8);
		msgb[1] = (unsigned char)(msg_size & 0xff);
		memcpy(&msgb[2], msg, msg_size);
		msgb[msg_size + 2] = 0;
		msgb[msg_size + 3] = 0;

		aim_writebartid(2, 4, (unsigned short)(msg_size + 4), msgb, msgoffset, msgbuf);
	}
	else aim_writebartid(2, 0, 0, NULL, msgoffset, msgbuf);

	unsigned short offset = 0;
	char* buf = (char*)alloca(SNAC_SIZE + TLV_HEADER_SIZE + msgoffset + 8);
	aim_writesnac(0x01, 0x1e, offset, buf);
	aim_writetlv(0x1d, msgoffset, msgbuf, offset, buf);

	return aim_sendflap(hServerConn, 0x02, offset, buf, seqno);
}

int CAimProto::aim_query_away_message(HANDLE hServerConn, unsigned short &seqno, const char* sn)
{
	unsigned short offset = 0;
	unsigned short sn_length = (unsigned short)strlen(sn);
	char *buf = (char*)alloca(SNAC_SIZE + 5 + sn_length);
	aim_writesnac(0x02, 0x15, offset, buf);
	aim_writegeneric(4, "\0\0\0\x02", offset, buf);
	aim_writegeneric(1, (char*)&sn_length, offset, buf);
	aim_writegeneric(sn_length, sn, offset, buf);
	int res = aim_sendflap(hServerConn, 0x02, offset, buf, seqno) == 0;
	return res;
}

