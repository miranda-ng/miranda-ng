#ifndef _XFIRESERVICES_H
#define _XFIRESERVICES_H

#define XFIRE_URLCALL "XFire/UrlCall"

#define XFIRE_SET_NICK "XFire/SetNickname"

#define XFIRE_SEND_PREFS "XFire/SendXFirePrefs"

#define XFIRE_INGAMESTATUSHOOK "XFire/Gamestatus"

#define MBOT_TRIGGER "MBot/Trigger"

//liefert den eigenen avatar zurück
INT_PTR GetMyAvatar(WPARAM wparam, LPARAM lparam);
int mBotNotify(WPARAM wparam, LPARAM lparam);

#endif