#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_langpack.h>
#include <m_protosvc.h>

#include "m_folders.h"

#include "..\pcre16\src\pcre.h"

#include "resource.h"
#include "version.h"

#define PLUGIN_NAME "Spam-o-tron"

#define SPAMOTRON_MODE_PLAIN 0
#define SPAMOTRON_MODE_MATH 1
#define SPAMOTRON_MODE_ROTATE 2
#define SPAMOTRON_MODE_RANDOM 3

#define MAX_BUFFER_LENGTH 1024

#define _getCOptB(a,b,c) db_get_b(a, PLUGIN_NAME, b, c)
#define _setCOptB(a,b,c) db_set_b(a, PLUGIN_NAME, b, c)
#define _getCOptD(a,b,c) db_get_dw(a, PLUGIN_NAME, b, c)
#define _setCOptD(a,b,c) db_set_dw(a, PLUGIN_NAME, b, c)
#define _getOptB(a,b) _getCOptB(NULL, a, b)
#define _setOptB(a,b) _setCOptB(NULL, a, b)
#define _getOptD(a,b) _getCOptD(NULL, a, b)
#define _setOptD(a,b) _setCOptD(NULL, a, b)

TCHAR* _getCOptS(TCHAR *buf, unsigned int buflen, MCONTACT hContact, const char* option, const TCHAR *def);
#define _getOptS(a,b,c,d) _getCOptS(a, b, NULL, c, d)
#define _setCOptTS(a,b,c) db_set_ts(a, PLUGIN_NAME, b, c)
#define _setCOptS(a,b,c) db_set_s(a, PLUGIN_NAME, b, c)
#define _setOptTS(a,b) _setCOptTS(NULL, a, b)

#define defaultMode SPAMOTRON_MODE_PLAIN
#define defaultChallenge TranslateT("Spam-o-tron needs to verify you're not a bot. Reply with \"%response%\" without quotes.")
#define defaultChallengeMath TranslateT("Spam-o-tron needs to verify you're not a bot. Reply with a result of expression %mathexpr%.")
#define defaultResponse _T("no-spam")
#define defaultResponseCC TRUE
#define defaultSuccessResponse TranslateT("Verified.")
#define defaultAuthChallenge TranslateT("Spam-o-tron delayed authorization request. First reply with \"%response%\" without quotes.")
#define defaultAuthChallengeMath TranslateT("Spam-o-tron delayed authorization request. First reply with a result of expression %mathexpr%.")
#define defaultReplyOnSuccess TRUE
#define defaultReplyOnAuth TRUE
#define defaultReplyOnMsg TRUE
#define defaultApproveOnMsgOut TRUE
#define defaultApproveOnMsgIn FALSE
#define defaultAddPermanently FALSE
#define defaultHideUnverified TRUE
#define defaultKeepBlockedMsg TRUE
#define defaultMarkMsgUnreadOnApproval FALSE
#define defaultLogActions TRUE
#define defaultNotifyPopup FALSE
#define defaultDontReplySameMsg TRUE
#define defaultDontReplyMsg TRUE
#define defaultApproveOnMsgInWordlist _T("")
#define defaultDontReplyMsgWordlist _T("Spam-o-tron, StopSpam, Anti-Spam")
#define defaultMaxMsgContactCountPerDay 3
#define defaultMaxSameMsgCountPerDay 2

#define defaultNotifyPopupBlocked TRUE
#define defaultNotifyPopupApproved TRUE
#define defaultNotifyPopupChallenge TRUE
#define defaultPopupDefaultColors FALSE
#define defaultPopupWindowsColors FALSE
#define defaultPopupDefaultTimeout TRUE
#define defaultPopupBlockedTimeout 2
#define defaultPopupApprovedTimeout 2
#define defaultPopupChallengeTimeout 2
#define defaultPopupBlockedForeground RGB(0, 0, 0)
#define defaultPopupBlockedBackground RGB(240, 128, 128)
#define defaultPopupApprovedForeground RGB(0, 0, 0)
#define defaultPopupApprovedBackground RGB(128, 240, 128)
#define defaultPopupChallengeForeground RGB(0, 0, 0)
#define defaultPopupChallengeBackground RGB(180, 210, 240)

#define _NOTIFYP _getOptB("NotifyPopup", defaultNotifyPopup)

TCHAR* ReplaceVars(TCHAR *dst, unsigned int len);
TCHAR* ReplaceVarsNum(TCHAR *dst, unsigned int len, int num);
TCHAR* ReplaceVar(TCHAR *dst, unsigned int len, const TCHAR *var, const TCHAR *rvar);
int get_response_id(const TCHAR *strvar);
int get_response_num(const TCHAR *str);
TCHAR* get_response(TCHAR* dst, unsigned int dstlen, int num);

TCHAR* _tcsstr_cc(TCHAR* str, TCHAR* strSearch, BOOL cc);
BOOL _isregex(TCHAR* strSearch);
BOOL _isvalidregex(TCHAR* strSearch);
BOOL _regmatch(TCHAR* str, TCHAR* strSearch);
BOOL Contains(TCHAR* dst, TCHAR* src);
BOOL isOneDay(DWORD timestamp1, DWORD timestamp2);
void MarkUnread(MCONTACT hContact);

int ShowPopup(MCONTACT hContact, BYTE popupType, TCHAR *line1, TCHAR *line2);
int ShowPopupPreview(HWND optDlg, BYTE popupType, TCHAR *line1, TCHAR *line2);
int _notify(MCONTACT hContact, BYTE type, TCHAR *message, TCHAR *origmessage);
int LogToSystemHistory(char *message, char *origmessage);
#define POPUP_DEFAULT 0
#define POPUP_BLOCKED 1
#define POPUP_APPROVED 2
#define POPUP_CHALLENGE 3

#ifdef _UNICODE
#define CONTACT_NAME(a) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)a, GCDNF_NOMYHANDLE | GCDNF_UNICODE | GCDNF_NOCACHE)
#else
#define CONTACT_NAME(a) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)a, GCDNF_NOMYHANDLE | GCDNF_NOCACHE)
#endif


/* bayes.c */

#include "sqlite3\sqlite3.h"
extern sqlite3 *bayesdb;
#define BAYESDB_PATH "spamotron"
#define BAYESDB_FILENAME "bayes.db"
#define defaultBayesEnabled TRUE
#define defaultBayesBlockMsg FALSE
#define defaultBayesAutoApprove FALSE
#define defaultBayesAutolearnApproved TRUE
#define defaultBayesAutolearnAutoApproved FALSE
#define defaultBayesAutolearnNotApproved TRUE
#define defaultBayesAutolearnOutgoing FALSE
#define defaultBayesWaitApprove 2
#define SCORE_C 0.0001
#define defaultBayesSpamScore 9500
#define defaultBayesHamScore 500
#define HAM 0
#define SPAM 1

int OpenBayes();
int CheckBayes();
void learn(int type, TCHAR *msg);
void learn_ham(TCHAR *msg);
void learn_spam(TCHAR *msg);
int get_token_count(int type);
int get_msg_count(int type);
double get_msg_score(TCHAR *msg);
void queue_message(MCONTACT hContact, DWORD msgtime, TCHAR *message);
void bayes_approve_contact(MCONTACT hContact);
void dequeue_messages();

#ifdef _DEBUG
extern sqlite3 *bayesdbg;
#define BAYESDBG_FILENAME "bayes.dbg"
#endif