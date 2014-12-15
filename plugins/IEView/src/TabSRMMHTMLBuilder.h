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
class TabSRMMHTMLBuilder;

#ifndef TABSRMMHTMLBUILDER_INCLUDED
#define TABSRMMHTMLBUILDER_INCLUDED

#include "TemplateHTMLBuilder.h"

class TabSRMMHTMLBuilder :public TemplateHTMLBuilder
{
protected:
	virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	char *timestampToString(DWORD dwFlags, time_t check, int isGroupBreak);
	time_t 		startedTime;
	time_t 		getStartedTime();
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	bool isDbEventShown(DBEVENTINFO * dbei);
	void appendEventNonTemplate(IEView *, IEVIEWEVENT *event);
public:
	TabSRMMHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);

};

#endif
