/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2007-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"
#include <netfw.h>

#ifndef CLSID_NetFwMgr
#define MDEF_CLSID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const CLSID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	MDEF_CLSID(CLSID_NetFwMgr, 0x304ce942, 0x6e39, 0x40d8, 0x94, 0x3a, 0xb9, 0x13, 0xc4, 0x0c, 0x9c, 0xd4);
	MDEF_CLSID(IID_INetFwMgr, 0xf7898af5, 0xcac4, 0x4632, 0xa2, 0xec, 0xda ,0x06, 0xe5, 0x11, 0x1a, 0xf2);
#endif


MyConnectionType MyConnection;

const char* conStr[] =
{
	"Unknown-Connect",
	"Direct-Connect",
	"Unknown-NAT",
	"IP-Restrict-NAT",
	"Port-Restrict-NAT",
	"Symmetric-NAT",
	"Firewall",
	"ISALike"
};


void CMsnProto::DecryptEchoPacket(UDPProbePkt& pkt)
{
	pkt.clientPort ^= 0x3141;
	pkt.discardPort ^= 0x3141;
	pkt.testPort ^= 0x3141;
	pkt.clientIP ^= 0x31413141;
	pkt.testIP ^= 0x31413141;


	IN_ADDR addr;
	debugLogA("Echo packet: version: 0x%x  service code: 0x%x  transaction ID: 0x%x",
		pkt.version, pkt.serviceCode, pkt.trId);
	addr.S_un.S_addr = pkt.clientIP;
	debugLogA("Echo packet: client port: %u  client addr: %s",
		pkt.clientPort, inet_ntoa(addr));
	addr.S_un.S_addr = pkt.testIP;
	debugLogA("Echo packet: discard port: %u  test port: %u test addr: %s",
		pkt.discardPort, pkt.testPort, inet_ntoa(addr));
}


static void DiscardExtraPackets(SOCKET s)
{
	Sleep(3000);

	static const TIMEVAL tv = {0, 0};
	unsigned buf;

	for (;;)
	{
		if (Miranda_Terminated()) break;

		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(s, &fd);

		if (select(1, &fd, NULL, NULL, &tv) == 1)
			recv(s, (char*)&buf, sizeof(buf), 0);
		else
			break;
	}
}


