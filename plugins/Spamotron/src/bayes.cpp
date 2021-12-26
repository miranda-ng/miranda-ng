#include "stdafx.h"
#include "io.h"

#define DELIMS " ,.;!?@-\\/+&\x0D\x0A"

sqlite3 *bayesdb;
#ifdef _DEBUG
sqlite3 *bayesdbg;
#endif
HANDLE hBayesFolder;

int CheckBayes()
{
	char bayesdb_fullpath[MAX_PATH];
	char bayesdb_tmp[MAX_PATH];

	char* tmp = Utils_ReplaceVars("%miranda_userdata%");
	if (tmp[mir_strlen(tmp)-1] == '\\')
		tmp[mir_strlen(tmp)-1] = 0;
	mir_snprintf(bayesdb_tmp, "%s\\%s", tmp, BAYESDB_PATH);
	mir_free(tmp);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		hBayesFolder = FoldersRegisterCustomPath(MODULENAME, Translate("Bayes database path"), bayesdb_tmp);
	} else hBayesFolder = nullptr;
	
	if (hBayesFolder)
		FoldersGetCustomPath(hBayesFolder, bayesdb_fullpath, MAX_PATH, bayesdb_tmp);
	else
		strncpy_s(bayesdb_fullpath, bayesdb_tmp, _TRUNCATE);

	mir_strcat(bayesdb_fullpath, "\\" BAYESDB_FILENAME);
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
		if (tmp[mir_strlen(tmp)-1] == '\\')
			tmp[mir_strlen(tmp)-1] = 0;
		mir_strcpy(bayesdb_fullpath, tmp);
		mir_strcat(bayesdb_fullpath, "\\" BAYESDB_PATH);
		mir_free(tmp);
	}

	CreateDirectoryTree(bayesdb_fullpath);

	mir_strcat(bayesdb_fullpath, "\\" BAYESDB_FILENAME);
	bayesdb_fullpath_utf8 = mir_utf8encode(bayesdb_fullpath);
	
	if (sqlite3_open(bayesdb_fullpath_utf8, &bayesdb) == SQLITE_OK)
	{
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS spam (token blob(16), num int)", nullptr, nullptr, &errmsg);
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS ham (token blob(16), num int)", nullptr, nullptr, &errmsg);
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS stats (key varchar(32), value int)", nullptr, nullptr, &errmsg);
		sqlite3_exec(bayesdb, "CREATE TABLE IF NOT EXISTS queue (contact int, msgtime int, message text)", nullptr, nullptr, &errmsg);
		sqlite3_prepare_v2(bayesdb, "SELECT count(1) FROM stats WHERE key='spam_msgcount' OR key='ham_msgcount'", -1, &stmt, nullptr);
		if (sqlite3_step(stmt) == SQLITE_ROW)
			if (sqlite3_column_int(stmt, 0) != 2) {
				sqlite3_exec(bayesdb, "INSERT INTO stats VALUES ('spam_msgcount', 0)", nullptr, nullptr, nullptr);
				sqlite3_exec(bayesdb, "INSERT INTO stats VALUES ('ham_msgcount', 0)", nullptr, nullptr, nullptr);
			}
	} else {
		MessageBoxA(nullptr, bayesdb_fullpath_utf8, "Can't open database", MB_OK);
	}

	mir_free(bayesdb_fullpath_utf8);

#ifdef _DEBUG
	tmp = Utils_ReplaceVars("%miranda_userdata%");
	if (tmp[mir_strlen(tmp)-1] == '\\')
		tmp[mir_strlen(tmp)-1] = 0;
	mir_snprintf(bayesdb_fullpath, "%s\\%s\\%s", tmp, BAYESDB_PATH, BAYESDBG_FILENAME);
	mir_free(tmp);
	bayesdb_fullpath_utf8 = mir_utf8encode(bayesdb_fullpath);
	if (sqlite3_open(bayesdb_fullpath_utf8, &bayesdbg) == SQLITE_OK)
	{
		sqlite3_exec(bayesdbg, "CREATE TABLE spam (token varchar(50), num int)", nullptr, nullptr, &errmsg);
		sqlite3_exec(bayesdbg, "CREATE TABLE ham (token varchar(50), num int)", nullptr, nullptr, &errmsg);
	}
	mir_free(bayesdb_fullpath_utf8);
#endif

	return 0;
}

char *tokenhash(const char *token, uint8_t *digest)
{
	mir_md5_hash((uint8_t *)token, (int)mir_strlen(token), digest);
	return (char*)digest;
}

