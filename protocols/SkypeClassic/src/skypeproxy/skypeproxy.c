/* 

Purpose
=======
This program opens a connection on a local TCP/IP port and sends/
receives Skype-API calls on it so that you can remote-control
Skype over a network.
Note, that there are currently NO SECURITY mechanisms, so don't
use this over an untrusted network!
  
Author
======
This program was written by leecher in 2005 (mailto:leecher@dose.0wnz.at)
Please give feedback at http://forum.skype.com/viewtopic.php?t=16187

Protocol
========
Basic protocol structure
------------------------
Sender and receiver have the same protocol:

  [(UINT)Length of message][(char[])Message]

The Length is so that you can malloc() enough space for the data buffer
to receive the next message.

A special case is, if the [Length of message] is 0. In this case the
client tells the server that he wants to switch to command mode.

Command mode
------------
The server expects

  [(char)Command]

next. Currently the following commands are supported:

  CAPABILITIES		- returns the Server's capabilities
  AUTHENTICATE		- Starts the authentication process

CAPABILITIES
------------
The server returns

  [(char)Capabilities]

where this currently can be the following:

  USE_AUTHENTICATION - The server supports and requires authentication

AUTHENTICATE
------------
The server returns

  [(char)0x01]

if authentication is supported AND needed (skypeproxy started with -k switch) or

  [(char)0x00]

if this is not the case.
If 0x01 was returned the server next expects a normal message 
(see "Basic protocol structure) containing the password. 
If the authentication was successful, the server replies with

  [(char)0x01]

otherwise with

  [(char)0x00]

PLEASE NOTE THAT THE AUTHENTICATION CURRENTLY IS PLAIN TEXT. SO DON'T
USE THIS PROGRAM OVER AN UNTRUSTED NETWORK, OTHERWISE THERE MAY BE THE
POSSIBILITY THAT SOMEONE SNIFFS YOUR PASSWORD!

Code example
------------

SOCKET MySocket;

int SendPacket(char *szSkypeMessage) {
	unsigned int length=strlen(szSkypeMsg);

	if (send(MySocket, (char *)&length, sizeof(length), 0)==SOCKET_ERROR ||
		send(MySocket, szSkypeMsg, length, 0)==SOCKET_ERROR) 
			return -1;
	return 0;
}

// don't forget to free() the return value on my Heap!!
char *ReceivePacket(void) {
	unsigned int lenght, received;
	char *buf;

	if ((received=recv(MySocket, (char *)&length, sizeof(length), 0))==SOCKET_ERROR ||
		received==0) 
			return NULL;
	if (!(buf=calloc(1, length+1))) return NULL;
	if (recv(MySocket, buf, length, 0)==SOCKET_ERROR) {
		free(buf);
		return NULL;
	}
	return buf;
}	


License
=======
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

History
========
V1.0alpha - First preview release
V1.0	  - Implemented killing & restarting Skype process when it dies
		  - BUGFIX: SendMessage() is a blocking call, if Skype hangs our app hangs too -> Fixed
		  - Added command line parsing.
		  - Renamed from Skype2Socket to skypeproxy
		  - Added authentication feature.
*/

#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <signal.h>
#include "skypeproxy.h"

UINT ControlAPIAttach, ControlAPIDiscover;
HWND hSkypeWnd=NULL, hWnd;
HANDLE SkypeReady, ServerThreadBye;
LONG AttachStatus=-1;
int exitcode=EXIT_SUCCESS;
char skype_path[MAX_PATH], *password=NULL;
BYTE WatchDog=1;
BOOL WatchDogRunning=FALSE, Authenticated=FALSE;
SOCKET ListenSocket, AcceptSocket;


void bail_out(int i) {
	OUTPUT("Got termination signal, bailing out.");
	if (i==1) exitcode=EXIT_FAILURE;
	PostMessage(hWnd, WM_QUIT, 0, 0);
}

BOOL CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam ) {
  DWORD dwID ;

  GetWindowThreadProcessId(hwnd, &dwID) ;
  if(dwID == (DWORD)lParam)
	PostMessage(hwnd, WM_CLOSE, 0, 0) ;	// May you be so kind to quit, please?

  return TRUE ;
}

/* ConnectToSkypeAPI
 * 
 * Purpose: Establish a connection to the Skype API
 * Params : ForceRestart - Kill Skype if it's running before restarting
 * Returns: 0 - Connecting succeeded
 *		   -1 - Something went wrong
 */
