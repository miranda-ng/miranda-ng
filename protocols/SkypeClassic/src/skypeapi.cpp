/*
 * SkypeAPI - All more or less important functions that deal with Skype
 */

#include "skype.h"
#include "skypeapi.h"
#include "utf8.h"
#include "debug.h"
#include "contacts.h"
#include "skypeproxy.h"
#include "pthread.h"
#include "gchat.h"
#include "alogon.h"
#include "msgq.h"
#include <malloc.h>
#pragma warning (push)
#pragma warning (disable: 4100) // unreferenced formal parameter
#include <m_utils.h>
#include <m_langpack.h>
#pragma warning (push)
#include <m_toptoolbar.h>

#pragma warning (disable: 4706) // assignment within conditional expression

// Imported Globals
extern HWND hSkypeWnd, g_hWnd;
extern BOOL SkypeInitialized, UseSockets, MirandaShuttingDown, bIsImoproxy;
extern int SkypeStatus, receivers;
extern HANDLE SkypeReady, SkypeMsgReceived, httbButton;
extern UINT ControlAPIAttach, ControlAPIDiscover;
extern LONG AttachStatus;
extern HINSTANCE hInst;
extern PLUGININFOEX pluginInfo;
extern HANDLE hProtocolAvatarsFolder, hHookSkypeApiRcv;
extern char DefaultAvatarsFolder[MAX_PATH+1], *pszProxyCallout, protocol, g_szProtoName[];

// -> Skype Message Queue functions //

static TYP_MSGQ SkypeMsgs, SkypeSendQueue;

status_map status_codes[] = {
	{ID_STATUS_AWAY, "AWAY"},
	{ID_STATUS_NA, "NA"},
	{ID_STATUS_DND, "DND"},
	{ID_STATUS_ONLINE, "ONLINE"},
	{ID_STATUS_FREECHAT, "SKYPEME"},	// Unfortunately Skype API tells us userstatus ONLINE, if we are free for chat
	{ID_STATUS_OFFLINE, "OFFLINE"},
	{ID_STATUS_INVISIBLE, "INVISIBLE"},
	{ID_STATUS_CONNECTING, "CONNECTING"},
	{0, NULL}
};

//status_map 


static CRITICAL_SECTION ConnectMutex;
static BOOL rcvThreadRunning=FALSE, isConnecting = FALSE;
static SOCKET ClientSocket=INVALID_SOCKET;
static HANDLE SkypeMsgToSend=NULL;

static char *m_szSendBuf = NULL;
static DWORD m_iBufSize = 0;


static int _ConnectToSkypeAPI(char *path, BOOL bStart);


/* SkypeReceivedMessage
 * 
 * Purpose: Hook to be called when a message is received, if some caller is 
 *          using our internal I/O services.
 * Params : wParam - Not used
 *          lParam - COPYDATASTRUCT like in WM_COPYDATA
 * Returns: Result from SendMessage
 */
INT_PTR SkypeReceivedAPIMessage(WPARAM wParam, LPARAM lParam) {
	return SendMessage(g_hWnd, WM_COPYDATALOCAL, (WPARAM)hSkypeWnd, lParam);
}

/*
 * Skype via Socket --> Skype2Socket connection
 */

void rcvThread(char *dummy) {
	unsigned int length;
	char *buf;
	COPYDATASTRUCT CopyData;
	int rcv;

	if (!UseSockets) return;
	rcvThreadRunning=TRUE;
	for ( ;; ) {
		if (ClientSocket==INVALID_SOCKET) {
			rcvThreadRunning=FALSE;
			return;
		}
		LOG(("rcvThread Receiving from socket.."));
		if ((rcv=recv(ClientSocket, (char *)&length, sizeof(length), 0))==SOCKET_ERROR || rcv==0) {
			rcvThreadRunning=FALSE;
			if (rcv==SOCKET_ERROR) {LOG(("rcvThread Socket error"));}
			else {LOG(("rcvThread lost connection, graceful shutdown"));}
			return;
		}
		LOG(("rcvThread Received length, recieving message.."));
		buf=(char *)calloc(1, length+1);
		if ((rcv = recv(ClientSocket, buf, length, 0))==SOCKET_ERROR || rcv==0) {
			rcvThreadRunning=FALSE;
			if (rcv==SOCKET_ERROR) {LOG(("rcvThread Socket error"));}
			else {LOG(("rcvThread lost connection, graceful shutdown"));}
			free(buf);
			return;
		}
		LOG(("Received message: %s", buf));

		CopyData.dwData=0; 
		CopyData.lpData=buf; 
		CopyData.cbData=(DWORD)strlen(buf)+1;
		if (!SendMessage(g_hWnd, WM_COPYDATALOCAL, (WPARAM)hSkypeWnd, (LPARAM)&CopyData))
		{
			LOG(("SendMessage failed: %08X", GetLastError()));
		}
		free(buf);
	}
}

void sendThread(char *dummy) {
	COPYDATASTRUCT CopyData;
	LRESULT SendResult;
	int oldstatus;
	unsigned int length;
	char *szMsg;
	
	while (SkypeMsgToSend) {
		if (WaitForSingleObject(SkypeMsgToSend, INFINITE) != WAIT_OBJECT_0) return;
		if (!(szMsg = MsgQ_Get(&SkypeSendQueue))) continue;
		length=(unsigned int)strlen(szMsg);

		if (UseSockets) {
			if (send(ClientSocket, (char *)&length, sizeof(length), 0) != SOCKET_ERROR &&
				send(ClientSocket, szMsg, length, 0) != SOCKET_ERROR) {
				free (szMsg);
				continue;
			}
			SendResult = 0;
		} else {
			CopyData.dwData=0; 
			CopyData.lpData=szMsg; 
			CopyData.cbData=length+1;

			// Internal comm channel
			if (pszProxyCallout) {
			   CallService (pszProxyCallout, 0, (LPARAM)&CopyData);
			   free(szMsg);
			   continue;
			}

			// If this didn't work, proceed with normal Skype API
			if (!hSkypeWnd) 
			{
			   LOG(("SkypeSend: DAMN! No Skype window handle! :("));
			}
			SendResult=SendMessage(hSkypeWnd, WM_COPYDATA, (WPARAM)g_hWnd, (LPARAM)&CopyData);
			LOG(("SkypeSend: SendMessage returned %d", SendResult));
			free(szMsg);
		}
		if (!SendResult) {
			SkypeInitialized=FALSE;
			AttachStatus=-1;
			ResetEvent(SkypeReady);
			if (g_hWnd) KillTimer (g_hWnd, 1);
			if (SkypeStatus!=ID_STATUS_OFFLINE) {
				// Go offline
				logoff_contacts(FALSE);
				oldstatus=SkypeStatus;
				InterlockedExchange((long *)&SkypeStatus, (int)ID_STATUS_OFFLINE);
				ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
			}
			// Reconnect to Skype
			ResetEvent(SkypeReady);
			pthread_create(LaunchSkypeAndSetStatusThread, (void *)ID_STATUS_ONLINE);
			WaitForSingleObject (SkypeReady, 10000);
			//	  SendMessageTimeout(HWND_BROADCAST, ControlAPIDiscover, (WPARAM)g_hWnd, 0, SMTO_ABORTIFHUNG, 3000, NULL);
		}
	}
}


/*
 * Skype Messagequeue - Implemented as a linked list 
 */

/* SkypeMsgInit
 * 
 * Purpose: Initializes the Skype Message queue and API
 * Returns: 0 - Success
 *         -1 - Memory allocation failure
 */
int SkypeMsgInit(void) {

	MsgQ_Init(&SkypeMsgs);
	MsgQ_Init(&SkypeSendQueue);
    InitializeCriticalSection(&ConnectMutex);
	if (SkypeMsgToSend=CreateSemaphore(NULL, 0, MAX_MSGS, NULL)) {
		if (m_szSendBuf = (char*)malloc(m_iBufSize=512)) {
			if (_beginthread(( pThreadFunc )sendThread, 0, NULL)!=-1)
				return 0;
			free(m_szSendBuf);
		}
		CloseHandle (SkypeMsgToSend);
	}
	return -1;
}

