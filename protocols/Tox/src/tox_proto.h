#ifndef _TOX_PROTO_H_
#define _TOX_PROTO_H_

#include "common.h"

#define TOX_SEARCH_BYUID 1001

struct CToxProto : public PROTO<CToxProto>
{
public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CToxProto(const char *protoName, const wchar_t *userName);
	~CToxProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int      __cdecl Authorize(HANDLE hDbEvent);
	virtual	int      __cdecl AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTOFILEEVENT*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT* hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// instances
	static CToxProto* InitProtoInstance(const char* protoName, const wchar_t* userName);
	static int        UninitProtoInstance(CToxProto* ppro);

	static CToxProto* GetContactInstance(MCONTACT hContact);
	static void       UninitInstances();

private:
	Tox *tox;
	mir_cs tox_lock;
	HANDLE connectionThread;
	HANDLE poolingThread;
	bool isTerminated;
	bool isConnected;
	ULONG hMessageProcess;

	// instances
	static LIST<CToxProto> instanceList;
	static int CompareProtos(const CToxProto *p1, const CToxProto *p2);

	// account
	bool IsOnline();

	void DoBootstrap();
	void DoTox();

	void __cdecl ConnectionThread(void*);
	void __cdecl PollingThread(void*);

	//events
	int OnModulesLoaded(WPARAM, LPARAM);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);
	static int __cdecl OnOptionsInit(void *obj, WPARAM wParam, LPARAM lParam);

	static void OnFriendRequest(Tox *tox, const uint8_t *userId, const uint8_t *message, const uint16_t messageSize, void *arg);
	static void OnFriendMessage(Tox *tox, const int friendnumber, const uint8_t *message, const uint16_t messageSize, void *arg);
	static void OnFriendNameChange(Tox *tox, const int friendnumber, const uint8_t *name, const uint16_t nameSize, void *arg);
	static void OnStatusMessageChanged(Tox *tox, const int friendnumber, const uint8_t* message, const uint16_t messageSize, void *arg);
	static void OnUserStatusChanged(Tox *tox, int32_t friendnumber, uint8_t usertatus, void *arg);
	static void OnConnectionStatusChanged(Tox *tox, const int friendId, const uint8_t status, void *arg);
	static void OnAction(Tox *tox, const int friendId, const uint8_t *message, const uint16_t messageSize, void *arg);
	static void OnReadReceipt(Tox *tox, int32_t friendnumber, uint32_t receipt, void *arg);

	// contacts
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);
	bool IsProtoContact(MCONTACT hContact);
	MCONTACT FindContact(const char *clientId);
	MCONTACT AddContact(const char *clientId, bool isHidden = false);

	void LoadContactList();

	void __cdecl SearchByUidAsync(void* arg);

	//services
	
	

	// utils
	TOX_USERSTATUS MirandaToToxStatus(int status);
	int ToxToMirandaStatus(TOX_USERSTATUS userstatus);

	HANDLE AddDbEvent(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob);
	
	std::vector<uint8_t> HexStringToData(std::string hex);
	std::string DataToHexString(std::vector<uint8_t>);

	int LoadToxData(const char *path);
	int SaveToxData(const char *path);

	static void fraddr_to_str(uint8_t *id_bin, char *id_str);
	static void get_id(Tox *m, char *data);

	// dialogs
	static INT_PTR CALLBACK MainOptionsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif //_TOX_PROTO_H_