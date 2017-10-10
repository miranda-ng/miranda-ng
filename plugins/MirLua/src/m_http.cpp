#include "stdafx.h"

/***********************************************/

#define MT_NETLIBHTTPHEADERS "NETLIBHTTPHEADERS"

struct NETLIBHTTPHEADERS
{
	const NETLIBHTTPHEADER *headers;
	int count;
};

static void AddHeader(NETLIBHTTPREQUEST *request, const char *name, const char *value)
{
	request->headers = (NETLIBHTTPHEADER*)mir_realloc(request->headers,
		sizeof(NETLIBHTTPHEADER)*(request->headersCount + 1));
	NETLIBHTTPHEADER &header = request->headers[request->headersCount];
	header.szName = mir_strdup(name);
	header.szValue = mir_strdup(value);
	request->headersCount++;
}

static void SetHeader(NETLIBHTTPREQUEST *request, const char *name, const char *value)
{
	for (int i = 0; i < request->headersCount; i++) {
		if (mir_strcmp(request->headers[i].szName, name) == 0) {
			mir_free(request->headers[i].szValue);
			request->headers[i].szValue = mir_strdup(value);
			return;
		}
	}
	AddHeader(request, name, value);
}

static int headers_Iterator(lua_State *L)
{
	NETLIBHTTPHEADER *headers = (NETLIBHTTPHEADER*)lua_touserdata(L, lua_upvalueindex(1));
	int count = lua_tointeger(L, lua_upvalueindex(2));
	int idx = lua_tointeger(L, lua_upvalueindex(3));

	if (idx < count) {
		lua_pushstring(L, headers[idx].szName);
		lua_pushstring(L, headers[idx].szValue);
		lua_pushinteger(L, idx + 1);
		lua_replace(L, lua_upvalueindex(3));
		return 2;
	}

	lua_pushnil(L);

	return 1;
}

static int headers__call(lua_State *L)
{
	NETLIBHTTPHEADERS *headers = (NETLIBHTTPHEADERS*)luaL_checkudata(L, 1, MT_NETLIBHTTPHEADERS);

	lua_pushlightuserdata(L, (void*)headers->headers);
	lua_pushinteger(L, headers->count);
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, headers_Iterator, 3);

	return 1;
}

static int headers__index(lua_State *L)
{
	NETLIBHTTPHEADERS *headers = (NETLIBHTTPHEADERS*)luaL_checkudata(L, 1, MT_NETLIBHTTPHEADERS);

	if (lua_isinteger(L, 2)) {
		int idx = lua_tointeger(L, 2);
		if (idx > 0 && idx <= headers->count) {
			lua_pushstring(L, headers->headers[idx - 1].szValue);
			return 1;
		}
	}

	const char *key = lua_tostring(L, 2);
	for (int i = 0; i < headers->count; i++) {
		if (mir_strcmp(headers->headers[i].szName, key) == 0) {
			lua_pushstring(L, headers->headers[i].szValue);
			return 1;
		}
	}

	lua_pushnil(L);

	return 1;
}

static int headers__len(lua_State *L)
{
	NETLIBHTTPHEADERS *headers = (NETLIBHTTPHEADERS*)luaL_checkudata(L, 1, MT_NETLIBHTTPHEADERS);

	lua_pushinteger(L, headers->count);

	return 1;
}

static const luaL_Reg headersApi[] =
{
	{ "__call", headers__call },
	{ "__index", headers__index },
	{ "__len", headers__len },

	{ NULL, NULL }
};

/***********************************************/

#define MT_NETLIBHTTPCONTENT "NETLIBHTTPCONTENT"

struct NETLIBHTTPCONTENT
{
	const char *data;
	int length;
};

static int content__index(lua_State *L)
{
	NETLIBHTTPCONTENT *content = (NETLIBHTTPCONTENT*)luaL_checkudata(L, 1, MT_NETLIBHTTPCONTENT);

	int idx = luaL_checkinteger(L, 2);
	if (idx > 0 && idx <= content->length) {
		lua_pushinteger(L, content->data[idx - 1]);
		return 1;
	}

	lua_pushnil(L);

	return 1;
}

static int content__len(lua_State *L)
{
	NETLIBHTTPCONTENT *content = (NETLIBHTTPCONTENT*)luaL_checkudata(L, 1, MT_NETLIBHTTPCONTENT);

	lua_pushinteger(L, content->length);

	return 1;
}

