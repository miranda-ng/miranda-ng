// Skype API defines
#define SKYPECONTROLAPI_ATTACH_SUCCESS 0
#define SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION 1
#define SKYPECONTROLAPI_ATTACH_REFUSED 2
#define SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE 3
#define SKYPECONTROLAPI_ATTACH_API_AVAILABLE 0x8001

#define MISC_ERROR 1
#define USER_NOT_FOUND 2
#define USER_NOT_ONLINE 3
#define USER_BLOCKED 4
#define TYPE_UNSUPPORTED 5
#define SENDER_NOT_FRIEND 6
#define SENDER_NOT_AUTHORIZED 7

#define MAX_ENTRIES 128		// Max. 128 number-Entries in Dial-dlg.


// Prototypes
int SkypeMsgInit(void);
int SkypeMsgAdd(char *msg);
void SkypeMsgCleanup(void);
char *SkypeMsgGet(void);
int SkypeSend(char*, ...);
char *SkypeRcv(char *what, DWORD maxwait);
char *SkypeRcvTime(char *what, time_t st, DWORD maxwait);
char *SkypeRcvMsg(char *what, time_t st, MCONTACT hContact, DWORD maxwait);
INT_PTR SkypeCall(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeCallHangup(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeOutCall(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeHup(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeHoldCall(WPARAM wParam, LPARAM lParam);
void SkypeFlush(void);
int SkypeStatusToMiranda(char *s);
char *MirandaStatusToSkype(int id);
char *GetSkypeErrorMsg(char *str);
BOOL testfor(char *what, DWORD maxwait);
int ConnectToSkypeAPI(char *path, BOOL  bStart);
int CloseSkypeAPI(char *skypePath);
INT_PTR SkypeAdduserDlg(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeAnswerCall(WPARAM wParam, LPARAM lParam);
int SkypeMsgCollectGarbage(time_t age);
INT_PTR SkypeSendGuiFile(WPARAM wParam, LPARAM);
INT_PTR SkypeBlockContact(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSetAvatar(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSetAwayMessage(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSetNick(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeChatCreate(WPARAM wParam, LPARAM lParam);
int SkypeSetProfile(char *szProperty, char *szValue);
char *SkypeGet(char *szWhat, char *szWho, char *szProperty);
char *SkypeGetID(char *szWhat, char *szWho, char *szProperty);
char *SkypeGetErr(char *szWhat, char *szWho, char *szProperty);
char *SkypeGetErrID(char *szWhat, char *szWho, char *szProperty);
WCHAR *SkypeGetW(char *szWhat, WCHAR *szWho, char *szProperty);
WCHAR *SkypeGetErrW(char *szWhat, TCHAR *szWho, char *szProperty);
#define SkypeGetT SkypeGetW
#define SkypeGetErrT SkypeGetErrW
char *SkypeGetProfile(char *szProperty);
void SetUserNamePassword();
INT_PTR SkypeAdduserDlg(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeReceivedAPIMessage(WPARAM wParam, LPARAM lParam);
