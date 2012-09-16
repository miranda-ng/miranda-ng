#include "headers.h"


MIRANDA_HOOK_EVENT(ME_DB_EVENT_ADDED, wParam, lParam)
{
	HANDLE hDbEvent = (HANDLE)lParam;

	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0);
	if (-1 == dbei.cbBlob)
		return 0;

	dbei.pBlob = new BYTE[dbei.cbBlob];
	CallService(MS_DB_EVENT_GET, lParam, (LPARAM)&dbei);

	// if event is in protocol that is not despammed
	if(plSets->ProtoDisabled(dbei.szModule)) {
		delete [] dbei.pBlob;
		return 0;
	}

	// event is an auth request
	if (!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST)
	{
		HANDLE hcntct = DbGetAuthEventContact(&dbei);

		// if request is from unknown or not marked Answered contact
		//and if I don't sent message to this contact

		if(DBGetContactSettingByte(hcntct, "CList", "NotOnList", 0) &&
			!DBGetContactSettingByte(hcntct, pluginName, answeredSetting, 0) &&
			!IsExistMyMessage(hcntct))
		{
			if (!plSets->HandleAuthReq.Get())
			{

					char * buf=mir_utf8encodeW(variables_parse(plSets->AuthRepl.Get(), hcntct).c_str());
					CallContactService(hcntct, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
					mir_free(buf);

			}
			char *AuthRepl;

				AuthRepl=mir_u2a(variables_parse(plSets->AuthRepl.Get(), hcntct).c_str());

			// ...send message
			std::string allowService = dbei.szModule;
			allowService += PS_AUTHDENY;
			CallService(allowService.c_str(), (WPARAM)hDbEvent, (LPARAM)AuthRepl);

				mir_free(AuthRepl);

			DBWriteContactSettingByte(hcntct, "CList", "NotOnList", 1);
			DBWriteContactSettingByte(hcntct, "CList", "Hidden", 1);
			if (!plSets->HistLog.Get())
				CallService(MS_DB_EVENT_DELETE, 0, (LPARAM)hDbEvent);
			delete [] dbei.pBlob;
			return 1;
		}
	}
	delete [] dbei.pBlob;
	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_EVENT_FILTER_ADD, w, l)
{
	HANDLE hContact = (HANDLE)w;

	if (!l) //fix potential DEP crash
		return 0;
	DBEVENTINFO * dbei = (DBEVENTINFO*)l;

	// if event is in protocol that is not despammed
	if(plSets->ProtoDisabled(dbei->szModule))
		// ...let the event go its way
		return 0;

	// if event is not a message, or if the message has been read or sent...
	if(dbei->flags & DBEF_SENT || dbei->flags & DBEF_READ || dbei->eventType != EVENTTYPE_MESSAGE )
		// ...let the event go its way
		return 0;

	// if message is from known or marked Answered contact
	if(DBGetContactSettingByte(hContact, pluginName, answeredSetting, 0))
		// ...let the event go its way
		return 0;

	// checking if message from self-added contact
	//Contact in Not in list icq group
	if (!DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) && DBGetContactSettingWord(hContact, dbei->szModule, "SrvGroupId", -1) != 1)
		return 0;

	//if I sent message to this contact
	if(IsExistMyMessage(hContact))
		return 0;

	// if message is corrupted or empty it cannot be an answer.
	if (!dbei->cbBlob || !dbei->pBlob)
		// reject processing of the event
		return 1;

	tstring message;

	if(dbei->flags & DBEF_UTF){
		WCHAR* msg_u=mir_utf8decodeW((char*)dbei->pBlob);

		message = msg_u;

		mir_free(msg_u);
	}
	else{

		WCHAR* msg_u = mir_a2u((char*)(dbei->pBlob));
		message = msg_u;
		mir_free(msg_u);

	}

	// if message equal right answer...
	tstring answers = variables_parse(plSets->Answer.Get(), hContact);
	answers.append(plSets->AnswSplitString.Get());
	tstring::size_type pos = 0;
	tstring::size_type prev_pos = 0;
	while((pos = answers.find(plSets->AnswSplitString.Get(), pos)) != tstring::npos)
    {
		// get one of answers and trim witespace chars
        tstring answer = trim(answers.substr(prev_pos, pos - prev_pos));
		// if answer not empty
		if (answer.length() > 0)
		{
			// if message equal right answer...
			if (plSets->AnswNotCaseSens.Get() ?
				!lstrcmpi(message.c_str(), answer.c_str()) :
				!lstrcmp(message.c_str(), answer.c_str())
			)
			{
				// unhide contact
				DBDeleteContactSetting(hContact, "CList", "Hidden");

				// mark contact as Answered
				DBWriteContactSettingByte(hContact, pluginName, answeredSetting, 1);

				//add contact permanently
				if(plSets->AddPermanent.Get())
					DBDeleteContactSetting(hContact, "CList", "NotOnList");

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
	if ( (!plSets->InfTalkProtection.Get() || tstring::npos==message.find(infTalkProtPrefix))
		&& (!plSets->MaxQuestCount.Get() || DBGetContactSettingDword(hContact, pluginName, questCountSetting, 0) < plSets->MaxQuestCount.Get()))
	{
		// send question
		tstring q = infTalkProtPrefix + variables_parse((tstring)(plSets->Question), hContact);


		char * buf=mir_utf8encodeW(q.c_str());
		CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
		mir_free(buf);


		// increment question count
		DWORD questCount = DBGetContactSettingDword(hContact, pluginName, questCountSetting, 0);
		DBWriteContactSettingDword(hContact, pluginName, questCountSetting, questCount + 1);

		// hide contact from contact list
	}
	DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
	DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);

	// save message from contact
	dbei->flags |= DBEF_READ;
	CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)dbei);

	// reject processing of the event
	return 1;
}

MIRANDA_HOOK_EVENT(ME_OPT_INITIALISE, w, l)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.pszGroup = LPGEN("Message Sessions");
	odp.pszTitle = pluginName;
	odp.position = -1;
	odp.hInstance = hInst;

	odp.pszTab = LPGEN("Main");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN);
	odp.pfnDlgProc = MainDlgProc;
	Options_AddPage(w, &odp);

	odp.pszTab = LPGEN("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MESSAGES);
	odp.pfnDlgProc = MessagesDlgProc;
	Options_AddPage(w, &odp);

	odp.pszTab = LPGEN("Protocols");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PROTO);
	odp.pfnDlgProc = ProtoDlgProc;
	Options_AddPage(w, &odp);

	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_CONTACT_SETTINGCHANGED, w, l)
{
	HANDLE hContact = (HANDLE)w;
	DBCONTACTWRITESETTING * cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove answeredSetting
	if(strcmp(cws->szModule, "CList"))
		return 0;
	if(strcmp(cws->szSetting, "NotOnList"))
		return 0;
	if (!cws->value.type)
	{
		DBDeleteContactSetting(hContact, pluginName, answeredSetting);
		DBDeleteContactSetting(hContact, pluginName, questCountSetting);
	}

	return 0;
}
