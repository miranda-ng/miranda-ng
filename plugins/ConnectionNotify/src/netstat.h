#pragma once



struct CONNECTION
{
	wchar_t strIntIp[16];
	wchar_t strExtIp[16];
	int intIntPort;
	int intExtPort;
	int state;
	DWORD Pid;
	wchar_t PName[260];
	struct CONNECTION *next;
};

struct CONNECTION* GetConnectionsTable();
void deleteConnectionsTable(struct CONNECTION* head);
struct CONNECTION* searchConnection(struct CONNECTION* head, wchar_t *intIp, wchar_t *extIp, int intPort, int extPort, int state);
void getDnsName(wchar_t *strIp, wchar_t *strHostName, size_t len);
