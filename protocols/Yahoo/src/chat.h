/*
 * $Id: chat.h 9334 2009-04-04 21:56:36Z gena01 $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#ifndef _YAHOO_CHAT_H_
#define _YAHOO_CHAT_H_

/* Conference handlers */
void ext_yahoo_got_conf_invite(int id, const char *me, const char *who, const char *room, const char *msg, YList *members);

void ext_yahoo_conf_userdecline(int id, const char *me, const char *who, const char *room, const char *msg);

void ext_yahoo_conf_userjoin(int id, const char *me, const char *who, const char *room);

void ext_yahoo_conf_userleave(int id, const char *me, const char *who, const char *room);

void ext_yahoo_conf_message(int id, const char *me, const char *who, const char *room, const char *msg, int utf8);

/* chat handlers */
void ext_yahoo_chat_cat_xml(int id, const char *xml);

void ext_yahoo_chat_join(int id, const char *me, const char *room, const char * topic, YList *members, INT_PTR fd);

void ext_yahoo_chat_userjoin(int id, const char *me, const char *room, struct yahoo_chat_member *who);

void ext_yahoo_chat_userleave(int id, const char *me, const char *room, const char *who);

void ext_yahoo_chat_message(int id, const char *me, const char *who, const char *room, const char *msg, int msgtype, int utf8);

void ext_yahoo_chat_yahoologout(int id, const char *me);

void ext_yahoo_chat_yahooerror(int id, const char *me);

#endif
