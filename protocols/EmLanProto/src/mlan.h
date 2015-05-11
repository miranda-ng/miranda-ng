//////////////////////////////////////////////////////////////////////////
// Miranda lan functions

#ifndef __mlan_h__
#define __mlan_h__

#define MLAN_TIMEOUT 5
#define MLAN_CHECK 50
#define MLAN_SLEEP 400
#define MAX_HOSTNAME_LEN 128

#define FILE_SEND_BLOCK 4096
//#define FILE_SEND_BLOCK 32768
#define FILE_INFO_REFRESH 131072
#define FILE_MIN_BLOCK 1024

class CMLan;

typedef struct
{
	PROTOSEARCHRESULT hdr;
	u_long ipaddr;
	WORD stat;
	u_long ver;
} EMPSEARCHRESULT;

struct TDataHolder
{
public:
	long id;
	long op;

	MCONTACT hContact;
	char* const msg;
	CMLan* lan;

	explicit TDataHolder(const CCSDATA* cc, unsigned long _id, long _op, CMLan* _lan) :
		msg(_strdup((char*)cc->lParam)), hContact(cc->hContact), id(_id), op(_op), lan(_lan)
	{}
	explicit TDataHolder(const char* str, unsigned long _id, long _op, CMLan* _lan) :
		msg(_strdup(str)), hContact(0), id(_id), op(_op), lan(_lan)
	{}
	~TDataHolder(){ delete[] msg; }
};

class CMLan : public CLan
{
public:
	CMLan();
	~CMLan();

	int GetMirandaStatus();
	void SetMirandaStatus(u_int status);
	void SetAllOffline();
	void RecvMessageUrl(CCSDATA* ccs);
	int SendMessageUrl(CCSDATA* ccs, bool isUrl);
	int GetAwayMsg(CCSDATA* ccs);
	int RecvAwayMsg(CCSDATA* ccs);
	int SetAwayMsg(u_int status, char* msg);

	INT_PTR AddToContactList(u_int flags, EMPSEARCHRESULT* psr);
	int Search(const char* name);
	void LoadSettings();
	void SaveSettings();

	char* GetName() { return m_name; }
	bool GetUseHostName() { return m_UseHostName; }
	void SetUseHostName(bool val) { m_UseHostName = val; }
	void SetRequiredIp(u_long ip) { m_RequiredIp = ip; }

	int SendFile(CCSDATA* ccs);
	void RecvFile(CCSDATA* ccs);
	int FileAllow(CCSDATA* ccs);
	int FileDeny(CCSDATA* ccs);
	int FileCancel(CCSDATA* ccs);
	int FileResume(int cid, PROTOFILERESUME* pfr);

protected:
	virtual void OnRecvPacket(u_char* mes, int len, in_addr from);
	virtual void OnInTCPConnection(u_long addr, SOCKET in_socket);
	virtual void OnOutTCPConnection(u_long addr, SOCKET out_socket, LPVOID lpParameter);
private:
	struct TContact
	{
		in_addr m_addr;
		u_int m_status;
		int m_time;
		u_long m_ver;
		char* m_nick;
		TContact* m_prev;
	};
	u_int m_mirStatus;
	TContact* m_pRootContact;
	HANDLE m_hCheckThread;

	char m_name[MAX_HOSTNAME_LEN];
	int m_nameLen;

	mir_cs m_csAccessClass;
	mir_cs m_csReceiveThreadLock;
	mir_cs m_csAccessAwayMes;

	void RequestStatus(bool answer = false, u_long m_addr = INADDR_BROADCAST);
	MCONTACT FindContact(in_addr addr, const char* nick, bool add_to_list, bool make_permanent, bool make_visible, u_int status = ID_STATUS_ONLINE);
	void DeleteCache();

	void StartChecking();
	void StopChecking();
	static void __cdecl CheckProc(void *lpParameter);
	void Check();

	int m_handleId;
	int GetRandomProcId() { return m_handleId++; } // TODO: must create propper CRITICAL SECTION, cause there may be collisions

	static void __cdecl LaunchExt(void *lpParameter);
	void SearchExt(TDataHolder* hold);
	void SendMessageExt(TDataHolder* hold);
	void GetAwayMsgExt(TDataHolder* hold);

	struct TPacket
	{
		u_int idVersion; // -1 means version is not sent
		u_int idStatus; // -1 means status is not sent
		char* strName; // NULL means no name
		bool flReqStatus; // false means no request
		char* strMessage; // NULL means no message
		int idMessage;
		bool flIsUrl; // true if message is an URL
		int idAckMessage; // 0 means no ack
		int idReqAwayMessage; // 0 means no request
		char* strAwayMessage; // NULL means no away message
		int idAckAwayMessage;
	};
	u_char* CreatePacket(TPacket& pak, int* pBufLen = NULL);
	void ParsePacket(TPacket& pak, u_char* buf, int len = 65536);
	void SendPacketExt(TPacket& pak, u_long addr);

	bool m_UseHostName;
	u_long m_RequiredIp;

	HANDLE m_hookIcqMsgReq;
	char* m_amesAway;
	char* m_amesNa;
	char* m_amesOccupied;
	char* m_amesDnd;
	char* m_amesFfc;


	struct TFileConnection
	{
		enum enumFileConnectionStates
		{
			FCS_OK = 0,
			FCS_TERMINATE,
			FCS_ALLOW,

			FCS_OVERWRITE,
			FCS_RESUME,
			FCS_RENAME,
			FCS_SKIP,
		};

		TFileConnection();
		~TFileConnection();
		void Lock() { mir_cslock lck(m_csAccess); }
		void Terminate() { Lock(); m_state = FCS_TERMINATE; }
		int Recv(bool halt = true);
		int Send(u_char* buf, int size);
		int SendRaw(u_char* buf, int size);

		CMLan* m_pLan;
		TFileConnection* m_pPrev;
		TFileConnection* m_pNext;
		int m_state;
		int m_cid;
		SOCKET m_socket;
		u_long m_addr;
		MCONTACT m_hContact;

		char* m_szDescription;
		char** m_szFiles;
		char* m_szDir;
		char* m_szRenamedFile;

		u_char* m_buf;
		int m_recSize;

		mir_cs m_csAccess;
	};
	void FileAddToList(TFileConnection* conn);
	void FileRemoveFromList(TFileConnection* conn);

	mir_cs m_csFileConnectionList;
	TFileConnection* m_pFileConnectionList;
};

#endif //__mlan_h__
