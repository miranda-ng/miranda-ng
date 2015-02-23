/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2010 by
 *          dufte <dufte@justmail.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *  Based on J. Lawler              - BaseProtocol
 *			 Herbert Poul/Beat Wolf - xfirelib
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

#include "stdafx.h"

#include <winsock2.h>
#include "tools.h"
#include "xdebug.h"

extern HANDLE hNetlib;
//convert buf to hexstring
/*char* tohex(unsigned char*buf,int size) {
	static char buffer[1024*10]="";

	strcpy(buffer,"");

	for(int i=0;i<size;i++)
	{
	if (i%16==0&&i!=0)
	mir_snprintf(buffer, SIZEOF(buffer), "%s\n%02x ", buffer, buf[i]);
	else
	mir_snprintf(buffer, SIZEOF(buffer), "%s%02x ", buffer, buf[i]);
	}

	return buffer;
	}*/

//von icqproto kopiert
void EnableDlgItem(HWND hwndDlg, UINT control, int state)
{
	EnableWindow(GetDlgItem(hwndDlg, control), state);
}


//eigene string replace funktion, da die von der std:string klasse immer abstürzt
BOOL str_replace(char*src, char*find, char*rep)
{
	string strpath = src;
	int pos = strpath.find(find);

	if (pos > -1)
	{
		char *temp = new char[strlen(src) + strlen(rep) + 1];

		strcpy(temp, src);
		*(temp + pos) = 0;

		strcat(temp, rep);
		strcat(temp, (src + pos + strlen(find)));
		strcpy(src, temp);

		delete[] temp;

		return TRUE;
	}
	return FALSE;
}

/* popup darstellen */
int displayPopup(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, HICON hicon)
{
	static signed char	bUsePopups = -1;
	static BOOL			bIconsNotLoaded = TRUE;

	static HICON hicNotify = NULL, hicWarning = NULL, hicError = NULL;

	if ((uType & MB_TYPEMASK) == MB_OK) {
		POPUPDATAT ppd = { 0 };
		ppd.lchIcon = hicon;

		if (bIconsNotLoaded) {
			hicNotify = Skin_GetIcon("popup_notify");
			hicWarning = Skin_GetIcon("popup_warning");
			hicError = Skin_GetIcon("popup_error");
			bIconsNotLoaded = FALSE;
		}

		mir_tstrncpy(ppd.lptzContactName, _A2T(lpCaption), SIZEOF(ppd.lptzContactName));
		mir_tstrncpy(ppd.lptzText, _A2T(lpText), SIZEOF(ppd.lptzText));

		if ((uType & MB_ICONMASK) == MB_ICONSTOP) {
			ppd.lchIcon = hicError;
			ppd.colorBack = RGB(191, 0, 0);
			ppd.colorText = RGB(255, 245, 225);
		}
		else if ((uType & MB_ICONMASK) == MB_ICONWARNING) {
			ppd.lchIcon = hicWarning;
			ppd.colorBack = RGB(210, 210, 150);
			ppd.colorText = RGB(0, 0, 0);
		}
		else {
			ppd.lchIcon = hicNotify;
			ppd.colorBack = RGB(230, 230, 230);
			ppd.colorText = RGB(0, 0, 0);
		}

		PUAddPopupT(&ppd);
	}

	return IDOK;
}

char*menuitemtext(char*mtext)
{
	static char temp[256] = "";
	int anz = 0;
	int j = 0;

	if (!mtext)
		return NULL;

	int size = strlen(mtext);

	if (!size || size > 255)
		return mtext;

	//alle & zeichen zählen
	for (int i = 0; i < size; i++, j++)
	{
		temp[j] = mtext[i];
		if (mtext[i] == '&')
		{
			j++;
			temp[j] = '&';
		}
	}
	//terminieren
	temp[j] = 0;

	return temp;
}

