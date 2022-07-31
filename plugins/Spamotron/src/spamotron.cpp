#include "stdafx.h"

time_t last_queue_check = 0;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// 14331048-5a73-4fdb-b909-2d7e1825a012
	{0x14331048, 0x5a73, 0x4fdb, {0xb9, 0x09, 0x2d, 0x7e, 0x18, 0x25, 0xa0, 0x12}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern int OnOptInitialize(WPARAM wParam, LPARAM lParam);

int OnDBEventFilterAdd(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	char protoOption[256] = {0};
	int buflen = MAX_BUFFER_LENGTH;
	wchar_t buf[MAX_BUFFER_LENGTH];
	wchar_t *challengeW = nullptr, *tmpW = nullptr;
	wchar_t mexpr[64];
	int maxmsglen = 0, a, b, i;
	BOOL bayesEnabled = g_plugin.getByte("BayesEnabled", defaultBayesEnabled);
	BOOL bCorrectResponse = FALSE;

	// get hContact from DBEVENTINFO as icq_proto.c doesn't pass hContact the usual way for some reason.
	if (dbei->eventType == EVENTTYPE_AUTHREQUEST)
		hContact = (MCONTACT)*(PDWORD(dbei->pBlob+sizeof(uint32_t)));

	// get maximum length of the message a protocol supports
	maxmsglen = CallProtoService(dbei->szModule, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, hContact);

	
	/*** Dequeue and learn messages ***/
	
	if (bayesEnabled && g_plugin.getByte("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved))
		if (time(0) - last_queue_check > 4*3600) { // dequeue every 4 hours
			dequeue_messages();
			last_queue_check = time(0);
		}

	/*** Check for conditional and unconditional approval ***/

	// Pass-through if protocol is not enabled
	mir_strcat(protoOption, "proto_");
	mir_strcat(protoOption, dbei->szModule);
	if (g_plugin.getByte(protoOption, 0) == 0) // Protocol is not handled by Spam-o-tron
		return 0;

	// Pass-through if the event is not of type EVENTTYPE_MESSAGE or EVENTTYPE_AUTHREQUEST
	if (dbei->eventType != EVENTTYPE_MESSAGE && dbei->eventType != EVENTTYPE_AUTHREQUEST)
		return 0;

	// Pass-through if contact is already verified.
	if (g_plugin.getByte(hContact, "Verified", 0) == 1)
		return 0;

	// Pass-through if the event is already read.
	if (dbei->flags & DBEF_READ)
		return 0;

	// Pass-through if event is from a contact that is already in the list. 
	if (!Contact::OnList(hContact)) // Already in the list
		return 0;

	// Pass-through if event is from a contact that is already in the server-side contact list
	if (db_get_w(hContact, dbei->szModule, "ServerId", 0))
		return 0;

	// Pass-through if contact is a MetaContact
	if (db_get_dw(hContact, "MetaContacts", "NumContacts", 0))
		return 0;

	// Pass-through and approve if outgoing event.
	if (dbei->flags & DBEF_SENT) {
		if (g_plugin.getByte("ApproveOnMsgOut", 0)) {
			g_plugin.setByte(hContact, "Verified", 1);
			if (g_plugin.getByte("AddPermanently", defaultAddPermanently))
				Contact::PutOnList(hContact);
			db_unset(hContact, "CList", "Delete");
		}
		return 0;
	}

	// Hide the contact until verified if option set.
	if (g_plugin.getByte("HideUnverified", defaultHideUnverified))
		Contact::Hide(hContact);

	// Fetch the incoming message body
	char *msgblob;
	if (dbei->eventType == EVENTTYPE_MESSAGE) {
		msgblob = (char *)dbei->pBlob;
	}
	else if (dbei->eventType == EVENTTYPE_AUTHREQUEST) {
		msgblob = (char *)(dbei->pBlob + sizeof(uint32_t) + sizeof(uint32_t));
		for(a = 4; a > 0; a--)
			msgblob += mir_strlen(msgblob)+1;
	}
	else msgblob = nullptr;

	wchar_t *message = nullptr;
	if (dbei->flags & DBEF_UTF)
		message = mir_utf8decodeW(msgblob);
	else
		message = mir_a2u(msgblob);
	
	/*** Check for words in white-list ***/
	if (g_plugin.getByte("ApproveOnMsgIn", defaultApproveOnMsgIn)) {
		wchar_t *whitelist = (wchar_t*)malloc(2048 * sizeof(wchar_t));
		if (whitelist != nullptr) {
			_getOptS(whitelist, 2048, "ApproveOnMsgInWordlist", defaultApproveOnMsgInWordlist);
			if (_isregex(whitelist)) {
				if (_regmatch(message, whitelist))
					bCorrectResponse = TRUE;
			}
			else {
				wchar_t *ptok = wcstok(whitelist, L" ");
				while (ptok != nullptr) {
					if (wcsstr(message, ptok)) {
						bCorrectResponse = TRUE;
						break;
					}
					ptok = wcstok(nullptr, L" ");
				}
			}
			free(whitelist);

			if (bCorrectResponse) {
				g_plugin.setByte(hContact, "Verified", 1);
				if (g_plugin.getByte("HideUnverified", defaultHideUnverified))
					Contact::Hide(hContact, false);
				if (g_plugin.getByte("AddPermanently", defaultAddPermanently))
					Contact::PutOnList(hContact);
				db_unset(hContact, "CList", "Delete");
				if (g_plugin.getByte("ReplyOnSuccess", defaultReplyOnSuccess) && (g_plugin.getByte(hContact, "MsgSent", 0))) {
					T2Utf response(_getOptS(buf, buflen, "SuccessResponse", defaultSuccessResponse));
					ProtoChainSend(hContact, PSS_MESSAGE, 0, response);
				}
				return 0;
			}
		}
	}


	/*** Check for correct answer ***/
	
	switch (g_plugin.getByte("Mode", defaultMode))
	{
		case SPAMOTRON_MODE_ROTATE:
		case SPAMOTRON_MODE_RANDOM:
			get_response(buf, buflen, g_plugin.getDword(hContact, "ResponseNum", 0));
			if (_isregex(buf)) {
				if (_regmatch(message, buf))
					bCorrectResponse = TRUE;
			} else {
				if (_tcsstr_cc(message, buf, g_plugin.getByte("ResponseCC", defaultResponseCC)) &&
					(mir_wstrlen(message) == mir_wstrlen(buf)))
					bCorrectResponse = TRUE;
			}
			break;
		
		case SPAMOTRON_MODE_PLAIN:
			_getOptS(buf, buflen, "Response", defaultResponse);
			i = get_response_num(buf);
			while (i-- > 0) {
				get_response(buf, buflen, i-1);
				if (_isregex(buf)) {
					if (_regmatch(message, buf)) {
						bCorrectResponse = TRUE;
						break;
					}
				} else {
					if (_tcsstr_cc(message, buf, g_plugin.getByte("ResponseCC", defaultResponseCC)) &&
						(mir_wstrlen(message) == mir_wstrlen(buf))) {
						bCorrectResponse = TRUE;
						break;
					}
				}
			}
			break;

		case SPAMOTRON_MODE_MATH:
			if (message == nullptr)
				break;
			_itow(g_plugin.getDword(hContact, "ResponseMath", -1), buf, 10);
			if (wcsstr(message, buf) && (mir_wstrlen(buf) == mir_wstrlen(message))) {
				bCorrectResponse = TRUE;
			}
			break;
	}

	if (bCorrectResponse)
	{
		g_plugin.setByte(hContact, "Verified", 1);
		if (g_plugin.getByte("HideUnverified", defaultHideUnverified))
			Contact::Hide(hContact, false);
		if (g_plugin.getByte("AddPermanently", defaultAddPermanently))
			Contact::PutOnList(hContact);
		db_unset(hContact, "CList", "Delete");
		db_unset(hContact, "CList", "ResponseNum");
		if (g_plugin.getByte("ReplyOnSuccess", defaultReplyOnSuccess)) {
			T2Utf response(_getOptS(buf, buflen, "SuccessResponse", defaultSuccessResponse));
			ProtoChainSend(hContact, PSS_MESSAGE, 0,	response);
		}
		_notify(hContact, POPUP_APPROVED, TranslateT("Contact %s approved."), nullptr);

		// Resubmit pending authorization request
		if (g_plugin.getByte(hContact, "AuthEventPending", FALSE)) {
			DBVARIANT _dbv;
			wchar_t AuthEventModule[100];
			char* szAuthEventModule;
			if (db_get(hContact, MODULENAME, "AuthEvent", &_dbv) == 0) {
				DBEVENTINFO dbei2 = {};
				dbei2.cbBlob = *(uint32_t *)_dbv.pbVal;
				dbei2.eventType = EVENTTYPE_AUTHREQUEST;
				_getCOptS(AuthEventModule, 100, hContact, "AuthEventModule", L"ICQ");
				szAuthEventModule = mir_u2a(AuthEventModule);
				dbei2.szModule = szAuthEventModule;
				dbei2.timestamp = dbei->timestamp;
				dbei2.pBlob = _dbv.pbVal + sizeof(uint32_t);
				db_event_add(hContact, &dbei2);

				g_plugin.delSetting(hContact, "AuthEvent");
				g_plugin.delSetting(hContact, "AuthEventPending");
				g_plugin.delSetting(hContact, "AuthEventModule");
				mir_free(szAuthEventModule);

				db_free(&_dbv);
			}
		}

		// User approved, learn from previous messages
		if (bayesEnabled && g_plugin.getByte("BayesAutolearnApproved", defaultBayesAutolearnApproved))
			bayes_approve_contact(hContact);

		// Mark previous messages unread if option set
		if (g_plugin.getByte("KeepBlockedMsg", defaultKeepBlockedMsg) && 
			g_plugin.getByte("MarkMsgUnreadOnApproval", defaultMarkMsgUnreadOnApproval) &&
			hContact != NULL) {
			// We will mark unread all blocked messages for the most recent day
			MarkUnread(hContact);
		}

		return 1;
	}


	
	/*** Check for rejection ***/

	// Completely reject if challenge was already sent today for MaxMsgContactCountPerDay times
	// and the option is turned on.
	if (isOneDay(dbei->timestamp, g_plugin.getDword(hContact, "MsgSentTime", 0)) &&
		g_plugin.getDword("MaxMsgContactCountPerDay", defaultMaxMsgContactCountPerDay) > 0 &&
		g_plugin.getDword(hContact, "MsgSent", 0) >= g_plugin.getDword("MaxMsgContactCountPerDay", defaultMaxMsgContactCountPerDay)) {
			_notify(hContact, POPUP_BLOCKED, TranslateT("Message from %s rejected because it reached a maximum for challenge requests per day."), message);
			if (bayesEnabled)
				queue_message(hContact, dbei->timestamp, message);
			return 1;
	}

	// Completely reject if duplicate incoming message found
	if (g_plugin.getDword("MaxSameMsgCountPerDay", defaultMaxSameMsgCountPerDay) > 0 &&
		g_plugin.getDword(hContact, "SameMsgCount", 0) >= g_plugin.getDword("MaxSameMsgCountPerDay", defaultMaxSameMsgCountPerDay) &&
		mir_wstrcmp(message, _getCOptS(buf, buflen, hContact, "LastInMsg", L"")) == 0) {
			_notify(hContact, POPUP_BLOCKED, TranslateT("Message from %s rejected because it reached a maximum for same responses per day."), message);
			if (bayesEnabled)
				queue_message(hContact, dbei->timestamp, message);
			return 1;
	}

	// Completely reject if incoming message contains any word from DontReplyMsgWordlist option
	if (g_plugin.getByte("DontReplyMsg", defaultDontReplyMsg) &&
		Contains(message, _getOptS(buf, buflen, "DontReplyMsgWordlist", defaultDontReplyMsgWordlist))) {
		_notify(hContact, POPUP_BLOCKED, TranslateT("Message from %s dropped because it has a word from black list."), message);
		return 1;
	}

	
	/*** Bayes checks ***/

	// Drop if score > spam score
	if (bayesEnabled && g_plugin.getByte("BayesBlockMsg", defaultBayesBlockMsg))
		if (get_msg_score(message) >= (double)g_plugin.getDword("BayesSpamScore", defaultBayesSpamScore) * SCORE_C) {
			_notify(hContact, POPUP_BLOCKED, TranslateT("Message from %s dropped because of high spam score."), message);
			if (bayesEnabled && g_plugin.getByte("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved))
				queue_message(hContact, dbei->timestamp, message);
			return 1;
		}

	// Accept if score < ham score
	if (bayesEnabled && g_plugin.getByte("BayesAutoApprove", defaultBayesAutoApprove)) 
		if (get_msg_score(message) <= (double)g_plugin.getDword("BayesHamScore", defaultBayesHamScore) * SCORE_C) {
			_notify(hContact, POPUP_APPROVED, TranslateT("Contact %s approved."), message);
			g_plugin.setByte(hContact, "Verified", 1);
			if (g_plugin.getByte("HideUnverified", defaultHideUnverified))
				Contact::Hide(hContact, false);
			if (g_plugin.getByte("AddPermanently", defaultAddPermanently))
				Contact::PutOnList(hContact);
			db_unset(hContact, "CList", "Delete");
			if (bayesEnabled && 
				g_plugin.getByte("BayesAutolearnApproved", defaultBayesAutolearnApproved) &&
				g_plugin.getByte("BayesAutolearnAutoApproved", defaultBayesAutolearnAutoApproved)) {
				queue_message(hContact, dbei->timestamp, message);
				bayes_approve_contact(hContact);
			}
			return 0;
		}

	// Accept if event is EVENTTYPE_AUTHREQUEST and ReplyOnAuth is NOT set
	if (dbei->eventType == EVENTTYPE_AUTHREQUEST && !g_plugin.getByte("ReplyOnAuth", defaultReplyOnAuth))
		return 0;
	// Accept if event is EVENTTYPE_MESSAGE and ReplyOnMsg is NOT set
	if (dbei->eventType == EVENTTYPE_MESSAGE && !g_plugin.getByte("ReplyOnMsg", defaultReplyOnMsg))
		return 0;

	/*** Send Challenge ***/

	challengeW = (wchar_t *)malloc(maxmsglen * sizeof(wchar_t));
	tmpW = (wchar_t *)malloc(maxmsglen * sizeof(wchar_t));
	switch (g_plugin.getByte("Mode", defaultMode)) {
	case SPAMOTRON_MODE_PLAIN:
		if (dbei->eventType == EVENTTYPE_AUTHREQUEST)
			_getOptS(challengeW, maxmsglen, "AuthChallenge", defaultAuthChallenge);
		else
			_getOptS(challengeW, maxmsglen, "Challenge", defaultChallenge);
		ReplaceVars(challengeW, maxmsglen);
		ProtoChainSend(hContact, PSS_MESSAGE, 0, T2Utf(challengeW));
		_notify(hContact, POPUP_CHALLENGE, TranslateT("Sending plain challenge to %s."), message);
		break;

	case SPAMOTRON_MODE_ROTATE:
		if (dbei->eventType == EVENTTYPE_AUTHREQUEST)
			_getOptS(challengeW, maxmsglen, "AuthChallenge", defaultAuthChallenge);
		else
			_getOptS(challengeW, maxmsglen, "Challenge", defaultChallenge);
		_getOptS(buf, buflen, "Response", defaultResponse);
		if (g_plugin.getDword(hContact, "ResponseNum", 0) >= (unsigned int)(get_response_num(buf) - 1))
			g_plugin.setDword(hContact, "ResponseNum", -1);

		g_plugin.setDword(hContact, "ResponseNum", g_plugin.getDword(hContact, "ResponseNum", -1) + 1);
		ReplaceVarsNum(challengeW, maxmsglen, g_plugin.getDword(hContact, "ResponseNum", 0));
		ProtoChainSend(hContact, PSS_MESSAGE, 0, T2Utf(challengeW));
		_notify(hContact, POPUP_CHALLENGE, TranslateT("Sending round-robin challenge to %s."), message);
		break;

	case SPAMOTRON_MODE_RANDOM:
		if (dbei->eventType == EVENTTYPE_AUTHREQUEST)
			_getOptS(challengeW, maxmsglen, "AuthChallenge", defaultAuthChallenge);
		else
			_getOptS(challengeW, maxmsglen, "Challenge", defaultChallenge);
		_getOptS(buf, buflen, "Response", defaultResponse);
		srand(time(0));
		g_plugin.setDword(hContact, "ResponseNum", rand() % get_response_num(buf));
		ReplaceVarsNum(challengeW, maxmsglen, g_plugin.getDword(hContact, "ResponseNum", 0));
		ProtoChainSend(hContact, PSS_MESSAGE, 0, T2Utf(challengeW));
		_notify(hContact, POPUP_CHALLENGE, TranslateT("Sending random challenge to %s."), message);
		break;

	case SPAMOTRON_MODE_MATH:
		a = (rand() % 10) + 1;
		b = (rand() % 10) + 1;
		mir_snwprintf(mexpr, L"%d + %d", a, b);
		if (dbei->eventType == EVENTTYPE_AUTHREQUEST)
			_getOptS(challengeW, maxmsglen, "AuthChallengeMath", defaultAuthChallengeMath);
		else
			_getOptS(challengeW, maxmsglen, "ChallengeMath", defaultChallengeMath);
		ReplaceVar(challengeW, maxmsglen, L"%mathexpr%", mexpr);
		g_plugin.setDword(hContact, "ResponseMath", a + b);
		ProtoChainSend(hContact, PSS_MESSAGE, 0, T2Utf(challengeW));
		_notify(hContact, POPUP_CHALLENGE, TranslateT("Sending math expression challenge to %s."), message);
		break;
	}
	free(challengeW);
	free(tmpW);

	// As a workaround for temporary NotOnList contact not being deleted from server-side list
	// (which was added by the ICQ server itself upon first outgoing challenge request message)
	// we need to set Delete setting, so that contacts gets deleted on next restart/connect.
	db_set_b(hContact, "CList", "Delete", 1);

	// Queue user message in Bayes db
	if (bayesEnabled && message != nullptr)
		queue_message(hContact, dbei->timestamp, message);


	/*** Do any post-send procedures we need to do ***/

	// Increment MsgSent if it was sent the same day. Otherwise set it to 1.
	if (isOneDay(dbei->timestamp, g_plugin.getDword(hContact, "MsgSentTime", 0)))
		g_plugin.setDword(hContact, "MsgSent", g_plugin.getDword(hContact, "MsgSent", 0) + 1);
	else
		g_plugin.setDword(hContact, "MsgSent", 1);
	g_plugin.setDword(hContact, "MsgSentTime", dbei->timestamp);

	// Save Last Msg and update SameMsgCount
	if (message != nullptr) {
		if (mir_wstrcmp(_getCOptS(buf, buflen, hContact, "LastInMsg", L""), message) == 0)
			g_plugin.setDword(hContact, "SameMsgCount", 1 + g_plugin.getDword(hContact, "SameMsgCount", 0));
		else
			g_plugin.setDword(hContact, "SameMsgCount", 1);
		g_plugin.setWString(hContact, "LastInMsg", message);
	}

	if (message != nullptr)
		mir_free(message);

	// Finally silently save the message to contact history if corresponding option is set
	if (g_plugin.getByte("KeepBlockedMsg", defaultKeepBlockedMsg)) {
		if (dbei->eventType == EVENTTYPE_AUTHREQUEST) {
			// Save the request to database so that it can be automatically submitted on user approval
			uint8_t *eventdata = (uint8_t*)malloc(sizeof(uint32_t) + dbei->cbBlob);
			if (eventdata != nullptr && dbei->cbBlob > 0) {
				memcpy(eventdata, &dbei->cbBlob, sizeof(uint32_t));
				memcpy(eventdata + sizeof(uint32_t), dbei->pBlob, dbei->cbBlob);
				db_set_blob(hContact, MODULENAME, "AuthEvent", eventdata, sizeof(uint32_t) + dbei->cbBlob);
				g_plugin.setString(hContact, "AuthEventModule", dbei->szModule);
				g_plugin.setByte(hContact, "AuthEventPending", TRUE);
				free(eventdata);
			}
		}
		else {
			if (g_plugin.getByte("MarkMsgUnreadOnApproval", defaultMarkMsgUnreadOnApproval)) {
				DBVARIANT _dbv;
				uint32_t dbei_size = 3 * sizeof(uint32_t) + sizeof(uint16_t) + dbei->cbBlob + (uint32_t)mir_strlen(dbei->szModule) + 1;
				uint8_t *eventdata = (uint8_t*)malloc(dbei_size);
				uint8_t *pos = eventdata;
				if (eventdata != nullptr && dbei->cbBlob > 0) {
					if (db_get(hContact, MODULENAME, "LastMsgEvents", &_dbv) == 0) {
						eventdata = (uint8_t*)realloc(eventdata, dbei_size + _dbv.cpbVal);
						pos = eventdata;
						memcpy(eventdata, _dbv.pbVal, _dbv.cpbVal);
						pos += _dbv.cpbVal;
						db_free(&_dbv);
					}
					memcpy(pos, &dbei->eventType, sizeof(uint16_t));
					memcpy(pos + sizeof(uint16_t), &dbei->flags, sizeof(uint32_t));
					memcpy(pos + sizeof(uint16_t) + sizeof(uint32_t), &dbei->timestamp, sizeof(uint32_t));
					memcpy(pos + sizeof(uint16_t) + sizeof(uint32_t) * 2, dbei->szModule, mir_strlen(dbei->szModule) + 1);
					memcpy(pos + sizeof(uint16_t) + sizeof(uint32_t) * 2 + mir_strlen(dbei->szModule) + 1, &dbei->cbBlob, sizeof(uint32_t));
					memcpy(pos + sizeof(uint16_t) + sizeof(uint32_t) * 3 + mir_strlen(dbei->szModule) + 1, dbei->pBlob, dbei->cbBlob);
					db_set_blob(hContact, MODULENAME, "LastMsgEvents", eventdata, (pos - eventdata) + dbei_size);
					free(eventdata);
				}

			}
			else {
				dbei->flags |= DBEF_READ;
				db_event_add(hContact, dbei);
			}
		}
	}
	return 1;
}

/* Removes NotOnList settings from contacts with Delete setting present */
void RemoveNotOnListSettings()
{
	DBVARIANT dbv;
	char protoName[256] = { 0 };
	mir_strcpy(protoName, "proto_");
	for (auto &hContact : Contacts()) {
		if (db_get_s(hContact, "Protocol", "p", &dbv) == 0) {
			mir_strcat(protoName, dbv.pszVal);
			if (g_plugin.getByte(protoName, 0) != 0) {
				if (db_get_b(hContact, "CList", "Delete", 0) == 1) {
					Contact::PutOnList(hContact);
				}
			}
			db_free(&dbv);
		}
		protoName[6] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	srand((unsigned)time(0));
	bayesdb = nullptr;
	if (g_plugin.getByte("BayesEnabled", defaultBayesEnabled)) {
		if (CheckBayes()) {
			OpenBayes();
			if (g_plugin.getByte("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved)) {
				dequeue_messages();
				last_queue_check = time(0);
			}
		}
	}

	HookEvent(ME_DB_EVENT_FILTER_ADD, OnDBEventFilterAdd);
	HookEvent(ME_OPT_INITIALISE, OnOptInitialize);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	RemoveNotOnListSettings();
	return 0;
}
