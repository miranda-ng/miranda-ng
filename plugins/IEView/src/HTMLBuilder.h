/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
class HTMLBuilder;

#ifndef HTMLBUILDER_INCLUDED
#define HTMLBUILDER_INCLUDED

#define EVENTTYPE_JABBER_CHATSTATES     2000

#include "IEView.h"
#include "Options.h"
#include "TextToken.h"

class HTMLBuilder {
private:
	IEVIEWEVENT lastIEViewEvent;
	static int mimFlags;
	enum MIMFLAGS {
		MIM_CHECKED = 1,
		MIM_UNICODE = 2
	};
protected:
	DWORD lastEventTime;
	int iLastEventType;
	enum ENCODEFLAGS {
		ENF_NONE = 0,
		ENF_SMILEYS = 1,
		ENF_NAMESMILEYS = 2,
		ENF_BBCODES = 4,
		ENF_LINKS = 8,
		ENF_ALL = 255,
		ENF_CHAT_FORMATTING = 256
	};
	//	virtual char *encode(const char *text, const char *proto, bool replaceSmiley);
	virtual char *encodeUTF8(MCONTACT hContact, const char *proto, const wchar_t *text, int flags, bool isSent);
	virtual char *encodeUTF8(MCONTACT hContact, const char *proto, const char *text, int flags, bool isSent);
	virtual char *encodeUTF8(MCONTACT hContact, const char *proto, const char *text, int cp, int flags, bool isSent);
	virtual wchar_t *encode(MCONTACT hContact, const char *proto, const wchar_t *text, int flags, bool isSent);
	virtual bool encode(MCONTACT hContact, const char *proto, const wchar_t *text, wchar_t **output, int *outputSize, int level, int flags, bool isSent);
	virtual char* getProto(MCONTACT hContact);
	virtual char* getProto(const char *proto, MCONTACT hContact);
	virtual char* getRealProto(MCONTACT hContact);
	virtual char* getRealProto(MCONTACT hContact, const char *proto);
	virtual wchar_t *getContactName(MCONTACT hContact, const char* szProto);
	virtual char *getEncodedContactName(MCONTACT hContact, const char *szProto, const char *szSmileyProto);
	virtual void getUINs(MCONTACT hContact, char *&uinIn, char *&uinOut);
	virtual MCONTACT getRealContact(MCONTACT hContact);
	virtual DWORD getLastEventTime();
	virtual void setLastEventTime(DWORD);
	virtual int getLastEventType();
	virtual void setLastEventType(int);
	virtual bool isSameDate(time_t time1, time_t time2);
	virtual bool isDbEventShown(DBEVENTINFO * dbei) = 0;
	virtual ProtocolSettings *getSRMMProtocolSettings(const char *protocolName);
	virtual ProtocolSettings *getSRMMProtocolSettings(MCONTACT hContact);
	virtual ProtocolSettings *getHistoryProtocolSettings(const char *protocolName);
	virtual ProtocolSettings *getHistoryProtocolSettings(MCONTACT hContact);
	virtual ProtocolSettings *getChatProtocolSettings(const char *protocolName);
	virtual ProtocolSettings *getChatProtocolSettings(MCONTACT hContact);
	void	setLastIEViewEvent(IEVIEWEVENT *event);
	virtual void buildHead(IEView *, IEVIEWEVENT *event) = 0;
public:
	HTMLBuilder();
	virtual ~HTMLBuilder();
	void appendEventOld(IEView *, IEVIEWEVENT *event);
	void appendEventNew(IEView *, IEVIEWEVENT *event);
	void clear(IEView *, IEVIEWEVENT *event);
	virtual void appendEvent(IEView *, IEVIEWEVENT *event) = 0;
};

#endif
