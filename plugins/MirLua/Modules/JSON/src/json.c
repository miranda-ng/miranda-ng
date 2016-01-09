#include "stdafx.h"

int vasprintf(char **strp, const char *fmt, va_list ap)
{
	int r = -1, size = _vscprintf(fmt, ap);

	if ((size >= 0) && (size < INT_MAX))
	{
		*strp = (char *)malloc(size + 1); //+1 for null
		if (*strp)
		{
			r = vsnprintf(*strp, size + 1, fmt, ap);  //+1 for null
			if ((r < 0) || (r > size))
			{
				free(*strp);
				*strp = 0;
				r = -1;
			}
		}
	}
	else { *strp = 0; }

	return(r);
}

static int
json_error(lua_State *L, const char *fmt, ...)
{
	va_list ap;
	int len;
	char *msg;

	va_start(ap, fmt);
	len = vasprintf(&msg, fmt, ap);
	va_end(ap);

	lua_pushnil(L);
	if (len != -1) {
		lua_pushstring(L, msg);
		free(msg);
	}
	else
		lua_pushstring(L, "internal error: vasprintf failed");
	longjmp(env, 0);
}

static unsigned int
digit2int(lua_State *L, const unsigned char digit)
{
	unsigned int val = 0;

	if (digit >= '0' && digit <= '9')
		val = digit - '0';
	else if (digit >= 'a' || digit <= 'f')
		val = digit - 'a' + 10;
	else if (digit >= 'A' || digit <= 'F')
		val = digit - 'A' + 10;
	else
		json_error(L, "Invalid hex digit");
	return val;
}

static unsigned int
fourhex2int(lua_State *L, const unsigned char *code)
{
	unsigned int utf = 0;

	utf += digit2int(L, code[0]) * 4096;
	utf += digit2int(L, code[1]) * 256;
	utf += digit2int(L, code[2]) * 16;
	utf += digit2int(L, code[3]);
	return utf;
}

static const char *
code2utf8(lua_State *L, const unsigned char *code, char buf[4])
{
	unsigned int utf = 0;

	utf = fourhex2int(L, code);
	if (utf < 128) {
		buf[0] = utf & 0x7F;
		buf[1] = buf[2] = buf[3] = 0;
	}
	else if (utf < 2048) {
		buf[0] = ((utf >> 6) & 0x1F) | 0xC0;
		buf[1] = (utf & 0x3F) | 0x80;
		buf[2] = buf[3] = 0;
	}
	else {
		buf[0] = ((utf >> 12) & 0x0F) | 0xE0;
		buf[1] = ((utf >> 6) & 0x3F) | 0x80;
		buf[2] = (utf & 0x3F) | 0x80;
		buf[3] = 0;
	}
	return buf;
}

static void
skip_ws(char **s)
{
	while (isspace(**s))
		(*s)++;
}

static void
decode_array(lua_State *L, char **s, int null)
{
	int i = 1;

	(*s)++;
	lua_newtable(L);
	for (i = 1; 1; i++) {
		skip_ws(s);
		lua_pushinteger(L, i);
		decode_value(L, s, null);
		lua_settable(L, -3);
		skip_ws(s);
		if (**s == ',') {
			(*s)++;
			skip_ws(s);
		}
		else
			break;
	}
	skip_ws(s);
	if (**s == ']')
		(*s)++;
	else
		json_error(L, "array does not end with ']'");
}

static void
decode_object(lua_State *L, char **s, int null)
{
	(*s)++;
	lua_newtable(L);
	skip_ws(s);

	if (**s != '}') {
		while (1) {
			skip_ws(s);
			decode_string(L, s);
			skip_ws(s);
			if (**s != ':')
				json_error(L, "object lacks separator ':'");
			(*s)++;
			skip_ws(s);
			decode_value(L, s, null);
			lua_settable(L, -3);
			skip_ws(s);
			if (**s == ',') {
				(*s)++;
				skip_ws(s);
			}
			else
				break;
		}
		skip_ws(s);
	}
	if (**s == '}')
		(*s)++;
	else
		json_error(L, "objects does not end with '}'");
}

