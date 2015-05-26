/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\commonheaders.h"

#include "JSONNode.h" 
#include "JSONNode.inl"
#include "JSONWorker.h"

extern JSONNode nullNode;

#ifdef JSON_VALIDATE
JSONNode JSONWorker::validate(const json_string & json){
	JSONNode res = parse(json);
	if (!res.validate()) {
		throw std::invalid_argument(EMPTY_STRING2);
	}
	return JSONNode(true, res);  //forces it to simply return the original interal, even with ref counting off
}
#endif

JSONNode JSONWorker::parse(const json_string & json){
	json_auto<json_char> s;
	#if defined JSON_DEBUG || defined JSON_SAFE
		json_char lastchar;
		s.set(RemoveWhiteSpace(json, lastchar));
	#else
		s.set(RemoveWhiteSpace(json));
	#endif

	#ifdef JSON_COMMENTS
		json_char firstchar = s.ptr[0];
		json_string _comment;
		json_char * runner = s.ptr;
		if (firstchar == '\5') {  //multiple comments will be consolidated into one
			newcomment:
			while(*(++runner) != '\5') {
				JSON_ASSERT(*runner, JSON_TEXT("Removing white space failed"));
				_comment += *runner;
			}
			firstchar = *(++runner); //step past the trailing tag
			if (firstchar == '\5') {
				_comment += '\n';
				goto newcomment;
			}
		}
	#else
		const json_char firstchar = s.ptr[0];
	#endif

	switch (firstchar){
		case '{':
		case '[':
			#if defined JSON_DEBUG || defined JSON_SAFE
				if (firstchar == '[') {
					if (lastchar != ']') {
						JSON_FAIL(JSON_TEXT("Missing final ]"));
						break;
					}
				} else {
					if (lastchar != '}') {
						JSON_FAIL(JSON_TEXT("Missing final }"));
						break;
					}
				}
			#endif
			#ifdef JSON_COMMENTS
				JSONNode foo(runner);
				foo.set_comment(_comment);
				return JSONNode(true, foo);  //forces it to simply return the original interal, even with ref counting off
			#else
				return JSONNode(s.ptr);
			#endif
	}

	JSON_FAIL(JSON_TEXT("Not JSON!"));
	return nullNode;
}

#define QUOTECASE()\
	case JSON_TEXT('\"'):\
		while (*(++p) != JSON_TEXT('\"')) {\
			JSON_ASSERT_SAFE(*p, JSON_TEXT("Null terminator inside of a quotation"), return json_string::npos;);\
		}\
		break;

#ifdef JSON_DEBUG
	#define NULLCASE(error)\
		case JSON_TEXT('\0'):\
			JSON_FAIL_SAFE(error, return json_string::npos;);\
			break;
#else
	#define NULLCASE(error)
#endif

#define BRACKET(left, right)\
	case left: {\
		size_t brac = 1;\
		while (brac){\
			switch (*(++p)) {\
				case right:\
				--brac;\
				break;\
				case left:\
				++brac;\
				break;\
				QUOTECASE()\
				NULLCASE(JSON_TEXT("Null terminator inside of a bracket"))\
			}\
		}\
		break;}\
	case right:\
		return json_string::npos;

size_t JSONWorker::FindNextRelevant(json_char ch, const json_string & value_t, const size_t pos){
	const json_char * start = value_t.c_str();
	const json_char * p = start + pos; //start at the correct offset
	do {
		if (*p == ch) return p - start;
		switch (*p){
			BRACKET(JSON_TEXT('['), JSON_TEXT(']'))
			BRACKET(JSON_TEXT('{'), JSON_TEXT('}'))
			QUOTECASE()
		}
	} while(*(++p));
	return json_string::npos;
}

#ifdef JSON_COMMENTS
	#define COMMENT_DELIMITER() *runner++='\5'
	#define AND_RUNNER ,runner
	inline void SingleLineComment(const json_char * & p, json_char * & runner){
		COMMENT_DELIMITER();
		while((*(++p)) && (*p != JSON_TEXT('\n'))) {
			*runner++=*p;
		}
		COMMENT_DELIMITER();
	}
#else
	#define COMMENT_DELIMITER() (void)0
	#define AND_RUNNER
#endif

inline void SingleLineComment(const json_char * & p){
	while((*(++p)) && (*p != JSON_TEXT('\n')));
}

