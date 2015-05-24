/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015 Robert Pösel

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
#define MINECRAFTDYNMAP_NAME							"Mincraft Dynmap"

// Limits
#define MINECRAFTDYNMAP_TIMEOUTS_LIMIT					6
// This is just guessed some wise limit
#define MINECRAFTDYNMAP_MESSAGE_LIMIT					256
#define MINECRAFTDYNMAP_MESSAGE_LIMIT_TEXT				"256"

#define MINECRAFTDYNMAP_QUESTION_MIN_LENGTH				10

// Request types
#define MINECRAFTDYNMAP_REQUEST_HOME					100 // getting server homepage
#define MINECRAFTDYNMAP_REQUEST_CONFIGURATION			101 // getting server configuration
#define MINECRAFTDYNMAP_REQUEST_EVENTS			        102 // receiving events
#define MINECRAFTDYNMAP_REQUEST_MESSAGE			        103 // sending messages

// DB settings
#define MINECRAFTDYNMAP_KEY_TIMEOUTS_LIMIT	"TimeoutsLimit" // [HIDDEN]

#define MINECRAFTDYNMAP_KEY_NAME			"Nick"
#define MINECRAFTDYNMAP_KEY_SERVER			"Server"
