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
	HCONTACT hContact;	// handle of contact
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
	__forceinline TFakeAckParams(HCONTACT p1, LONG p2, LPCSTR p3) :
		hContact(p1),
		id(p2),
		msg(p3)
		{}

	HCONTACT hContact;
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
pUinKey addContact(HCONTACT hContact);
void    delContact(HCONTACT hContact);
pSupPro getSupPro(HCONTACT);
pUinKey findUinKey(HCONTACT hContact);
pUinKey getUinKey(HCONTACT hContact);
pUinKey getUinCtx(HANDLE);
void addMsg2Queue(pUinKey,WPARAM,LPSTR);

void getContactName(HCONTACT hContact, LPSTR szName);
void getContactNameA(HCONTACT hContact, LPSTR szName);
void getContactUin(HCONTACT hContact, LPSTR szUIN);
void getContactUinA(HCONTACT hContact, LPSTR szUIN);

// crypt_check.cpp
int getContactStatus(HCONTACT);

bool isSecureProtocol(HCONTACT hContact);
BYTE isContactSecured(HCONTACT hContact);
bool isClientMiranda(pUinKey ptr, BOOL emptyMirverAsMiranda=FALSE);
bool isClientMiranda(HCONTACT hContact, BOOL emptyMirverAsMiranda=FALSE);
bool isProtoSmallPackets(HCONTACT);
bool isContactInvisible(HCONTACT);
bool isNotOnList(HCONTACT);
bool isContactNewPG(HCONTACT);
bool isContactPGP(HCONTACT);
bool isContactGPG(HCONTACT);
bool isContactRSAAES(HCONTACT);
bool isContactRSA(HCONTACT);
bool isChatRoom(HCONTACT);
bool isFileExist(LPCSTR);
bool isSecureIM(pUinKey ptr, BOOL emptyMirverAsSecureIM=FALSE);
bool isSecureIM(HCONTACT hContact, BOOL emptyMirverAsSecureIM=FALSE);

// crypt_icons.cpp
HICON mode2icon(int,int);
HANDLE mode2clicon(int mode, int type);
void RefreshContactListIcons(void);
void ShowStatusIcon(HCONTACT, UINT);
void ShowStatusIcon(HCONTACT);
void ShowStatusIconNotify(HCONTACT);

// crypt_popups.cpp
//static int CALLBACK PopupDlgProc(HWND,UINT,WPARAM,LPARAM);
void showPopup(LPCSTR, HCONTACT, HICON, UINT);
void showPopupEC(HCONTACT);
void showPopupDCmsg(HCONTACT, LPCSTR);
void showPopupDC(HCONTACT);
void showPopupKS(HCONTACT);
void showPopupKRmsg(HCONTACT, LPCSTR);
void showPopupKR(HCONTACT);
void showPopupSM(HCONTACT);
void showPopupRM(HCONTACT);

// crypt_meta.cpp
BOOL isProtoMetaContacts(HCONTACT);
BOOL isDefaultSubContact(HCONTACT);
HCONTACT getMetaContact(HCONTACT);
HCONTACT getMostOnline(HCONTACT);
void DeinitMetaContact(HCONTACT);

// crypt_dll.cpp
LPSTR InitKeyA(pUinKey, int);
int InitKeyB(pUinKey, LPCSTR);
void InitKeyX(pUinKey, BYTE*);
BOOL CalculateKeyX(pUinKey, HCONTACT);
LPSTR encodeMsg(pUinKey, LPARAM);
LPSTR decodeMsg(pUinKey, LPARAM, LPSTR);
BOOL LoadKeyPGP(pUinKey);
BOOL LoadKeyGPG(pUinKey);

// crypt_misc.cpp
void waitForExchange(pUinKey ptr, int flag = 1);

#endif
