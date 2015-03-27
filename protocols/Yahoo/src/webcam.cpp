/*
 * $Id: webcam.cpp 9228 2009-03-26 13:47:12Z ghazan $
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
#include <time.h>
#include <sys/stat.h>
#include <malloc.h>
#include <io.h>

/*
 * Miranda headers
 */
#include "yahoo.h"
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_message.h>

/* WEBCAM callbacks */
void ext_yahoo_got_webcam_image(int id, const char *who,
		const unsigned char *image, unsigned int image_size, unsigned int real_size,
		unsigned int timestamp)
{
	LOG(("ext_yahoo_got_webcam_image"));
}

void ext_yahoo_webcam_viewer(int id, const char *who, int connect)
{
	LOG(("ext_yahoo_webcam_viewer"));
}

void ext_yahoo_webcam_closed(int id, const char *who, int reason)
{
	LOG(("ext_yahoo_webcam_closed"));
}

void ext_yahoo_webcam_data_request(int id, int send)
{
	LOG(("ext_yahoo_webcam_data_request"));
}

void ext_yahoo_webcam_invite(int id, const char *me, const char *from)
{
	LOG(("ext_yahoo_webcam_invite"));

	GETPROTOBYID( id )->ext_got_im(me, from, 0, Translate("[miranda] Got webcam invite. (not currently supported)"), 0, 0, 0, -1);
}

void ext_yahoo_webcam_invite_reply(int id, const char *me, const char *from, int accept)
{
	LOG(("ext_yahoo_webcam_invite_reply"));
}
