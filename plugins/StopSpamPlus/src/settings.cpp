#include "stdafx.h"

Settings g_sets;

/////////////////////////////////////////////////////////////////////////////////////////

Settings::Settings() :
	Question(MODULENAME, "Question", TranslateW(L"Spammers made me to install small anti-spam system you are now speaking with. Please reply \"nospam\" without quotes and spaces if you want to contact me.")),
	AuthRepl(MODULENAME, "AuthReply", TranslateW(L"StopSpam: send a message and reply to an anti-spam bot question.")),
	Answer(MODULENAME, "Answer", L"nospam"),
	Congratulation(MODULENAME, "Congratulation", TranslateW(L"Congratulations! You just passed human/robot test. Now you can write me a message.")),
	DisabledProtoList(MODULENAME, "DisabledProtoList", "MetaContacts RSSNews"),
	InfTalkProtection(MODULENAME, "InfTalkProtection", 1),
	AddPermanent(MODULENAME, "AddPermanent", 0),
	HandleAuthReq(MODULENAME, "HandleAuthReq", 0),
	MaxQuestCount(MODULENAME, "MaxQuestCount", 2),
	AnswNotCaseSens(MODULENAME, "AnswNotCaseSens", 1),
	AnswSplitString(MODULENAME, "AnswSplitString", L"|"),
	RemTmpAll(MODULENAME, "RemTmpAll", 1),
	HistLog(MODULENAME, "HistLog", 0)
{
}
