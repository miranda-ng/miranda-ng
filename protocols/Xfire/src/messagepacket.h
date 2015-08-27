/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *    http://xfirelib.sphene.net
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef __MESSAGEPACKET_H
#define __MESSAGEPACKET_H

#include "xfirerecvpacketcontent.h"
#include "variablevalue.h"

#define XFIRE_MESSAGE_ID 133

namespace xfirelib {

	class MessagePacket : public XFireRecvPacketContent {
	public:

		MessagePacket();
		XFirePacketContent* newPacket() { return new MessagePacket(); }

		int getPacketContent(char *packet);
		int getPacketId() { return m_packetID; }
		int getPacketSize() { return 1024; };
		void parseContent(char *buf, int length, int numberOfAtts);
		std::string getMessage() {return m_message; }
		int getMessageType(){return m_msgtype->getValue()[0];}
		int getImIndex(){ return m_imindex->getValue()[0];}//TODO: fix this if we have more than 255 messages
		int getPacketAttributeCount(){ return 2; }
		char * getSid(){ return m_sid->getValue(); }

	private:
		VariableValue *m_sid;
		VariableValue *m_peermsg;
		VariableValue *m_msgtype;
		VariableValue *m_imindex;
		std::string m_message;
		char m_buf[150];
		int m_bufLength;
		int m_packetID;//Special case because we have to answer this packet with id 2
	};
};

#endif