int get_token_count(int type)
{
	char q[200];
	int count = 0;
	sqlite3_stmt *stmt;

	if (bayesdb == nullptr)
		return 0;
	mir_snprintf(q, "SELECT COUNT(1) FROM %s", type == SPAM ? "spam" : "ham");
	sqlite3_prepare_v2(bayesdb, q, -1, &stmt, nullptr);
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

	if (bayesdb == nullptr)
		return 0;
	sqlite3_prepare_v2(bayesdb, "SELECT value FROM stats WHERE key=?", -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, type == SPAM ? "spam_msgcount" : "ham_msgcount", type == SPAM ? 13 : 12, nullptr);
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
	for (i = 0; i < mir_strlen(token); i++) {
		if ((unsigned char)token[i] >= 48 && (unsigned char)token[i] <= 57)
			return FALSE;
	}
	
	// skip 1- and 2-character tokens
	if (mir_strlen(token) < 3)
		return FALSE;

	// skip "www", "com", "org", etc.
	if (!mir_strcmp(token, "www") || !mir_strcmp(token, "com") || !mir_strcmp(token, "org") || !mir_strcmp(token, "edu") ||
		!mir_strcmp(token, "net") || !mir_strcmp(token, "biz") || !mir_strcmp(token, "http") || !mir_strcmp(token, "ftp"))
		return FALSE;

	return TRUE;
}

int get_token_score(int type, char *token)
{
	char sql[200];
	int score = 0;
	uint8_t digest[16];
	sqlite3_stmt *stmt;

	if (bayesdb == nullptr)
		return 0;
	mir_snprintf(sql, "SELECT num FROM %s WHERE token=?", type == SPAM ? "spam" : "ham");
	tokenhash(token, digest);
	sqlite3_prepare_v2(bayesdb, sql, -1, &stmt, nullptr);
	sqlite3_bind_blob(stmt, 1, digest, 16, nullptr);
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		score = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
	return score;
}

