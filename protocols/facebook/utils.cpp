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

#include "common.h"

std::string utils::url::encode(const std::string &s)
{
	char *encoded = reinterpret_cast<char*>(CallService( MS_NETLIB_URLENCODE,
		0,reinterpret_cast<LPARAM>(s.c_str()) ));
	std::string ret = encoded;
	HeapFree(GetProcessHeap(),0,encoded);

	return ret;
}

std::string utils::url::decode(std::string data)
{
	// TODO: Better and universal method?
	utils::text::replace_all( &data, "%2F", "/" );
	utils::text::replace_all( &data, "%3F", "?" );
	utils::text::replace_all( &data, "%3D", "=" );
	utils::text::replace_all( &data, "%26", "&" );

	return data;
}

std::string utils::time::unix_timestamp( )
{
	time_t in = ::time( NULL );
	return utils::conversion::to_string( ( void* )&in, UTILS_CONV_TIME_T );
}

std::string utils::time::mili_timestamp( )
{
	SYSTEMTIME st;
	std::string timestamp = utils::time::unix_timestamp();
	GetSystemTime(&st);
	timestamp.append(utils::conversion::to_string( ( void* )&st.wMilliseconds, UTILS_CONV_UNSIGNED_NUMBER ));
	return timestamp;
}

std::string utils::conversion::to_string( void* data, WORD type )
{
	std::stringstream out;

	switch ( type )
	{
  	case UTILS_CONV_BOOLEAN:
		out << (data ? "true" : "false");

    case UTILS_CONV_TIME_T:
		out << (*( time_t* )data);
		break;

	case UTILS_CONV_SIGNED_NUMBER:
  		out << (*( signed int* )data);
		break;

	case UTILS_CONV_UNSIGNED_NUMBER:
		out << (*( unsigned int* )data);
		break;
	}

	return out.str( );
}

void utils::text::replace_first( std::string* data, std::string from, std::string to )
{
	std::string::size_type position = data->find(from);
	if ( position != std::string::npos )
	{
		data->replace( position, from.size(), to );
	}
}