void Message(LPVOID msg)
{
	switch (db_get_b(NULL, protocolname, "nomsgbox", 0))
	{
	case 1:
		return;
	case 2:
		displayPopup(NULL, (LPCSTR)msg, PLUGIN_TITLE, MB_OK);
		return;
	}

	MSGBOXPARAMSA mbp;
	mbp.cbSize = sizeof(mbp);
	mbp.hwndOwner = NULL;
	mbp.hInstance = hinstance;
	mbp.lpszText = (char*)msg;
	mbp.lpszCaption = PLUGIN_TITLE;
	mbp.dwStyle = MB_USERICON;
	mbp.lpszIcon = MAKEINTRESOURCEA(IDI_TM);
	mbp.dwContextHelpId = NULL;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = LANG_ENGLISH;
	MessageBoxIndirectA(&mbp);
	//MessageBoxA(0,(char*)msg,PLUGIN_TITLE,MB_OK|MB_ICONINFORMATION);
}

void MessageE(LPVOID msg)
{
	static BOOL already = FALSE;
	switch (db_get_b(NULL, protocolname, "nomsgbox", 0))
	{
	case 0:
		if (!already)
		{
			already = TRUE; //keine doppelte fehlernachrichten
			Message(msg);
			already = FALSE;
		}
		break;
	case 2:
		displayPopup(NULL, (LPCSTR)msg, PLUGIN_TITLE, MB_OK | MB_ICONSTOP);
		break;
	}
}

//funktion soll pfad erkennen und zurückgeben
char* GetLaunchPath(char*launch)
{
	static char temp[XFIRE_MAX_STATIC_STRING_LEN] = "";
	char find[] = ".exe ";  //gesucht wird
	char * p = temp;
	char * f = find;

	if (launch == NULL)
		return temp;

	strcpy(temp, launch);

	while (*p != 0 && *f != 0)
	{
		if (tolower(*p) == *f)
		{
			f++;
		}
		else
			f = find;

		p++;
	}

	if (*f == 0)
	{
		*p = 0;
	}
	else
		return temp;

	if (strrchr(temp, '\\'))
	{
		*(strrchr(temp, '\\')) = 0;
	}

	return temp;
}
//roll bits, vllt ein tickschneller als die funktionen von winsock
unsigned short r(unsigned short data)
{
#if defined(_WIN64)
	return ((data & 0xFF) << 8) + (data >> 8);
#else
	_asm {
		mov ax, data
			rol ax, 8
			mov data, ax
	}
	return data;
#endif
}

//simple und hoffetnlich schnelle teamspeakdetection
BOOL FindTeamSpeak(DWORD*pid, int*vid) {
	BOOL found = FALSE;
	if (pid == NULL)
		return FALSE;

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	PROCESSENTRY32* processInfo = new PROCESSENTRY32;
	processInfo->dwSize = sizeof(PROCESSENTRY32);

	//	XFireLog("Scanning for voiceprograms...");


	while (Process32Next(hSnapShot, processInfo) != FALSE)
	{
		if (processInfo->th32ProcessID != 0) {
			int size = _tcslen(processInfo->szExeFile);

			if (size == 13)
			{
				if ((processInfo->szExeFile[0] == 'T' || processInfo->szExeFile[0] == 't') &&
					processInfo->szExeFile[1] == 'e'&&
					processInfo->szExeFile[2] == 'a'&&
					processInfo->szExeFile[3] == 'm'&&
					processInfo->szExeFile[4] == 'S'&&
					processInfo->szExeFile[5] == 'p'&&
					processInfo->szExeFile[6] == 'e'&&
					processInfo->szExeFile[7] == 'a'&&
					processInfo->szExeFile[8] == 'k')
				{
					*pid = processInfo->th32ProcessID;
					found = TRUE;
					*vid = 32;
					break;
				}
			}
			else if (size == 12)
			{
				if ((processInfo->szExeFile[0] == 'V' || processInfo->szExeFile[0] == 'v') &&
					processInfo->szExeFile[1] == 'e'&&
					processInfo->szExeFile[2] == 'n'&&
					processInfo->szExeFile[3] == 't'&&
					processInfo->szExeFile[4] == 'r'&&
					processInfo->szExeFile[5] == 'i'&&
					processInfo->szExeFile[6] == 'l'&&
					processInfo->szExeFile[7] == 'o')
				{
					*pid = processInfo->th32ProcessID;
					found = TRUE;
					*vid = 33;
					break;
				}
			}
			else if (size == 10)
			{
				if ((processInfo->szExeFile[0] == 'm' || processInfo->szExeFile[0] == 'M') &&
					processInfo->szExeFile[1] == 'u'&&
					processInfo->szExeFile[2] == 'm'&&
					processInfo->szExeFile[3] == 'b'&&
					processInfo->szExeFile[4] == 'l'&&
					processInfo->szExeFile[5] == 'e')
				{
					*pid = processInfo->th32ProcessID;
					found = TRUE;
					*vid = 34;
					break;
				}
			}
		}
	}
	CloseHandle(hSnapShot);
	return found;
}

