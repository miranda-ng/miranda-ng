#include "stdafx.h"

int OnDbEventAdded(WPARAM, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;

	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei))
		return 0;

	// if event is in protocol that is not despammed
	if (g_sets.ProtoDisabled(dbei.szModule))
		return 0;

	// event is an auth request
	if (!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST) {
		MCONTACT hcntct = DbGetAuthEventContact(&dbei);

		// if request is from unknown or not marked Answered contact
		//and if I don't sent message to this contact
		if (!Contact::OnList(hcntct) && !g_plugin.getByte(hcntct, DB_KEY_ANSWERED) && !IsExistMyMessage(hcntct)) {
			if (!g_sets.HandleAuthReq) {
				char *buf = mir_utf8encodeW(variables_parse(g_sets.getReply(), hcntct).c_str());
				ProtoChainSend(hcntct, PSS_MESSAGE, 0, (LPARAM)buf);
				mir_free(buf);
			}

			// ...send message
			CallProtoService(dbei.szModule, PS_AUTHDENY, hDbEvent, (LPARAM)_T2A(variables_parse(g_sets.getReply(), hcntct).c_str()));

			Contact::RemoveFromList(hcntct);
			Contact::Hide(hcntct);
			if (!g_sets.HistLog)
				db_event_delete(hDbEvent);
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
	if (g_plugin.getByte(hContact, DB_KEY_ANSWERED))
		// ...let the event go its way
		return 0;

	// checking if message from self-added contact
	//Contact in Not in list icq group
	if (Contact::OnList(hContact) && db_get_w(hContact, dbei->szModule, "SrvGroupId", -1) != 1)
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
		wchar_t* msg_u = mir_utf8decodeW((char*)dbei->pBlob);
		message = msg_u;
		mir_free(msg_u);
	}
	else {
		wchar_t* msg_u = mir_a2u((char*)(dbei->pBlob));
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
				Contact::Hide(hContact, false);

				// mark contact as Answered
				g_plugin.setByte(hContact, DB_KEY_ANSWERED, 1);

				//add contact permanently
				if (g_sets.AddPermanent)
					Contact::PutOnList(hContact);

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
	if ((!g_sets.InfTalkProtection || tstring::npos == message.find(pwszPrefix)) && (!g_sets.MaxQuestCount || g_plugin.getDword(hContact, DB_KEY_QUESTCOUNT) < g_sets.MaxQuestCount)) {
		// send question
		tstring q = pwszPrefix + variables_parse(g_sets.getQuestion(), hContact);


		char * buf = mir_utf8encodeW(q.c_str());
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

int OnDbContactSettingchanged(WPARAM hContact, LPARAM l)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove DB_KEY_ANSWERED
	if (strcmp(cws->szModule, "CList"))
		return 0;
	if (strcmp(cws->szSetting, "NotOnList"))
		return 0;

	if (!cws->value.type) {
		g_plugin.delSetting(hContact, DB_KEY_ANSWERED);
		g_plugin.delSetting(hContact, DB_KEY_QUESTCOUNT);
	}

	return 0;
}