double get_msg_score(wchar_t *msg)
{
	char *message, *token;
	double spam_prob, ham_prob, tmp1 = 1, tmp2 = 1;
	double *scores = nullptr;
	int spam_msgcount, ham_msgcount, n = 0, i;

	if (bayesdb == nullptr)
		return 0;

	message = mir_u2a(msg);
	spam_msgcount = get_msg_count(SPAM);
	ham_msgcount = get_msg_count(HAM);
	token = strtok(message, DELIMS);
	while (token)
	{
		if (!is_token_valid(token)) {
			token = strtok(nullptr, DELIMS);
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
		
		token = strtok(nullptr, DELIMS);
	}
	
	for (i = 0; i < n; i++) {
		tmp1 *= scores[i];
		tmp2 *= 1-scores[i];
	}
	
	mir_free(message);
	free(scores);
	return tmp1 / (tmp1 + tmp2);
}

void queue_message(MCONTACT hContact, uint32_t msgtime, wchar_t *message)
{
	char *tmp;
	sqlite3_stmt *stmt;

	if (!g_plugin.getByte("BayesAutolearnApproved", defaultBayesAutolearnApproved) &&
		!g_plugin.getByte("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved))
		return;

	if (g_plugin.getByte("BayesEnabled", defaultBayesEnabled) == 0) 
		return;
	if (bayesdb == nullptr)
		OpenBayes();

	sqlite3_prepare_v2(bayesdb, "INSERT INTO queue VALUES(?,?,?)", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, (uint32_t)hContact);
	sqlite3_bind_int(stmt, 2, msgtime);
	tmp = mir_u2a(message);
	sqlite3_bind_text(stmt, 3, tmp, (int)mir_strlen(tmp), nullptr);
	sqlite3_step(stmt);
	mir_free(tmp);
	sqlite3_finalize(stmt);
}

void bayes_approve_contact(MCONTACT hContact)
{
	const char *message;
	wchar_t *messageW;
	int d = 0;
	sqlite3_stmt *stmt;

	if (bayesdb == nullptr)
		return;

	sqlite3_prepare_v2(bayesdb, "SELECT message FROM queue WHERE contact=?", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, (uint32_t)hContact);
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
		sqlite3_prepare_v2(bayesdb, "DELETE FROM queue WHERE contact=?", -1, &stmt, nullptr);
		sqlite3_bind_int(stmt, 1, (uint32_t)hContact);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	
}

void dequeue_messages()
{
	time_t t = time(0);
	sqlite3_stmt *stmt;
	const char *message;
	wchar_t *messageW;
	int d = 0;

	if (bayesdb == nullptr)
		return;

	sqlite3_prepare_v2(bayesdb, "SELECT message FROM queue WHERE msgtime + ? < ?", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, g_plugin.getDword("BayesWaitApprove", defaultBayesWaitApprove)*86400);
	sqlite3_bind_int(stmt, 2, (uint32_t)t);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		d = 1;
		message = (char*)sqlite3_column_text(stmt, 0);
		messageW = mir_a2u(message);
		learn_spam(messageW);
		mir_free(messageW);
	}
	sqlite3_finalize(stmt);
	if (d) {
		sqlite3_prepare_v2(bayesdb, "DELETE FROM queue WHERE msgtime + ? < ?", -1, &stmt, nullptr);
		sqlite3_bind_int(stmt, 1, g_plugin.getDword("BayesWaitApprove", defaultBayesWaitApprove)*86400);
		sqlite3_bind_int(stmt, 2, (uint32_t)t);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
}

/* Learn one message as either SPAM or HAM as specified in type parameter */
void learn(int type, wchar_t *msg)
{
	char *tok, *message;
	uint8_t digest[16];
	char sql_select[200], sql_update[200], sql_insert[200], sql_counter[200];
	sqlite3_stmt *stmt;
#ifdef _DEBUG
	sqlite3_stmt *stmtdbg;
#endif

	if (g_plugin.getByte("BayesEnabled", defaultBayesEnabled) == 0) 
		return;
	if (bayesdb == nullptr)
		OpenBayes();

	message = mir_u2a(msg);
	tok = strtok(message, DELIMS);
	mir_snprintf(sql_counter, "UPDATE stats SET value=value+1 WHERE key='%s'", type == SPAM ? "spam_msgcount" : "ham_msgcount");
	mir_snprintf(sql_select, "SELECT 1 FROM %s WHERE token=?", type == SPAM ? "spam" : "ham");
	mir_snprintf(sql_update, "UPDATE %s SET num=num+1 WHERE token=?", type ? "spam" : "ham");
	mir_snprintf(sql_insert, "INSERT INTO %s VALUES(?, 1)", type ? "spam" : "ham");
#ifdef _DEBUG
	sqlite3_exec(bayesdbg, "BEGIN", nullptr, nullptr, nullptr);
#endif
	sqlite3_exec(bayesdb, "BEGIN", nullptr, nullptr, nullptr);
	while (tok) {
		if (!is_token_valid(tok)) {
			tok = strtok(nullptr, DELIMS);
			continue;
		}
		tokenhash(tok, digest);
		sqlite3_prepare_v2(bayesdb, sql_select, -1, &stmt, nullptr);
		sqlite3_bind_blob(stmt, 1, digest, 16, SQLITE_STATIC);
		if (SQLITE_ROW == sqlite3_step(stmt)) {
			sqlite3_finalize(stmt);
			sqlite3_prepare_v2(bayesdb, sql_update, -1, &stmt, nullptr);
		} else {
			sqlite3_finalize(stmt);
			sqlite3_prepare_v2(bayesdb, sql_insert, -1, &stmt, nullptr);
		}
		sqlite3_bind_blob(stmt, 1, digest, 16, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

#ifdef _DEBUG
		sqlite3_prepare_v2(bayesdbg, sql_select, -1, &stmtdbg, nullptr);
		sqlite3_bind_text(stmtdbg, 1, tok, (int)mir_strlen(tok), nullptr);
		if (SQLITE_ROW == sqlite3_step(stmtdbg)) {
			sqlite3_finalize(stmtdbg);
			sqlite3_prepare_v2(bayesdbg, sql_update, -1, &stmtdbg, nullptr);
		} else {
			sqlite3_finalize(stmtdbg);
			sqlite3_prepare_v2(bayesdbg, sql_insert, -1, &stmtdbg, nullptr);
		}
		sqlite3_bind_text(stmtdbg, 1, tok, (int)mir_strlen(tok), SQLITE_STATIC);
		sqlite3_step(stmtdbg);
		sqlite3_finalize(stmtdbg);
#endif

		tok = strtok(nullptr, DELIMS);
	}
	sqlite3_exec(bayesdb, sql_counter, nullptr, nullptr, nullptr);
	sqlite3_exec(bayesdb, "COMMIT", nullptr, nullptr, nullptr);
#ifdef _DEBUG
	sqlite3_exec(bayesdbg, "COMMIT", nullptr, nullptr, nullptr);
#endif
	mir_free(message);
}

void learn_ham(wchar_t *msg)
{
	learn(0, msg);
}

void learn_spam(wchar_t *msg)
{
	learn(1, msg);
}