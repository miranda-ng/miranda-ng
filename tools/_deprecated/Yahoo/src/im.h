/*
 * $Id: im.h 8594 2008-11-25 14:05:22Z gena01 $
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
#ifndef _YAHOO_IM_H_
#define _YAHOO_IM_H_

void ext_yahoo_got_im(int id, const char *me, const char *who, int protocol, 
					const char *msg, long tm, int stat, int utf8, int buddy_icon,
					const char *seqn, int sendn);

int YahooSendNudge(WPARAM wParam, LPARAM lParam);
int YahooRecvMessage(WPARAM wParam, LPARAM lParam);
int YahooSendMessageW(WPARAM wParam, LPARAM lParam);
int YahooSendMessage(WPARAM wParam, LPARAM lParam);

#endif