/* SkypeMsgAdd
 * 
 * Purpose: Add Message to linked list
 * Params : msg - Message to add to queue
 * Returns: 0 - Success
 *         -1 - Memory allocation failure
 */
int SkypeMsgAdd(char *msg) {
	return MsgQ_Add(&SkypeMsgs, msg)?0:-1;
}

/* SkypeMsgCleanup
 * 
 * Purpose: Clean up the whole MESSagequeue - free() all
 */
void SkypeMsgCleanup(void) {
	int i;

	LOG(("SkypeMsgCleanup Cleaning up message queue.."));
	if (receivers>1)
	{
		LOG (("SkypeMsgCleanup Releasing %d receivers", receivers));
		for (i=0;i<receivers; i++)
		{
			SkypeMsgAdd ("ERROR Semaphore was blocked");
		}
		ReleaseSemaphore (SkypeMsgReceived, receivers, NULL);	
	}

	EnterCriticalSection(&ConnectMutex);
	MsgQ_Exit(&SkypeMsgs);
	LeaveCriticalSection(&ConnectMutex);
	DeleteCriticalSection(&ConnectMutex);
	CloseHandle(SkypeMsgToSend);
	SkypeMsgToSend=NULL;
	MsgQ_Exit(&SkypeSendQueue);
	if (m_szSendBuf)
	{
		free (m_szSendBuf);
		m_szSendBuf = NULL;
		m_iBufSize = 0;
	}
	LOG(("SkypeMsgCleanup Done."));
}

/* SkypeMsgGet
 * 
 * Purpose: Fetch next message from message queue
 * Returns: The next message
 * Warning: Don't forget to free() return value!
 */
char *SkypeMsgGet(void) {
	return MsgQ_Get(&SkypeMsgs);
}

// Message sending routine, for internal use by SkypeSend
static int __sendMsg(char *szMsg) {
   COPYDATASTRUCT CopyData;

   LOG(("> %s", szMsg));

   // Fake PING-PONG, as PING-PONG is not supported by Skype2Socket
   if ((UseSockets || bIsImoproxy) && !strcmp(szMsg, "PING")) {
	 CopyData.dwData=0; 
	 CopyData.lpData="PONG"; 
	 CopyData.cbData=5;
	 SendMessage(g_hWnd, WM_COPYDATALOCAL, (WPARAM)hSkypeWnd, (LPARAM)&CopyData);
	 return 0;
   }

   if (UseSockets && ClientSocket==INVALID_SOCKET) return -1;
   if (!MsgQ_Add(&SkypeSendQueue, szMsg) || !ReleaseSemaphore(SkypeMsgToSend, 1, NULL))
	   return -1;
   return 0;
}

/* SkypeSend
 * 
 * Purpose: Sends the specified message to the Skype API.
 *		    If it fails, try to reconnect zu the Skype API
 * Params:  use like sprintf without first param (dest. buffer)
 * Returns: 0 - Success
 *		   -1 - Failure
 */
int SkypeSend(char *szFmt, ...) {
	char *pNewBuf;
	va_list ap;
	size_t iLen;
   
   // 0.0.0.17+  - Build message-String from supplied parameter list
   // so the user doesn't have to care about memory allocation any more.
   // 0.0.0.47+  - No more restrictions apply to the format string.
   // The temporary buffer remains allocated during the session and gets
   // dynamically expanded when needed. This makes sense, as this function
   // is used very often and therefore it is faster to not allocate
   // memory on every send.
	if (!m_szSendBuf && !(m_szSendBuf=(char*)malloc(m_iBufSize=512))) return -1;
	do
	{
		va_start(ap, szFmt);
		iLen = _vsnprintf(m_szSendBuf, m_iBufSize, szFmt, ap); 
		va_end(ap);
		if (iLen == -1)
		{
		  if (!(pNewBuf = (char*)realloc (m_szSendBuf, m_iBufSize*2)))
		  {
			  iLen = strlen (m_szSendBuf);
			  break;
		  }
		  m_szSendBuf = pNewBuf;
		  m_iBufSize*=2;
		}
	} while (iLen == -1);

   return __sendMsg(m_szSendBuf);
}

/* SkypeRcvTime
 * 
 * Purpose: Wait, until either the message "what" is received or maxwait-Time has passed
 *		    or there was an error and return it
 * Params : what	- Wait for this string-part at the beginning of a received string
 *					  If the first character of the string is NULL, the rest after the NULL
 *					  character will be searched in the entire received message-string.
 *                    You can tokenize the string by using NULL characters.
 *                    You HAVE TO end the string with a extra \0, otherwise the tokenizer
 *                    will run amok in memory!
 *			st		- The message timestamp must be newer or equal to st.
 *					  Set to 0, if you do not need this and want the first message of this 
 *					  kind in the queue.
 *		    maxwait - Wait this time before returning, if nothing was received,
 *					  can be INFINITE
 * Returns: The received message containing "what" or a ERROR-Message or NULL if 
 *			time is up and nothing was received
 * Warning: Don't forget to free() return value!
 */
char *SkypeRcvTime(char *what, time_t st, DWORD maxwait) {
    char *msg, *token=NULL;
	struct MsgQueue *ptr;
	int j;
	DWORD dwWaitStat;
	BOOL bChatMsg = FALSE, bIsChatMsg = FALSE;

	LOG (("SkypeRcv - Requesting answer: %s", what));
	if (what) bChatMsg = strncmp(what, "CHATMESSAGE", 11)==0;
	do {
		EnterCriticalSection(&SkypeMsgs.cs);
		// First, search for the requested message. On second run, also accept an ERROR
		for (j=0; j<2; j++)
		{
			for (ptr=SkypeMsgs.l.tqh_first; ptr; ptr=ptr->l.tqe_next) {
				if (what && what[0]==0) {
					// Tokenizer syntax active
					token=what+1;
					while (*token) {
						if (!strstr (ptr->message, token)) {
							token=NULL;
							break;
						}
						token+=strlen(token)+1;
					}
				}

				//if (j==1) {LOG(("SkypeRcv compare %s (%lu) -- %s (%lu)", ptr->message, ptr->tReceived, what, st));}
				if ((st == 0 || ptr->tReceived >= st) &&
					(what==NULL || token || (what[0] && !strncmp(ptr->message, what, strlen(what))) || 
					(bIsChatMsg = (j==1 && bChatMsg && !strncmp(ptr->message, what+4, strlen(what+4)))) || 
					(j==1 && !strncmp(ptr->message, "ERROR", 5)))) 
				{
					msg=MsgQ_RemoveMsg(&SkypeMsgs, ptr);
					LOG(("<SkypeRcv: %s", msg));
					if (bIsChatMsg) {
						msg=(char*)realloc(msg, strlen(msg)+5);
						memmove (msg+4, msg, strlen(msg)+1);
						memcpy (msg, "CHAT", 4);

						// This may be a sign that protocol negotiation failed, so we can try to send
						// our supported protocol version again, just in case... (Skype API bug?)
						//SkypeSend(SKYPE_PROTO); 
					}
					LeaveCriticalSection(&SkypeMsgs.cs);
					return msg;
				}
			}
		}
		LeaveCriticalSection(&SkypeMsgs.cs);
		InterlockedIncrement ((long *)&receivers); //receivers++;
		dwWaitStat = WaitForSingleObject(SkypeMsgReceived, maxwait);
		if (receivers>1) InterlockedDecrement ((long *)&receivers); //  receivers--;
		if (receivers>1) {LOG (("SkypeRcv: %d receivers still waiting", receivers));}
		
	} while(dwWaitStat == WAIT_OBJECT_0 && !MirandaShuttingDown);	
	InterlockedDecrement ((long *)&receivers);
	LOG(("<SkypeRcv: (empty)"));	
	return NULL;
}

