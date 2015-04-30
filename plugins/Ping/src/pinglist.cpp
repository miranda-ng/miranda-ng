#include "common.h"

PINGLIST list_items;
mir_cs list_cs;
HANDLE reload_event_handle;

DWORD NextID = 1;

BOOL clist_handle_changing = FALSE;

BOOL changing_clist_handle()
{
	return clist_handle_changing;
}


void set_changing_clist_handle(BOOL flag)
{
	clist_handle_changing = flag;
}


const bool PINGADDRESS::operator==(const PINGADDRESS &b) const
{
	return index == b.index;
}


const bool PINGADDRESS::operator<(const PINGADDRESS &b) const
{
	return index < b.index;
}

// lParam is address of pointer to a std::list<PINGADDRESS> 
// copies data into this structure
INT_PTR GetPingList(WPARAM,LPARAM lParam)
{
	PINGLIST *pa = (PINGLIST *)lParam;

	mir_cslock lck(list_cs);
	*pa = list_items;
	
	return 0;
}

INT_PTR GetListSize(WPARAM, LPARAM)
{
	INT_PTR ret = 0;
	mir_cslock lck(list_cs);
	ret = list_items.size();
	return ret;
}


void write_ping_address(PINGADDRESS &i)
{
	char buff[16];
	mir_snprintf(buff, SIZEOF(buff), "PING_DEST_%d", i.index);

	if(i.item_id == 0) {
		i.item_id = NextID++;
		db_set_dw(0, PLUG, "NextID", NextID);
	}

	db_set_dw(0, buff, "Id", i.item_id);
	db_set_ts(0, buff, "Address", i.pszName);
	db_set_ts(0, buff, "Label", i.pszLabel);
	db_set_w(0, buff, "Status", i.status);
	db_set_dw(0, buff, "Port", i.port);
	db_set_s(0, buff, "Proto", i.pszProto);
	if(_tcslen(i.pszCommand))
		db_set_ts(0, buff, "Command", i.pszCommand);
	else
		db_unset(0, buff, "Command");
	if(_tcslen(i.pszParams))
		db_set_ts(0, buff, "CommandParams", i.pszParams);
	else
		db_unset(0, buff, "CommandParams");
	db_set_w(0, buff, "SetStatus", i.set_status);
	db_set_w(0, buff, "GetStatus", i.get_status);
	db_set_w(0, buff, "Index", i.index);
}

// call with list_cs locked
void write_ping_addresses()
{
	int index = 0;
	for(pinglist_it i = list_items.begin(); i != list_items.end(); ++i, index++)
	{
		i->index = index;
		write_ping_address(*i);
	}

	// mark further destinations in the DB as invalid
	char buff[16];
	bool found;

	do {
		found = false;
		mir_snprintf(buff, SIZEOF(buff), "PING_DEST_%d", index++);
		if(db_get_dw(0, buff, "Id", 0) != 0) {
			found = true;
			db_set_dw(0, buff, "Id", 0);
		}
	} while(found);
}

bool read_ping_address(PINGADDRESS &pa) {
	int index = pa.index;

	char buff[16];
	mir_snprintf(buff, SIZEOF(buff), "PING_DEST_%d", index);

	// return if not more contacts, or only deleted contacts remaining
	if((pa.item_id = db_get_dw(0, buff, "Id", 0)) == 0)	return false;

	DBVARIANT dbv;
	if(!db_get_ts(0, buff, "Address", &dbv)) {
		_tcsncpy(pa.pszName, dbv.ptszVal, MAX_PINGADDRESS_STRING_LENGTH);
		db_free(&dbv);
	} else return false;

	if(!db_get_ts(0, buff, "Label", &dbv)) {
		_tcsncpy(pa.pszLabel, dbv.ptszVal, MAX_PINGADDRESS_STRING_LENGTH);
		db_free(&dbv);
	} else return false;

	pa.status = db_get_w(0, buff, "Status", PS_NOTRESPONDING);
	if(pa.status != PS_DISABLED) pa.status = PS_NOTRESPONDING;

	pa.port = (int)db_get_dw(0, buff, "Port", -1);

	if(!db_get_s(0, buff, "Proto", &dbv)) {
		strncpy(pa.pszProto, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		db_free(&dbv);
	} else pa.pszProto[0] = '\0';

	if(!db_get_ts(0, buff, "Command", &dbv)) {
		_tcsncpy(pa.pszCommand, dbv.ptszVal, MAX_PATH);
		db_free(&dbv);
	} else
		pa.pszCommand[0] = '\0';
	if(!db_get_ts(0, buff, "CommandParams", &dbv)) {
		_tcsncpy(pa.pszParams, dbv.ptszVal, MAX_PATH);
		db_free(&dbv);
	} else
		pa.pszParams[0] = '\0';

	pa.set_status = db_get_w(0, buff, "SetStatus", ID_STATUS_ONLINE);
	pa.get_status = db_get_w(0, buff, "GetStatus", ID_STATUS_OFFLINE);

	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = db_get_w(0, buff, "Index", 0);

	pa.index = index;
	if(pa.item_id >= NextID) {
		NextID = pa.item_id + 1;
		db_set_dw(0, PLUG, "NextID", NextID);
	}

	return true;
}

// call with list_cs locked
void read_ping_addresses()
{
	PINGADDRESS pa;
	pa.index = 0;

	list_items.clear();
	while (read_ping_address(pa))
	{
		list_items.push_back(pa);
		++pa.index;
	}
}


INT_PTR LoadPingList(WPARAM wParam, LPARAM lParam)
{
	mir_cslock lck(list_cs);
	read_ping_addresses();
	NotifyEventHooks(reload_event_handle, 0, 0);
	return 0;
}

// wParam is zero
// lParam is zero
INT_PTR SavePingList(WPARAM wParam, LPARAM lParam)
{
	mir_cslock lck(list_cs);
	write_ping_addresses();

	return 0;
}

// wParam is address of a PINGLIST structure to replace the current one
// lParam is zero
INT_PTR SetPingList(WPARAM wParam, LPARAM lParam)
{
	PINGLIST *pli = (PINGLIST *)wParam;

	mir_cslock lck(list_cs);
	list_items = *pli;
	NotifyEventHooks(reload_event_handle, 0, 0);

	return 0;
}

// wParam is address of a PINGLIST structure to replace the current one
// lParam is zero
INT_PTR SetAndSavePingList(WPARAM wParam, LPARAM lParam)
{
	PINGLIST *pli = (PINGLIST *)wParam;

	mir_cslock lck(list_cs);

	// set new list
	list_items = *pli;
	write_ping_addresses();

	NotifyEventHooks(reload_event_handle, 0, 0);

	return 0;
}

INT_PTR ClearPingList(WPARAM wParam, LPARAM lParam)
{
	mir_cslock lck(list_cs);
	list_items.clear();

	NotifyEventHooks(reload_event_handle, 0, 0);
	return 0;
}

