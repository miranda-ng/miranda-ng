#pragma once

const char *getName(const TD::usernames *pName);

TD::int53 getReplyId(const TD::MessageReplyTo *pReply);
TD::int53 getThreadId(const TD::MessageTopic *pTopic);

TD::object_ptr<TD::inputFileLocal> makeFile(const wchar_t *pwszFilename);

TD::object_ptr<TD::formattedText> formatBbcodes(const char *pszText);

TG_FILE_REQUEST::Type AutoDetectType(const wchar_t *pwszFilename);

CMStringA msg2id(const TD::message *pMsg);
CMStringA msg2id(TD::int53 chatId, TD::int53 msgId);
TD::int53 dbei2id(const DBEVENTINFO &dbei);
