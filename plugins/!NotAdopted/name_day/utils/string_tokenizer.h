/**
 * @brief tokenizer
 *
 */
#ifndef string_tokenizer_h
#define string_tokenizer_h

#include <string>
#include <vector>
using namespace std;

static vector<string> string_tokenizer(const string &base_string, const string &delims)
{
	vector<string> tokens;
	
	// Skip delimiters at beginning.
	string::size_type last_pos = base_string.find_first_not_of(delims, 0);

	// find first "non-delimiter".
	string::size_type pos = base_string.find_first_of(delims, last_pos);
	
	while (string::npos != pos || string::npos != last_pos) {
		
		// found a token, add it to the vector.

		tokens.push_back(base_string.substr(last_pos, pos - last_pos));

		// skip delimiters.
		
		last_pos = base_string.find_first_not_of(delims, pos);

		// find next "non-delimiter"
		pos = base_string.find_first_of(delims, last_pos);
	}

	return tokens;
}


#endif