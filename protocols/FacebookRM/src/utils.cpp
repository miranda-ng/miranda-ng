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
// getting owned/admined pages list

HttpRequest* facebook_client::getPagesRequest()
{
	return new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/bookmarks/pages");
}

//////////////////////////////////////////////////////////////////////////////////////////
// refreshing captcha dialog (changing captcha type)

HttpRequest* facebook_client::refreshCaptchaRequest(const char *captchaPersistData)
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/captcha/refresh_ajax.php");

	p << INT_PARAM("__a", 1)
		<< CHAR_PARAM("new_captcha_type", "TFBCaptcha")
		<< CHAR_PARAM("skipped_captcha_data", captchaPersistData)
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("__user", self_.user_id.c_str());

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// getting data for given url (for sending/posting reasons)

HttpRequest* facebook_client::linkScraperRequest(status_data *status)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/composerx/attachment/link/scraper/");

	p << INT_PARAM("__a", 1)
		<< INT_PARAM("composerurihash", 2)
		<< CHAR_PARAM("scrape_url", status->url.c_str());

	p->Body
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("targetid", status->user_id.empty() ? self_.user_id.c_str() : status->user_id.c_str())
		<< CHAR_PARAM("xhpc_targetid", status->user_id.empty() ? self_.user_id.c_str() : status->user_id.c_str())
		<< INT_PARAM("istimeline", 1)
		<< CHAR_PARAM("composercontext", "composer")
		<< INT_PARAM("onecolumn", 1)
		<< CHAR_PARAM("nctr[_mod]", "pagelet_timeline_recent")
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__user", status->isPage && !status->user_id.empty() ? status->user_id.c_str() : self_.user_id.c_str())
		<< CHAR_PARAM("loaded_components[0]", "maininput")
		<< CHAR_PARAM("loaded_components[1]", "backdateicon")
		<< CHAR_PARAM("loaded_components[2]", "withtaggericon")
		<< CHAR_PARAM("loaded_components[3]", "cameraicon")
		<< CHAR_PARAM("loaded_components[4]", "placetaggericon")
		<< CHAR_PARAM("loaded_components[5]", "mainprivacywidget")
		<< CHAR_PARAM("loaded_components[6]", "withtaggericon")
		<< CHAR_PARAM("loaded_components[7]", "backdateicon")
		<< CHAR_PARAM("loaded_components[8]", "placetaggericon")
		<< CHAR_PARAM("loaded_components[9]", "cameraicon")
		<< CHAR_PARAM("loaded_components[10]", "mainprivacywidget")
		<< CHAR_PARAM("loaded_components[11]", "maininput")
		<< CHAR_PARAM("loaded_components[12]", "explicitplaceinput")
		<< CHAR_PARAM("loaded_components[13]", "hiddenplaceinput")
		<< CHAR_PARAM("loaded_components[14]", "placenameinput")
		<< CHAR_PARAM("loaded_components[15]", "hiddensessionid")
		<< CHAR_PARAM("loaded_components[16]", "withtagger")
		<< CHAR_PARAM("loaded_components[17]", "backdatepicker")
		<< CHAR_PARAM("loaded_components[18]", "placetagger")
		<< CHAR_PARAM("loaded_components[19]", "citysharericon");

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// changing identity to post status for pages

HttpRequest* facebook_client::switchIdentityRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/identity_switch.php");

	p << INT_PARAM("__a", 1);

	p->Body << CHAR_PARAM("fb_dtsg", dtsg_.c_str()) << CHAR_PARAM("user_id", userId) << CHAR_PARAM("url", FACEBOOK_URL_HOMEPAGE);

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// posting status to our or friends's wall

HttpRequest* facebook_client::sharePostRequest(status_data *status, const char *linkData)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/updatestatus.php");

	p << INT_PARAM("__a", 1);

	p->Body
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__user", status->isPage && !status->user_id.empty() ? status->user_id.c_str() : self_.user_id.c_str())
		<< CHAR_PARAM("xhpc_targetid", status->user_id.empty() ? self_.user_id.c_str() : status->user_id.c_str())
		<< CHAR_PARAM("xhpc_message", status->text.c_str())
		<< CHAR_PARAM("xhpc_message_text", status->text.c_str())
		<< CHAR_PARAM("xhpc_context", "profile")
		<< INT_PARAM("xhpc_ismeta", 1)
		<< INT_PARAM("xhpc_timeline", 1)
		<< CHAR_PARAM("xhpc_composerid", "u_0_2y")
		<< BOOL_PARAM("disable_location_sharing", false)
		<< CHAR_PARAM("nctr[_mod]", "pagelet_composer");

	if (!status->isPage)
		p->Body << CHAR_PARAM("audience[0][value]", get_privacy_type().c_str());

	if (!status->place.empty())
		p->Body << CHAR_PARAM("composertags_place_name", status->place.c_str());

	// Status with users
	for (std::vector<facebook_user*>::size_type i = 0; i < status->users.size(); i++) {
		CMStringA withId(::FORMAT, "composertags_with[%i]", i);
		CMStringA withName(::FORMAT, "text_composertags_with[%i]", i);

		p->Body
			<< CHAR_PARAM(withId.c_str(), status->users[i]->user_id.c_str())
			<< CHAR_PARAM(withName.c_str(), status->users[i]->real_name.c_str());
	}

	// Link attachment
	if (mir_strlen(linkData) > 0)
		p->Body << linkData;

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// sending pokes

HttpRequest* facebook_client::sendPokeRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/pokes/dialog/");

	p << INT_PARAM("__a", 1);

	p->Body
		<< INT_PARAM("do_confirm", 0)
		<< CHAR_PARAM("poke_target", userId)
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str());

	return p;
}
