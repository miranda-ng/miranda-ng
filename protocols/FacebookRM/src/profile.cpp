/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-19 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////////////
// getting own name, avatar, ...

HttpRequest* facebook_client::homeRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_MOBILE "/profile.php");
	p->flags |= NLHRF_REDIRECT;
	p << CHAR_PARAM("v", "info");
	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// getting fb_dtsg

HttpRequest* facebook_client::dtsgRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_MOBILE "/profile/basic/intro/bio/");
	p->flags |= NLHRF_REDIRECT;
	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// request mobile page containing profile picture

HttpRequest* facebook_client::profilePictureRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FORMAT, "%s/profile/picture/view/", mbasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE);
	p->flags |= NLHRF_REDIRECT;
	p << CHAR_PARAM("profile_id", userId);
	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// request mobile page containing user profile

HttpRequest* facebook_client::profileRequest(const char *data)
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FORMAT, "%s/%s", mbasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE, data);
	p << CHAR_PARAM("v", "info");
	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// request mobile page containing user profile by his id, and in english language (for parsing data)

HttpRequest* facebook_client::profileInfoRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FORMAT, "%s/profile.php", mbasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE);
	p << CHAR_PARAM("id", userId) << CHAR_PARAM("v", "info") << CHAR_PARAM("locale", "en_US");
	return p;
}
