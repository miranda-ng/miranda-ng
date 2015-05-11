//////////////////////////////////////////////////////////////////////////
// Lan functions

#ifndef __lan_h__
#define __lan_h__

#define MAKE_PORT(x) (HIBYTE(x)|(LOBYTE(x)<<8))

#define MAX_INTERNAL_IP 32
#define PORT_NUMBER MAKE_PORT(34074)

//! Class for operating with LAN
class CLan
{
public:
    //! constructor
	CLan();
	//! destructor
	~CLan();

	//! Helper function - returns status
	int GetStatus() { return m_status; }
	//! Helper function - returns mode
	int GetMode() { return m_mode; }
	//! Getting host addresses count
	int GetHostAddrCount() { return m_hostAddrCount; }
	//! Getting host addresses
	in_addr GetHostAddress(int ind) { return m_hostAddr[ind]; }
	//! Get current host address
	in_addr GetCurHostAddress() { return m_curAddr; }

protected:
	//! Lan status
	enum enumStatus
	{
		LS_OK, //!< no problems
		LS_WRONG_WINSOCK, //!< not found winsock of propper version
		LS_CANT_CREATE_SOCKET, //!< can not create income socket
		LS_CANT_GET_HOSTADDR, //!< can not find host address
		LS_CANT_TURN_ON_BROADCAST, //!< can not allow broadcast messages for socket
		LS_CANT_BIND_SOCKET, //!< can not bind socket to the address
		LS_CANT_START_LISTEN, //!< can not start listen on TCP socket
		LS_CANT_CREATE_THREADS, //!< can not create threads for listen and accept
	};

	//! Lan mode
	enum enumMode
	{
		LM_OFF, //!< Winsock is turned off
		LM_ON, //!< Winsock is on
		LM_LISTEN, //!< Listening for incoming messages
	};

	//! Starts winsock
	void Startup();
	//! Stops winsock
	void Shutdown();
	//! Listen
	void StartListen();
	//! Stop Listen
	void StopListen();

	//! Set current host address
	void SetCurHostAddress(in_addr addr);

	//! Send packet
	void SendPacket(in_addr addr, const u_char* mes, int len);
	//! Send broadcast packet
	void SendPacketBroadcast(const u_char* mes, int len);

	//! Event - called when packet is received
	virtual void OnRecvPacket(u_char* mes, int len, in_addr from) { };
	//! Event - called when new incoming tcp connection is created (new thread is created)
	virtual void OnInTCPConnection(u_long addr, SOCKET m_socket) { };
	//! Event - called when new outgoing tcp connection is created )new thread is created)
	virtual void OnOutTCPConnection(u_long addr, SOCKET m_socket, LPVOID lpParameter) {};
	//! Creates new outgoing TCP connection
	SOCKET CreateTCPConnection(u_long addr, LPVOID lpParameter);

private:
	//! Launches Listen procedure when in new thread
	static DWORD WINAPI ListenProc(LPVOID lpParameter);
	//! Listnes for incoming messages
	void Listen();
	//! Listen thread handle
	HANDLE m_hListenThread;
	//! Structure passed to new TCP connection thread
	struct TTCPConnect
	{
		CLan* m_lan;
		u_long m_addr;
		SOCKET m_socket;
		LPVOID m_lpParameter;
	};
	//! Launches accept procedure for TCP connections in new thread
	static DWORD WINAPI AcceptTCPProc(LPVOID lpParameter);
	//! Accepts TCP connections
	void AcceptTCP();
	//! Accept TCP thread handle
	HANDLE m_hAcceptTCPThread;
	//! Semaphore for killing accept thread
	mir_cs m_csAcceptTCPThread;

	//! Called when new income TCP connection is created
	static DWORD WINAPI OnInTCPConnectionProc(LPVOID lpParameter);
	//! Called when new ougoing TCP connectio is created
	static DWORD WINAPI OnOutTCPConnectionProc(LPVOID lpParameter);

	//! Stores retrieved host addresses
	in_addr m_hostAddr[MAX_INTERNAL_IP];
	//! Current address count
	int m_hostAddrCount;

	//! Stores current host address
	in_addr m_curAddr;
	//! Socket for income messages
	SOCKET m_income;
	//! Socket for income files
	SOCKET m_filesoc;
	//! Current status
	int m_status;
	//! Current mode
	int m_mode;
};

#endif //__lan_h__
