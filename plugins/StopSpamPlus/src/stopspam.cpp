#include "stdafx.h"

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {553811EE-DEB6-48B8-8902-A8A00C1FD679}
	{ 0x553811ee, 0xdeb6, 0x48b8, { 0x89, 0x2, 0xa8, 0xa0, 0xc, 0x1f, 0xd6, 0x79 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	Question(MODULENAME, "Question"),
	AuthRepl(MODULENAME, "AuthReply"),
	Answer(MODULENAME, "Answer", L"nospam"),
	Congratulation(MODULENAME, "Congratulation"),
	DisabledProtoList(MODULENAME, "DisabledProtoList", "MetaContacts RSSNews"),
	bInfTalkProtection(MODULENAME, "InfTalkProtection", 1),
	bAddPermanent(MODULENAME, "AddPermanent", 0),
	iMaxQuestCount(MODULENAME, "MaxQuestCount", 2),
	iAnswerTimeout(MODULENAME, "AnswerTimeout", 5),
	bAnswNotCaseSens(MODULENAME, "AnswNotCaseSens", 1),
	AnswSplitString(MODULENAME, "AnswSplitString", L"|"),
	bHistLog(MODULENAME, "HistLog", 0)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	m_impl.timerAnswer.Start(60000);

	HookEvent(ME_DB_EVENT_ADDED, OnDbEventAdded);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnDbEventFilterAdd);
	HookEvent(ME_OPT_INITIALISE, OnOptInit);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	return 0;
}

int CMPlugin::Unload()
{
	m_impl.timerAnswer.Stop();
	return 0;
}