char *SkypeRcv(char *what, DWORD maxwait)
{
	return SkypeRcvTime(what, 0, maxwait);
}

char *SkypeRcvMsg(char *what, time_t st, MCONTACT hContact, DWORD maxwait)
{
	char *msg, msgid[32]={0}, *pMsg, *pCurMsg;
	struct MsgQueue *ptr;
	int iLenWhat = (int)strlen(what);
	DWORD dwWaitStat;
	BOOL bIsError, bProcess;

	LOG (("SkypeRcvMsg - Requesting answer: %s ", what));
	do {
		EnterCriticalSection(&SkypeMsgs.cs);
		ptr=SkypeMsgs.l.tqh_first;
		while(ptr) {
			//LOG (("SkypeRcvMsg - msg: %s -- %s", ptr->message, what));
			pCurMsg = ptr->message;
			bIsError = FALSE;
			if (*what && !strncmp(pCurMsg, what, iLenWhat)) {
				// Now we received a MESSAGE with an identifier. So this one is definitely for us
				// However the status can be SENDING instead of SENT and next message with this number 
				// isn't using the ID anymore, so we have to save the ID as new identifier for message recognition
				pCurMsg+=iLenWhat;
				if ((pMsg = strchr (pCurMsg, ' ')) && (pMsg=strchr (pMsg+1, ' ')))
					strncpy (msgid, pCurMsg, pMsg-pCurMsg);
				else if (strncmp (pCurMsg, "ERROR", 5) == 0) bIsError = TRUE;
			}

			if ((*msgid && strncmp (pCurMsg, msgid, strlen(msgid)) == 0) ||
				(!*what && ptr->tReceived >= st  && 
				 (strncmp(pCurMsg, "MESSAGE", 7) == 0 || strncmp(pCurMsg, "CHATMESSAGE", 11) == 0 )
				) || bIsError || 
				(ptr->tReceived >= st  && ptr->tReceived <=st+1 && 
				  (bIsError=(strncmp(pCurMsg, "ERROR 26", 8)==0 || strncmp(pCurMsg, "ERROR 43", 8)==0))
				  ) )
			{
				bProcess = bIsError;
				if (!bIsError) {
					if ((pMsg = strchr (pCurMsg, ' ')) && (pMsg=strchr (pMsg+1, ' '))) {
						pMsg++;
						if (strncmp (pMsg, "STATUS ", 7) == 0) {
							pMsg+=7;
							if (strcmp (pMsg, "SENDING") == 0) {
								// Remove dat shit
								struct MsgQueue *ptr_=ptr->l.tqe_next;

								free(MsgQ_RemoveMsg(&SkypeMsgs, ptr));
								ptr=ptr_;
								continue;
							}
							bProcess = (strcmp (pMsg, "SENT") == 0 || strcmp (pMsg, "QUEUED") == 0 ||
								strcmp (pMsg, "FAILED") == 0 || strcmp (pMsg, "IGNORED") == 0 ||
								strcmp (pMsg, "SENDING") == 0);
						}
					}
				}
				if (bProcess) {
					msg=MsgQ_RemoveMsg(&SkypeMsgs, ptr);
					LOG(("<SkypeRcv: %s", msg));
					LeaveCriticalSection(&SkypeMsgs.cs);
					return msg;
				}
			}
			ptr=ptr->l.tqe_next;
		}
		LeaveCriticalSection(&SkypeMsgs.cs);
		InterlockedIncrement ((long *)&receivers); //receivers++;
		dwWaitStat = WaitForSingleObject(SkypeMsgReceived, maxwait);
		if (receivers>1) InterlockedDecrement ((long *)&receivers); //  receivers--;
		if (receivers>1) {LOG (("SkypeRcvMsg: %d receivers still waiting", receivers));}
		
	} while(dwWaitStat == WAIT_OBJECT_0 && !MirandaShuttingDown);	
	InterlockedDecrement ((long *)&receivers);
	LOG(("<SkypeRcvMsg: (empty)"));	
	return NULL;
}

/*
  Introduced in 0.0.0.17
  
  Issues a GET szWhat szWho szProperty and waits until the answer is received
  Returns the answer or NULL on failure
  BEWARE: Don't forget to free() return value!

  For example:  SkypeGet("USER", dbv.pszVal, "FULLNAME");
*/ 
static char *__SkypeGet(char *szID, char *szWhat, char *szWho, char *szProperty) {
  char *str, *ptr;
  size_t len, len_id;
  time_t st = 0;

  st = *szID?0:SkypeTime(NULL);
  str=(char *)_alloca((len=strlen(szWhat)+strlen(szWho)+strlen(szProperty)+(*szWho?2:1)+(len_id=strlen(szID)))+5);
  sprintf(str, "%sGET %s%s%s %s", szID, szWhat, *szWho?" ":"", szWho, szProperty);
  if (__sendMsg(str)) return NULL;
  if (*szProperty) len++;
  if (*szID) {
	sprintf(str, "%s%s%s%s %s", szID, szWhat, *szWho?" ":"", szWho, szProperty);
	ptr = SkypeRcvTime(str, st, INFINITE);
  } else ptr = SkypeRcvTime(str+4, st, INFINITE);
  if (ptr && strncmp (ptr+len_id, "ERROR", 5)) memmove(ptr, ptr+len, strlen(ptr)-len+1);
  LOG(("SkypeGet - Request %s -> Answer %s", str, ptr));
  return ptr;
}

char *SkypeGetID(char *szWhat, char *szWho, char *szProperty) {
	char szID[16]={0};
	static DWORD dwId = 0;

	if (protocol>=4 || bIsImoproxy) sprintf (szID, "#G%d ", dwId++);
	return __SkypeGet (szID, szWhat, szWho, szProperty);
}

char *SkypeGet(char *szWhat, char *szWho, char *szProperty) {
	return __SkypeGet ("", szWhat, szWho, szProperty);
}

#ifdef _UNICODE
WCHAR *SkypeGetW(char *szWhat, WCHAR *szWho, char *szProperty) {
	char *ptszWho = (char*)make_utf8_string(szWho);
	char *pRet = SkypeGet (szWhat, ptszWho, szProperty);
	free (ptszWho);
	if (pRet) {
		WCHAR *ptr = make_unicode_string((const unsigned char*)pRet);
		free (pRet);
		return ptr;
	}
	return NULL;
}
#endif

char *SkypeGetErr(char *szWhat, char *szWho, char *szProperty) {
	char *ret = SkypeGet(szWhat, szWho, szProperty);
	if (ret && !strncmp(ret, "ERROR", 5)) {
		free (ret);
		return NULL;
	}
	return ret;
}

#ifdef _UNICODE
WCHAR *SkypeGetErrW(char *szWhat, TCHAR *szWho, char *szProperty) {
	WCHAR *ret = SkypeGetW(szWhat, szWho, szProperty);
	if (ret && !_tcsncmp(ret, _T("ERROR"), 5)) {
		free (ret);
		return NULL;
	}
	return ret;
}
#endif


/* SkypeGetProfile
 *
 * Issues a SET PROFILE szProperty szValue and waits until the answer is received
 * Returns the answer or NULL on failure
 * BEWARE: Don't forget to free() return value!
 *
 * For example:  SkypeGetProfile("FULLNAME", "Tweety");
*/ 
char *SkypeGetProfile(char *szProperty) {
  return SkypeGet ("PROFILE", "", szProperty);
}

/* SkypeSetProfile
 *
 * 
*/ 
int SkypeSetProfile(char *szProperty, char *szValue) {
  return SkypeSend("SET PROFILE %s %s", szProperty, szValue);
}

/* SkypeMsgCollectGarbage
 * 
 * Purpose: Runs the garbage collector on the Skype Message-Queue to throw out old 
 *		    messages which may unnecessarily eat up memory.
 * Params : age     - Time in seconds. Messages older than this value will be 
 *					  thrown out.
 * Returns: 0  - No messages were thrown out
 *			>0 - n messages were thrown out
 */
