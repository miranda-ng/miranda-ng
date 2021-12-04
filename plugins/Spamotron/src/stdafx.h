#pragma once

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_contacts.h>
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
#include <m_folders.h>
#include <pcre.h>

#include "resource.h"
#include "version.h"

#define MODULENAME "Spam-o-tron"

#define SPAMOTRON_MODE_PLAIN 0
#define SPAMOTRON_MODE_MATH 1
#define SPAMOTRON_MODE_ROTATE 2
#define SPAMOTRON_MODE_RANDOM 3

#define MAX_BUFFER_LENGTH 1024

wchar_t* _getCOptS(wchar_t *buf, unsigned int buflen, MCONTACT hContact, const char* option, const wchar_t *def);
#define _getOptS(a,b,c,d) _getCOptS(a, b, NULL, c, d)

#define defaultMode SPAMOTRON_MODE_PLAIN
#define defaultChallenge TranslateT("Spam-o-tron needs to verify you're not a bot. Reply with \"%response%\" without quotes.")
#define defaultChallengeMath TranslateT("Spam-o-tron needs to verify you're not a bot. Reply with a result of expression %mathexpr%.")
#define defaultResponse L"no-spam"
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
#define defaultApproveOnMsgInWordlist L""
#define defaultDontReplyMsgWordlist L"Spam-o-tron, StopSpam, Anti-Spam"
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

#define _NOTIFYP g_plugin.getByte("NotifyPopup", defaultNotifyPopup)

wchar_t* ReplaceVars(wchar_t *dst, unsigned int len);
wchar_t* ReplaceVarsNum(wchar_t *dst, unsigned int len, int num);
wchar_t* ReplaceVar(wchar_t *dst, unsigned int len, const wchar_t *var, const wchar_t *rvar);
int get_response_id(const wchar_t *strvar);
int get_response_num(const wchar_t *str);
wchar_t* get_response(wchar_t* dst, unsigned int dstlen, int num);

wchar_t* _tcsstr_cc(wchar_t* str, wchar_t* strSearch, BOOL cc);
BOOL _isregex(wchar_t* strSearch);
BOOL _isvalidregex(wchar_t* strSearch);
BOOL _regmatch(wchar_t* str, wchar_t* strSearch);
BOOL Contains(wchar_t* dst, wchar_t* src);
BOOL isOneDay(DWORD timestamp1, DWORD timestamp2);
void MarkUnread(MCONTACT hContact);

int ShowPopup(MCONTACT hContact, BYTE popupType, wchar_t *line1, wchar_t *line2);
int ShowPopupPreview(HWND optDlg, BYTE popupType, wchar_t *line1, wchar_t *line2);
int _notify(MCONTACT hContact, BYTE type, wchar_t *message, wchar_t *origmessage);
int LogToSystemHistory(char *message, char *origmessage);
#define POPUP_DEFAULT 0
#define POPUP_BLOCKED 1
#define POPUP_APPROVED 2
#define POPUP_CHALLENGE 3

/* bayes.c */

#include <sqlite3.h>

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
void learn(int type, wchar_t *msg);
void learn_ham(wchar_t *msg);
void learn_spam(wchar_t *msg);
int get_token_count(int type);
int get_msg_count(int type);
double get_msg_score(wchar_t *msg);
void queue_message(MCONTACT hContact, DWORD msgtime, wchar_t *message);
void bayes_approve_contact(MCONTACT hContact);
void dequeue_messages();

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#ifdef _DEBUG
extern sqlite3 *bayesdbg;
#define BAYESDBG_FILENAME "bayes.dbg"
#endif
