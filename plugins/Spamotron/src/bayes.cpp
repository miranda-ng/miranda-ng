#include "common.h"
#include "io.h"

#define DELIMS " ,.;!?@-\\/+&\x0D\x0A"

sqlite3 *bayesdb;
#ifdef _DEBUG
sqlite3 *bayesdbg;
#endif
HANDLE hBayesFolder;

int CheckBayes()
{
	FOLDERSGETDATA fgd = {0};
	char bayesdb_fullpath[MAX_PATH];
	char bayesdb_tmp[MAX_PATH];

	char* tmp = Utils_ReplaceVars("%miranda_userdata%");
	if (tmp[strlen(tmp)-1] == '\\')
		tmp[strlen(tmp)-1] = 0;
	mir_snprintf(bayesdb_tmp, SIZEOF(bayesdb_tmp), "%s\\%s", tmp, BAYESDB_PATH);
	mir_free(tmp);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		hBayesFolder = FoldersRegisterCustomPath(PLUGIN_NAME, Translate("Bayes database path"), bayesdb_tmp);
	} else hBayesFolder = 0;
	
	if (hBayesFolder)
		FoldersGetCustomPath(hBayesFolder, bayesdb_fullpath, MAX_PATH, bayesdb_tmp);
	else
		strncpy_s(bayesdb_fullpath, bayesdb_tmp, _TRUNCATE);

	strcat(bayesdb_fullpath, "\\"BAYESDB_FILENAME);
	if (_access(bayesdb_fullpath,0) == 0)
		return 1;
	
	return 0;
}

int OpenBayes()
{
	char bayesdb_fullpath[MAX_PATH];
	char *bayesdb_fullpath_utf8;
	char *errmsg, *tmp;
	sqlite3_stmt *stmt;

	if (hBayesFolder) {
		FoldersGetCustomPath(hBayesFolder, bayesdb_fullpath, MAX_PATH, "");
	}
	else {
		tmp = Utils_ReplaceVars("%miranda_userdata%");
		if (tmp[strlen(tmp)-1] == '\\')
			tmp[strlen(tmp)-1] = 0;
		strcpy(bayesdb_fullpath, tmp);
		strcat(bayesdb_fullpath, "\\"BAYESDB_PATH);
		mir_free(tmp);
	}

	CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)bayesdb_fullpath);

	strcat(bayesdb_fullpath, "\\"BAYESDB_FILENAME);
	bayesdb_fullpath_utf8 = mir_utf8encode(bayesdb_fullpath);
	
	if (sqlite3_open(bayesdb_fullpath_utf8, &bayesdb) == SQLITE_OK)
	{
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS spam (token blob(16), num int)", NULL, NULL, &errmsg);
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS ham (token blob(16), num int)", NULL, NULL, &errmsg);
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS stats (key varchar(32), value int)", NULL, NULL, &errmsg);
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS queue (contact int, msgtime int, message text)", NULL, NULL, &errmsg);
		sqlite3_prepare_v2(bayesdb, "SELECT count(1) FROM stats WHERE key='spam_msgcount' OR key='ham_msgcount'", -1, &stmt, NULL);
		if (sqlite3_step(stmt) == SQLITE_ROW)
			if (sqlite3_column_int(stmt, 0) != 2) {
				sqlite3_exec(bayesdb, "INSERT INTO stats VALUES ('spam_msgcount', 0)", NULL, NULL, NULL);
				sqlite3_exec(bayesdb, "INSERT INTO stats VALUES ('ham_msgcount', 0)", NULL, NULL, NULL);
			}
	} else {
		MessageBoxA(NULL, bayesdb_fullpath_utf8, "Can't open database", MB_OK);
	}

	mir_free(bayesdb_fullpath_utf8);

#ifdef _DEBUG
	tmp = Utils_ReplaceVars("%miranda_userdata%");
	if (tmp[strlen(tmp)-1] == '\\')
		tmp[strlen(tmp)-1] = 0;
	mir_snprintf(bayesdb_fullpath, SIZEOF(bayesdb_fullpath), "%s\\%s\\%s", tmp, BAYESDB_PATH, BAYESDBG_FILENAME);
	mir_free(tmp);
	bayesdb_fullpath_utf8 = mir_utf8encode(bayesdb_fullpath);
	if (sqlite3_open(bayesdb_fullpath_utf8, &bayesdbg) == SQLITE_OK)
	{
		sqlite3_exec(bayesdbg, "CREATE TABLE spam (token varchar(50), num int)", NULL, NULL, &errmsg);
		sqlite3_exec(bayesdbg, "CREATE TABLE ham (token varchar(50), num int)", NULL, NULL, &errmsg);
	}
	mir_free(bayesdb_fullpath_utf8);
#endif

	return 0;
}

char *tokenhash(const char *token, BYTE *digest)
{
	mir_md5_hash((BYTE *)token, (int)strlen(token), digest);
	return (char*)digest;
}

