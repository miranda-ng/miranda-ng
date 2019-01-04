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

// searching
HttpRequest* facebook_client::searchRequest(const char *query, int s, int pn, const char *ssid)
{
	HttpRequest* p = new HttpRequest(REQUEST_GET, FORMAT, "%s/search/", mbasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE);
	p->flags |= NLHRF_REDIRECT;

	p << CHAR_PARAM("q", query)
		<< INT_PARAM("s", s)
		<< INT_PARAM("pn", pn);

	if (mir_strlen(ssid) > 0)
		p << CHAR_PARAM("ssid", ssid);
	
	return p;
}