int SkypeMsgCollectGarbage(time_t age) {
	return MsgQ_CollectGarbage(&SkypeMsgs, age);
}


/* SkypeCall
 * 
 * Purpose: Give a Skype call to the given User in wParam
 *          or hangs up existing call 
 *          (hangUp is moved over to SkypeCallHangup)
 * Params : wParam - Handle to the User to be called
 *			lParam - Can be NULL
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeCall(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	char *msg=0;
	int res;

	if (!db_get_s((MCONTACT)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
		res = -1; // no direct return, because dbv needs to be freed
	} else {
		if (db_get_s((MCONTACT)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return -1;
		msg=(char *)malloc(strlen(dbv.pszVal)+6);
		strcpy(msg, "CALL ");
		strcat(msg, dbv.pszVal);
		res=SkypeSend(msg);
	}
	db_free(&dbv);
	free(msg);
	return res;
}

/* SkypeCallHangup
 *
 * Prupose: Hangs up the existing call to the given User 
 *          in wParam.
 *
 * Params : wParam - Handle to the User to be called
 *          lParam - Can be NULL
 *
 * Returns: 0 - Success
 *          -1 - Failure
 *
 */
INT_PTR SkypeCallHangup(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *msg=0;
	int res = -1;

	if (!db_get_s((MCONTACT)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
		msg=(char *)malloc(strlen(dbv.pszVal)+21);
		sprintf(msg, "SET %s STATUS FINISHED", dbv.pszVal);
		//sprintf(msg, "ALTER CALL %s HANGUP", dbv.pszVal);
		res=SkypeSend(msg);
#if _DEBUG
		db_unset((MCONTACT)wParam, SKYPE_PROTONAME, "CallId");
#endif
	//} else {
	//	if (db_get((HANDLE)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return -1;
	//	msg=(char *)malloc(strlen(dbv.pszVal)+6);
	//	strcpy(msg, "CALL ");
	//	strcat(msg, dbv.pszVal);
	//	res=SkypeSend(msg);
	}
	db_free(&dbv);
	free(msg);
	return res;
}

/* FixNumber
 * 
 * Purpose: Eliminates all non-numeric chars from the given phonenumber
 * Params : p	- Pointer to the buffer with the number
 */
static void FixNumber(char *p) {
	unsigned int i;

	for (i=0;i<=strlen(p);i++)
		if ((p[i]<'0' || p[i]>'9')) 
			if (p[i]) {
				memmove(p+i, p+i+1, strlen(p+i));
				i--;
			} else break;
}


/* DialDlgProc
 * 
 * Purpose: Dialog procedure for the Dial-Dialog
 */
static INT_PTR CALLBACK DialDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static MCONTACT hContact;
	static unsigned int entries=0;
	BOOL TempAdded=FALSE;
	char number[64], *msg, *ptr=NULL;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			hContact=(MCONTACT)lParam;
			Utils_RestoreWindowPosition(hwndDlg, NULL, SKYPE_PROTONAME, "DIALdlg");
			TranslateDialogDefault(hwndDlg);

			if (lParam) {
				DBVARIANT dbv;
				BOOL bDialNow=TRUE;

				if (!db_get(hContact,"UserInfo","MyPhone1",&dbv)) {
					int j;
					char idstr[16];

					// Multiple phone numbers, select one
					bDialNow=FALSE;
					db_free(&dbv);
					for(j=0;;j++) {
						sprintf(idstr,"MyPhone%d",j);
						if(db_get_s(hContact,"UserInfo",idstr,&dbv)) break;
						FixNumber(dbv.pszVal+1); // Leave + alone
						SendDlgItemMessage(hwndDlg,IDC_NUMBER,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
						db_free(&dbv);
					}
				}
				if (db_get_s(hContact,SKYPE_PROTONAME,"SkypeOutNr",&dbv)) {
					db_get_s(hContact,"UserInfo","MyPhone0",&dbv);
					FixNumber(dbv.pszVal+1);
				}
				SetDlgItemTextA(hwndDlg, IDC_NUMBER, dbv.pszVal);
				db_free(&dbv);
				if (bDialNow) PostMessage(hwndDlg, WM_COMMAND, IDDIAL, 0);
			} else {
				DBVARIANT dbv;
				char number[64];

				for (entries=0;entries<MAX_ENTRIES;entries++) {
					sprintf(number, "LastNumber%d", entries);
					if (!db_get_ts(NULL, SKYPE_PROTONAME, number, &dbv)) {
						SendDlgItemMessage(hwndDlg,IDC_NUMBER,CB_ADDSTRING,0,(LPARAM)dbv.ptszVal);
						db_free(&dbv);
					} else break	;
				}
			}
			SetFocus(GetDlgItem(hwndDlg, IDC_NUMBER));
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDDIAL:
					EnableWindow(GetDlgItem(hwndDlg, IDDIAL), FALSE);
					GetDlgItemTextA(hwndDlg, IDC_NUMBER, number, sizeof(number));
					if (!strncmp(number, "00", 2)) {					
						memmove(number, number+1, sizeof(number)-1);
						number[0]='+';
						number[sizeof(number)]=0;
					}
					if (!hContact) {
						if (!(hContact=add_contact(number, PALF_TEMPORARY))) {
							DestroyWindow(hwndDlg);
							break;
						}
						db_unset(hContact, "CList", "Hidden");
						db_set_w(hContact, SKYPE_PROTONAME, "Status", (WORD)SkypeStatusToMiranda("SKYPEOUT"));
						if (SendDlgItemMessage(hwndDlg,IDC_NUMBER,CB_FINDSTRING,0,(LPARAM)number)==CB_ERR) {
							int i;
							char buf[64];
							DBVARIANT dbv;

							if (entries>MAX_ENTRIES) entries=MAX_ENTRIES;
							for (i=entries;i>0;i--) {
								sprintf(buf, "LastNumber%d", i-1);
								if (!db_get_s(NULL, SKYPE_PROTONAME, buf, &dbv)) {
									sprintf(buf, "LastNumber%d", i);
									db_set_s(NULL, SKYPE_PROTONAME, buf, dbv.pszVal);
									db_free(&dbv);
								} else break;
							}
							db_set_s(NULL, SKYPE_PROTONAME, "LastNumber0", number);
						}
						TempAdded=TRUE;
					}
					if (!db_set_s(hContact, SKYPE_PROTONAME, "SkypeOutNr", number)) {
						msg=(char *)malloc(strlen(number)+6);
						strcpy(msg, "CALL ");
						strcat(msg, number);
						if (SkypeSend(msg) || (ptr=SkypeRcv("ERROR", 500))) {
							db_unset(hContact, SKYPE_PROTONAME, "SkypeOutNr");
							if (ptr) {
								OUTPUTA(ptr);
								free(ptr);
							}
							if (TempAdded) CallService(MS_DB_CONTACT_DELETE, hContact, 0);
						}
						free(msg);
					}
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
			}			
			break;
		case WM_DESTROY:
			Utils_SaveWindowPosition(hwndDlg, NULL, SKYPE_PROTONAME, "DIALdlg");
			if (httbButton) CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)httbButton, TTBST_RELEASED);
			break;
	}
	return FALSE;
}

/* CallstatDlgProc
 * 
 * Purpose: Dialog procedure for the CallStatus Dialog
 */
