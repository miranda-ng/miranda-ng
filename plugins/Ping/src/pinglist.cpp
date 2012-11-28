#include "common.h"
#include "pinglist.h"

#include "options.h"

PINGLIST list_items;
CRITICAL_SECTION list_cs;
HANDLE reload_event_handle;

DWORD NextID = 1;

BOOL clist_handle_changing = FALSE;

BOOL changing_clist_handle() {
	return clist_handle_changing;
}

void set_changing_clist_handle(BOOL flag) {
	clist_handle_changing = flag;
}

const bool PINGADDRESS::operator==(const PINGADDRESS &b) const {
	return index == b.index;
}

const bool PINGADDRESS::operator<(const PINGADDRESS &b) const {
	return index < b.index;
}

// lParam is address of pointer to a std::list<PINGADDRESS> 
// copies data into this structure
INT_PTR GetPingList(WPARAM wParam,LPARAM lParam)
{
	PINGLIST *pa = (PINGLIST *)lParam;

	EnterCriticalSection(&list_cs);
	*pa = list_items;
	LeaveCriticalSection(&list_cs);
	
	return 0;
}

INT_PTR GetListSize(WPARAM wParam, LPARAM lParam) {
	INT_PTR ret = 0;
	EnterCriticalSection(&list_cs);
	ret = list_items.size();
	LeaveCriticalSection(&list_cs);
	return ret;
}

void write_ping_address(PINGADDRESS *i) {
	char buff[16];
	sprintf(buff, "PING_DEST_%d", i->index);

	if(i->item_id == 0) {
		i->item_id = NextID++;
		DBWriteContactSettingDword(0, PLUG, "NextID", NextID);
	}

	DBWriteContactSettingDword(0, buff, "Id", i->item_id);
	DBWriteContactSettingString(0, buff, "Address", i->pszName);
	DBWriteContactSettingString(0, buff, "Label", i->pszLabel);
	DBWriteContactSettingWord(0, buff, "Status", i->status);
	DBWriteContactSettingDword(0, buff, "Port", i->port);
	DBWriteContactSettingString(0, buff, "Proto", i->pszProto);
	if(strlen(i->pszCommand))
		DBWriteContactSettingString(0, buff, "Command", i->pszCommand);
	else
		DBDeleteContactSetting(0, buff, "Command");
	if(strlen(i->pszParams))
		DBWriteContactSettingString(0, buff, "CommandParams", i->pszParams);
	else
		DBDeleteContactSetting(0, buff, "CommandParams");
	DBWriteContactSettingWord(0, buff, "SetStatus", i->set_status);
	DBWriteContactSettingWord(0, buff, "GetStatus", i->get_status);
	DBWriteContactSettingWord(0, buff, "Index", i->index);
}

// call with list_cs locked
void write_ping_addresses() {
	int index = 0;
	for(PINGLIST::Iterator i = list_items.start(); i.has_val(); i.next(), index++) {
		i.val().index = index;
		write_ping_address(&i.val());
	}

	// mark further destinations in the DB as invalid
	char buff[16];
	bool found;

	do {
		found = false;
		sprintf(buff, "PING_DEST_%d", index++);
		if(DBGetContactSettingDword(0, buff, "Id", 0) != 0) {
			found = true;
			DBWriteContactSettingDword(0, buff, "Id", 0);
		}
	} while(found);
}

bool read_ping_address(PINGADDRESS &pa) {
	int index = pa.index;

	char buff[16];
	sprintf(buff, "PING_DEST_%d", index);

	// return if not more contacts, or only deleted contacts remaining
	if((pa.item_id = DBGetContactSettingDword(0, buff, "Id", 0)) == 0)	return false;

	DBVARIANT dbv;
	if(!DBGetContactSetting(0, buff, "Address", &dbv)) {
		strncpy(pa.pszName, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else return false;

	if(!DBGetContactSetting(0, buff, "Label", &dbv)) {
		strncpy(pa.pszLabel, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else return false;

	pa.status = DBGetContactSettingWord(0, buff, "Status", PS_NOTRESPONDING);
	if(pa.status != PS_DISABLED) pa.status = PS_NOTRESPONDING;

	pa.port = (int)DBGetContactSettingDword(0, buff, "Port", -1);

	if(!DBGetContactSetting(0, buff, "Proto", &dbv)) {
		strncpy(pa.pszProto, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else pa.pszProto[0] = '\0';

	if(!DBGetContactSetting(0, buff, "Command", &dbv)) {
		strncpy(pa.pszCommand, dbv.pszVal, MAX_PATH);
		DBFreeVariant(&dbv);
	} else
		pa.pszCommand[0] = '\0';
	if(!DBGetContactSetting(0, buff, "CommandParams", &dbv)) {
		strncpy(pa.pszParams, dbv.pszVal, MAX_PATH);
		DBFreeVariant(&dbv);
	} else
		pa.pszParams[0] = '\0';

	pa.set_status = DBGetContactSettingWord(0, buff, "SetStatus", ID_STATUS_ONLINE);
	pa.get_status = DBGetContactSettingWord(0, buff, "GetStatus", ID_STATUS_OFFLINE);

	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = DBGetContactSettingWord(0, buff, "Index", 0);

	pa.index = index;
	if(pa.item_id >= NextID) {
		NextID = pa.item_id + 1;
		DBWriteContactSettingDword(0, PLUG, "NextID", NextID);
	}

	return true;
}

// call with list_cs locked
void read_ping_addresses() {
	PINGADDRESS pa;

	pa.index = 0;


	list_items.clear();
	while(read_ping_address(pa)) {
		list_items.add(pa);
		pa.index++;
	}
}

INT_PTR LoadPingList(WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);
	read_ping_addresses();
	LeaveCriticalSection(&list_cs);
	NotifyEventHooks(reload_event_handle, 0, 0);	
	return 0;
}

// wParam is zero
// lParam is zero
INT_PTR SavePingList(WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);
	write_ping_addresses();
	LeaveCriticalSection(&list_cs);
	//NotifyEventHooks(reload_event_handle, 0, 0);
	
	return 0;
}

// wParam is address of a PINGLIST structure to replace the current one
// lParam is zero
INT_PTR SetPingList(WPARAM wParam, LPARAM lParam) {
	PINGLIST *pli = (PINGLIST *)wParam;
	
	EnterCriticalSection(&list_cs);
	list_items = *pli;
	LeaveCriticalSection(&list_cs);
	NotifyEventHooks(reload_event_handle, 0, 0);
	
	return 0;
}

// wParam is address of a PINGLIST structure to replace the current one
// lParam is zero
INT_PTR SetAndSavePingList(WPARAM wParam, LPARAM lParam) {
	PINGLIST *pli = (PINGLIST *)wParam;
	
	EnterCriticalSection(&list_cs);

	// set new list
	list_items = *pli;
	write_ping_addresses();
	LeaveCriticalSection(&list_cs);
	
	NotifyEventHooks(reload_event_handle, 0, 0);
	
	return 0;
}

INT_PTR ClearPingList(WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);
	list_items.clear();
	LeaveCriticalSection(&list_cs);
	
	NotifyEventHooks(reload_event_handle, 0, 0);
	return 0;
}