#include <vector>

#define maxuppackets 4

//funktion liefer ip/port einer verbindung
BOOL GetServerIPPort(DWORD pid, char*localaddrr, unsigned long localaddr, char*ip1, char*ip2, char*ip3, char*ip4, long*port)
{
	static std::vector<int> localport;
	static const int hdrInclude = 1;
	static int lastip = 0;
	static int lastport = 0;
	static int lastpid = 0;

	//DUMP("***Suche IP/Port***","");

	if (pid != lastpid)
	{
		lastip = lastport = 0;
		lastpid = pid;
	}

	DWORD size = 0;

	GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	MIB_UDPTABLE_OWNER_PID *ptab = (MIB_UDPTABLE_OWNER_PID*)malloc(size);
	if (ptab == NULL)
		return FALSE;
	int ret = GetExtendedUdpTable(ptab, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	//alle grad geöffnet updverb nach der pid vom spiel suchen, um an den port ranzukommen
	if (ret == NO_ERROR)
	{
		BOOL notfound = TRUE;
		for (unsigned int i = 0; i < ptab->dwNumEntries; i++)
		{
			if (ptab->table[i].dwOwningPid == pid) //spiel gefunden
			{
				localport.push_back(ptab->table[i].dwLocalPort);
				//DUMP("Localport: %d",ptab->table[i].dwLocalPort);
				//localport=; //port wird gesichert
				//break; //wir brauchen nicht mehr suchen
				notfound = FALSE;
			}
		}
		if (notfound) //kein port gefunden
		{
			//DUMP("Kein Localport gefunden","");
			XFireLog("no local port found");
			return FALSE; //dann erstmal schluss
		}
	}
	else
	{
		XFireLog("GetExtendedUdpTable error!");
		return FALSE;
	}

	free(ptab); //speicher frei machn


	//socker erstellen
	SOCKET s;
	s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		//DUMP("Kann Rawsocket nicht erstellen. Error: %d",WSAGetLastError());
		XFireLog("unable to create raw socket %d", WSAGetLastError());
		closesocket(s);
		return FALSE;
	}

	static struct sockaddr_in msockaddr;
	memset(&msockaddr, 0, sizeof(msockaddr));
	msockaddr.sin_addr.s_addr = localaddr;
	msockaddr.sin_family = AF_INET;
	msockaddr.sin_port = 0;

	//socket an nw binden
	if (bind(s, (sockaddr *)&msockaddr, sizeof(msockaddr)) == SOCKET_ERROR)
	{
		//DUMP("Kann Rawsocket nicht binden. Error: %d",WSAGetLastError());
		XFireLog("unable to bind raw socket %d", WSAGetLastError());
		closesocket(s);
		return FALSE;
	}

	//wir wollen alles was da reinkommt haben
	static int I = 1;
	static DWORD b;
	if (WSAIoctl(s, _WSAIOW(IOC_VENDOR, 1), &I, sizeof(I), NULL, NULL, &b, NULL, NULL) == SOCKET_ERROR)
	{
		//DUMP("IOCTL Error","");
		/*closesocket(s);
		return FALSE;*/
		XFireLog("IOCTL error %d", WSAGetLastError());
		//unter bestimmten umständen schlägt es hier fehl, dann lass trotzdem ip weiter erkennen
	}

	//socket soll timeout auswerfen, wenn nix kommt, damit der gamethread nicht hängt
	//DUMP("timeout>>>","");
	static int timeout = 200;
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) == SOCKET_ERROR))
	{
		XFireLog("setsockopt(SO_RCVTIMEO) error %d", WSAGetLastError());
	}

	//updstruct, nur mit wichtigen sachen
	struct mpacket {
		unsigned char ipv;
		char dmp[11]; //dummy
		//srcip, serverip
		unsigned char ip1;
		unsigned char ip2;
		unsigned char ip3;
		unsigned char ip4;
		//unsere nw
		unsigned int ipdst;
		char temp[1024];
	};
	struct mpacket2 {
		unsigned char ipv;
		char dmp[11]; //dummy
		//srcip, serverip
		unsigned long srcip;
		//server ip
		unsigned char ip1;
		unsigned char ip2;
		unsigned char ip3;
		unsigned char ip4;
		char temp[1024];
	};
	struct udp {
		//srcport
		u_short srcport;
		//dstport
		u_short dstport;
	};

	mpacket temp = { 0 }; //empfamngsbuffer
	udp * temp2;
	char * temp3;
	mpacket2 * temp4;

	for (int I = 0; I < maxuppackets; I++) //maximal 4 packete, das reicht
	{
		int msize = recv(s, (char*)&temp, sizeof(mpacket), 0);
		if (msize) //empfangen
		{
			/*DUMP("Packet empfangen","");
			DUMP("Dump Full packet##############","");
			DUMP(tohex((unsigned char*)&temp,msize),"");
			DUMP("Dump Full packet##############","");

			DUMP("Headersize: %d",(temp.ipv & 0x0f)*4);*/

			temp3 = (char*)&temp;
			temp3 += (temp.ipv & 0x0f) * 4;
			temp2 = (udp*)temp3;
			temp4 = (mpacket2*)&temp;

			/*DUMP("Dump Udp##############","");
			DUMP(tohex((unsigned char*)temp2,sizeof(udp)),"");
			DUMP("Dump Udp##############","");*/


			for (unsigned int i = 0; i < localport.size(); i++)
			{
				//DUMP("destport %d ==",temp2->dstport);
				//DUMP("== %d",localport.at(i));
				if (temp2->dstport == localport.at(i)/*FIX: für XP SP3 ->*/ && temp4->srcip != localaddr) //ist das ziel des packets, gleich dem port des spiels
				{
					*port = r(temp2->srcport); //ja dann serverdaten an gamethread übermitteln
					*ip1 = temp.ip1;
					*ip2 = temp.ip2;
					*ip3 = temp.ip3;
					*ip4 = temp.ip4;
					closesocket(s); //socket zumachn

					//DUMP("SourceIP %d",temp4->srcip);
					//DUMP("SourcePort %d",temp2->srcport);
					if (lastip != temp4->srcip || temp2->srcport != lastport)
					{
						lastport = temp2->srcport; //fixed port wechsel, damit dieser auch mitgetielt wird, wenn zb vorher nur serverinfos angefordert wurden
						lastip = temp4->srcip;
						closesocket(s);
						//DUMP("IP gefunden","");
						XFireLog("got ip!");
						return TRUE;
					}

					XFireLog("no serverip found!");
					return FALSE;
				}
				/*	else if (temp4->srcip==localaddr && temp2->srcport==localport.at(i)) //gesendete gamepackets
					{
					*port=r(temp2->dstport); //ja dann serverdaten an gamethread übermitteln
					*ip1=temp4->ip1;
					*ip2=temp4->ip2;
					*ip3=temp4->ip3;
					*ip4=temp4->ip4;
					closesocket(s); //socket zumachn
					return TRUE;
					}*/
			}
		}
		else if (msize == SOCKET_ERROR)
		{
			XFireLog("recv() error %d", WSAGetLastError());
		}
	}
	closesocket(s); //socket zumachn
	lastip = 0;
	lastport = 0;
	return TRUE;
}

