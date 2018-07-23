#include "stdafx.h"

char const *answeredSetting = "Answered";
char const *questCountSetting = "QuestionCount";

int OnDbEventAdded(WPARAM, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;

	DBEVENTINFO dbei = {};
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (-1 == dbei.cbBlob)
		return 0;

	mir_ptr<BYTE> blob((LPBYTE)mir_alloc(dbei.cbBlob));
	dbei.pBlob = blob;
	db_event_get(hDbEvent, &dbei);

	// if event is in protocol that is not despammed
	if (g_sets.ProtoDisabled(dbei.szModule))
		return 0;

	// event is an auth request
	if (!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST) {
		MCONTACT hcntct = DbGetAuthEventContact(&dbei);

		// if request is from unknown or not marked Answered contact
		//and if I don't sent message to this contact
		if (db_get_b(hcntct, "CList", "NotOnList", 0) && !db_get_b(hcntct, MODULENAME, answeredSetting, 0) && !IsExistMyMessage(hcntct)) {
			if (!g_sets.HandleAuthReq) {
				char *buf = mir_utf8encodeW(variables_parse(g_sets.AuthRepl, hcntct).c_str());
				ProtoChainSend(hcntct, PSS_MESSAGE, 0, (LPARAM)buf);
				mir_free(buf);
			}

			// ...send message
			char *AuthRepl = mir_u2a(variables_parse(g_sets.AuthRepl, hcntct).c_str());
			CallProtoService(dbei.szModule, PS_AUTHDENY, hDbEvent, (LPARAM)AuthRepl);
			mir_free(AuthRepl);

			db_set_b(hcntct, "CList", "NotOnList", 1);
			db_set_b(hcntct, "CList", "Hidden", 1);
			if (!g_sets.HistLog)
				db_event_delete(0, hDbEvent);
			return 1;
		}
	}
	return 0;
}

int OnDbEventFilterAdd(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	DBEVENTINFO *dbei = (DBEVENTINFO*)l;
	if (dbei == nullptr || dbei->szModule == nullptr) //fix potential DEP crash
		return 0;

	// if event is in protocol that is not despammed
	if (g_sets.ProtoDisabled(dbei->szModule))
		// ...let the event go its way
		return 0;

	// if event is not a message, or if the message has been read or sent...
	if (dbei->flags & DBEF_SENT || dbei->flags & DBEF_READ || dbei->eventType != EVENTTYPE_MESSAGE)
		// ...let the event go its way
		return 0;

	// if message is from known or marked Answered contact
	if (db_get_b(hContact, MODULENAME, answeredSetting, 0))
		// ...let the event go its way
		return 0;

	// checking if message from self-added contact
	//Contact in Not in list icq group
	if (!db_get_b(hContact, "CList", "NotOnList", 0) && db_get_w(hContact, dbei->szModule, "SrvGroupId", -1) != 1)
		return 0;

	//if I sent message to this contact
	if (IsExistMyMessage(hContact))
		return 0;

	// if message is corrupted or empty it cannot be an answer.
	if (!dbei->cbBlob || !dbei->pBlob)
		// reject processing of the event
		return 1;

	tstring message;

	if (dbei->flags & DBEF_UTF) {
		WCHAR* msg_u = mir_utf8decodeW((char*)dbei->pBlob);
		message = msg_u;
		mir_free(msg_u);
	}
	else {
		WCHAR* msg_u = mir_a2u((char*)(dbei->pBlob));
		message = msg_u;
		mir_free(msg_u);
	}

	// if message equal right answer...
	tstring answers = variables_parse(g_sets.Answer, hContact);
	answers.append(g_sets.AnswSplitString);
	tstring::size_type pos = 0;
	tstring::size_type prev_pos = 0;
	while ((pos = answers.find(g_sets.AnswSplitString, pos)) != tstring::npos) {
		// get one of answers and trim witespace chars
		tstring answer = trim(answers.substr(prev_pos, pos - prev_pos));
		// if answer not empty
		if (answer.length() > 0) {
			// if message equal right answer...
			if (g_sets.AnswNotCaseSens ? !mir_wstrcmpi(message.c_str(), answer.c_str()) : !mir_wstrcmp(message.c_str(), answer.c_str())) {
				// unhide contact
				db_unset(hContact, "CList", "Hidden");

				// mark contact as Answered
				db_set_b(hContact, MODULENAME, answeredSetting, 1);

				//add contact permanently
				if (g_sets.AddPermanent)
					db_unset(hContact, "CList", "NotOnList");

				// send congratulation

				char * buf = mir_utf8encodeW(variables_parse(g_sets.getCongrats(), hContact).c_str());
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
	if ((!g_sets.InfTalkProtection || tstring::npos == message.find(pwszPrefix)) && (!g_sets.MaxQuestCount || db_get_dw(hContact, MODULENAME, questCountSetting, 0) < g_sets.MaxQuestCount)) {
		// send question
		tstring q = pwszPrefix + variables_parse(g_sets.getQuestion(), hContact);


		char * buf = mir_utf8encodeW(q.c_str());
		ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)buf);
		mir_free(buf);


		// increment question count
		DWORD questCount = db_get_dw(hContact, MODULENAME, questCountSetting, 0);
		db_set_dw(hContact, MODULENAME, questCountSetting, questCount + 1);
	}

	// hide contact from contact list
	db_set_b(hContact, "CList", "NotOnList", 1);
	db_set_b(hContact, "CList", "Hidden", 1);

	// save message from contact
	dbei->flags |= DBEF_READ;
	db_event_add(hContact, dbei);

	// reject processing of the event
	return 1;
}

int OnDbContactSettingchanged(WPARAM hContact, LPARAM l)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove answeredSetting
	if (strcmp(cws->szModule, "CList"))
		return 0;
	if (strcmp(cws->szSetting, "NotOnList"))
		return 0;

	if (!cws->value.type) {
		db_unset(hContact, MODULENAME, answeredSetting);
		db_unset(hContact, MODULENAME, questCountSetting);
	}

	return 0;
}