void CMsnProto::MSNatDetect(void)
{
	unsigned i;

	PHOSTENT host = gethostbyname("echo.edge.messenger.live.com");
	if (host == NULL)
	{
		debugLogA("P2PNAT could not find echo server \"echo.edge.messenger.live.com\"");
		return;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = _htons(7001);
	addr.sin_addr = *( PIN_ADDR )host->h_addr_list[0];

	debugLogA("P2PNAT Detected echo server IP %d.%d.%d.%d",
		addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2,
		addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4);

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	connect(s, (SOCKADDR*)&addr, sizeof(addr));

	UDPProbePkt pkt = { 0 };
	UDPProbePkt pkt2;

	// Detect My IP
	pkt.version = 2;
	pkt.serviceCode = 4;
	send(s, (char*)&pkt, sizeof(pkt), 0);

	SOCKADDR_IN  myaddr;
	int szname = sizeof(myaddr);
	getsockname(s, (SOCKADDR*)&myaddr, &szname);

	MyConnection.intIP = myaddr.sin_addr.S_un.S_addr;
	debugLogA("P2PNAT Detected IP facing internet %d.%d.%d.%d",
		myaddr.sin_addr.S_un.S_un_b.s_b1, myaddr.sin_addr.S_un.S_un_b.s_b2,
		myaddr.sin_addr.S_un.S_un_b.s_b3, myaddr.sin_addr.S_un.S_un_b.s_b4);

	SOCKET s1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	pkt.version = 2;
	pkt.serviceCode = 1;
	pkt.clientPort = 0x3141;
	pkt.clientIP = 0x31413141;

	UDPProbePkt rpkt = {0};

	// NAT detection
	for (i=0; i<4; ++i)
	{
		if (Miranda_Terminated()) break;

		// Send echo request to server 1
		debugLogA("P2PNAT Request 1 attempt %d sent", i);
		sendto(s1, (char*)&pkt, sizeof(pkt), 0, (SOCKADDR*)&addr, sizeof(addr));

		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(s1, &fd);
		TIMEVAL tv = {0, 200000 * (1 << i) };

		if (select(1, &fd, NULL, NULL, &tv) == 1)
		{
			debugLogA("P2PNAT Request 1 attempt %d response", i);
			recv(s1, (char*)&rpkt, sizeof(rpkt), 0);
			pkt2 = rpkt;
			DecryptEchoPacket(rpkt);
			break;
		}
		else
			debugLogA("P2PNAT Request 1 attempt %d timeout", i);
	}

	closesocket(s);

	// Server did not respond
	if (i >= 4)
	{
		MyConnection.udpConType = conFirewall;
		closesocket(s1);
		return;
	}

	MyConnection.extIP = rpkt.clientIP;

	// Check if NAT not found
	if (MyConnection.extIP == MyConnection.intIP)
	{
		if (msnExternalIP != NULL && inet_addr(msnExternalIP) != MyConnection.extIP)
			MyConnection.udpConType = conISALike;
		else
			MyConnection.udpConType = conDirect;

		closesocket(s1);
		return;
	}

	// Detect UPnP NAT
	NETLIBBIND nlb = {0};
	nlb.cbSize = sizeof(nlb);
	nlb.pfnNewConnectionV2 = MSN_ConnectionProc;
	nlb.pExtra = this;

	HANDLE sb = (HANDLE) CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hNetlibUser, (LPARAM)&nlb);
	if ( sb != NULL )
	{
		MyConnection.upnpNAT = htonl(nlb.dwExternalIP) == MyConnection.extIP;
		Sleep(100);
		Netlib_CloseHandle(sb);
	}

	DiscardExtraPackets(s1);

	// Start IP Restricted NAT detection
	UDPProbePkt rpkt2 = {0};
	pkt2.serviceCode = 3;
	SOCKADDR_IN addr2 = addr;
	addr2.sin_addr.S_un.S_addr = rpkt.testIP;
	addr2.sin_port = rpkt.discardPort;
	for (i=0; i<4; ++i)
	{
		if (Miranda_Terminated()) break;

		debugLogA("P2PNAT Request 2 attempt %d sent", i);
		// Remove IP restriction for server 2
		sendto(s1, NULL, 0, 0, (SOCKADDR*)&addr2, sizeof(addr2));
		// Send echo request to server 1 for server 2
		sendto(s1, (char*)&pkt2, sizeof(pkt2), 0, (SOCKADDR*)&addr, sizeof(addr));

		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(s1, &fd);
		TIMEVAL tv = {0, 200000 * (1 << i) };

		if (select(1, &fd, NULL, NULL, &tv) == 1)
		{
			debugLogA("P2PNAT Request 2 attempt %d response", i);
			recv(s1, (char*)&rpkt2, sizeof(rpkt2), 0);
			DecryptEchoPacket(rpkt2);
			break;
		}
		else
			debugLogA("P2PNAT Request 2 attempt %d timeout", i);
	}

	// Response recieved so it's an IP Restricted NAT (Restricted Cone NAT)
	// (MSN does not detect Full Cone NAT and consider it as IP Restricted NAT)
	if (i < 4)
	{
		MyConnection.udpConType = conIPRestrictNAT;
		closesocket(s1);
		return;
	}

	DiscardExtraPackets(s1);

	// Symmetric NAT detection
	addr2.sin_port = rpkt.testPort;
	for (i=0; i<4; ++i)
	{
		if (Miranda_Terminated()) break;

		debugLogA("P2PNAT Request 3 attempt %d sent", i);
		// Send echo request to server 1
		sendto(s1, (char*)&pkt, sizeof(pkt), 0, (SOCKADDR*)&addr2, sizeof(addr2));

		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(s1, &fd);
		TIMEVAL tv = {1 << i, 0 };

		if ( select(1, &fd, NULL, NULL, &tv) == 1 )
		{
			debugLogA("P2PNAT Request 3 attempt %d response", i);
			recv(s1, (char*)&rpkt2, sizeof(rpkt2), 0);
			DecryptEchoPacket(rpkt2);
			break;
		}
		else
			debugLogA("P2PNAT Request 3 attempt %d timeout", i);
	}
	if (i < 4)
	{
		// If ports different it's symmetric NAT
		MyConnection.udpConType = rpkt.clientPort == rpkt2.clientPort ?
			conPortRestrictNAT : conSymmetricNAT;
	}
	closesocket(s1);
}


