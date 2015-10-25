/*
Copyright (c) 2014 Robert P�sel

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

struct CDummyProto;
extern LIST<CDummyProto> dummy_Instances;
extern HINSTANCE hInst;

#define DUMMY_ID_TEMPLATE "Template"
#define DUMMY_ID_TEXT "UniqueIdText"
#define DUMMY_ID_SETTING "UniqueIdSetting"

typedef struct {
	const char *name;
	const char *setting;
	const char *text;
} ttemplate;

static const ttemplate templates[] = {
		{ LPGEN("Custom"), "", "" },
		{ "AIM", "SN", LPGEN("Screen Name") },
		{ "EmLAN", "Nick", LPGEN("User name") },
		{ "Facebook", "ID", LPGEN("Facebook ID") },
		{ "GG", "UIN", LPGEN("Gadu-Gadu Number") },
		{ "ICQ", "UIN", LPGEN("User ID") },
		{ "ICQCorp", "UIN", LPGEN("ICQ number") },
		{ "IRC", "Nick", LPGEN("Nickname") },
		{ "Jabber", "jid", LPGEN("JID") },
		{ "MinecraftDynmap", "Nick", LPGEN("Visible name") },
		{ "MRA", "e-mail", LPGEN("E-mail address") },
		{ "MSN", "e-mail", LPGEN("Live ID") },
		{ "Omegle", "nick", LPGEN("Visible name") },
		{ "Sametime", "stid", LPGEN("ID") },
		{ "Skype (SkypeKit)", "sid", LPGEN("Skype name") },
		{ "Skype (Classic)", "Username", LPGEN("Skype name") },
		{ "Skype (Web)", "Username", LPGEN("Skype name") },
		{ "Steam", "SteamID", LPGEN("Steam ID") },
		{ "Tlen", "jid", LPGEN("Tlen login") },
		{ "Tox", "ToxID", LPGEN("Tox ID") },
		{ "Twitter", "Username", LPGEN("Username") },
		{ "VK", "ID", LPGEN("VK ID") },
		{ "WhatsApp", "ID", LPGEN("WhatsApp ID") },
		{ "XFire", "Username", LPGEN("Username") },
		{ "Yahoo", "yahoo_id", LPGEN("ID") },
};
