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

#include "stdafx.h"

std::string utils::url::encode(const std::string &s)
{
	return (char*)ptrA( mir_urlEncode( s.c_str()));
}

void utils::text::replace_first( std::string* data, const std::string &from, const std::string &to )
{
	std::string::size_type position = 0;

	if ( ( position = data->find(from, position)) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

void utils::text::replace_all( std::string* data, const std::string &from, const std::string &to )
{
	std::string::size_type position = 0;

	while ( ( position = data->find( from, position )) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

void utils::text::treplace_all(std::tstring* data, const std::tstring &from, const std::tstring &to)
{
	std::tstring::size_type position = 0;

	while ((position = data->find(from, position)) != std::tstring::npos)
	{
		data->replace(position, from.size(), to);
		position++;
	}
}


std::string utils::text::special_expressions_decode( std::string data )
{
	utils::text::replace_all( &data, "\\r", "\r" );
	utils::text::replace_all( &data, "\\n", "\n" );
	utils::text::replace_all( &data, "\\\"", "\"" );
	utils::text::replace_all( &data, "\\\\", "\\" );

	return data;
}


std::string utils::text::slashu_to_utf8(const std::string &data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '\\' && (i+1) < data.length( ) && data.at(i+1) == 'u' )
		{
			unsigned int udn = strtol( data.substr( i + 2, 4 ).c_str(), NULL, 16 );

			if ( udn >= 128 && udn <= 2047 )
			{ // U+0080 .. U+07FF
				new_string += ( char )( 192 + ( udn / 64 ));
				new_string += ( char )( 128 + ( udn % 64 ));
			} 
			else if ( udn >= 2048 && udn <= 65535 )
			{ // U+0800 .. U+FFFF
				new_string += ( char )( 224 + ( udn / 4096 ));
				new_string += ( char )( 128 + ( ( udn / 64 ) % 64 ));
				new_string += ( char )( 128 + ( udn % 64  ));
			}
			else if ( udn <= 127 )
			{ // U+0000 .. U+007F (should not appear)
				new_string += ( char )udn;
			}

			i += 5;
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::trim(const std::string &data )
{
	std::string spaces = " \t\r\n";
	std::string::size_type begin = data.find_first_not_of( spaces );
	std::string::size_type end = data.find_last_not_of( spaces ) + 1;

	return (begin != std::string::npos) ? data.substr( begin, end - begin ) : "";
}

int utils::debug::log(const std::string &file_name, const std::string &text)
{
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInstance, szFile, SIZEOF(szFile));
	std::string path = szFile;
	path = path.substr( 0, path.rfind( "\\" ));
	path = path.substr( 0, path.rfind( "\\" ) + 1 );
	path = path + file_name.c_str() + ".txt";

	SYSTEMTIME time;
	GetLocalTime( &time );

	std::ofstream out( path.c_str(), std::ios_base::out | std::ios_base::app | std::ios_base::ate );
	out << "[" << (time.wHour < 10 ? "0" : "") << time.wHour << ":" << (time.wMinute < 10 ? "0" : "") << time.wMinute << ":" << (time.wSecond < 10 ? "0" : "") << time.wSecond << "] " << text << std::endl;
	out.close( );

	return EXIT_SUCCESS;
}

void __fastcall utils::mem::detract(char** str )
{
	utils::mem::detract( ( void** )str );
}

void __fastcall utils::mem::detract(void** p)
{
	utils::mem::detract((void*)(*p));
}

void __fastcall utils::mem::detract(void* p)
{
	mir_free(p);
}

void* __fastcall utils::mem::allocate(size_t size)
{
	return mir_calloc(size);
}