static bool IsIcfEnabled(void)
{
	HRESULT hr;
	VARIANT_BOOL fwEnabled = VARIANT_FALSE;

	INetFwProfile* fwProfile = NULL;
	INetFwMgr* fwMgr = NULL;
	INetFwPolicy* fwPolicy = NULL;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;
	BSTR fwBstrProcessImageFileName = NULL;
	wchar_t *wszFileName = NULL;

	hr = CoInitialize(NULL);
	if (FAILED(hr)) return false;

	// Create an instance of the firewall settings manager.
	hr = CoCreateInstance(CLSID_NetFwMgr, NULL, CLSCTX_INPROC_SERVER,
			IID_INetFwMgr, (void**)&fwMgr );
	if (FAILED(hr)) goto error;

	// Retrieve the local firewall policy.
	hr = fwMgr->get_LocalPolicy(&fwPolicy);
	if (FAILED(hr)) goto error;

	// Retrieve the firewall profile currently in effect.
	hr = fwPolicy->get_CurrentProfile(&fwProfile);
	if (FAILED(hr)) goto error;

	// Get the current state of the firewall.
	hr = fwProfile->get_FirewallEnabled(&fwEnabled);
	if (FAILED(hr)) goto error;

	if (fwEnabled == VARIANT_FALSE) goto error;

	// Retrieve the authorized application collection.
	hr = fwProfile->get_AuthorizedApplications(&fwApps);
	if (FAILED(hr)) goto error;

	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, SIZEOF(szFileName));

	wszFileName = mir_t2u(szFileName);

	// Allocate a BSTR for the process image file name.
	fwBstrProcessImageFileName = SysAllocString(wszFileName);
	if (FAILED(hr)) goto error;

	// Attempt to retrieve the authorized application.
	hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
	if (SUCCEEDED(hr))
	{
		// Find out if the authorized application is enabled.
		fwApp->get_Enabled(&fwEnabled);
		fwEnabled = ~fwEnabled;
	}

error:
	// Free the BSTR.
	SysFreeString(fwBstrProcessImageFileName);
	mir_free(wszFileName);

	// Release the authorized application instance.
	if (fwApp != NULL) fwApp->Release();

	// Release the authorized application collection.
	if (fwApps != NULL) fwApps->Release();

	// Release the firewall profile.
	if (fwProfile != NULL) fwProfile->Release();

	// Release the local firewall policy.
	if (fwPolicy != NULL) fwPolicy->Release();

	// Release the firewall settings manager.
	if (fwMgr != NULL) fwMgr->Release();

	CoUninitialize();

	return fwEnabled != VARIANT_FALSE;
}


