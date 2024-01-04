#include "../stdafx.h"

#define MT_NETLIBHTTPHEADERS "NETLIBHTTPHEADERS"

static NETLIBHTTPREQUEST* CreateRequest()
{
	NETLIBHTTPREQUEST *request = (NETLIBHTTPREQUEST*)mir_calloc(sizeof(NETLIBHTTPREQUEST));
	request->flags = NLHRF_HTTP11 | NLHRF_NODUMP;
	return request;
}

static void SetUrl(NETLIBHTTPREQUEST *request, const char *url)
{
	request->szUrl = mir_strdup(url);
	if (mir_strncmpi(request->szUrl, "https", 5) == 0)
		request->flags |= NLHRF_SSL;
	else
		request->flags &= ~(NLHRF_SSL);
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

	request->headers = (NETLIBHTTPHEADER*)mir_realloc(request->headers,
		sizeof(NETLIBHTTPHEADER)*(request->headersCount + 1));
	NETLIBHTTPHEADER &header = request->headers[request->headersCount];
	header.szName = mir_strdup(name);
	header.szValue = mir_strdup(value);
	request->headersCount++;
}

static void DropHeader(NETLIBHTTPREQUEST *request, const char *name)
{
	for (int i = 0; i < request->headersCount - 1; i++) {
		if (mir_strcmp(request->headers[i].szName, name) == 0) {
			mir_free(request->headers[i].szName);
			mir_free(request->headers[i].szValue);
			request->headersCount--;
			request->headers[i].szName = request->headers[request->headersCount].szName;
			request->headers[i].szValue = request->headers[request->headersCount].szValue;
		}
	}
	request->headers = (NETLIBHTTPHEADER*)mir_realloc(request->headers,
		sizeof(NETLIBHTTPHEADER)*(request->headersCount + 1));
}

static void ClearHeaders(NETLIBHTTPREQUEST *request)
{
	for (int i = 0; i < request->headersCount; i++) {
		mir_free(request->headers[i].szName);
		mir_free(request->headers[i].szValue);
	}
	request->headersCount = 0;
	request->headers = (NETLIBHTTPHEADER*)mir_realloc(request->headers,
			sizeof(NETLIBHTTPHEADER)*(request->headersCount + 1));
}

static void SetContent(NETLIBHTTPREQUEST *request, const char *data, size_t length)
{
	if (request->pData != nullptr)
		mir_free(request->pData);
	request->pData = mir_strdup(data);
	request->dataLength = (int)length;
}

/***********************************************/

struct NETLIBHTTPHEADERS
{
	const NETLIBHTTPHEADER *headers;
	int count;
};

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

static int headers__pairs(lua_State *L)
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
	{ "__pairs", headers__pairs },
	{ "__index", headers__index },
	{ "__len", headers__len },

	{ nullptr, nullptr }
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

	{ nullptr, nullptr }
};

/***********************************************/

#define MT_NETLIBHTTPRESPONSE "NETLIBHTTPRESPONSE"

static NETLIBHTTPREQUEST* response_Create(lua_State *L, NETLIBHTTPREQUEST *request)
{
	NETLIBHTTPREQUEST *response = Netlib_HttpTransaction(g_plugin.hNetlib, request);
	NETLIBHTTPREQUEST **udata = (NETLIBHTTPREQUEST**)lua_newuserdata(L, sizeof(NETLIBHTTPREQUEST*));
	*udata = response;
	luaL_setmetatable(L, MT_NETLIBHTTPRESPONSE);
	return response;
}

static int response__index(lua_State *L)
{
	NETLIBHTTPREQUEST *response = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPRESPONSE);
	const char *key = lua_tostring(L, 2);

	if (mir_strcmpi(key, "IsSuccess") == 0) {
		lua_pushboolean(L, response != nullptr && HTTP_CODE_SUCCESS(response->resultCode));
		return 1;
	}
	if (response == nullptr) {
		lua_pushnil(L);
		return 1;
	}
	if (mir_strcmpi(key, "StatusCode") == 0)
		lua_pushinteger(L, response->resultCode);
	else if (mir_strcmpi(key, "Headers") == 0) {
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

	{ nullptr, nullptr }
};

/***********************************************/

struct HttpRequestParam
{
	lua_State *L;
	int threadRef;
	int callbackRef;

	NETLIBHTTPREQUEST *request;
};

