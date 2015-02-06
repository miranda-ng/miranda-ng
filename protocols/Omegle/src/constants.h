/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

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

#pragma once

// Product management
#define OMEGLE_NAME               "Omegle"
#define OMEGLE_SERVER_REGULAR     "http://www.omegle.com"
#define OMEGLE_SERVER_CHAT        "http://%s.omegle.com"

// Limits
#define OMEGLE_TIMEOUTS_LIMIT		6
// There is "no limit" on Omegle, but we should set some wise limit
#define OMEGLE_MESSAGE_LIMIT		4096
#define OMEGLE_MESSAGE_LIMIT_TEXT	"4096"

#define OMEGLE_QUESTION_MIN_LENGTH	10

// Omegle request types
#define OMEGLE_REQUEST_HOME						100 // getting server name
#define OMEGLE_REQUEST_COUNT					105 // get count of connected users
#define OMEGLE_REQUEST_START					110 // starting conversation
#define OMEGLE_REQUEST_STOP						120 // ending conversation
#define OMEGLE_REQUEST_SEND						300 // sending message
#define OMEGLE_REQUEST_EVENTS			        301 // receiving events
#define OMEGLE_REQUEST_TYPING_START				310 // started typing
#define OMEGLE_REQUEST_TYPING_STOP				311 // stoped typing
#define OMEGLE_REQUEST_RECAPTCHA				400 // recaptcha handling

// Servers list
//static const char *servers[] = {"Random", "bajor", "cardassia", "promenade", "odo-bucket", "ferengi", "quilt", "empok-nor", "quibbler"};
static const char *servers[] = {LPGEN("Random"), "front1", "front2", "front3", "front4", "front5", "front6", "front7", "front8", "front9"};

static const struct {
	const char *id;
	const char *lang;
} languages[] = {
	{"en", "English"},
	{"af", "Afrikaans"},
	{"sq", "Albanian"},
	{"ar", "Arabic"},
	{"hy", "Armenian"},
	{"az", "Azerbaijani"},
	{"eu", "Basque"},
	{"be", "Belarusian"},
	{"bn", "Bengali"},
	{"bs", "Bosnian"},
	{"bg", "Bulgarian"},
	{"ceb", "Cebuanese"},
	{"cs", "Czech"},
	{"zh-CN", "Chinese (simplified)"},
	{"zh-TW", "Chinese (traditional)"},
	{"da", "Danish"},
	{"eo", "Esperanto"},
	{"et", "Estonian"},
	{"tl", "Philipino"},
	{"fi", "Finnish"},
	{"fr", "French"},
	{"gl", "Galician"},
	{"ka", "Georgian"},
	{"gu", "Gujarati"},
	{"ht", "Haitian Creole"},
	{"iw", "Hebrew"},
	{"hi", "Hindi"},
	{"hmn", "Hmong"},
	{"nl", "Dutch"},
	{"hr", "Croat"},
	{"id", "Indonesian"},
	{"ga", "Irish"},
	{"is", "Icelandic"},
	{"it", "Italian"},
	{"ja", "Japanese"},
	{"jw", "Javanese"},
	{"yi", "Yiddish"},
	{"kn", "Kannada"},
	{"ca", "Catalan"},
	{"km", "Khmer"},
	{"ko", "Korean"},
	{"lo", "Lao"},
	{"la", "Latina"},
	{"lt", "Lithuanian"},
	{"lv", "Latvian"},
	{"hu", "Hungarian"},
	{"mk", "Macedonian"},
	{"ms", "Malay"},
	{"mt", "Maltese"},
	{"mr", "Marathi"},
	{"de", "German"},
	{"no", "Norwegian"},
	{"fa", "Persian"},
	{"pl", "Polish"},
	{"pt", "Portugese"},
	{"ro", "Romañian"},
	{"ru", "Russian"},
	{"el", "Greek"},
	{"sk", "Slovak"},
	{"sl", "Slovenian"},
	{"sr", "Serbian"},
	{"sw", "Swahili"},
	{"es", "Spanish"},
	{"sv", "Swedish"},
	{"ta", "Tamil"},
	{"te", "Telugu"},
	{"th", "Thai"},
	{"tr", "Turkish"},
	{"uk", "Ukrainian"},
	{"ur", "Urdu"},
	{"cy", "Welsh"},
	{"vi", "Vietnamese"}
};

// Known server messages, only to inform lpgen
static const char *server_messages[] = {
	LPGEN("Stranger is using Omegle's mobile Web site (omegle.com on a phone or tablet)"),
	LPGEN("You both speak the same language.")
};
