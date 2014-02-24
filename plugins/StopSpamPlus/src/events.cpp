#include "headers.h"

MIRANDA_HOOK_EVENT(ME_DB_EVENT_ADDED, wParam, lParam)
{
	HANDLE hDbEvent = (HANDLE)lParam;

	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (-1 == dbei.cbBlob)
		return 0;

	mir_ptr<BYTE> blob((LPBYTE)mir_alloc(dbei.cbBlob));
	dbei.pBlob = blob;
	db_event_get(hDbEvent, &dbei);

	// if event is in protocol that is not despammed
	if (plSets->ProtoDisabled(dbei.szModule))
		return 0;

	// event is an auth request
	if (!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST) {
		MCONTACT hcntct = DbGetAuthEventContact(&dbei);

		// if request is from unknown or not marked Answered contact
		//and if I don't sent message to this contact
		if (db_get_b(hcntct, "CList", "NotOnList", 0) && !db_get_b(hcntct, pluginName, answeredSetting, 0) && !IsExistMyMessage(hcntct)) {
			if (!plSets->HandleAuthReq.Get()) {
				char *buf = mir_utf8encodeW(variables_parse(plSets->AuthRepl.Get(), hcntct).c_str());
				CallContactService(hcntct, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
				mir_free(buf);
			}

			// ...send message
			char *AuthRepl = mir_u2a(variables_parse(plSets->AuthRepl.Get(), hcntct).c_str());
			ProtoCallService(dbei.szModule, PS_AUTHDENY, (WPARAM)hDbEvent, (LPARAM)AuthRepl);
			mir_free(AuthRepl);

			db_set_b(hcntct, "CList", "NotOnList", 1);
			db_set_b(hcntct, "CList", "Hidden", 1);
			if (!plSets->HistLog.Get())
				db_event_delete(0, hDbEvent);
			return 1;
		}
	}
	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_EVENT_FILTER_ADD, w, l)
{
	MCONTACT hContact = (MCONTACT)w;
	DBEVENTINFO *dbei = (DBEVENTINFO*)l;
	if (dbei == NULL) //fix potential DEP crash
		return 0;

	// if event is in protocol that is not despammed
	if (plSets->ProtoDisabled(dbei->szModule))
		// ...let the event go its way
		return 0;

	// if event is not a message, or if the message has been read or sent...
	if (dbei->flags & DBEF_SENT || dbei->flags & DBEF_READ || dbei->eventType != EVENTTYPE_MESSAGE )
		// ...let the event go its way
		return 0;

	// if message is from known or marked Answered contact
	if (db_get_b(hContact, pluginName, answeredSetting, 0))
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

	if (dbei->flags & DBEF_UTF){
		WCHAR* msg_u=mir_utf8decodeW((char*)dbei->pBlob);
		message = msg_u;
		mir_free(msg_u);
	}
	else {
		WCHAR* msg_u = mir_a2u((char*)(dbei->pBlob));
		message = msg_u;
		mir_free(msg_u);
	}

	// if message equal right answer...
	tstring answers = variables_parse(plSets->Answer.Get(), hContact);
	answers.append(plSets->AnswSplitString.Get());
	tstring::size_type pos = 0;
	tstring::size_type prev_pos = 0;
	while((pos = answers.find(plSets->AnswSplitString.Get(), pos)) != tstring::npos) {
		// get one of answers and trim witespace chars
		tstring answer = trim(answers.substr(prev_pos, pos - prev_pos));
		// if answer not empty
		if (answer.length() > 0) {
			// if message equal right answer...
			if (plSets->AnswNotCaseSens.Get() ? !lstrcmpi(message.c_str(), answer.c_str()) : !lstrcmp(message.c_str(), answer.c_str())) {
				// unhide contact
				db_unset(hContact, "CList", "Hidden");

				// mark contact as Answered
				db_set_b(hContact, pluginName, answeredSetting, 1);

				//add contact permanently
				if (plSets->AddPermanent.Get())
					db_unset(hContact, "CList", "NotOnList");

				// send congratulation

				char * buf=mir_utf8encodeW(variables_parse(plSets->Congratulation.Get(), hContact).c_str());
				CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
				mir_free(buf);

				// process the event
				return 1;
			}
		}
		prev_pos = ++pos;
	}

	// if message message does not contain infintite talk protection prefix
	// and question count for this contact is less then maximum
	if ((!plSets->InfTalkProtection.Get() || tstring::npos == message.find(infTalkProtPrefix))
		&& (!plSets->MaxQuestCount.Get() || db_get_dw(hContact, pluginName, questCountSetting, 0) < plSets->MaxQuestCount.Get()))
	{
		// send question
		tstring q = infTalkProtPrefix + variables_parse((tstring)(plSets->Question), hContact);


		char * buf=mir_utf8encodeW(q.c_str());
		CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
		mir_free(buf);


		// increment question count
		DWORD questCount = db_get_dw(hContact, pluginName, questCountSetting, 0);
		db_set_dw(hContact, pluginName, questCountSetting, questCount + 1);
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

MIRANDA_HOOK_EVENT(ME_OPT_INITIALISE, w, l)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.pszGroup = LPGEN("Message Sessions");
	odp.pszTitle = pluginName;
	odp.position = -1;
	odp.hInstance = hInst;

	odp.pszTab = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN);
	odp.pfnDlgProc = MainDlgProc;
	Options_AddPage(w, &odp);

	odp.pszTab = LPGEN("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MESSAGES);
	odp.pfnDlgProc = MessagesDlgProc;
	Options_AddPage(w, &odp);

	odp.pszTab = LPGEN("Accounts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PROTO);
	odp.pfnDlgProc = ProtoDlgProc;
	Options_AddPage(w, &odp);
	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_CONTACT_SETTINGCHANGED, hContact, l)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove answeredSetting
	if (strcmp(cws->szModule, "CList"))
		return 0;
	if (strcmp(cws->szSetting, "NotOnList"))
		return 0;

	if (!cws->value.type) {
		db_unset(hContact, pluginName, answeredSetting);
		db_unset(hContact, pluginName, questCountSetting);
	}

	return 0;
}
