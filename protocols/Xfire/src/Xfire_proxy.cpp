#include "stdafx.h"

#include "Xfire_proxy.h"

extern HANDLE hNetlib;
HANDLE hBindPort=NULL;
HANDLE netlibcon=NULL;


void FromServerToClient(LPVOID lParam) {
	char buf[1024]={0};

	HANDLE hConnection=(HANDLE)lParam;
	do {
		if (!hBindPort) return;

		int cbRead = Netlib_Recv(netlibcon, buf, sizeof(buf), 0);
		if ( cbRead == SOCKET_ERROR)
			break;

		if (cbRead) {
			Netlib_Send(hConnection, buf, cbRead, 0);
		}
	}
	while(1);
}

//xfireclient baut verbindung auf
void XfireclientConnecting(HANDLE hConnection, DWORD, void* extra )
{
	char buf[1024]={0};

	//verbindung zum richtigen xfire server aufbauen
	NETLIBOPENCONNECTION ncon = { 0 };
	ncon.cbSize = sizeof(ncon);
	ncon.szHost = "206.220.42.147";
	ncon.wPort = (WORD)atol("25999");
	ncon.timeout=5;
	netlibcon = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) hNetlib, (LPARAM) & ncon);

	if (!netlibcon) {
		Netlib_CloseHandle(hConnection);
	}

	mir_forkthread(FromServerToClient,(LPVOID)hConnection);

	//schleife behandelt empfangende daten
	do {
		int cbRead = Netlib_Recv(hConnection, buf, sizeof(buf), 0);
		if ( cbRead == SOCKET_ERROR)
		{
			Netlib_CloseHandle(hConnection);
			Netlib_CloseHandle(netlibcon);
			break;
		}

		if (cbRead) {
			if (!Netlib_Send(netlibcon, buf, cbRead, 0))
			{
				Netlib_CloseHandle(hConnection);
				Netlib_CloseHandle(netlibcon);
				break;
			}
		}
	}
	while(1);
}

//inits nachdem alle module geladen wurden
int AfterSystemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	//init netlib handle
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_INCOMING;
	nlu.szSettingsModule = protocolname;
	nlu.szDescriptiveName = "XFire server connection";
	hNetlib = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);

	//init socket server
	NETLIBBIND nb = { sizeof(nb) };
	nb.pfnNewConnectionV2 = XfireclientConnecting;
	nb.wPort = 25999;
	hBindPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)hNetlib,(LPARAM) &nb);
	return 0;
}

int initXfireProxy()
{
	//inits nach dem alle module geladen wurden
	HookEvent(ME_SYSTEM_MODULESLOADED, AfterSystemModulesLoaded);
	return 0;
}