static void __cdecl SendHttpRequestThread(HttpRequestParam *param)
{
	Thread_SetName(MODULENAME ": SendHttpRequestThread");

	lua_rawgeti(param->L, LUA_REGISTRYINDEX, param->callbackRef);
	response_Create(param->L, param->request);
	luaM_pcall(param->L, 1, 0);

	luaL_unref(param->L, LUA_REGISTRYINDEX, param->callbackRef);
	luaL_unref(param->L, LUA_REGISTRYINDEX, param->threadRef);
	Netlib_FreeHttpRequest(param->request);
	delete param;
}

static void SendRequestAsync(lua_State *L, int idx, NETLIBHTTPREQUEST *request)
{
	HttpRequestParam *param = new HttpRequestParam();
	param->request = request;
	param->L = lua_newthread(L);
	param->threadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_pushvalue(L, idx);
	param->callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
	mir_forkThread<HttpRequestParam>(SendHttpRequestThread, param);
}

/***********************************************/

#define MT_NETLIBHTTPREQUEST "NETLIBHTTPREQUEST"

static const char *httpMethods[] = { "GET", "POST", "PUT", "DELETE", nullptr };

static int request_SetMethod(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	request->requestType = (1 << (luaL_checkoption(L, 2, nullptr, httpMethods)));
	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetUrl(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	const char *url = luaL_checkstring(L, 2);

	SetUrl(request, url);
	lua_pushvalue(L, 1);

	return 1;
}

static int request_SetHeaders(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	luaL_checktype(L, 2, LUA_TTABLE);

	ClearHeaders(request);
	for (lua_pushnil(L); lua_next(L, 2); lua_pop(L, 1)) {
		const char *name = lua_tostring(L, -2);
		const char *value = lua_tostring(L, -1);
		SetHeader(request, name, value);
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetContent(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);

	switch (lua_type(L, 2)) {
	case LUA_TNONE:
	case LUA_TNIL:
		SetContent(request, nullptr, 0);
		DropHeader(request, "Content-Type");
		break;
	case LUA_TSTRING:
	{
		const char *data = lua_tostring(L, 2);
		SetContent(request, data, mir_strlen(data));
		SetHeader(request, "Content-Type", "text/plain");
		break;
	}
	case LUA_TTABLE:
	{
		CMStringA formData;
		for (lua_pushnil(L); lua_next(L, 2); lua_pop(L, 1)) {
			const char *name = lua_tostring(L, -2);
			const char *value = lua_tostring(L, -1);
			formData.AppendFormat("&%s=%s", name, value);
		}
		formData.Delete(0);
		SetContent(request, formData.GetString(), formData.GetLength());
		SetHeader(request, "Content-Type", "application/x-www-form-urlencoded");
		break;
	}
	default:
		luaL_argerror(L, 2, luaL_typename(L, 2));
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetContentType(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	if (!lua_isstring(L, 2)) {
		lua_pushvalue(L, 1);
		return 1;
	}

	const char *type = lua_tostring(L, 2);
	SetHeader(request, "Content-Type", type);

	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetTimeout(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	request->timeout = luaL_optinteger(L, -1, 0);
	lua_pushvalue(L, 1);
	return 1;
}

static int request_Send(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);

	NETLIBHTTPREQUEST *newRequest = (NETLIBHTTPREQUEST*)mir_calloc(sizeof(NETLIBHTTPREQUEST));
	newRequest->flags = request->flags;
	newRequest->requestType = request->requestType;
	newRequest->szUrl = mir_strdup(request->szUrl);
	newRequest->headersCount = request->headersCount;
	newRequest->headers = (NETLIBHTTPHEADER*)mir_calloc(sizeof(NETLIBHTTPHEADER)*(request->headersCount + 1));
	for (int i = 0; i < request->headersCount; i++) {
		newRequest->headers[i].szName = mir_strdup(request->headers[i].szName);
		newRequest->headers[i].szValue = mir_strdup(request->headers[i].szValue);
	}
	newRequest->dataLength = request->dataLength;
	newRequest->pData = (char*)mir_calloc(request->dataLength + 1);
	memcpy(newRequest->pData, request->pData, request->dataLength);
	newRequest->timeout = request->timeout;

	if (lua_isfunction(L, 2)) {
		SendRequestAsync(L, 2, newRequest);
		return 0;
	}

	response_Create(L, newRequest);
	Netlib_FreeHttpRequest(newRequest);

	return 1;
}

static int request__index(lua_State *L)
{
	luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	const char *key = lua_tostring(L, 2);

	if (mir_strcmpi(key, "Method") == 0)
		lua_pushcfunction(L, request_SetMethod);
	else if (mir_strcmpi(key, "Url") == 0)
		lua_pushcfunction(L, request_SetUrl);
	else if (mir_strcmpi(key, "Headers") == 0)
		lua_pushcfunction(L, request_SetHeaders);
	else if (mir_strcmpi(key, "Content") == 0)
		lua_pushcfunction(L, request_SetContent);
	else if (mir_strcmpi(key, "ContentType") == 0)
		lua_pushcfunction(L, request_SetContentType);
	else if (mir_strcmpi(key, "Timeout") == 0)
		lua_pushcfunction(L, request_SetTimeout);
	else if (mir_strcmpi(key, "Send") == 0)
		lua_pushcfunction(L, request_Send);
	else
		lua_pushnil(L);

	return 1;
}

static int request__gc(lua_State *L)
{
	NETLIBHTTPREQUEST *request = *(NETLIBHTTPREQUEST**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	Netlib_FreeHttpRequest(request);
	return 0;
}

static const luaL_Reg requestApi[] =
{
	{ "__index", request__index },
	{ "__gc", request__gc },

	{ nullptr, nullptr }
};

/***********************************************/

static int http_Request(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest();
	NETLIBHTTPREQUEST **udata = (NETLIBHTTPREQUEST**)lua_newuserdata(L, sizeof(NETLIBHTTPREQUEST*));
	*udata = request;
	request->requestType = (1 << (luaL_checkoption(L, 1, nullptr, httpMethods)));
	SetUrl(request, luaL_checkstring(L, 2));
	luaL_setmetatable(L, MT_NETLIBHTTPREQUEST);

	return 1;
}

static int http_Get(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest();
	request->requestType = REQUEST_GET;

	const char *url = luaL_checkstring(L, 1);
	SetUrl(request, url);

	if (lua_isfunction(L, 2)) {
		SendRequestAsync(L, 2, request);
		return 0;
	}

	response_Create(L, request);
	Netlib_FreeHttpRequest(request);

	return 1;
}

static int http_Post(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest();
	request->requestType = REQUEST_POST;
	
	const char *url = luaL_checkstring(L, 1);
	SetUrl(request, url);

	lua_pushcfunction(L, request_SetContent);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, 2);
	luaM_pcall(L, 2, 1);

	lua_pushcfunction(L, request_SetContentType);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, 3);
	luaM_pcall(L, 2, 1);

	if (lua_isfunction(L, 4)) {
		SendRequestAsync(L, 4, request);
		return 0;
	}

	response_Create(L, request);
	Netlib_FreeHttpRequest(request);

	return 1;
}

static int http_Put(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest();
	request->requestType = REQUEST_PUT;

	const char *url = luaL_checkstring(L, 1);
	SetUrl(request, url);

	lua_pushcfunction(L, request_SetContent);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, 2);
	luaM_pcall(L, 2, 1);

	lua_pushcfunction(L, request_SetContentType);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, 3);
	luaM_pcall(L, 2, 1);

	if (lua_isfunction(L, 4)) {
		SendRequestAsync(L, 4, request);
		return 0;
	}

	response_Create(L, request);
	Netlib_FreeHttpRequest(request);

	return 1;
}

static int http_Delete(lua_State *L)
{
	NETLIBHTTPREQUEST *request = CreateRequest();
	request->requestType = REQUEST_DELETE;

	const char *url = luaL_checkstring(L, 1);
	SetUrl(request, url);

	lua_pushcfunction(L, request_SetContent);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, 2);
	luaM_pcall(L, 2, 1);

	lua_pushcfunction(L, request_SetContentType);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, 3);
	luaM_pcall(L, 2, 1);

	if (lua_isfunction(L, 4)) {
		SendRequestAsync(L, 4, request);
		return 0;
	}

	response_Create(L, request);

	return 1;
}

static const luaL_Reg httpApi[] =
{
	{ "Request", http_Request },
	{ "Get", http_Get },
	{ "Post", http_Post },
	{ "Put", http_Put },
	{ "Delete", http_Delete },

	{ nullptr, nullptr }
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
