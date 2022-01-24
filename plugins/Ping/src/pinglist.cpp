#include "stdafx.h"

PINGLIST list_items;
mir_cs list_cs;
HANDLE reload_event_handle;

uint32_t NextID = 1;

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

void write_ping_address(PINGADDRESS &i)
{
	char buff[16];
	mir_snprintf(buff, "PING_DEST_%d", i.index);

	if (i.item_id == 0) {
		i.item_id = NextID++;
		g_plugin.setDword("NextID", NextID);
	}

	db_set_dw(0, buff, "Id", i.item_id);
	db_set_ws(0, buff, "Address", i.pszName);
	db_set_ws(0, buff, "Label", i.pszLabel);
	db_set_w(0, buff, "Status", i.status);
	db_set_dw(0, buff, "Port", i.port);
	db_set_s(0, buff, "Proto", i.pszProto);
	if (mir_wstrlen(i.pszCommand))
		db_set_ws(0, buff, "Command", i.pszCommand);
	else
		db_unset(0, buff, "Command");
	if (mir_wstrlen(i.pszParams))
		db_set_ws(0, buff, "CommandParams", i.pszParams);
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
	for (auto &it : list_items) {
		it.index = index;
		write_ping_address(it);
	}

	// mark further destinations in the DB as invalid
	char buff[16];
	bool found;

	do {
		found = false;
		mir_snprintf(buff, "PING_DEST_%d", index++);
		if (db_get_dw(0, buff, "Id", 0) != 0) {
			found = true;
			db_set_dw(0, buff, "Id", 0);
		}
	} while (found);
}

bool read_ping_address(PINGADDRESS &pa)
{
	int index = pa.index;

	char buff[16];
	mir_snprintf(buff, "PING_DEST_%d", index);

	// return if not more contacts, or only deleted contacts remaining
	if ((pa.item_id = db_get_dw(0, buff, "Id", 0)) == 0)	return false;

	DBVARIANT dbv;
	if (!db_get_ws(0, buff, "Address", &dbv)) {
		mir_wstrncpy(pa.pszName, dbv.pwszVal, _countof(pa.pszName));
		db_free(&dbv);
	}
	else return false;

	if (!db_get_ws(0, buff, "Label", &dbv)) {
		mir_wstrncpy(pa.pszLabel, dbv.pwszVal, _countof(pa.pszLabel));
		db_free(&dbv);
	}
	else return false;

	pa.status = db_get_w(0, buff, "Status", PS_NOTRESPONDING);
	if (pa.status != PS_DISABLED) pa.status = PS_NOTRESPONDING;

	pa.port = (int)db_get_dw(0, buff, "Port", -1);

	if (!db_get_s(0, buff, "Proto", &dbv)) {
		mir_strncpy(pa.pszProto, dbv.pszVal, _countof(pa.pszProto));
		db_free(&dbv);
	}
	else pa.pszProto[0] = '\0';

	if (!db_get_ws(0, buff, "Command", &dbv)) {
		mir_wstrncpy(pa.pszCommand, dbv.pwszVal, _countof(pa.pszCommand));
		db_free(&dbv);
	}
	else
		pa.pszCommand[0] = '\0';
	if (!db_get_ws(0, buff, "CommandParams", &dbv)) {
		mir_wstrncpy(pa.pszParams, dbv.pwszVal, _countof(pa.pszParams));
		db_free(&dbv);
	}
	else
		pa.pszParams[0] = '\0';

	pa.set_status = db_get_w(0, buff, "SetStatus", ID_STATUS_ONLINE);
	pa.get_status = db_get_w(0, buff, "GetStatus", ID_STATUS_OFFLINE);

	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = db_get_w(0, buff, "Index", 0);

	pa.index = index;
	if (pa.item_id >= NextID) {
		NextID = pa.item_id + 1;
		g_plugin.setDword("NextID", NextID);
	}

	return true;
}

// call with list_cs locked
void read_ping_addresses()
{
	PINGADDRESS pa;
	pa.index = 0;

	list_items.clear();
	while (read_ping_address(pa)) {
		list_items.push_back(pa);
		++pa.index;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// copies data into this structure

void GetPingList(PINGLIST &list)
{
	mir_cslock lck(list_cs);
	list = list_items;
}

int GetListSize()
{
	mir_cslock lck(list_cs);
	return (int)list_items.size();
}

void LoadPingList()
{
	{	mir_cslock lck(list_cs);
		read_ping_addresses();
	}
	
	NotifyEventHooks(reload_event_handle, 0, 0);
}

void SavePingList()
{
	mir_cslock lck(list_cs);
	write_ping_addresses();
}

void SetPingList(const PINGLIST &list)
{
	{	mir_cslock lck(list_cs);
		list_items = list;
	}

	NotifyEventHooks(reload_event_handle, 0, 0);
}

void SetAndSavePingList(const PINGLIST &list)
{
	{	mir_cslock lck(list_cs);
		list_items = list;
		write_ping_addresses();
	}

	NotifyEventHooks(reload_event_handle, 0, 0);
}

void ClearPingList()
{
	{	mir_cslock lck(list_cs);
		list_items.clear();
	}

	NotifyEventHooks(reload_event_handle, 0, 0);
}
