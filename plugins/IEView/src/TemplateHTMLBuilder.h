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
class TemplateHTMLBuilder;

#ifndef TEMPLATEHTMLBUILDER_INCLUDED
#define TEMPLATEHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"
#include "Template.h"

class TemplateHTMLBuilder :public HTMLBuilder
{
protected:
	char *timestampToString(uint32_t dwFlags, time_t check, int mode);
	time_t 		startedTime;
	time_t 		getStartedTime();
	const char *groupTemplate;
	char *getAvatar(MCONTACT hContact, const char *szProto);
	void buildHeadTemplate(IEView *, IEVIEWEVENT *event, ProtocolSettings* protoSettings);
	void appendEventTemplate(IEView *, IEVIEWEVENT *event, ProtocolSettings* protoSettings);
	virtual TemplateMap *getTemplateMap(ProtocolSettings *);
	virtual int getFlags(ProtocolSettings *);
public:
	TemplateHTMLBuilder();
	virtual ~TemplateHTMLBuilder();
	//	void buildHead(IEView *, IEVIEWEVENT *event);
};

#endif