void CMsnProto::MSNConnDetectThread( void* )
{
	char parBuf[512] = "";

	memset(&MyConnection, 0, sizeof(MyConnection));

	MyConnection.icf = IsIcfEnabled();
	bool portsMapped = getByte("NLSpecifyIncomingPorts", 0) != 0;

	unsigned gethst = getByte("AutoGetHost", 1);
	switch (gethst)
	{
		case 0:
			debugLogA("P2PNAT User overwrote IP connection is guessed by user settings only");

			// User specified host by himself so check if it matches MSN information
			// if it does, move to connection type autodetection,
			// if it does not, guess connection type from available info
			db_get_static(NULL, m_szModuleName, "YourHost", parBuf, sizeof(parBuf));
			if (msnExternalIP == NULL || strcmp(msnExternalIP, parBuf) != 0)
			{
				MyConnection.extIP = inet_addr(parBuf);
				if (MyConnection.extIP == INADDR_NONE)
				{
					PHOSTENT myhost = gethostbyname(parBuf);
					if (myhost != NULL)
						MyConnection.extIP = ((PIN_ADDR)myhost->h_addr)->S_un.S_addr;
					else
						setByte("AutoGetHost", 1);
				}
				if (MyConnection.extIP != INADDR_NONE)
				{
					MyConnection.intIP = MyConnection.extIP;
					MyConnection.udpConType = MyConnection.extIP ? (ConEnum)portsMapped : conUnknown;
					MyConnection.CalculateWeight();
					return;
				}
				else
					MyConnection.extIP = 0;
			}
			break;

		case 1:
			if (msnExternalIP != NULL)
				MyConnection.extIP = inet_addr(msnExternalIP);
			else
			{
				gethostname(parBuf, sizeof(parBuf));
				PHOSTENT myhost = gethostbyname(parBuf);
				if (myhost != NULL)
					MyConnection.extIP = ((PIN_ADDR)myhost->h_addr)->S_un.S_addr;
			}
			MyConnection.intIP = MyConnection.extIP;
			break;

		case 2:
			MyConnection.udpConType = conUnknown;
			MyConnection.CalculateWeight();
			return;
	}

	if (getByte( "NLSpecifyOutgoingPorts", 0))
	{
		// User specified outgoing ports so the connection must be firewalled
		// do not autodetect and guess connection type from available info
		MyConnection.intIP = MyConnection.extIP;
		MyConnection.udpConType = (ConEnum)portsMapped;
		MyConnection.upnpNAT = false;
		MyConnection.CalculateWeight();
		return;
	}

	MSNatDetect();

	// If user mapped incoming ports consider direct connection
	if (portsMapped)
	{
		debugLogA("P2PNAT User manually mapped ports for incoming connection");
		switch(MyConnection.udpConType)
		{
		case conUnknown:
		case conFirewall:
		case conISALike:
			MyConnection.udpConType = conDirect;
			break;

		case conUnknownNAT:
		case conPortRestrictNAT:
		case conIPRestrictNAT:
		case conSymmetricNAT:
			MyConnection.upnpNAT = true;
			break;
		}
	}

	debugLogA("P2PNAT Connection %s found UPnP: %d ICF: %d", conStr[MyConnection.udpConType],
		MyConnection.upnpNAT, MyConnection.icf);

	MyConnection.CalculateWeight();
}


void MyConnectionType::SetUdpCon(const char* str)
{
	for (unsigned i=0; i<sizeof(conStr)/sizeof(char*); ++i)
	{
		if (strcmp(conStr[i], str) == 0)
		{
			udpConType = (ConEnum)i;
			break;
		}
	}
}


void MyConnectionType::CalculateWeight(void)
{
	if (icf) weight = 0;
	else if (udpConType == conDirect) weight = 6;
	else if (udpConType >= conIPRestrictNAT && udpConType <= conSymmetricNAT)
		weight = upnpNAT ? 5 : 2;
	else if (udpConType == conUnknownNAT)
		weight = upnpNAT ? 4 : 1;
	else if (udpConType == conUnknown) weight = 1;
	else if (udpConType == conFirewall) weight = 2;
	else if (udpConType == conISALike) weight = 3;
}