static void
decode_string(lua_State *L, char **s)
{
	size_t len;
	char *newstr = NULL, *newc, *beginning, *end, *nextEscape = NULL;
	char utfbuf[4] = "";

	(*s)++;
	beginning = *s;
	for (end = NULL; **s != '\0' && end == NULL; (*s)++) {
		if (**s == '"' && (*((*s) - 1) != '\\'))
			end = *s;
	}
	*s = beginning;
	len = strlen(*s);
	newstr = (char*)malloc(len + 1);
	memset(newstr, 0, len + 1);
	newc = newstr;
	while (*s != end) {
		nextEscape = strchr(*s, '\\');
		if (nextEscape > end)
			nextEscape = NULL;
		if (nextEscape == *s) {
			switch (*((*s) + 1)) {
			case '"':
				*newc = '"';
				newc++;
				(*s) += 2;
				break;
			case '\\':
				*newc = '\\';
				newc++;
				(*s) += 2;
				break;
			case '/':
				*newc = '/';
				newc++;
				(*s) += 2;
				break;
			case 'b':
				*newc = '\b';
				newc++;
				(*s) += 2;
				break;
			case 'f':
				*newc = '\f';
				newc++;
				(*s) += 2;
				break;
			case 'n':
				*newc = '\n';
				newc++;
				(*s) += 2;
				break;
			case 'r':
				*newc = '\r';
				newc++;
				(*s) += 2;
				break;
			case 't':
				*newc = '\t';
				newc++;
				(*s) += 2;
				break;
			case 'u':
				code2utf8(L, (unsigned char *)(*s) + 2, utfbuf);
				strcpy(newc, utfbuf);
				newc += strlen(utfbuf);
				(*s) += 6;
				break;
			default:
				json_error(L, "invalid escape character");
				break;
			}
		}
		else if (nextEscape != NULL) {
			size_t len = nextEscape - *s;
			strncpy(newc, *s, len);
			newc += len;
			(*s) += len;
		}
		else {
			size_t len = end - *s;
			strncpy(newc, *s, len);
			newc += len;
			(*s) += len;
		}
	}
	*newc = 0;
	lua_pushstring(L, newstr);
	(*s)++;
	free(newstr);
}

static void
decode_value(lua_State *L, char **s, int null)
{
	skip_ws(s);

	if (!strncmp(*s, "false", 5)) {
		lua_pushboolean(L, 0);
		*s += 5;
	}
	else if (!strncmp(*s, "true", 4)) {
		lua_pushboolean(L, 1);
		*s += 4;
	}
	else if (!strncmp(*s, "null", 4)) {
		switch (null) {
		case 0:
			lua_pushstring(L, "");
			break;
		case 1:
			lua_newtable(L);
			luaL_getmetatable(L, JSON_NULL_METATABLE);
			lua_setmetatable(L, -2);
			break;
		case 2:
			lua_pushnil(L);
			break;
		}
		*s += 4;
	}
	else if (isdigit(**s) || **s == '+' || **s == '-') {
		lua_pushnumber(L, atof(*s));
		/* advance pointer past the number */
		while (isdigit(**s) || **s == '+' || **s == '-'
			|| **s == 'e' || **s == 'E' || **s == '.')
			(*s)++;
	}
	else {
		switch (**s) {
		case '[':
			decode_array(L, s, null);
			break;
		case '{':
			decode_object(L, s, null);
			break;
		case '"':
			decode_string(L, s);
			break;
		case ']':	/* ignore end of empty array */
			lua_pushnil(L);
			break;
		default:
			json_error(L, "syntax error");
			break;
		}
	}
}

static int
json_decode(lua_State *L)
{
	char *s;
	int null;
	const char *const options[] = {
		"empty-string",
		"json-null",
		"nil",
		NULL
	};

	s = (char *)luaL_checkstring(L, 1);

	null = luaL_checkoption(L, 2, "json-null", options);

	if (!setjmp(env)) {
		decode_value(L, &s, null);
		return 1;
	}
	else
		return 2;
}

/* encode JSON */

/* encode_string assumes an UTF-8 string */
static void
encode_string(lua_State *L, luaL_Buffer *b, unsigned char *s)
{
	char hexbuf[6];

	luaL_addchar(b, '"');
	for (; *s; s++) {
		switch (*s) {
		case '\\':
			luaL_addstring(b, "\\\\");
			break;
		case '"':
			luaL_addstring(b, "\\\"");
			break;
		case '\b':
			luaL_addstring(b, "\\b");
			break;
		case '\f':
			luaL_addstring(b, "\\f");
			break;
		case '\n':
			luaL_addstring(b, "\\n");
			break;
		case '\r':
			luaL_addstring(b, "\\r");
			break;
		case '\t':
			luaL_addstring(b, "\\t");
			break;
		default:
			/* Convert UTF-8 to unicode
			* 00000000 - 0000007F: 0xxxxxxx
			* 00000080 - 000007FF: 110xxxxx 10xxxxxx
			* 00000800 - 0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
			* 00010000 - 001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			*/
			if ((*s & 0x80) == 0)
				luaL_addchar(b, *s);
			else if (((*s >> 5) & 0x07) == 0x06) {
				luaL_addstring(b, "\\u");
				_snprintf(hexbuf, sizeof hexbuf, "%04x",
					((*s & 0x1f) << 6) | (*(s + 1) & 0x3f));
				luaL_addstring(b, hexbuf);
				s++;
			}
			else if (((*s >> 4) & 0x0f) == 0x0e) {
				luaL_addstring(b, "\\u");
				_snprintf(hexbuf, sizeof hexbuf, "%04x",
					((*s & 0x0f) << 12) |
					((*(s + 1) & 0x3f) << 6) |
					(*(s + 2) & 0x3f));
				s += 2;
			}
			else if (((*s >> 3) & 0x1f) == 0x1e) {
				/* deliberately ignored */
				s += 3;
			}
			break;
		}
	}
	luaL_addchar(b, '"');
}

