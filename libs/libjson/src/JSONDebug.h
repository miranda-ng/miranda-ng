#ifndef JSON_DEBUG_H
#define JSON_DEBUG_H

#include "JSONDefs.h"
#include "JSONOptions.h"

#ifdef JSON_UNIT_TEST
	#define JSON_PRIVATE
#else
	#define JSON_PRIVATE private:
#endif

#ifdef JSON_DEBUG
	#ifdef JSON_SAFE
		#define JSON_ASSERT_SAFE(condition, msg, code)\
			{\
				if (!(condition)) {\
					JSON_FAIL(msg);\
					code\
				}\
			}
		#define JSON_FAIL_SAFE(msg, code)\
			{\
				JSON_FAIL(msg);\
				code\
			}
	#else
		#define JSON_ASSERT_SAFE(condition, msg, code) JSON_ASSERT(condition, msg)
		#define JSON_FAIL_SAFE(msg, code) JSON_FAIL(msg)
	#endif

	#define JSON_FAIL JSONDebug::_JSON_FAIL
	#define JSON_ASSERT JSONDebug::_JSON_ASSERT

	class JSONDebug {
	public:
		#ifndef JSON_STDERROR
			static void register_callback(json_error_callback_t callback);
		#endif
		static void _JSON_FAIL(const json_string & msg);
		static void _JSON_ASSERT(bool condition, const json_string & msg);
	};
#else
	#ifdef JSON_SAFE
		#define JSON_ASSERT_SAFE(condition, msg, code)\
			{\
				if (!(condition)) {\
					code\
				}\
			}
		#define JSON_FAIL_SAFE(msg, code)\
			{\
				code\
			}
	#else
		#define JSON_ASSERT_SAFE(condition, msg, code)
		#define JSON_FAIL_SAFE(msg, code)
	#endif

	#define JSON_ASSERT(condition, msg)
	#define JSON_FAIL(msg)
#endif

static const json_string EMPTY_STRING;
static const std::string EMPTY_STRING2;

#endif

