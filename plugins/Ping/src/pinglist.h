#ifndef _PINGLIST_H
#define _PINGLIST_H

#define PS_RESPONDING		1
#define PS_NOTRESPONDING	2
#define PS_TESTING			3
#define PS_DISABLED			4

#define MAX_PINGADDRESS_STRING_LENGTH	256

struct PINGADDRESS
{
	int cbSize;	//size in bytes of this structure
	uint32_t item_id;
	wchar_t pszName[MAX_PINGADDRESS_STRING_LENGTH];		//IP address or domain name
	wchar_t pszLabel[MAX_PINGADDRESS_STRING_LENGTH];
	bool responding;
	int status;
	short round_trip_time;
	int miss_count;
	int port; // -1 for ICMP, non-zero for TCP
	char pszProto[MAX_PINGADDRESS_STRING_LENGTH];
	wchar_t pszCommand[MAX_PATH];
	wchar_t pszParams[MAX_PATH];
	unsigned int get_status;		// on success, if status equals this
	unsigned int set_status;	// set it to this
	int index;

	const bool operator==(const PINGADDRESS &b) const;
	const bool operator<(const PINGADDRESS &b) const;
};

typedef std::list<PINGADDRESS> PINGLIST;
typedef std::list<PINGADDRESS>::iterator pinglist_it;

void ClearPingList(void);
int  GetListSize(void);
void GetPingList(PINGLIST &list);
void LoadPingList(void);
void SavePingList(void);
void SetPingList(const PINGLIST &list); // use when you modified db yourself
void SetAndSavePingList(const PINGLIST &list);

// only call with list_cs locked!
void write_ping_addresses();

BOOL changing_clist_handle();
void set_changing_clist_handle(BOOL flag);
#endif
