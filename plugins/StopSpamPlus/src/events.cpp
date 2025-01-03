#include "stdafx.h"

static std::map<MCONTACT, time_t> g_times;

int OnDbEventAdded(WPARAM, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;

	DB::EventInfo dbei(hDbEvent);
	if (!dbei)
		return 0;

	// if event is in protocol that is not despammed
	if (g_plugin.ProtoDisabled(dbei.szModule))
		return 0;

	// event is an auth request
	if (!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST) {
		MCONTACT hContact = DbGetAuthEventContact(&dbei);

		// if request is from unknown or not marked Answered contact
		//and if I don't sent message to this contact
		if (!Contact::OnList(hContact) && !g_plugin.getByte(hContact, DB_KEY_ANSWERED) && !g_plugin.getByte(hContact, DB_KEY_HASSENT)) {
			if (!g_plugin.bHandleAuthReq)
				ProtoChainSend(hContact, PSS_MESSAGE, 0, ptrA(mir_utf8encodeW(variables_parse(g_plugin.getReply(), hContact).c_str())));

			if (g_plugin.iAnswerTimeout)
				g_times[hContact] = time(0);

			g_plugin.setDword(hContact, DB_KEY_HASAUTH, hDbEvent);
			Contact::RemoveFromList(hContact);
			Contact::Hide(hContact);
			if (!g_plugin.bHistLog)
				db_event_delete(hDbEvent);
			return 1;
		}
	}
	return 0;
}

int OnDbEventFilterAdd(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	DBEVENTINFO *dbei = (DBEVENTINFO *)l;
	if (dbei == nullptr || dbei->szModule == nullptr) //fix potential DEP crash
		return 0;

	// if event is in protocol that is not despammed
	if (g_plugin.ProtoDisabled(dbei->szModule))
		return 0; // ...let the event go its way

	// if event is not a message, or if the message has been read or sent...
	if (dbei->eventType != EVENTTYPE_MESSAGE || (dbei->flags & DBEF_READ) != 0)
		return 0;

	if (dbei->flags & DBEF_SENT) {
		g_plugin.setByte(hContact, DB_KEY_HASSENT, 1);
		return 0;
	}

	// if message is from known or marked Answered contact
	if (g_plugin.getByte(hContact, DB_KEY_ANSWERED) || g_plugin.getByte(hContact, DB_KEY_HASSENT))
		return 0;

	// checking if message from self-added contact
	if (Contact::OnList(hContact))
		return 0;

	// if message is corrupted or empty it cannot be an answer.
	if (!dbei->cbBlob || !dbei->pBlob)
		// reject processing of the event
		return 1;

	tstring message;

	if (dbei->flags & DBEF_UTF) {
		wchar_t *msg_u = mir_utf8decodeW((char *)dbei->pBlob);
		message = msg_u;
		mir_free(msg_u);
	}
	else {
		wchar_t *msg_u = mir_a2u((char *)(dbei->pBlob));
		message = msg_u;
		mir_free(msg_u);
	}

	// if message equal right answer...
	tstring answers = variables_parse(g_plugin.Answer, hContact);
	answers.append(g_plugin.AnswSplitString);
	tstring::size_type pos = 0;
	tstring::size_type prev_pos = 0;
	while ((pos = answers.find(g_plugin.AnswSplitString, pos)) != tstring::npos) {
		// get one of answers and trim witespace chars
		tstring answer = trim(answers.substr(prev_pos, pos - prev_pos));
		// if answer not empty
		if (answer.length() > 0) {
			// if message equal right answer...
			if (g_plugin.bAnswNotCaseSens ? !mir_wstrcmpi(message.c_str(), answer.c_str()) : !mir_wstrcmp(message.c_str(), answer.c_str())) {
				// unhide contact
				Contact::Hide(hContact, false);

				// add contact permanently and delete our temporary variables
				if (g_plugin.bAddPermanent) {
					Contact::PutOnList(hContact);
					db_delete_module(hContact, MODULENAME);
				}
				else g_plugin.setByte(hContact, DB_KEY_ANSWERED, 1);

				// send congratulation
				char *buf = mir_utf8encodeW(variables_parse(g_plugin.getCongrats(), hContact).c_str());
				ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)buf);
				mir_free(buf);

				// process the event
				return 1;
			}
		}
		prev_pos = ++pos;
	}

	// if message message does not contain infintite talk protection prefix
	// and question count for this contact is less then maximum
	const wchar_t *pwszPrefix = TranslateT("StopSpam automatic message:\r\n");
	if ((!g_plugin.bInfTalkProtection || tstring::npos == message.find(pwszPrefix)) && (!g_plugin.iMaxQuestCount || g_plugin.getDword(hContact, DB_KEY_QUESTCOUNT) < g_plugin.iMaxQuestCount)) {
		// send question
		tstring q = pwszPrefix + variables_parse(g_plugin.getQuestion(), hContact);

		char *buf = mir_utf8encodeW(q.c_str());
		ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)buf);
		mir_free(buf);

		// increment question count
		uint32_t questCount = g_plugin.getDword(hContact, DB_KEY_QUESTCOUNT);
		g_plugin.setDword(hContact, DB_KEY_QUESTCOUNT, questCount + 1);
	}

	// hide contact from contact list
	Contact::RemoveFromList(hContact);
	Contact::Hide(hContact);

	// mark message as read and allow to insert it into the history
	dbei->flags |= DBEF_READ;
	return 0;
}

void CMPlugin::Impl::OnTimer(CTimer *)
{
	int iTimeout = g_plugin.iAnswerTimeout;
	if (!iTimeout)
		return;

	time_t now = time(0);

	for (auto &it : g_times) {
		if (now - it.second < iTimeout * 60)
			continue;

		if (MEVENT hDbEvent = g_plugin.getDword(it.first, DB_KEY_HASAUTH)) {
			char *szProto = Proto_GetBaseAccountName(it.first);
			CallProtoService(szProto, PS_AUTHDENY, hDbEvent, (LPARAM)_T2A(variables_parse(g_plugin.getReply(), it.first).c_str()));

			Netlib_Logf(0, "StopSpam: removing temporary contact %d", it.first);
			db_delete_contact(it.first);
		}
	}
}

int OnShutdown(WPARAM, LPARAM)
{
	for (auto &cc : Contacts()) {
		if (Contact::OnList(cc))
			continue;

		if (MEVENT hDbEvent = g_plugin.getDword(cc, DB_KEY_HASAUTH)) {
			char *szProto = Proto_GetBaseAccountName(cc);
			CallProtoService(szProto, PS_AUTHDENY, hDbEvent, (LPARAM)_T2A(variables_parse(g_plugin.getReply(), cc).c_str()));
		}
	}
	
	return 0;
}