int get_token_count(int type)
{
	char q[200];
	int count = 0;
	sqlite3_stmt *stmt;

	if (bayesdb == NULL)
		return 0;
	mir_snprintf(q, SIZEOF(q), "SELECT COUNT(1) FROM %s", type == SPAM ? "spam" : "ham");
	sqlite3_prepare_v2(bayesdb, q, -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
	return count;
}

int get_msg_count(int type)
{
	int count = 0;
	sqlite3_stmt *stmt;

	if (bayesdb == NULL)
		return 0;
	sqlite3_prepare_v2(bayesdb, "SELECT value FROM stats WHERE key=?", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, type == SPAM ? "spam_msgcount" : "ham_msgcount", type == SPAM ? 13 : 12, NULL);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
	return count;
}

BOOL is_token_valid(char *token)
{
	unsigned int i;
	// skip digits only tokens
	for (i = 0; i < strlen(token); i++) {
		if ((unsigned char)token[i] >= 48 && (unsigned char)token[i] <= 57)
			return FALSE;
	}
	
	// skip 1- and 2-character tokens
	if (strlen(token) < 3)
		return FALSE;

	// skip "www", "com", "org", etc.
	if (!strcmp(token, "www") || !strcmp(token, "com") || !strcmp(token, "org") || !strcmp(token, "edu") ||
		!strcmp(token, "net") || !strcmp(token, "biz") || !strcmp(token, "http") || !strcmp(token, "ftp"))
		return FALSE;

	return TRUE;
}

int get_token_score(int type, char *token)
{
	char sql[200];
	int score = 0;
	BYTE digest[16];
	sqlite3_stmt *stmt;

	if (bayesdb == NULL)
		return 0;
	mir_snprintf(sql, SIZEOF(sql), "SELECT num FROM %s WHERE token=?", type == SPAM ? "spam" : "ham");
	tokenhash(token, digest);
	sqlite3_prepare_v2(bayesdb, sql, -1, &stmt, NULL);
	sqlite3_bind_blob(stmt, 1, digest, 16, NULL);
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		score = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
	return score;
}

double get_msg_score(TCHAR *msg)
{
	char *message, *token;
	double spam_prob, ham_prob, tmp1 = 1, tmp2 = 1;
	double *scores = NULL;
	int spam_msgcount, ham_msgcount, n = 0, i;

	if (bayesdb == NULL)
		return 0;

	message = mir_u2a(msg);
	spam_msgcount = get_msg_count(SPAM);
	ham_msgcount = get_msg_count(HAM);
	token = strtok(message, DELIMS);
	while (token)
	{
		if (!is_token_valid(token)) {
			token = strtok(NULL, DELIMS);
			continue;
		}
		scores = (double*)realloc(scores, sizeof(double)*(n + 1));
		spam_prob = spam_msgcount == 0 ? 0 : (double)get_token_score(SPAM, token) / (double)spam_msgcount;
		ham_prob = ham_msgcount == 0 ? 0 : (double)get_token_score(HAM, token) / (double)ham_msgcount;
		if (ham_prob == 0 && spam_prob == 0) {
			spam_prob = 0.4; ham_prob = 0.6;
		}
		spam_prob = spam_prob > 1.0 ? 1.0 : (spam_prob < 0.01 ? 0.01 : spam_prob);
		ham_prob = ham_prob > 1.0 ? 1.0 : (ham_prob < 0.01 ? 0.01 : ham_prob);
		scores[n++] = spam_prob / (spam_prob + ham_prob);
		
		token = strtok(NULL, DELIMS);
	}
	
	for (i = 0; i < n; i++) {
		tmp1 *= scores[i];
		tmp2 *= 1-scores[i];
	}
	
	mir_free(message);
	free(scores);
	return tmp1 / (tmp1 + tmp2);
}

void queue_message(MCONTACT hContact, DWORD msgtime, TCHAR *message)
{
	char *tmp;
	sqlite3_stmt *stmt;

	if (!_getOptB("BayesAutolearnApproved", defaultBayesAutolearnApproved) &&
		!_getOptB("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved))
		return;

	if (_getOptB("BayesEnabled", defaultBayesEnabled) == 0) 
		return;
	if (bayesdb == NULL)
		OpenBayes();

	sqlite3_prepare_v2(bayesdb, "INSERT INTO queue VALUES(?,?,?)", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, (DWORD)hContact);
	sqlite3_bind_int(stmt, 2, msgtime);
	tmp = mir_u2a(message);
	sqlite3_bind_text(stmt, 3, tmp, (int)strlen(tmp), NULL);
	sqlite3_step(stmt);
	mir_free(tmp);
	sqlite3_finalize(stmt);
}

void bayes_approve_contact(MCONTACT hContact)
{
	const char *message;
	TCHAR *messageW;
	int d = 0;
	sqlite3_stmt *stmt;

	if (bayesdb == NULL)
		return;

	sqlite3_prepare_v2(bayesdb, "SELECT message FROM queue WHERE contact=?", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, (DWORD)hContact);
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		d = 1;
		message = (char*)sqlite3_column_text(stmt, 0);
		messageW = mir_a2u(message);
		learn_ham(messageW);
		mir_free(messageW);
	}
	sqlite3_finalize(stmt);
	if (d) {
		sqlite3_prepare_v2(bayesdb, "DELETE FROM queue WHERE contact=?", -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, (DWORD)hContact);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	
}

void dequeue_messages()
{
	time_t t = time(NULL);
	sqlite3_stmt *stmt;
	const char *message;
	TCHAR *messageW;
	int d = 0;

	if (bayesdb == NULL)
		return;

	sqlite3_prepare_v2(bayesdb, "SELECT message FROM queue WHERE msgtime + ? < ?", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, _getOptD("BayesWaitApprove", defaultBayesWaitApprove)*86400);
	sqlite3_bind_int(stmt, 2, (DWORD)t);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		d = 1;
		message = (char*)sqlite3_column_text(stmt, 0);
		messageW = mir_a2u(message);
		learn_spam(messageW);
		mir_free(messageW);
	}
	sqlite3_finalize(stmt);
	if (d) {
		sqlite3_prepare_v2(bayesdb, "DELETE FROM queue WHERE msgtime + ? < ?", -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, _getOptD("BayesWaitApprove", defaultBayesWaitApprove)*86400);
		sqlite3_bind_int(stmt, 2, (DWORD)t);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
}

/* Learn one message as either SPAM or HAM as specified in type parameter */
void learn(int type, TCHAR *msg)
{
	char *tok, *message;
	BYTE digest[16];
	char sql_select[200], sql_update[200], sql_insert[200], sql_counter[200];
	sqlite3_stmt *stmt;
#ifdef _DEBUG
	sqlite3_stmt *stmtdbg;
#endif

	if (_getOptB("BayesEnabled", defaultBayesEnabled) == 0) 
		return;
	if (bayesdb == NULL)
		OpenBayes();

	message = mir_u2a(msg);
	tok = strtok(message, DELIMS);
	mir_snprintf(sql_counter, SIZEOF(sql_counter), "UPDATE stats SET value=value+1 WHERE key='%s'", type == SPAM ? "spam_msgcount" : "ham_msgcount");
	mir_snprintf(sql_select, SIZEOF(sql_select), "SELECT 1 FROM %s WHERE token=?", type == SPAM ? "spam" : "ham");
	mir_snprintf(sql_update, SIZEOF(sql_update), "UPDATE %s SET num=num+1 WHERE token=?", type ? "spam" : "ham");
	mir_snprintf(sql_insert, SIZEOF(sql_insert), "INSERT INTO %s VALUES(?, 1)", type ? "spam" : "ham");
#ifdef _DEBUG
	sqlite3_exec(bayesdbg, "BEGIN", NULL, NULL, NULL);
#endif
	sqlite3_exec(bayesdb, "BEGIN", NULL, NULL, NULL);
	while (tok) {
		if (!is_token_valid(tok)) {
			tok = strtok(NULL, DELIMS);
			continue;
		}
		tokenhash(tok, digest);
		sqlite3_prepare_v2(bayesdb, sql_select, -1, &stmt, NULL);
		sqlite3_bind_blob(stmt, 1, digest, 16, SQLITE_STATIC);
		if (SQLITE_ROW == sqlite3_step(stmt)) {
			sqlite3_finalize(stmt);
			sqlite3_prepare_v2(bayesdb, sql_update, -1, &stmt, NULL);
		} else {
			sqlite3_finalize(stmt);
			sqlite3_prepare_v2(bayesdb, sql_insert, -1, &stmt, NULL);
		}
		sqlite3_bind_blob(stmt, 1, digest, 16, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

#ifdef _DEBUG
		sqlite3_prepare_v2(bayesdbg, sql_select, -1, &stmtdbg, NULL);
		sqlite3_bind_text(stmtdbg, 1, tok, (int)strlen(tok), NULL);
		if (SQLITE_ROW == sqlite3_step(stmtdbg)) {
			sqlite3_finalize(stmtdbg);
			sqlite3_prepare_v2(bayesdbg, sql_update, -1, &stmtdbg, NULL);
		} else {
			sqlite3_finalize(stmtdbg);
			sqlite3_prepare_v2(bayesdbg, sql_insert, -1, &stmtdbg, NULL);
		}
		sqlite3_bind_text(stmtdbg, 1, tok, (int)strlen(tok), SQLITE_STATIC);
		sqlite3_step(stmtdbg);
		sqlite3_finalize(stmtdbg);
#endif

		tok = strtok(NULL, DELIMS);
	}
	sqlite3_exec(bayesdb, sql_counter, NULL, NULL, NULL);
	sqlite3_exec(bayesdb, "COMMIT", NULL, NULL, NULL);
#ifdef _DEBUG
	sqlite3_exec(bayesdbg, "COMMIT", NULL, NULL, NULL);
#endif
	mir_free(message);
}

void learn_ham(TCHAR *msg)
{
	learn(0, msg);
}

void learn_spam(TCHAR *msg)
{
	learn(1, msg);
}