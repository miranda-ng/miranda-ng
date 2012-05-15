/*
 * $Id: webcam.h 3627 2006-08-28 16:11:15Z gena01 $
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
#ifndef _YAHOO_WEBCAM_H_
#define _YAHOO_WEBCAM_H_

/* WEBCAM callbacks */
void ext_yahoo_got_webcam_image(int id, const char *who,
		const unsigned char *image, unsigned int image_size, unsigned int real_size,
		unsigned int timestamp);

void ext_yahoo_webcam_viewer(int id, const char *who, int connect);

void ext_yahoo_webcam_closed(int id, const char *who, int reason);

void ext_yahoo_webcam_data_request(int id, int send);

void ext_yahoo_webcam_invite(int id, const char *me, const char *from);

void ext_yahoo_webcam_invite_reply(int id, const char *me, const char *from, int accept);

#endif