void ConnectToSkypeAPI(void *ForceRestart) {
	BOOL SkypeLaunched=FALSE;
	int counter=0, i, j;
	char *args[5];
	char *SkypeOptions[]={"/notray", "/nosplash", "/minimized"};
	char *szFuncName="ConnectToSkypeAPI";

	ResetEvent(SkypeReady);
	AttachStatus=-1;
	if ((BOOL)ForceRestart) {
		HANDLE hProc;
		DWORD dwPID=0;

		if (!hSkypeWnd) {
			OUTPUT("I can't kill Skype, as I don't even have its window handle!");
			return;
		}
		GetWindowThreadProcessId(hSkypeWnd, &dwPID);
		LOG(("%s: Shutting down Skype as it was not behaving the way it should...", szFuncName));
		if (hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, dwPID)) {

			// Try to shutdown Skype the nice way by asking it to close
			EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM) dwPID);

			if(WaitForSingleObject(hProc, 10000)!=WAIT_OBJECT_0) {
				// Try it the hard way by killing it
				LOG(("%s: I tried it the nice way, but you were not listening! Now DIIIIEEE!", szFuncName));
				if (!TerminateProcess(hProc,0)) {
					OUTPUT("Argh, process refused to die, it's too mighty for me, I've given up");
					CloseHandle(hProc);
					return;
				}
				LOG(("%s: Process killed! >:)", szFuncName));
			}
			CloseHandle(hProc);
		}
	}
	do {
		/*	To initiate communication, Client should broadcast windows message
			('SkypeControlAPIDiscover') to all windows in the system, specifying its own
			window handle in wParam parameter.
		 */
		LOG(("%s: Sending discover message..", szFuncName));
		SendMessageTimeout(HWND_BROADCAST, ControlAPIDiscover, (WPARAM)hWnd, 0, SMTO_ABORTIFHUNG, 3000, NULL);
		LOG(("%s: Discover message sent, waiting for Skype to become ready..", szFuncName));

		/*	In response, Skype responds with
			message 'SkypeControlAPIAttach' to the handle specified, and indicates
			connection status
			SkypeReady is set if there is an answer by Skype other than API_AVAILABLE.
			If there is no answer after 3 seconds, launch Skype as it's propably
			not running.
		*/
		if (WaitForSingleObject(SkypeReady, 3000)==WAIT_TIMEOUT &&
			AttachStatus!=SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION) 
		{
			if (hWnd==NULL) {
				LOG(("%s: hWnd of SkypeDispatchWindow not yet set..", szFuncName));
				continue;
			}
			if (!SkypeLaunched && skype_path) {
				LOG(("%s: Starting Skype, as it's not running", szFuncName));
				args[0]=skype_path;
				j=1;
				for (i=0; i<3; i++) {
					args[j]=SkypeOptions[i];
					LOG(("%s: Using Skype parameter: ", szFuncName, args[j]));
					j++;
				}
				args[j]=NULL;
				_spawnv(_P_NOWAIT, skype_path, args);
				ResetEvent(SkypeReady);
				SkypeLaunched=TRUE;
				LOG(("%s: Skype process started.", szFuncName));
				// Skype launching iniciated, keep sending Discover messages until it responds.
				continue;
			} else {
				LOG(("%s: Check if Skype was launchable..", szFuncName));
				if (!skype_path) {
					OUTPUT("There was no correct path for Skype application");
					bail_out(1);
					return;
				}
				LOG("%s: Trying to attach: #%d", szFuncName, counter));
				counter++;
				if (counter==5) {
					OUTPUT("ERROR: Skype not running / too old / working!");
					bail_out(1);
					return;
				}
			}
		}
		LOG(("%s: Attachstatus %d", szFuncName, AttachStatus));
	} while (AttachStatus==SKYPECONTROLAPI_ATTACH_API_AVAILABLE || AttachStatus==-1);
	
	while (AttachStatus==SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION) Sleep(1000);
	LOG(("%s: Attachstatus %d", szFuncName, AttachStatus));
	if (AttachStatus!=SKYPECONTROLAPI_ATTACH_SUCCESS) {
		switch(AttachStatus) {
			case SKYPECONTROLAPI_ATTACH_REFUSED:
				OUTPUT("Skype refused the connection :(");
				break;
			case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
				OUTPUT("The Skype API is not available");
				break;
			default:
				LOG(("%s: ERROR: AttachStatus: %d", szFuncName, AttachStatus));
				OUTPUT("Wheee, Skype won't let me use the API. :(");
		}
		bail_out(1);
		return;
	}
	OUTPUT("Attached to Skype successfully.");
	if (!WatchDogRunning)
		if (_beginthread(WatchDogTimer, 0, NULL)==-1) {
			OUTPUT("Cannot start Watchdog.");
			bail_out(1);
		}
	return;
}

