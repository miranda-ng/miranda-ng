#ifndef JSON_WORKER_H
#define JSON_WORKER_H

class MIR_CORE_EXPORT JSONWorker
{
public:
	static JSONNode parse(const json_string & json);
	#ifdef JSON_VALIDATE
		static JSONNode validate(const json_string & json);
	#endif
	#if defined JSON_DEBUG || defined JSON_SAFE
		static json_char * RemoveWhiteSpace(const json_string & value_t, json_char & last);
	#else
		static json_char * RemoveWhiteSpace(const json_string & value_t);
	#endif
	static json_string RemoveWhiteSpaceAndComments(const json_string & value_t);

	static void DoArray(const internalJSONNode * parent, const json_string & value_t);
	static void DoNode(const internalJSONNode * parent, const json_string & value_t);

	#ifdef JSON_LESS_MEMORY
		#define NAME_ENCODED this, true
		#define STRING_ENCODED this, false
		static json_string FixString(const json_string & value_t, const internalJSONNode * flag, bool which);
	#else
		#define NAME_ENCODED _name_encoded
		#define STRING_ENCODED _string_encoded
		static json_string FixString(const json_string & value_t, bool & flag);
	#endif
	static json_string UnfixString(const json_string & value_t, bool flag);
JSON_PRIVATE
	static json_char Hex(const json_char * & pos);
	static json_uchar UTF8(const json_char * & pos);
	static json_char* UTF8_2(const json_char * & pos);
	static json_string toUTF8(json_uchar p);
	#ifdef JSON_UNICODE
		static json_string UTF(const json_char * & pos);
		static json_string toSurrogatePair(json_uchar pos);
	#endif
	static void SpecialChar(const json_char * & pos, json_string & res);
	static size_t FindNextRelevant(json_char ch, const json_string & value_t, const size_t pos);
	static void NewNode(const internalJSONNode * parent, const json_string & name, const json_string & value, bool array);
};

#endif