void utils::text::replace_all( std::string* data, std::string from, std::string to )
{
	std::string::size_type position = 0;

	while ( ( position = data->find( from, position ) ) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

unsigned int utils::text::count_all( std::string* data, std::string term )
{
	unsigned int count = 0;
	std::string::size_type position = 0;

	while ( ( position = data->find( term, position ) ) != std::string::npos )
	{
		count++;
		position++;
	}

	return count;
}

std::string utils::text::special_expressions_decode( std::string data )
{
	utils::text::replace_all( &data, "&amp;", "&" );
	utils::text::replace_all( &data, "&quot;", "\"" );
	utils::text::replace_all( &data, "&#039;", "'" );
	utils::text::replace_all( &data, "&#64;", "@" );
	utils::text::replace_all( &data, "&lt;", "<" );
	utils::text::replace_all( &data, "&gt;", ">" );

	utils::text::replace_all( &data, "&hearts;", "\xE2\x99\xA5" ); // direct byte replacement
//	utils::text::replace_all( &data, "&hearts;", "\\u2665" );      // indirect slashu replacement

	utils::text::replace_all( &data, "\\/", "/" );
	utils::text::replace_all( &data, "\\\\", "\\" );

	// TODO: Add more to comply general usage
	// http://www.utexas.edu/learn/html/spchar.html
	// http://www.webmonkey.com/reference/Special_Characters
	// http://www.degraeve.com/reference/specialcharacters.php
	// http://www.chami.com/tips/internet/050798i.html
	// http://www.w3schools.com/tags/ref_entities.asp
	// http://www.natural-innovations.com/wa/doc-charset.html
	// http://webdesign.about.com/library/bl_htmlcodes.htm
	
	return data;
}

std::string utils::text::edit_html( std::string data )
{
	std::string::size_type end = 0;
	std::string::size_type start = 0;
	std::string new_string = "";
  
	while ( end != std::string::npos )
	{
		end = data.find( "<span class=\\\"text_exposed_hide", start );
		if ( end != std::string::npos )
		{
			new_string += data.substr( start, end - start );
			start = data.find( "<\\/span", end );
		} else {
			new_string += data.substr( start, data.length() - start );
		}
	}

	start = end = 0;
	data = new_string;
	new_string = "";

	while ( end != std::string::npos )
	{
		end = data.find( "<span class=\\\"uiTooltipText", start );
		if ( end != std::string::npos )
		{
			new_string += data.substr( start, end - start );
			start = data.find( "<\\/span", end );
		} else {
			new_string += data.substr( start, data.length() - start );
		}
	}

	// Remove "Translate" link
	start = end = 0;
	data = new_string;
	new_string = "";
	while ( end != std::string::npos )
	{
		end = data.find( "translate_story_link\\\">", start );
		if ( end != std::string::npos )
		{
			new_string += data.substr( start, end - start );
			start = data.find( "<\\/div", end );
		} else {
			new_string += data.substr( start, data.length() - start );
		}
	}

	// Append newline after attachement title
	start = new_string.find( "class=\\\"uiAttachmentTitle", 0 );
	if ( start != std::string::npos )
	{
		data = new_string.substr( 0, start );
		data = utils::text::trim( data );

		start = new_string.find( ">", start );
		if ( start != std::string::npos )
			new_string.insert(start+1, "\n\n");

		start = new_string.find( "<\\/div>", start );
		if ( start != std::string::npos )
			new_string.insert(start, "\n");
	}

	// Append newline between attachement link and description
	start = new_string.find( "uiAttachmentDesc", 0 );
	if ( start != std::string::npos )
	{
		start = new_string.find( ">", start );
		if ( start != std::string::npos )
			new_string.insert(start+1, "\n");

		start = new_string.find( "<\\/div>", start );
		if ( start != std::string::npos )
			new_string.insert(start, "\n");
	}
  
	utils::text::replace_all( &new_string, "<br \\/>", "\n" );
	utils::text::replace_all( &new_string, "\n\n\n", "\n\n" );
	//utils::text::replace_all( &new_string, "\\t", "" );
	//utils::text::replace_all( &new_string, "\\n", "" );
	return new_string;
}


std::string utils::text::remove_html( std::string data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '<' && data.at(i+1) != ' ' )
		{
			i = data.find( ">", i );
			if (i == std::string::npos)
				break;

			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::slashu_to_utf8( std::string data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '\\' && (i+1) < data.length( ) && data.at(i+1) == 'u' )
		{
			unsigned int udn = strtol( data.substr( i + 2, 4 ).c_str(), NULL, 16 );

			if ( udn >= 128 && udn <= 2047 )
			{ // U+0080 .. U+07FF
				new_string += ( char )( 192 + ( udn / 64 ) );
				new_string += ( char )( 128 + ( udn % 64 ) );
			} 
			else if ( udn >= 2048 && udn <= 65535 )
			{ // U+0800 .. U+FFFF
				new_string += ( char )( 224 + ( udn / 4096 ) );
				new_string += ( char )( 128 + ( ( udn / 64 ) % 64 ) );
				new_string += ( char )( 128 + ( udn % 64  ) );
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

std::string utils::text::trim( std::string data )
{
	std::string spaces = " \t\r\n"; // TODO: include "nbsp"?
	std::string::size_type begin = data.find_first_not_of( spaces );
	std::string::size_type end = data.find_last_not_of( spaces ) + 1;

	return (begin != std::string::npos) ? data.substr( begin, end - begin ) : "";
}

void utils::text::explode(std::string str, std::string separator, std::vector<std::string>* results)
{
	std::string::size_type pos;
	pos = str.find_first_of(separator);
	while (pos != std::string::npos) {
		if (pos > 0) {
			results->push_back(str.substr(0,pos));
		}
		str = str.substr(pos+1);
		pos = str.find_first_of(separator);
	}
	if (str.length() > 0) {
		results->push_back(str);
	}
}

std::string utils::text::source_get_value( std::string* data, unsigned int argument_count, ... )
{
	va_list arg;
	std::string ret;
	std::string::size_type start = 0, end = 0;
	
	va_start( arg, argument_count );
	
	for ( unsigned int i = argument_count; i > 0; i-- )
	{
		if ( i == 1 )
		{
			end = data->find( va_arg( arg, char* ), start );
			if ( start == std::string::npos || end == std::string::npos )
				break;
			ret = data->substr( start, end - start );
		} else {
			std::string term = va_arg( arg, char* );
			start = data->find( term, start );
			if ( start == std::string::npos )
				break;
			start += term.length();
		}
	}
	
	va_end( arg );	
	return ret;
}

std::string utils::text::source_get_value2( std::string* data, const char *term, const char *endings)
{
	std::string::size_type start = 0, end = 0;
	std::string ret;

	start = data->find(term);
	if (start != std::string::npos) {
		start += strlen(term);

		end = data->find_first_of(endings, start);
		if (end != std::string::npos) {
			ret = data->substr( start, end - start );
		}
	}

	return ret;
}

int utils::number::random( )
{
	srand( ::time( NULL ) );
	return rand( );
}

int utils::debug::log(std::string file_name, std::string text)
{
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInstance, szFile, SIZEOF(szFile));
	std::string path = szFile;
	path = path.substr( 0, path.rfind( "\\" ) );
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

int ext_to_format(const std::string &ext)
{
	for(size_t i=0; i<SIZEOF(formats); i++)
	{
		if(ext == formats[i].ext)
			return formats[i].fmt;
	}
	
	return PA_FORMAT_UNKNOWN;
}
