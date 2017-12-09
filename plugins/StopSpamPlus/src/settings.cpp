#include "stdafx.h"

Settings g_sets;

/////////////////////////////////////////////////////////////////////////////////////////

Settings::Settings() :
	Question(pluginName, "Question", TranslateW(L"Spammers made me to install small anti-spam system you are now speaking with. Please reply \"nospam\" without quotes and spaces if you want to contact me.")),
	AuthRepl(pluginName, "AuthReply", TranslateW(L"StopSpam: send a message and reply to an anti-spam bot question.")),
	Answer(pluginName, "Answer", L"nospam"),
	Congratulation(pluginName, "Congratulation", TranslateW(L"Congratulations! You just passed human/robot test. Now you can write me a message.")),
	DisabledProtoList(pluginName, "DisabledProtoList", "MetaContacts RSSNews"),
	InfTalkProtection(pluginName, "InfTalkProtection", 1),
	AddPermanent(pluginName, "AddPermanent", 0),
	HandleAuthReq(pluginName, "HandleAuthReq", 0),
	MaxQuestCount(pluginName, "MaxQuestCount", 2),
	AnswNotCaseSens(pluginName, "AnswNotCaseSens", 1),
	AnswSplitString(pluginName, "AnswSplitString", L"|"),
	RemTmpAll(pluginName, "RemTmpAll", 1),
	HistLog(pluginName, "HistLog", 0)
{
}
