/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-18 Miranda NG team

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
// refreshing captcha dialog (changing captcha type)

RefreshCaptchaRequest::RefreshCaptchaRequest(facebook_client *fc, const char *captchaPersistData) :
	HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/captcha/refresh_ajax.php")
{
	Url
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("new_captcha_type", "TFBCaptcha")
		<< CHAR_PARAM("skipped_captcha_data", captchaPersistData)
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< CHAR_PARAM("__rev", fc->__rev())
		<< CHAR_PARAM("__user", fc->self_.user_id.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// getting data for given url (for sending/posting reasons)

LinkScraperRequest::LinkScraperRequest(facebook_client *fc, status_data *status) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/composerx/attachment/link/scraper/")
{
	Url
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("composerurihash", 2)
		<< CHAR_PARAM("scrape_url", ptrA(mir_urlEncode(status->url.c_str())));

	Body
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("targetid", status->user_id.empty() ? fc->self_.user_id.c_str() : status->user_id.c_str())
		<< CHAR_PARAM("xhpc_targetid", status->user_id.empty() ? fc->self_.user_id.c_str() : status->user_id.c_str())
		<< INT_PARAM("istimeline", 1)
		<< CHAR_PARAM("composercontext", "composer")
		<< INT_PARAM("onecolumn", 1)
		<< CHAR_PARAM("nctr[_mod]", "pagelet_timeline_recent")
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
		<< CHAR_PARAM("__user", status->isPage && !status->user_id.empty() ? status->user_id.c_str() : fc->self_.user_id.c_str())
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
}

//////////////////////////////////////////////////////////////////////////////////////////
// changing identity to post status for pages

SwitchIdentityRequest::SwitchIdentityRequest(const char *dtsg, const char *userId) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/identity_switch.php")
{
	Url << INT_PARAM("__a", 1);

	Body << CHAR_PARAM("fb_dtsg", dtsg) << CHAR_PARAM("user_id", userId) << CHAR_PARAM("url", FACEBOOK_URL_HOMEPAGE);
}

//////////////////////////////////////////////////////////////////////////////////////////
// posting status to our or friends's wall

SharePostRequest::SharePostRequest(facebook_client *fc, status_data *status, const char *linkData) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/updatestatus.php")
{
	Url << INT_PARAM("__a", 1);

	ptrA text(mir_urlEncode(status->text.c_str()));

	Body
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
		<< CHAR_PARAM("__user", status->isPage && !status->user_id.empty() ? status->user_id.c_str() : fc->self_.user_id.c_str())
		<< CHAR_PARAM("xhpc_targetid", status->user_id.empty() ? fc->self_.user_id.c_str() : status->user_id.c_str())
		<< CHAR_PARAM("xhpc_message", text)
		<< CHAR_PARAM("xhpc_message_text", text)
		<< CHAR_PARAM("xhpc_context", "profile")
		<< INT_PARAM("xhpc_ismeta", 1)
		<< INT_PARAM("xhpc_timeline", 1)
		<< CHAR_PARAM("xhpc_composerid", "u_0_2y")
		<< BOOL_PARAM("disable_location_sharing", false)
		<< CHAR_PARAM("nctr[_mod]", "pagelet_composer");

	if (!status->isPage)
		Body << CHAR_PARAM("audience[0][value]", fc->get_privacy_type().c_str());

	if (!status->place.empty())
		Body << CHAR_PARAM("composertags_place_name", ptrA(mir_urlEncode(status->place.c_str())));

	// Status with users
	for (std::vector<facebook_user*>::size_type i = 0; i < status->users.size(); i++) {
		CMStringA withId(::FORMAT, "composertags_with[%i]", i);
		CMStringA withName(::FORMAT, "text_composertags_with[%i]", i);

		Body
			<< CHAR_PARAM(withId.c_str(), status->users[i]->user_id.c_str())
			<< CHAR_PARAM(withName.c_str(), status->users[i]->real_name.c_str());
	}

	// Link attachment
	if (mir_strlen(linkData) > 0)
		Body << linkData;
}

//////////////////////////////////////////////////////////////////////////////////////////
// sending pokes

SendPokeRequest::SendPokeRequest(facebook_client *fc, const char *userId) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/pokes/dialog/")
{
	Url << INT_PARAM("__a", 1);

	Body
		<< INT_PARAM("do_confirm", 0)
		<< CHAR_PARAM("poke_target", userId)
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
		<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str());
}
