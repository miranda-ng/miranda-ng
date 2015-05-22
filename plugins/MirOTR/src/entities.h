// (C) of entities.h: Christoph
// http://mercurial.intuxication.org/hg/cstuff/raw-file/tip/entities.h
// http://stackoverflow.com/questions/1082162/how-to-unescape-html-in-c/1082191#1082191
// modified by ProgAndy

#ifndef DECODE_HTML_ENTITIES_UTF8
#define DECODE_HTML_ENTITIES_UTF8

extern size_t decode_html_entities_utf8(char *dest, const char *src, size_t len);
/*	if `src` is `NULL`, input will be taken from `dest`, decoding
	the entities in-place

	otherwise, the output will be placed in `dest`, which should point
	to a buffer big enough to hold `mir_strlen(src) + 1` characters, while
	`src` remains unchanged
	if `len` is given, `dest` must be at least big enough 
	to hold `len + 1` characters.

	the function returns the length of the decoded string
*/

extern char * encode_html_entities_utf8(const char *src);

#endif