static INT_PTR CALLBACK CallstatDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int selected;
	static DBVARIANT dbv, dbv2={0};

	switch (uMsg){
		case WM_INITDIALOG:	
		{
			MCONTACT hContact;

			if (!db_get_s((MCONTACT)lParam, SKYPE_PROTONAME, "CallId", &dbv)) {

				// Check, if another call is in progress
				for (hContact=db_find_first();hContact != NULL;hContact=db_find_next(hContact)) {
					char *szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, hContact, 0 );
					if (szProto != NULL && !strcmp(szProto, SKYPE_PROTONAME) && hContact != (MCONTACT)lParam &&
						db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 0 &&
						!db_get_s(hContact, SKYPE_PROTONAME, "CallId", &dbv2)) 
					{
						if (db_get_b(hContact, SKYPE_PROTONAME, "OnHold", 0)) {
							db_free(&dbv2);
							continue;
						} else break;
					}
				}
	
				if (dbv2.pszVal)
				{
					char buf[256], buf2[256];
					char *szOtherCaller=(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,0);

					Utils_RestoreWindowPosition(hwndDlg, NULL, SKYPE_PROTONAME, "CALLSTATdlg");
					TranslateDialogDefault(hwndDlg);
					SendMessage(hwndDlg, WM_COMMAND, IDC_JOIN, 0);

					GetWindowTextA(hwndDlg, buf, sizeof(buf));
					_snprintf(buf2, sizeof(buf), buf, CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)lParam,0));
					SetWindowTextA(hwndDlg, buf2);
					
					GetDlgItemTextA(hwndDlg, IDC_JOIN, buf, sizeof(buf));
					_snprintf(buf2, sizeof(buf), buf, szOtherCaller);
					SetDlgItemTextA(hwndDlg, IDC_JOIN, buf2);

					GetDlgItemTextA(hwndDlg, IDC_HOLD, buf, sizeof(buf));
					_snprintf(buf2, sizeof(buf), buf, szOtherCaller);
					SetDlgItemTextA(hwndDlg, IDC_HOLD, buf2);

					return TRUE;
				}

				// No other call in progress, no need for this Dlg., just answer the call
				SkypeSend("SET %s STATUS INPROGRESS", dbv.pszVal);
                testfor ("ERROR", 200);
				db_free(&dbv);
			}
			DestroyWindow(hwndDlg);
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_JOIN:
				case IDC_HOLD:
				case IDC_HANGUP:
					CheckRadioButton(hwndDlg, IDC_JOIN, IDC_HANGUP, (selected=LOWORD(wParam)));
					break;
				case IDOK:
				{
					char *szIdCall2;

					switch (selected) {
						case IDC_JOIN: 
							if (szIdCall2=strchr(dbv2.pszVal, ' '))
								SkypeSend("SET %s JOIN_CONFERENCE%s", dbv.pszVal, szIdCall2);
							break;
						case IDC_HOLD:
							SkypeSend("SET %s STATUS ONHOLD", dbv2.pszVal);
							SkypeSend("SET %s STATUS INPROGRESS", dbv.pszVal);
							break;
						case IDC_HANGUP:
							SkypeSend("SET %s STATUS FINISHED", dbv.pszVal);
							break;
					}
					
					db_free(&dbv);
					db_free(&dbv2);
					DestroyWindow(hwndDlg);
					break;
				}
			}
			break;
		case WM_DESTROY:
			Utils_SaveWindowPosition(hwndDlg, NULL, SKYPE_PROTONAME, "CALLSTATdlg");
			break;
	}
	return FALSE;
}


/* SkypeOutCallErrorCheck
 * 
 * Purpose: Checks, if an error has occured after call and
 *          if so, hangs up the call 
 *		    This procedure is a seperate thread to not block the core 
 *		    while waiting for "ERROR"
 * Params : szCallId - ID of the call
 */
void SkypeOutCallErrorCheck(char *szCallId) {
	if (testfor("ERROR", 500)) EndCallThread(szCallId);
}

/* SkypeOutCall
 * 
 * Purpose: Give a SkypeOut call to the given User in wParam
 *          or hangs up existing call
 *			The user's record is searched for Phone-number entries.
 *			If there is more than 1 entry, the Dial-Dialog is shown
 * Params : wParam - Handle to the User to be called
 *					 If NULL, the dial-dialog is shown
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeOutCall(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	int res = -1;

	if (wParam && !db_get_s((MCONTACT)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
		res=SkypeSend("SET %s STATUS FINISHED", dbv.pszVal);
		pthread_create(( pThreadFunc )SkypeOutCallErrorCheck, _strdup(dbv.pszVal));
		db_free(&dbv);
	} else if (!CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIAL), NULL, DialDlgProc, (LPARAM)wParam)) return -1;
	return res;
}

/* SkypeHoldCall
 * 
 * Purpose: Put the call to the User given in wParam on Hold or Resumes it
 * Params : wParam - Handle to the User 
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeHoldCall(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	int retval;

	LOG(("SkypeHoldCall started"));
	if (!wParam || db_get_s((MCONTACT)wParam, SKYPE_PROTONAME, "CallId", &dbv))
		return -1;
	retval = SkypeSend ("SET %s STATUS %s", dbv.pszVal, 
		db_get_b((MCONTACT)wParam, SKYPE_PROTONAME, "OnHold", 0)?"INPROGRESS":"ONHOLD");
	db_free(&dbv);
	return retval;
}

/* SkypeAnswerCall
 * 
 * Purpose: Answer a Skype-call when a user double-clicks on
 *			The incoming-call-Symbol. Works for both, Skype and SkypeOut-calls
 * Params : wParam - Not used
 *			lParam - CLISTEVENT*
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeAnswerCall(WPARAM wParam, LPARAM lParam) {

	LOG(("SkypeAnswerCall started"));
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CALLSTAT), NULL, CallstatDlgProc, (LPARAM)((CLISTEVENT*)lParam)->hContact);
	return 0;
}
/* SkypeSetNick
 * 
 * Purpose: Set Full Name in profile
 * Params : wParam=0
 *			lParam=(LPARAM)(const char*)Nick text
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeSetNick(WPARAM wParam, LPARAM lParam) {
	int retval = -1;
	char *Nick = NULL;
	
	if (wParam & SMNN_UNICODE)
	{
		db_set_ws(0, SKYPE_PROTONAME, "Nick", (WCHAR*)lParam);
		if (AttachStatus == SKYPECONTROLAPI_ATTACH_SUCCESS &&
			!(Nick = (char*)make_utf8_string((WCHAR*)lParam))) return -1;
	}
	else
	{
		db_set_s(0, SKYPE_PROTONAME, "Nick", (char*)lParam);
		if(AttachStatus == SKYPECONTROLAPI_ATTACH_SUCCESS &&
			utf8_encode((const char *)lParam, &Nick) == -1 ) return -1;
	}
	if(AttachStatus == SKYPECONTROLAPI_ATTACH_SUCCESS)
		retval = SkypeSend("SET PROFILE FULLNAME %s", Nick);
	if (Nick) free (Nick);

	return retval;

}
/* SkypeSetAwayMessage
 * 
 * Purpose: Set Mood message in profile
 * Params : wParam=status mode
 *			lParam=(LPARAM)(const char*)message text
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeSetAwayMessage(WPARAM wParam, LPARAM lParam) {
	int retval = -1;
	char *Mood = NULL;
	
	if (!lParam) lParam=(LPARAM)"";
	if(utf8_encode((const char *)lParam, &Mood) == -1 ) return -1;
	db_set_s(NULL, SKYPE_PROTONAME, "MoodText", (const char *)lParam);
	 
	if(AttachStatus == SKYPECONTROLAPI_ATTACH_SUCCESS)
		retval = SkypeSend("SET PROFILE MOOD_TEXT %s", Mood);
	free (Mood);

	return retval;
}
INT_PTR SkypeSetAwayMessageW(WPARAM wParam, LPARAM lParam) {
	int retval = -1;
	char *Mood = NULL;
	
	if (!lParam) lParam=(LPARAM)"";
	if (!(Mood = (char*)make_utf8_string((WCHAR*)lParam))) return -1;
	db_set_ws(NULL, SKYPE_PROTONAME, "MoodText", (WCHAR*)lParam);
	 
	if(AttachStatus == SKYPECONTROLAPI_ATTACH_SUCCESS)
		retval = SkypeSend("SET PROFILE MOOD_TEXT %s", Mood);
	free (Mood);

	return retval;
}

/* SkypeSetAvatar
 * 
 * Purpose: Set user avatar in profile
 * Params : wParam=0
 *			lParam=(LPARAM)(const char*)filename
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeSetAvatar(WPARAM wParam, LPARAM lParam) {
	char *filename = (char *) lParam, *ext;
	char AvatarFile[MAX_PATH+1], OldAvatarFile[1024];
	char *ptr = NULL;
	int ret;
	char command[500];
	DBVARIANT dbv = {0};
	BOOL hasOldAvatar = (db_get_s(NULL, SKYPE_PROTONAME, "AvatarFile", &dbv) == 0 && dbv.type == DBVT_ASCIIZ);
	size_t len;

	if (AttachStatus != SKYPECONTROLAPI_ATTACH_SUCCESS)
		return -3;
	
	if (filename == NULL)
		return -1;
	len = strlen(filename);
	if (len < 4)
		return -1;

	ext = &filename[len-4];
	if (_stricmp(ext, ".jpg")==0 || _stricmp(ext-1, ".jpeg")==0)
		ext = "jpg";
	else if (_stricmp(ext, ".png")==0)
		ext = "png";
	else
		return -2;
	
	FoldersGetCustomPath(hProtocolAvatarsFolder, AvatarFile, sizeof(AvatarFile), DefaultAvatarsFolder);
	if (!*AvatarFile) strcpy (AvatarFile, DefaultAvatarsFolder);
	mir_snprintf(AvatarFile, sizeof(AvatarFile), "%s\\%s avatar.%s", AvatarFile, SKYPE_PROTONAME, ext);

	// Backup old file
	if (hasOldAvatar)
	{
		strncpy(OldAvatarFile, dbv.pszVal, sizeof(OldAvatarFile)-4);
		OldAvatarFile[sizeof(OldAvatarFile)-5] = '\0';
		strcat(OldAvatarFile, "_old");
		DeleteFileA(OldAvatarFile);
		if (!MoveFileA(dbv.pszVal, OldAvatarFile))
		{
			db_free(&dbv);
			return -3;
		}
	}

	// Copy new file
	if (!CopyFileA(filename, AvatarFile, FALSE))
	{
		if (hasOldAvatar)
		{
			MoveFileA(OldAvatarFile, dbv.pszVal);
			db_free(&dbv);
		}
		return -3;
	}

	// Try to set with skype
	mir_snprintf(command, sizeof(command), "SET AVATAR 1 %s", AvatarFile);
	if (SkypeSend(command) || (ptr = SkypeRcv(command+4, INFINITE)) == NULL || !strncmp(ptr, "ERROR", 5))
	{
		DeleteFileA(AvatarFile);

		if (hasOldAvatar)
			MoveFileA(OldAvatarFile, dbv.pszVal);

		ret = -4;
	}
	else
	{
		if (hasOldAvatar)
			DeleteFileA(OldAvatarFile);

		db_set_s(NULL, SKYPE_PROTONAME, "AvatarFile", AvatarFile);

		ret = 0;
	}

	if (ptr != NULL)
		free(ptr);

	if (hasOldAvatar)
		db_free(&dbv);

	return ret;
}


/* SkypeSendFile
 * 
 * Purpose: Opens the Skype-dialog to send a file
 * Params : wParam - Handle to the User 
 *          lParam - Not used
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeSendFile(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	int retval;

	if (!wParam || db_get_s((MCONTACT)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
		return -1;
	retval=SkypeSend("OPEN FILETRANSFER %s", dbv.pszVal);
	db_free(&dbv);
	return retval;
}

/* SkypeChatCreate
 * 
 * Purpose: Creates a groupchat with the user
 * Params : wParam - Handle to the User 
 *          lParam - Not used
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeChatCreate(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	MCONTACT hContact=(MCONTACT)wParam;
	char *ptr, *ptr2;

	if (!hContact || db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
		return -1;
	// Flush old messages
	while (testfor("\0CHAT \0 STATUS \0", 0));
	if (SkypeSend("CHAT CREATE %s", dbv.pszVal) || !(ptr=SkypeRcv ("\0CHAT \0 STATUS \0", INFINITE)))
	{
		db_free(&dbv);
		return -1;
	}
	db_free(&dbv);
    if (ptr2=strstr (ptr, "STATUS")) {
		*(ptr2-1)=0;
		ChatStart (ptr+5, FALSE);
	}
	free(ptr);
	return 0;
}

/* SkypeAdduserDlg
 * 
 * Purpose: Show Skype's Add user Dialog
 */
