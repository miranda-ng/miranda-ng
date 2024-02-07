#include "../stdafx.h"

#define MT_NETLIBHTTPHEADERS "NETLIBHTTPHEADERS"

static MHttpRequest* CreateRequest()
{
	MHttpRequest *request = new MHttpRequest(REQUEST_GET);
	request->flags = NLHRF_HTTP11 | NLHRF_NODUMP;
	return request;
}

static void SetUrl(MHttpRequest *request, const char *url)
{
	request->m_szUrl = url;
	if (mir_strncmpi(request->m_szUrl, "https", 5) == 0)
		request->flags |= NLHRF_SSL;
	else
		request->flags &= ~(NLHRF_SSL);
}

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

static MHttpResponse* response_Create(lua_State *L, MHttpRequest *request)
{
	auto *response = Netlib_HttpTransaction(g_plugin.hNetlib, request);
	MHttpResponse **udata = (MHttpResponse **)lua_newuserdata(L, sizeof(MHttpResponse *));
	*udata = response;
	luaL_setmetatable(L, MT_NETLIBHTTPRESPONSE);
	return response;
}

static int response__index(lua_State *L)
{
	MHttpResponse *response = *(MHttpResponse **)luaL_checkudata(L, 1, MT_NETLIBHTTPRESPONSE);
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
	/*else if (mir_strcmpi(key, "Headers") == 0) {
		NETLIBHTTPHEADERS *headers = (NETLIBHTTPHEADERS*)lua_newuserdata(L, sizeof(NETLIBHTTPHEADERS));
		headers->headers = response->headers;
		headers->count = response->headersCount;
		luaL_setmetatable(L, MT_NETLIBHTTPHEADERS);
	}*/
	else if (mir_strcmpi(key, "Content") == 0) {
		NETLIBHTTPCONTENT *content = (NETLIBHTTPCONTENT*)lua_newuserdata(L, sizeof(NETLIBHTTPCONTENT));
		content->data = response->body.GetBuffer();
		content->length = response->body.GetLength();
		luaL_setmetatable(L, MT_NETLIBHTTPCONTENT);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int response__gc(lua_State *L)
{
	MHttpResponse **response = (MHttpResponse **)luaL_checkudata(L, 1, MT_NETLIBHTTPRESPONSE);
	delete *response;
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

	MHttpRequest *request;
};

static void __cdecl SendHttpRequestThread(HttpRequestParam *param)
{
	Thread_SetName(MODULENAME ": SendHttpRequestThread");

	lua_rawgeti(param->L, LUA_REGISTRYINDEX, param->callbackRef);
	response_Create(param->L, param->request);
	luaM_pcall(param->L, 1, 0);

	luaL_unref(param->L, LUA_REGISTRYINDEX, param->callbackRef);
	luaL_unref(param->L, LUA_REGISTRYINDEX, param->threadRef);
	delete param->request;
	delete param;
}

static void SendRequestAsync(lua_State *L, int idx, MHttpRequest *request)
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

#define MT_NETLIBHTTPREQUEST "MHttpRequest"

static const char *httpMethods[] = { "GET", "POST", "PUT", "DELETE", nullptr };

static int request_SetMethod(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	request->requestType = (1 << (luaL_checkoption(L, 2, nullptr, httpMethods)));
	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetUrl(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	const char *url = luaL_checkstring(L, 2);

	SetUrl(request, url);
	lua_pushvalue(L, 1);

	return 1;
}

static int request_SetHeaders(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	luaL_checktype(L, 2, LUA_TTABLE);

	request->destroy();
	for (lua_pushnil(L); lua_next(L, 2); lua_pop(L, 1)) {
		const char *name = lua_tostring(L, -2);
		const char *value = lua_tostring(L, -1);
		request->AddHeader(name, value);
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetContent(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);

	switch (lua_type(L, 2)) {
	case LUA_TNONE:
	case LUA_TNIL:
		request->SetData(nullptr, 0);
		request->DeleteHeader("Content-Type");
		break;
	case LUA_TSTRING:
		{
			const char *data = lua_tostring(L, 2);
			request->SetData(data, mir_strlen(data));
			request->AddHeader("Content-Type", "text/plain");
		}
		break;
	case LUA_TTABLE:
		{
			CMStringA formData;
			for (lua_pushnil(L); lua_next(L, 2); lua_pop(L, 1)) {
				const char *name = lua_tostring(L, -2);
				const char *value = lua_tostring(L, -1);
				formData.AppendFormat("&%s=%s", name, value);
			}
			formData.Delete(0);
			request->SetData(formData.GetString(), formData.GetLength());
			request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
		}
		break;
	default:
		luaL_argerror(L, 2, luaL_typename(L, 2));
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetContentType(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	if (!lua_isstring(L, 2)) {
		lua_pushvalue(L, 1);
		return 1;
	}

	const char *type = lua_tostring(L, 2);
	request->AddHeader("Content-Type", type);

	lua_pushvalue(L, 1);
	return 1;
}

static int request_SetTimeout(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	request->timeout = luaL_optinteger(L, -1, 0);
	lua_pushvalue(L, 1);
	return 1;
}

static int request_Send(lua_State *L)
{
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);

	MHttpRequest *newRequest = new MHttpRequest(request->requestType);
	newRequest->flags = request->flags;
	newRequest->m_szUrl = request->m_szUrl;
	newRequest->m_szParam = request->m_szParam;
	newRequest->timeout = request->timeout;
	for (auto &it : *request)
		newRequest->AddHeader(it->szName, it->szValue);

	if (lua_isfunction(L, 2)) {
		SendRequestAsync(L, 2, newRequest);
		return 0;
	}

	response_Create(L, newRequest);
	delete newRequest;
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
	MHttpRequest *request = *(MHttpRequest**)luaL_checkudata(L, 1, MT_NETLIBHTTPREQUEST);
	delete request;
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
	MHttpRequest *request = CreateRequest();
	MHttpRequest **udata = (MHttpRequest**)lua_newuserdata(L, sizeof(MHttpRequest*));
	*udata = request;
	request->requestType = (1 << (luaL_checkoption(L, 1, nullptr, httpMethods)));
	SetUrl(request, luaL_checkstring(L, 2));
	luaL_setmetatable(L, MT_NETLIBHTTPREQUEST);

	return 1;
}

static int http_Get(lua_State *L)
{
	MHttpRequest *request = CreateRequest();
	request->requestType = REQUEST_GET;

	const char *url = luaL_checkstring(L, 1);
	SetUrl(request, url);

	if (lua_isfunction(L, 2)) {
		SendRequestAsync(L, 2, request);
		return 0;
	}

	response_Create(L, request);
	delete request;
	return 1;
}

static int http_Post(lua_State *L)
{
	MHttpRequest *request = CreateRequest();
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
	delete request;
	return 1;
}

static int http_Put(lua_State *L)
{
	MHttpRequest *request = CreateRequest();
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
	delete request;
	return 1;
}

static int http_Delete(lua_State *L)
{
	MHttpRequest *request = CreateRequest();
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

	// luaL_newmetatable(L, MT_NETLIBHTTPHEADERS);
	// luaL_setfuncs(L, headersApi, 0);
	// lua_pop(L, 1);

	luaL_newmetatable(L, MT_NETLIBHTTPCONTENT);
	luaL_setfuncs(L, contentApi, 0);
	lua_pop(L, 1);

	return 1;
}
