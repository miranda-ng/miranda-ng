/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "..\commonheaders.h"

#include "JSONNode.h"
#ifdef JSON_WRITER
#include "JSONWorker.h"

const static json_string WRITER_EMPTY;
#ifndef JSON_NEWLINE
	const static json_string NEW_LINE(JSON_TEXT("\n"));
#else
	const static json_string NEW_LINE(JSON_TEXT(JSON_NEWLINE));
#endif

#ifdef JSON_INDENT
	const static json_string INDENT(JSON_TEXT(JSON_INDENT));

	inline json_string makeIndent(unsigned int amount){
		if (amount == 0xFFFFFFFF) return WRITER_EMPTY;
		json_string result;
		result.reserve(amount);
		for(unsigned int i=0; i < amount; ++i){
			result += INDENT;
		}
		return result;
	}
#else
	inline json_string makeIndent(unsigned int amount){
		if (amount == 0xFFFFFFFF) return WRITER_EMPTY;
		return json_string(amount, JSON_TEXT('\t'));
	}
#endif

json_string internalJSONNode::WriteName(bool formatted, bool arrayChild) const {
	if (arrayChild){
		return WRITER_EMPTY ;
	} else {
		return JSON_TEXT("\"") + JSONWorker::UnfixString(_name, _name_encoded) + ((formatted) ? JSON_TEXT("\" : ") : JSON_TEXT("\":"));
	}
}

json_string internalJSONNode::WriteChildren(unsigned int indent){
	//Iterate through the children and write them
	if (Children.empty()) return WRITER_EMPTY;

	json_string indent_plus_one;
	json_string indent_this;
	json_string res;
	//handle whether or not it's formatted JSON
	if (indent != 0xFFFFFFFF){  //it's formatted, make the indentation strings
		indent_this = NEW_LINE + makeIndent(indent);
		indent_plus_one = NEW_LINE + makeIndent(++indent);
	}
	//else it's not formatted, leave the indentation strings empty
	const size_t size_minus_one = Children.size() - 1;
	size_t i=0;
	json_foreach(Children, it){
		res += indent_plus_one + (*it) -> internal -> Write(indent, type() == JSON_ARRAY);
		if (i < size_minus_one) res += JSON_TEXT(",");  //the last one does not get a comma, but all of the others do
		++i;
	}
	return res + indent_this;
}

#ifdef JSON_COMMENTS
	#ifdef JSON_WRITE_BASH_COMMENTS
		const static json_string SINGLELINE(JSON_TEXT("#"));
	#else
		const static json_string SINGLELINE(JSON_TEXT("//"));
	#endif

	json_string internalJSONNode::WriteComment(unsigned int indent) const {
		if (indent == 0xFFFFFFFF) return WRITER_EMPTY;
		if (_comment.empty()) return WRITER_EMPTY;
		size_t pos = _comment.find(JSON_TEXT('\n'));
		if (pos == json_string::npos){  //Single line comment
			return NEW_LINE + makeIndent(indent) + SINGLELINE + _comment + NEW_LINE + makeIndent(indent);
		}

		/*
		Multiline comments
		*/
		#if defined(JSON_WRITE_BASH_COMMENTS) || defined(JSON_WRITE_SINGLE_LINE_COMMENTS)
			json_string result(NEW_LINE + makeIndent(indent));
		#else
			json_string result(NEW_LINE + makeIndent(indent) + JSON_TEXT("/*") + NEW_LINE + makeIndent(indent + 1));
		#endif
		size_t old = 0;
		while(pos != json_string::npos){
			if (pos && _comment[pos - 1] == JSON_TEXT('\r')) --pos;
			#if defined(JSON_WRITE_BASH_COMMENTS) || defined(JSON_WRITE_SINGLE_LINE_COMMENTS)
				result += SINGLELINE;
			#endif
			result += _comment.substr(old, pos - old) + NEW_LINE;
			#if defined(JSON_WRITE_BASH_COMMENTS) || defined(JSON_WRITE_SINGLE_LINE_COMMENTS)
				result += makeIndent(indent);
			#else
				result += makeIndent(indent + 1);
			#endif
			old = (_comment[pos] == JSON_TEXT('\r')) ? pos + 2 : pos + 1;
			pos = _comment.find(JSON_TEXT('\n'), old);
		}
		#if defined(JSON_WRITE_BASH_COMMENTS) || defined(JSON_WRITE_SINGLE_LINE_COMMENTS)
			result += SINGLELINE;
		#endif
		result += _comment.substr(old, pos - old) + NEW_LINE + makeIndent(indent);
		#if defined(JSON_WRITE_BASH_COMMENTS) || defined(JSON_WRITE_SINGLE_LINE_COMMENTS)
			return result;
		#else
			return result + JSON_TEXT("*/") + NEW_LINE + makeIndent(indent);
		#endif
	}
#else
	json_string internalJSONNode::WriteComment(unsigned int) const {
		return WRITER_EMPTY;
	}
#endif

json_string internalJSONNode::Write(unsigned int indent, bool arrayChild){
	const bool formatted = indent != 0xFFFFFFFF;

	#ifndef JSON_PREPARSE
		if (!(formatted || fetched)) {  //It's not formatted or fetched, just do a raw dump
			return WriteComment(indent) + WriteName(false, arrayChild) + _string;
		}
	#endif

	//It's either formatted or fetched
	switch (type()) {
		case JSON_NODE:	//got members, write the members
			Fetch();
			return WriteComment(indent) + WriteName(formatted, arrayChild) + JSON_TEXT("{") + WriteChildren(indent) + JSON_TEXT("}");
		case JSON_ARRAY:		//write out the child nodes int he array
			Fetch();
			return WriteComment(indent) + WriteName(formatted, arrayChild) + JSON_TEXT("[") + WriteChildren(indent) + JSON_TEXT("]");
		case JSON_NUMBER:	//write out a literal, without quotes
		case JSON_NULL:
		case JSON_BOOL:
			return WriteComment(indent) + WriteName(formatted, arrayChild) + _string;
	}

	JSON_ASSERT_SAFE(type() == JSON_STRING, JSON_TEXT("Writing an unknown JSON node type"), return JSON_TEXT(""););
	//If it go here, then it's a json_string
	#ifndef JSON_PREPARSE
		if (fetched) return WriteComment(indent) + WriteName(formatted, arrayChild) + JSON_TEXT("\"") + JSONWorker::UnfixString(_string, _string_encoded) + JSON_TEXT("\"");  //It's already been fetched, meaning that it's unescaped
		return WriteComment(indent) + WriteName(formatted, arrayChild) + _string;  //it hasn't yet been fetched, so it's already unescaped, just do a dump
	#else
		return WriteComment(indent) + WriteName(formatted, arrayChild) + JSON_TEXT("\"") + JSONWorker::UnfixString(_string, _string_encoded) + JSON_TEXT("\"");
	#endif
}
#endif
