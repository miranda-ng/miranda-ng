#include "ConnectionNotify.h"
 
struct CONNECTION *GetConnectionsTable()
{
	// Declare and initialize variables
	MIB_TCPTABLE_OWNER_PID *pTcpTable = (MIB_TCPTABLE_OWNER_PID *) MALLOC(sizeof (MIB_TCPTABLE_OWNER_PID));
	if (pTcpTable == NULL) {
		//printf("Error allocating memory!\n");
		return NULL;
	}

	DWORD dwSize = sizeof (MIB_TCPTABLE_OWNER_PID);
	// Make an initial call to GetTcpTable to
	// get the necessary size into the dwSize variable
	DWORD dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0);
	if (dwRetVal ==  ERROR_INSUFFICIENT_BUFFER) {
		FREE(pTcpTable);
		pTcpTable = (MIB_TCPTABLE_OWNER_PID *) MALLOC(dwSize);
		if (pTcpTable == NULL) {
			//printf("Error allocating memory\n");
			return NULL;
		}
	}

	// Make a second call to GetTcpTable to get
	// the actual data we require
	if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0)) != NO_ERROR) {
		//printf("\tGetTcpTable() failed with return value %d\n", dwRetVal);
		FREE(pTcpTable);
		return NULL;
	}
	//printf("\tLocal Addr\tLocal Port\tRemote Addr\tRemote Port\n");
	//printf("Number of entries: %d\n", (int) pTcpTable->dwNumEntries);
	struct in_addr IpAddr;
	struct CONNECTION *connHead = NULL;
	for (DWORD i = 0; i < pTcpTable->dwNumEntries; i ++) {
		struct CONNECTION *newConn = (struct CONNECTION*)mir_alloc(sizeof(struct CONNECTION));
		memset(newConn, 0, sizeof(struct CONNECTION));
		//pid2name(pTcpTable->table[i].dwOwningPid,&newConn->Pname);
		
		if (pTcpTable->table[i].dwLocalAddr) {
			IpAddr.S_un.S_addr = (ULONG) pTcpTable->table[i].dwLocalAddr;
			//_snprintf(newConn->strIntIp,_countof(newConn->strIntIp),"%d.%d.%d.%d",IpAddr.S_un.S_un_b.s_b1,IpAddr.S_un.S_un_b.s_b2,IpAddr.S_un.S_un_b.s_b3,IpAddr.S_un.S_un_b.s_b4);
			TCHAR *strIntIp = mir_a2t(inet_ntoa(IpAddr));
			_tcsncpy(newConn->strIntIp, strIntIp, SIZEOF(newConn->strIntIp)-1);
			mir_free(strIntIp);
		}
		
		if (pTcpTable->table[i].dwRemoteAddr) {
			IpAddr.S_un.S_addr = (u_long) pTcpTable->table[i].dwRemoteAddr;
			TCHAR *strExtIp = mir_a2t(inet_ntoa(IpAddr));
			_tcsncpy(newConn->strExtIp, strExtIp, SIZEOF(newConn->strExtIp)-1);
			mir_free(strExtIp);
		}
		newConn->state = pTcpTable->table[i].dwState;
		newConn->intIntPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
		newConn->intExtPort = ntohs((u_short)pTcpTable->table[i].dwRemotePort);
		newConn->Pid = pTcpTable->table[i].dwOwningPid;
		switch (pTcpTable->table[i].dwState) {
		case MIB_TCP_STATE_CLOSED:
			//printf("CLOSED\n");
			break;
		case MIB_TCP_STATE_LISTEN:
			//printf("LISTEN\n");
			break;
		case MIB_TCP_STATE_SYN_SENT:
			//printf("SYN-SENT\n");
			break;
		case MIB_TCP_STATE_SYN_RCVD:
			//printf("SYN-RECEIVED\n");
			break;
		case MIB_TCP_STATE_ESTAB:
			//printf("ESTABLISHED\n");
			break;
		case MIB_TCP_STATE_FIN_WAIT1:
			//printf("FIN-WAIT-1\n");
			break;
		case MIB_TCP_STATE_FIN_WAIT2:
			//printf("FIN-WAIT-2 \n");
			break;
		case MIB_TCP_STATE_CLOSE_WAIT:
			//printf("CLOSE-WAIT\n");
			break;
		case MIB_TCP_STATE_CLOSING:
			//printf("CLOSING\n");
			break;
		case MIB_TCP_STATE_LAST_ACK:
			//printf("LAST-ACK\n");
			break;
		case MIB_TCP_STATE_TIME_WAIT:
			//printf("TIME-WAIT\n");
			break;
		case MIB_TCP_STATE_DELETE_TCB:
			//printf("DELETE-TCB\n");
			break;
		default:
			//printf("UNKNOWN dwState value\n");
			break;
		}
		newConn->next = connHead;
		connHead = newConn;
		//printf("TCP[%d]:%s%15d%20s%15d\n", i, szLocalAddr,ntohs((u_short)pTcpTable->table[i].dwLocalPort), szRemoteAddr,ntohs((u_short)pTcpTable->table[i].dwRemotePort));
		//printf("\tTCP[%d] Local Addr: %s\n", i, szLocalAddr);
		// printf("\tTCP[%d] Local Port: %d \n", i, ntohs((u_short)pTcpTable->table[i].dwLocalPort));
		//printf("\tTCP[%d] Remote Addr: %s\n", i, szRemoteAddr);
		//printf("\tTCP[%d] Remote Port: %d\n", i, ntohs((u_short)pTcpTable->table[i].dwRemotePort));
	}
	FREE(pTcpTable);

	return connHead;
}

void deleteConnectionsTable(struct CONNECTION *head)
{
	struct CONNECTION *cur = head, *del;

	while (cur != NULL) {
		del = cur;
		cur = cur->next;
		mir_free(del);
		head = cur;
	}
	head = NULL;
}

struct CONNECTION *searchConnection(struct CONNECTION *head, TCHAR *intIp, TCHAR *extIp, int intPort, int extPort, int state)
{
	for(struct CONNECTION *cur = head; cur != NULL; cur = cur->next) {
		if (_tcscmp(cur->strIntIp, intIp) == 0 &&
		    _tcscmp(cur->strExtIp, extIp) == 0 &&
		    cur->intExtPort == extPort &&
		    cur->intIntPort == intPort &&
		    cur->state == state)
			return cur;
	}
	return NULL;
}

void getDnsName(TCHAR *strIp, TCHAR *strHostName, size_t len)
{
	in_addr iaHost;

	char *szStrIP = mir_t2a(strIp);
	iaHost.s_addr = inet_addr(szStrIP);
	mir_free(szStrIP);
	hostent *h = gethostbyaddr((char *)&iaHost, sizeof(struct in_addr), AF_INET);
	_tcsncpy_s(strHostName, len, (h == NULL) ? strIp : _A2T(h->h_name), _TRUNCATE);
}
