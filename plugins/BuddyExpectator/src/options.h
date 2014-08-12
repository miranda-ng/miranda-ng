/*
	Buddy Expectator+ plugin for Miranda-IM (www.miranda-im.org)
	(c)2005 Anar Ibragimoff (ai91@mail.ru)
	(c)2006 Scott Ellis (mail@scottellis.com.au)
	(c)2007,2008 Alexander Turyak (thief@miranda-im.org.ua)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	File name      : $URL: http://svn.miranda.im/mainrepo/buddyexpectator/trunk/options.h $
	Revision       : $Rev: 1003 $
	Last change on : $Date: 2008-01-12 17:15:47 +0200 (Сб, 12 янв 2008) $
	Last change by : $Author: Thief $
*/

#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef enum GoneContactAction { GCA_NOACTION, GCA_DELETE, GCA_UDETAILS, GCA_MESSAGE };

typedef struct {
	int iAbsencePeriod;		// notify period for returning contacts
	int iShowPopup;
	int iShowEvent;
	int iShowUDetails;
	int iShowMessageWindow;

	bool notifyFirstOnline;
	bool enableMissYou;
	bool hideInactive;

	int iAbsencePeriod2;	// notify period for contacts that have remained away
	int iShowPopup2;
	int iShowEvent2;
	GoneContactAction action2;

	int iSilencePeriod;	    // notify period for contacts that didn't send messages

	int iPopupColorBack;
	int iPopupColorFore;
	int iPopupDelay;
	int iUsePopupColors;
	int iUseWinColors;

} Options;

extern Options options;

void InitOptions();

#endif
