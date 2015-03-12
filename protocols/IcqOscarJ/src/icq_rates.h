// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Rate management
// -----------------------------------------------------------------------------

#ifndef __ICQ_RATES_H
#define __ICQ_RATES_H

#define MAX_RATES_GROUP_COUNT   5

struct rates_group
{
	DWORD dwWindowSize;
	DWORD dwClearLevel;
	DWORD dwAlertLevel;
	DWORD dwLimitLevel;
	DWORD dwMaxLevel;
	
	// current level
	int rCurrentLevel;
	int tCurrentLevel;
	
	// links
	WORD *pPairs;
	int nPairs;
};

struct rates : public MZeroedObject
{
private:
	CIcqProto *ppro;
	int nGroups;
	rates_group groups[MAX_RATES_GROUP_COUNT];

	rates_group *getGroup(WORD wGroup);
public:
	rates(CIcqProto *ppro, BYTE *pBuffer, size_t wLen);
	~rates();

	WORD getGroupFromSNAC(WORD wFamily, WORD wCommand);
	WORD getGroupFromPacket(icq_packet *pPacket);

	int getLimitLevel(WORD wGroup, int nLevel);
	int getDelayToLimitLevel(WORD wGroup, int nLevel);
	int getNextRateLevel(WORD wGroup);

	void packetSent(icq_packet *pPacket);
	void updateLevel(WORD wGroup, int nLevel);

	void initAckPacket(icq_packet *pPacket);
};

#define RML_CLEAR       0x01
#define RML_ALERT       0x02
#define RML_LIMIT       0x03
#define RML_IDLE_10     0x10
#define RML_IDLE_30     0x11
#define RML_IDLE_50     0x12
#define RML_IDLE_70     0x13

// Rates - Level 2

// queue types
#define RQT_DEFAULT     0 // standard - pushes all items without much delay
#define RQT_REQUEST     1 // request - pushes only first item on duplicity
#define RQT_RESPONSE    2 // response - pushes only last item on duplicity

//
// generic queue item
//
struct rates_queue_item : public MZeroedObject
{
	friend class rates_queue;
protected:
	CIcqProto *ppro;
	BOOL bCreated;
	WORD wGroup;

	virtual BOOL isEqual(rates_queue_item *pItem);
	virtual rates_queue_item* copyItem(rates_queue_item *pDest = NULL);
public:
	rates_queue_item(CIcqProto *ppro, WORD wGroup);
	virtual ~rates_queue_item();

	BOOL isOverRate(int nLevel);

	virtual void execute();

	MCONTACT hContact;
	DWORD dwUin;
	char *szUid;
};

class rates_queue;
typedef void (rates_queue::*IcqRateFunc)(void);

//
// generic item queue (FIFO)
//
class rates_queue : public MZeroedObject
{
	CIcqProto  *ppro;
	const char *szDescr;
	int         duplicates;

	mir_cs listsMutex;  // we need to be thread safe
	LIST<rates_queue_item> lstPending;
	
protected:
	void cleanup();
	void processQueue();
	void initDelay(int nDelay, IcqRateFunc delaycode);

public:
	rates_queue(CIcqProto *ppro, const char *szDescr, int nLimitLevel, int nWaitLevel, int nDuplicates = 0);
	~rates_queue();

	void putItem(rates_queue_item *pItem, int nMinDelay);

	int limitLevel; // RML_*
	int waitLevel;
};


#endif /* __ICQ_RATES_H */
