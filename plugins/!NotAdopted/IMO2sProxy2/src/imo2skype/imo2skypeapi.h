typedef void(*IMO2SCB)(char *pszMsg, void *pUser);
struct _tagIMOSAPI;
typedef struct _tagIMOSAPI IMOSAPI;

#define IMO2S_FLAG_CURRTIMESTAMP	1	// Use current timestamp on MSG receive
#define IMO2S_FLAG_ALLOWINTERACT	2	// Allow GUI interaction (CALL)

IMOSAPI *Imo2S_Init(IMO2SCB Callback, void *pUser, int iFlags);
void Imo2S_Exit (IMOSAPI *pInst);
void Imo2S_SetLog (IMOSAPI *pInst, FILE *fpLog);
int Imo2S_Login (IMOSAPI *pInst, char *pszUser, char *pszPass, char **ppszError);
void Imo2S_Logout(IMOSAPI *pInst);
int Imo2S_Send (IMOSAPI *pInst, char *pszMsg);