INT_PTR SkypeAdduserDlg(WPARAM wParam, LPARAM lParam) {
	SkypeSend("OPEN ADDAFRIEND");
	return 0;
}

/* SkypeFlush
 * 
 * Purpose: Flush the Skype Message-List
 */
void SkypeFlush(void) {
	char *ptr;

	while ((ptr=SkypeRcv(NULL, 0))!=NULL) free(ptr);
}

/* SkypeStatusToMiranda
 * 
 * Purpose: Converts the specified Skype-Status mode to the corresponding Miranda-Status mode
 * Params : s - Skype Status
 * Returns: The correct Status
 *		    0 - Nothing found
 */
int SkypeStatusToMiranda(char *s) {
	int i;
	if (!strcmp("SKYPEOUT", s)) return db_get_dw(NULL, SKYPE_PROTONAME, "SkypeOutStatusMode", ID_STATUS_ONTHEPHONE);
	for(i=0; status_codes[i].szStat; i++)
		if (!strcmp(status_codes[i].szStat, s))
		return status_codes[i].id;
	return 0;
}

/* MirandaStatusToSkype
 * 
 * Purpose: Converts the specified Miranda-Status mode to the corresponding Skype-Status mode
 * Params : id - Miranda Status
 * Returns: The correct Status
 *		    NULL - Nothing found
 */
char *MirandaStatusToSkype(int id) {
	int i;
	if (db_get_b(NULL, SKYPE_PROTONAME, "NoSkype3Stats", 0)) {
		switch (id)
		{
		case ID_STATUS_NA: return "AWAY";
		case ID_STATUS_FREECHAT: return "ONLINE";
		}
	}
	for(i=0; status_codes[i].szStat; i++)
		if (status_codes[i].id==id)
			return status_codes[i].szStat;
	return NULL;
}

/* GetSkypeErrorMsg
 * 
 * Purpose: Get a human-readable Error-Message for the supplied Skype Error-Message
 * Params : str - Skype Error-Message string
 * Returns: Human-readable Error Message or NULL, if nothing was found
 * Warning: Don't forget to free() return value
 */
char *GetSkypeErrorMsg(char *str) {
	char *pos, *reason, *msg;

    LOG (("GetSkypeErrorMsg received error: %s", str));
	if (!strncmp(str, "ERROR", 5)) {
		reason=_strdup(str);
		return reason;
	}
	if ((pos=strstr(str, "FAILURE")) ) {
		switch(atoi(pos+14)) {
			case MISC_ERROR: msg="Misc. Error"; break;
			case USER_NOT_FOUND: msg="User does not exist, check username"; break;
			case USER_NOT_ONLINE: msg="Trying to send IM to an user, who is not online"; break;
			case USER_BLOCKED: msg="IM blocked by recipient"; break;
			case TYPE_UNSUPPORTED: msg="Type unsupported"; break;
			case SENDER_NOT_FRIEND: msg="Sending IM message to user, who has not added you to friendslist and has chosen 'only people in my friendslist can start IM'"; break;
			case SENDER_NOT_AUTHORIZED: msg="Sending IM message to user, who has not authorized you and has chosen 'only people whom I have authorized can start IM'"; break;
			default: msg="Unknown error";
		}
		reason=(char *)malloc(strlen(pos)+strlen(msg)+3);
		sprintf (reason, "%s: %s", pos, msg);
		return reason;
	}
	return NULL;
}

/* testfor
 * 
 * Purpose: Wait, until the given Message-Fragment is received from Skype within
 *			the given amount of time
 * Params : see SkypeRcv
 * Returns: TRUE - Message was received within the given amount of time
 *			FALSE- nope, sorry
 */
