#pragma once



struct CONNECTION
{
	TCHAR strIntIp[16];
	TCHAR strExtIp[16];
	int intIntPort;
	int intExtPort;
	int state;
	DWORD Pid;
	TCHAR PName[260];
	struct CONNECTION *next;
};

struct CONNECTION* GetConnectionsTable();
void deleteConnectionsTable(struct CONNECTION* head);
struct CONNECTION* searchConnection(struct CONNECTION* head, TCHAR *intIp, TCHAR *extIp, int intPort, int extPort, int state);
void getDnsName(TCHAR *strIp, TCHAR *strHostName, size_t len);