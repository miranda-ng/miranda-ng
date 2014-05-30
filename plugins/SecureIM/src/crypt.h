#ifndef __CRYPT_H__
#define __CRYPT_H__

#define KEYSIZE 256
#define NAMSIZE 512
#define DEFMSGS 4096

#define KEY_A_SIG	0x000000
#define KEY_B_SIG	0x010000

// struct to store all supported protocols
struct SupPro {
	LPSTR name;
	BOOL inspecting;
	int split_on, tsplit_on;
	int split_off, tsplit_off;
};
typedef SupPro *pSupPro;

// struct to store wainting messages
struct waitingMessage {
	LPSTR Message;
	WPARAM wParam;
	waitingMessage *nextMessage;
};
typedef waitingMessage* pWM;

struct partitionMessage {
	int id;
	LPSTR *message; // array of message parts
	partitionMessage *nextMessage;
};
typedef partitionMessage* pPM;

#define HEADER 0xABCD1234
#define FOOTER 0x9876FEDC
#define EMPTYH 0xF1E2D3C4

// memory struct for keys
struct UinKey
{
	MCONTACT hContact;	// handle of contact
	u_int header;		// HEADER
	pSupPro proto;		// proto struct
	BYTE mode,tmode;	// mode: Native,PGP,GPG,RSA/AES,RSA [0..4]
	BYTE status,tstatus;	// status: Disabled,Enabled,AlwaysTry [0..2] for Native mode
	LPSTR msgSplitted;	// message to combine
	pPM msgPart;		// parts of message
	pWM msgQueue;		// last messages not sended or to resend;
	BOOL sendQueue;		// идет отсылка очереди - не обрабатываются сообщения
	BOOL offlineKey;
	char waitForExchange;	// 0 - сбросить очередь
				// 1 - ожидаем
				// 2 - дослать с шифрованием
				// 3 - дослать без шифрования с вопросом
	BOOL decoded;		// false on decode error
	short features;
	HANDLE cntx;		// crypto context
	BYTE keyLoaded;		// ( 1-PGP, 2-GPG ) | 1-RSA
	BYTE gpgMode,tgpgMode;	// 0-UTF8, 1-ANSI
	char *lastFileRecv;
	char *lastFileSend;
	char **fileSend;
	BOOL finFileRecv;
	BOOL finFileSend;
	LPSTR tmp;		// tmp text string
	u_int footer;		// FOOTER
};
typedef UinKey* pUinKey;

struct TFakeAckParams
{
	__forceinline TFakeAckParams(MCONTACT p1, LONG p2, LPCSTR p3) :
		hContact(p1),
		id(p2),
		msg(p3)
		{}

	MCONTACT hContact;
	LONG   id;
	LPCSTR msg;
};

extern char szUIN[NAMSIZE];
extern char szName[NAMSIZE];

extern LIST<SupPro> arProto;
extern LIST<UinKey> arClist;

// crypt_lists.cpp
void loadContactList();
void freeContactList();
pUinKey addContact(MCONTACT hContact);
void    delContact(MCONTACT hContact);
pSupPro getSupPro(MCONTACT);
pUinKey findUinKey(MCONTACT hContact);
pUinKey getUinKey(MCONTACT hContact);
pUinKey getUinCtx(HANDLE);
void addMsg2Queue(pUinKey,WPARAM,LPSTR);

void getContactName(MCONTACT hContact, LPSTR szName);
void getContactNameA(MCONTACT hContact, LPSTR szName);
void getContactUin(MCONTACT hContact, LPSTR szUIN);
void getContactUinA(MCONTACT hContact, LPSTR szUIN);

// crypt_check.cpp
int getContactStatus(MCONTACT);

bool isSecureProtocol(MCONTACT hContact);
BYTE isContactSecured(MCONTACT hContact);
bool isClientMiranda(pUinKey ptr, BOOL emptyMirverAsMiranda=FALSE);
bool isClientMiranda(MCONTACT hContact, BOOL emptyMirverAsMiranda=FALSE);
bool isProtoSmallPackets(MCONTACT);
bool isContactInvisible(MCONTACT);
bool isNotOnList(MCONTACT);
bool isContactNewPG(MCONTACT);
bool isContactPGP(MCONTACT);
bool isContactGPG(MCONTACT);
bool isContactRSAAES(MCONTACT);
bool isContactRSA(MCONTACT);
bool isChatRoom(MCONTACT);
bool isFileExist(LPCSTR);
bool isSecureIM(pUinKey ptr, BOOL emptyMirverAsSecureIM=FALSE);
bool isSecureIM(MCONTACT hContact, BOOL emptyMirverAsSecureIM=FALSE);

// crypt_icons.cpp
HICON mode2icon(int,int);
HANDLE mode2clicon(int mode, int type);
void RefreshContactListIcons(void);
void ShowStatusIcon(MCONTACT, UINT);
void ShowStatusIcon(MCONTACT);
void ShowStatusIconNotify(MCONTACT);

// crypt_popups.cpp
//static int CALLBACK PopupDlgProc(HWND,UINT,WPARAM,LPARAM);
void showPopup(LPCSTR, MCONTACT, HICON, UINT);
void showPopupEC(MCONTACT);
void showPopupDCmsg(MCONTACT, LPCSTR);
void showPopupDC(MCONTACT);
void showPopupKS(MCONTACT);
void showPopupKRmsg(MCONTACT, LPCSTR);
void showPopupKR(MCONTACT);
void showPopupSM(MCONTACT);
void showPopupRM(MCONTACT);

// crypt_meta.cpp
BOOL isProtoMetaContacts(MCONTACT);
BOOL isDefaultSubContact(MCONTACT);
void DeinitMetaContact(MCONTACT);

// crypt_dll.cpp
LPSTR InitKeyA(pUinKey, int);
int InitKeyB(pUinKey, LPCSTR);
void InitKeyX(pUinKey, BYTE*);
BOOL CalculateKeyX(pUinKey, MCONTACT);
LPSTR encodeMsg(pUinKey, LPARAM);
LPSTR decodeMsg(pUinKey, LPARAM, LPSTR);
BOOL LoadKeyPGP(pUinKey);
BOOL LoadKeyGPG(pUinKey);

// crypt_misc.cpp
void waitForExchange(pUinKey ptr, int flag = 1);

#endif