static void
encode(lua_State *L, luaL_Buffer *b)
{
	int e, t, n, m;

	switch (lua_type(L, -1)) {
	case LUA_TBOOLEAN:
		luaL_addstring(b, lua_toboolean(L, -1) ? "true" : "false");
		lua_pop(L, 1);
		break;
	case LUA_TNUMBER:
		luaL_addvalue(b);
		break;
	case LUA_TSTRING:
		encode_string(L, b, (unsigned char *)lua_tostring(L, -1));
		lua_pop(L, 1);
		break;
	case LUA_TTABLE:
		/* check if this is the null value */
		lua_checkstack(L, 2);
		if (lua_getmetatable(L, -1)) {
			luaL_getmetatable(L, JSON_NULL_METATABLE);
#if LUA_VERSION_NUM >= 502
			if (lua_compare(L, -2, -1, LUA_OPEQ)) {
#else
			if (lua_equal(L, -2, -1)) {
#endif
				lua_pop(L, 2);
				luaL_addstring(b, "null");
				lua_pop(L, 1);
				break;
			}
			lua_pop(L, 2);
		}
		/* if there are t[1] .. t[n], output them as array */
		n = 0;
		e = 1;
		for (m = 1;; m++) {
			lua_pushnumber(L, m);
			lua_gettable(L, -2);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				break;
			}
			luaL_addchar(b, n ? ',' : '[');
			encode(L, b);
			n++;
		}
		if (n) {
			luaL_addchar(b, ']');
			lua_pop(L, 1);
			e = 0;
			break;
		}

		/* output non-numerical indices as object */
		t = lua_gettop(L);
		lua_pushnil(L);
		n = 0;
		while (lua_next(L, t) != 0) {
			if (lua_type(L, -2) == LUA_TNUMBER) {
				lua_pop(L, 1);
				continue;
			}
			luaL_addstring(b, n ? ",\"" : "{\"");
			luaL_addstring(b, lua_tostring(L, -2));
			luaL_addstring(b, "\":");
			encode(L, b);
			n++;
		}
		if (n) {
			luaL_addchar(b, '}');
			e = 0;
		}

		if (e)
			luaL_addstring(b, "[]");
		lua_pop(L, 1);
		break;
	case LUA_TNIL:
		luaL_addstring(b, "null");
		lua_pop(L, 1);
		break;
	default:
		json_error(L, "Lua type %s is incompatible with JSON",
			luaL_typename(L, -1));
		lua_pop(L, 1);
	}
}

static int
json_encode(lua_State *L)
{
	luaL_Buffer b;

	luaL_buffinit(L, &b);
	encode(L, &b);
	luaL_pushresult(&b);
	return 1;
}

static int
json_isnull(lua_State *L)
{
	if (lua_getmetatable(L, -1)) {
		luaL_getmetatable(L, JSON_NULL_METATABLE);
#if LUA_VERSION_NUM >= 502
		if (lua_compare(L, -2, -1, LUA_OPEQ)) {
#else
		if (lua_equal(L, -2, -1)) {
#endif
			lua_pop(L, 2);
			lua_pushboolean(L, 1);
			goto done;
		}
		lua_pop(L, 2);
	}
	lua_pushboolean(L, 0);
done:
	return 1;
}

static void
json_set_info(lua_State *L)
{
	lua_pushliteral(L, "_COPYRIGHT");
	lua_pushliteral(L, "Copyright (C) 2011 - 2015 "
		"micro systems marc balmer");
	lua_settable(L, -3);
	lua_pushliteral(L, "_DESCRIPTION");
	lua_pushliteral(L, "JSON encoder/decoder for Lua");
	lua_settable(L, -3);
	lua_pushliteral(L, "_VERSION");
	lua_pushliteral(L, "json 1.2.3");
	lua_settable(L, -3);
}

static int
json_null(lua_State *L)
{
	lua_pushstring(L, "null");
	return 1;
}

LUA_LIBRARY_EXPORT
luaopen_json(lua_State* L)
{
	static const struct luaL_Reg methods[] = {
		{ "decode", json_decode },
		{ "encode", json_encode },
		{ "isnull", json_isnull },
		{ NULL, NULL }
	};
	static const struct luaL_Reg null_methods[] = {
		{ "__tostring", json_null },
		{ "__call", json_null },
		{ NULL, NULL }
	};

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, methods);
#else
	luaL_register(L, "json", methods);
#endif
	json_set_info(L);

	lua_newtable(L);
	/* The null metatable */
	if (luaL_newmetatable(L, JSON_NULL_METATABLE)) {
#if LUA_VERSION_NUM >= 502
		luaL_setfuncs(L, null_methods, 0);
#else
		luaL_register(L, NULL, null_methods);
#endif
		lua_pushliteral(L, "__metatable");
		lua_pushliteral(L, "must not access this metatable");
		lua_settable(L, -3);

	}
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "null");
	return 1;
}