void SkypeSend(char *szMsg) {
   COPYDATASTRUCT CopyData;
   int count=0;

   if (!hSkypeWnd) {
	   LOG(("SkypeSend: DAMN! No Skype window handle! :("));
	   return;
   }
   if (mir_strcmp(szMsg, "PING")) {LOG(("> %s", szMsg));}
   CopyData.dwData=0; 
   CopyData.lpData=szMsg; 
   CopyData.cbData=strlen(szMsg)+1;
   while (!SendMessageTimeout(hSkypeWnd, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&CopyData, SMTO_ABORTIFHUNG, 3000, NULL)) {
	   count++;
	   LOG(("SkypeSend: failed, try #%d", count));
	   if (count==5) {
		   OUTPUT("Sending message to Skype failed too often.");
		   OUTPUT("Skype may have died unexpectedly, I will try to restart it.");
		   ConnectToSkypeAPI((void *)TRUE);
		   OUTPUT("Restart complete. Trying to deliver message one more time.");
		   if (!SendMessageTimeout(hSkypeWnd, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&CopyData, SMTO_ABORTIFHUNG, 3000, NULL)) {
			   OUTPUT("It still failed. Skype seems to be completely f*cked up. I've given up. Bye..");
			   bail_out(1);
			   break;
		   } else { 
			   OUTPUT("Now it worked! :)");
			   break;
		   }
	   }
	   Sleep(1000);
   }
}

void ServerThread(char *dummy) {
	unsigned int length, received;
	char *buf, command, reply;

	LOG(("ServerThread started"));
	AcceptSocket=INVALID_SOCKET;
	while( AcceptSocket == INVALID_SOCKET) {
		if ((AcceptSocket = accept( ListenSocket, NULL, NULL ))==INVALID_SOCKET) {
			LOG(("ServerThread: Byebye..."));
			SetEvent(ServerThreadBye);
			bail_out(1);
			return;
		}
		OUTPUT("Connection by client");
		while(1) {
			if ((received=recv(AcceptSocket, (char *)&length, sizeof(length), 0))==SOCKET_ERROR ||
				received==0) 
			{
				OUTPUT("Connection was closed by client. See ya soon! :)");
				break;
			}
			// Command mode
			if (length==0) {
				reply=0;
				if (recv(AcceptSocket, (char *)&command, 1, 0)==SOCKET_ERROR) {
					OUTPUT("Connection to client was lost.");
					break;
				}
#ifdef USE_AUTHENTICATION
				if (command==AUTHENTICATE)
					if (password) reply=0x01;		 // Ok, go ahead
					else command=0;
#endif
				if (command==CAPABILITIES)
					reply=password?USE_AUTHENTICATION:0;

				if (send(AcceptSocket, (char *)&reply, 1, 0)==SOCKET_ERROR) {
					OUTPUT("Connection to client was lost.");
					break;
				}
				continue;
			}
			// Normal Skype API-call
			if (!(buf=calloc(1, length+1))) {
				OUTPUT("Out of memory error while allocating buffer space.");
				break;
			}
			if (recv(AcceptSocket, buf, length, 0)==SOCKET_ERROR) {
				OUTPUT("Connection to client was lost.");
				free(buf);
				break;
			}
			switch (command) {
#ifdef USE_AUTHENTICATION
			case 0x01: // Compare hash
				if (password && !mir_strcmp(password, buf)) Authenticated=TRUE;
				else Authenticated=FALSE;
				if (Authenticated) {
					OUTPUT("User authenticated succesfully.");
					reply=1; 
				} else {
					OUTPUT("User authentication failed!! (Intruder?)");
					reply=0;
				}
				if (send(AcceptSocket, (char *)&reply, 1, 0)==SOCKET_ERROR) {
					OUTPUT("Connection to client was lost.");
					break;
				}
				command=0;
				break;
#endif
			default:
#ifdef USE_AUTHENTICATION
				if (password && !Authenticated) break;
#endif
				SkypeSend(buf);
			}
			command=0;
			free(buf);
		}
		AcceptSocket=INVALID_SOCKET;
#ifdef USE_AUTHENTICATION
		Authenticated=FALSE;
#endif
	}
}