//funktion liefert ip/port einer verbindung, dupliziert für teamspeak/ventrilo, wegen static vals
//TODO: eventuell umbauen, damit es für beide genutzt werden kann
BOOL GetServerIPPort2(DWORD pid, char*localaddrr, unsigned long localaddr, char*ip1, char*ip2, char*ip3, char*ip4, long*port)
{
	static std::vector<int> localport;
	static const int hdrInclude = 1;
	static int lastip = 0;
	static int lastpid = 0;
	static int lastport = 0;

	if (pid != lastpid)
	{
		lastip = lastport = 0;
		lastpid = pid;
	}

	DWORD size = 0;

	GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	MIB_UDPTABLE_OWNER_PID *ptab = (MIB_UDPTABLE_OWNER_PID*)malloc(size);
	if (ptab == NULL)
		return FALSE;
	int ret = GetExtendedUdpTable(ptab, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	//alle grad geöffnet updverb nach der pid vom spiel suchen, um an den port ranzukommen
	if (ret == NO_ERROR)
	{
		BOOL notfound = TRUE;
		for (unsigned int i = 0; i < ptab->dwNumEntries; i++)
		{
			if (ptab->table[i].dwOwningPid == pid) //spiel gefunden
			{
				localport.push_back(ptab->table[i].dwLocalPort);
				//localport=; //port wird gesichert
				//break; //wir brauchen nicht mehr suchen
				notfound = FALSE;
			}
		}
		if (notfound) //kein port gefunden
		{
			if (lastip != 0)
			{
				lastip = 0;
				lastport = 0;
				return TRUE;
			}
			return FALSE; //dann erstmal schluss
		}
	}
	else
		return FALSE;

	free(ptab); //speicher frei machn


	//socker erstellen
	SOCKET s;
	s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

	static struct sockaddr_in msockaddr;
	memset(&msockaddr, 0, sizeof(msockaddr));
	msockaddr.sin_addr.s_addr = localaddr;
	msockaddr.sin_family = AF_INET;
	msockaddr.sin_port = 0;

	//socket an nw binden
	if (bind(s, (sockaddr *)&msockaddr, sizeof(msockaddr)) == SOCKET_ERROR)
	{
		closesocket(s);
		return FALSE;
	}

	//wir wollen alles was da reinkommt haben
	static int I = 1;
	DWORD b;
	if (WSAIoctl(s, _WSAIOW(IOC_VENDOR, 1), &I, sizeof(I), NULL, NULL, &b, NULL, NULL) == SOCKET_ERROR)
	{
		/*closesocket(s);
		return FALSE;*/
		//unter bestimmten umständen schlägt es hier fehl, dann lass trotzdem ip weiter erkennen
	}

	//socket soll timeout auswerfen, wenn nix kommt, damit der gamethread nicht hängt
	//DUMP("timeout>>>","");
	static int timeout = 200;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	//updstruct, nur mit wichtigen sachen
	struct mpacket {
		unsigned char ipv;
		char dmp[11]; //dummy
		//srcip, serverip
		unsigned char ip1;
		unsigned char ip2;
		unsigned char ip3;
		unsigned char ip4;
		//unsere nw
		unsigned int ipdst;
		char temp[1024];
	};
	struct mpacket2 {
		unsigned char ipv;
		char dmp[11]; //dummy
		//srcip, serverip
		unsigned long srcip;
		//server ip
		unsigned char ip1;
		unsigned char ip2;
		unsigned char ip3;
		unsigned char ip4;
		char temp[1024];
	};
	struct udp {
		//srcport
		u_short srcport;
		//dstport
		u_short dstport;
	};

	mpacket temp = { 0 }; //empfamngsbuffer
	udp * temp2;
	char * temp3;
	mpacket2 * temp4;

	for (int I = 0; I < maxuppackets; I++) //maximal 4 packete, das reicht
	{
		int msize = recv(s, (char*)&temp, sizeof(mpacket), 0);
		if (msize) //empfangen
		{
			temp3 = (char*)&temp;
			temp3 += (temp.ipv & 0x0f) * 4;
			temp2 = (udp*)temp3;
			temp4 = (mpacket2*)&temp;

			for (unsigned int i = 0; i < localport.size(); i++)
				if (temp2->dstport == localport.at(i)/*FIX: für XP SP3 ->*/ && temp4->srcip != localaddr) //ist das ziel des packets, gleich dem port des spiels
				{
				*port = r(temp2->srcport); //ja dann serverdaten an gamethread übermitteln
				*ip1 = temp.ip1;
				*ip2 = temp.ip2;
				*ip3 = temp.ip3;
				*ip4 = temp.ip4;
				closesocket(s); //socket zumachn

				if (lastip != temp4->srcip || temp2->srcport != lastport)
				{
					lastport = temp2->srcport; //fixed port wechsel, damit dieser auch mitgetielt wird, wenn zb vorher nur serverinfos angefordert wurden
					lastip = temp4->srcip;
					return TRUE;
				}

				return FALSE;
				}
			/*	else if (temp4->srcip==localaddr && temp2->srcport==localport.at(i)) //gesendete gamepackets
				{
				*port=r(temp2->dstport); //ja dann serverdaten an gamethread übermitteln
				*ip1=temp4->ip1;
				*ip2=temp4->ip2;
				*ip3=temp4->ip3;
				*ip4=temp4->ip4;
				closesocket(s); //socket zumachn
				return TRUE;
				}*/
		}
	}
	closesocket(s); //socket zumachn
	lastip = 0;
	lastport = 0;
	return TRUE;
}


char * getItem(char * string, char delim, int count)
{
	static char item[255];
	char i = 0;

	while (*string != '\0'&&count > 0)
	{
		if (*string == delim)
		{
			item[i] = 0;
			i = 0;
			count--;
			string++;
		}
		else {
			item[i] = *string;
			i++;
			string++;
		}
	}
	if (*string == '\0')
		item[i] = 0;
	if (count > 1)
		item[0] = 0;

	for (unsigned int i = 0; i < strlen(item); i++)
	{
		item[i] = tolower(item[i]);
	}

	return item;
}

// soll commandline der spiele prüfen
//
// TRUE bedeutet, entweder ok, oder konnte wegen systemfehler nicht geprüft werden
// FALSE bedeutet beinhaltet nicht das, was es soll
//
// get process comamndline quelle hier:
// http://forum.sysinternals.com/forum_posts.asp?TID=6510
#define cb 1024

pZwQueryInformationProcess _ZwQueryInformationProcess = NULL;
//pZwClose _ZwClose = NULL;
pZwReadVirtualMemory _ZwReadVirtualMemory = NULL;

BOOL checkCommandLine(HANDLE hProcess, char * mustcontain, char * mustnotcontain)
{
	WCHAR * buffer;
	char * buffer2;
	PPEB peb = NULL;
	PPROCESS_PARAMETERS proc_params = NULL;
	PVOID UserPool = (PVOID)LocalAlloc(LPTR, 8192);
	PROCESS_BASIC_INFORMATION ProcessInfo;

	//strings leer abbruch
	if (!mustcontain&&!mustnotcontain)
		return TRUE;

	//prüfe und lade nötige funktionen
	if (_ZwQueryInformationProcess == NULL)
	{
		_ZwQueryInformationProcess = (pZwQueryInformationProcess)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwQueryInformationProcess");
		if (_ZwQueryInformationProcess == NULL)
		{
			return TRUE;
		}
	}
	if (_ZwReadVirtualMemory == NULL)
	{
		_ZwReadVirtualMemory = (pZwReadVirtualMemory)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "ZwReadVirtualMemory");
		if (_ZwReadVirtualMemory == NULL)
		{
			return TRUE;
		}
	}

	//commandline bekommen, siehe link oben
	ULONG rc = _ZwQueryInformationProcess(hProcess, ProcessBasicInformation, &ProcessInfo, sizeof(ProcessInfo), NULL);

	rc = _ZwReadVirtualMemory(hProcess, ProcessInfo.PebBaseAddress, UserPool, sizeof(PEB), NULL);

	peb = (PPEB)UserPool;
	rc = _ZwReadVirtualMemory(hProcess, peb->ProcessParameters, UserPool, sizeof(PROCESS_PARAMETERS), NULL);

	proc_params = (PPROCESS_PARAMETERS)UserPool;
	ULONG uSize = 0;
	LPVOID pBaseAddress = NULL;

	uSize = proc_params->CommandLine.Length;
	pBaseAddress = proc_params->CommandLine.Buffer;

	//keine commandline?!
	if (uSize == 0 || pBaseAddress == NULL)
	{
		LocalFree(UserPool);
		return FALSE;
	}

	buffer = (WCHAR*)new char[uSize];

	rc = _ZwReadVirtualMemory(hProcess, pBaseAddress, buffer, uSize, NULL);

	//in ansi umwandeln
	int correctsize = WideCharToMultiByte(CP_OEMCP, 0, buffer, -1, NULL, 0, NULL, NULL);

	if (correctsize == 0)
	{
		LocalFree(UserPool);
		return FALSE;
	}

	buffer2 = new char[correctsize];
	WideCharToMultiByte(CP_OEMCP, 0, buffer, -1, buffer2, correctsize, NULL, NULL);
	buffer2[correctsize - 1] = 0;


	for (unsigned int i = 0; i < strlen(buffer2); i++)
	{
		buffer2[i] = tolower(buffer2[i]);
	}

	//lowercase mustcontain/mustnotcontain
	if (mustcontain)
		for (unsigned int i = 0; i < strlen(mustcontain); i++)
		{
		mustcontain[i] = tolower(mustcontain[i]);
		}
	if (mustnotcontain)
		for (unsigned int i = 0; i < strlen(mustnotcontain); i++)
		{
		mustnotcontain[i] = tolower(mustnotcontain[i]);
		}

	string cmdline = buffer2;

	if (mustcontain)
		if (cmdline.find(mustcontain) != string::npos)
		{
		delete[] buffer;
		delete[] buffer2;
		LocalFree(UserPool);
		return TRUE;
		}
		else
		{
			delete[] buffer;
			delete[] buffer2;
			LocalFree(UserPool);
			return FALSE;
		}

	int count = 1;
	if (mustnotcontain)
	{
		char*str = getItem(mustnotcontain, ';', count);
		do {
			if (cmdline.find(str) != string::npos)
			{
				delete[] buffer;
				delete[] buffer2;
				LocalFree(UserPool);
				return FALSE;
			}
			count++;
			str = getItem(mustnotcontain, ';', count);
		} while (*str != 0);
	}

	//_ZwClose(hProcess);
	LocalFree(UserPool);
	delete[] buffer;
	delete[] buffer2;

	return TRUE;
}

