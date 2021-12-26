#ifndef _CEVENTLOG_H_
#define _CEVENTLOG_H_

#include "LCDFramework/CLCDList.h"

class CEventLogEntry
{
public:
	MCONTACT hContact;
	tstring strValue;
	tstring strTimestamp;
	tm Time;
	EventType eType;
};

class CEventLog : public CLCDList<CEventLogEntry*>
{
public:
	// constructor
	CEventLog();
	// destructor
	~CEventLog();

	// initializes the list
	bool Initialize();
	// deinitializes the list
	bool Shutdown();
	// adds an entry to the list
	CListItem<CEventLogEntry*> *AddItem(CEventLogEntry *);

	void SetPosition(CListEntry<CEventLogEntry*> *pEntry);
	bool ScrollUp();
	bool ScrollDown();

	bool SetFont(LOGFONT &lf);

protected:
	// Called to delete the specified entry
	void DeleteEntry(CEventLogEntry *pEntry);
	// Called to draw the specified entry
	void DrawEntry(CLCDGfx *pGfx, CEventLogEntry *pEntry, bool bSelected);

	uint32_t m_dwLastScroll = 0;
};

#endif
