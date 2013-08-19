#ifndef NUMBERTOSTRING_H
#define NUMBERTOSTRING_H

#include "JSONDebug.h"
#include "JSONMemory.h"
#include <cstdio>

static unsigned int getlen(unsigned int size){
	switch (size){
		case 1:
			return 5;   //3 digits for the number, plus null terminator and negation
		case 2:
			return 7;   //5 digits for the number, plus null terminator and negation
		case 4:
			return 12;  //10 digits for the number, plus null terminator and negation
		case 8:
			return 22;  //20 digits for the number, plus null terminator and negation
	}
	JSON_ASSERT(size == 16, JSON_TEXT("template is not recognized 2^x in size"));
	return 41;  //39 digits for the number, plus null terminator and negation
}

class NumberToString {
public:
	template<typename T>
	static json_string _itoa(T val, unsigned int size){
		long value = (long)val;
		const unsigned int digits = getlen(size);
		json_auto<json_char> result(digits);
		result.ptr[digits - 1] = JSON_TEXT('\0'); //null terminator
		json_char * runner = &result.ptr[digits - 2];
		bool negative;

		//first thing, check if it's negative, if so, make it positive
		if (value < 0){
			value = -value;
			negative = true;
		} else {
			negative = false;
		}

		//create the string
		do {
			*runner--=(json_char)(value % 10) + JSON_TEXT('0');
		} while(value /= 10);

		//if it's negative, add the negation
		json_string res;
		if (negative){
			*runner = JSON_TEXT('-');
			res = runner;
		} else {
			res = runner + 1;
		}
		return res;   
	}

	#ifndef JSON_LIBRARY
		template<typename T>
		static json_string _uitoa(T val, unsigned int size){
			unsigned long value = val;
			const unsigned int digits = getlen(size) - 1;  //minus one because no '-' char
			json_auto<json_char> result(digits);
			result.ptr[digits - 1] = JSON_TEXT('\0'); //null terminator
			json_char * runner = &result.ptr[digits - 2];

			//create the string
			do {
				*runner--=(json_char)(value % 10) + JSON_TEXT('0');
			} while(value /= 10);

			json_string res = runner + 1;
			return res;   
		}
	#endif

	template<typename T>
	static json_string _ftoa(T value){
		json_char result[64];
		#ifdef JSON_UNICODE
			mir_snwprintf(result, 63, L"%f", value); 
		#else
			mir_snprintf(result, 63, "%f", value); 
		#endif
		//strip the trailing zeros
		for(json_char * pos = &result[0]; *pos; ++pos){
			if (*pos == '.') {  //only care about after the decimal
				for(json_char * runner = pos + 1; *runner; ++runner){
					if (*runner != JSON_TEXT('0')) pos = runner + 1;
				}
				*pos = JSON_TEXT('\0');
				break;
			}
		}
		return result;
	}

	static inline bool areEqual(const double & one, const double & two){
		const double temp = one - two;
		return (temp > 0.0) ? temp < 0.00001 : temp > -0.00001;
	}
};

#endif
