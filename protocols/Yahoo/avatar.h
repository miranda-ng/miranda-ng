/*
 * $Id: avatar.h 8461 2008-10-23 14:39:20Z gena01 $
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
#ifndef _YAHOO_AVATAR_H_
#define _YAHOO_AVATAR_H_

#include <m_avatars.h>

void YAHOO_request_avatar(const char* who);
void GetAvatarFileName(HANDLE hContact, char* pszDest, int cbLen, int type);

void YAHOO_SendAvatar(const char *szFile);

void YAHOO_set_avatar(int buddy_icon);

int YAHOO_SaveBitmapAsAvatar( HBITMAP hBitmap, const char* szFileName );

HBITMAP YAHOO_StretchBitmap( HBITMAP hBitmap );

void yahoo_reset_avatar(HANDLE 	hContact);

HBITMAP YAHOO_SetAvatar(const char *szFile);

void YAHOO_get_avatar(const char *who, const char *pic_url, long cksum);

/**
 * AVS Services - loadavatars.dll uses these to get the info from us
 */
int YahooGetAvatarCaps(WPARAM wParam, LPARAM lParam);

int YahooGetAvatarInfo(WPARAM wParam,LPARAM lParam);

int YahooGetMyAvatar(WPARAM wParam, LPARAM lParam);

int YahooSetMyAvatar(WPARAM wParam, LPARAM lParam);

/**
 * Callbacks for libyahoo2
 */
void ext_yahoo_got_picture(int id, const char *me, const char *who, const char *pic_url, int cksum, int type);

void ext_yahoo_got_picture_checksum(int id, const char *me, const char *who, int cksum);

void ext_yahoo_got_picture_update(int id, const char *me, const char *who, int buddy_icon);

void ext_yahoo_got_picture_status(int id, const char *me, const char *who, int buddy_icon);

void ext_yahoo_got_picture_upload(int id, const char *me, const char *url,unsigned int ts);

void ext_yahoo_got_avatar_share(int id, int buddy_icon);
#endif