BOOL testfor(char *what, DWORD maxwait) {
	char *res;

	if ((res=SkypeRcv(what, maxwait))==NULL) return FALSE;
	free(res);
	return TRUE;
}

char SendSkypeproxyCommand(char command) {
	int length=0;
	char reply=0;
	BOOL res;
	
	res = send(ClientSocket, (char *)&length, sizeof(length), 0)==SOCKET_ERROR
		|| send(ClientSocket, (char *)&command, sizeof(command), 0)==SOCKET_ERROR
		|| recv(ClientSocket, (char *)&reply, sizeof(reply), 0)==SOCKET_ERROR;
	if (res)
		return -1;
	else
		return reply;
}

/* ConnectToSkypeAPI
 * 
 * Purpose: Establish a connection to the Skype API
 * Params : path - Path to the Skype application
 *          iStart - Need to start skype for status change.
 *             1 = Normal start if Skype not running
 *             2 = Forced startp code execution no matter what
 * Returns: 0 - Connecting succeeded
 *		   -1 - Something went wrong
 */
int ConnectToSkypeAPI(char *path, int iStart) {
	static int iRet = -1; // last request result
	static volatile long newRequest = TRUE;

	InterlockedExchange(&newRequest, TRUE); // place new request
	EnterCriticalSection(&ConnectMutex); // Prevent reentrance
	if (iRet == -1 || newRequest)
	{
		iRet = _ConnectToSkypeAPI(path, iStart);
		InterlockedExchange(&newRequest, FALSE); // every thread which is waiting for connect mutex will get our result as well.. but subsequent calls will set this value to true and call _Connect again
	}
	LeaveCriticalSection(&ConnectMutex);
	return iRet;
}

void TranslateMirandaRelativePathToAbsolute(LPCSTR cszPath, LPSTR szAbsolutePath, BOOL fQuoteSpaces) {
	*szAbsolutePath = 0;
	CallService (MS_UTILS_PATHTOABSOLUTE, (WPARAM)(*cszPath ? cszPath : ".\\"), (LPARAM)szAbsolutePath);
	if(fQuoteSpaces && strchr((LPCSTR)szAbsolutePath, ' ')){
		memmove (szAbsolutePath+1, szAbsolutePath, strlen(szAbsolutePath)+1);
		*szAbsolutePath='"';
		strcat (szAbsolutePath, "\"");
	}

	TRACEA(szAbsolutePath);
}

static int my_spawnv(const char *cmdname, const char *const *argv, PROCESS_INFORMATION *pi)
{
	int i, iLen=0;
	char *CommandLine;
	STARTUPINFOA si={0};
	BOOL bRet;

	memset (pi, 0, sizeof(PROCESS_INFORMATION));
	for (i=0; argv[i]; i++)
		iLen += (int)strlen(argv[i])+1;
	if (!(CommandLine = (char*)calloc(1, iLen))) return -1;
	for (i=0; argv[i]; i++) {
		if (i) strcat (CommandLine, " ");
		strcat (CommandLine, argv[i]);
	}
	si.cb = sizeof(si);

	bRet = CreateProcessA( cmdname,CommandLine,NULL,NULL,FALSE,0,NULL,NULL,&si,pi);
	free(CommandLine);
	if (!bRet) return -1;
	return (DWORD)pi->hProcess;
}