void WatchDogTimer(char *dummy) {
	LOG(("WatchDogTimer started"));
	WatchDogRunning=TRUE;
	while (1) {
		Sleep(PING_INTERVAL);
		if (!WatchDog) {
			OUTPUT("Ouch.. It seems that Skype has died unexpectedly. Trying to restart.");
			ConnectToSkypeAPI((void *)TRUE);
		}
		WatchDog=0;
		SkypeSend("PING");
	}
}

LONG APIENTRY WndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam) 
{ 
    PCOPYDATASTRUCT CopyData; 
	char *szSkypeMsg=NULL;

    switch (message) 
    { 
        case WM_COPYDATA: 
//		 LOG("WM_COPYDATA", "start");
		 if(hSkypeWnd==(HWND)wParam) { 
			CopyData=(PCOPYDATASTRUCT)lParam;
			szSkypeMsg=strdup(CopyData->lpData);
			ReplyMessage(1);
			if (!mir_strcmp(szSkypeMsg, "PONG")) {
				WatchDog=1;
				break;
			} // Hide PING-PONG
			LOG(("< %s", szSkypeMsg));
			if (!mir_strcmp(szSkypeMsg, "USERSTATUS LOGGEDOUT")) {
				OUTPUT("Skype shut down gracefully. I'll leave too, bye.. :)");
				bail_out(1);
			}
#ifdef USE_AUTHENTICATION
			if (password && !Authenticated) break;
#endif
			if (AcceptSocket!=INVALID_SOCKET) {
				unsigned int length=strlen(szSkypeMsg);

				if (send(AcceptSocket, (char *)&length, sizeof(length), 0)==SOCKET_ERROR ||
					send(AcceptSocket, szSkypeMsg, length, 0)==SOCKET_ERROR) 
					OUTPUT("Cannot send to client :(");
			}
		 }
        break; 

        case WM_DESTROY: 
            PostQuitMessage(0); 
        break; 

        default: 
		 if(message==ControlAPIAttach) {
				// Skype responds with Attach to the discover-message
				AttachStatus=lParam;
				if (AttachStatus==SKYPECONTROLAPI_ATTACH_SUCCESS) 
					hSkypeWnd=(HWND)wParam;	   // Skype gave us the communication window handle
				if (AttachStatus!=SKYPECONTROLAPI_ATTACH_API_AVAILABLE)
					SetEvent(SkypeReady);
				break;
		 }
		 return (DefWindowProc(hWnd, message, wParam, lParam)); 
    }
//	LOG("WM_COPYDATA", "exit");
	if (szSkypeMsg) free(szSkypeMsg);
	return 1;
} 


void TellError(DWORD err) {
	LPVOID lpMsgBuf;
	
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
        MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
        LocalFree( lpMsgBuf );
		return;
}