#if defined JSON_DEBUG || defined JSON_SAFE
	json_char * JSONWorker::RemoveWhiteSpace(const json_string & value_t, json_char & last){
#else
	json_char * JSONWorker::RemoveWhiteSpace(const json_string & value_t){
#endif
		json_char * result;
		json_char * runner = result = json_malloc<json_char>(value_t.length() + 1);  //dealing with raw memory is faster than adding to a json_string
		JSON_ASSERT(result, JSON_TEXT("Out of memory"));
		const json_char * p = value_t.c_str();
		while(*p){
			switch(*p){
				case JSON_TEXT(' '):  //defined as white space
				case JSON_TEXT('\t'):  //defined as white space
				case JSON_TEXT('\n'):  //defined as white space
				case JSON_TEXT('\r'):  //defined as white space
				break;
				case JSON_TEXT('/'):  //a C comment
				if (*(++p) == JSON_TEXT('*')) {  //a multiline comment
					COMMENT_DELIMITER();
					while ((*(++p) != JSON_TEXT('*')) || (*(p + 1) != JSON_TEXT('/'))) {
						JSON_ASSERT_SAFE(*p, JSON_TEXT("Null terminator inside of a multiline quote"), COMMENT_DELIMITER(); goto endofloop;);
						*runner++=*p;
					}
					++p;
					COMMENT_DELIMITER();
					break;
				}
				//Should be a single line C comment, so let it fall through to use the bash comment stripper
				JSON_ASSERT_SAFE(*p == JSON_TEXT('/'), JSON_TEXT("stray / character, not quoted, or a comment"), goto endofloop;);
				case JSON_TEXT('#'):  //a bash comment
				SingleLineComment(p AND_RUNNER);
				break;
				case JSON_TEXT('\"'):  //a quote
				*runner++=JSON_TEXT('\"');
				while(*(++p) != JSON_TEXT('\"')) {  //find the end of the quotation, as white space is preserved within it
					JSON_ASSERT_SAFE(*p, JSON_TEXT("Null terminator inside of a quotation"), goto endofloop;);
					switch(*p){
						case JSON_TEXT('\\'):
							*runner++=JSON_TEXT('\\');
							*runner++=(*++p == JSON_TEXT('\"')) ? JSON_TEXT('\1') : *p;  //an escaped quote will reak havoc will all of my searching functions, so change it into an illegal character in JSON for convertion later on
							break;
						default:
							*runner++=*p;
							break;
					}
				}
				//no break, let it fall through so that the trailing quote gets added
				default:
				JSON_ASSERT_SAFE((json_uchar)*p >= 32, JSON_TEXT("Invalid JSON character detected (lo)"), goto endofloop;);
				JSON_ASSERT_SAFE((json_uchar)*p <= 126, JSON_TEXT("Invalid JSON character detected (hi)"), goto endofloop;);
				*runner++=*p;
				break;
			}
			++p;
		}
		#ifdef JSON_SAFE
			endofloop:
		#endif
		#if defined JSON_DEBUG || defined JSON_SAFE
			last = *(runner - 1);
		#endif
		*runner = JSON_TEXT('\0');
		return result;
	}

json_string JSONWorker::RemoveWhiteSpaceAndComments(const json_string & value_t){
	json_string result;
	result.reserve(value_t.length());
	const json_char * p = value_t.c_str();
	while(*p){
		switch(*p){
			case JSON_TEXT(' '):	//defined as white space
			case JSON_TEXT('\t'):  //defined as white space
			case JSON_TEXT('\n'):  //defined as white space
			case JSON_TEXT('\r'):  //defined as white space
				break;
			case JSON_TEXT('/'):  //a C comment
				if (*(++p) == JSON_TEXT('*')) {  //a multiline comment
				while ((*(++p) != JSON_TEXT('*')) || (*(p + 1) != JSON_TEXT('/'))) {
					JSON_ASSERT_SAFE(*p, JSON_TEXT("Null terminator inside of a multiline quote"), goto endofloop;);
				}
				++p;
				break;
				}
				//Should be a single line C comment, so let it fall through to use the bash comment stripper
				JSON_ASSERT_SAFE(*p == JSON_TEXT('/'), JSON_TEXT("stray / character, not quoted, or a comment"), goto endofloop;);
			case JSON_TEXT('#'):  //a bash comment
				SingleLineComment(p);
				break;
			case JSON_TEXT('\"'):  //a quote
				result += JSON_TEXT('\"');
				while(*(++p) != JSON_TEXT('\"')) {  //find the end of the quotation, as white space is preserved within it
				JSON_ASSERT(*p, JSON_TEXT("Null terminator inside of a quotation"));
				switch(*p){
					case JSON_TEXT('\\'):
						result += JSON_TEXT('\\');
						result += (*++p == JSON_TEXT('\"')) ? JSON_TEXT('\1') : *p;  //an escaped quote will reak havoc will all of my searching functions, so change it into an illegal character in JSON for convertion later on
						break;
					default:
						result += *p;
						break;
				}
				}
				//no break, let it fall through so that the trailing quote gets added
			default:
				JSON_ASSERT_SAFE((json_uchar)*p >= 32, JSON_TEXT("Invalid JSON character detected (lo)"), goto endofloop;);
				JSON_ASSERT_SAFE((json_uchar)*p <= 126, JSON_TEXT("Invalid JSON character detected (hi)"), goto endofloop;);
				result += *p;
				break;
		}
		++p;
	}
	#ifdef JSON_SAFE
		endofloop:
	#endif
	return result;
}

/*
 These three functions analyze json_string literals and convert them into std::strings
 This includes dealing with special characters and utf characters
 */
#ifdef JSON_UNICODE
	inline json_uchar SurrogatePair(const json_uchar hi, const json_uchar lo){
		JSON_ASSERT(sizeof(unsigned int) == 4, JSON_TEXT("size of unsigned int is not 32-bit"));
		JSON_ASSERT(sizeof(json_uchar) == 4, JSON_TEXT("size of json_char is not 32-bit"));
		return (((hi << 10) & 0x1FFC00) + 0x10000) | lo & 0x3FF;
	}

	json_string JSONWorker::UTF(const json_char * & pos){
		json_string result;
		json_uchar first = UTF8(pos);
		if ((*(pos + 1) == '\\') && (*(pos + 2) == 'u')) {
			pos += 2;
			json_uchar second = UTF8(pos);
			//surrogate pair, not two characters
			if ((first > 0xD800) && (first < 0xDBFF) && (second > 0xDC00) && (second < 0xDFFF)) {
				result += SurrogatePair(first, second);
			} else {
				result += first;
				result += second;
			}
		} else {
			result += first;
		}
		JSON_ASSERT(!result.empty(), JSON_TEXT("missing case, somehow UTF returned empty"));
		return result;
	}
#endif

json_uchar JSONWorker::UTF8(const json_char * & pos){
	#ifdef JSON_UNICODE
		++pos;
		json_uchar temp = Hex(pos) << 8;
		++pos;
		return temp | Hex(pos);
	#else
		JSON_ASSERT(*(pos + 1) == JSON_TEXT('0'), JSON_TEXT("wide utf character (hihi)"));
		JSON_ASSERT(*(pos + 2) == JSON_TEXT('0'), JSON_TEXT("wide utf character (hilo)"));
		pos += 3;
		return Hex(pos);
	#endif
}

static json_char szU8Buffer[10];

json_char* JSONWorker::UTF8_2(const json_char * & pos){
	#ifdef JSON_UNICODE
		++pos;
		json_uchar temp = Hex(pos) << 8;
		++pos;
		*szU8Buffer = temp | Hex(pos);
		szU8Buffer[1] = 0;
		return szU8Buffer;
	#else
		union {
			unsigned short uChar;
			unsigned char uByte[2];
		};
		pos++;
		strncpy(szU8Buffer+5,pos,4);
		szU8Buffer[9] = 0;
		uChar = strtoul(szU8Buffer+5,NULL,16);
		if (uChar<0x80) {
			szU8Buffer[0] = uChar;
			szU8Buffer[1] = 0;
		} else if (uChar<0x7ff) {
			szU8Buffer[0] = 0xc0+(uByte[1]<<2)+(uByte[0]>>6);
			szU8Buffer[1] = 0x80+(uByte[0]&0x3f);
			szU8Buffer[2] = 0;
		} else {
			szU8Buffer[0] = 0xe0+(uByte[1]>>4);
			szU8Buffer[1] = 0x80+((uByte[1]&0x0f)<<2)+(uByte[0]>>6);
			szU8Buffer[2] = 0x80+(uByte[0]&0x3f);
			szU8Buffer[3] = 0;
		}

		pos += 3;
		return szU8Buffer;
	#endif
}

json_char JSONWorker::Hex(const json_char * & pos){
	/*
	takes the numeric value of the next two characters and convert them
	\u0058 becomes 0x58

	In case of \u, it's SpecialChar's responsibility to move past the first two chars
	as this method is also used for \x
	*/
	//First character
	json_uchar hi = *pos++ - 48;
	if (hi > 48){  //A-F don't immediately follow 0-9, so have to pull them down a little
		hi -= 39;
	} else if (hi > 9){  //neither do a-f
		hi -= 7;
	}
	//second character
	json_uchar lo = *pos - 48;
	if (lo > 48){  //A-F don't immediately follow 0-9, so have to pull them down a little
		lo -= 39;
	} else if (lo > 9){  //neither do a-f
		lo -= 7;
	}
	//combine them
	return (json_char)((hi << 4) | lo);
}

inline json_char FromOctal(const json_char * & str){
	JSON_ASSERT(json_strlen(str) > 3, JSON_TEXT("Octal will go out of bounds"));
	const json_uchar top = ((json_uchar)(*(str++) - 48));
	const json_uchar middle = (json_uchar)(*(str++) - 48);
	const json_uchar bottom = (json_uchar)(*str - 48);
	return (json_char)((top << 6) | (middle << 3) | bottom);
}

void JSONWorker::SpecialChar(const json_char * & pos, json_string & res){
	/*
	Since JSON uses forward slash escaping for special characters within strings, I have to
	convert these escaped characters into C characters
	*/
	switch(*pos){
		case JSON_TEXT('\1'):  //quote character (altered by RemoveWhiteSpace)
			res += JSON_TEXT('\"');
			break;
		case JSON_TEXT('t'):	//tab character
			res += JSON_TEXT('\t');
			break;
		case JSON_TEXT('n'):	//newline character
			res += JSON_TEXT('\n');
			break;
		case JSON_TEXT('r'):	//return character
			res += JSON_TEXT('\r');
			break;
		case JSON_TEXT('\\'):	//backslash
			res += JSON_TEXT('\\');
			break;
		case JSON_TEXT('/'):	//forward slash
			res += JSON_TEXT('/');
			break;
		case JSON_TEXT('b'):	//backspace
			res += JSON_TEXT('\b');
			break;
		case JSON_TEXT('f'):	//formfeed
			res += JSON_TEXT('\f');
			break;
		case JSON_TEXT('v'):	//vertical tab
			res += JSON_TEXT('\v');
			break;
		case JSON_TEXT('\''):	//apostrophe
			res += JSON_TEXT('\'');
			break;
		case JSON_TEXT('x'):	//hexidecimal ascii code
			res += Hex(++pos);
			break;
		case JSON_TEXT('u'):	//utf character
			#ifdef JSON_UNICODE
				res += UTF(pos);
			#else
				//res += UTF8(pos);
				res.append(UTF8_2(pos));
			#endif
			break;

			//octal encoding
		case JSON_TEXT('1'):
		case JSON_TEXT('2'):
		case JSON_TEXT('3'):
		case JSON_TEXT('4'):
		case JSON_TEXT('5'):
		case JSON_TEXT('6'):
		case JSON_TEXT('7'):
		case JSON_TEXT('0'):
			res += FromOctal(pos);
			break;

		#ifdef JSON_DEBUG
			default:
				JSON_FAIL(JSON_TEXT("Unsupported escaped character"));
		#endif
	}
}

#ifdef JSON_LESS_MEMORY
		inline void doflag(const internalJSONNode * flag, bool which, bool x){
			if (which){
				flag -> _name_encoded = x;
			} else {
				flag -> _string_encoded = x;
			}
		}

		json_string JSONWorker::FixString(const json_string & value_t, const internalJSONNode * flag, bool which){
		#define setflag(x) doflag(flag, which, x)
#else
		json_string JSONWorker::FixString(const json_string & value_t, bool & flag){
	#define setflag(x) flag = x
#endif
	/*
	Do things like unescaping
	*/
	setflag(false);
	json_string res;
	res.reserve(value_t.length());		//since it goes one character at a time, want to reserve it first so that it doens't have to reallocating
	const json_char * p = value_t.c_str();
	while(*p){
		switch (*p){
			case JSON_TEXT('\\'):
				setflag(true);
				SpecialChar(++p, res);
				break;
			default:
				res += *p;
				break;
		}
		++p;
	}
	return res;
}

#ifdef JSON_UNICODE
	json_string JSONWorker::toSurrogatePair(json_uchar C){
		JSON_ASSERT(sizeof(unsigned int) == 4, JSON_TEXT("size of unsigned int is not 32-bit"));
		JSON_ASSERT(sizeof(unsigned short) == 2, JSON_TEXT("size of unsigned short is not 16-bit"));
		JSON_ASSERT(sizeof(json_uchar) == 4, JSON_TEXT("json_char is not 32-bit"));
		//Compute the high surrogate
		const unsigned int U = (C >> 16) & 31;
		unsigned short HiSurrogate = 0xD800 | (((unsigned short)U - 1) << 6) | ((unsigned short)C) >> 10;

		//compute the low surrogate
		unsigned short LoSurrogate = (unsigned short) (0xDC00 | ((unsigned short)C) & 1023);

		json_string res;
		res += toUTF8(HiSurrogate);
		res += toUTF8(LoSurrogate);
		return res;
	}
#endif

json_string JSONWorker::toUTF8(json_uchar p){
	#ifdef JSON_UNICODE
		if (p > 0xFFFF) return toSurrogatePair(p);
	#endif
	json_string res(JSON_TEXT("\\u"));
	#ifdef JSON_UNICODE
		json_uchar hihi = ((p & 0xF000) >> 12) + 48;
		if (hihi > 57) hihi += 7; //A-F don't immediately follow 0-9, so have to further adjust those
		json_uchar hilo = ((p & 0x0F00) >> 8) + 48;
		if (hilo > 57) hilo += 7; //A-F don't immediately follow 0-9, so have to further adjust those
		res += hihi;
		res += hilo;
		json_uchar hi = ((p & 0x00F0) >> 4) + 48;
	#else
		res += JSON_TEXT("00");
		json_uchar hi = (p >> 4) + 48;
	#endif
	//convert the character to be escaped into two digits between 0 and 15
	if (hi > 57) hi += 7; //A-F don't immediately follow 0-9, so have to further adjust those
	json_uchar lo = (p & 0x000F) + 48;
	if (lo > 57) lo += 7; //A-F don't immediately follow 0-9, so have to further adjust those
	res += hi;
	res += lo;
	return res;
}

json_string JSONWorker::UnfixString(const json_string & value_t, bool flag){
	if (!flag) return value_t;
	/*
	Re-escapes a json_string so that it can be written out into a JSON file
	*/
	json_string res;
	res.reserve(value_t.length());  //since it goes one character at a time, want to reserve it first so that it doens't have to reallocating
	const json_char * p = value_t.c_str();
	while(*p){
		switch(*p){
			case JSON_TEXT('\"'):  //quote character
				res += JSON_TEXT("\\\"");
				break;
			case JSON_TEXT('\t'):	//tab character
				res += JSON_TEXT("\\t");
				break;
			case JSON_TEXT('\n'):	//newline character
				res += JSON_TEXT("\\n");
				break;
			case JSON_TEXT('\r'):	//return character
				res += JSON_TEXT("\\r");
				break;
			case JSON_TEXT('\\'):	//backslash
				res += JSON_TEXT("\\\\");
				break;
			case JSON_TEXT('/'):	//forward slash
				res += JSON_TEXT("\\/");
				break;
			case JSON_TEXT('\b'):	//backspace
				res += JSON_TEXT("\\b");
				break;
			case JSON_TEXT('\f'):	//formfeed
				res += JSON_TEXT("\\f");
				break;
			case JSON_TEXT('\v'):	//vertical tab
				res += JSON_TEXT("\\v");
				break;
			case JSON_TEXT('\''):	//apostrophe
				res += JSON_TEXT("\\\'");
				break;
			default:
				/*if (((json_uchar)(*p) < 32) || ((json_uchar)(*p) > 126)) {
				//res += toUTF8((json_uchar)(*p));
				} else*/ {
				res += *p;
				}
				break;
		}
		++p;
	}
	return res;
}

//Create a childnode
#ifdef JSON_COMMENTS
	#define ARRAY_PARAM bool array  //Just to supress warnings
#else
	#define ARRAY_PARAM bool
#endif
inline void JSONWorker::NewNode(const internalJSONNode * parent, const json_string & name, const json_string & value, ARRAY_PARAM){
	#ifdef JSON_COMMENTS
	const json_char * runner = (array) ? value.c_str() : name.c_str();
		json_string _comment;
		if (*runner == '\5') {  //multiple comments will be consolidated into one
			newcomment:
			while(*(++runner) != '\5') {
				JSON_ASSERT(*runner, JSON_TEXT("Removing white space failed"));
				_comment += *runner;
			}
			if (*(++runner) == '\5') { //step past the trailing tag
				_comment += '\n';
				goto newcomment;
			}
		}
		internalJSONNode * myinternal;
		if (array){
			myinternal = internalJSONNode::newInternal(name, runner);
		} else {
			myinternal = internalJSONNode::newInternal(++runner, value);
		}
		JSONNode * child = JSONNode::newJSONNode(myinternal);
		child -> set_comment(_comment);
		const_cast<internalJSONNode*>(parent) -> Children.push_back(child);	//attach it to the parent node
	#else
		const_cast<internalJSONNode*>(parent) -> Children.push_back(JSONNode::newJSONNode(internalJSONNode::newInternal(name.c_str() + 1, value)));		//attach it to the parent node
	#endif
}

//Create a subarray
void JSONWorker::DoArray(const internalJSONNode * parent, const json_string & value_t){
	/*
	This takes an array and creates nodes out of them
	*/
	JSON_ASSERT(!value_t.empty(), JSON_TEXT("DoArray is empty"));
	JSON_ASSERT_SAFE(value_t[0] == JSON_TEXT('['), JSON_TEXT("DoArray is not an array"), parent -> Nullify(); return;);
	const size_t len = value_t.length();
	if (len <= 2) return;  // just a [] (blank array)

	//Not sure what's in the array, so we have to use commas
	size_t starting = 1;  //ignore the [
	size_t ending = FindNextRelevant(JSON_TEXT(','), value_t, 1);
	while (ending != json_string::npos){
		#ifdef JSON_SAFE
			json_string newValue = value_t.substr(starting, ending - starting);
			JSON_ASSERT_SAFE(FindNextRelevant(JSON_TEXT(':'), newValue, 0) == json_string::npos, JSON_TEXT("Key/Value pairs are not allowed in arrays"), parent -> Nullify(); return;);
			NewNode(parent, JSON_TEXT(""), newValue, true);
		#else
			NewNode(parent, JSON_TEXT(""), value_t.substr(starting, ending - starting), true);
		#endif
			starting = ending + 1;
			ending = FindNextRelevant(JSON_TEXT(','), value_t, starting);
		}
		//since the last one will not find the comma, we have to add it here, but ignore the final ]

	#ifdef JSON_SAFE
		json_string newValue = value_t.substr(starting, len - starting - 1);
		JSON_ASSERT_SAFE(FindNextRelevant(JSON_TEXT(':'), newValue, 0) == json_string::npos, JSON_TEXT("Key/Value pairs are not allowed in arrays"), parent -> Nullify(); return;);
		NewNode(parent, JSON_TEXT(""), newValue, true);
	#else
		NewNode(parent, JSON_TEXT(""), value_t.substr(starting, len - starting - 1), true);
	#endif
}

//Create all child nodes
void JSONWorker::DoNode(const internalJSONNode * parent, const json_string & value_t){
	/*
	This take a node and creates its members and such
	*/
	JSON_ASSERT(!value_t.empty(), JSON_TEXT("DoNode is empty"));
	JSON_ASSERT_SAFE(value_t[0] == JSON_TEXT('{'), JSON_TEXT("DoNode is not an node"), parent -> Nullify(); return;);
	const size_t len = value_t.length();
	if (len <= 2) return;  // just a {} (blank node)

	size_t name_starting = 1;  //ignore the {
	size_t name_ending = FindNextRelevant(JSON_TEXT(':'), value_t, 1);  //find where the name ends
	JSON_ASSERT_SAFE(name_ending != json_string::npos, JSON_TEXT("Missing :"), parent -> Nullify(); return;);
	json_string name = value_t.substr(name_starting, name_ending - 2);		//pull the name out
	size_t value_ending = FindNextRelevant(JSON_TEXT(','), value_t, name_ending);  //find the end of the value
	while (value_ending != json_string::npos){
		NewNode(parent, name, value_t.substr(name_ending + 1, value_ending - name_ending - 1), false);
		name_starting = value_ending + 1;
		name_ending = FindNextRelevant(JSON_TEXT(':'), value_t, name_starting);
		JSON_ASSERT_SAFE(name_ending != json_string::npos, JSON_TEXT("Missing :"), parent -> Nullify(); return;);
		name = value_t.substr(name_starting, name_ending - name_starting - 1);
		value_ending = FindNextRelevant(JSON_TEXT(','), value_t, name_ending);
	}
	//since the last one will not find the comma, we have to add it here
	NewNode(parent, name, value_t.substr(name_ending + 1, len - name_ending - 2), false);
}