static int _ConnectToSkypeAPI(char *path, int iStart) {
	BOOL SkypeLaunched=FALSE;
	BOOL UseCustomCommand = db_get_b(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	int counter=0, i, j, maxattempts=db_get_w(NULL, SKYPE_PROTONAME, "ConnectionAttempts", 10);
	char *args[16], *pFree = NULL;
	char *SkypeOptions[]={"/notray", "/nosplash", "/minimized", "/removable", "/datapath:", "/secondary"};
	const int SkypeDefaults[]={0, 1, 1, 0, 0};

	char szAbsolutePath[MAX_PATH];

	LOG(("ConnectToSkypeAPI started."));
	if (UseSockets) 
	{
		SOCKADDR_IN service;
		DBVARIANT dbv;
		long inet;
		struct hostent *hp;

		LOG(("ConnectToSkypeAPI: Connecting to Skype2socket socket..."));
        if ((ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==INVALID_SOCKET) return -1;

		if (!db_get_s(NULL, SKYPE_PROTONAME, "Host", &dbv)) {
			if ((inet=inet_addr(dbv.pszVal))==-1) {
				if (hp=gethostbyname(dbv.pszVal))
					memcpy(&inet, hp->h_addr, sizeof(inet));
				else {
					OUTPUT(_T("Cannot resolve host!"));
					db_free(&dbv);
					return -1;
				}
			}
			db_free(&dbv);
		} else {
			OUTPUT(_T("Cannot find valid host to connect to."));
			return -1;
		}

		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet;
		service.sin_port = htons((unsigned short)db_get_w(NULL, SKYPE_PROTONAME, "Port", 1401));
	
		if ( connect( ClientSocket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR) return -1;
            
		if (db_get_b(NULL, SKYPE_PROTONAME, "RequiresPassword", 0) && !db_get_s(NULL, SKYPE_PROTONAME, "Password", &dbv)) 
		{
				char reply=0;

				if ((reply=SendSkypeproxyCommand(AUTHENTICATE))==-1) {
					db_free(&dbv);
					return -1;
				}
				if (!reply) {
					OUTPUT(_T("Authentication is not supported/needed for this Skype proxy server. It will be disabled."));
					db_set_b(NULL, SKYPE_PROTONAME, "RequiresPassword", 0);
				} else {
					unsigned int length=(unsigned int)strlen(dbv.pszVal);
					BOOL res;
					res = send(ClientSocket, (char *)&length, sizeof(length), 0)==SOCKET_ERROR
						|| send(ClientSocket, dbv.pszVal, length, 0)==SOCKET_ERROR
						|| recv(ClientSocket, (char *)&reply, sizeof(reply), 0)==SOCKET_ERROR;
					if (res)
					{
							db_free(&dbv);
							return -1;
					}
					if (!reply) 
					{
						OUTPUT(_T("Authentication failed for this server, connection was not successful. Verify that your password is correct!"));
						db_free(&dbv);
						return -1;
					}
				}
				db_free(&dbv);
		} 
		else 
		{
			char reply=0;

			if ((reply=SendSkypeproxyCommand(CAPABILITIES))==-1) return -1;
			if (reply&USE_AUTHENTICATION) {
				OUTPUT(_T("The server you specified requires authentication, but you have not supplied a password for it. Check the Skype plugin settings and try again."));
				return -1;
			}
		}


		if (!rcvThreadRunning)
			if(_beginthread(( pThreadFunc )rcvThread, 0, NULL)==-1) return -1;
                
		AttachStatus=SKYPECONTROLAPI_ATTACH_SUCCESS;
		return 0;
	}

	if (pszProxyCallout)
	{
		if (SkypeSend("SET USERSTATUS ONLINE")==-1)
		{
			AttachStatus=SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE;
			return -1;
		}
		for ( ;; ) {
			char *ptr = SkypeRcv ("CONNSTATUS", INFINITE);
			if (!ptr) 
			{
				AttachStatus=SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE;
				return -1;
			}

			if (strcmp (ptr+11, "CONNECTING"))
			{
				free (ptr);
				break;
			}
			free (ptr);
		}

		AttachStatus=SKYPECONTROLAPI_ATTACH_SUCCESS;
		return 0;
	}

	do 
	{
        int retval;
		/*	To initiate communication, Client should broadcast windows message
			('SkypeControlAPIDiscover') to all windows in the system, specifying its own
			window handle in wParam parameter.
		 */
		if (iStart != 2 || counter)
		{
			LOG(("ConnectToSkypeAPI sending discover message.. hWnd=%08X", (long)g_hWnd));
			retval = (int)SendMessageTimeout(HWND_BROADCAST, ControlAPIDiscover, (WPARAM)g_hWnd, 0, SMTO_ABORTIFHUNG, 3000, NULL);
			LOG(("ConnectToSkypeAPI sent discover message returning %d", retval));
		}

		/*	In response, Skype responds with
			message 'SkypeControlAPIAttach' to the handle specified, and indicates
			connection status
			SkypeReady is set if there is an answer by Skype other than API_AVAILABLE.
			If there is no answer after 3 seconds, launch Skype as it's propably
			not running.
		*/
		if (iStart == 2 || (WaitForSingleObject(SkypeReady, 3000)==WAIT_TIMEOUT && AttachStatus!=SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION)) 
		{
			if (iStart != 2 && g_hWnd==NULL) 
			{
				LOG(("ConnectToSkypeAPI: hWnd of SkypeDispatchWindow not yet set.."));
				continue;
			}
			if ((iStart == 2 || !SkypeLaunched) && (path ||  UseCustomCommand)) 
			{
				static PROCESS_INFORMATION pi={0};
				DWORD dwExitStatus = 0;

				if ((!pi.hProcess || !GetExitCodeProcess(pi.hProcess, &dwExitStatus) || dwExitStatus != STILL_ACTIVE) &&
					(db_get_b(NULL, SKYPE_PROTONAME, "StartSkype", 1) || iStart))
				{
					LOG(("ConnectToSkypeAPI Starting Skype, as it's not running"));

					j=1;
					for (i=0; i < SIZEOF(SkypeOptions); i++)
						if (db_get_b(NULL, SKYPE_PROTONAME, SkypeOptions[i]+1, SkypeDefaults[i])) {
							DBVARIANT dbv;

							switch (i)
							{
							case 4:
								if(!db_get_s(NULL,SKYPE_PROTONAME,"datapath",&dbv)) 
								{
									int paramSize;
									TranslateMirandaRelativePathToAbsolute(dbv.pszVal, szAbsolutePath, TRUE);
									paramSize = (int)strlen(SkypeOptions[i]) + (int)strlen(szAbsolutePath);
									pFree = args[j] = (char*)malloc(paramSize + 1);
									sprintf(args[j],"%s%s",SkypeOptions[i],szAbsolutePath);
									db_free(&dbv);
								}
								break;
							case 2:
								args[j++]="/legacylogin";
							default:
								args[j]=SkypeOptions[i];
								break;
							}
							LOG(("Using Skype parameter: %s", args[j]));
							//MessageBox(NULL,"Using Skype parameter: ",args[j],0);
							j++;
						}
					args[j]=NULL;

					if(UseCustomCommand)
					{
						DBVARIANT dbv;

						if(!db_get_s(NULL,SKYPE_PROTONAME,"CommandLine",&dbv)) 
						{
							TranslateMirandaRelativePathToAbsolute(dbv.pszVal, szAbsolutePath, FALSE);
							args[0] = (LPSTR)szAbsolutePath;
							LOG(("ConnectToSkypeAPI: Launch skype using command line"));
							if (!*szAbsolutePath || my_spawnv(szAbsolutePath, args, &pi) == -1) {
								LOG(("ConnectToSkypeAPI: Failed to launch skype!"));
							} else {
								WaitForInputIdle((HANDLE)pi.hProcess, 5000);
								setUserNamePassword(pi.dwProcessId);
							}
							db_free(&dbv);
						}
					}
					else
					{
						args[0]=path;
						LOG(("ConnectToSkypeAPI: Launch skype"));
						/*for(int i=0;i<j;i++)
						{
							if(args[i] != NULL)
									LOG("ConnectToSkypeAPI", args[i]);
						}*/

						// if there is no skype installed and no custom command line, then exit .. else it crashes
						if (args[0] == NULL || strlen(args[0])==0)
						{
							return -1;
						}
						if (my_spawnv(path, args, &pi) != -1) {
							WaitForInputIdle((HANDLE)pi.hProcess, 5000);
							setUserNamePassword(pi.dwProcessId);
						}
					}
					if (pFree) free(pFree);
				}
				ResetEvent(SkypeReady);
				SkypeLaunched=TRUE;
				LOG(("ConnectToSkypeAPI: Skype process started."));
				// Skype launching iniciated, keep sending Discover messages until it responds.
				continue;
			} 
			else 
			{
				LOG(("ConnectToSkypeAPI: Check if Skype was launchable.."));
				if (db_get_b(NULL, SKYPE_PROTONAME, "StartSkype", 1) && !(path ||  UseCustomCommand)) return -1;
				LOG(("Trying to attach: #%d", counter));
				counter++;
				if (counter>=maxattempts && AttachStatus==-1) 
				{
					int oldstatus=SkypeStatus;
					InterlockedExchange((long *)&SkypeStatus, (int)ID_STATUS_OFFLINE);
					ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
					OUTPUT(_T("ERROR: Skype not running / too old / working!"));
					return -1;
				}
			}
		}
		LOG(("Attachstatus %d", AttachStatus));
	} while (AttachStatus==SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE || AttachStatus==SKYPECONTROLAPI_ATTACH_API_AVAILABLE || AttachStatus==-1);
	
	while (AttachStatus==SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION) Sleep(1000);
	LOG(("Attachstatus %d", AttachStatus));
	if (AttachStatus!=SKYPECONTROLAPI_ATTACH_SUCCESS) {
		int oldstatus;

		switch(AttachStatus) {
			case SKYPECONTROLAPI_ATTACH_REFUSED:
				OUTPUT(_T("Skype refused the connection :("));
				break;
			case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
				OUTPUT(_T("The Skype API is not available"));
				break;
			default:
				LOG(("ERROR: AttachStatus: %d", AttachStatus));
				OUTPUT(_T("Wheee, Skype won't let me use the API. :("));
		}
		oldstatus=SkypeStatus;
		InterlockedExchange((long *)&SkypeStatus, (int)ID_STATUS_OFFLINE);
		ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldstatus, SkypeStatus);
		return -1;
	}
	
	return 0;
}

/* CloseSkypeAPI
 * Purpose: Closes existing api connection
 * Params: path - Path to the Skype application; could be NULL when using proxy
 * Returns: always 0
 */
int CloseSkypeAPI(char *skypePath)
{
	char szAbsolutePath[MAX_PATH];

	logoff_contacts(TRUE);
	if (UseSockets)
	{
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
	}
	else {
		if (!pszProxyCallout)
		{
			if (AttachStatus!=-1) 
			{
				// it was crashing when the skype-network-proxy is used (imo2sproxy for imo.im) and skype-path is empty 
				// now, with the "UseSockets" check and the skypePath[0] != 0 check its fixed
				if (skypePath != NULL && skypePath[0] != 0) {
					TranslateMirandaRelativePathToAbsolute(skypePath, szAbsolutePath, FALSE);
					_spawnl(_P_NOWAIT, szAbsolutePath, szAbsolutePath, "/SHUTDOWN", NULL);
				}
			}
		}
	}
	SkypeInitialized=FALSE;
	ResetEvent(SkypeReady);
	AttachStatus=-1;
	if (g_hWnd) KillTimer (g_hWnd, 1);
	return 0;
}
/* ConnectToSkypeAPI
 * 
 * Purpose: Establish a connection to the Skype API
 * Params : path - Path to the Skype application
 * Returns: 0 - Connecting succeeded
 *		   -1 - Something went wrong
 */
//int __connectAPI(char *path) {
//  int retval;
//  
//  EnterCriticalSection(&ConnectMutex);
//  if (AttachStatus!=-1) {
//	  LeaveCriticalSection(&ConnectMutex);
//	  return -1;
//  }
//  InterlockedExchange((long *)&SkypeStatus, ID_STATUS_CONNECTING);
//  ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) ID_STATUS_OFFLINE, SkypeStatus);
//  retval=__connectAPI(path);
//  if (retval==-1) {
//	logoff_contacts();
//	InterlockedExchange((long *)&SkypeStatus, ID_STATUS_OFFLINE);
//	ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) ID_STATUS_CONNECTING, SkypeStatus);
//  }
//  LeaveCriticalSection(&ConnectMutex);
//  return retval;
//}