int main(int argc, char *argv[]) {
	DWORD Buffsize;
	HKEY MyKey;
	BOOL SkypeInstalled=TRUE;
    MSG Message; 
	WNDCLASS WndClass; 
	SOCKADDR_IN service;
	WSADATA wsaData;
	int ExitCode=STILL_ACTIVE;
	unsigned short BindPort=1401;
	char BindIP[16]="0.0.0.0";
	
	printf("Skypeproxy V1.0, by leecher 2005 <leecher@dose.0wnz.at>\n\n");

	if (argc>1) {
		int i;

		if (!mir_strcmpi(argv[1], "-h") || !mir_strcmpi(argv[1], "--help") || !mir_strcmpi(argv[1], "/?")) {
			printf("Usage: %s [-i BindIP] [-p BindPort]", argv[0]);
#ifdef USE_AUTHENTICATION
			printf(" [-k Password]");
#endif
			printf("\n\n");
			return EXIT_SUCCESS;
		}
		for (i=0;i<argc;i++) {
			if (!mir_strcmpi(argv[i], "-i") && argc>i+1)
				strncpy(BindIP, argv[i+1], sizeof(BindIP));
			if (!mir_strcmpi(argv[i], "-p") && argc>i+1) 
				if (!(BindPort=atoi(argv[i+1]))) {
					OUTPUT("ERROR: Cannot convert port to int. bye..");
					return EXIT_FAILURE;
				}
#ifdef USE_AUTHENTICATION
			if (!mir_strcmpi(argv[i], "-k") && argc>i+1)
				password=strdup(argv[i+1]);
#endif
		}
	}

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Skype\\Phone", 0, KEY_READ, &MyKey)!=ERROR_SUCCESS ||
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Skype\\Phone", 0, KEY_READ, &MyKey)!=ERROR_SUCCESS)
			SkypeInstalled=FALSE;
	Buffsize=sizeof(skype_path);
	if (SkypeInstalled==FALSE || 
		RegQueryValueEx(MyKey, "SkypePath", NULL, NULL, skype_path,  &Buffsize)!=ERROR_SUCCESS) {
		    OUTPUT("Skype was not found on this machine :(");
			RegCloseKey(MyKey);
			skype_path[0]=0;
			return EXIT_FAILURE;
	}
	RegCloseKey(MyKey);

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR) {
		OUTPUT("Error at loading windows sockets.");
		return EXIT_FAILURE;
	}

	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		printf("* Error at creating socket(): Error %d", WSAGetLastError());
		return EXIT_FAILURE;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(BindIP);
	service.sin_port = htons(BindPort);

	printf("* Binding to interface %s, Port %d..", BindIP, BindPort);
	if (bind( ListenSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR ||
		listen( ListenSocket, 1 ) == SOCKET_ERROR)
	{
		OUTPUT("Failed.");
		closesocket(ListenSocket);
		return EXIT_FAILURE;
	}
	printf("OK\n");


	if (!(ControlAPIAttach=RegisterWindowMessage("SkypeControlAPIAttach")) ||
		!(ControlAPIDiscover=RegisterWindowMessage("SkypeControlAPIDiscover"))) {
			OUTPUT("Cannot register Windows message.");
			closesocket(ListenSocket);
			return EXIT_FAILURE;
	}
	
	// Create window class
	hSkypeWnd=NULL;
    WndClass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS; 
    WndClass.lpfnWndProc = (WNDPROC)WndProc; 
    WndClass.cbClsExtra = 0; 
    WndClass.cbWndExtra = 0; 
    WndClass.hInstance =  NULL;
    WndClass.hIcon = NULL; 
    WndClass.hCursor = NULL;
    WndClass.hbrBackground = NULL;
    WndClass.lpszMenuName = NULL; 
    WndClass.lpszClassName = "SkypeApiDispatchWindow"; 
    RegisterClass(&WndClass);
	// Do not check the retval of RegisterClass, because on non-unicode
	// win98 it will fail, as it is a stub that returns false() there
	
	// Create main window
	hWnd=CreateWindowEx( WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,
		"SkypeApiDispatchWindow", "", WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, NULL, 0, (HINSTANCE)WndClass.hInstance, 0);

    if (!hWnd) {
		OUTPUT("Cannot create window.");
		TellError(GetLastError());
		closesocket(ListenSocket);
		CloseHandle(WndClass.hInstance);
        return EXIT_FAILURE; 
	}
    ShowWindow(hWnd, 0); 
    UpdateWindow(hWnd); 

	if (!(SkypeReady=CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(ServerThreadBye=CreateEvent(NULL, TRUE, FALSE, NULL))) {
		 OUTPUT("Unable to create Mutex!");
		 closesocket(ListenSocket);
		 CloseHandle(WndClass.hInstance);
	 	 return EXIT_FAILURE;
	}

	if (_beginthread(ConnectToSkypeAPI, 0, (void *)FALSE)==-1 ||
		_beginthread(ServerThread, 0, NULL)==-1) {
		OUTPUT("Cannot create thread. Bye..");
		closesocket(ListenSocket);
		CloseHandle(WndClass.hInstance);
		CloseHandle(SkypeReady);
		return EXIT_FAILURE;
	}

	signal(SIGINT, &bail_out);
	LOG(("Startup: Messagepump started.\nPress CTRL+C to terminate\n"));

	while (GetMessage(&Message, hWnd, 0, 0)) 
    { 
        TranslateMessage(&Message); 
        DispatchMessage(&Message);
    }
	
	LOG(("Shutdown: Messagepump stopped"));

	if (password) free(password);
	if (AcceptSocket != INVALID_SOCKET) closesocket(AcceptSocket);
	closesocket(ListenSocket);
	LOG(("Shutdown: Waiting for serverthread to quit..."));
	if (WaitForSingleObject(ServerThreadBye, 3000)==WAIT_TIMEOUT)
		{OUTPUT("Serverthread didn't terminate correctly, shutting down anyway...");}
	else
		{LOG(("ServerThread terminated"));}
	CloseHandle(WndClass.hInstance);
	CloseHandle(SkypeReady);
	return exitcode;
}