#define RECV_BUFFER_SIZE 6144

BOOL CheckWWWContent(char*address) {
	Netlib_Logf(hNetlib, "Check Url %s ...", address);

	//netlib request
	NETLIBHTTPREQUEST nlhr = { 0 };
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_HEAD;
	nlhr.flags = NLHRF_NODUMP | NLHRF_GENERATEHOST | NLHRF_SMARTAUTHHEADER;
	nlhr.szUrl = address;

	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlib, (LPARAM)&nlhr);

	if (nlhrReply) {
		//nicht auf dem server
		Netlib_Logf(hNetlib, "Resultcode %d ...", nlhrReply->resultCode);
		if (nlhrReply->resultCode != 200) {
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
			return FALSE;
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	else
		return FALSE;

	return TRUE;
}


BOOL GetWWWContent2(char*address, char*filename, BOOL dontoverwrite, char**tobuf, unsigned int* size) {
	if (dontoverwrite == TRUE)
	{
		if (GetFileAttributesA(filename) != 0xFFFFFFFF)
		{
			Netlib_Logf(hNetlib, "%s already exists, no overwrite.", filename);
			return TRUE;
		}
	}
	Netlib_Logf(hNetlib, "Download Url %s ...", address);

	//netlib request
	NETLIBHTTPREQUEST nlhr = { 0 };
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_NODUMP | NLHRF_GENERATEHOST | NLHRF_SMARTAUTHHEADER;
	nlhr.szUrl = address;

	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlib, (LPARAM)&nlhr);

	if (nlhrReply) {
		//nicht auf dem server
		if (nlhrReply->resultCode != 200) {
			Netlib_Logf(hNetlib, "Bad statuscode: %d", nlhrReply->resultCode);
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
			return FALSE;
		}
		//keine daten für mich
		else if (nlhrReply->dataLength < 1 || nlhrReply->pData == NULL)
		{
			Netlib_Logf(hNetlib, "No data received.");
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
			return FALSE;
		}
		else
		{
			if (tobuf == NULL)
			{
				FILE * f = fopen(filename, "wb");
				if (f == NULL)
				{
					Netlib_Logf(hNetlib, "Cannot open %s for binary write mode.", filename);
					CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
					return FALSE;
				}
				fwrite(nlhrReply->pData, nlhrReply->dataLength, 1, f);
				fclose(f);
			}
			else
			{
				if (*tobuf == NULL)
				{
					*tobuf = new char[nlhrReply->dataLength + 1];
					memcpy_s(*tobuf, nlhrReply->dataLength, nlhrReply->pData, nlhrReply->dataLength);
					//0 terminieren
					(*tobuf)[nlhrReply->dataLength] = 0;
					//größe zurückliefern, wenn gewollt
					if (size)
						*size = nlhrReply->dataLength + 1;
				}
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	else
	{
		Netlib_Logf(hNetlib, "No valid Netlib Request.", filename);
		return FALSE;
	}
	return TRUE;
}
//eigener www downloader, da winet exceptions erzeugt
BOOL GetWWWContent(char*host, char* request, char*filename, BOOL dontoverwrite) {
	char add[1024] = "http://";
	strcat(add, host);
	strcat(add, request);

	return GetWWWContent2(add, filename, dontoverwrite);
}

unsigned int getfilesize(char*path)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return 0;
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fclose(f);
	return size;
}

//funktion soll erst in der userini suchen, danach in der xfire_games.ini
DWORD xfire_GetPrivateProfileString(__in   LPCSTR lpAppName, __in   LPCSTR lpKeyName, __in   LPCSTR lpDefault, __out  LPSTR lpReturnedString, __in   DWORD nSize, __in   LPCSTR lpFileName) {
	//xfire_games.ini
	int size = strlen(lpFileName);
	if (size > 15)
	{
		char*file = (char*)lpFileName;
		int ret = 0;
		*(file + size - 14) = 'u';
		*(file + size - 13) = 's';
		*(file + size - 12) = 'e';
		*(file + size - 11) = 'r';
		ret = GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
		if (ret)
		{
			return ret;
		}
		else
		{
			*(file + size - 14) = 'f';
			*(file + size - 13) = 'i';
			*(file + size - 12) = 'r';
			*(file + size - 11) = 'e';
			return GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
		}
	}
	return GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
}


BOOL mySleep(int ms, HANDLE evt) {
	switch (WaitForSingleObject(evt, ms))
	{
	case WAIT_TIMEOUT:
		return FALSE;
	case WAIT_ABANDONED:
		//MessageBoxA(NULL,"Abbruch","Abbruch",0);
		return TRUE;
	default:
		return TRUE;
	}

	return FALSE;
}