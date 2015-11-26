#ifndef _LUA_METATABLE_H_
#define _LUA_METATABLE_H_

#include <map>
#include <string>

#define LUA_TINTEGER LUA_NUMTAGS + 1
#define LUA_TSTRINGA LUA_NUMTAGS + 2
#define LUA_TSTRINGW LUA_NUMTAGS + 3

struct MTField
{
	size_t offset;
	size_t size;
	int type;

	MTField(size_t offset, size_t size, int type)
		: offset(offset), size(size), type(type) { }
};

template<typename T>
class MT
{
private:
	static const char *name;
	static std::map<std::string, MTField*> fields;

	template<typename R>
	static R GetValue(const T *obj, size_t offset, size_t size)
	{
		R res = NULL;
		memcpy(&res, ((char*)obj) + offset, size);
		return res;
	}

	static int lua__new(lua_State *L)
	{
		T *obj = NULL;
		T *udata = NULL;

		int type = lua_type(L, 1);
		switch (type)
		{
		case LUA_TLIGHTUSERDATA:
			obj = (T*)MT::Load(L);
			if (obj == NULL) break;
		//case LUA_TNONE:
			udata = (T*)lua_newuserdata(L, sizeof(T));
			memcpy(udata, obj, sizeof(T));
		case LUA_TUSERDATA:
			luaL_setmetatable(L, MT::name);
			return 1;
		}
		lua_pushnil(L);

		return 1;
	}

	static int lua__index(lua_State *L)
	{
		T *obj = (T*)luaL_checkudata(L, 1, MT::name);
		const char *key = lua_tostring(L, 2);

		auto it = fields.find(key);
		if (it == fields.end())
		{
			lua_pushnil(L);
			return 1;
		}

		MTField *field = it->second;
		size_t offset = field->offset;
		size_t size = field->size;
		
		switch (field->type)
		{
		case LUA_TBOOLEAN:
			lua_pushboolean(L, GetValue<bool>(obj, offset, size));
			break;
		case LUA_TINTEGER:
			lua_pushinteger(L, GetValue<long long>(obj, offset, size));
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, GetValue<double>(obj, offset, size));
			break;
		case LUA_TSTRING:
			lua_pushstring(L, GetValue<char*>(obj, offset, size));
			break;
		case LUA_TSTRINGA:
			lua_pushstring(L, ptrA(mir_utf8encode(GetValue<char*>(obj, offset, size))));
			break;
		case LUA_TSTRINGW:
			lua_pushstring(L, ptrA(mir_utf8encodeW(GetValue<wchar_t*>(obj, offset, size))));
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, GetValue<void*>(obj, offset, size));
			break;
		default:
			lua_pushnil(L);
		}

		return 1;
	}

	static int lua__gc(lua_State *L)
	{
		T* obj = (T*)luaL_checkudata(L, 1, MT::name);
		MT::Free(&obj);
		return 0;
	}

	static T* Load(lua_State *L) { return (T*)lua_touserdata(L, 1); }
	static void Free(T **obj) { *obj = NULL; }

public:
	MT(lua_State *L, const char *tname)
	{
		MT::name = tname;

		lua_pushcfunction(L, lua__new);
		lua_setglobal(L, MT::name);

		luaL_newmetatable(L, MT::name);
		lua_pushcfunction(L, lua__index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, lua__gc);
		lua_setfield(L, -2, "__gc");
	}

	template<typename R>
	MT& Field(R T::*M, const char *name, int type, size_t size = sizeof(R))
	{
		size_t offset = reinterpret_cast<size_t>(&(((T*)0)->*M));
		if (type != LUA_TNONE)
			fields[name] = new MTField(offset, size, type);
		return *this;
	}

	static void Set(lua_State *L, T* obj)
	{
		if (obj == NULL)
		{
			lua_pushnil(L);
			return;
		}

		T *udata = (T*)lua_newuserdata(L, sizeof(T));
		memcpy(udata, obj, sizeof(T));
		luaL_setmetatable(L, MT::name);
	}
};

template<typename T>
const char *MT<T>::name;

template<typename T>
std::map<std::string, MTField*> MT<T>::fields;

#endif //_LUA_METATABLE_H_