static int content__tostring(lua_State *L)
{
	NETLIBHTTPCONTENT *content = (NETLIBHTTPCONTENT*)luaL_checkudata(L, 1, MT_NETLIBHTTPCONTENT);

	lua_pushlstring(L, content->data, content->length);

	return 1;
}

static const luaL_Reg contentApi[] =
{
	{ "__index", content__index },
	{ "__len", content__len },
	{ "__tostring", content__tostring },

	{ NULL, NULL }
};

/***********************************************/

#define MT_NETLIBHTTPRESPONSE "NETLIBHTTPRESPONSE"

static NETLIBHTTPREQUEST* response_Create(lua_State *L, NETLIBHTTPREQUEST *request)
{
	NETLIBHTTPREQUEST **response = (NETLIBHTTPREQUEST**)lua_newuserdata(L, sizeof(NETLIBHTTPREQUEST*));
	*response = Netlib_HttpTransaction(hNetlib, request);
	luaL_setmetatable(L, MT_NETLIBHTTPRESPONSE);

	return *response;
}

static int response__index(lua_State *L)
{
	NETLIBHTTPREQUEST *response = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPRESPONSE);
	const char *key = lua_tostring(L, 2);

	if (mir_strcmpi(key, "Headers") == 0) {
		NETLIBHTTPHEADERS *headers = (NETLIBHTTPHEADERS*)lua_newuserdata(L, sizeof(NETLIBHTTPHEADERS));
		headers->headers = response->headers;
		headers->count = response->headersCount;
		luaL_setmetatable(L, MT_NETLIBHTTPHEADERS);
	}
	else if (mir_strcmpi(key, "Content") == 0) {
		NETLIBHTTPCONTENT *content = (NETLIBHTTPCONTENT*)lua_newuserdata(L, sizeof(NETLIBHTTPCONTENT));
		content->data = response->pData;
		content->length = response->dataLength;
		luaL_setmetatable(L, MT_NETLIBHTTPCONTENT);
	}
	else if (mir_strcmpi(key, "StatusCode") == 0)
		lua_pushinteger(L, response->resultCode);
	else if (mir_strcmpi(key, "IsSuccess") == 0) {
		lua_pushboolean(L, HTTP_CODE_SUCCESS(response->resultCode));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int response__gc(lua_State *L)
{
	NETLIBHTTPREQUEST **response = (NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPRESPONSE);

	Netlib_FreeHttpRequest(*response);

	return 0;
}

static const luaL_Reg responseApi[] =
{
	{ "__index", response__index },
	{ "__gc", response__gc },

	{ NULL, NULL }
};

/***********************************************/

#define MT_NETLIBHTTPREQUEST "NETLIBHTTPREQUEST"

static NETLIBHTTPREQUEST* CreateRequest(lua_State *L)
{
	NETLIBHTTPREQUEST **request = (NETLIBHTTPREQUEST**)lua_newuserdata(L, sizeof(NETLIBHTTPREQUEST*));
	*request = (NETLIBHTTPREQUEST*)mir_calloc(sizeof(NETLIBHTTPREQUEST));
	(*request)->cbSize = sizeof(NETLIBHTTPREQUEST);
	(*request)->flags = NLHRF_HTTP11 | NLHRF_NODUMP;

	luaL_setmetatable(L, MT_NETLIBHTTPREQUEST);

	return *request;
}

static void request_SetUrl(lua_State *L, int idx, NETLIBHTTPREQUEST *request)
{
	const char *url = luaL_checkstring(L, idx);
	request->szUrl = mir_strdup(url);
	if (mir_strncmpi(request->szUrl, "https", 5) == 0)
		request->flags |= NLHRF_SSL;
	else
		request->flags &= ~(NLHRF_SSL);
}

static void request_SetHeaders(lua_State *L, int idx, NETLIBHTTPREQUEST *request)
{
	if (lua_isnoneornil(L, idx))
		return;

	luaL_checktype(L, idx, LUA_TTABLE);

	idx = lua_absindex(L, idx);
	for (lua_pushnil(L); lua_next(L, idx); lua_pop(L, 2)) {
		lua_pushvalue(L, -2);
		const char *name = lua_tostring(L, -1);
		const char *value = lua_tostring(L, -2);
		AddHeader(request, name, value);
	}
}

static void request_SetContent(lua_State *L, int idx, NETLIBHTTPREQUEST *request)
{
	CMStringA data;

	switch (lua_type(L, idx))
	{
	case LUA_TNONE:
	case LUA_TNIL:
		return;
	case LUA_TSTRING:
		data = lua_tostring(L, idx);
		SetHeader(request, "Content-Type", "text/plain");
		break;
	case LUA_TTABLE:
	{
		idx = lua_absindex(L, idx);
		for (lua_pushnil(L); lua_next(L, idx); lua_pop(L, 2)) {
			lua_pushvalue(L, -2);
			const char *name = lua_tostring(L, -1);
			const char *value = lua_tostring(L, -2);
			data.AppendFormat("&%s=%s", name, value);
		}
		data.Delete(0);
		SetHeader(request, "Content-Type", "application/x-www-form-urlencoded");
		break;
	}
	default:
		luaL_argerror(L, idx, luaL_typename(L, idx));
	}

	request->pData = mir_strdup(data);
	request->dataLength = data.GetLength();
}

static void request_SetContentType(lua_State *L, int idx, NETLIBHTTPREQUEST *request)
{
	if (!lua_isstring(L, 2))
		return;

	const char *type = lua_tostring(L, 2);
	SetHeader(request, "Content-Type", type);
}

static const char *httpMethods[] = { "GET", "POST", "PUT", "DELETE", NULL };

static int request_Send(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	NETLIBHTTPREQUEST *request = CreateRequest(L);

	lua_getfield(L, 1, "Method");
	request->requestType = (1 << (luaL_checkoption(L, -1, NULL, httpMethods)));
	lua_pop(L, 1);

	lua_getfield(L, 1, "Url");
	request_SetUrl(L, -1, request);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Headers");
	request_SetHeaders(L, -1, request);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Content");
	request_SetContent(L, -1, request);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Timeout");
	request->timeout = luaL_optinteger(L, -1, 0);
	lua_pop(L, 1);

	response_Create(L, request);

	return 1;
}

static int request_Get(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest(L);
	request->requestType = REQUEST_GET;
	request_SetUrl(L, 1, request);

	response_Create(L, request);

	return 1;
}

static int request_Post(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest(L);
	request->requestType = REQUEST_POST;
	request_SetUrl(L, 1, request);
	request_SetContent(L, 2, request);
	request_SetContentType(L, 3, request);

	response_Create(L, request);

	return 1;
}

static int request_Put(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest(L);
	request->requestType = REQUEST_PUT;
	request_SetUrl(L, 1, request);
	request_SetContent(L, 2, request);
	request_SetContentType(L, 3, request);

	response_Create(L, request);

	return 1;
}

static int request_Delete(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest(L);
	request->requestType = REQUEST_DELETE;
	request_SetUrl(L, 1, request);
	request_SetContent(L, 2, request);
	request_SetContentType(L, 2, request);

	response_Create(L, request);

	return 1;
}

static int request__gc(lua_State *L)
{
	NETLIBHTTPREQUEST **request = (NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);

	mir_free((*request)->szUrl);
	for (size_t i = 0; i < (*request)->headersCount; i++) {
		mir_free((*request)->headers[i].szName);
		mir_free((*request)->headers[i].szValue);
	}
	mir_free((*request)->headers);
	mir_free((*request)->pData);

	return 0;
}

static const luaL_Reg requestApi[] =
{
	{ "__gc", request__gc },

	{ NULL, NULL }
};

/***********************************************/

static const luaL_Reg httpApi[] =
{
	{ "Send", request_Send },
	{ "Get", request_Get },
	{ "Post", request_Post },
	{ "Put", request_Put },
	{ "Delete", request_Delete },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_http(lua_State *L)
{
	luaL_newlib(L, httpApi);

	luaL_newmetatable(L, MT_NETLIBHTTPREQUEST);
	luaL_setfuncs(L, requestApi, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, MT_NETLIBHTTPRESPONSE);
	luaL_setfuncs(L, responseApi, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, MT_NETLIBHTTPHEADERS);
	luaL_setfuncs(L, headersApi, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, MT_NETLIBHTTPCONTENT);
	luaL_setfuncs(L, contentApi, 0);
	lua_pop(L, 1);

	return 1;
}
