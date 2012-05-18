/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

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

// Parser front-end

#define lltoa _i64toa

class facebook_json_parser
{
public:
	FacebookProto* proto;
	int parse_buddy_list( void*, List::List< facebook_user >* );
	int parse_friends( void*, std::map< std::string, facebook_user* >* );
	int parse_notifications( void*, std::vector< facebook_notification* >* );
	int parse_messages( void*, std::vector< facebook_message* >*, std::vector< facebook_notification* >* );

	facebook_json_parser( FacebookProto* proto )
	{
		this->proto = proto;
	}
};
